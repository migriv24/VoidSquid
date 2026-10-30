/* app.cpp — the application, and the one file read in Void Squid v0. */
#include "app.hpp"

#include "camera.hpp"
#include "effects.hpp"
#include "forge/effect.hpp"
#include "library/library.hpp"
#include "molerat/effect.hpp"
#include "presets.hpp"
#include "read/recon.hpp"
#include "table.hpp"
#include "voidjelly/verbs.hpp"
#include "voidjelly/readers/gltf.hpp"
#include "voidjelly/store.hpp"

#include "json.hpp"

#include <cctype>
#include <map>
#include <optional>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string_view>

namespace squid {
namespace {

/* `{"args":["open","C:\\scenes\\room.ink"]}` -> {"open", "C:\scenes\room.ink"}.
 *
 * The effect verb hands its arguments to the handler as a JSON array of strings
 * (SPEC §9). This reads exactly that shape and nothing more: Void Squid vendors no
 * JSON library yet, and a general parser for one known shape is how a host grows a
 * second copy of something a sibling already owns. If a third effect needs
 * structured arguments, vendor cJSON like the rest of the family. */
std::vector<std::string> effect_args(std::string_view json) {
    std::vector<std::string> out;
    const size_t key = json.find("\"args\"");
    if (key == std::string_view::npos) return out;
    size_t i = json.find('[', key);
    if (i == std::string_view::npos) return out;
    ++i;
    while (i < json.size()) {
        while (i < json.size() && (json[i] == ' ' || json[i] == ',')) ++i;
        if (i >= json.size() || json[i] == ']') break;
        if (json[i] != '"') return out;   // not a string array; give up honestly
        ++i;
        std::string s;
        while (i < json.size() && json[i] != '"') {
            if (json[i] == '\\' && i + 1 < json.size()) {
                ++i;
                switch (json[i]) {
                    case 'n': s += '\n'; break;
                    case 't': s += '\t'; break;
                    case 'r': s += '\r'; break;
                    case 'b': s += '\b'; break;
                    case 'f': s += '\f'; break;
                    case 'u': {
                        /* \uXXXX: only the BMP, bytes written as UTF-8. Paths rarely
                         * carry these; correctness over completeness. */
                        if (i + 4 < json.size()) {
                            unsigned cp = 0;
                            for (int k = 1; k <= 4; ++k) {
                                const char c = json[i + k];
                                cp <<= 4;
                                cp |= (c >= '0' && c <= '9') ? c - '0'
                                    : (c >= 'a' && c <= 'f') ? c - 'a' + 10
                                    : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : 0;
                            }
                            if (cp < 0x80) s += static_cast<char>(cp);
                            else if (cp < 0x800) { s += static_cast<char>(0xC0 | (cp >> 6)); s += static_cast<char>(0x80 | (cp & 0x3F)); }
                            else { s += static_cast<char>(0xE0 | (cp >> 12)); s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); s += static_cast<char>(0x80 | (cp & 0x3F)); }
                            i += 4;
                        }
                        break;
                    }
                    default: s += json[i]; break;   // \" \\ \/
                }
            } else {
                s += json[i];
            }
            ++i;
        }
        ++i;   // closing quote
        out.push_back(std::move(s));
    }
    return out;
}

std::string quote(const std::string& s) {
    std::string out = "\"";
    for (const char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            default: out += c;
        }
    }
    return out + "\"";
}

/* Is `exe` on PATH? An effect (it reads the environment and the file system), so it
 * lives here and not in squid_read. Windows tries the usual executable suffixes. */
int on_path(const std::string& exe) {
    if (exe.empty()) return -1;
    const char* env = std::getenv("PATH");
    if (!env) return 0;
#ifdef _WIN32
    const char sep = ';';
    const char* suffixes[] = {"", ".exe", ".cmd", ".bat"};
#else
    const char sep = ':';
    const char* suffixes[] = {""};
#endif
    const std::string path(env);
    size_t start = 0;
    while (start <= path.size()) {
        const size_t end = path.find(sep, start);
        const std::string dir = path.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!dir.empty()) {
            for (const char* suf : suffixes) {
                std::error_code ec;
                if (std::filesystem::is_regular_file(std::filesystem::path(dir) / (exe + suf), ec)) return 1;
            }
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return 0;
}

}  // namespace

