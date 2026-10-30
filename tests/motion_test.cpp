/* motion_test.cpp — the runtime's motion verdicts, counted, without a file or a window.
 *
 * What it pins: the summary counts clips and keeps the cast's order; the mean and the lowest
 * are the clips' own overall rates, unchanged; a check is weak below 80% and not at it (the
 * runtime's threshold); a check with no trials is never weak; "(as ...)" survives as
 * used_verb only when it differs; rows without a character are skipped and counted; pace pools
 * passed/trials per rule and speed in the order measured; a missing pace is an empty list.
 *
 * The rows are made up in the runtime's shape (src/g2/game.cpp's judge), never a game's.
 */
#include "motion/judge.hpp"

#include <cmath>
#include <cstdio>

using nlohmann::json;
using namespace squid::motion;

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } \
    } while (0)

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

static json clip(const char* who, const char* verb, const char* name, const char* used, double rate, json checks) {
    return {{"character", who}, {"verb", verb}, {"clip", name}, {"used_verb", used}, {"confidence", "watched"},
            {"rate", rate}, {"lo", rate - 0.05}, {"hi", rate}, {"checks", checks}};
}

static json check(const char* name, double passed, double trials) {
    return {{"check", name}, {"passed", passed}, {"trials", trials}, {"note", "a note"}};
}

static void counts() {
    const json judge = json::array({
        clip("hero", "locomotion.walk", "walk", "locomotion.walk", 1.0, json::array({check("present", 60, 60)})),
        clip("hero", "air.fall", "jmped", "air.land", 0.9,
             json::array({check("no pops", 5, 10), check("colour held", 8, 10), check("repeats", 0, 0)})),
        json{{"verb", "idle"}, {"rate", 0.1}},   // no character: skipped
        clip("kart", "drive", "drive", "drive", 0.95, json::array({check("drawn as predicted", 70, 100)})),
    });
    const json s = summarize(judge, nullptr);
    CHECK(s["clips"] == 3);
    CHECK(s["skipped"] == 1);
    CHECK(near(s["mean"].get<double>(), (1.0 + 0.9 + 0.95) / 3));
    CHECK(s["lowest"]["character"] == "hero" && s["lowest"]["clip"] == "jmped");
    CHECK(s["weak"] == 2);   // 5/10 and 70/100; 8/10 is the threshold itself, 0/0 has no trials

    const json& chars = s["characters"];
    CHECK(chars.size() == 2 && chars[0]["character"] == "hero" && chars[1]["character"] == "kart");
    if (chars.size() == 2) {
        CHECK(chars[0]["clips"] == 2);
        CHECK(near(chars[0]["mean"].get<double>(), 0.95));
        CHECK(chars[0]["lowest"]["verb"] == "air.fall");
        CHECK(chars[0]["weak"].size() == 1 && chars[0]["weak"][0]["check"] == "no pops");
        CHECK(chars[0]["verbs"][1]["used_verb"] == "air.land");          // the runtime's "(as ...)"
        CHECK(!chars[0]["verbs"][0].contains("used_verb"));              // the same verb: not repeated
        CHECK(chars[0]["verbs"][0]["confidence"] == "watched");
        CHECK(near(chars[1]["lowest"]["lo"].get<double>(), 0.90));       // Onda's interval, passed through
    }
    CHECK(s["pace"].is_array() && s["pace"].empty());
}

static void pace() {
    auto row = [](const char* who, double speed, const char* rule, double passed, double trials) {
        return json{{"character", who}, {"speed", speed}, {"rule", rule}, {"clip", "run"}, {"rate", 1.5},
                    {"passed", passed}, {"trials", trials}, {"note", ""}};
    };
    const json p = json::array({row("hero", 3, "before", 5, 10), row("hero", 3, "nearest pace", 8, 10),
                                row("kart", 3, "before", 1, 10), row("kart", 3, "nearest pace", 6, 10),
                                row("hero", 6, "nearest pace", 7, 10)});
    const json s = summarize(json::array(), p);
    CHECK(s["clips"] == 0 && near(s["mean"].get<double>(), 0) && s["lowest"].is_null());
    const json& pooled = s["pace"];
    CHECK(pooled.size() == 3);
    if (pooled.size() == 3) {
        CHECK(pooled[0]["rule"] == "before" && near(pooled[0]["speed"].get<double>(), 3));
        CHECK(near(pooled[0]["rate"].get<double>(), 0.3) && pooled[0]["characters"] == 2);
        CHECK(pooled[1]["rule"] == "nearest pace" && near(pooled[1]["rate"].get<double>(), 0.7));
        CHECK(near(pooled[2]["speed"].get<double>(), 6) && pooled[2]["characters"] == 1);
        CHECK(!pooled[0].contains("lo"));   // no interval: that is Onda's to compute
    }
}

static void not_a_report() {
    const json s = summarize(json::object({{"oops", 1}}), json("text"));
    CHECK(s["clips"] == 0 && s["characters"].empty() && s["pace"].empty());
    CHECK(s["method"].is_string());
}

int main() {
    counts();
    pace();
    not_a_report();
    if (failures) {
        std::printf("motion: %d check(s) failed\n", failures);
        return 1;
    }
    std::printf("motion: all checks passed\n");
    return 0;
}
