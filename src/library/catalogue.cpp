/* library/catalogue.cpp — see catalogue.hpp. Pure: no file, no network. */
#include "library/catalogue.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace squid::library {

using json = nlohmann::json;

std::string fold_title(const std::string& t) {
    std::string s;
    int depth = 0;
    for (size_t i = 0; i < t.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(t[i]);
        if (c == '(' || c == '[') { ++depth; continue; }
        if (c == ')' || c == ']') { if (depth) --depth; continue; }
        if (depth) continue;
        if (c == '&') { s += "and"; continue; }
        if (std::isalnum(c)) s += static_cast<char>(std::tolower(c));
        // é and the like: UTF-8 lead/continuation bytes are dropped; "Pokémon" -> "pokmon",
        // the same on both sides, which is all a comparison needs
    }
    return s;
}

std::string platform_of_heading(const std::string& h) {
    std::string l;
    for (char c : h) l += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    auto has = [&](const char* w) { return l.find(w) != std::string::npos; };
    if (has("nintendo 64")) return "n64";
    if (has("3ds")) return "3ds";
    if (has("nintendo ds")) return "ds";
    if (has("wii u")) return "wiiu";
    if (has("wii")) return "wii";
    if (has("gamecube")) return "gc";
    if (has("switch")) return "switch";
    if (has("advance")) return "gba";
    if (has("colour") || has("color")) return "gbc";
    if (has("gameboy") || has("game boy")) return "gb";
    if (has("snes") || has("super famicom")) return "snes";
    if (has("nes") || has("famicom")) return "nes";
    if (has("playstation 2")) return "ps2";
    if (has("playstation")) return "ps1";
    if (has("xbox 360")) return "x360";
    if (has("xbox one")) return "xone";
    if (has("xbox")) return "xbox";
    if (has("pc") || has("computer")) return "pc";
    if (has("mobile")) return "mobile";
    return "";
}

std::vector<KnownGame> parse_seed(const json& j) {
    std::vector<KnownGame> out;
    if (!j.contains("games") || !j["games"].is_array()) return out;
    for (const auto& g : j["games"]) {
        KnownGame k;
        k.title = g.value("title", "");
        if (g.contains("aliases") && g["aliases"].is_array())
            for (const auto& a : g["aliases"]) if (a.is_string()) k.aliases.push_back(a.get<std::string>());
        k.platform = g.value("platform", "");
        k.project = g.value("project", "");
        k.url = g.value("url", "");
        k.kind = g.value("kind", "");
        k.route = g.value("route", "");
        k.route_id = g.value("route_id", "");
        k.notes = g.value("notes", "");
        k.from = "seed";
        if (!k.title.empty()) out.push_back(k);
    }
    return out;
}

std::vector<KnownGame> parse_community_list(const std::string& md) {
    std::vector<KnownGame> out;
    std::istringstream in(md);
    std::string line, platform, heading;
    static const std::regex item(R"(^\s*-\s*(//)?\s*\[([^\]]+)\]\(([^)\s]+)\))");
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("## ", 0) == 0) { heading = line.substr(3); platform = platform_of_heading(heading); continue; }
        if (line.rfind("# ", 0) == 0) { heading.clear(); platform.clear(); continue; }
        std::smatch m;
        if (heading.empty() || !std::regex_search(line, m, item)) continue;
        KnownGame k;
        std::string title = m[2].str();
        const std::string warn = "\xE2\x9A\xA0";   // the list's AI mark
        if (title.find(warn) != std::string::npos) {
            k.ai_flagged = true;
            title.erase(0, title.find(warn) + warn.size());
            while (!title.empty() && (static_cast<unsigned char>(title[0]) >= 0x80 || title[0] == ' ')) title.erase(0, 1);
        }
        k.work_in_progress = m[1].matched;
        // "Battle City (Browser reimplementation)": the parenthesis says what kind of project
        const auto lp = title.find('(');
        std::string kind = "decompilation";
        if (lp != std::string::npos) {
            std::string paren = title.substr(lp);
            std::string low;
            for (char c : paren) low += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (low.find("recomp") != std::string::npos) kind = "recompilation";
            else if (low.find("reimplement") != std::string::npos || low.find("port") != std::string::npos) kind = "reimplementation";
            else if (low.find("disassembl") != std::string::npos) kind = "disassembly";
        }
        const std::string url = m[3].str();
        if (url.find("isassembly") != std::string::npos || url.find("disasm") != std::string::npos) kind = "disassembly";
        if (url.find("recomp") != std::string::npos) kind = "recompilation";
        k.title = title;
        k.platform = platform;
        k.url = url;
        k.kind = kind;
        k.project = url.substr(url.find("//") == std::string::npos ? 0 : url.find("//") + 2);
        k.from = "community list";
        out.push_back(k);
    }
    return out;
}