std::string recon(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path root(path);
    read::Identity id;

    if (fs::is_directory(root, ec)) {
        /* THE EFFECT, bounded: at most kReconEntries files, at most kReconDepth folders
         * deep. An install of 200,000 files gives the same size of answer as one of 200. */
        constexpr size_t kReconEntries = 20000;
        constexpr int kReconDepth = 8;
        std::vector<read::Entry> listing;
        bool truncated = false;
        fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
        while (!ec && it != end) {
            if (it.depth() >= kReconDepth) it.disable_recursion_pending();
            std::error_code fe;
            if (it->is_regular_file(fe)) {
                if (listing.size() >= kReconEntries) { truncated = true; break; }
                listing.push_back({fs::relative(it->path(), root, fe).generic_string(), it->file_size(fe)});
            }
            it.increment(ec);
        }
        if (ec) return "{\"error\":" + quote("cannot walk '" + path + "': " + ec.message()) + "}";
        id = read::identify_install(listing, truncated);
    } else if (fs::is_regular_file(root, ec)) {
        /* THE EFFECT: the first KiB of one file. Every header recon knows ends before it. */
        std::ifstream in(path, std::ios::binary);
        if (!in) return "{\"error\":" + quote("cannot read '" + path + "'") + "}";
        std::vector<uint8_t> head(1024);
        in.read(reinterpret_cast<char*>(head.data()), static_cast<std::streamsize>(head.size()));
        head.resize(static_cast<size_t>(in.gcount()));
        const uint64_t size = fs::file_size(root, ec);
        id = read::identify_file(root.filename().string(), head, ec ? 0 : size);
    } else {
        return "{\"error\":" + quote("no such file or folder: '" + path + "'") + "}";
    }

    std::vector<read::Route> routes = read::routes_for(id);
    for (read::Route& r : routes) r.found = on_path(r.exe);   // THE EFFECT: one PATH lookup per tool
    return read::recon_json(id, routes);
}

namespace {
std::optional<std::vector<uint8_t>> read_bytes(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return std::nullopt;
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}
bool is_gltf(const std::string& path) {
    std::string e = std::filesystem::path(path).extension().string();
    for (auto& c : e) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return e == ".glb" || e == ".gltf";
}
/* The host's half of Jelly's glTF reader: files the model names, beside the model. */
voidjelly::UriResolver beside(const std::filesystem::path& model) {
    const std::filesystem::path dir = model.parent_path();
    return [dir](const std::string& uri) { return read_bytes(dir / std::filesystem::u8path(uri)); };
}
void hold(Scene& scene, std::string path, voidjelly::Ink ink, std::vector<voidjelly::Loss> read_losses) {
    scene.path = std::move(path);
    scene.ink = std::move(ink);
    scene.bytes = voidjelly::write_ink(scene.ink);   // the verbs (inspect) read an .ink's bytes
    const std::string hash = voidjelly::hash_bytes(scene.bytes).hex();
    scene.gl = voidjelly::emit_gl(scene.ink, voidjelly::GlProfile::textured(), hash);
    for (auto& l : read_losses) scene.gl.residual.losses.insert(scene.gl.residual.losses.begin(), std::move(l));
    scene.loaded = true;
    ++scene.generation;
}
}  // namespace

