/* effects.cpp — see effects.hpp. */
#include "effects.hpp"

#include "app.hpp"
#include "host/paths.hpp"
#include "host/process.hpp"
#include "library/catalogue.hpp"
#include "library/library.hpp"
#include "library/loadgame.hpp"
#include "library/progress.hpp"
#include "library/pilot.hpp"
#include "library/setup.hpp"
#include "motion/effect.hpp"   // ocean: `effect motion` (S34), its own file

#include "json.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifndef SQUID_SOURCE_DIR
#define SQUID_SOURCE_DIR "."
#endif

namespace squid {

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

const char* kCommunityList =
    "https://raw.githubusercontent.com/CharlotteCross1998/awesome-game-decompilations/main/README.md";

fs::path community_cache() { return host::config_dir() / "awesome-game-decompilations.md"; }

std::string err(const std::string& m) { return json{{"error", m}}.dump(); }

std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return {};
    std::ostringstream s;
    s << in.rdbuf();
    return s.str();
}

bool write_json(const fs::path& p, const json& j) {
    const fs::path tmp = p.string() + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out) return false;
        out << j.dump(1) << "\n";
    }
    std::error_code ec;
    fs::rename(tmp, p, ec);
    return !ec;
}

/* Both lists: the seed shipped beside the binary (or in the checkout), and the
 * community list if the person refreshed it. */
std::vector<library::KnownGame> all_known() {
    std::vector<library::KnownGame> games;
    const std::string seed = host::first_existing({host::ship_dir() / "data" / "known-games.json",
                                                    fs::path(SQUID_SOURCE_DIR) / "data" / "known-games.json"});
    if (!seed.empty()) {
        try { games = library::parse_seed(json::parse(read_file(seed))); } catch (...) {}
    }
    const std::string md = read_file(community_cache());
    if (!md.empty()) {
        auto more = library::parse_community_list(md);
        games.insert(games.end(), more.begin(), more.end());
    }
    return games;
}

fs::path exe_in(const fs::path& dir, const std::string& name) {
#ifdef _WIN32
    return dir / (name + ".exe");
#else
    return dir / name;
#endif
}

}  // namespace

std::string runtime_dir() {
    std::vector<fs::path> c;
    if (const std::string set = host::load_settings().runtime_dir; !set.empty()) c.push_back(set);   // Settings says
    if (const char* e = std::getenv("VOIDSQUID_RUNTIME")) c.push_back(e);
    c.push_back(host::ship_dir());
    c.push_back(host::ship_dir() / ".." / ".." / ".." / "ds-mashup" / "build" / "bin");
    c.push_back(fs::path(SQUID_SOURCE_DIR) / ".." / "ds-mashup" / "build" / "bin");
    std::error_code ec;
    for (const auto& d : c)
        if (fs::exists(exe_in(d, "goomba_galaxy2"), ec)) return fs::absolute(d, ec).lexically_normal().string();
    return {};
}

std::string toolbox_dir() {
    std::vector<fs::path> c;
    if (const std::string set = host::load_settings().toolbox_dir; !set.empty()) c.push_back(set);
    if (const char* e = std::getenv("VOIDSQUID_TOOLBOX")) c.push_back(e);
    c.push_back(host::ship_dir() / ".." / ".." / ".." / "ds-mashup" / "toolbox");
    c.push_back(fs::path(SQUID_SOURCE_DIR) / ".." / "ds-mashup" / "toolbox");
    std::error_code ec;
    for (const auto& d : c)
        if (fs::exists(d / "build_goomba_world.py", ec)) return fs::absolute(d, ec).lexically_normal().string();
    return {};
}

