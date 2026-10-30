/* emit.cpp — re-author the research into two mantles and a valid state document.
 *
 * The normal form here is the one the carried Reduce ruleset describes: scaffolding
 * (molerat.call, molerat.sequence, molerat.measure_seed) is eliminated, and a measure
 * seed's value is patched onto the surviving game.measure. The pure library computes
 * that normal form directly; the ruleset travels in config.transform.reduce so the
 * Python reduce seam can re-derive the same shape.
 */
#include "molerat/molerat.hpp"

#include "json.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <map>
#include <set>
#include <sstream>

namespace squid::molerat {
namespace {

using json = nlohmann::json;

uint64_t fnv1a64(std::string_view s) {
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

std::string hex12(uint64_t v) {
    char buf[13];
    std::snprintf(buf, sizeof buf, "%012llx",
                  static_cast<unsigned long long>(v & 0xffffffffffffull));
    return std::string(buf);
}

std::string rune_id(const std::string& seed, const std::string& mantle,
                    const std::string& name) {
    return "rune_" + hex12(fnv1a64(seed + "|" + mantle + "|" + name));
}

std::string mantle_id(const std::string& seed, const std::string& name) {
    return "mantle_" + hex12(fnv1a64(seed + "|mantle|" + name));
}

std::string sanitize(std::string s) {
    for (char& c : s) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') c = '_';
    }
    if (s.empty()) s = "rune";
    return s;
}

json jstrs(const std::vector<std::string>& v) {
    json a = json::array();
    for (const std::string& s : v) a.push_back(s);
    return a;
}

json parse_or_object(const std::string& text) {
    if (text.empty()) return json::object();
    try {
        json j = json::parse(text);
        return j.is_object() ? j : json::object();
    } catch (...) {
        return json::object();
    }
}

std::string mapping_shape(const Research& research, const std::string& construct) {
    for (const MappingRecord& m : research.mappings) {
        if (m.source_construct == construct) return m.void_shape;
    }
    return {};
}

RuneSpec make_rune(const std::string& name, const std::string& glyph,
                   const json& content, const std::string& confidence,
                   const std::string& depth, const std::string& what,
                   const std::string& why, const std::string& how,
                   const std::string& run_id, const std::string& where) {
    RuneSpec r;
    r.name = name;
    r.glyph = glyph;
    r.who = "molerat";
    r.what = what;
    r.when = "2026-10-18T00:00:00Z";
    r.where = where;
    r.why = why;
    r.how = how;
    r.tags = {"molerat:run/" + run_id, "confidence:" + confidence, "depth:" + depth};
    r.content_json = content.dump();
    return r;
}

json reduce_spec() {
    json spec;
    spec["signatures"] = json::object();
    spec["signatures"]["molerat.call"] = 2;
    spec["signatures"]["molerat.sequence"] = 2;
    spec["signatures"]["molerat.measure_seed"] = 1;
    spec["signatures"]["game.thing"] = 1;
    spec["signatures"]["game.act"] = 2;
    spec["signatures"]["game.measure"] = 1;
    spec["rules"] = json::array();
    spec["rules"].push_back(json{{"glyphs", json::array({"molerat.call", "molerat.sequence"})},
                                 {"rule", "annihilate"}});
    spec["rules"].push_back(json{{"glyphs", json::array({"molerat.measure_seed", "game.measure"})},
                                 {"rule", "patch"}, {"keep", "game.measure"},
                                 {"copy", json{{"value", "value"}}},
                                 {"set", json{{"mapped", true}}}});
    return spec;
}

json mantle_json(const MantleSpec& mantle, const std::string& seed) {
    json m;
    m["id"] = mantle_id(seed, mantle.name);
    m["name"] = mantle.name;
    m["domain"] = mantle.domain.empty() ? json(nullptr) : json(mantle.domain);
    m["runes"] = json::array();
    for (const RuneSpec& r : mantle.runes) {
        json rj;
        rj["spirit"] = json{{"id", r.id.empty() ? rune_id(seed, mantle.name, r.name) : r.id},
                            {"name", r.name}};
        rj["glyph"] = r.glyph;
        rj["facets"] = json{{"who", r.who}, {"what", r.what}, {"when", r.when},
                            {"where", r.where}, {"why", r.why}, {"how", r.how}};
        rj["tags"] = jstrs(r.tags);
        rj["content"] = parse_or_object(r.content_json);
        rj["placement"] = nullptr;
        rj["relations"] = json::array();
        if (!r.quantity_json.empty()) {
            try {
                rj["quantity"] = json::parse(r.quantity_json);
            } catch (...) {
                // A malformed quantity is omitted; validate_document will report the
                // measure rune that lacks one.
            }
        }
        m["runes"].push_back(std::move(rj));
    }
    m["tags"] = json::object();
    m["layout"] = json::object();
    m["layout"]["edges"] = json::array();
    for (const LinkSpec& l : mantle.links) {
        m["layout"]["edges"].push_back(json{{"from", l.from}, {"to", l.to},
                                            {"relation", l.relation}, {"weight", l.weight},
                                            {"directed", l.directed}});
    }
    m["rules"] = json::array();
    return m;
}

}  // namespace

ComposeResult compose(const ForeignIR& ir, const Research& research,
                      const std::string& run_id, const std::string& seed) {
    (void)seed;
    ComposeResult out;
    out.game.name = run_id + ":game";
    out.game.domain = run_id;
    out.research.name = "molerat:" + run_id;
    out.research.domain = run_id;

    std::set<std::string> used;
    auto unique = [&](const std::string& raw) {
        const std::string base = sanitize(raw);
        std::string candidate = base;
        int k = 2;
        while (used.count(candidate)) candidate = base + "_" + std::to_string(k++);
        used.insert(candidate);
        return candidate;
    };

    /* The game mantle: the re-authored shape, with no scaffolding. */
    out.game.runes.push_back(make_rune(
        unique("player"), "game.thing", json{{"summary", "the avatar"}}, "weak", "structure",
        "the player's body", "the input holiday controls it",
        "mapped from the IR's player state", run_id, ir.target.origin));

    std::map<std::string, std::string> fn_rune;
    for (const Function& f : ir.functions) {
        std::string shape = mapping_shape(research, f.name);
        if (shape.empty()) shape = f.writes.empty() ? "game.thing" : "game.act";
        const std::string name = unique(f.name);
        json content;
        content["source_function"] = f.name;
        content["reads"] = jstrs(f.reads);
        content["writes"] = jstrs(f.writes);
        content["calls"] = jstrs(f.calls);
        out.game.runes.push_back(make_rune(
            name, shape, content, "weak", "structure", "recovered from " + f.name,
            "the IR's dataflow", "mapped from the foreign IR", run_id, ir.target.origin));
        fn_rune[f.name] = name;
    }

    /* A predicate names the thing it guards: door_check yields a door. */
    for (const Function& f : ir.functions) {
        if (f.name.size() <= 6 || f.name.compare(f.name.size() - 6, 6, "_check") != 0) continue;
        const std::string base = f.name.substr(0, f.name.size() - 6);
        if (base.empty()) continue;
        const std::string name = unique(base);
        out.game.runes.push_back(make_rune(
            name, "game.thing", json{{"guards", f.name}}, "weak", "structure",
            "the thing " + f.name + " guards", "a predicate names it",
            "inferred from the function name", run_id, ir.target.origin));
        if (fn_rune.count(f.name)) {
            out.game.links.push_back({name, fn_rune[f.name], "guards", 1.0, true});
        }
        out.game.links.push_back({name, "player", "blocks", 1.0, true});
    }

    /* Table fields with a value or a unit become measure runes with quantities. */
    std::map<std::string, std::string> field_rune;
    std::map<std::string, double> field_value;
    for (const Table& t : ir.tables) {
        for (const Field& fld : t.fields) {
            if (!fld.has_value && fld.unit.empty()) continue;
            const std::string name = unique(fld.name);
            const std::string source = t.name + "." + fld.name;
            json content;
            content["value"] = fld.value;
            content["source"] = source;
            RuneSpec r = make_rune(
                name, "game.measure", content, "weak", "structure",
                "the measure " + fld.name, "the table declares it",
                "mapped from " + source, run_id, ir.target.origin);
            json q;
            q["level"] = "ratio";
            q["unit"] = fld.unit.empty() ? "count" : fld.unit;
            if (fld.has_value) {
                q["min"] = std::min(0.0, fld.value * 2.0);
                q["max"] = std::max(0.0, fld.value * 2.0);
            }
            r.quantity_json = q.dump();
            out.game.runes.push_back(std::move(r));
            field_rune[source] = name;
            if (fld.has_value) field_value[source] = fld.value;
        }
    }

    /* Links: who performs, who calls, who uses a measure, and one attribute
     * assertion from the player to a speed-like measure. */
    std::string first_act;
    for (const Function& f : ir.functions) {
        if (fn_rune.count(f.name) && mapping_shape(research, f.name) == "game.act") {
            first_act = fn_rune[f.name];
            break;
        }
    }
    if (!first_act.empty()) out.game.links.push_back({"player", first_act, "performs", 1.0, true});

    for (const Function& f : ir.functions) {
        if (!fn_rune.count(f.name)) continue;
        for (const std::string& callee : f.calls) {
            if (fn_rune.count(callee)) {
                out.game.links.push_back({fn_rune[f.name], fn_rune[callee], "calls", 1.0, true});
            }
        }
        for (const std::string& read : f.reads) {
            std::string leaf = read;
            const size_t dot = leaf.find_last_of('.');
            if (dot != std::string::npos) leaf = leaf.substr(dot + 1);
            for (const auto& entry : field_rune) {
                const std::string key = entry.first;
                const size_t kdot = key.find_last_of('.');
                const std::string fleaf = kdot == std::string::npos ? key : key.substr(kdot + 1);
                if (fleaf == leaf) {
                    out.game.links.push_back({fn_rune[f.name], entry.second, "uses", 1.0, true});
                }
            }
        }
    }
    for (const auto& entry : field_rune) {
        const auto it = field_value.find(entry.first);
        if (it == field_value.end()) continue;
        if (entry.first.find("speed") != std::string::npos) {
            out.game.links.push_back({"player", entry.second, "speed", it->second, true});
        }
    }

    /* The research mantle: the records, linked locally. */
    std::set<std::string> research_used;
    auto unique_research = [&](const std::string& raw) {
        const std::string base = sanitize(raw);
        std::string candidate = base;
        int k = 2;
        while (research_used.count(candidate)) candidate = base + "_" + std::to_string(k++);
        research_used.insert(candidate);
        return candidate;
    };

    json target_content;
    target_content["id"] = ir.target.id;
    target_content["title"] = ir.target.title;
    target_content["platform"] = ir.target.platform;
    target_content["origin"] = ir.target.origin;
    target_content["depth"] = ir.target.depth;
    target_content["functions"] = static_cast<int>(ir.functions.size());
    target_content["tables"] = static_cast<int>(ir.tables.size());
    const std::string target_name = unique_research("target");
    out.research.runes.push_back(make_rune(
        target_name, "molerat.target", target_content, "weak", ir.target.depth,
        "the target", "the forge's input", "read from the foreign IR", run_id,
        ir.target.origin));

    std::set<std::string> model_names;
    for (const ModelRecord& m : research.models) {
        const std::string name = unique_research(m.name);
        model_names.insert(m.source);
        json content;
        content["domain"] = m.domain;
        content["operations"] = jstrs(m.operations);
        content["laws"] = jstrs(m.laws);
        content["composition"] = jstrs(m.composition);
        content["observation"] = m.observation;
        content["citations"] = jstrs(m.citations);
        content["evidence"] = m.evidence;
        out.research.runes.push_back(make_rune(
            name, "molerat.model", content, m.confidence, ir.target.depth, m.name,
            m.evidence, "model", run_id, ir.target.origin));
        out.research.links.push_back({target_name, name, "models", 1.0, true});
    }

    std::set<std::string> mapping_names;
    for (const MappingRecord& m : research.mappings) {
        const std::string name = unique_research(m.name);
        mapping_names.insert(m.name);
        json content;
        content["source_construct"] = m.source_construct;
        content["void_shape"] = m.void_shape;
        content["fields"] = jstrs(m.fields);
        content["lost"] = jstrs(m.lost);
        content["evidence"] = m.evidence;
        out.research.runes.push_back(make_rune(
            name, "molerat.mapping", content, m.confidence, ir.target.depth, m.name,
            m.evidence, "mapping", run_id, ir.target.origin));
        /* model -> mapping, when the mapping names a construct the model covers. */
        for (const ModelRecord& model : research.models) {
            const bool same = m.source_construct == model.source;
            const bool below = m.source_construct.rfind(model.source + ".", 0) == 0;
            const bool table = model.name == "model-table-" + m.source_construct;
            if (!same && !below && !table) continue;
            /* Find the model's rune name by suffix match; the unique pass preserved
             * the model name unless it collided. */
            for (size_t i = 0; i < out.research.runes.size(); ++i) {
                const RuneSpec& candidate = out.research.runes[i];
                if (candidate.glyph != "molerat.model") continue;
                if (candidate.name == sanitize(model.name) ||
                    candidate.name.rfind(sanitize(model.name) + "_", 0) == 0) {
                    out.research.links.push_back({candidate.name, name, "maps", 1.0, true});
                    break;
                }
            }
        }
    }

    for (const ResidualRecord& m : research.residuals) {
        const std::string name = unique_research(m.name);
        json content;
        content["what"] = m.what;
        content["why"] = m.why;
        content["evidence"] = m.evidence;
        content["closest"] = m.closest;
        out.research.runes.push_back(make_rune(
            name, "molerat.residual", content, m.confidence, ir.target.depth, m.what,
            m.why, "residual", run_id, ir.target.origin));
        out.research.links.push_back({target_name, name, "leaves", 1.0, true});
    }

    for (const HolidayRecord& m : research.holidays) {
        const std::string name = unique_research(m.name);
        json content;
        content["target"] = m.target;
        content["interface"] = m.iface;
        content["channels"] = m.channels;
        content["lifecycle"] = m.lifecycle;
        content["ownership"] = m.ownership;
        content["units"] = m.units;
        content["not_covered"] = m.not_covered;
        out.research.runes.push_back(make_rune(
            name, "molerat.holiday", content, "weak", ir.target.depth, m.name,
            "behaviour that runs as the original", "translation holiday", run_id,
            ir.target.origin));
        out.research.links.push_back({target_name, name, "delegates", 1.0, true});
    }

    /* Scaffolding the normal form eliminates. It is named here so a host can show what
     * the carried ruleset removed; none of these names appears in either mantle. */
    const std::string first_fn = ir.functions.empty() ? "entry" : ir.functions.front().name;
    out.scaffolding.push_back("sequence_" + sanitize(first_fn));
    for (const Function& f : ir.functions) {
        for (const std::string& callee : f.calls) {
            out.scaffolding.push_back("call_" + sanitize(f.name) + "_" + sanitize(callee));
        }
    }
    for (const Table& t : ir.tables) {
        for (const Field& fld : t.fields) {
            out.scaffolding.push_back("seed_" + sanitize(t.name) + "_" + sanitize(fld.name));
        }
    }

    out.ok = true;
    return out;
}

std::string emit_document(const ComposeResult& composed, const ForeignIR& ir,
                          const std::string& run_id, const std::string& seed,
                          std::string& error) {
    (void)ir;   // the target's provenance is already on the research runes
    error.clear();
    if (!composed.ok) {
        error = composed.error.empty() ? "the compose step did not succeed" : composed.error;
        return {};
    }

    json doc;
    doc["version"] = 1;
    doc["domains"] = json::object();
    doc["domains"][run_id] = json{{"name", run_id}, {"repo", nullptr}, {"liveUrl", nullptr},
                                  {"build", nullptr}, {"deploy", nullptr},
                                  {"preview", nullptr}, {"port", nullptr}};
    doc["mantles"] = json::array();
    doc["mantles"].push_back(mantle_json(composed.game, seed));
    doc["mantles"].push_back(mantle_json(composed.research, seed));
    doc["bindings"] = json::array();

    auto decl = [](const std::string& name, const std::string& kind,
                   const std::vector<std::string>& fields, const json& kinds) {
        json d;
        d["glyph"] = name;
        d["label"] = name;
        d["editor"] = "text";
        d["fields"] = jstrs(fields);
        d["kind"] = kind;
        d["kinds"] = kinds;
        d["presentations"] = json::object();
        d["schema"] = nullptr;
        return d;
    };

    json measure_kinds = json::object();
    measure_kinds["value"] = json{{"level", "ratio"}};

    doc["glyphs"] = json::object();
    doc["glyphs"]["game.thing"] = decl("game.thing", "entity", {"summary"}, json::object());
    doc["glyphs"]["game.act"] = decl("game.act", "act", {"effect"}, json::object());
    doc["glyphs"]["game.measure"] = decl("game.measure", "measure", {"value"}, measure_kinds);
    doc["glyphs"]["molerat.target"] = decl(
        "molerat.target", "entity", {"path", "platform", "depth", "provenance"}, json::object());
    doc["glyphs"]["molerat.model"] = decl(
        "molerat.model", "entity",
        {"domain", "operations", "laws", "composition", "observation", "citations"},
        json::object());
    doc["glyphs"]["molerat.mapping"] = decl(
        "molerat.mapping", "entity",
        {"source_construct", "void_shape", "fields", "lost"}, json::object());
    doc["glyphs"]["molerat.residual"] = decl(
        "molerat.residual", "entity",
        {"what", "why", "evidence", "closest_void_shape"}, json::object());
    doc["glyphs"]["molerat.holiday"] = decl(
        "molerat.holiday", "entity",
        {"interface", "channels", "lifecycle", "ownership", "units", "not_covered"},
        json::object());

    doc["scripts"] = json::object();
    doc["config"] = json{{"actor", "molerat"},
                         {"transform", json{{"reduce", reduce_spec()}}}};
    doc["active"] = json{{"mantle", composed.game.name}, {"domain", composed.game.domain}};
    doc["_baseline"] = doc["mantles"];
    return doc.dump(2);
}

Validation validate_document(std::string_view document) {
    Validation v;
    json j;
    try {
        j = json::parse(document.begin(), document.end());
    } catch (const std::exception& e) {
        v.problems.push_back(std::string("the document is not JSON: ") + e.what());
        return v;
    }
    if (!j.is_object()) {
        v.problems.push_back("the document must be a JSON object");
        return v;
    }
    if (j.value("version", 0) != 1) {
        v.problems.push_back("the document must have version 1");
    }

    const std::set<std::string> builtins = {"text", "richtext", "image", "imageList",
                                            "color", "link", "group"};
    std::set<std::string> declared;
    if (j.contains("glyphs") && j["glyphs"].is_object()) {
        for (auto it = j["glyphs"].begin(); it != j["glyphs"].end(); ++it) {
            declared.insert(it.key());
        }
    }
    auto glyph_ok = [&](const std::string& g) {
        return builtins.count(g) || declared.count(g);
    };
    auto glyph_kind = [&](const std::string& g) -> std::string {
        if (j.contains("glyphs") && j["glyphs"].is_object() &&
            j["glyphs"].contains(g) && j["glyphs"][g].is_object()) {
            return j["glyphs"][g].value("kind", "entity");
        }
        return "entity";
    };

    std::set<std::string> ids;
    std::set<std::string> mantle_names;
    if (!j.contains("mantles") || !j["mantles"].is_array()) {
        v.problems.push_back("the document has no mantles array");
        return v;
    }
    for (const json& m : j["mantles"]) {
        if (!m.is_object()) {
            v.problems.push_back("a mantle is not an object");
            continue;
        }
        const std::string mn = m.value("name", "");
        if (mn.empty()) {
            v.problems.push_back("a mantle has no name");
            continue;
        }
        if (!mantle_names.insert(mn).second) {
            v.problems.push_back("duplicate mantle name '" + mn + "'");
        }
        std::set<std::string> names;
        if (!m.contains("runes") || !m["runes"].is_array()) {
            v.problems.push_back("mantle '" + mn + "' has no runes array");
            continue;
        }
        for (const json& r : m["runes"]) {
            if (!r.is_object() || !r.contains("spirit") || !r["spirit"].is_object()) {
                v.problems.push_back("a rune in '" + mn + "' has no spirit");
                continue;
            }
            const std::string rn = r["spirit"].value("name", "");
            const std::string rid = r["spirit"].value("id", "");
            if (rn.empty()) {
                v.problems.push_back("a rune in '" + mn + "' has no name");
            } else if (!names.insert(rn).second) {
                v.problems.push_back("duplicate rune name '" + rn + "' in '" + mn + "'");
            }
            if (!rid.empty() && !ids.insert(rid).second) {
                v.problems.push_back("duplicate rune id '" + rid + "'");
            }
            const std::string g = r.value("glyph", "");
            if (g.empty()) {
                v.problems.push_back("rune '" + rn + "' has no glyph");
            } else if (!glyph_ok(g)) {
                v.problems.push_back("rune '" + rn + "' uses undeclared glyph '" + g + "'");
            }
            if (!r.contains("facets") || !r["facets"].is_object()) {
                v.problems.push_back("rune '" + rn + "' has no facets object");
            }
            const bool is_measure = glyph_kind(g) == "measure";
            if (is_measure) {
                if (!r.contains("quantity") || !r["quantity"].is_object()) {
                    v.problems.push_back("measure rune '" + rn + "' has no quantity");
                } else {
                    const std::string level = r["quantity"].value("level", "");
                    if (level != "nominal" && level != "ordinal" &&
                        level != "interval" && level != "ratio") {
                        v.problems.push_back("measure rune '" + rn +
                                             "' has an invalid quantity level");
                    }
                }
            } else if (r.contains("quantity")) {
                v.problems.push_back("non-measure rune '" + rn +
                                     "' carries a quantity");
            }
        }
        if (m.contains("layout") && m["layout"].is_object() &&
            m["layout"].contains("edges") && m["layout"]["edges"].is_array()) {
            for (const json& e : m["layout"]["edges"]) {
                if (!e.is_object()) continue;
                const std::string from = e.value("from", "");
                const std::string to = e.value("to", "");
                if (!names.count(from)) {
                    v.problems.push_back("link in '" + mn + "' names missing rune '" + from + "'");
                }
                if (!names.count(to)) {
                    v.problems.push_back("link in '" + mn + "' names missing rune '" + to + "'");
                }
            }
        } else {
            v.problems.push_back("mantle '" + mn + "' has no layout.edges array");
        }
    }

    if (!j.contains("_baseline") || !j["_baseline"].is_array()) {
        v.problems.push_back("the document has no _baseline array");
    }
    if (j.contains("active") && j["active"].is_object() &&
        j["active"].contains("mantle") && !j["active"]["mantle"].is_null()) {
        const std::string active = j["active"].value("mantle", "");
        if (!mantle_names.count(active)) {
            v.problems.push_back("active.mantle names missing mantle '" + active + "'");
        }
    }

    v.ok = v.problems.empty();
    return v;
}

bool run_slice(std::string_view ir_json, const std::string& run_id,
               const std::string& seed, Slice& out, std::string& error) {
    out = Slice{};
    if (!parse_ir(ir_json, out.ir, error)) return false;
    out.analysis = analyze(out.ir);
    out.research = research(out.ir, out.analysis);
    out.compose = compose(out.ir, out.research, run_id, seed);
    if (!out.compose.ok) {
        error = out.compose.error;
        return false;
    }
    out.document = emit_document(out.compose, out.ir, run_id, seed, error);
    if (out.document.empty()) return false;
    out.validation = validate_document(out.document);
    for (const ResidualRecord& r : out.research.residuals) {
        out.residual.push_back(r.what);
    }
    return out.validation.ok;
}

}  // namespace squid::molerat
