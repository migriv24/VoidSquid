/* effect.cpp — MoleRat's effects: read the IR, cache the run, write the document. */
#include "molerat/effect.hpp"

#include "molerat/molerat.hpp"

#include "json.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <set>
#include <string_view>

namespace squid {
namespace {

using json = nlohmann::json;
namespace fs = std::filesystem;

std::string err(const std::string& message) {
    /* The op's own verdict. The host's `--json` envelope reports that the dispatch
     * succeeded; this `ok` reports whether the operation did. A refused answer is
     * `ok:false` and has written nothing. */
    return json{{"ok", false}, {"error", message}}.dump();
}

std::string now_iso() {
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

uint64_t fnv1a64(std::string_view s) {
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

std::string hex64(uint64_t v) {
    char buf[17];
    std::snprintf(buf, sizeof buf, "%016llx", static_cast<unsigned long long>(v));
    return std::string(buf);
}

std::string random_seed() {
    std::random_device rd;
    const uint64_t a = (static_cast<uint64_t>(rd()) << 32) ^ rd();
    const uint64_t b = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    return hex64(a ^ b);
}

std::string root_dir() {
    const char* env = std::getenv("MOLERAT_OUT");
    return (env && *env) ? std::string(env) : std::string("molerat/out");
}

fs::path run_dir(const std::string& run) {
    return fs::path(root_dir()) / run;
}

bool read_text(const fs::path& path, size_t max_bytes, std::string& out,
               std::string& error) {
    std::error_code ec;
    const auto size = fs::file_size(path, ec);
    if (ec) {
        error = "cannot read '" + path.string() + "': " + ec.message();
        return false;
    }
    if (size > max_bytes) {
        error = "'" + path.string() + "' is larger than the bounded read";
        return false;
    }
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "cannot open '" + path.string() + "'";
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    if (!in.good() && !in.eof()) {
        error = "cannot read '" + path.string() + "'";
        return false;
    }
    return true;
}

bool write_text(const fs::path& path, const std::string& text, std::string& error) {
    std::error_code ec;
    if (!path.parent_path().empty()) fs::create_directories(path.parent_path(), ec);
    if (ec) {
        error = "cannot create '" + path.parent_path().string() + "': " + ec.message();
        return false;
    }
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        error = "cannot write '" + path.string() + "'";
        return false;
    }
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!out) {
        error = "cannot write '" + path.string() + "'";
        return false;
    }
    return true;
}

bool read_json_file(const fs::path& path, json& out, std::string& error) {
    std::string text;
    if (!read_text(path, 8 * 1024 * 1024, text, error)) return false;
    try {
        out = json::parse(text);
    } catch (const std::exception& e) {
        error = "'" + path.string() + "' is not JSON: " + e.what();
        return false;
    }
    return true;
}

bool write_json_file(const fs::path& path, const json& value, std::string& error) {
    return write_text(path, value.dump(2), error);
}

struct Args {
    std::vector<std::string> pos;
    std::map<std::string, std::string> flags;

