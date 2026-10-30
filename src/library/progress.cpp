/* library/progress.cpp — see progress.hpp. */
#include "library/progress.hpp"
#include "library/pilot.hpp"

#include "sqlite3.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace squid::library {

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
std::vector<std::string> split_list(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        while (!item.empty() && item.front() == ' ') item.erase(0, 1);
        while (!item.empty() && item.back() == ' ') item.pop_back();
        if (!item.empty()) out.push_back(item);
    }
    return out;
}
/* A section's family: "wikipedia:gameplay" stays itself; a fan-wiki section is its own word. */
std::string family(const std::string& section) { return section; }
bool has_cat(const WikiEntity& e, const char* word) {
    for (const auto& c : e.categories) if (c == word) return true;
    return false;
}
bool cat_contains(const WikiEntity& e, const char* word) {
    for (const auto& c : e.categories) if (c.find(word) != std::string::npos) return true;
    return false;
}
}  // namespace

std::string classify(const WikiEntity& e) {
    if (has_cat(e, "Games") || cat_contains(e, " games") || cat_contains(e, "Launch titles")) return "game";
    if (has_cat(e, "Game mechanics") || has_cat(e, "Terms") || cat_contains(e, "abilities") || cat_contains(e, "Moves")) return "system";
    // by category alone: a wiki's location list also names the characters found there
    bool place = false;
    if (cat_contains(e, "locations") || cat_contains(e, "Galaxies") || cat_contains(e, "courses") || cat_contains(e, "levels") || cat_contains(e, "Worlds")) place = true;
    return place ? "place" : "thing";
}

json progress_report(const std::vector<WikiEntity>& wiki, const std::vector<TurkeyRow>& turkeys,
                     const std::vector<std::pair<std::string, std::string>>& mentions, const json& worlds,
                     const json& known_games) {
    // which entities a turkey stands for (by its wiki match, and through a redirect)
    std::map<std::string, std::string> redirect;
    for (const auto& e : wiki) if (!e.redirect.empty()) redirect[e.name] = e.redirect;
    std::map<std::string, std::vector<const TurkeyRow*>> by_entity;
    for (const auto& t : turkeys) {
        if (t.wiki.empty()) continue;
        by_entity[t.wiki].push_back(&t);
        auto r = redirect.find(t.wiki);
        if (r != redirect.end()) by_entity[r->second].push_back(&t);
    }
    std::map<std::string, int> degree;
    for (const auto& [a, b] : mentions) { ++degree[a]; ++degree[b]; }

    std::set<std::string> games;
    for (const auto& e : wiki) for (const auto& g : e.games) games.insert(g);
    for (const auto& t : turkeys) if (!t.game.empty()) games.insert(t.game);

    json out = json::array();
    for (const auto& game : games) {
        json g = {{"title", game}};
        // ── the wikis ──
        std::vector<std::string> table_titles;
        if (worlds.is_object() && worlds.contains("worlds"))
            for (const auto& x : worlds["worlds"]) table_titles.push_back(x.value("title", ""));
        int total = 0, matched = 0, wp_total = 0, wp_matched = 0, other_games = 0;
        std::map<std::string, std::pair<int, int>> by_class;
        std::map<std::string, std::pair<int, int>> by_section;
        std::vector<std::tuple<int, std::string, std::string, std::string>> missing;   // degree, name, section, description
        for (const auto& e : wiki) {
            if (std::find(e.games.begin(), e.games.end(), game) == e.games.end()) continue;
            if (!e.redirect.empty()) continue;   // the page it redirects to is counted instead
            const std::string cls = classify(e);
            if (cls == "game") { ++other_games; continue; }   // another game, mentioned: not in this one
            ++total;
            bool have = by_entity.count(e.name) > 0;
            if (!have && cls == "place")   // a place is recovered when a table holds it
                for (const auto& t : table_titles) if (!e.name.empty() && t.find(e.name) != std::string::npos) have = true;
            auto& bc = by_class[cls];
            ++bc.first;
            if (have) ++bc.second;
            if (have) ++matched;   // (a place held by a table counts too)
            bool on_wikipedia = false;
            std::string first_section;
            for (const auto& s : e.sections) {
                auto& c = by_section[family(s)];
                ++c.first;
                if (have) ++c.second;
                if (s.rfind("wikipedia:", 0) == 0) on_wikipedia = true;
                else if (first_section.empty()) first_section = s;
            }
            if (on_wikipedia) { ++wp_total; if (have) ++wp_matched; }
            if (!have) missing.push_back({degree[e.name], e.name, cls + (first_section.empty() ? std::string() : ", " + first_section), e.description});
        }
        std::sort(missing.begin(), missing.end(), [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });
        json sections = json::object();
        for (const auto& [s, c] : by_section) sections[s] = {{"total", c.first}, {"recovered", c.second}};
        json miss = json::array();
        for (size_t i = 0; i < missing.size() && i < 40; ++i)
            miss.push_back({{"name", std::get<1>(missing[i])}, {"section", std::get<2>(missing[i])}, {"mentions", std::get<0>(missing[i])},
                            {"description", std::get<3>(missing[i])}});
        json classes = json::object();
        for (const auto& [c, v] : by_class) classes[c] = {{"total", v.first}, {"recovered", v.second}};
        g["wiki"] = {{"total", total}, {"recovered", matched}, {"percent", total ? 100.0 * matched / total : 0.0},
                     {"classes", classes}, {"sections", sections}, {"missing", miss}, {"other_games_mentioned", other_games}};
        g["wikipedia"] = {{"total", wp_total}, {"recovered", wp_matched}, {"percent", wp_total ? 100.0 * wp_matched / wp_total : 0.0}};
        // ── the library ──
        int n = 0, playable = 0, anims = 0;
        std::map<std::string, int> depth, kinds;
        for (const auto& t : turkeys) {
            if (t.game != game) continue;
            ++n;
            if (t.rig) { ++playable; anims += t.animations; }
            ++depth[t.depth.empty() ? "unstated" : t.depth];
            ++kinds[t.kind];
        }
        g["library"] = {{"turkeys", n}, {"playable", playable}, {"animations", anims}, {"depth", depth}, {"kinds", kinds}};
        int w = 0;
        if (worlds.is_object() && worlds.contains("worlds"))
            for (const auto& x : worlds["worlds"]) {
                const std::string title = x.value("title", "");
                if (title.find(game) != std::string::npos) ++w;
            }
        g["library"]["worlds"] = w;
        // ── the project ──
        if (known_games.is_object() && known_games.contains("games"))
            for (const auto& k : known_games["games"])
                if (k.value("title", "") == game) {
                    g["project"] = {{"name", k.value("project", "")}, {"url", k.value("url", "")}, {"kind", k.value("kind", "")}};
                    if (k.contains("progress_url")) g["project"]["progress_url"] = k["progress_url"];
                }
        out.push_back(g);
    }
    return {{"games", out},
            {"method", "Recovered = a turkey matched to the wiki page (by name or Japanese romanization; a proposal, S26). "
                       "The wikis are the game's fan wiki lists and the pages its Wikipedia article links under Gameplay and Plot. "
                       "Missing pages are ranked by how many other pages of the same game mention them."}};
}