bool open_ink(Scene& scene, const std::string& path) {
    scene.error.clear();

    /* THE EFFECT. Everything after this line is pure and Jelly's. */
    auto bytes = read_bytes(path);
    if (!bytes) {
        scene.error = "cannot read '" + path + "'";
        return false;
    }
    if (is_gltf(path)) {   // a recovered model: through Jelly's reader, its own files beside it
        voidjelly::GltfReadResult r = voidjelly::read_gltf(*bytes, beside(path), std::filesystem::path(path).stem().string());
        if (!r.ok) {
            scene.error = "Void Jelly could not read '" + path + "': " + r.error;
            return false;
        }
        hold(scene, path, std::move(r.ink), std::move(r.residual.losses));
        return true;
    }

    const voidjelly::ReadResult parsed = voidjelly::read_ink(*bytes);
    if (!parsed.ok) {
        scene.error = "Void Jelly refused '" + path + "': " + parsed.error;
        return false;
    }

    scene.path = path;
    scene.bytes = std::move(*bytes);
    scene.ink = parsed.ink;
    /* Hashed once here; emit_gl's hash-taking overload lets a later re-emit skip the
     * re-serialisation that was half its cost at 10 000 objects (log, 2026-10-09). The
     * textured profile since 2026-10-12: an .ink without materials draws as before. */
    scene.gl = voidjelly::emit_gl(scene.ink, voidjelly::GlProfile::textured(),
                                  voidjelly::canonical_hash(scene.ink).hex());
    scene.loaded = true;
    ++scene.generation;
    return true;
}

bool open_world(Scene& scene, const std::string& library, const std::string& world_file) {
    scene.error.clear();
    namespace fs = std::filesystem;
    const fs::path lib(library);
    auto wb = read_bytes(lib / world_file);
    if (!wb) { scene.error = "cannot read the world file '" + (lib / world_file).string() + "'"; return false; }
    nlohmann::json w;
    try { w = nlohmann::json::parse(wb->begin(), wb->end()); } catch (...) { scene.error = "the world file is not JSON"; return false; }
    if (!w.contains("objects") || !w["objects"].is_array()) { scene.error = "the world file has no objects"; return false; }
    voidjelly::Ink world;
    world.name = fs::path(world_file).stem().string();
    std::map<std::string, std::optional<voidjelly::Ink>> models;   // each model read once, placed many times
    std::vector<voidjelly::Loss> losses;
    size_t placed = 0, missing = 0, skies = 0;
    for (size_t i = 0; i < w["objects"].size(); ++i) {
        const auto& o = w["objects"][i];
        if (!o.contains("model") || !o["model"].is_string() || !o.contains("matrix")) continue;
        const std::string model = o["model"].get<std::string>();
        const std::string cls = o.contains("class") && o["class"].is_string() ? o["class"].get<std::string>() : "";
        if (cls.find("Sky") != std::string::npos || model.find("/VR") != std::string::npos) { ++skies; continue; }
        auto it = models.find(model);
        if (it == models.end()) {
            std::optional<voidjelly::Ink> ink;
            if (auto mb = read_bytes(lib / model)) {
                voidjelly::GltfReadResult r = voidjelly::read_gltf(*mb, beside(lib / model), fs::path(model).stem().string());
                if (r.ok) ink = std::move(r.ink);
            }
            it = models.emplace(model, std::move(ink)).first;
        }
        if (!it->second) { ++missing; continue; }
        voidjelly::Transform at;
        const auto& m = o["matrix"];
        for (size_t r = 0; r < 4 && r < m.size(); ++r)
            for (size_t c = 0; c < 4 && c < m[r].size(); ++c)
                at.m[r * 4 + c] = m[r][c].get<double>();
        voidjelly::append_ink(world, *it->second, at, std::to_string(i) + ":");
        ++placed;
    }
    if (skies)
        losses.push_back({"*", "omitted", "a sky dome encloses the whole level", std::to_string(skies) + " sky object(s) not drawn, so the level can be seen", 0, 0, false});
    if (missing)
        losses.push_back({"*", "dropped", "the model file could not be read", std::to_string(missing) + " object(s) whose model is missing or unreadable", 0, 0, false});
    losses.push_back({"*", "omitted", "a world file is the exercise's format, not an .ink",
                      "gravity, lights, cameras, zones and paths are not drawn; " + std::to_string(placed) + " models placed", 0, 0, false});
    hold(scene, (lib / world_file).string(), std::move(world), std::move(losses));
    return true;
}

