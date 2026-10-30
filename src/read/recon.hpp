/* recon.hpp — the first stage of the ingress arrow: what is this artifact?
 *
 * okf/concepts/recon.md; okf/design/decompilation.md §6.
 *
 * Construct and deconstruct (okf/concepts/construct-and-deconstruct.md): a READER is
 * the inverse of a nib. Before any reader can run, something has to say which reader,
 * and that is recon: given the first bytes of a file, or the listing of an install
 * folder, say what kind of artifact it is, which platform and engine it belongs to,
 * what its header claims, which tools reach which DEPTH of recovery, and what recon
 * itself could not determine.
 *
 * ── Pure, like squid_sim ──
 *
 * This target links nothing but the standard library. It never opens a file, walks a
 * directory, reads PATH or spawns a process: the host does those (app.cpp, `effect
 * recon`) and hands the bytes and the listing in. So recon runs in a unit test with no
 * filesystem (CLAUDE.md rule 5), and the same function will one day run on the LITO
 * host over a capability instead of a path.
 *
 * ── Honesty ──
 *
 * Every conclusion carries its evidence and a confidence, after REA's "evidence and
 * limitations behind each conclusion" and universal-modder's evidence levels:
 *   verified  a checksum the format defines was recomputed and agreed;
 *   strong    a magic number or a signature file matched;
 *   weak      a file name or extension suggested it, nothing more.
 * What recon could not determine goes in `residual`, never in silence.
 *
 * ── What it is NOT ──
 *
 * Not a full fingerprinter for PC installs. universal-modder's `um scan` knows engine
 * versions, anti-cheat, loaders, save folders and per-game routes for hundreds of
 * games, and recon names it as the deeper tool instead of re-implementing it
 * (okf/sources/universal-modder.md). What recon adds that `um` does not have is the
 * consoles: Nintendo DS, N64, GBA, Game Boy, GameCube and Wii headers, read and
 * checked — the platforms the family's own nibs target and decomp.dev tracks.
 */
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace squid::read {

/* How far a piece of a game has been recovered (okf/design/decompilation.md §3).
 * Recon itself is the zeroth step; the four after it are the depth ladder. */
enum class Depth {
    recon,      // knowing what the artifact is
    bytes,      // its assets and binaries extracted and listed
    opaque,     // its code runs as the original, behind a contract (emulator, recompilation, loader)
    source,     // its code read as source (decompiled; matching where the community has done it)
    structure,  // its pieces lifted into the game model's nouns
};
const char* depth_name(Depth d);

struct Field {
    std::string key, value;
};

struct Finding {
    std::string claim;
    std::string evidence;
    std::string confidence;   // "verified" | "strong" | "weak"
};

struct Identity {
    std::string kind = "unknown";      // rom | disc | install | archive | executable | unknown
    std::string platform = "unknown";  // nds | n64 | gba | gb | gamecube | wii | pc | java | android | unknown
    std::string engine;                // "" for consoles; unity-mono, unity-il2cpp, unreal, godot, gamemaker, ...
    std::string title;
    std::vector<Field> header;         // what the header claims, in the header's own terms
    std::vector<Finding> findings;
    std::vector<std::string> also;     // other engines whose signals were present (launchers, helpers)
    std::vector<std::string> residual; // what recon could not determine
};

/* One file, by its name and its first bytes. `head` should be at least 0x200 bytes
 * when the file is that large (the DS header CRC sits at 0x15E); fewer bytes means
 * fewer checks, and the residual says which. */
Identity identify_file(std::string_view name, std::span<const uint8_t> head, uint64_t file_size);

/* One install folder, by its listing. Paths are relative to the folder, '/'-separated,
 * any case. `truncated` says the host stopped walking before the end. */
struct Entry {
    std::string path;
    uint64_t size = 0;
};
Identity identify_install(const std::vector<Entry>& listing, bool truncated);

/* Which tools reach which depth for this identity. `exe` is the name the host looks
 * for on PATH; empty when the tool is not discoverable that way (a Python module, a
 * GUI, a loader that lives in the game folder), and then `found` stays unknown. The
 * structure depth is always listed, and always unbuilt, because saying so is the
 * point (CLAUDE.md rule 11). */
struct Route {
    Depth depth;
    std::string tool;
    std::string exe;
    std::string does;
    int found = -1;   // -1 unknown (not discoverable), 0 not on PATH, 1 on PATH
};
std::vector<Route> routes_for(const Identity& id);

/* The report, bounded: a handful of header fields, findings with evidence, routes,
 * residual. It does not grow with the size of the artifact (okf/design/smart-logs.md). */
std::string recon_json(const Identity& id, const std::vector<Route>& routes);

/* CRC-16/MODBUS (reflected 0xA001, init 0xFFFF), which is what the DS header uses.
 * Exposed so a test can check it against the standard check value. */
uint16_t crc16_modbus(std::span<const uint8_t> bytes);

}  // namespace squid::read
