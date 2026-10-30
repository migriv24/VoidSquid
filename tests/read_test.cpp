/* read_test.cpp — recon, as pure functions (okf/concepts/recon.md).
 *
 * No file is opened here: every header is built in memory, which is the point of
 * squid_read being pure. In the order each would be most expensive to be wrong:
 *   1. The checksum: CRC-16/MODBUS against the standard check value, so the DS
 *      header tests below are not verifying the function with itself.
 *   2. A verified claim is only made when a checksum agrees; one changed byte drops it.
 *   3. The three N64 byte orders read as one header.
 *   4. Install fingerprints, and the residual that says what recon did not look at.
 *   5. Routes always end at the structure depth, unbuilt, and the report stays small.
 */
#include "read/recon.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace squid::read;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { ++failures; std::printf("  FAIL  %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static bool contains(const std::string& h, const std::string& n) { return h.find(n) != std::string::npos; }
static std::string get(const Identity& id, const std::string& key) {
    for (const Field& f : id.header) if (f.key == key) return f.value;
    return "<absent>";
}
static void put(std::vector<uint8_t>& b, size_t at, const std::string& s) { std::memcpy(b.data() + at, s.data(), s.size()); }
static void put32le(std::vector<uint8_t>& b, size_t at, uint32_t v) { for (int k = 0; k < 4; ++k) b[at + k] = static_cast<uint8_t>(v >> (8 * k)); }
static void put32be(std::vector<uint8_t>& b, size_t at, uint32_t v) { for (int k = 0; k < 4; ++k) b[at + k] = static_cast<uint8_t>(v >> (24 - 8 * k)); }
static bool has_confidence(const Identity& id, const std::string& c) {
    for (const Finding& f : id.findings) if (f.confidence == c) return true;
    return false;
}

static std::vector<uint8_t> nds_header() {
    std::vector<uint8_t> h(0x200, 0);
    put(h, 0x000, "VOIDSQUID");
    put(h, 0x00C, "AVSE");
    put(h, 0x010, "01");
    h[0x014] = 7;                       // 128 KiB << 7 = 16 MiB
    put32le(h, 0x02C, 0x40000);         // ARM9 size
    put32le(h, 0x03C, 0x20000);         // ARM7 size
    put32le(h, 0x04C, 42 * 8);          // FAT: 42 files
    put32le(h, 0x054, 3 * 32);          // three ARM9 overlays
    h[0x15C] = 0x56; h[0x15D] = 0xCF;   // the logo CRC constant
    const uint16_t crc = crc16_modbus(std::span<const uint8_t>(h.data(), 0x15E));
    h[0x15E] = static_cast<uint8_t>(crc); h[0x15F] = static_cast<uint8_t>(crc >> 8);
    return h;
}

