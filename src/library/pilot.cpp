/* library/pilot.cpp — see pilot.hpp. Everything here is pure except the model, which arrives
 * as a function. */
#include "library/pilot.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <set>

namespace squid::library {

using json = nlohmann::json;

namespace {

std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

/* "KinokoOneUp" -> {"kinoko", "one", "up"}; "1-Up Mushroom" -> {"1", "up", "mushroom"}. */
std::vector<std::string> words(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    auto flush = [&] { if (!cur.empty()) { out.push_back(lower(cur)); cur.clear(); } };
    for (size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (!std::isalnum(static_cast<unsigned char>(c))) { flush(); continue; }
        if (std::isupper(static_cast<unsigned char>(c)) && !cur.empty() && std::islower(static_cast<unsigned char>(cur.back()))) flush();
        if (std::isdigit(static_cast<unsigned char>(c)) != (cur.empty() ? 0 : std::isdigit(static_cast<unsigned char>(cur.back())))) flush();
        cur += c;
    }
    flush();
    return out;
}

/* A rule, not intelligence: Nintendo's internal words and the English the wikis use. Small on
 * purpose; every entry is checkable against a game's own files and its wiki. */
const std::map<std::string, std::vector<std::string>>& glossary() {
    static const std::map<std::string, std::vector<std::string>> g = {
        {"kinoko", {"mushroom"}}, {"kuribo", {"goomba"}}, {"nokonoko", {"koopa", "troopa"}}, {"packun", {"piranha", "plant"}},
        {"pakkun", {"piranha", "plant"}}, {"teresa", {"boo"}}, {"bomhei", {"bob", "omb"}}, {"wanwan", {"chain", "chomp"}},
        {"pukupuku", {"cheep"}}, {"hatena", {"question"}}, {"piece", {"bit"}}, {"oneup", {"1", "up"}}, {"one", {"1"}},
        {"takobo", {"octoomba"}}, {"tico", {"luma"}}, {"chiko", {"luma"}}, {"karon", {"dry", "bones"}}, {"heyho", {"shy", "guy"}},
        {"killer", {"bullet", "bill"}}, {"dossun", {"thwomp"}}, {"jugem", {"lakitu"}}, {"togezo", {"spiny"}},
        {"earthen", {"warp"}}, {"dokan", {"warp", "pipe"}}, {"driver", {"star"}}, {"spin", {"sling", "launch"}},
        {"koura", {"shell"}}, {"coin", {"coin"}}, {"rosetta", {"rosalina"}}, {"kinopio", {"toad"}}, {"koopa", {"bowser"}},
    };
    return g;
}

std::set<std::string> turkey_words(const PilotTurkey& t) {
    std::set<std::string> w;
    for (const auto& s : {t.name, t.cls}) {
        for (const auto& x : words(s)) {
            w.insert(x);
            auto it = glossary().find(x);
            if (it != glossary().end()) for (const auto& e : it->second) w.insert(e);
        }
        // the whole name too ("starpiece", "oneup"), for glossary entries written joined
        std::string joined;
        for (const auto& x : words(s)) joined += x;
        auto it = glossary().find(joined);
        if (it != glossary().end()) for (const auto& e : it->second) w.insert(e);
    }
    return w;
}

std::string singular(std::string w) {
    if (w.size() > 3 && w.back() == 's' && w[w.size() - 2] != 's') w.pop_back();
    return w;
}

}  // namespace

bool kind_fits(const std::string& kind, const std::vector<std::string>& sections) {
    if (sections.empty() || kind.empty() || kind == "deco") return true;
    std::set<std::string> want;
    if (kind == "enemy") want = {"enemy", "boss", "obstacle", "character"};
    else if (kind == "coin" || kind == "starbit" || kind == "oneup" || kind == "powerstar" || kind == "itembox") want = {"item", "power-up", "object", "collectible"};
    else if (kind == "npc" || kind == "player") want = {"character", "enemy", "boss"};
    else if (kind == "planet") want = {"location", "level", "object", "boss"};
    else if (kind == "launch" || kind == "sling") want = {"object", "item"};
    else return true;
    for (const auto& s : sections) if (want.count(s) || s.rfind("wikipedia:", 0) == 0) return true;
    return false;
}

std::vector<std::pair<std::string, double>> shortlist(const PilotTurkey& t, const std::vector<PilotEntity>& ents, size_t n) {
    const auto tw = turkey_words(t);
    std::vector<std::pair<std::string, double>> scored;
    for (const auto& e : ents) {
        if (std::find(e.games.begin(), e.games.end(), t.game) == e.games.end()) continue;
        double s = 0;
        for (const auto& w : words(e.name)) {
            const std::string sw = singular(w);
            if (tw.count(w) || tw.count(sw)) s += 3.0;
        }
        if (kind_fits(t.kind, e.sections)) s += 1.0;
        s += 0.15 * std::log1p(e.mentions);   // a central page is a likelier answer than an obscure one
        scored.push_back({e.name, s});
    }
    std::stable_sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    if (scored.size() > n) scored.resize(n);
    return scored;
}

json propose(const PilotTurkey& t, const std::vector<PilotEntity>& ents, const AskModel& ask, const std::vector<std::string>& taken) {
    const auto sl = shortlist(t, ents, 12);
    json out = {{"turkey", t.id}, {"name", t.name}};
    json slj = json::array();
    std::vector<std::string> names;
    for (const auto& [nm, sc] : sl) { slj.push_back({{"name", nm}, {"score", sc}}); names.push_back(nm); }
    out["shortlist"] = slj;
    out["baseline"] = names.empty() ? json(nullptr) : json(names[0]);
    if (names.empty() || !ask) { out["proposal"] = nullptr; return out; }

    std::map<std::string, const PilotEntity*> by_name;
    for (const auto& e : ents) by_name[e.name] = &e;
    auto prompt_for = [&](const std::vector<std::string>& order) {
        std::string p = "A thing was recovered from the files of the video game " + t.game + ". Its internal name is \"" + t.name + "\"";
        if (!t.cls.empty() && t.cls != t.name) p += " (class " + t.cls + ")";
        if (!t.category.empty()) p += ", the game files file it under \"" + t.category + "\"";
        p += ", and the game uses it as: " + t.kind + ".\nWhich page of the game's wiki is about this thing? Candidates:\n";
        for (const auto& nm : order) {
            const auto* e = by_name[nm];
            std::string d = e ? e->description.substr(0, 110) : "";
            p += "- " + nm + (d.empty() ? "" : ": " + d) + "\n";
        }
        p += "Answer with the candidate's exact name, or \"none\" if none of them is this thing.";
        return p;
    };
    json enumv = json::array();
    for (const auto& nm : names) enumv.push_back(nm);
    enumv.push_back("none");
    const json schema = {{"type", "object"},
                         {"properties", {{"match", {{"type", "string"}, {"enum", enumv}}}, {"confidence", {{"type", "number"}}}}},
                         {"required", {"match"}}};
    std::vector<std::string> rev(names.rbegin(), names.rend());
    const json a = ask(prompt_for(names), schema);
    const json b = ask(prompt_for(rev), schema);
    const std::string ma = a.is_object() ? a.value("match", std::string()) : std::string();
    const std::string mb = b.is_object() ? b.value("match", std::string()) : std::string();
    out["answers"] = {ma, mb};
    if (a.contains("error")) out["error"] = a["error"];
    const bool agreed = !ma.empty() && ma == mb;
    const bool in_list = std::find(names.begin(), names.end(), ma) != names.end();
    const bool fits = in_list && by_name.count(ma) && kind_fits(t.kind, by_name[ma]->sections);
    const bool free = std::find(taken.begin(), taken.end(), ma) == taken.end();
    // where a rule is certain, it outranks the model: a candidate whose name IS the thing's own
    // name (brackets aside) is the answer, and a model that picks another is overruled
    auto fold = [](std::string x) {
        const auto b = x.find(" (");
        if (b != std::string::npos) x = x.substr(0, b);
        std::string o;
        for (char c : x) if (std::isalnum(static_cast<unsigned char>(c))) o += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return o;
    };
    std::string exact;
    for (const auto& nm : names) if (fold(nm) == fold(t.name) && (exact.empty() || nm.size() < exact.size())) exact = nm;
    const bool lexical = exact.empty() || ma == exact;
    out["guards"] = {{"agreed", agreed}, {"in_shortlist", in_list}, {"fits", fits}, {"free", free}, {"exact_name_respected", lexical}};
    out["proposal"] = (agreed && in_list && fits && free && lexical) ? json(ma) : json(nullptr);
    return out;
}

json pilot_test(const std::vector<PilotTurkey>& turkeys, const std::vector<PilotEntity>& ents, const AskModel& ask,
                const std::string& model, size_t limit) {
    json results = json::array();
    int cases = 0, correct = 0, baseline = 0, recall = 0, abstained = 0, wrong = 0;
    for (const auto& t : turkeys) {
        if (t.known.empty() || static_cast<size_t>(cases) >= limit) continue;
        // only cases the wiki can answer: the known page is one of this game's entities
        bool answerable = false;
        for (const auto& e : ents)
            if (e.name == t.known && std::find(e.games.begin(), e.games.end(), t.game) != e.games.end()) answerable = true;
        if (!answerable) continue;
        ++cases;
        PilotTurkey hidden = t;
        hidden.known.clear();
        json r = propose(hidden, ents, ask, {});
        bool in_sl = false;
        for (const auto& s : r["shortlist"]) if (s.value("name", "") == t.known) in_sl = true;
        if (in_sl) ++recall;
        if (r["baseline"].is_string() && r["baseline"].get<std::string>() == t.known) ++baseline;
        if (r["proposal"].is_string()) {
            if (r["proposal"].get<std::string>() == t.known) ++correct; else ++wrong;
        } else ++abstained;
        r["truth"] = t.known;
        results.push_back(r);
    }
    return {{"model", model}, {"cases", cases}, {"model_correct", correct}, {"baseline_correct", baseline},
            {"shortlist_recall", recall}, {"abstained", abstained}, {"wrong_accepted", wrong}, {"results", results},
            {"method", "Each case is a turkey whose wiki page is already known (matched by name or Japanese romanization). The "
                       "match is hidden; the model must pick it from a 12-name shortlist twice, in two orders. Accepted only when "
                       "both answers agree, the answer fits the turkey's kind, and no other turkey holds it. The baseline is the "
                       "shortlist's first name, with no model."}};
}

}  // namespace squid::library
