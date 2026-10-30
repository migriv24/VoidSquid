/* motion/effect.cpp — see effect.hpp. */
#include "motion/effect.hpp"

#include "effects.hpp"
#include "host/process.hpp"
#include "motion/judge.hpp"

#include "json.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace squid::motion {

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

std::string err(const std::string& m) { return json{{"error", m}}.dump(); }

json read_json(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return nullptr;
    std::ostringstream s;
    s << in.rdbuf();
    return json::parse(s.str(), nullptr, false);   // discarded (not an exception) when broken
}

}  // namespace

maiz::EffectOp effect_op() {
    return {"motion",
            "Motion, judged by Void Onda in the runtime. `effect motion judge <library-folder> [out-folder]` starts the "
            "runtime's judge on the library (every character's clips rendered unlit and checked from their pixels) and "
            "returns at once; `effect motion report <folder>` reads the judge.json and pace.json it wrote and answers how "
            "many clips, their mean and lowest, and the checks under 80%. The gallery is Play, then G.",
            false,
            "judge starts a program that opens its own window and writes judge.json, pace.json and one picture per clip "
            "in out-folder (default <library>/judge), removing that folder's previous judge.json and pace.json; "
            "report reads two files and writes nothing"};
}

void effect(const std::vector<std::string>& args, std::string& reply) {
    auto arg = [&](size_t i) { return i < args.size() ? args[i] : std::string(); };

    if (arg(0) == "judge") {
        if (arg(1).empty()) { reply = err("effect motion judge needs the library folder"); return; }
        const std::string rt = runtime_dir();
        if (rt.empty()) {
            reply = err("the runtime was not found: set VOIDSQUID_RUNTIME to the folder holding goomba_galaxy2, or build ../ds-mashup");
            return;
        }
        const fs::path out = arg(2).empty() ? fs::path(arg(1)) / "judge" : fs::path(arg(2));
        std::error_code ec;
        fs::create_directories(out, ec);
        if (ec) { reply = err("could not make " + out.string() + ": " + ec.message()); return; }
        fs::remove(out / "judge.json", ec);
        fs::remove(out / "pace.json", ec);
#ifdef _WIN32
        const fs::path exe = fs::path(rt) / "goomba_galaxy2.exe";
#else
        const fs::path exe = fs::path(rt) / "goomba_galaxy2";
#endif
        const bool ok = host::launch({exe.string(), arg(1), "--autotest", "judge"}, out.string());
        reply = ok ? json{{"started", "goomba_galaxy2 --autotest judge"}, {"library", arg(1)}, {"out", out.string()},
                          {"report", (out / "judge.json").string()},
                          {"next", "`effect motion report " + out.string() + "` once judge.json exists"}}.dump()
                   : err("could not start " + exe.string());
        return;
    }
    if (arg(0) == "report") {
        if (arg(1).empty()) { reply = err("effect motion report needs the folder the judge wrote in"); return; }
        const fs::path dir = arg(1);
        const json judge = read_json(dir / "judge.json");
        if (judge.is_null()) {
            reply = err("no judge.json in " + dir.string() + " (yet): `effect motion judge <library> " + dir.string() + "` writes one");
            return;
        }
        if (!judge.is_array()) { reply = err((dir / "judge.json").string() + " is not the runtime's report (not a JSON array)"); return; }
        json pace = read_json(dir / "pace.json");
        json s = summarize(judge, pace.is_array() ? pace : json(nullptr));
        s["folder"] = dir.string();
        reply = s.dump();
        return;
    }
    reply = err("effect motion needs judge <library-folder> [out-folder] | report <folder>");
}

}  // namespace squid::motion