std::vector<Match> match(const std::vector<std::string>& titles, const std::string& platform,
                         const std::vector<KnownGame>& games, size_t limit) {
    std::vector<std::string> folded;
    for (const auto& t : titles) {
        const std::string f = fold_title(t);
        if (f.size() >= 3 && std::find(folded.begin(), folded.end(), f) == folded.end()) folded.push_back(f);
    }
    std::vector<Match> out;
    for (const auto& g : games) {
        std::vector<std::string> names{g.title};
        names.insert(names.end(), g.aliases.begin(), g.aliases.end());
        int best = 0;
        std::string why;
        for (const auto& n : names) {
            const std::string gn = fold_title(n);
            if (gn.size() < 3) continue;
            for (const auto& f : folded) {
                int s = 0;
                std::string w;
                if (f == gn) { s = 90; w = "same title"; }
                else if (f.size() >= 6 && gn.size() >= 6 && (f.find(gn) != std::string::npos || gn.find(f) != std::string::npos)) {
                    // containment is weaker, and weaker still when the lengths differ a lot
                    const double ratio = static_cast<double>(std::min(f.size(), gn.size())) / std::max(f.size(), gn.size());
                    s = static_cast<int>(40 + 40 * ratio);
                    w = "one title contains the other";
                }
                if (s > best) { best = s; why = w + " (\"" + n + "\")"; }
            }
        }
        if (best == 0) continue;
        if (!platform.empty() && !g.platform.empty()) {
            // recon may say "gamecube-or-wii" when only the container is known: either counts
            bool same = platform == g.platform;
            for (size_t a = 0, b; !same && a <= platform.size(); a = b + 4) {
                b = platform.find("-or-", a);
                if (b == std::string::npos) b = platform.size();
                same = platform.substr(a, b - a) == g.platform || (platform.substr(a, b - a) == "gamecube" && g.platform == "gc");
                if (b == platform.size()) break;
            }
            if (same) { best += 10; why += ", same platform"; }
            else { best -= 45; why += ", but a different platform (" + g.platform + ")"; }
        }
        if (g.from == "seed") { best += 2; }
        if (best <= 0) continue;
        out.push_back({g, why, std::min(best, 100)});
    }
    std::stable_sort(out.begin(), out.end(), [](const Match& a, const Match& b) { return a.score > b.score; });
    if (out.size() > limit) out.resize(limit);
    return out;
}

json to_json(const KnownGame& g) {
    json j = {{"title", g.title}, {"platform", g.platform}, {"project", g.project}, {"url", g.url},
              {"kind", g.kind}, {"from", g.from}};
    if (!g.route.empty()) j["route"] = g.route;
    if (!g.route_id.empty()) j["route_id"] = g.route_id;
    if (!g.notes.empty()) j["notes"] = g.notes;
    if (g.ai_flagged) j["ai_flagged"] = true;
    if (g.work_in_progress) j["work_in_progress"] = true;
    return j;
}

}  // namespace squid::library