    bool has(const std::string& key) const { return flags.count(key) != 0; }
    std::string get(const std::string& key, const std::string& fallback = {}) const {
        const auto it = flags.find(key);
        return it == flags.end() ? fallback : it->second;
    }
};

Args parse_args(const std::vector<std::string>& v) {
    Args a;
    for (size_t i = 0; i < v.size(); ++i) {
        const std::string& s = v[i];
        if (s.rfind("--", 0) == 0) {
            std::string key = s.substr(2);
            std::string value;
            const size_t eq = key.find('=');
            if (eq != std::string::npos) {
                value = key.substr(eq + 1);
                key = key.substr(0, eq);
            } else if (i + 1 < v.size() && v[i + 1].rfind("--", 0) != 0) {
                value = v[++i];
            }
            a.flags[key] = value;
        } else {
            a.pos.push_back(s);
        }
    }
    return a;
}

bool load_ir(const std::string& run, squid::molerat::ForeignIR& ir,
             json& run_json, std::string& error) {
    const fs::path dir = run_dir(run);
    if (!read_json_file(dir / "run.json", run_json, error)) return false;
    std::string text;
    if (!read_text(dir / "foreign.ir.json", 8 * 1024 * 1024, text, error)) return false;
    return squid::molerat::parse_ir(text, ir, error);
}

bool load_research(const std::string& run, squid::molerat::Research& research,
                   std::string& error) {
    std::string text;
    if (!read_text(run_dir(run) / "research.json", 8 * 1024 * 1024, text, error)) return false;
    return squid::molerat::research_from_json(text, research, error);
}

void touch_run(const std::string& run, const std::string& stage,
               const std::string& next) {
    const fs::path path = run_dir(run) / "run.json";
    json j;
    std::string error;
    if (!read_json_file(path, j, error)) return;
    j["stage"] = stage;
    j["next"] = next;
    j["updated"] = now_iso();
    write_json_file(path, j, error);
}

bool gather(const Args& a, std::string& reply) {
    if (a.pos.empty() && !a.has("ir")) {
        reply = err("effect molerat gather needs <target> or --ir <path>");
        return true;
    }
    const std::string target = a.pos.empty() ? a.get("ir") : a.pos.front();
    const std::string ir_path = a.get("ir", target);
    const std::string seed = a.get("seed", random_seed());
    const std::string run = a.get("run", "run-" + seed.substr(0, 8));

    std::string text;
    std::string error;
    if (!read_text(ir_path, 8 * 1024 * 1024, text, error)) {
        reply = err(error);
        return true;
    }
    squid::molerat::ForeignIR ir;
    if (!squid::molerat::parse_ir(text, ir, error)) {
        reply = err(error);
        return true;
    }

    const fs::path dir = run_dir(run);
    if (!write_text(dir / "foreign.ir.json", text, error)) {
        reply = err(error);
        return true;
    }
    json run_json;
    run_json["run"] = run;
    run_json["seed"] = seed;
    run_json["created"] = now_iso();
    run_json["target"] = target;
    run_json["ir"] = ir_path;
    run_json["hash"] = "fnv1a64:" + hex64(fnv1a64(text));
    run_json["stage"] = "gathered";
    run_json["next"] = "research";
    run_json["functions"] = ir.functions.size();
    run_json["tables"] = ir.tables.size();
    if (!write_json_file(dir / "run.json", run_json, error)) {
        reply = err(error);
        return true;
    }

    json r;
    r["ok"] = true;
    r["run"] = run;
    r["stage"] = "gathered";
    r["target"] = ir.target.id;
    r["depth"] = ir.target.depth;
    r["functions"] = ir.functions.size();
    r["tables"] = ir.tables.size();
    r["ir"] = (dir / "foreign.ir.json").string();
    r["next"] = "research";
    reply = r.dump();
    return true;
}

bool research_op(const Args& a, std::string& reply) {
    if (a.pos.empty()) {
        reply = err("effect molerat research needs <run>");
        return true;
    }
    const std::string run = a.pos.front();
    squid::molerat::ForeignIR ir;
    json run_json;
    std::string error;
    if (!load_ir(run, ir, run_json, error)) {
        reply = err(error);
        return true;
    }
    const squid::molerat::Analysis analysis = squid::molerat::analyze(ir);
    const squid::molerat::Research research = squid::molerat::research(ir, analysis);
    if (!write_text(run_dir(run) / "research.json",
                    squid::molerat::research_to_json(research), error)) {
        reply = err(error);
        return true;
    }
    touch_run(run, "researched", "compose");

    json r;
    r["ok"] = true;
    r["run"] = run;
    r["stage"] = "researched";
    r["models"] = research.models.size();
    r["mappings"] = research.mappings.size();
    r["residuals"] = research.residuals.size();
    r["holidays"] = research.holidays.size();
    r["next"] = "compose";
    reply = r.dump();
    return true;
}

bool compose_op(const Args& a, std::string& reply) {
    if (a.pos.empty()) {
        reply = err("effect molerat compose needs <run>");
        return true;
    }
    const std::string run = a.pos.front();
    squid::molerat::ForeignIR ir;
    json run_json;
    std::string error;
    if (!load_ir(run, ir, run_json, error)) {
        reply = err(error);
        return true;
    }
    squid::molerat::Research research;
    if (!load_research(run, research, error)) {
        reply = err(error);
        return true;
    }
    const std::string seed = run_json.value("seed", "seed");
    const squid::molerat::ComposeResult composed =
        squid::molerat::compose(ir, research, run, seed);
    if (!composed.ok) {
        reply = err(composed.error);
        return true;
    }
    json summary;
    summary["run"] = run;
    summary["game"] = composed.game.name;
    summary["research"] = composed.research.name;
    summary["runes"] = composed.game.runes.size() + composed.research.runes.size();
    summary["links"] = composed.game.links.size() + composed.research.links.size();
    summary["scaffolding"] = composed.scaffolding;
    summary["next"] = "residual or build";
    if (!write_json_file(run_dir(run) / "compose.json", summary, error)) {
        reply = err(error);
        return true;
    }
    touch_run(run, "composed", "residual or build");

    summary["ok"] = true;
    summary["stage"] = "composed";
    reply = summary.dump();
    return true;
}

bool residual_op(const Args& a, std::string& reply) {
    if (a.pos.empty()) {
        reply = err("effect molerat residual needs <run>");
        return true;
    }
    const std::string run = a.pos.front();
    squid::molerat::ForeignIR ir;
    json run_json;
    std::string error;
    if (!load_ir(run, ir, run_json, error)) {
        reply = err(error);
        return true;
    }
    squid::molerat::Research research;
    if (!load_research(run, research, error)) {
        reply = err(error);
        return true;
    }
    reply = squid::molerat::residual_json(ir, research);
    return true;
}

bool build_op(const Args& a, std::string& reply) {
    if (a.pos.empty()) {
        reply = err("effect molerat build needs <run>");
        return true;
    }
    const std::string run = a.pos.front();
    squid::molerat::ForeignIR ir;
    json run_json;
    std::string error;
    if (!load_ir(run, ir, run_json, error)) {
        reply = err(error);
        return true;
    }
    squid::molerat::Research research;
    if (!load_research(run, research, error)) {
        reply = err(error);
        return true;
    }
    const std::string seed = run_json.value("seed", "seed");
    const squid::molerat::ComposeResult composed =
        squid::molerat::compose(ir, research, run, seed);
    if (!composed.ok) {
        reply = err(composed.error);
        return true;
    }
    std::string document = squid::molerat::emit_document(composed, ir, run, seed, error);
    if (document.empty()) {
        reply = err(error);
        return true;
    }
    const squid::molerat::Validation validation =
        squid::molerat::validate_document(document);

    const fs::path dir = a.has("out") ? fs::path(a.get("out")) : run_dir(run);
    const fs::path state = dir / (run + ".squid");
    if (!write_text(state, document, error) ||
        !write_text(dir / "research.md",
                    squid::molerat::research_markdown(ir, research, run), error) ||
        !write_text(dir / "residual.json",
                    squid::molerat::residual_json(ir, research), error)) {
        reply = err(error);
        return true;
    }
    json provenance;
    provenance["run"] = run;
    provenance["seed"] = seed;
    provenance["created"] = now_iso();
    provenance["target"] = ir.target.id;
    provenance["origin"] = ir.target.origin;
    provenance["depth"] = ir.target.depth;
    provenance["ir"] = (dir / "foreign.ir.json").string();
    provenance["hash"] = run_json.value("hash", "");
    provenance["compute"] = "squid_molerat (pure; no model, no network)";
    provenance["reduce"] = "the ruleset travels in config.transform.reduce; the C++ normal form is computed locally";
    if (!write_json_file(dir / "provenance.json", provenance, error)) {
        reply = err(error);
        return true;
    }
    touch_run(run, "emitted", "verify");

    json r;
    r["ok"] = validation.ok;
    r["run"] = run;
    r["stage"] = "emitted";
    r["document"] = state.string();
    r["research"] = (dir / "research.md").string();
    r["residual"] = (dir / "residual.json").string();
    r["provenance"] = (dir / "provenance.json").string();
    r["problems"] = validation.problems;
    r["next"] = "verify";
    reply = r.dump();
    return true;
}

bool status_op(const Args& a, std::string& reply) {
    std::string run = a.pos.empty() ? "" : a.pos.front();
    if (run.empty()) {
        std::error_code ec;
        const fs::path root = root_dir();
        json runs = json::array();
        if (fs::exists(root, ec)) {
            for (const auto& entry : fs::directory_iterator(root, ec)) {
                if (!entry.is_directory()) continue;
                json j;
                std::string error;
                if (read_json_file(entry.path() / "run.json", j, error)) runs.push_back(j);
            }
        }
        reply = json{{"ok", true}, {"runs", runs}}.dump();
        return true;
    }
    json run_json;
    std::string error;
    if (!read_json_file(run_dir(run) / "run.json", run_json, error)) {
        reply = err(error);
        return true;
    }
    run_json["ok"] = true;
    reply = run_json.dump();
    return true;
}

bool task_op(const Args& a, std::string& reply) {
    if (a.pos.empty()) {
        reply = err("effect molerat task needs <run>");
        return true;
    }
    const std::string run = a.pos.front();
    json run_json;
    std::string error;
    if (!read_json_file(run_dir(run) / "run.json", run_json, error)) {
        reply = err(error);
        return true;
    }
    const std::string stage = run_json.value("stage", "gathered");
    json task;
    task["id"] = run + "-" + stage;
    task["run"] = run;
    task["stage"] = stage;
    task["budget_bytes"] = 16384;
    task["answer_schema"] = {
        {"claims", json::array({json{{"claim", "string"}, {"evidence", "string"},
                                     {"confidence", "verified|strong|weak|asserted"},
                                     {"citations", json::array({"string"})}}})}};
    task["acceptance"] = json::array({
        "every claim has non-empty evidence",
        "every claim has at least one citation",
        "confidence is one of verified, strong, weak, asserted",
        "no code blocks and no game bytes",
        "the answer is under 16 KiB",
    });
    if (stage == "gathered") {
        task["question"] = "State the mathematical model of each function and table, then map each construct onto a Void shape. Name what does not map.";
    } else if (stage == "researched") {
        task["question"] = "Review the mappings: which Void glyph, kind, link and quantity carry each construct, and what is lost?";
    } else if (stage == "composed") {
        task["question"] = "Review the residual and the holiday contracts. Is any lost behaviour actually recoverable?";
    } else {
        task["question"] = "Review the emitted document. Are the runes, links and quantities faithful to the research records?";
    }
    reply = task.dump(2);
    return true;
}

bool answer_op(const Args& a, std::string& reply) {
    if (a.pos.size() < 2) {
        reply = err("effect molerat answer needs <run> '<json>'");
        return true;
    }
    const std::string run = a.pos[0];
    const std::string text = a.pos[1];
    if (text.size() > 16 * 1024) {
        reply = err("the answer is over the 16 KiB budget");
        return true;
    }
    json answer;
    try {
        answer = json::parse(text);
    } catch (const std::exception& e) {
        reply = err(std::string("the answer is not JSON: ") + e.what());
        return true;
    }
    if (!answer.is_object() || !answer.contains("claims") || !answer["claims"].is_array() ||
        answer["claims"].empty()) {
        reply = err("the answer needs a non-empty claims array");
        return true;
    }
    const std::set<std::string> allowed = {"verified", "strong", "weak", "asserted"};
    for (const json& claim : answer["claims"]) {
        if (!claim.is_object()) {
            reply = err("a claim is not an object");
            return true;
        }
        if (claim.value("evidence", "").empty()) {
            reply = err("a claim has no evidence");
            return true;
        }
        if (!allowed.count(claim.value("confidence", ""))) {
            reply = err("a claim has an invalid confidence");
            return true;
        }
        if (!claim.contains("citations") || !claim["citations"].is_array() ||
            claim["citations"].empty()) {
            reply = err("a claim has no citations");
            return true;
        }
    }
    const fs::path dir = run_dir(run) / "answers";
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        reply = err("cannot create '" + dir.string() + "': " + ec.message());
        return true;
    }
    size_t next = 1;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (entry.is_regular_file()) ++next;
    }
    char name[32];
    std::snprintf(name, sizeof name, "%03zu.json", next);
    std::string error;
    if (!write_json_file(dir / name, answer, error)) {
        reply = err(error);
        return true;
    }
    reply = json{{"ok", true}, {"run", run}, {"accepted", answer["claims"].size()},
                 {"answer", (dir / name).string()},
                 {"note", "the answer is stored and validated; applying it to the document is planned"}}
                .dump();
    return true;
}

