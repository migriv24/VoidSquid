/* ir.cpp — parse the normalized foreign IR and say what it does structurally. */
#include "molerat/molerat.hpp"

#include "json.hpp"

#include <algorithm>
#include <functional>
#include <set>
#include <sstream>

namespace squid::molerat {
namespace {

using json = nlohmann::json;

std::string get_string(const json& j, const char* key) {
    if (!j.is_object() || !j.contains(key) || !j[key].is_string()) return {};
    return j[key].get<std::string>();
}

std::vector<std::string> get_strings(const json& j, const char* key) {
    std::vector<std::string> out;
    if (!j.is_object() || !j.contains(key) || !j[key].is_array()) return out;
    for (const json& v : j[key]) {
        if (v.is_string()) out.push_back(v.get<std::string>());
    }
    return out;
}

void add_unique(std::vector<std::string>& v, const std::string& s) {
    if (s.empty()) return;
    if (std::find(v.begin(), v.end(), s) == v.end()) v.push_back(s);
}

}  // namespace

bool parse_ir(std::string_view text, ForeignIR& out, std::string& error) {
    out = ForeignIR{};
    error.clear();

    json j;
    try {
        j = json::parse(text.begin(), text.end());
    } catch (const std::exception& e) {
        error = std::string("the IR is not JSON: ") + e.what();
        return false;
    }
    if (!j.is_object()) {
        error = "the IR must be a JSON object";
        return false;
    }

    out.schema = get_string(j, "schema");
    if (out.schema.rfind("molerat.foreign-ir", 0) != 0) {
        error = "unsupported IR schema '" + out.schema +
                "'; expected molerat.foreign-ir/0.1";
        return false;
    }

    if (!j.contains("target") || !j["target"].is_object()) {
        error = "the IR has no target object";
        return false;
    }
    const json& t = j["target"];
    out.target.id = get_string(t, "id");
    if (out.target.id.empty()) {
        error = "the target has no id";
        return false;
    }
    out.target.title = get_string(t, "title");
    out.target.platform = get_string(t, "platform");
    out.target.origin = get_string(t, "origin");
    out.target.depth = get_string(t, "depth");
    if (out.target.depth.empty()) out.target.depth = "source";

    if (!j.contains("functions") || !j["functions"].is_array()) {
        error = "the IR has no functions array";
        return false;
    }
    for (const json& fj : j["functions"]) {
        if (!fj.is_object()) {
            error = "a function is not an object";
            return false;
        }
        Function f;
        f.name = get_string(fj, "name");
        if (f.name.empty()) {
            error = "a function has no name";
            return false;
        }
        f.addr = get_string(fj, "addr");
        f.calls = get_strings(fj, "calls");
        f.reads = get_strings(fj, "reads");
        f.writes = get_strings(fj, "writes");
        f.evidence = get_strings(fj, "evidence");

        if (fj.contains("blocks") && fj["blocks"].is_array()) {
            for (const json& bj : fj["blocks"]) {
                Block b;
                b.id = get_string(bj, "id");
                if (bj.contains("ops") && bj["ops"].is_array()) {
                    for (const json& oj : bj["ops"]) {
                        Op o;
                        o.kind = get_string(oj, "kind");
                        if (o.kind.empty()) {
                            error = "an op has no kind in function " + f.name;
                            return false;
                        }
                        o.dst = get_string(oj, "dst");
                        o.src = get_string(oj, "src");
                        o.value = get_string(oj, "value");
                        b.ops.push_back(std::move(o));
                    }
                }
                f.blocks.push_back(std::move(b));
            }
        }

        /* The ops are the ground truth; the explicit lists are a convenience. Merge
         * them so a fixture that omits the lists still analyses. */
        for (const Block& b : f.blocks) {
            for (const Op& o : b.ops) {
                if (o.kind == "load") add_unique(f.reads, o.src);
                if (o.kind == "store") add_unique(f.writes, o.dst);
                if (o.kind == "call") add_unique(f.calls, o.value);
            }
        }
        out.functions.push_back(std::move(f));
    }
    if (out.functions.empty()) {
        error = "the IR has no functions";
        return false;
    }

    if (j.contains("tables") && j["tables"].is_array()) {
        for (const json& tj : j["tables"]) {
            if (!tj.is_object()) {
                error = "a table is not an object";
                return false;
            }
            Table table;
            table.name = get_string(tj, "name");
            if (table.name.empty()) {
                error = "a table has no name";
                return false;
            }
            if (tj.contains("fields") && tj["fields"].is_array()) {
                for (const json& fj : tj["fields"]) {
                    if (!fj.is_object()) {
                        error = "a field in table " + table.name + " is not an object";
                        return false;
                    }
                    Field field;
                    field.name = get_string(fj, "name");
                    if (field.name.empty()) {
                        error = "a field in table " + table.name + " has no name";
                        return false;
                    }
                    field.type = get_string(fj, "type");
                    field.unit = get_string(fj, "unit");
                    if (fj.contains("value") && fj["value"].is_number()) {
                        field.value = fj["value"].get<double>();
                        field.has_value = true;
                    }
                    table.fields.push_back(std::move(field));
                }
            }
            out.tables.push_back(std::move(table));
        }
    }

    out.constants = get_strings(j, "constants");
    out.residual = get_strings(j, "residual");
    return true;
}

Analysis analyze(const ForeignIR& ir) {
    Analysis a;
    std::set<std::string> known;
    for (const Function& f : ir.functions) known.insert(f.name);

    for (const Function& f : ir.functions) {
        std::set<std::string> reads(f.reads.begin(), f.reads.end());
        std::set<std::string> writes(f.writes.begin(), f.writes.end());
        std::set<std::string> calls(f.calls.begin(), f.calls.end());
        for (const Block& b : f.blocks) {
            for (const Op& o : b.ops) {
                if (o.kind == "load" && !o.src.empty()) reads.insert(o.src);
                if (o.kind == "store" && !o.dst.empty()) writes.insert(o.dst);
                if (o.kind == "call" && !o.value.empty()) calls.insert(o.value);
            }
        }
        a.reads[f.name] = std::vector<std::string>(reads.begin(), reads.end());
        a.writes[f.name] = std::vector<std::string>(writes.begin(), writes.end());
        a.calls[f.name] = std::vector<std::string>(calls.begin(), calls.end());
    }

    /* Post-order over the call graph. A cycle is tolerated: the visited set stops it,
     * and the order is still deterministic. */
    std::set<std::string> visited;
    std::function<void(const std::string&)> visit = [&](const std::string& name) {
        if (visited.count(name)) return;
        visited.insert(name);
        auto it = a.calls.find(name);
        if (it != a.calls.end()) {
            for (const std::string& callee : it->second) {
                if (known.count(callee)) visit(callee);
            }
        }
        a.order.push_back(name);
    };
    for (const Function& f : ir.functions) visit(f.name);

    for (const Function& f : ir.functions) {
        std::ostringstream s;
        s << f.name;
        const auto& w = a.writes[f.name];
        if (!w.empty()) {
            s << " writes ";
            for (size_t i = 0; i < w.size(); ++i) s << (i ? ", " : "") << w[i];
        }
        const auto& c = a.calls[f.name];
        if (!c.empty()) {
            s << "; calls ";
            for (size_t i = 0; i < c.size(); ++i) s << (i ? ", " : "") << c[i];
        }
        a.findings.push_back(s.str());
    }
    return a;
}

}  // namespace squid::molerat
