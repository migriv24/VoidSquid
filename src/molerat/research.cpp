/* research.cpp — the four research records, their reports, and the host's cache form. */
#include "molerat/molerat.hpp"

#include "json.hpp"

#include <algorithm>
#include <sstream>

namespace squid::molerat {
namespace {

using json = nlohmann::json;

bool has(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}

std::string join(const std::vector<std::string>& v, const char* sep = ", ") {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) out += (i ? sep : "") + v[i];
    return out;
}

json jstrs(const std::vector<std::string>& v) {
    json a = json::array();
    for (const std::string& s : v) a.push_back(s);
    return a;
}

std::vector<std::string> vstrs(const json& j, const char* key) {
    std::vector<std::string> out;
    if (!j.is_object() || !j.contains(key) || !j[key].is_array()) return out;
    for (const json& v : j[key]) {
        if (v.is_string()) out.push_back(v.get<std::string>());
    }
    return out;
}

std::string str(const json& j, const char* key) {
    if (!j.is_object() || !j.contains(key) || !j[key].is_string()) return {};
    return j[key].get<std::string>();
}

}  // namespace

Research research(const ForeignIR& ir, const Analysis& analysis) {
    Research r;

    for (const Function& f : ir.functions) {
        const std::vector<std::string> reads = analysis.reads.count(f.name)
            ? analysis.reads.at(f.name) : f.reads;
        const std::vector<std::string> writes = analysis.writes.count(f.name)
            ? analysis.writes.at(f.name) : f.writes;
        const std::vector<std::string> calls = analysis.calls.count(f.name)
            ? analysis.calls.at(f.name) : f.calls;

        ModelRecord m;
        m.name = "model-" + f.name;
        m.source = f.name;
        m.domain = writes.empty() ? "predicate over world state"
                                  : (has(writes, "player_vy") || has(reads, "gravity")
                                         ? "state vector with vertical velocity"
                                         : "record transform");
        std::vector<std::string> ops;
        for (const Block& b : f.blocks) {
            for (const Op& o : b.ops) {
                if (std::find(ops.begin(), ops.end(), o.kind) == ops.end()) ops.push_back(o.kind);
            }
        }
        m.operations = ops;
        if (has(reads, "gravity") && (has(writes, "player_vy") || has(writes, "vy"))) {
            m.laws.push_back("vy_next = vy + gravity * dt");
            m.laws.push_back("dt is not in the IR");
        } else if (writes.empty()) {
            m.laws.push_back("the predicate's truth value is a function of the read fields");
        } else {
            m.laws.push_back("the transform's law is not recovered");
        }
        m.composition = calls;
        m.observation = writes.empty() ? "a boolean" : "the written fields over time";
        m.citations = {"learned-decompilers", "VoidCore:/sources/lafont-interaction-nets.md"};
        m.confidence = "weak";
        m.evidence = f.evidence.empty() ? "abstract interpretation of the IR block"
                                        : join(f.evidence, "; ");
        r.models.push_back(std::move(m));

        MappingRecord map;
        map.name = "map-" + f.name;
        map.source_construct = f.name;
        map.void_shape = writes.empty() ? "game.thing" : "game.act";
        map.fields = {"name=" + f.name, "glyph=" + map.void_shape,
                      "reads=" + std::to_string(reads.size()),
                      "writes=" + std::to_string(writes.size())};
        map.lost.push_back(writes.empty()
            ? "the predicate body is not re-expressed"
            : "the function body is not re-expressed; only its dataflow is mapped");
        map.confidence = "weak";
        map.evidence = m.evidence;
        r.mappings.push_back(std::move(map));
    }

    for (const Table& t : ir.tables) {
        ModelRecord m;
        m.name = "model-table-" + t.name;
        m.source = t.name;
        m.domain = "typed record";
        m.operations = {"read"};
        m.laws.push_back("fields carry the declared units");
        m.composition = {"read by the functions that name its fields"};
        m.observation = "field values";
        m.citations = {"learned-decompilers"};
        m.confidence = "weak";
        m.evidence = "table declaration in the IR";
        r.models.push_back(std::move(m));

        for (const Field& fld : t.fields) {
            MappingRecord map;
            map.name = "map-" + t.name + "-" + fld.name;
            map.source_construct = t.name + "." + fld.name;
            map.void_shape = (fld.has_value || !fld.unit.empty()) ? "game.measure" : "game.thing";
            map.fields = {"name=" + fld.name, "glyph=" + map.void_shape};
            if (fld.has_value) map.fields.push_back("value=" + std::to_string(fld.value));
            if (!fld.unit.empty()) map.fields.push_back("unit=" + fld.unit);
            map.lost.push_back("the field's writer is not identified");
            map.confidence = "weak";
            map.evidence = "table declaration in the IR";
            r.mappings.push_back(std::move(map));
        }
    }

    int ordinal = 0;
    auto add_residual = [&](const std::string& what, const std::string& why,
                            const std::string& closest) {
        ResidualRecord rr;
        rr.name = "residual-" + std::to_string(++ordinal);
        rr.what = what;
        rr.why = why;
        rr.closest = closest;
        rr.evidence = "the IR does not carry it";
        rr.confidence = "weak";
        r.residuals.push_back(std::move(rr));
    };

    add_residual("the original tick rate", "dt is not in the IR", "a rule constant");
    add_residual("input mapping", "the IR has no input ports", "an input holiday");
    add_residual("collision response", "the IR has no collision query", "a collision holiday");
    add_residual("rendering", "the IR has no draw data", "Void Jelly");
    add_residual("behaviour equivalence", "no original was run; the fixture is synthetic",
                 "an opaque oracle");

    for (const Function& f : ir.functions) {
        const std::vector<std::string> calls = analysis.calls.count(f.name)
            ? analysis.calls.at(f.name) : f.calls;
        for (const std::string& callee : calls) {
            bool known = false;
            for (const Function& g : ir.functions) {
                if (g.name == callee) known = true;
            }
            if (!known) {
                add_residual("the callee " + callee + " of " + f.name,
                             "the callee is not in the IR", "a holiday or a residual");
            }
        }
        const std::vector<std::string> writes = analysis.writes.count(f.name)
            ? analysis.writes.at(f.name) : f.writes;
        if (writes.empty()) {
            add_residual("the body of " + f.name,
                         "the IR records a predicate but not its full body", "a rule");
        }
    }
    for (const std::string& s : ir.residual) add_residual(s, "stated by the IR", "a holiday or a residual");

    HolidayRecord input;
    input.name = "holiday-input";
    input.target = "the player";
    input.iface = "input() -> {forward, turn, jump, use}";
    input.channels = "snapshot, every tick";
    input.lifecycle = "start, pause, resume, stop";
    input.ownership = "the host owns input; the avatar consumes it";
    input.units = "normalized [-1, 1]";
    input.not_covered = "device mapping, dead zones, rebinding";
    r.holidays.push_back(std::move(input));

    HolidayRecord collision;
    collision.name = "holiday-collision";
    collision.target = "the world";
    collision.iface = "solid(point, radius) -> bool";
    collision.channels = "query, per move";
    collision.lifecycle = "stateless";
    collision.ownership = "the host owns geometry; the avatar asks";
    collision.units = "metres";
    collision.not_covered = "slopes, moving platforms, one-way surfaces";
    r.holidays.push_back(std::move(collision));

    HolidayRecord render;
    render.name = "holiday-render";
    render.target = "the scene";
    render.iface = "emit(scene) -> drawlist";
    render.channels = "frame";
    render.lifecycle = "start, frame, stop";
    render.ownership = "Void Jelly computes; the host presents";
    render.units = "the target's";
    render.not_covered = "everything the target cannot hold";
    r.holidays.push_back(std::move(render));

    return r;
}