bool verify_op(const Args& a, std::string& reply) {
    if (a.pos.empty()) {
        reply = err("effect molerat verify needs <run>");
        return true;
    }
    const std::string run = a.pos.front();
    squid::molerat::ForeignIR ir;
    json run_json;
    std::string error;
    if (!load_ir(run, ir, run_json, error)) {
        reply = err(error);
        return true;
    }
    squid::molerat::Research research;
    if (!load_research(run, research, error)) {
        reply = err(error);
        return true;
    }
    const std::string seed = run_json.value("seed", "seed");
    const squid::molerat::ComposeResult composed =
        squid::molerat::compose(ir, research, run, seed);
    if (!composed.ok) {
        reply = err(composed.error);
        return true;
    }
    std::string document = squid::molerat::emit_document(composed, ir, run, seed, error);
    if (document.empty()) {
        reply = err(error);
        return true;
    }
    const squid::molerat::Validation validation =
        squid::molerat::validate_document(document);

    json checks = json::array();
    auto check = [&](const std::string& name, bool ok, const std::string& detail) {
        checks.push_back(json{{"check", name}, {"ok", ok}, {"detail", detail}});
    };
    check("document_valid", validation.ok,
          validation.ok ? "the state document validates" : "see problems");
    check("residual_present", !research.residuals.empty(),
          std::to_string(research.residuals.size()) + " residual records");
    check("no_ir_embedded", document.find("molerat.foreign-ir") == std::string::npos,
          "the document carries records, not the IR");
    check("synthetic_declared",
          ir.target.origin.find("synthetic") != std::string::npos ||
              ir.target.platform.find("synthetic") != std::string::npos,
          ir.target.origin.empty() ? "origin not stated" : ir.target.origin);
    bool all = true;
    for (const json& c : checks) all = all && c.value("ok", false);

    json r;
    r["ok"] = all;
    r["run"] = run;
    r["checks"] = checks;
    r["problems"] = validation.problems;
    reply = r.dump(2);
    return true;
}

}  // namespace

