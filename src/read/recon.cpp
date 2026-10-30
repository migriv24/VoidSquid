/* recon.cpp — what is this artifact? Pure; see recon.hpp. */
#include "read/recon.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace squid::read {
namespace {

/* ── small byte readers; every one bounds-checks, because `head` may be short ── */

bool has_bytes(std::span<const uint8_t> b, size_t at, size_t n) { return at + n <= b.size(); }

uint16_t le16(std::span<const uint8_t> b, size_t at) {
    return has_bytes(b, at, 2) ? static_cast<uint16_t>(b[at] | (b[at + 1] << 8)) : 0;
}
uint32_t le32(std::span<const uint8_t> b, size_t at) {
    if (!has_bytes(b, at, 4)) return 0;
    return static_cast<uint32_t>(b[at]) | (static_cast<uint32_t>(b[at + 1]) << 8) |
           (static_cast<uint32_t>(b[at + 2]) << 16) | (static_cast<uint32_t>(b[at + 3]) << 24);
}
uint32_t be32(std::span<const uint8_t> b, size_t at) {
    if (!has_bytes(b, at, 4)) return 0;
    return (static_cast<uint32_t>(b[at]) << 24) | (static_cast<uint32_t>(b[at + 1]) << 16) |
           (static_cast<uint32_t>(b[at + 2]) << 8) | static_cast<uint32_t>(b[at + 3]);
}
bool magic(std::span<const uint8_t> b, size_t at, std::string_view m) {
    if (!has_bytes(b, at, m.size())) return false;
    for (size_t i = 0; i < m.size(); ++i)
        if (b[at + i] != static_cast<uint8_t>(m[i])) return false;
    return true;
}

/* Header text: printable ASCII kept, anything else shown as '?', trailing blanks and
 * NULs trimmed, stopping at the first NUL. Shift-JIS titles (N64, GameCube) come out
 * as question marks, and the residual says so rather than guessing an encoding. */
std::string text(std::span<const uint8_t> b, size_t at, size_t n, bool* non_ascii = nullptr) {
    std::string s;
    for (size_t i = 0; i < n && at + i < b.size(); ++i) {
        const uint8_t c = b[at + i];
        if (c == 0) break;
        if (c >= 0x20 && c < 0x7F) s += static_cast<char>(c);
        else { s += '?'; if (non_ascii) *non_ascii = true; }
    }
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

std::string hex32(uint32_t v) {
    char buf[16];
    std::snprintf(buf, sizeof buf, "0x%08X", v);
    return buf;
}
std::string hex16(uint16_t v) {
    char buf[8];
    std::snprintf(buf, sizeof buf, "0x%04X", v);
    return buf;
}

std::string lower(std::string_view s) {
    std::string out(s);
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}
std::string extension(std::string_view name) {
    const size_t dot = name.rfind('.');
    const size_t slash = name.find_last_of("/\\");
    if (dot == std::string_view::npos || (slash != std::string_view::npos && dot < slash)) return "";
    return lower(name.substr(dot + 1));
}

void field(Identity& id, std::string key, std::string value) { id.header.push_back({std::move(key), std::move(value)}); }
void find(Identity& id, std::string claim, std::string evidence, std::string confidence) {
    id.findings.push_back({std::move(claim), std::move(evidence), std::move(confidence)});
}

/* ── consoles ─────────────────────────────────────────────────────────────── */

/* Nintendo DS (GBATEK, "DS Cartridge Header"). The header CRC at 0x15E covers
 * 0x000–0x15D, and the logo CRC at 0x15C is the constant 0xCF56 on every licensed
 * cartridge. Recomputing the first is what makes this `verified`. */
bool try_nds(Identity& id, std::span<const uint8_t> h, const std::string& ext) {
    const bool by_name = ext == "nds" || ext == "dsi" || ext == "srl";
    bool crc_ok = false;
    if (has_bytes(h, 0, 0x160)) crc_ok = crc16_modbus(h.subspan(0, 0x15E)) == le16(h, 0x15E);
    if (!crc_ok && !(by_name && has_bytes(h, 0, 0x60))) return false;

    id.kind = "rom";
    id.platform = "nds";
    id.title = text(h, 0x000, 12);
    field(id, "title", id.title);
    field(id, "game_code", text(h, 0x00C, 4));
    field(id, "maker_code", text(h, 0x010, 2));
    const uint8_t unit = has_bytes(h, 0x12, 1) ? h[0x12] : 0;
    field(id, "unit", unit == 0 ? "NDS" : unit == 2 ? "NDS+DSi" : unit == 3 ? "DSi" : "unknown (" + std::to_string(unit) + ")");
    if (has_bytes(h, 0x14, 1) && h[0x14] < 16) field(id, "capacity_bytes", std::to_string(131072ull << h[0x14]));
    if (has_bytes(h, 0x1E, 1)) field(id, "rom_version", std::to_string(h[0x1E]));
    field(id, "arm9_bytes", std::to_string(le32(h, 0x02C)));
    field(id, "arm7_bytes", std::to_string(le32(h, 0x03C)));
    field(id, "files", std::to_string(le32(h, 0x04C) / 8));            // FAT: 8 bytes per file
    field(id, "arm9_overlays", std::to_string(le32(h, 0x054) / 32));   // OVT: 32 bytes per overlay
    field(id, "arm7_overlays", std::to_string(le32(h, 0x05C) / 32));
    if (has_bytes(h, 0x80, 4)) field(id, "used_rom_bytes", std::to_string(le32(h, 0x080)));

    if (crc_ok) {
        find(id, "a Nintendo DS cartridge image", "header CRC-16 at 0x15E recomputed over 0x000-0x15D: " + hex16(le16(h, 0x15E)), "verified");
    } else {
        find(id, "a Nintendo DS cartridge image, by name only", "extension ." + ext, "weak");
        id.residual.push_back(has_bytes(h, 0, 0x160) ? "DS header CRC does not match; the header may be modified or this is not a DS image"
                                                      : "fewer than 0x160 bytes supplied; DS header CRC not checked");
    }
    if (has_bytes(h, 0x15C, 2)) {
        if (le16(h, 0x15C) == 0xCF56) find(id, "the Nintendo logo CRC is the licensed constant", "0xCF56 at 0x15C", "strong");
        else id.residual.push_back("logo CRC at 0x15C is " + hex16(le16(h, 0x15C)) + ", not 0xCF56: homebrew or modified");
    }
    id.residual.push_back("NitroFS names (FNT) and overlay contents not read: header only");
    return true;
}

/* N64: the first word says the byte order of the dump. Normalised to big-endian
 * (.z64) before any field is read, so one set of offsets serves all three. */
bool try_n64(Identity& id, std::span<const uint8_t> h, const std::string& ext) {
    if (!has_bytes(h, 0, 0x40)) return false;
    const uint32_t w = be32(h, 0);
    std::string order;
    if (w == 0x80371240) order = "big-endian (.z64)";
    else if (w == 0x37804012) order = "byte-swapped (.v64)";
    else if (w == 0x40123780) order = "little-endian (.n64)";
    else return false;

    std::vector<uint8_t> n(h.begin(), h.begin() + 0x40);
    if (w == 0x37804012) for (size_t i = 0; i + 1 < n.size(); i += 2) std::swap(n[i], n[i + 1]);
    if (w == 0x40123780) for (size_t i = 0; i + 3 < n.size(); i += 4) { std::swap(n[i], n[i + 3]); std::swap(n[i + 1], n[i + 2]); }
    const std::span<const uint8_t> z(n);

    id.kind = "rom";
    id.platform = "n64";
    bool non_ascii = false;
    id.title = text(z, 0x20, 20, &non_ascii);
    field(id, "title", id.title);
    field(id, "game_code", text(z, 0x3B, 4));
    field(id, "version", std::to_string(z[0x3F]));
    field(id, "byte_order", order);
    field(id, "entry_point", hex32(be32(z, 0x08)));
    find(id, "a Nintendo 64 cartridge image", "PI header word " + hex32(w) + " at 0x00", "strong");
    if (non_ascii) id.residual.push_back("title has non-ASCII bytes (likely Shift-JIS); shown as '?'");
    id.residual.push_back("boot-code checksum not verified (it covers 1 MiB; recon reads the header only)");
    (void)ext;
    return true;
}

/* GBA: fixed 0x96 at 0xB2 and a complement byte at 0xBD over 0xA0–0xBC. */
bool try_gba(Identity& id, std::span<const uint8_t> h, const std::string& ext) {
    if (!has_bytes(h, 0, 0xC0) || h[0xB2] != 0x96) return false;
    uint8_t chk = 0;
    for (size_t i = 0xA0; i <= 0xBC; ++i) chk = static_cast<uint8_t>(chk - h[i]);
    chk = static_cast<uint8_t>(chk - 0x19);
    const bool ok = chk == h[0xBD];
    if (!ok && ext != "gba") return false;

    id.kind = "rom";
    id.platform = "gba";
    id.title = text(h, 0xA0, 12);
    field(id, "title", id.title);
    field(id, "game_code", text(h, 0xAC, 4));
    field(id, "maker_code", text(h, 0xB0, 2));
    field(id, "version", std::to_string(h[0xBC]));
    if (ok) find(id, "a Game Boy Advance cartridge image", "complement check at 0xBD recomputed over 0xA0-0xBC", "verified");
    else {
        find(id, "a Game Boy Advance cartridge image, by name and the fixed 0x96", "extension .gba, 0x96 at 0xB2", "weak");
        id.residual.push_back("GBA complement check at 0xBD does not match");
    }
    return true;
}

/* Game Boy / Color: the logo at 0x104 and the header checksum at 0x14D. */
bool try_gb(Identity& id, std::span<const uint8_t> h, const std::string& ext) {
    if (!has_bytes(h, 0, 0x150) || !(h[0x104] == 0xCE && h[0x105] == 0xED && h[0x106] == 0x66 && h[0x107] == 0x66)) return false;
    uint8_t x = 0;
    for (size_t i = 0x134; i <= 0x14C; ++i) x = static_cast<uint8_t>(x - h[i] - 1);
    const bool ok = x == h[0x14D];
    const bool cgb = (h[0x143] & 0x80) != 0;

    id.kind = "rom";
    id.platform = "gb";
    id.title = text(h, 0x134, cgb ? 15 : 16);
    field(id, "title", id.title);
    field(id, "color", (h[0x143] == 0xC0) ? "Game Boy Color only" : cgb ? "Game Boy Color enhanced" : "no");
    field(id, "cartridge_type", hex16(h[0x147]));
    if (h[0x148] < 9) field(id, "rom_bytes", std::to_string(32768ull << h[0x148]));
    if (ok) find(id, "a Game Boy cartridge image", "header checksum at 0x14D recomputed over 0x134-0x14C", "verified");
    else {
        find(id, "a Game Boy cartridge image, by its logo", "logo bytes at 0x104", "strong");
        id.residual.push_back("Game Boy header checksum at 0x14D does not match");
    }
    (void)ext;
    return true;
}

/* GameCube and Wii discs: a magic word in the disc header; decomp.dev's home ground. */
bool try_disc(Identity& id, std::span<const uint8_t> h, const std::string& ext) {
    if (magic(h, 0, "RVZ\x01") || magic(h, 0, "WIA\x01")) {
        id.kind = "disc";
        id.platform = "gamecube-or-wii";
        find(id, "a compressed GameCube or Wii disc image", magic(h, 0, "RVZ\x01") ? "RVZ magic" : "WIA magic", "strong");
        id.residual.push_back("compressed image: the disc header inside was not read (Dolphin can convert to ISO)");
        return true;
    }
    if (!has_bytes(h, 0, 0x60)) return false;
    const bool gc = be32(h, 0x1C) == 0xC2339F3D;
    const bool wii = be32(h, 0x18) == 0x5D1C9EA3;
    if (!gc && !wii) return false;
    id.kind = "disc";
    id.platform = wii ? "wii" : "gamecube";
    bool non_ascii = false;
    id.title = text(h, 0x20, 64, &non_ascii);
    field(id, "title", id.title);
    field(id, "game_id", text(h, 0x00, 6));
    field(id, "disc", std::to_string(h[0x06]));
    field(id, "version", std::to_string(h[0x07]));
    find(id, wii ? "a Wii disc image" : "a GameCube disc image",
         wii ? "Wii magic 0x5D1C9EA3 at 0x18" : "GameCube magic 0xC2339F3D at 0x1C", "strong");
    if (non_ascii) id.residual.push_back("title has non-ASCII bytes (likely Shift-JIS); shown as '?'");
    if (wii) id.residual.push_back("Wii partitions are encrypted; recon reads the plain disc header only");
    (void)ext;
    return true;
}

/* ── containers and executables ───────────────────────────────────────────── */

bool try_container(Identity& id, std::span<const uint8_t> h, const std::string& ext) {
    if (magic(h, 0, "GDPC")) {
        id.kind = "archive";
        id.platform = "pc";
        id.engine = "godot";
        const std::string v = std::to_string(le32(h, 8)) + "." + std::to_string(le32(h, 12)) + "." + std::to_string(le32(h, 16));
        field(id, "godot_version", v);
        field(id, "pack_format", std::to_string(le32(h, 4)));
        find(id, "a Godot resource pack", "GDPC magic; engine " + v, "strong");
        return true;
    }
    if (magic(h, 0, "FORM") && magic(h, 8, "GEN8")) {
        id.kind = "archive";
        id.platform = "pc";
        id.engine = "gamemaker";
        if (has_bytes(h, 17, 1)) field(id, "bytecode_version", std::to_string(h[17]));
        find(id, "a GameMaker data file", "FORM container with a GEN8 chunk", "strong");
        return true;
    }
    if (magic(h, 0, "UnityFS")) {
        id.kind = "archive";
        id.platform = "pc";
        id.engine = "unity";
        find(id, "a Unity asset bundle", "UnityFS magic", "strong");
        return true;
    }
    if (magic(h, 0, "RPA-")) {
        id.kind = "archive";
        id.platform = "pc";
        id.engine = "renpy";
        field(id, "rpa", text(h, 0, 7));
        find(id, "a Ren'Py archive", "RPA magic", "strong");
        return true;
    }
    if (magic(h, 0, "PK\x03\x04")) {
        id.kind = "archive";
        if (ext == "jar") { id.platform = "java"; find(id, "a Java archive", "ZIP magic, extension .jar", "strong"); }
        else if (ext == "apk") { id.platform = "android"; find(id, "an Android package", "ZIP magic, extension .apk", "strong"); }
        else if (ext == "love") { id.platform = "pc"; id.engine = "love2d"; find(id, "a LÖVE game", "ZIP magic, extension .love", "strong"); }
        else find(id, "a ZIP archive", "ZIP magic", "strong");
        id.residual.push_back("archive members not listed: header only");
        return true;
    }
    return false;
}

bool try_executable(Identity& id, std::span<const uint8_t> h) {
    if (magic(h, 0, "MZ")) {
        id.kind = "executable";
        id.platform = "pc";
        const uint32_t pe = le32(h, 0x3C);
        if (has_bytes(h, pe, 6) && magic(h, pe, std::string_view("PE\0\0", 4))) {
            const uint16_t m = le16(h, pe + 4);
            const char* arch = m == 0x14C ? "x86" : m == 0x8664 ? "x86-64" : m == 0xAA64 ? "ARM64" : "other";
            field(id, "machine", std::string(arch) + " (" + hex16(m) + ")");
            find(id, "a Windows PE executable", "MZ and PE signatures", "strong");
        } else {
            find(id, "a DOS/Windows executable", "MZ signature; PE header beyond the bytes supplied", "weak");
        }
        id.residual.push_back("managed (.NET) or native not determined: the CLR header was not read");
        id.residual.push_back("for a game install, run recon on the folder, and `um scan` for engine version, anti-cheat and loaders");
        return true;
    }
    if (magic(h, 0, "\x7F" "ELF") && has_bytes(h, 0, 0x14)) {
        id.kind = "executable";
        const bool big = h[5] == 2;
        const uint16_t m = big ? static_cast<uint16_t>((h[0x12] << 8) | h[0x13]) : le16(h, 0x12);
        const char* arch = m == 0x03 ? "x86" : m == 0x08 ? "MIPS" : m == 0x14 ? "PowerPC" : m == 0x15 ? "PowerPC64"
                         : m == 0x28 ? "ARM" : m == 0x3E ? "x86-64" : m == 0xB7 ? "AArch64" : "other";
        field(id, "class", h[4] == 2 ? "64-bit" : "32-bit");
        field(id, "byte_order", big ? "big-endian" : "little-endian");
        field(id, "machine", arch);
        find(id, "an ELF executable", std::string("ELF magic; ") + arch, "strong");
        id.residual.push_back("platform not determined: MIPS ELFs are PS2/PSP, PowerPC64 big-endian is PS3, x86-64 is usually Linux");
        return true;
    }
    if (be32(h, 0) == 0xCAFEBABE) {
        id.kind = "executable";
        id.platform = "java";
        find(id, "a Java class file", "0xCAFEBABE", "strong");
        return true;
    }
    return false;
}

/* ── installs ─────────────────────────────────────────────────────────────── */

/* '*' matches any run of characters, slashes included. Patterns are lower case. */
bool glob(std::string_view p, std::string_view s) {
    size_t pi = 0, si = 0, star = std::string_view::npos, mark = 0;
    while (si < s.size()) {
        if (pi < p.size() && p[pi] == '*') { star = pi++; mark = si; }
        else if (pi < p.size() && p[pi] == s[si]) { ++pi; ++si; }
        else if (star != std::string_view::npos) { pi = star + 1; si = ++mark; }
        else return false;
    }
    while (pi < p.size() && p[pi] == '*') ++pi;
    return pi == p.size();
}

struct Listing {
    std::vector<std::string> paths;   // lower case, '/'-separated
    std::vector<std::string> original;
    std::vector<std::string> match(std::string_view pattern, size_t limit = 3) const {
        std::vector<std::string> out;
        for (size_t i = 0; i < paths.size() && out.size() < limit; ++i)
            if (glob(pattern, paths[i])) out.push_back(original[i]);
        return out;
    }
    bool has(std::string_view pattern) const { return !match(pattern, 1).empty(); }
    /* Only paths at most `max_slashes` folders down. A game's own .jar or .exe sits at
     * the top; one deep in the tree is a tool's (a Gradle wrapper, a crash handler). */
    std::vector<std::string> shallow(std::string_view pattern, size_t max_slashes, size_t limit) const {
        std::vector<std::string> out;
        for (size_t i = 0; i < paths.size() && out.size() < limit; ++i)
            if (static_cast<size_t>(std::count(paths[i].begin(), paths[i].end(), '/')) <= max_slashes && glob(pattern, paths[i]))
                out.push_back(original[i]);
        return out;
    }
};

std::string stem(const std::string& path) {
    const size_t slash = path.find_last_of('/');
    std::string s = slash == std::string::npos ? path : path.substr(slash + 1);
    const size_t dot = s.rfind('.');
    return dot == std::string::npos ? s : s.substr(0, dot);
}

std::string join(const std::vector<std::string>& v) {
    std::string s;
    for (const std::string& x : v) s += (s.empty() ? "" : ", ") + x;
    return s;
}

}  // namespace

const char* depth_name(Depth d) {
    switch (d) {
        case Depth::recon: return "recon";
        case Depth::bytes: return "bytes";
        case Depth::opaque: return "opaque";
        case Depth::source: return "source";
        case Depth::structure: return "structure";
    }
    return "?";
}

uint16_t crc16_modbus(std::span<const uint8_t> bytes) {
    uint16_t crc = 0xFFFF;
    for (const uint8_t b : bytes) {
        crc ^= b;
        for (int k = 0; k < 8; ++k) crc = (crc & 1) ? static_cast<uint16_t>((crc >> 1) ^ 0xA001) : static_cast<uint16_t>(crc >> 1);
    }
    return crc;
}

Identity identify_file(std::string_view name, std::span<const uint8_t> head, uint64_t file_size) {
    Identity id;
    const std::string ext = extension(name);
    field(id, "file_bytes", std::to_string(file_size));
    /* Order matters only where signatures could overlap: checksummed console headers
     * first, because a passing checksum is the strongest evidence recon can produce. */
    if (try_nds(id, head, ext) || try_gba(id, head, ext) || try_gb(id, head, ext) || try_n64(id, head, ext) ||
        try_disc(id, head, ext) || try_container(id, head, ext) || try_executable(id, head)) {
        return id;
    }
    id.residual.push_back("no known header in the first " + std::to_string(head.size()) + " bytes" +
                          (ext.empty() ? "" : " (extension ." + ext + ")"));
    return id;
}

Identity identify_install(const std::vector<Entry>& entries, bool truncated) {
    Identity id;
    id.kind = "install";
    id.platform = "pc";
    Listing L;
    for (const Entry& e : entries) {
        std::string p = e.path;
        std::replace(p.begin(), p.end(), '\\', '/');
        L.original.push_back(p);
        L.paths.push_back(lower(p));
    }
    field(id, "entries", std::to_string(entries.size()) + (truncated ? " (truncated)" : ""));

    struct Hit { std::string engine; int score; std::string evidence; std::string confidence; std::string title; };
    std::vector<Hit> hits;

    // Unity: the player DLL or the data folder; IL2CPP leaves GameAssembly and its metadata.
    if (L.has("*unityplayer.dll") || L.has("*_data/globalgamemanagers") || L.has("*_data/data.unity3d")) {
        /* "MyGame_Data/globalgamemanagers" names the game: the folder, less "_Data". */
        std::string title;
        if (const auto data = L.match("*_data/globalgamemanagers", 1); !data.empty()) {
            const std::string dir = stem(data[0].substr(0, data[0].rfind('/')) + ".x");
            title = dir.size() > 5 ? dir.substr(0, dir.size() - 5) : dir;
        }
        if (L.has("*gameassembly.dll") || L.has("*il2cpp_data/metadata/global-metadata.dat"))
            hits.push_back({"unity-il2cpp", 100, "UnityPlayer + GameAssembly / global-metadata.dat", "strong", title});
        else if (L.has("*_data/managed/assembly-csharp.dll"))
            hits.push_back({"unity-mono", 100, "UnityPlayer + Managed/Assembly-CSharp.dll", "strong", title});
        else
            hits.push_back({"unity-mono", 70, "UnityPlayer, no scripting backend found", "weak", title});
    }
    if (const auto s = L.match("*-win64-shipping.exe", 1); !s.empty()) {
        std::string t = stem(s[0]);
        t = t.substr(0, t.size() - std::string("-Win64-Shipping").size());
        hits.push_back({"unreal", 100, s[0], "strong", t});
    } else if (const auto p = L.match("*content/paks/*.pak", 1); !p.empty()) {
        hits.push_back({"unreal", 70, p[0], "weak", ""});
    }
    if (const auto p = L.match("*.pck", 1); !p.empty()) hits.push_back({"godot", 60, p[0] + " (name only)", "weak", stem(p[0])});
    if (L.has("data.win") || L.has("game.unx")) hits.push_back({"gamemaker", 90, L.has("data.win") ? "data.win" : "game.unx", "strong", ""});
    if (L.has("renpy/*") && L.has("game/*")) hits.push_back({"renpy", 100, "renpy/ + game/", "strong", ""});
    if (L.has("www/js/rpg_core.js") || L.has("js/rpg_core.js")) hits.push_back({"rpgmaker-mv", 100, "rpg_core.js", "strong", ""});
    if (L.has("js/rmmz_core.js") || L.has("www/js/rmmz_core.js")) hits.push_back({"rpgmaker-mz", 100, "rmmz_core.js", "strong", ""});
    if (const auto g = L.match("*/gameinfo.gi", 1); !g.empty()) hits.push_back({"source2", 100, g[0], "strong", ""});
    else if (const auto t = L.match("*/gameinfo.txt", 1); !t.empty() && (L.has("*_dir.vpk") || L.has("bin/engine.dll")))
        hits.push_back({"source", 100, t[0] + " + VPK/engine.dll", "strong", ""});
    if (L.has("*content/*.xnb") && L.has("*.exe")) hits.push_back({"dotnet-xna", 80, join(L.match("*content/*.xnb", 1)), "strong", ""});
    if (const auto l = L.match("*.love", 1); !l.empty()) hits.push_back({"love2d", 70, l[0], "weak", stem(l[0])});
    if (const auto j = L.shallow("*.jar", 1, 1); !j.empty()) hits.push_back({"java", 40, j[0], "weak", ""});
    if (const auto e = L.shallow("*.exe", 1, 3); !e.empty()) hits.push_back({"native", 10, join(e), "weak", stem(e[0])});

    std::stable_sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b) { return a.score > b.score; });
    if (hits.empty()) {
        id.residual.push_back("no engine signature and no executable in the listing");
    } else {
        const Hit& top = hits.front();
        id.engine = top.engine;
        if (top.engine == "java") id.platform = "java";
        for (const Hit& h : hits) if (!h.title.empty()) { id.title = h.title; break; }
        find(id, "engine: " + top.engine, top.evidence, top.confidence);
        for (size_t i = 1; i < hits.size(); ++i)
            if (hits[i].engine != "native" && hits[i].engine != top.engine) id.also.push_back(hits[i].engine + " (" + hits[i].evidence + ")");
        if (top.engine == "native") id.residual.push_back("no engine signature: a native executable, read with Ghidra or REA");
    }
    if (truncated) id.residual.push_back("listing truncated: signals past the cut were not seen");
    id.residual.push_back("signature files were matched by name; their headers were not read (run `effect recon` on one to check it)");
    id.residual.push_back("engine version, anti-cheat, loaders and save folders not checked: `um scan` does (universal-modder)");
    return id;
}