std::vector<maiz::EffectOp> studio_effect_ops() {
    return {
        {"library", "A game library on this machine (models, worlds, assets.db). `effect library import <folder|assets.db>` "
                    "reads its catalogue and answers the mantles a front-end splices into the project; "
                    "`effect library export <folder> <feast-json> <worlds-json>` writes the runtime's feast.json and worlds.json.",
         false, "import reads files and writes nothing; export writes two files in the library folder"},
        {"setup", "What this machine can do: the GPU, Python, Ollama and its models, Void Bicho, coding agents, "
                  "decompilation tools, and which of the four setups (minimum, local model, vision, agent) are ready. "
                  "`effect setup [deep]`; deep also asks Bicho's PyTorch about CUDA.",
         true, "runs a few `--version`-style queries; installs and changes nothing"},
        {"loadgame", "Load a game the person owns: `effect loadgame <file|folder>`. Recon reads it, the catalogue says "
                     "whether a community already recovered it, the setups say what this machine can do next. "
                     "Answers a plan; changes nothing.",
         true, "reads a file header or a folder listing; writes nothing"},
        {"catalogue", "Known decompilations. `effect catalogue refresh` downloads the community list "
                      "(github.com/CharlotteCross1998/awesome-game-decompilations) into this person's settings folder; "
                      "`effect catalogue search <title> [platform]` matches a title against it and the shipped seed.",
         false, "refresh makes one network request and writes one file in the settings folder"},
        {"play", "Play a world: `effect play <library-folder> <world-file> [feast-json]` writes the feast and starts "
                 "the runtime on that world in its own window.",
         false, "writes feast.json in the library and starts a program"},
        {"table3d", "Open the 3D table on a world: `effect table3d <library-folder> [world-file]`. The exercise's raylib "
                    "editor, until Void Squid's own 3D table lands.",
         false, "starts a program"},
        {"decomp", "The community decompilations' own progress, from decomp.dev: `effect decomp refresh` downloads its project list "
                   "(one request, into the settings folder); `effect decomp` says whether a copy is here. Code and data matched, per project."},
        {"ingest", "Bring a game into a library: `effect ingest <route> <game-file-or-folder> <library-folder>` starts "
                   "the route (toolbox/route.py: dk64, ds or smg) as its own process and returns at once; it writes "
                   "<library>/ingest.log as it goes and <library>/ingest.done when it ends. Then open the library.",
         false, "starts a program that reads the game you own and writes models, worlds and assets.db into the library folder"},
        {"progress", "How much of each game a library holds: `effect progress <library-folder>`. Measured against "
                     "what the game's fan wiki lists and what its Wikipedia article links under Gameplay and Plot, "
                     "with the missing pages ranked by how central they are, plus characters, animations and worlds.",
         true, "reads the library's assets.db; writes nothing"},
        {"pilot", "A small model, held to account (library/pilot.hpp). `effect pilot test <library> [model] [cases]` hides "
                  "wiki matches already known to be right and measures whether the local model recovers them, against a "
                  "no-model baseline. `effect pilot propose <library> [model] [count]` proposes wiki pages for turkeys with "
                  "none; each proposal passes a shortlist, a schema-constrained answer, self-agreement and a kind check, and is "
                  "still only a proposal. Needs Ollama running.",
         true, "asks the local model (Ollama on this machine, nothing leaves it); writes nothing"},
        {"reveal", "Show a file or folder in the system's file manager: `effect reveal <path>`.", false,
         "opens a window of another program"},
        motion::effect_op(),   // ocean: motion/effect.hpp
    };
}