std::vector<maiz::EffectOp> molerat_effect_ops() {
    return {
        {"molerat",
         "Void MoleRat: a decompilation-and-re-expression harness over a normalized foreign "
         "IR. `effect molerat gather <target> [--ir <path>] [--run <id>] [--seed <s>]` reads "
         "the IR and creates molerat/out/<run>/; `research <run>` writes the model, mapping, "
         "residual and holiday records; `compose <run>` maps them onto the <run>:game and "
         "molerat:<run> mantles; `residual <run>` reports what did not map; `build <run> "
         "[--out <dir>]` writes <run>.squid plus research.md, residual.json and "
         "provenance.json; `status`, `task`, `answer` and `verify` support the agent harness. "
         "No model is embedded; no game bytes leave the machine.",
         false,
         "gather reads a JSON IR; build writes the state document and its reports; nothing else is read or written"},
    };
}

bool molerat_effect(const std::vector<std::string>& args, std::string& reply) {
    if (args.empty()) {
        reply = err("effect molerat needs gather|research|compose|residual|build|status|task|answer|verify");
        return true;
    }
    const std::string op = args.front();
    const std::vector<std::string> rest(args.begin() + 1, args.end());
    const Args a = parse_args(rest);
    if (op == "gather") return gather(a, reply);
    if (op == "research") return research_op(a, reply);
    if (op == "compose") return compose_op(a, reply);
    if (op == "residual") return residual_op(a, reply);
    if (op == "build") return build_op(a, reply);
    if (op == "status") return status_op(a, reply);
    if (op == "task") return task_op(a, reply);
    if (op == "answer") return answer_op(a, reply);
    if (op == "verify") return verify_op(a, reply);
    reply = err("unknown molerat op '" + op +
                "'; known: gather, research, compose, residual, build, status, task, answer, verify");
    return true;
}

}  // namespace squid