json progress_of_library(const std::string& library_dir, const json& known_games) {
    const fs::path db = fs::path(library_dir) / "assets.db";
    std::error_code ec;
    if (!fs::exists(db, ec)) return {{"error", "no assets.db in " + library_dir}};
    sqlite3* h = nullptr;
    const std::string uri = "file:" + db.generic_string() + "?mode=ro";
    if (sqlite3_open_v2(uri.c_str(), &h, SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, nullptr) != SQLITE_OK) {
        if (h) sqlite3_close(h);
        return {{"error", "cannot open " + db.string()}};
    }
    auto query = [&](const char* sql, auto&& row) {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(h, sql, -1, &st, nullptr) != SQLITE_OK) return;
        while (sqlite3_step(st) == SQLITE_ROW) {
            auto col = [&](int i) {
                const unsigned char* t = sqlite3_column_text(st, i);
                return t ? std::string(reinterpret_cast<const char*>(t)) : std::string();
            };
            row(col);
        }
        sqlite3_finalize(st);
    };
    std::map<std::string, WikiEntity> ents;
    query("SELECT name FROM runes WHERE mantle='wiki'", [&](auto col) { ents[col(0)].name = col(0); });
    query("SELECT rune, key, value FROM fields WHERE mantle='wiki'", [&](auto col) {
        auto it = ents.find(col(0));
        if (it == ents.end()) return;
        const std::string k = col(1);
        if (k == "games") it->second.games = split_list(col(2));
        else if (k == "sections") it->second.sections = split_list(col(2));
        else if (k == "description") it->second.description = col(2);
        else if (k == "categories") it->second.categories = split_list(col(2));
    });
    std::vector<std::pair<std::string, std::string>> mentions;
    query("SELECT from_rune, to_rune, relation FROM links WHERE mantle='wiki'", [&](auto col) {
        if (col(2) == "mentions") mentions.push_back({col(0), col(1)});
        else if (col(2) == "redirects-to") {
            auto it = ents.find(col(0));
            if (it != ents.end()) it->second.redirect = col(1);
        }
    });
    std::map<std::string, TurkeyRow> tk;
    query("SELECT rune, key, value FROM fields WHERE mantle='turkeys'", [&](auto col) {
        TurkeyRow& t = tk[col(0)];
        t.id = col(0);
        const std::string k = col(1);
        if (k == "game") t.game = col(2);
        else if (k == "wiki") t.wiki = col(2);
        else if (k == "kind") t.kind = col(2);
        else if (k == "depth") t.depth = col(2);
        else if (k == "rig") t.rig = true;
    });
    sqlite3_close(h);
    // animations per character, from the cast
    std::ifstream cf(fs::path(library_dir) / "characters.json");
    if (cf) {
        try {
            json cast = json::parse(cf);
            for (const auto& c : cast.value("characters", json::array())) {
                auto it = tk.find(c.value("id", ""));
                if (it != tk.end()) it->second.animations = static_cast<int>(c.value("states", json::object()).size());
            }
        } catch (...) {}
    }
    json worlds;
    std::ifstream wf(fs::path(library_dir) / "worlds.json");
    if (wf) { try { worlds = json::parse(wf); } catch (...) {} }
    std::vector<WikiEntity> wv;
    for (auto& [n, e] : ents) wv.push_back(std::move(e));
    std::vector<TurkeyRow> tv;
    for (auto& [n, t] : tk) tv.push_back(std::move(t));
    // the shelf writes short game names for some DS pieces ("sm64ds"); the database already normalised most
    return progress_report(wv, tv, mentions, worlds, known_games);
}

