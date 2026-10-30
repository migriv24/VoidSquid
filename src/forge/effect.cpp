/* effect.cpp — the forge's effects: read a device library, plan a route, run the
 * match loop, compile a candidate, and place a shortcut.
 *
 * The host does the reads, writes and subprocess calls; the pure library
 * (src/forge/forge.hpp) does the computing. The compile-and-run verifier is the
 * first REAL verifier: it invokes a compiler and a runtime, so `effect forge match
 * --verifier compile` is an effect that actually runs. The synthetic verifier
 * remains the default and still says so.
 */
#include "forge/effect.hpp"

#include "forge/forge.hpp"

#include "json.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace squid {
namespace {

using json = nlohmann::json;
namespace fs = std::filesystem;

std::string err(const std::string& message) {
    return json{{"ok", false}, {"error", message}}.dump();
}

bool read_text(const fs::path& path, std::string& out, std::string& error) {
    std::error_code ec;
    const auto size = fs::file_size(path, ec);
    if (ec) {
        error = "cannot read '" + path.string() + "': " + ec.message();
        return false;
    }
    if (size > 8 * 1024 * 1024) {
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

struct Args {
    std::vector<std::string> pos;
    std::map<std::string, std::string> flags;
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

fs::path manifest_path(const std::string& p) {
    fs::path path(p);
    std::error_code ec;
    if (fs::is_directory(path, ec)) path = path / "device-library.json";
    return path;
}

bool load_library(const std::string& p, forge::DeviceLibrary& lib, std::string& error) {
    std::string text;
    if (!read_text(manifest_path(p), text, error)) return false;
    return forge::parse_device_library(text, lib, error);
}

/* Run a command through the shell; returns the process exit code. */
int run_silent(const std::string& cmd) {
    return std::system(cmd.c_str());
}

std::string op_devices(const Args& a) {
    if (a.pos.empty())
        return err("effect forge devices needs <manifest-dir-or-file>");
    forge::DeviceLibrary lib;
    std::string error;
    if (!load_library(a.pos[0], lib, error)) return err(error);

    json j;
    j["ok"] = true;
    j["count"] = lib.devices.size();
    j["devices"] = json::array();
    for (const forge::Device& d : lib.devices) {
        json dj;
        dj["id"] = d.id;
        dj["name"] = d.name;
        dj["cpu"] = d.cpu;
        dj["kind"] = d.kind;
        dj["toolchain"] = d.toolchain;
        dj["runtime"] = d.runtime;
        dj["subsystems"] = json::array();
        for (const forge::Subsystem& s : d.subsystems)
            dj["subsystems"].push_back({{"name", s.name}, {"impl", s.impl}});
        j["devices"].push_back(std::move(dj));
    }
    return j.dump();
}

std::string op_route(const Args& a) {
    if (a.pos.size() < 3)
        return err("effect forge route needs <manifest-dir-or-file> <source> <target>");
    forge::DeviceLibrary lib;
    std::string error;
    if (!load_library(a.pos[0], lib, error)) return err(error);
    const auto route = forge::plan_route(lib, a.pos[1], a.pos[2], error);
    if (!route) return err(error);

    json j;
    j["ok"] = true;
    j["source"] = route->source;
    j["target"] = route->target;
    j["toolchain"] = route->toolchain;
    j["runtime"] = route->runtime;
    j["steps"] = json::array();
    for (const forge::RouteStep& s : route->steps)
        j["steps"].push_back(
            {{"subsystem", s.subsystem}, {"source", s.source_impl}, {"target", s.target_impl}});
    return j.dump();
}

/* The logic registers: the behavioural half of a route, one row per register
 * (okf/design/logic-translation-layer.md). Pure data, like the route. */
std::string op_registers(const Args& a) {
    if (a.pos.size() < 3)
        return err("effect forge registers needs <manifest-dir-or-file> <source> <target>");
    forge::DeviceLibrary lib;
    std::string error;
    if (!load_library(a.pos[0], lib, error)) return err(error);
    const auto plans = forge::plan_registers(lib, a.pos[1], a.pos[2], error);
    if (plans.empty() && !error.empty()) return err(error);

    json j;
    j["ok"] = true;
    j["source"] = a.pos[1];
    j["target"] = a.pos[2];
    j["registers"] = json::array();
    for (const forge::RegisterPlan& p : plans)
        j["registers"].push_back(
            {{"name", p.name}, {"source", p.source_impl}, {"target", p.target_impl}});
    return j.dump();
}

/* The split stage: disassemble a PowerPC DOL into the normalized foreign IR.
 * Invokes the Capstone-based tool as a process holiday; the pure library never
 * sees the bytes. */
std::string op_split(const Args& a) {
    if (a.pos.empty())
        return err("effect forge split needs <main.dol> [--out <ir.json>] [--limit N]");
    const std::string dol = a.pos[0];
    const std::string out = a.flags.count("out") ? a.flags.at("out") : "forge/split.ir.json";
    const std::string limit = a.flags.count("limit") ? a.flags.at("limit") : "64";

    std::error_code ec;
    if (!fs::exists(dol, ec)) return err("cannot find DOL '" + dol + "'");

#ifndef SQUID_SOURCE_DIR
#define SQUID_SOURCE_DIR "."
#endif
    const fs::path tool = fs::path(SQUID_SOURCE_DIR) / "forge" / "toolbox" / "ppc_split.py";
    const std::string cmd = "python \"" + tool.string() + "\" \"" + dol + "\" --out \"" +
                            out + "\" --limit " + limit;

    FILE* pipe = ::popen(cmd.c_str(), "r");
    if (!pipe) return err("could not start the split tool");
    std::string got;
    char buf[1024];
    while (std::fgets(buf, sizeof buf, pipe)) got += buf;
    const int rc = ::pclose(pipe);
    if (got.empty())
        return err("split tool produced no output (exit " + std::to_string(rc) + ")");
    return got;
}

std::string op_match(const Args& a) {
    if (a.pos.empty())
        return err("effect forge match needs <task.json> [--manifest <dir-or-file>] "
                   "[--verifier synthetic|compile] [--candidate-dir <dir>] [--compiler g++]");

    std::string text;
    std::string error;
    if (!read_text(a.pos[0], text, error)) return err(error);
    json task;
    try {
        task = json::parse(text);
    } catch (const std::exception& e) {
        return err(std::string("task is not JSON: ") + e.what());
    }
    if (!task.is_object() || !task.contains("schema") ||
        task["schema"] != "forge.match-task/0.1")
        return err("schema must be forge.match-task/0.1");
    if (!task.contains("functions") || !task["functions"].is_array())
        return err("missing functions array");

    const std::string source = task.value("source", "");
    const std::string target = task.value("target", "");
    const int budget = task.value("budget", 8);
    const std::string verifier = a.flags.count("verifier") ? a.flags.at("verifier") : "synthetic";
    const std::string cand_dir = a.flags.count("candidate-dir") ? a.flags.at("candidate-dir") : "";
    const std::string compiler = a.flags.count("compiler") ? a.flags.at("compiler") : "g++";

    if (verifier == "compile" && cand_dir.empty())
        return err("--verifier compile needs --candidate-dir <dir>");

    /* The synthetic verifier: honest about there being no real toolchain behind it. */
    auto synthetic = [](const std::string& candidate) -> forge::MatchAttempt {
        forge::MatchAttempt a;
        a.candidate = candidate;
        if (candidate == "ok") {
            a.result = forge::AttemptResult::Match;
            a.evidence =
                "synthetic verifier (no real toolchain is invoked yet): 'ok' accepted";
        } else {
            a.result = forge::AttemptResult::Mismatch;
            a.evidence = "synthetic verifier (no real toolchain is invoked yet): '" +
                         candidate + "' mismatched";
        }
        return a;
    };

    json out;
    out["ok"] = true;
    out["source"] = source;
    out["target"] = target;
    out["verifier"] = verifier;
    out["outcomes"] = json::array();

    for (const json& f : task["functions"]) {
        if (!f.contains("name") || !f["name"].is_string() || !f.contains("candidates"))
            continue;
        std::vector<std::string> candidates;
        for (const json& c : f["candidates"])
            if (c.is_string()) candidates.push_back(c.get<std::string>());

        const std::string oracle_stdout =
            (f.contains("oracle") && f["oracle"].is_object())
                ? f["oracle"].value("stdout", "")
                : "";

        std::function<forge::MatchAttempt(const std::string&)> verifier_fn;
        if (verifier == "compile") {
            /* The REAL verifier: compile the candidate with a real compiler, run it,
             * and compare its stdout to the oracle. This is the effect the loop was
             * built around; the synthetic path remains for tests and for devices whose
             * toolchain is not on this machine. */
            verifier_fn = [&](const std::string& cand) -> forge::MatchAttempt {
                forge::MatchAttempt r;
                r.candidate = cand;
                const fs::path src = fs::path(cand_dir) / (cand + ".cpp");
                const fs::path exe = fs::temp_directory_path() / ("forge_" + cand + ".exe");
                std::error_code ec;
                if (!fs::exists(src, ec)) {
                    r.result = forge::AttemptResult::Error;
                    r.evidence = "candidate source not found: " + src.string();
                    return r;
                }
                const std::string build = compiler + " -O0 -std=c++17 \"" + src.string() +
                                          "\" -o \"" + exe.string() + "\"";
                int code = run_silent(build);
                if (code != 0) {
                    r.result = forge::AttemptResult::Error;
                    r.evidence = "candidate does not compile (compiler exit " +
                                 std::to_string(code) + ")";
                    return r;
                }
                std::string got;
                FILE* pipe = ::popen(("\"" + exe.string() + "\"").c_str(), "r");
                if (!pipe) {
                    r.result = forge::AttemptResult::Error;
                    r.evidence = "could not run candidate";
                    return r;
                }
                char buf[512];
                while (std::fgets(buf, sizeof buf, pipe)) got += buf;
                const int rc = ::pclose(pipe);
                if (rc != 0) {
                    r.result = forge::AttemptResult::Error;
                    r.evidence = "candidate ran with exit " + std::to_string(rc);
                    return r;
                }
                if (got == oracle_stdout) {
                    r.result = forge::AttemptResult::Match;
                    r.evidence = "compiled and ran; stdout matches the oracle";
                } else {
                    r.result = forge::AttemptResult::Mismatch;
                    std::string shown = got.substr(0, 64);
                    if (got.size() > 64) shown += "...";
                    r.evidence = "stdout differs: got \"" + shown + "\"";
                }
                return r;
            };
        } else {
            verifier_fn = synthetic;
        }

        const forge::MatchOutcome o =
            forge::run_match_loop(f["name"].get<std::string>(), candidates, budget, verifier_fn);
        json oj;
        oj["function"] = o.function;
        oj["verdict"] = o.verdict == forge::LoopVerdict::Accepted ? "accepted" : "rejected";
        oj["attempts"] = o.attempts;
        oj["accepted_candidate"] = o.accepted_candidate;
        oj["evidence"] = o.evidence;
        out["outcomes"].push_back(std::move(oj));
    }

    if (a.flags.count("manifest")) {
        forge::DeviceLibrary lib;
        if (load_library(a.flags.at("manifest"), lib, error)) {
            const auto route = forge::plan_route(lib, source, target, error);
            if (route) {
                json r;
                r["toolchain"] = route->toolchain;
                r["runtime"] = route->runtime;
                r["steps"] = json::array();
                for (const forge::RouteStep& s : route->steps)
                    r["steps"].push_back(
                        {{"subsystem", s.subsystem}, {"source", s.source_impl}, {"target", s.target_impl}});
                out["route"] = std::move(r);
            }
        }
    }
    return out.dump();
}

/* The emit+compile stage: one source file into a native executable. */
std::string op_compile(const Args& a) {
    if (a.pos.size() < 2)
        return err("effect forge compile needs <source.cpp> <out.exe> [--compiler g++]");
    const std::string source = a.pos[0];
    const std::string out_exe = a.pos[1];
    const std::string compiler = a.flags.count("compiler") ? a.flags.at("compiler") : "g++";

    std::error_code ec;
    if (!fs::exists(source, ec)) return err("cannot find source '" + source + "'");

    std::string build = compiler + " -O0 -std=c++17";
    if (a.flags.count("windowed")) build += " -mwindows -municode";   // GUI, Unicode entry
    build += " \"" + source + "\" -o \"" + out_exe + "\"";
    const int code = run_silent(build);
    if (code != 0) return err("compiler exited " + std::to_string(code));

    const auto size = fs::file_size(out_exe, ec);
    return json{{"ok", true}, {"exe", out_exe}, {"bytes", ec ? 0 : size}}.dump();
}

/* The package stage: place a Desktop shortcut to the built executable. */
std::string op_shortcut(const Args& a) {
    if (a.pos.size() < 2) return err("effect forge shortcut needs <exe-path> <name>");
    const std::string exe = a.pos[0];
    const std::string name = a.pos[1];
#ifdef _WIN32
    std::string cmd =
        "powershell -NoProfile -Command \""
        "$d=[Environment]::GetFolderPath('Desktop'); "
        "$p=Join-Path $d '" + name + ".lnk'; "
        "$s=(New-Object -ComObject WScript.Shell).CreateShortcut($p); "
        "$s.TargetPath='" + exe + "'; "
        "$s.Save()\"";
    const int code = run_silent(cmd);
    if (code != 0)
        return err("shortcut creation failed (exit " + std::to_string(code) + ")");
    return json{{"ok", true}, {"shortcut", name + ".lnk"},
                {"target", exe}, {"on", "the user's Desktop"}}.dump();
#else
    (void)exe;
    return err("shortcut creation is Windows-only");
#endif
}

}  // namespace

bool forge_effect(const std::vector<std::string>& args, std::string& reply) {
    if (args.empty()) return false;
    const std::string op = args[0];
    const Args a = parse_args(std::vector<std::string>(args.begin() + 1, args.end()));
    if (op == "devices")  { reply = op_devices(a);  return true; }
    if (op == "route")     { reply = op_route(a);     return true; }
    if (op == "registers") { reply = op_registers(a); return true; }
    if (op == "split")     { reply = op_split(a);     return true; }
    if (op == "match")    { reply = op_match(a);    return true; }
    if (op == "compile")  { reply = op_compile(a);  return true; }
    if (op == "shortcut") { reply = op_shortcut(a); return true; }
    return false;
}

}  // namespace squid
