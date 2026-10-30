/* library/library.cpp — see library.hpp. */
#include "library/library.hpp"

#include "sqlite3.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>

namespace squid::library {

namespace fs = std::filesystem;

namespace {

/* FNV-1a: a stable identity from a name, so a re-import lines runes up. Not a security
 * property; two names colliding would need 2^32 runes before it is likely. */
std::string stable_id(const char* prefix, const std::string& key) {
    uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : key) { h ^= c; h *= 1099511628211ULL; }
    char buf[32];
    std::snprintf(buf, sizeof buf, "%s_%010llx", prefix, static_cast<unsigned long long>(h & 0xffffffffffULL));
    return buf;
}

/* The thumbnail name the toolbox writes (asset_db.py: thumbnails()). */
std::string thumb_name(const std::string& id) {
    std::string s;
    for (char c : id) s += (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') ? c : '_';
    return "_thumbs/" + s + ".png";
}

const char* const kKinds = "player,planet,deco,enemy,npc,coin,starbit,oneup,powerstar,launch,sling,itembox,water";

/* Keys that are the person's, carried across a re-import. */
bool person_key(const std::string& k) {
    return k.rfind("sound_", 0) == 0 || k == "notes" || k == "kind_source";
}

}  // namespace

const std::vector<std::string>& library_mantles() {
    static const std::vector<std::string> m = {"turkeys", "games", "kinds", "tracks", "tables"};
    return m;
}

std::vector<std::string> glyphs() {
    std::vector<std::string> g;
    g.push_back(
        R"G({"glyph":"turkey","label":"Turkey","fields":["name","game","kind","description","wiki","category","class","model","collision","unit","size","depth","source","named_by","sound_collect","sound_defeat","sound_touch","sound_talk","notes"],)G"
        R"G("hints":{"color":"#d9824b","editors":{"kind":"combo:)G" + std::string(kKinds) +
        R"G(","description":"multiline:60","notes":"multiline:60","unit":"number:0.001,0,100","size":"number:0.1,0,10000","model":"path","collision":"path","sound_collect":"path","sound_defeat":"path","sound_touch":"path","sound_talk":"path"},)G"
        R"G("labels":{"name":"Name","game":"Game","kind":"Kind (what the game does with it)","description":"Description","wiki":"Wiki entity","category":"The game's own category","class":"The game's own class","model":"Model","collision":"Collision","unit":"Metres per model unit","size":"Size (m)","depth":"Depth recovered","source":"Where it came from","named_by":"Named by","sound_collect":"Sound: collect","sound_defeat":"Sound: defeat","sound_touch":"Sound: touch","sound_talk":"Sound: talk","notes":"Notes"}}})G");
    g.push_back(
        R"G({"glyph":"game","label":"Game","fields":["title","platform","wiki"],)G"
        R"G("hints":{"color":"#6b8fd6","labels":{"title":"Title","platform":"Platform","wiki":"Wiki page"}}})G");
    g.push_back(
        R"G({"glyph":"kind","label":"Kind","fields":["title"],)G"
        R"G("hints":{"color":"#8bb174","labels":{"title":"Title"}}})G");
    g.push_back(
        R"G({"glyph":"track","label":"Track","fields":["title","game","source"],)G"
        R"G("hints":{"color":"#b07cc6","labels":{"title":"Title","game":"Game","source":"Source"}}})G");
    g.push_back(
        R"G({"glyph":"table","label":"Table","fields":["title","file","music","order"],)G"
        R"G("hints":{"color":"#c9a227","editors":{"order":"number:1,0,999","file":"path"},)G"
        R"G("labels":{"title":"Title","file":"World file (in the library)","music":"Music","order":"Order in the feast"}}})G");
    g.push_back(
        R"G({"glyph":"feast","label":"Feast","fields":["player","walk","run","jump","gravity","lives","hp"],)G"
        R"G("hints":{"color":"#e0607e","editors":{"walk":"number:0.1,0.5,40","run":"number:0.1,0.5,60","jump":"number:0.1,0.5,60","gravity":"number:0.5,1,200","lives":"number:1,1,99","hp":"number:1,1,99"},)G"
        R"G("labels":{"player":"Player (a turkey)","walk":"Walk speed (m/s)","run":"Run speed (m/s)","jump":"Jump speed (m/s)","gravity":"Gravity (m/s²)","lives":"Lives","hp":"Hit points"}}})G");
    return g;
}

json make_rune(const std::string& mantle, const std::string& name, const std::string& glyph) {
    return json{{"spirit", {{"id", stable_id("rune", mantle + "/" + name)}, {"name", name}}},
                {"glyph", glyph},
                {"facets", {{"who", ""}, {"what", ""}, {"when", ""}, {"where", ""}, {"why", ""}, {"how", ""}}},
                {"tags", json::array()},
                {"content", json::object()},
                {"placement", nullptr},
                {"relations", json::array()}};
}

json make_mantle(const std::string& name) {
    return json{{"id", stable_id("mantle", name)}, {"name", name}, {"domain", nullptr},
                {"runes", json::array()}, {"tags", json::object()},
                {"layout", {{"edges", json::array()}}}, {"rules", json::array()}};
}

// ── reading a library (the effect) ──────────────────────────────────────────

json read_library(const std::string& path) {
    std::error_code ec;
    fs::path p = fs::absolute(path, ec);
    fs::path dir = fs::is_directory(p, ec) ? p : p.parent_path();
    fs::path db = fs::is_directory(p, ec) ? p / "assets.db" : p;
    if (!fs::exists(db, ec))
        return {{"error", "no assets.db in " + dir.string() + " (a library is the folder the toolbox built: models, worlds, assets.db)"}};

    sqlite3* h = nullptr;
    const std::string uri = "file:" + db.generic_string() + "?mode=ro";
    if (sqlite3_open_v2(uri.c_str(), &h, SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, nullptr) != SQLITE_OK) {
        std::string e = h ? sqlite3_errmsg(h) : "cannot open";
        if (h) sqlite3_close(h);
        return {{"error", "cannot open " + db.string() + ": " + e}};
    }

    // rows: (mantle, rune) -> glyph; fields; tags; links
    std::map<std::string, std::map<std::string, std::string>> glyph_of;   // mantle -> rune -> glyph
    std::map<std::pair<std::string, std::string>, std::map<std::string, std::string>> fields;
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> tags;
    std::map<std::string, std::vector<std::array<std::string, 3>>> links;  // mantle -> (from,to,relation)
    auto query = [&](const char* sql, auto&& row) {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(h, sql, -1, &st, nullptr) != SQLITE_OK) return false;
        while (sqlite3_step(st) == SQLITE_ROW) {
            auto col = [&](int i) {
                const unsigned char* t = sqlite3_column_text(st, i);
                return t ? std::string(reinterpret_cast<const char*>(t)) : std::string();
            };
            row(col);
        }
        sqlite3_finalize(st);
        return true;
    };
    const bool ok = query("SELECT mantle, name, glyph FROM runes", [&](auto col) { glyph_of[col(0)][col(1)] = col(2); });
    query("SELECT mantle, rune, key, value FROM fields", [&](auto col) { fields[{col(0), col(1)}][col(2)] = col(3); });
    query("SELECT mantle, rune, tag FROM tags", [&](auto col) { tags[{col(0), col(1)}].push_back(col(2)); });
    query("SELECT mantle, from_rune, to_rune, relation FROM links", [&](auto col) { links[col(0)].push_back({col(1), col(2), col(3)}); });
    sqlite3_close(h);
    if (!ok) return {{"error", db.string() + " is not an asset database (no runes table)"}};

    json mantles = json::array();
    json counts = json::object();

    // turkeys
    {
        json m = make_mantle("turkeys");
        int thumbs = 0;
        for (const auto& [name, glyph] : glyph_of["turkeys"]) {
            json r = make_rune("turkeys", name, "turkey");
            json& c = r["content"];
            json sources = json::object();
            for (const auto& [k, v] : fields[{"turkeys", name}]) {
                if (k.rfind("tag.", 0) == 0) { sources[k.substr(4)] = v; continue; }
                if (k.rfind("sound.", 0) == 0) { c["sound_" + k.substr(6)] = v; continue; }
                if (k == "sprite") continue;   // the runtime reads it from turkeys.json
                c[k] = v;
            }
            for (const auto& t : tags[{"turkeys", name}]) r["tags"].push_back(t);
            c["tag_sources"] = sources;
            const std::string th = thumb_name(name);
            if (fs::exists(dir / th, ec)) { c["thumb"] = th; ++thumbs; }
            m["runes"].push_back(r);
        }
        for (const auto& l : links["turkeys"])
            if (l[2] == "from-game" || l[2] == "kind")
                m["layout"]["edges"].push_back({{"from", l[0]}, {"to", l[1]}, {"relation", l[2]}, {"weight", 1}, {"directed", true}});
        counts["turkeys"] = m["runes"].size();
        counts["thumbnails"] = thumbs;
        mantles.push_back(m);
    }
    // games, kinds, tracks: fields as they are, tags as they are
    for (const auto& [mn, glyph] : std::vector<std::pair<std::string, std::string>>{{"games", "game"}, {"kinds", "kind"}, {"tracks", "track"}}) {
        json m = make_mantle(mn);
        for (const auto& [name, g] : glyph_of[mn]) {
            json r = make_rune(mn, name, glyph);
            for (const auto& [k, v] : fields[{mn, name}]) r["content"][k] = v;
            for (const auto& t : tags[{mn, name}]) r["tags"].push_back(t);
            if (glyph == "track" && !r["content"].contains("title")) {
                const std::string n = fs::path(name.substr(name.find(':') + 1)).stem().string();
                r["content"]["title"] = n;
            }
            m["runes"].push_back(r);
        }
        counts[mn] = m["runes"].size();
        mantles.push_back(m);
    }
    // tables: worlds.json gives the order; the maps mantle gives each one's music
    {
        json m = make_mantle("tables");
        json worlds;
        std::ifstream wf(dir / "worlds.json");
        if (wf) { try { worlds = json::parse(wf); } catch (...) {} }
        int order = 1;
        std::set<std::string> seen;
        auto add_table = [&](const std::string& file, const std::string& title) {
            if (file.empty() || !seen.insert(file).second) return;
            json r = make_rune("tables", file, "table");
            r["content"]["title"] = title.empty() ? file : title;
            r["content"]["file"] = file;
            r["content"]["order"] = order++;
            auto f = fields.find({"maps", file});
            if (f != fields.end() && f->second.count("music")) r["content"]["music"] = f->second.at("music");
            if (!fs::exists(dir / file, ec)) r["tags"].push_back("missing");
            m["runes"].push_back(r);
        };
        if (worlds.is_object() && worlds["worlds"].is_array())
            for (const auto& w : worlds["worlds"]) add_table(w.value("file", ""), w.value("title", ""));
        for (const auto& [name, g] : glyph_of["maps"]) {
            auto f = fields.find({"maps", name});
            add_table(name, f != fields.end() && f->second.count("title") ? f->second.at("title") : name);
        }
        counts["tables"] = m["runes"].size();
        mantles.push_back(m);
    }
    // the feast, if the library has one (the splice keeps the project's own when it exists)
    json feast = {{"player", "smg.Kuribo"}, {"walk", 6.0}, {"run", 9.5}, {"jump", 9.2}, {"gravity", 22.0}, {"lives", 3}, {"hp", 3}};
    {
        std::ifstream ff(dir / "feast.json");
        if (ff) {
            try {
                json f = json::parse(ff);
                for (const char* k : {"player", "walk", "run", "jump", "gravity", "lives", "hp"})
                    if (f.contains(k)) feast[k] = f[k];
            } catch (...) {}
        }
    }
    return {{"library", dir.lexically_normal().string()}, {"mantles", mantles}, {"feast", feast}, {"counts", counts}};
}

// ── the splice (pure) ───────────────────────────────────────────────────────

std::string splice(const std::string& state_json, const json& imported) {
    json st;
    try { st = json::parse(state_json); } catch (...) { return {}; }
    if (!st.is_object()) return {};
    if (!st["mantles"].is_array()) st["mantles"] = json::array();
    json& mantles = st["mantles"];

    auto find_mantle = [&](const std::string& name) -> json* {
        for (auto& m : mantles) if (m.value("name", "") == name) return &m;
        return nullptr;
    };

    for (const json& fresh : imported["mantles"]) {
        const std::string name = fresh.value("name", "");
        json next = fresh;
        if (json* old = find_mantle(name)) {
            next["id"] = (*old)["id"];   // the mantle keeps its identity
            std::map<std::string, const json*> old_runes;
            for (const auto& r : (*old)["runes"]) old_runes[r["spirit"].value("name", "")] = &r;
            std::set<std::string> fresh_names;
            for (auto& r : next["runes"]) {
                const std::string rn = r["spirit"].value("name", "");
                fresh_names.insert(rn);
                auto it = old_runes.find(rn);
                if (it == old_runes.end()) continue;
                const json& o = *it->second;
                r["spirit"]["id"] = o["spirit"]["id"];
                r["placement"] = o["placement"];
                const json& oc = o["content"];
                // the person's fields
                for (auto f = oc.begin(); f != oc.end(); ++f)
                    if (person_key(f.key())) r["content"][f.key()] = f.value();
                if (oc.value("kind_source", "") == "person" && oc.contains("kind")) r["content"]["kind"] = oc["kind"];
                // the person's tags, with their source
                if (oc.contains("tag_sources") && oc["tag_sources"].is_object()) {
                    for (auto t = oc["tag_sources"].begin(); t != oc["tag_sources"].end(); ++t) {
                        if (!t.value().is_string() || t.value().get<std::string>().rfind("person", 0) != 0) continue;
                        r["content"]["tag_sources"][t.key()] = t.value();
                        bool has = false;
                        for (const auto& x : r["tags"]) if (x == t.key()) has = true;
                        if (!has) r["tags"].push_back(t.key());
                    }
                }
                // a tag the person REMOVED stays removed: recorded as "removed by person"
                if (oc.contains("tags_removed") && oc["tags_removed"].is_array()) {
                    r["content"]["tags_removed"] = oc["tags_removed"];
                    json kept = json::array();
                    for (const auto& x : r["tags"]) {
                        bool gone = false;
                        for (const auto& y : oc["tags_removed"]) if (x == y) gone = true;
                        if (!gone) kept.push_back(x);
                    }
                    r["tags"] = kept;
                }
            }
            // a rune the person made in this mantle (not from the library) survives
            for (const auto& [rn, o] : old_runes)
                if (!fresh_names.count(rn) && (*o)["content"].value("made_by", "") == "person") next["runes"].push_back(*o);
            *old = next;
        } else {
            mantles.push_back(next);
        }
    }
    // the feast: the project's own wins; otherwise the library's (or the defaults)
    if (!find_mantle("feast")) {
        json m = make_mantle("feast");
        json r = make_rune("feast", "feast", "feast");
        r["content"] = imported.value("feast", json::object());
        m["runes"].push_back(r);
        mantles.push_back(m);
    }
    if (!st["config"].is_object()) st["config"] = json::object();
    st["config"]["squid.library"] = imported.value("library", "");
    if (!st["active"].is_object()) st["active"] = json::object();
    if (st["active"]["mantle"].is_null() || !st["active"].contains("mantle")) st["active"]["mantle"] = "turkeys";
    return st.dump();
}

json feast_file(const json& c) {
    json f = {{"about", "The feast: who the player is and how they move. Written by Void Squid from the project's feast rune; read by the runtime."}};
    auto num = [&](const char* k, double d) {
        if (!c.contains(k)) return d;
        if (c[k].is_number()) return c[k].get<double>();
        if (c[k].is_string()) { try { return std::stod(c[k].get<std::string>()); } catch (...) {} }
        return d;
    };
    f["player"] = c.value("player", std::string("smg.Kuribo"));
    f["walk"] = num("walk", 6.0);
    f["run"] = num("run", 9.5);
    f["jump"] = num("jump", 9.2);
    f["gravity"] = num("gravity", 22.0);
    f["lives"] = num("lives", 3);
    f["hp"] = num("hp", 3);
    return f;
}

json worlds_file(const json& tables) {
    std::vector<std::pair<double, json>> rows;
    for (const auto& r : tables["runes"]) {
        const json& c = r["content"];
        double o = 999;
        if (c.contains("order")) {
            if (c["order"].is_number()) o = c["order"].get<double>();
            else if (c["order"].is_string()) { try { o = std::stod(c["order"].get<std::string>()); } catch (...) {} }
        }
        rows.push_back({o, json{{"title", c.value("title", "")}, {"file", c.value("file", "")}}});
    }
    std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    json w = {{"worlds", json::array()}};
    for (auto& r : rows) w["worlds"].push_back(r.second);
    return w;
}

}  // namespace squid::library