std::string residual_json(const ForeignIR& ir, const Research& research) {
    json j;
    j["target"] = ir.target.id;
    j["title"] = ir.target.title;
    j["count"] = research.residuals.size();
    j["items"] = json::array();
    for (const ResidualRecord& r : research.residuals) {
        j["items"].push_back({{"name", r.name}, {"what", r.what}, {"why", r.why},
                              {"confidence", r.confidence}, {"closest", r.closest}});
    }
    return j.dump(2);
}

std::string research_markdown(const ForeignIR& ir, const Research& research,
                              const std::string& run_id) {
    /* A Squid source id becomes an ordinary OKF body link, so `linked from` names the
     * blast radius. A cross-bundle id (`VoidCore:/...`) stays a code span: the generated
     * page sits in molerat/out/, and inventing a relative path it cannot resolve would be
     * worse than saying exactly what the citation is. */
    auto cite = [](const std::string& c) {
        if (c.find(':') == std::string::npos) return "[" + c + "](/sources/" + c + ".md)";
        return "`" + c + "`";
    };
    std::ostringstream out;
    out << "---\n";
    out << "type: Report\n";
    out << "title: MoleRat research - " << ir.target.id << "\n";
    out << "status: planned\n";
    out << "confidence: weak\n";
    out << "run: " << run_id << "\n";
    out << "---\n\n";
    out << "# MoleRat research: " << (ir.target.title.empty() ? ir.target.id : ir.target.title) << "\n\n";
    out << "**This is a proposed reading, not accepted truth.** Every claim carries its evidence\n";
    out << "and confidence; what did not map is in the residual.\n\n";
    out << "## Target\n\n";
    out << "- id: `" << ir.target.id << "`\n";
    out << "- platform: `" << ir.target.platform << "`\n";
    out << "- depth: `" << ir.target.depth << "`\n";
    out << "- origin: " << (ir.target.origin.empty() ? "(not stated)" : ir.target.origin) << "\n\n";

    out << "## Models\n\n";
    for (const ModelRecord& m : research.models) {
        out << "### `" << m.name << "`\n\n";
        out << "- domain: " << m.domain << "\n";
        out << "- operations: " << join(m.operations) << "\n";
        out << "- laws: " << join(m.laws) << "\n";
        out << "- composition: " << join(m.composition) << "\n";
        out << "- observation: " << m.observation << "\n";
        out << "- confidence: `" << m.confidence << "`\n";
        out << "- evidence: " << m.evidence << "\n";
        out << "- citations: ";
        for (size_t i = 0; i < m.citations.size(); ++i) {
            out << (i ? ", " : "") << cite(m.citations[i]);
        }
        out << "\n\n";
    }

    out << "## Mappings\n\n";
    for (const MappingRecord& m : research.mappings) {
        out << "- `" << m.source_construct << "` -> `" << m.void_shape << "`";
        if (!m.lost.empty()) out << "; lost: " << join(m.lost);
        out << "\n";
    }
    out << "\n";

    out << "## Residual\n\n";
    for (const ResidualRecord& r : research.residuals) {
        out << "- **" << r.what << "** - " << r.why << " (nearest: " << r.closest
            << "; `" << r.confidence << "`)\n";
    }
    out << "\n";

    out << "## Holidays\n\n";
    for (const HolidayRecord& h : research.holidays) {
        out << "### `" << h.name << "`\n\n";
        out << "- interface: `" << h.iface << "`\n";
        out << "- channels: " << h.channels << "\n";
        out << "- lifecycle: " << h.lifecycle << "\n";
        out << "- ownership: " << h.ownership << "\n";
        out << "- units: " << h.units << "\n";
        out << "- not covered: " << h.not_covered << "\n\n";
    }
    return out.str();
}