std::string scene_summary_json(const Scene& scene) {
    if (!scene.loaded) return R"({"loaded":false})";
    const voidjelly::Residual& r = scene.gl.residual;
    std::string j = "{\"loaded\":true";
    j += ",\"path\":" + quote(scene.path);
    j += ",\"ink\":" + quote(scene.gl.drawlist.ink);
    j += ",\"name\":" + quote(scene.ink.name);
    j += ",\"objects\":" + std::to_string(scene.ink.objects.size());
    j += ",\"links\":" + std::to_string(scene.ink.links.size());
    j += ",\"polygons\":" + std::to_string(scene.ink.polygon_count());
    j += ",\"distinct_meshes\":" + std::to_string(scene.ink.distinct_mesh_count());
    j += ",\"draws\":" + std::to_string(scene.gl.drawlist.draws.size());
    j += ",\"triangles\":" + std::to_string(scene.gl.drawlist.indices.size() / 3);
    j += ",\"verdict\":" + quote(voidjelly::verdict_name(r.verdict));
    j += ",\"losses\":" + std::to_string(r.losses.size());
    j += ",\"dropped\":" + std::to_string(r.count("dropped"));
    j += "}";
    return j;
}

App build_app() {
    App app;
    app.scene = std::make_shared<Scene>();
    auto scene = app.scene;

    maiz::HostApp& h = app.host;
    h.id = "squid";
    h.label = "Void Squid";
    h.version = kVersion;
    h.okf_root = "okf";

    /* The studio's catalogue glyphs (turkey, game, kind, track, table, feast;
     * library/library.hpp), and nothing else. The .ink's glyphs are still Jelly's and
     * live in Jelly's own Core session, never re-registered here (railguard §4). These
     * describe the ASSET LIBRARY a project holds, not the game model, which S9 and S14
     * still keep design-only. */
    h.glyphs = library::glyphs();

    h.effect_ops = {
        {"world", "A table in 3D: `effect world <library-folder> <world-file>` reads every model the world "
                  "file places through Void Jelly's glTF reader, appends each at its matrix into one Ink, and "
                  "emits it with the textured GL profile. The world's gravity, lights and paths are named in the "
                  "residual, not drawn.",
         false, "reads the world file and the model files it names; writes nothing"},
        {"open", "Read one .ink, .glb or .gltf off disk, parse it through Void Jelly (a glTF through its "
                 "reader, with the files it names read from beside it), and emit it "
                 "through the GL nib. Then `config set squid.ink <path>` so the document "
                 "remembers what is open.",
         false, "reads one file from disk; writes nothing"},
        {"residual", "The GL nib's residual for the open scene, as JSON: what an "
                     "unconstrained target still could not carry as authored.",
         true, "none; computes in memory from the bytes `open` read"},
        {"inspect", "Void Jelly's `inspect` verb over the open scene: counts, the "
                    "instancing win, the tag census.",
         true, "none; computes in memory from the bytes `open` read"},
        {"survey", "The open scene summarised into a coarse grid: bounds, occupied "
                   "cells, a sample of ids and the top tags per cell. `effect survey [cells]`.",
         true, "none; computes in memory"},
        {"near", "Objects within a radius of a point, nearest first. "
                 "`effect near x y z radius [limit]`, scaled units.",
         true, "none; computes in memory"},
        {"make", "Build a preset scene from Void Jelly's primitives and write it as an "
                 ".ink: `effect make traffic scene.ink` (presets: traffic, shapes).",
         false, "writes one file to disk"},
        {"sim", "Model-rate simulation of the open scene: `sim new [instances] [seed] "
                "[rules.allo]` compiles the scene and an Allomone rule set into a World; "
                "`sim step [ticks] [dt]`, `sim fork <i> [seed]`, `sim log [i] [limit]`, "
                "`sim compare`. Deterministic; replies are bounded event summaries.",
         true, "none beyond reading an optional rules file; computes in memory"},
        {"recon", "What is this artifact? `effect recon <file-or-folder>`: a ROM or disc header "
                  "read and checked (DS, N64, GBA, Game Boy, GameCube, Wii), a container or "
                  "executable identified, or an install folder fingerprinted by its listing. "
                  "Answers with findings (each with evidence and a confidence), the tools that "
                  "reach each depth of recovery and whether they are on PATH, and a residual. "
                  "The first stage of decompilation (okf/concepts/recon.md).",
         false, "reads the first KiB of one file or a bounded folder listing, and PATH; writes nothing, runs nothing"},
        // molerat: Void MoleRat, the decompilation-and-re-expression harness.
        {"molerat", "Void MoleRat: a decompilation-and-re-expression harness over a normalized "
                    "foreign IR. `effect molerat gather <target> [--ir <path>] [--run <id>] "
                    "[--seed <s>]`, then `research`, `compose`, `residual` and `build`, with "
                    "`status`, `task`, `answer` and `verify` for the agent harness. The output "
                    "is a state document Void Squid can open, plus its research projection, "
                    "residual and provenance. No model is embedded; no game bytes leave the "
                    "machine.",
         false, "gather reads a JSON IR; build writes the state document and its reports; nothing else is read or written"},
        // forge: the device library and the cross-device match loop.
        {"forge", "The forge: a device library and a cross-device match loop. "
                  "`effect forge devices <manifest-dir-or-file>` lists the installed "
                  "device plugins; `effect forge route <manifest> <source> <target>` "
                  "plans the subsystem bridges; `effect forge registers <manifest> "
                  "<source> <target>` plans the logic registers (physics/motion/rules/"
                  "presentation/narrative); `effect forge split <main.dol> "
                  "[--out <ir.json>] [--limit N]` disassembles PowerPC into the Molerat "
                  "IR (Capstone, invoked as a tool); `effect forge match <task.json> "
                  "[--manifest <dir>] [--verifier compile --candidate-dir <dir>]` runs "
                  "the loop (synthetic by default, or a real compile-and-run verifier); "
                  "`effect forge compile <src.cpp> <out.exe> [--windowed]` builds; "
                  "`effect forge shortcut <exe> <name>` places a Desktop shortcut.",
         false, "split invokes python+capstone; match --verifier compile invokes a compiler and runs candidates; compile invokes a compiler; shortcut writes a .lnk; devices/route/registers/synthetic-match only read"},
        {"look", "What a camera sees: objects in the view frustum, largest on screen "
                 "first, with screen position. Occlusion is not computed. "
                 "`effect look [\"yaw pitch dist tx ty tz\"] [aspect] [limit]`; with no "
                 "camera, the one that frames the whole scene.",
         true, "none; computes in memory"},
    };
    for (maiz::EffectOp& op : studio_effect_ops()) h.effect_ops.push_back(std::move(op));

    h.effects = [scene](std::string_view op, std::string_view args_json) -> std::string {
        if (op == "save") return "";   // the Session persists the document; nothing else is ours to write
        if (op == "deploy" || op == "build" || op == "preview") {
            return "{\"error\":\"" + std::string(op) + " is not one of Void Squid's effects yet; known: open, residual, inspect\"}";
        }

        /* `effect open room.ink` reaches here as op == "open", args == ["room.ink"]
         * (measured 2026-10-09 against Void Core 0.2.14, which hands the custom op
         * through as the op itself). The other reading, op == "effect" with the sub-op
         * as args[0], is accepted too, so a future core that changes its mind does
         * not silently turn every effect into "unknown". */
        std::vector<std::string> args = effect_args(args_json);
        std::string sub(op);
        if (op == "effect") {
            if (args.empty()) return "{\"error\":\"effect needs an op: open|residual|inspect\"}";
            sub = args.front();
            args.erase(args.begin());
        }

        {
            std::string reply;
            if (studio_effect(sub, args, reply)) return reply;
        }

        if (sub == "world") {   // a whole table, every model through Jelly's reader, as one Ink
            if (args.size() < 2) return "{\"error\":\"effect world needs <library-folder> <world-file>\"}";
            if (!open_world(*scene, args[0], args[1])) return "{\"error\":" + quote(scene->error) + "}";
            scene->sim_ready = false;
            scene->instances.clear();
            return scene_summary_json(*scene);
        }
        if (sub == "open") {
            if (args.empty()) return "{\"error\":\"effect open needs a path\"}";
            if (!open_ink(*scene, args[0])) return "{\"error\":" + quote(scene->error) + "}";
            scene->sim_ready = false;
            scene->instances.clear();
            return scene_summary_json(*scene);
        }

        /* `effect make <preset> <path>`: build a preset scene from Jelly's primitives
         * and write it. A file write, so it is an effect like any other. */
        if (sub == "make") {
            if (args.size() < 2) {
                std::string known;
                for (const std::string& n : presets::names()) known += (known.empty() ? "" : ", ") + n;
                return "{\"error\":\"effect make needs <preset> <path>; presets: " + known + "\"}";
            }
            const voidjelly::Ink ink = presets::by_name(args[0]);
            if (ink.objects.empty()) return "{\"error\":\"unknown preset '" + args[0] + "'\"}";
            const std::vector<uint8_t> bytes = voidjelly::write_ink(ink);
            std::ofstream out(args[1], std::ios::binary);
            if (!out) return "{\"error\":\"cannot write '" + args[1] + "'\"}";
            out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            return "{\"made\":" + quote(args[0]) + ",\"path\":" + quote(args[1]) + ",\"objects\":" +
                   std::to_string(ink.objects.size()) + ",\"bytes\":" + std::to_string(bytes.size()) + "}";
        }
        /* `effect recon <path>`: the ingress arrow's first stage (okf/concepts/recon.md).
         * Needs no scene; the identity is derived, re-runnable from the path alone. */
        if (sub == "recon") {
            if (args.empty()) return "{\"error\":\"effect recon needs a file or folder path\"}";
            return recon(args[0]);
        }
        // molerat: the decompilation-and-re-expression harness. It needs no scene.
        if (sub == "molerat") {
            std::string reply;
            if (molerat_effect(args, reply)) return reply;
            return "{\"error\":\"effect molerat needs gather|research|compose|residual|build|status|task|answer|verify\"}";
        }
        // forge: the device library and the cross-device match loop. Needs no scene.
        if (sub == "forge") {
            std::string reply;
            if (forge_effect(args, reply)) return reply;
            return "{\"error\":\"effect forge needs devices|route|registers|split|match|compile|shortcut\"}";
        }
        if (!scene->loaded) return "{\"error\":\"no scene open; run `effect open <path>` first\"}";
        if (sub == "residual") return scene->gl.residual.to_json();

        /* ── the spatial log (okf/design/smart-logs.md) ── every answer is Jelly's
         * pure geometry; this host only parses arguments and holds the last look. */
        if (sub == "survey") {
            const int cells = args.empty() ? 4 : std::atoi(args[0].c_str());
            return voidjelly::survey_json(voidjelly::survey(scene->ink, cells), scene->ink.unit_scale);
        }
        if (sub == "near") {
            if (args.size() < 4) return "{\"error\":\"effect near needs x y z radius (scaled units) [limit]\"}";
            const voidjelly::Vec3 p{std::atof(args[0].c_str()), std::atof(args[1].c_str()), std::atof(args[2].c_str())};
            const double r = std::atof(args[3].c_str());
            const size_t limit = args.size() > 4 ? static_cast<size_t>(std::max(1, std::atoi(args[4].c_str()))) : 20;
            return voidjelly::near_json(voidjelly::near(scene->ink, p, r, limit), p, r);
        }
        if (sub == "look") {
            /* `effect look ["yaw pitch dist tx ty tz"] [aspect] [limit]`. With no
             * camera, the framing camera, so a headless agent's first look is useful. */
            Camera3D cam = frame_all(scene->gl.drawlist, Camera3D{});
            if (!args.empty() && !Camera3D::parse(args[0], cam)) {
                return "{\"error\":\"effect look: cannot parse camera '" + args[0] + "'; expected \\\"yaw pitch dist tx ty tz\\\"\"}";
            }
            const double aspect = args.size() > 1 ? std::atof(args[1].c_str()) : 16.0 / 10.0;
            const size_t limit = args.size() > 2 ? static_cast<size_t>(std::max(1, std::atoi(args[2].c_str()))) : 20;
            const voidjelly::LookCamera lc = cam.to_look(aspect);
            scene->last_look = voidjelly::look(scene->ink, lc, limit);
            scene->has_look = true;
            return voidjelly::look_json(scene->last_look, lc);
        }
        if (sub == "inspect") {
            const voidjelly::VerbResult r = voidjelly::dispatch(
                "inspect", "{}", scene->bytes.data(), scene->bytes.size());
            return r.json;
        }
        /* ── simulation (okf/design/simulation.md) ──
         *   effect sim new [instances=1] [seed=1] [rules.allo]
         *   effect sim step [ticks=20] [dt]
         *   effect sim fork <index> [new-seed]
         *   effect sim log [index=0] [limit=20]
         *   effect sim compare
         * The forge compiles; squid_sim steps; every reply is bounded. */
        if (sub == "sim") {
            const std::string op = args.empty() ? "" : args[0];
            auto arg_at = [&](size_t i, const char* def) { return args.size() > i ? args[i] : std::string(def); };
            if (op == "new") {
                const int count = std::clamp(std::atoi(arg_at(1, "1").c_str()), 1, 4096);
                const uint64_t seed = std::strtoull(arg_at(2, "1").c_str(), nullptr, 10);
                std::string rules = kDefaultRules;
                if (args.size() > 3) {
                    std::ifstream in(args[3], std::ios::binary);   // a second file read: the table's rules
                    if (!in) return "{\"error\":\"cannot read rules '" + args[3] + "'\"}";
                    rules.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
                }
                scene->table = compile_table(scene->ink, rules);
                scene->instances.clear();
                for (int k = 0; k < count; ++k) scene->instances.push_back(sim::make_instance(scene->table.world, seed + static_cast<uint64_t>(k)));
                scene->sim_ready = true;
                std::string j = "{\"instances\":" + std::to_string(count) + ",\"bodies\":" +
                                std::to_string(scene->table.world.bodies.size()) + ",\"dynamic\":" + std::to_string(scene->table.dynamic) +
                                ",\"solid\":" + std::to_string(scene->table.solid) + ",\"regions\":" + std::to_string(scene->table.regions) +
                                ",\"rules\":" + quote(args.size() > 3 ? args[3] : "default") + ",\"diagnostics\":[";
                for (size_t i = 0; i < scene->table.diagnostics.size(); ++i) j += (i ? "," : "") + quote(scene->table.diagnostics[i]);
                return j + "]}";
            }
            if (!scene->sim_ready) return "{\"error\":\"no simulation; run `effect sim new` first\"}";
            const sim::World& w = scene->table.world;
            if (op == "step") {
                const int ticks = std::clamp(std::atoi(arg_at(1, "20").c_str()), 1, 1000000);
                if (args.size() > 2) scene->sim_dt = std::max(1e-6, std::atof(args[2].c_str()));
                sim::step_all(w, scene->instances, scene->sim_dt, ticks);
                return sim::compare_json(w, scene->instances);
            }
            if (op == "fork") {
                const size_t idx = static_cast<size_t>(std::atoi(arg_at(1, "0").c_str()));
                if (idx >= scene->instances.size()) return "{\"error\":\"no instance " + std::to_string(idx) + "\"}";
                const bool reseed = args.size() > 2;
                scene->instances.push_back(sim::fork(w, scene->instances[idx], reseed ? std::strtoull(args[2].c_str(), nullptr, 10) : 0, reseed));
                return "{\"forked\":" + std::to_string(idx) + ",\"as\":" + std::to_string(scene->instances.size() - 1) +
                       ",\"reseeded\":" + (reseed ? "true" : "false") + "}";
            }
            if (op == "log") {
                const size_t idx = static_cast<size_t>(std::atoi(arg_at(1, "0").c_str()));
                if (idx >= scene->instances.size()) return "{\"error\":\"no instance " + std::to_string(idx) + "\"}";
                return sim::instance_json(w, scene->instances[idx], static_cast<size_t>(std::max(1, std::atoi(arg_at(2, "20").c_str()))));
            }
            if (op == "compare") return sim::compare_json(w, scene->instances);
            return "{\"error\":\"effect sim needs new|step|fork|log|compare\"}";
        }

        return "{\"error\":\"unknown effect '" + sub + "'; known: open, make, recon, molerat, forge, residual, inspect, survey, near, look, sim, library, setup, loadgame, catalogue, play, table3d, reveal\"}";
    };

    return app;
}

}  // namespace squid
