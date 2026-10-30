/* molerat_test.cpp — the pure slice, with no filesystem, network, display or clock.
 *
 * What it checks, in the order each would be most expensive to be wrong:
 *   1. The IR parses, and a bad schema is refused.
 *   2. The analysis orders the call graph and records reads/writes/calls.
 *   3. The research produces all four record kinds, and every function is covered.
 *   4. Compose maps to two mantles, leaves no scaffolding, and resolves every link.
 *   5. The emitted state document validates.
 *   6. The same seed is deterministic; a different seed produces different ids.
 *   7. Tampering (duplicate name, missing quantity, dangling link) is caught.
 *
 * No framework; plain checks, like the rest of the family.
 */
#include "molerat/molerat.hpp"

#include "json.hpp"

#include <cstdio>
#include <set>
#include <string>

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) { ++failures; std::printf("  FAIL  %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

static bool contains(const std::string& h, const std::string& n) {
    return h.find(n) != std::string::npos;
}

static const char* kIR = R"IR({
  "schema": "molerat.foreign-ir/0.1",
  "target": {"id":"toy","title":"Toy","platform":"synthetic","origin":"authored test fixture","depth":"source"},
  "functions": [
    {"name":"update","addr":"0x100","blocks":[{"id":"e","ops":[
        {"kind":"load","src":"stats.gravity"},
        {"kind":"add","src":"stats.gravity"},
        {"kind":"store","dst":"stats.vy"},
        {"kind":"call","value":"fall"}]}],
     "calls":["fall"],"reads":["stats.gravity"],"writes":["stats.vy"],
     "evidence":["writes vy"]},
    {"name":"fall","addr":"0x180","blocks":[{"id":"e","ops":[
        {"kind":"load","src":"stats.gravity"},
        {"kind":"store","dst":"stats.vy"}]}],
     "calls":[],"reads":["stats.gravity"],"writes":["stats.vy"],
     "evidence":["writes vy"]},
    {"name":"door_check","addr":"0x200","blocks":[{"id":"e","ops":[
        {"kind":"load","src":"doors.count"},
        {"kind":"cmp","value":">0"},
        {"kind":"branch","dst":"open"}]}],
     "calls":[],"reads":["doors.count"],"writes":[],
     "evidence":["branches on a count"]}
  ],
  "tables": [
    {"name":"stats","fields":[
        {"name":"gravity","type":"f32","unit":"m/s^2","value":-9.8},
        {"name":"vy","type":"f32","unit":"m/s","value":0.0},
        {"name":"move_speed","type":"f32","unit":"m/s","value":6.0}]},
    {"name":"doors","fields":[
        {"name":"count","type":"u8","unit":"count","value":1}]}
  ],
  "constants":["0"],
  "residual":["tick rate is not in the IR"]
})IR";