int main() {
    /* 1. The standard check value for CRC-16/MODBUS over "123456789". */
    {
        const char* s = "123456789";
        CHECK(crc16_modbus(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(s), 9)) == 0x4B37);
    }

    /* 2. Nintendo DS: verified by its CRC, whatever the file is called. */
    {
        const std::vector<uint8_t> h = nds_header();
        const Identity id = identify_file("dump.bin", h, 16u << 20);
        CHECK(id.platform == "nds");
        CHECK(id.kind == "rom");
        CHECK(id.title == "VOIDSQUID");
        CHECK(get(id, "game_code") == "AVSE");
        CHECK(get(id, "files") == "42");
        CHECK(get(id, "arm9_overlays") == "3");
        CHECK(get(id, "capacity_bytes") == "16777216");
        CHECK(has_confidence(id, "verified"));

        std::vector<uint8_t> bad = h;
        bad[0x000] = 'W';   // one byte changed, CRC now wrong
        CHECK(identify_file("dump.bin", bad, 0).platform != "nds");   // no name, no checksum: not claimed
        const Identity named = identify_file("game.nds", bad, 0);
        CHECK(named.platform == "nds");
        CHECK(!has_confidence(named, "verified"));
        CHECK(has_confidence(named, "weak"));
        bool said = false;
        for (const std::string& r : named.residual) said |= contains(r, "CRC does not match");
        CHECK(said);
    }

    /* 3. N64: three byte orders, one header. */
    {
        std::vector<uint8_t> z(0x40, 0);
        put32be(z, 0x00, 0x80371240);
        put32be(z, 0x08, 0x80000400);
        put(z, 0x20, "VOID SQUID          ");
        put(z, 0x3B, "NVSE");
        z[0x3F] = 1;
        std::vector<uint8_t> v = z, n = z;
        for (size_t i = 0; i + 1 < v.size(); i += 2) std::swap(v[i], v[i + 1]);
        for (size_t i = 0; i + 3 < n.size(); i += 4) { std::swap(n[i], n[i + 3]); std::swap(n[i + 1], n[i + 2]); }
        for (const auto* b : {&z, &v, &n}) {
            const Identity id = identify_file("rom", *b, 8u << 20);
            CHECK(id.platform == "n64");
            CHECK(id.title == "VOID SQUID");
            CHECK(get(id, "game_code") == "NVSE");
            CHECK(get(id, "entry_point") == "0x80000400");
        }
        CHECK(contains(get(identify_file("rom", v, 0), "byte_order"), ".v64"));
        CHECK(contains(get(identify_file("rom", n, 0), "byte_order"), ".n64"));
    }

    /* GBA and Game Boy: their own checksums, computed here from the documented formulas. */
    {
        std::vector<uint8_t> g(0xC0, 0);
        put(g, 0xA0, "VOIDSQUID");
        put(g, 0xAC, "BVSE");
        put(g, 0xB0, "01");
        g[0xB2] = 0x96;
        int chk = 0;
        for (size_t i = 0xA0; i <= 0xBC; ++i) chk -= g[i];
        g[0xBD] = static_cast<uint8_t>((chk - 0x19) & 0xFF);
        const Identity id = identify_file("x.bin", g, 0);
        CHECK(id.platform == "gba");
        CHECK(get(id, "game_code") == "BVSE");
        CHECK(has_confidence(id, "verified"));

        std::vector<uint8_t> b(0x150, 0);
        b[0x104] = 0xCE; b[0x105] = 0xED; b[0x106] = 0x66; b[0x107] = 0x66;
        put(b, 0x134, "VOIDSQUID");
        b[0x148] = 2;
        int x = 0;
        for (size_t i = 0x134; i <= 0x14C; ++i) x = x - b[i] - 1;
        b[0x14D] = static_cast<uint8_t>(x & 0xFF);
        const Identity gb = identify_file("x.gb", b, 0);
        CHECK(gb.platform == "gb");
        CHECK(gb.title == "VOIDSQUID");
        CHECK(get(gb, "rom_bytes") == "131072");
        CHECK(has_confidence(gb, "verified"));
    }

    /* GameCube and Wii discs; a Godot pack; a PE executable; garbage. */
    {
        std::vector<uint8_t> gc(0x80, 0);
        put(gc, 0x00, "GVSE01");
        put32be(gc, 0x1C, 0xC2339F3D);
        put(gc, 0x20, "Void Squid");
        const Identity c = identify_file("x.iso", gc, 0);
        CHECK(c.platform == "gamecube");
        CHECK(get(c, "game_id") == "GVSE01");
        CHECK(c.title == "Void Squid");

        std::vector<uint8_t> wii(0x80, 0);
        put32be(wii, 0x18, 0x5D1C9EA3);
        CHECK(identify_file("x.iso", wii, 0).platform == "wii");

        std::vector<uint8_t> pck(0x20, 0);
        put(pck, 0, "GDPC");
        put32le(pck, 4, 2); put32le(pck, 8, 4); put32le(pck, 12, 3); put32le(pck, 16, 0);
        const Identity gd = identify_file("game.pck", pck, 0);
        CHECK(gd.engine == "godot");
        CHECK(get(gd, "godot_version") == "4.3.0");

        std::vector<uint8_t> pe(0x100, 0);
        put(pe, 0, "MZ");
        put32le(pe, 0x3C, 0x80);
        put(pe, 0x80, std::string("PE\0\0", 4));
        pe[0x84] = 0x64; pe[0x85] = 0x86;
        const Identity ex = identify_file("Game.exe", pe, 0);
        CHECK(ex.kind == "executable");
        CHECK(contains(get(ex, "machine"), "x86-64"));

        const std::vector<uint8_t> junk(64, 0xAB);
        const Identity u = identify_file("mystery.dat", junk, 64);
        CHECK(u.kind == "unknown");
        CHECK(!u.residual.empty());
        CHECK(identify_file("empty", std::span<const uint8_t>(), 0).kind == "unknown");   // no bytes: no crash, no claim
    }

    /* 4. Installs, by listing. */
    {
        const Identity mono = identify_install({{"MyGame.exe", 1}, {"UnityPlayer.dll", 1}, {"MyGame_Data/globalgamemanagers", 1},
                                                {"MyGame_Data/Managed/Assembly-CSharp.dll", 1}, {"UnityCrashHandler64.exe", 1}}, false);
        CHECK(mono.engine == "unity-mono");
        CHECK(mono.title == "MyGame");

        const Identity il2 = identify_install({{"UnityPlayer.dll", 1}, {"GameAssembly.dll", 1},
                                               {"X_Data/il2cpp_data/Metadata/global-metadata.dat", 1}}, false);
        CHECK(il2.engine == "unity-il2cpp");

        const Identity ue = identify_install({{"Brave/Binaries/Win64/Brave-Win64-Shipping.exe", 1}, {"Brave/Content/Paks/Brave-Windows.pak", 1}}, false);
        CHECK(ue.engine == "unreal");
        CHECK(ue.title == "Brave");

        CHECK(identify_install({{"data.win", 1}, {"Game.exe", 1}}, false).engine == "gamemaker");
        CHECK(identify_install({{"renpy/common/00start.rpy", 1}, {"game/script.rpy", 1}}, false).engine == "renpy");
        CHECK(identify_install({{"Game.exe", 1}, {"Game.pck", 1}}, false).engine == "godot");

        /* A tool's .jar deep in a source tree is not a Java game (found running recon
         * on universal-modder's own folder, 2026-10-10). */
        CHECK(identify_install({{"examples/mc/gradle/wrapper/gradle-wrapper.jar", 1}}, false).engine.empty());
        CHECK(identify_install({{"minecraft.jar", 1}}, false).engine == "java");

        const Identity none = identify_install({}, false);
        CHECK(none.engine.empty());
        CHECK(!none.residual.empty());

        const Identity cut = identify_install({{"Game.exe", 1}}, true);
        bool said = false, um = false;
        for (const std::string& r : cut.residual) { said |= contains(r, "truncated"); um |= contains(r, "um scan"); }
        CHECK(said);
        CHECK(um);   // recon says what it did not check, and who does
    }

    /* 5. Routes end at structure, unbuilt; the report is bounded. */
    {
        const Identity n64 = identify_file("x.z64", [] { std::vector<uint8_t> z(0x40, 0); put32be(z, 0, 0x80371240); return z; }(), 0);
        const std::vector<Route> r = routes_for(n64);
        CHECK(!r.empty());
        CHECK(r.back().depth == Depth::structure);
        CHECK(contains(r.back().does, "NOT BUILT"));
        bool recomp = false;
        for (const Route& x : r) recomp |= x.tool == "N64Recomp" && x.depth == Depth::opaque;
        CHECK(recomp);
        for (const Identity& id : {identify_file("x", std::span<const uint8_t>(), 0), identify_install({}, false)})
            CHECK(routes_for(id).back().depth == Depth::structure);

        std::vector<Entry> big;
        for (int i = 0; i < 20000; ++i) big.push_back({"Content/Paks/chunk" + std::to_string(i) + ".pak", 1});
        const Identity huge = identify_install(big, true);
        const std::string j = recon_json(huge, routes_for(huge));
        std::printf("  20000-entry install: engine=%s, answer %zu bytes\n", huge.engine.c_str(), j.size());
        CHECK(huge.engine == "unreal");
        CHECK(j.size() < 4096);
        CHECK(contains(j, "\"found\":null"));
    }

    std::printf(failures ? "squid_read: %d failure(s)\n" : "squid_read: all checks passed\n", failures);
    return failures ? 1 : 0;
}
