/* motion/judge.cpp — see judge.hpp. */
#include "motion/judge.hpp"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace squid::motion {

using json = nlohmann::json;

namespace {

double num(const json& row, const char* key) {
    const auto it = row.find(key);
    return it != row.end() && it->is_number() ? it->get<double>() : 0.0;
}

std::string text(const json& row, const char* key) {
    const auto it = row.find(key);
    return it != row.end() && it->is_string() ? it->get<std::string>() : std::string();
}

/* The clip as the studio shows it: what was asked, what played, and Onda's verdict. */
json clip_of(const json& row) {
    json c = {{"verb", text(row, "verb")}, {"clip", text(row, "clip")}, {"rate", num(row, "rate")},
              {"lo", num(row, "lo")}, {"hi", num(row, "hi")}};
    const std::string used = text(row, "used_verb");
    if (!used.empty() && used != text(row, "verb")) c["used_verb"] = used;   // the runtime prints "(as ...)"
    if (const std::string conf = text(row, "confidence"); !conf.empty()) c["confidence"] = conf;
    return c;
}

}  // namespace

json summarize(const json& judge, const json& pace) {
    json out = json::object();
    std::vector<std::string> order;                 // the runtime's cast order, as it judged them
    std::map<std::string, json> by_char;
    size_t clips = 0, skipped = 0, weak = 0;
    double sum = 0;
    json lowest = nullptr;

    for (const json& row : judge.is_array() ? judge : json::array()) {
        if (!row.is_object() || text(row, "character").empty()) { ++skipped; continue; }
        const std::string who = text(row, "character");
        auto [it, fresh] = by_char.try_emplace(who, json{{"character", who}, {"clips", 0}, {"mean", 0.0},
                                                         {"lowest", nullptr}, {"verbs", json::array()},
                                                         {"weak", json::array()}});
        if (fresh) order.push_back(who);
        json& ch = it->second;
        const json c = clip_of(row);
        const double rate = c["rate"].get<double>();
        ch["verbs"].push_back(c);
        ch["clips"] = ch["clips"].get<int>() + 1;
        ch["mean"] = ch["mean"].get<double>() + rate;   // a sum until the end
        if (ch["lowest"].is_null() || rate < ch["lowest"]["rate"].get<double>()) ch["lowest"] = c;
        if (lowest.is_null() || rate < lowest["rate"].get<double>()) { lowest = c; lowest["character"] = who; }
        for (const json& chk : row.contains("checks") && row["checks"].is_array() ? row["checks"] : json::array()) {
            const double trials = num(chk, "trials"), passed = num(chk, "passed");
            if (trials <= 0 || passed / trials >= kWeak) continue;
            ch["weak"].push_back({{"verb", c["verb"]}, {"clip", c["clip"]}, {"check", text(chk, "check")},
                                  {"passed", passed}, {"trials", trials}, {"note", text(chk, "note")}});
            ++weak;
        }
        ++clips;
        sum += rate;
    }

    json characters = json::array();
    for (const std::string& who : order) {
        json& ch = by_char[who];
        ch["mean"] = ch["mean"].get<double>() / ch["clips"].get<int>();
        characters.push_back(std::move(ch));
    }

    // feet on a treadmill, pooled per (rule, speed) in the order the runtime measured them
    json pooled = json::array();
    std::vector<std::pair<std::string, double>> keys;
    std::map<std::pair<std::string, double>, json> by_key;
    for (const json& row : pace.is_array() ? pace : json::array()) {
        if (!row.is_object() || text(row, "character").empty()) continue;
        const auto key = std::make_pair(text(row, "rule"), num(row, "speed"));
        auto [it, fresh] = by_key.try_emplace(key, json{{"rule", key.first}, {"speed", key.second}, {"passed", 0.0},
                                                        {"trials", 0.0}, {"characters", 0}});
        if (fresh) keys.push_back(key);
        json& p = it->second;
        p["passed"] = p["passed"].get<double>() + num(row, "passed");
        p["trials"] = p["trials"].get<double>() + num(row, "trials");
        p["characters"] = p["characters"].get<int>() + 1;
    }
    for (const auto& k : keys) {
        json& p = by_key[k];
        const double t = p["trials"].get<double>();
        p["rate"] = t > 0 ? p["passed"].get<double>() / t : 0.0;
        pooled.push_back(std::move(p));
    }

    out["clips"] = clips;
    out["mean"] = clips ? sum / static_cast<double>(clips) : 0.0;
    out["lowest"] = lowest;
    out["weak"] = weak;
    out["skipped"] = skipped;
    out["characters"] = std::move(characters);
    out["pace"] = std::move(pooled);
    out["method"] =
        "Read from the runtime's judge.json and pace.json. Every verdict is Void Onda's, computed by the runtime "
        "from pixels; this only counts them: the mean and lowest of the clips' overall rates, the checks under 80% "
        "(the runtime's 'weak'), and pace pooled as passed/trials per rule and speed, with no interval.";
    return out;
}

}  // namespace squid::motion