std::vector<Route> routes_for(const Identity& id) {
    std::vector<Route> r;
    auto add = [&](Depth d, std::string tool, std::string exe, std::string does) { r.push_back({d, std::move(tool), std::move(exe), std::move(does)}); };
    const std::string& p = id.platform;
    const std::string& e = id.engine;

    if (p == "nds") {
        add(Depth::bytes, "ndstool", "ndstool", "unpack NitroFS, the ARM9/ARM7 binaries, overlays and banner");
        add(Depth::opaque, "melonDS", "melonDS", "run the original; memory and input are the contract");
        add(Depth::source, "Ghidra (headless)", "analyzeHeadless", "disassemble and decompile ARM9, ARM7 and overlays");
        add(Depth::source, "objdiff", "objdiff-cli", "per-function match of recompiled code against the original");
    } else if (p == "n64") {
        add(Depth::bytes, "splat", "", "split the ROM into assembly, data and assets (a Python module)");
        add(Depth::opaque, "N64Recomp", "N64Recomp", "statically recompile to C against N64ModernRuntime; runs natively");
        add(Depth::opaque, "ares", "ares", "run the original under an emulator");
        add(Depth::source, "m2c", "", "first-pass C from assembly (a Python module)");
        add(Depth::source, "objdiff", "objdiff-cli", "per-function match against the original: the matching-decompilation oracle");
        add(Depth::source, "decomp-permuter", "", "search for code shapes that match (a Python tool)");
    } else if (p == "gba" || p == "gb") {
        add(Depth::opaque, "mGBA", "mgba", "run the original; mGBA's scripting reads and writes memory");
        if (p == "gb") add(Depth::source, "mgbdis", "", "disassemble to RGBDS assembly (a Python tool)");
        if (p == "gb") add(Depth::source, "RGBDS", "rgbasm", "reassemble; a byte-identical rebuild is the lens");
        if (p == "gba") add(Depth::source, "Ghidra (headless)", "analyzeHeadless", "disassemble and decompile ARM/Thumb");
        if (p == "gba") add(Depth::source, "objdiff", "objdiff-cli", "per-function match against the original");
    } else if (p == "gamecube" || p == "wii" || p == "gamecube-or-wii") {
        add(Depth::bytes, "Dolphin", "Dolphin", "extract the disc's file system; convert RVZ/WIA to ISO");
        add(Depth::opaque, "Dolphin", "Dolphin", "run the original under an emulator");
        add(Depth::source, "decomp-toolkit", "dtk", "split the DOL and RELs into relocatable objects and generate the link");
        add(Depth::source, "objdiff", "objdiff-cli", "per-function match against the original: decomp.dev's measure");
    } else if (p == "pc" || p == "java" || p == "android") {
        if (p == "pc") add(Depth::recon, "universal-modder", "um", "full fingerprint: engine version, anti-cheat, loaders, saves, community route");
        if (e == "unity-mono") {
            add(Depth::bytes, "AssetRipper", "", "export scenes, meshes, textures and audio (a GUI)");
            add(Depth::opaque, "BepInEx", "", "a loader in the game folder; the original runs and plugins are the contract");
            add(Depth::source, "ILSpy", "ilspycmd", "decompile Assembly-CSharp.dll to C#");
        } else if (e == "unity-il2cpp") {
            add(Depth::bytes, "AssetRipper", "", "export scenes, meshes, textures and audio (a GUI)");
            add(Depth::source, "Cpp2IL", "Cpp2IL", "recover types and method signatures from GameAssembly + metadata");
            add(Depth::source, "Il2CppDumper", "Il2CppDumper", "dump types and addresses for Ghidra or IDA");
        } else if (e == "unreal") {
            add(Depth::bytes, "repak", "repak", "list and unpack .pak archives");
            add(Depth::opaque, "UE4SS", "", "a loader in the game folder; Lua and C++ hooks are the contract");
        } else if (e == "godot") {
            add(Depth::source, "GDRE Tools", "gdre_tools", "recover the Godot project: scenes, scripts, resources");
        } else if (e == "gamemaker") {
            add(Depth::source, "UndertaleModTool", "UndertaleModCli", "decompile GML; sprites and rooms from data.win");
        } else if (p == "java") {
            add(Depth::source, "Vineflower", "", "decompile Java bytecode (a JAR run with java)");
        }
        add(Depth::source, "Ghidra (headless)", "analyzeHeadless", "disassemble and decompile native code");
        add(Depth::source, "REA", "", "agent-driven reverse engineering over Ghidra, JADX and others (npx rea-agents)");
    }
    add(Depth::structure, "Void Squid reader + Void Bicho", "", "lift pieces into the game model's nouns, as proposals: NOT BUILT");
    return r;
}