std::string research_to_json(const Research& r) {
    json j;
    j["models"] = json::array();
    for (const ModelRecord& m : r.models) {
        j["models"].push_back({{"name", m.name}, {"source", m.source}, {"domain", m.domain},
                               {"operations", jstrs(m.operations)}, {"laws", jstrs(m.laws)},
                               {"composition", jstrs(m.composition)},
                               {"observation", m.observation},
                               {"citations", jstrs(m.citations)},
                               {"confidence", m.confidence}, {"evidence", m.evidence}});
    }
    j["mappings"] = json::array();
    for (const MappingRecord& m : r.mappings) {
        j["mappings"].push_back({{"name", m.name}, {"source_construct", m.source_construct},
                                 {"void_shape", m.void_shape}, {"fields", jstrs(m.fields)},
                                 {"lost", jstrs(m.lost)}, {"confidence", m.confidence},
                                 {"evidence", m.evidence}});
    }
    j["residuals"] = json::array();
    for (const ResidualRecord& m : r.residuals) {
        j["residuals"].push_back({{"name", m.name}, {"what", m.what}, {"why", m.why},
                                  {"evidence", m.evidence}, {"closest", m.closest},
                                  {"confidence", m.confidence}});
    }
    j["holidays"] = json::array();
    for (const HolidayRecord& m : r.holidays) {
        j["holidays"].push_back({{"name", m.name}, {"target", m.target}, {"iface", m.iface},
                                 {"channels", m.channels}, {"lifecycle", m.lifecycle},
                                 {"ownership", m.ownership}, {"units", m.units},
                                 {"not_covered", m.not_covered}});
    }
    return j.dump(2);
}