bool read_pilot_rows(const std::string& library_dir, std::vector<PilotTurkey>& turkeys, std::vector<PilotEntity>& ents,
                     std::string& error) {
    const fs::path db = fs::path(library_dir) / "assets.db";
    std::error_code ec;
    if (!fs::exists(db, ec)) { error = "no assets.db in " + library_dir; return false; }
    sqlite3* h = nullptr;
    const std::string uri = "file:" + db.generic_string() + "?mode=ro";
    if (sqlite3_open_v2(uri.c_str(), &h, SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, nullptr) != SQLITE_OK) {
        if (h) sqlite3_close(h);
        error = "cannot open " + db.string();
        return false;
    }
    auto query = [&](const char* sql, auto&& row) {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(h, sql, -1, &st, nullptr) != SQLITE_OK) return;
        while (sqlite3_step(st) == SQLITE_ROW) {
            auto col = [&](int i) {
                const unsigned char* t = sqlite3_column_text(st, i);
                return t ? std::string(reinterpret_cast<const char*>(t)) : std::string();
            };
            row(col);
        }
        sqlite3_finalize(st);
    };
    std::map<std::string, PilotEntity> em;
    std::set<std::string> redirects;
    query("SELECT name FROM runes WHERE mantle='wiki'", [&](auto col) { em[col(0)].name = col(0); });
    query("SELECT rune, key, value FROM fields WHERE mantle='wiki'", [&](auto col) {
        auto it = em.find(col(0));
        if (it == em.end()) return;
        if (col(1) == "games") it->second.games = split_list(col(2));
        else if (col(1) == "sections") it->second.sections = split_list(col(2));
        else if (col(1) == "description") it->second.description = col(2);
    });
    query("SELECT from_rune, to_rune, relation FROM links WHERE mantle='wiki'", [&](auto col) {
        if (col(2) == "mentions") { ++em[col(1)].mentions; }
        else if (col(2) == "redirects-to") redirects.insert(col(0));
    });
    std::map<std::string, PilotTurkey> tm;
    query("SELECT rune, key, value FROM fields WHERE mantle='turkeys'", [&](auto col) {
        PilotTurkey& t = tm[col(0)];
        t.id = col(0);
        const std::string k = col(1);
        if (k == "name") t.name = col(2);
        else if (k == "class") t.cls = col(2);
        else if (k == "category") t.category = col(2);
        else if (k == "kind") t.kind = col(2);
        else if (k == "game") t.game = col(2);
        else if (k == "wiki") t.known = col(2);
    });
    sqlite3_close(h);
    for (auto& [n, e] : em) if (!redirects.count(n) && !e.name.empty()) ents.push_back(std::move(e));
    for (auto& [n, t] : tm) turkeys.push_back(std::move(t));
    return true;
}

}  // namespace squid::library
