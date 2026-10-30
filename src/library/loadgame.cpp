/* library/loadgame.cpp — see loadgame.hpp. */
#include "library/loadgame.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>

namespace squid::library {

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
const std::set<std::string>& game_exts() {
    static const std::set<std::string> e = {".z64", ".n64", ".v64", ".nds", ".dsi", ".gba", ".gb", ".gbc", ".nes",
                                            ".sfc", ".smc", ".iso", ".gcm", ".rvz", ".wbfs", ".ciso", ".wad",
                                            ".3ds", ".cia", ".xci", ".nsp", ".cue", ".chd"};
    return e;
}
std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool ready(const json& inv, const char* id) {
    for (const auto& s : inv.value("setups", json::array()))
        if (s.value("id", "") == id) return s.value("ready", false);
    return false;
}
}  // namespace

std::string pick_game_file(const std::string& path) {
    std::error_code ec;
    if (!fs::is_directory(path, ec)) return path;
    struct Norm { static std::string of(const fs::path& p) { return p.lexically_normal().make_preferred().string(); } };
    std::vector<fs::path> hits;
    int n = 0;
    for (const auto& e : fs::directory_iterator(path, ec)) {
        if (++n > 512) break;   // an install folder, not a ROM folder
        if (e.is_regular_file(ec) && game_exts().count(lower(e.path().extension().string()))) hits.push_back(e.path());
    }
    return hits.size() == 1 ? Norm::of(hits[0]) : path;
}

std::vector<std::string> candidate_titles(const json& recon, const std::string& path) {
    std::vector<std::string> t;
    if (recon.contains("title") && recon["title"].is_string()) t.push_back(recon["title"].get<std::string>());
    const fs::path p(path);
    t.push_back(p.stem().string());
    if (p.has_parent_path()) t.push_back(p.parent_path().filename().string());
    return t;
}

json plan(const std::string& path, const json& recon, const std::vector<KnownGame>& games, const json& inv) {
    json out;
    out["target"] = path;
    out["recon"] = {{"kind", recon.value("kind", json())}, {"platform", recon.value("platform", json())},
                    {"title", recon.value("title", json())}};
    if (recon.contains("header")) out["recon"]["header"] = recon["header"];
    const auto titles = candidate_titles(recon, path);
    out["titles"] = titles;
    const std::string platform = recon.value("platform", json()).is_string() ? recon["platform"].get<std::string>() : "";
    const auto matches = match(titles, platform, games);
    json mj = json::array();
    for (const auto& m : matches) {
        json j = to_json(m.game);
        j["why"] = m.why;
        j["score"] = m.score;
        mj.push_back(j);
    }
    out["matches"] = mj;

    const bool known = !matches.empty() && matches[0].score >= 80;
    const bool routed = known && !matches[0].game.route.empty();
    const bool local = ready(inv, "local"), agent = ready(inv, "agent"), vision = ready(inv, "vision");

    json steps = json::array();
    auto step = [&](const std::string& s, const std::string& setup, const std::string& state, const std::string& how) {
        steps.push_back({{"step", s}, {"setup", setup}, {"state", state}, {"how", how}});
    };
    step("Identify the game", "minimum", "done",
         recon.value("kind", json()).is_string()
             ? "recon read it as " + recon["kind"].get<std::string>() + (platform.empty() ? "" : " for " + platform) +
                   (recon.value("title", json()).is_string() ? ", titled \"" + recon["title"].get<std::string>() + "\"" : "")
             : "recon could not identify it; see its residual");
    if (known)
        step("Find an existing decompilation", "minimum", "done",
             "matched " + matches[0].game.title + " (" + matches[0].why + ")" +
                 (matches[0].game.url.empty() ? "" : ": " + matches[0].game.url));
    else if (!matches.empty())
        step("Find an existing decompilation", "minimum", "check",
             "only weak matches; confirm one by hand, or refresh the community list (Load a game > Refresh list)");
    else
        step("Find an existing decompilation", "minimum", "none",
             "no known project; refresh the community list, or this game needs the agent setup");
    if (routed) {
        step("Extract its assets", "minimum", "ready", matches[0].game.route);
        out["route_id"] = matches[0].game.route_id;
    }
    else if (known)
        step("Extract its assets", "agent", agent ? "ready" : "needs",
             "a decompilation exists but Void Squid has no route for it yet; an agent can write one against the project's tools");
    else
        step("Extract its assets", "agent", agent ? "ready" : "needs",
             "nobody has recovered this game yet: decompiling it is the agent setup's job (REA, Ghidra, universal-modder)");
    step("Name and tag what came out", "minimum", "manual",
         "names from the game's own data, tags from the game, its wiki and the names (the toolbox's rules); check them in the Turkey workspace");
    step("Better names and tags from a local model", "local", local ? "planned" : "optional",
         local ? "Ollama is ready here, but Void Squid does not ask it yet: planned, with every proposal checked against the vocabulary and the wiki"
               : "optional: Ollama with a small model (SETUP.md); planned, not built");
    step("Tag by what things look like", "vision", vision ? "ready" : "optional",
         vision ? "Void Bicho's CLIP tags each thumbnail" : "optional: install Void Bicho's environment (SETUP.md)");
    step("Open the library in Void Squid", "minimum", "ready", "File > Open library..., then build worlds in the Table");
    out["steps"] = steps;

    std::string summary;
    if (routed) summary = "Known, and Void Squid has a route: the minimum setup can bring it in.";
    else if (known) summary = "Known to the community, but Void Squid has no route for it yet.";
    else summary = "Not in any list Void Squid knows. Recovering it is a decompilation job.";
    out["summary"] = summary;
    return out;
}

}  // namespace squid::library