int main() {
    using namespace squid::molerat;

    /* 1. Parse and refuse. */
    ForeignIR ir;
    std::string error;
    CHECK(parse_ir(kIR, ir, error));
    CHECK(ir.target.id == "toy");
    CHECK(ir.functions.size() == 3);
    CHECK(ir.tables.size() == 2);
    {
        ForeignIR bad;
        CHECK(!parse_ir("{\"schema\":\"other/1\",\"target\":{\"id\":\"x\"},\"functions\":[]}", bad, error));
        CHECK(contains(error, "schema"));
        CHECK(!parse_ir("not json", bad, error));
    }

    /* 2. Analysis. */
    const Analysis analysis = analyze(ir);
    CHECK(analysis.order.size() == 3);
    CHECK(!analysis.order.empty() && analysis.order[0] == "fall");   // post-order
    CHECK(analysis.order.size() > 1 && analysis.order[1] == "update");
    CHECK(analysis.reads.at("update").size() == 1);
    CHECK(analysis.writes.at("fall").size() == 1);
    CHECK(analysis.calls.at("update").size() == 1);
    CHECK(analysis.findings.size() == 3);

    /* 3. Research. */
    const Research records = squid::molerat::research(ir, analysis);
    CHECK(records.models.size() == 5);        // 3 functions + 2 tables
    CHECK(records.mappings.size() == 7);      // 3 functions + 4 fields
    CHECK(records.residuals.size() >= 6);
    CHECK(records.holidays.size() == 3);
    for (const Function& f : ir.functions) {
        bool model = false;
        bool mapping = false;
        for (const ModelRecord& m : records.models) if (m.source == f.name) model = true;
        for (const MappingRecord& m : records.mappings) if (m.source_construct == f.name) mapping = true;
        CHECK(model);
        CHECK(mapping);
    }
    CHECK(contains(residual_json(ir, records), "\"count\":"));
    CHECK(contains(research_markdown(ir, records, "toy"), "# MoleRat research"));
    CHECK(contains(research_markdown(ir, records, "toy"),
                   "[learned-decompilers](/sources/learned-decompilers.md)"));
    CHECK(contains(research_markdown(ir, records, "toy"),
                   "`VoidCore:/sources/lafont-interaction-nets.md`"));

    /* The host cache round-trips. */
    {
        Research back;
        CHECK(research_from_json(research_to_json(records), back, error));
        CHECK(back.models.size() == records.models.size());
        CHECK(back.mappings.size() == records.mappings.size());
        CHECK(back.residuals.size() == records.residuals.size());
        CHECK(back.holidays.size() == records.holidays.size());
    }

    /* 4. Compose. */
    const ComposeResult composed = compose(ir, records, "toy", "seed-1");
    CHECK(composed.ok);
    CHECK(composed.game.name == "toy:game");
    CHECK(composed.research.name == "molerat:toy");
    CHECK(!composed.scaffolding.empty());

    std::set<std::string> game_names;
    for (const RuneSpec& r : composed.game.runes) game_names.insert(r.name);
    std::set<std::string> research_names;
    for (const RuneSpec& r : composed.research.runes) research_names.insert(r.name);
    CHECK(game_names.count("player") == 1);
    CHECK(game_names.count("door") == 1);
    CHECK(game_names.count("gravity") == 1);
    CHECK(game_names.count("move_speed") == 1);

    /* No scaffolding survives the normal form. */
    for (const std::string& s : composed.scaffolding) {
        CHECK(game_names.count(s) == 0);
        CHECK(research_names.count(s) == 0);
    }

    /* Every link resolves within its own mantle (v1 links are rune-to-rune). */
    for (const LinkSpec& l : composed.game.links) {
        CHECK(game_names.count(l.from) == 1);
        CHECK(game_names.count(l.to) == 1);
    }
    for (const LinkSpec& l : composed.research.links) {
        CHECK(research_names.count(l.from) == 1);
        CHECK(research_names.count(l.to) == 1);
    }

    /* The attribute assertion is a measure edge: player --speed 6--> move_speed. */
    bool speed_edge = false;
    for (const LinkSpec& l : composed.game.links) {
        if (l.from == "player" && l.to == "move_speed" && l.relation == "speed") {
            speed_edge = true;
            CHECK(l.weight > 5.9 && l.weight < 6.1);
        }
    }
    CHECK(speed_edge);

    /* 5. Emit and validate. */
    const std::string document = emit_document(composed, ir, "toy", "seed-1", error);
    CHECK(!document.empty());
    const Validation validation = validate_document(document);
    CHECK(validation.ok);
    CHECK(contains(document, "\"config\""));
    CHECK(contains(document, "\"reduce\""));
    CHECK(contains(document, "\"molerat.model\""));

    /* 6. Determinism. */
    {
        Slice a;
        Slice b;
        CHECK(run_slice(kIR, "toy", "seed-1", a, error));
        CHECK(run_slice(kIR, "toy", "seed-1", b, error));
        CHECK(a.document == b.document);
        Slice c;
        CHECK(run_slice(kIR, "toy", "seed-2", c, error));
        CHECK(a.document != c.document);
        CHECK(!a.residual.empty());
    }

    /* 7. Tampering is caught. */
    {
        nlohmann::json j = nlohmann::json::parse(document);
        j["mantles"][0]["runes"][1]["spirit"]["name"] =
            j["mantles"][0]["runes"][0]["spirit"]["name"];
        CHECK(!validate_document(j.dump()).ok);
    }
    {
        nlohmann::json j = nlohmann::json::parse(document);
        for (auto& r : j["mantles"][0]["runes"]) {
            if (r["glyph"] == "game.measure") { r.erase("quantity"); break; }
        }
        CHECK(!validate_document(j.dump()).ok);
    }
    {
        nlohmann::json j = nlohmann::json::parse(document);
        j["mantles"][0]["layout"]["edges"][0]["to"] = "missing";
        CHECK(!validate_document(j.dump()).ok);
    }
    {
        nlohmann::json j = nlohmann::json::parse(document);
        j["mantles"][0]["runes"][0]["glyph"] = "undeclared.glyph";
        CHECK(!validate_document(j.dump()).ok);
    }

    if (failures == 0) {
        std::printf("ok    squid molerat: pure slice\n");
        return 0;
    }
    std::printf("FAIL  squid molerat (%d)\n", failures);
    return 1;
}