bool studio_effect(const std::string& sub, const std::vector<std::string>& args, std::string& reply) {
    auto arg = [&](size_t i) { return i < args.size() ? args[i] : std::string(); };

    if (sub == "library") {
        if (arg(0) == "import") {
            if (arg(1).empty()) { reply = err("effect library import needs a library folder or its assets.db"); return true; }
            reply = library::read_library(arg(1)).dump();
            return true;
        }
        if (arg(0) == "export") {
            if (args.size() < 4) { reply = err("effect library export needs <folder> <feast-json> <worlds-json>"); return true; }
            json feast, worlds;
            try { feast = json::parse(arg(2)); worlds = json::parse(arg(3)); } catch (const std::exception& e) {
                reply = err(std::string("not JSON: ") + e.what());
                return true;
            }
            const fs::path dir = arg(1);
            json done = json::array();
            if (!feast.is_null() && write_json(dir / "feast.json", feast)) done.push_back("feast.json");
            if (!worlds.is_null() && write_json(dir / "worlds.json", worlds)) done.push_back("worlds.json");
            reply = json{{"wrote", done}, {"library", dir.string()}}.dump();
            return true;
        }
        reply = err("effect library needs import|export");
        return true;
    }
    if (sub == "setup") {
        reply = library::inventory(arg(0) == "deep").dump();
        return true;
    }
    if (sub == "loadgame") {
        if (arg(0).empty()) { reply = err("effect loadgame needs a game file or folder"); return true; }
        const std::string target = library::pick_game_file(arg(0));
        json rj;
        try { rj = json::parse(recon(target)); } catch (...) { rj = json::object(); }
        if (rj.contains("error")) { reply = rj.dump(); return true; }
        json p = library::plan(target, rj, all_known(), library::inventory(false));
        p["community_list"] = fs::exists(community_cache()) ? "downloaded" : "not downloaded (Refresh list)";
        reply = p.dump();
        return true;
    }
    if (sub == "decomp") {
        const fs::path out = host::config_dir() / "decomp_dev.json";
        if (arg(0) == "refresh") {
            const auto c = host::capture({"curl", "-sSfL", "--max-time", "30", "-H", "Accept: application/json", "-o", out.string(),
                                          "https://decomp.dev/projects.json"}, 4096, 40000);
            if (!c.ran) { reply = err("curl was not found; install curl"); return true; }
            if (c.exit_code != 0) { reply = err("download failed: " + c.out); return true; }
        }
        json d = json::parse(read_file(out), nullptr, false);
        if (d.is_object() && d.contains("projects")) d = d["projects"];   // the API answers {"projects": [...]}
        if (!d.is_array()) { reply = err("no copy of decomp.dev's list yet: `effect decomp refresh` fetches it (one request)"); return true; }
        reply = json{{"saved", out.string()}, {"projects", d.size()}, {"from", "https://decomp.dev/projects.json"}}.dump();
        return true;
    }
    if (sub == "catalogue") {
        if (arg(0) == "refresh") {
            const fs::path out = community_cache();
            const auto c = host::capture({"curl", "-sSfL", "--max-time", "30", "-o", out.string(), kCommunityList}, 4096, 40000);
            if (!c.ran) { reply = err("curl was not found; install curl, or download the list by hand to " + out.string()); return true; }
            if (c.exit_code != 0) { reply = err("download failed: " + c.out); return true; }
            const auto games = library::parse_community_list(read_file(out));
            reply = json{{"saved", out.string()}, {"entries", games.size()}, {"from", kCommunityList}}.dump();
            return true;
        }
        if (arg(0) == "search") {
            const auto m = library::match({arg(1)}, arg(2), all_known(), 20);
            json a = json::array();
            for (const auto& x : m) { json j = library::to_json(x.game); j["why"] = x.why; j["score"] = x.score; a.push_back(j); }
            reply = json{{"matches", a}}.dump();
            return true;
        }
        const auto all = all_known();
        reply = json{{"entries", all.size()}, {"community_list", fs::exists(community_cache())}}.dump();
        return true;
    }
    if (sub == "play" || sub == "table3d") {
        const std::string rt = runtime_dir();
        if (rt.empty()) {
            reply = err("the runtime was not found: set VOIDSQUID_RUNTIME to the folder holding goomba_galaxy2, or build ../ds-mashup");
            return true;
        }
        if (arg(0).empty()) { reply = err("effect " + sub + " needs the library folder"); return true; }
        if (sub == "play") {
            if (args.size() > 2 && !arg(2).empty()) {
                try { write_json(fs::path(arg(0)) / "feast.json", json::parse(arg(2))); } catch (...) {
                    reply = err("the feast is not JSON");
                    return true;
                }
            }
            std::vector<std::string> argv{exe_in(rt, "goomba_galaxy2").string(), arg(0)};
            if (!arg(1).empty()) { argv.push_back("--world"); argv.push_back(arg(1)); }
            const bool ok = host::launch(argv, rt);
            reply = ok ? json{{"started", "goomba_galaxy2"}, {"world", arg(1)}, {"runtime", rt}}.dump()
                       : err("could not start " + argv[0]);
            return true;
        }
        std::vector<std::string> argv{exe_in(rt, "turkey_table").string(), arg(0)};
        if (!arg(1).empty()) { argv.push_back("--open"); argv.push_back(arg(1)); }
        const bool ok = host::launch(argv, rt);
        reply = ok ? json{{"started", "turkey_table"}, {"world", arg(1)}}.dump() : err("could not start " + argv[0]);
        return true;
    }
    if (sub == "pilot") {
        const std::string mode = arg(0), lib = arg(1);
        if ((mode != "test" && mode != "propose") || lib.empty()) { reply = err("effect pilot needs test|propose <library> [model] [count] [turkey]"); return true; }
        std::string model = arg(2);
        if (model.empty()) model = host::load_settings().ollama_model;
        if (model.empty()) model = "qwen3:0.6b";
        const size_t count = arg(3).empty() ? 25 : static_cast<size_t>(std::max(1, std::atoi(arg(3).c_str())));
        std::vector<library::PilotTurkey> turkeys;
        std::vector<library::PilotEntity> ents;
        std::string why;
        if (!library::read_pilot_rows(lib, turkeys, ents, why)) { reply = err(why); return true; }
        const fs::path req = host::config_dir() / "pilot_request.json";
        library::AskModel ask = [&](const std::string& prompt, const json& schema) -> json {
            const json body = {{"model", model}, {"stream", false}, {"think", false}, {"format", schema},
                               {"options", {{"temperature", 0}}}, {"messages", json::array({{{"role", "user"}, {"content", prompt}}})}};
            { std::ofstream o(req, std::ios::binary); o << body.dump(); }
            const auto c = host::capture({"curl", "-s", "-X", "POST", "http://localhost:11434/api/chat", "--data-binary", "@" + req.string()}, 1 << 20, 120000);
            if (!c.ran || c.exit_code != 0) return {{"error", "Ollama did not answer (is it running?)"}};
            try {
                const json r = json::parse(c.out);
                if (r.contains("error")) return {{"error", r["error"]}};
                return json::parse(r["message"].value("content", std::string("{}")));
            } catch (...) { return {{"error", "the model's answer was not JSON"}}; }
        };
        if (mode == "test") {
            reply = library::pilot_test(turkeys, ents, ask, model, count).dump();
            return true;
        }
        std::map<std::string, std::vector<std::string>> taken;   // per game: pages already matched
        for (const auto& t : turkeys) if (!t.known.empty()) taken[t.game].push_back(t.known);
        json props = json::array();
        size_t n = 0;
        const std::string only = arg(4);   // one turkey, when the studio asks about the one on screen
        for (const auto& t : turkeys) {
            if (!only.empty() && t.id != only) continue;
            if (!t.known.empty() || n >= count || (only.empty() && t.kind == "planet")) continue;
            ++n;
            json r = library::propose(t, ents, ask, taken[t.game]);
            if (r["proposal"].is_string()) taken[t.game].push_back(r["proposal"].get<std::string>());
            r["model"] = model;
            props.push_back(r);
        }
        // kept in the library, by turkey: the studio shows them on the Turkey card for a person to decide
        const fs::path keep = fs::path(lib) / "pilot_proposals.json";
        json all = json::parse(read_file(keep), nullptr, false);
        if (!all.is_object()) all = json::object();
        for (const auto& p : props) all[p.value("turkey", "")] = p;
        write_json(keep, all);
        reply = json{{"model", model}, {"asked", n}, {"proposals", props}, {"saved", keep.string()}}.dump();
        return true;
    }
    if (sub == "progress") {
        if (arg(0).empty()) { reply = err("effect progress needs the library folder"); return true; }
        json seed;
        const std::string sp = host::first_existing({host::ship_dir() / "data" / "known-games.json", fs::path(SQUID_SOURCE_DIR) / "data" / "known-games.json"});
        if (!sp.empty()) { try { seed = json::parse(read_file(sp)); } catch (...) {} }
        reply = library::progress_of_library(arg(0), seed).dump();
        return true;
    }
    if (sub == "ingest") {
        if (args.size() < 3) { reply = err("effect ingest needs <route> <game> <library-folder> [minimum|local|vision]"); return true; }
        const std::string tb = toolbox_dir();
        if (tb.empty()) { reply = err("the ingest routes were not found: set the toolbox folder in Settings > Places"); return true; }
        std::string py = host::which("python");
        if (py.empty()) py = host::which("python3");
        if (py.empty()) { reply = err("Python was not found; the routes are Python (SETUP.md)"); return true; }
        std::error_code ec;
        fs::create_directories(arg(2), ec);
        const host::Settings st = host::load_settings();
        const std::string tier = args.size() > 3 ? arg(3) : st.load_tier;
        fs::create_directories(host::tools_dir(), ec);
        const bool ok = host::launch({py, (fs::path(tb) / "route.py").string(), arg(0), arg(1), arg(2), "--tools", host::tools_dir().string(),
                                      "--tier", tier.empty() ? "minimum" : tier}, tb);
        reply = ok ? json{{"started", arg(0)}, {"game", arg(1)}, {"library", arg(2)}, {"log", (fs::path(arg(2)) / "ingest.log").string()}}.dump()
                   : err("could not start the route");
        return true;
    }
    if (sub == "reveal") {
        reply = host::open_with_system(arg(0)) ? json{{"opened", arg(0)}}.dump() : err("could not open " + arg(0));
        return true;
    }
    if (sub == "motion") { motion::effect(args, reply); return true; }   // ocean: motion/effect.hpp
    return false;
}

}  // namespace squid