namespace {
std::string q(std::string_view s) {
    std::string out = "\"";
    for (const char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\u%04x", c); out += b; }
                else out += c;
        }
    }
    return out + "\"";
}
}  // namespace

std::string recon_json(const Identity& id, const std::vector<Route>& routes) {
    std::string j = "{\"kind\":" + q(id.kind) + ",\"platform\":" + q(id.platform);
    j += ",\"engine\":" + (id.engine.empty() ? std::string("null") : q(id.engine));
    j += ",\"title\":" + (id.title.empty() ? std::string("null") : q(id.title));
    j += ",\"header\":{";
    for (size_t i = 0; i < id.header.size(); ++i) j += (i ? "," : "") + q(id.header[i].key) + ":" + q(id.header[i].value);
    j += "},\"findings\":[";
    for (size_t i = 0; i < id.findings.size(); ++i) {
        const Finding& f = id.findings[i];
        j += std::string(i ? "," : "") + "{\"claim\":" + q(f.claim) + ",\"evidence\":" + q(f.evidence) + ",\"confidence\":" + q(f.confidence) + "}";
    }
    j += "],\"also\":[";
    for (size_t i = 0; i < id.also.size(); ++i) j += (i ? "," : "") + q(id.also[i]);
    j += "],\"routes\":[";
    for (size_t i = 0; i < routes.size(); ++i) {
        const Route& r = routes[i];
        j += std::string(i ? "," : "") + "{\"depth\":" + q(depth_name(r.depth)) + ",\"tool\":" + q(r.tool) +
             ",\"exe\":" + (r.exe.empty() ? std::string("null") : q(r.exe)) + ",\"does\":" + q(r.does) +
             ",\"found\":" + (r.found < 0 ? "null" : r.found ? "true" : "false") + "}";
    }
    j += "],\"residual\":[";
    for (size_t i = 0; i < id.residual.size(); ++i) j += (i ? "," : "") + q(id.residual[i]);
    return j + "]}";
}

}  // namespace squid::read