bool research_from_json(std::string_view text, Research& out, std::string& error) {
    out = Research{};
    error.clear();
    json j;
    try {
        j = json::parse(text.begin(), text.end());
    } catch (const std::exception& e) {
        error = std::string("the research cache is not JSON: ") + e.what();
        return false;
    }
    if (!j.is_object()) {
        error = "the research cache must be an object";
        return false;
    }
    for (const json& m : j.value("models", json::array())) {
        ModelRecord r;
        r.name = str(m, "name");
        r.source = str(m, "source");
        r.domain = str(m, "domain");
        r.operations = vstrs(m, "operations");
        r.laws = vstrs(m, "laws");
        r.composition = vstrs(m, "composition");
        r.observation = str(m, "observation");
        r.citations = vstrs(m, "citations");
        r.confidence = str(m, "confidence");
        r.evidence = str(m, "evidence");
        if (r.name.empty()) { error = "a model record has no name"; return false; }
        out.models.push_back(std::move(r));
    }
    for (const json& m : j.value("mappings", json::array())) {
        MappingRecord r;
        r.name = str(m, "name");
        r.source_construct = str(m, "source_construct");
        r.void_shape = str(m, "void_shape");
        r.fields = vstrs(m, "fields");
        r.lost = vstrs(m, "lost");
        r.confidence = str(m, "confidence");
        r.evidence = str(m, "evidence");
        if (r.name.empty()) { error = "a mapping record has no name"; return false; }
        out.mappings.push_back(std::move(r));
    }
    for (const json& m : j.value("residuals", json::array())) {
        ResidualRecord r;
        r.name = str(m, "name");
        r.what = str(m, "what");
        r.why = str(m, "why");
        r.evidence = str(m, "evidence");
        r.closest = str(m, "closest");
        r.confidence = str(m, "confidence");
        if (r.name.empty()) { error = "a residual record has no name"; return false; }
        out.residuals.push_back(std::move(r));
    }
    for (const json& m : j.value("holidays", json::array())) {
        HolidayRecord r;
        r.name = str(m, "name");
        r.target = str(m, "target");
        r.iface = str(m, "iface");
        r.channels = str(m, "channels");
        r.lifecycle = str(m, "lifecycle");
        r.ownership = str(m, "ownership");
        r.units = str(m, "units");
        r.not_covered = str(m, "not_covered");
        if (r.name.empty()) { error = "a holiday record has no name"; return false; }
        out.holidays.push_back(std::move(r));
    }
    return true;
}

}  // namespace squid::molerat
