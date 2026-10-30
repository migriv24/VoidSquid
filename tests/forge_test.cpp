/* forge_test.cpp — the forge's pure half, with no filesystem, network, display
 * or clock. What it checks, in the order each would be most expensive to be
 * wrong:
 *   1. The device library parses, and a wrong schema is refused.
 *   2. A library round-trips through its JSON form.
 *   3. plan_route emits the cpu bridge and the per-subsystem remaps.
 *   4. A missing source or target is refused.
 *   5. A source subsystem the target lacks becomes "reimplement".
 *   6. The match loop accepts the first Match, rejects after the budget, and
 *      rejects an empty candidate list.
 *
 * No framework; plain checks, like the rest of the family.
 */
#include "forge/forge.hpp"

#include <cstdio>
#include <string>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) { ++failures; std::printf("  FAIL  %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

static const char* kLibrary = R"JSON({
  "schema": "forge.device-library/0.1",
  "devices": [
    {"id": "wii", "name": "Nintendo Wii", "cpu": "powerpc-broadway", "kind": "source",
     "toolchain": "mwcceppc", "runtime": "dolphin",
     "subsystems": [{"name": "gpu", "impl": "GX FIFO"}, {"name": "audio", "impl": "AX + DSP"}],
     "registers": [{"name": "physics", "impl": "planet gravity"}, {"name": "rules", "impl": "decomp"}]},
    {"id": "pc", "name": "PC", "cpu": "x86-64", "kind": "target",
     "toolchain": "clang++", "runtime": "native",
     "subsystems": [{"name": "gpu", "impl": "OpenGL"}, {"name": "audio", "impl": "WASAPI"}],
     "registers": [{"name": "physics", "impl": "ocean gravity"}]},
    {"id": "gb", "name": "Game Boy", "cpu": "sm83", "kind": "source",
     "toolchain": "rgbds", "runtime": "bgb",
     "subsystems": [{"name": "gpu", "impl": "PPU"}],
     "registers": [{"name": "physics", "impl": "flat gravity"}]}
  ]
})JSON";

int main() {
    using namespace squid::forge;
    std::string error;

    /* 1. Parse and refuse. */
    DeviceLibrary lib;
    CHECK(parse_device_library(kLibrary, lib, error));
    CHECK(lib.devices.size() == 3);
    CHECK(lib.devices[0].id == "wii");
    CHECK(lib.devices[0].cpu == "powerpc-broadway");
    CHECK(lib.devices[0].subsystems.size() == 2);
    CHECK(lib.devices[0].registers.size() == 2);
    {
        DeviceLibrary bad;
        CHECK(!parse_device_library("not json", bad, error));
        CHECK(!parse_device_library("{\"schema\":\"other/1\"}", bad, error));
        CHECK(!parse_device_library("{\"schema\":\"forge.device-library/0.1\"}", bad, error));
    }

    /* 2. Round-trip. */
    {
        DeviceLibrary back;
        CHECK(parse_device_library(device_library_to_json(lib), back, error));
        CHECK(back.devices.size() == 3);
        CHECK(back.devices[0].id == "wii");
    }

    /* 3. A route emits the cpu bridge and the subsystem remaps. */
    {
        const auto route = plan_route(lib, "wii", "pc", error);
        CHECK(route.has_value());
        bool cpu = false, gpu = false, audio = false;
        if (route) {
            CHECK(route->steps.size() == 3);   // cpu, gpu, audio
            for (const RouteStep& s : route->steps) {
                if (s.subsystem == "cpu" && s.source_impl == "powerpc-broadway" &&
                    s.target_impl == "x86-64") cpu = true;
                if (s.subsystem == "gpu" && s.source_impl == "GX FIFO" &&
                    s.target_impl == "OpenGL") gpu = true;
                if (s.subsystem == "audio" && s.source_impl == "AX + DSP" &&
                    s.target_impl == "WASAPI") audio = true;
            }
        }
        CHECK(cpu);
        CHECK(gpu);
        CHECK(audio);
    }

    /* 4. A missing device is refused. */
    {
        const auto route = plan_route(lib, "wii", "dreamcast", error);
        CHECK(!route.has_value());
    }

    /* 5. A source subsystem the target lacks becomes "reimplement". */
    {
        const auto route = plan_route(lib, "pc", "gb", error);
        CHECK(route.has_value());
        bool reimplemented = false;
        if (route) {
            for (const RouteStep& s : route->steps)
                if (s.subsystem == "audio" && s.target_impl == "reimplement") reimplemented = true;
        }
        CHECK(reimplemented);
    }

    /* 5b. The logic registers (okf/design/logic-translation-layer.md). */
    {
        const auto plans = plan_registers(lib, "wii", "pc", error);
        CHECK(plans.size() == 2);
        bool physics = false, rules = false;
        for (const RegisterPlan& p : plans) {
            if (p.name == "physics" && p.target_impl == "ocean gravity") physics = true;
            if (p.name == "rules" && p.target_impl == "reimplement") rules = true;
        }
        CHECK(physics);
        CHECK(rules);
        const auto missing = plan_registers(lib, "wii", "dreamcast", error);
        CHECK(missing.empty());
    }

    /* 6. The match loop. */
    int calls = 0;
    auto verifier = [&](const std::string& c) -> MatchAttempt {
        ++calls;
        MatchAttempt a;
        a.candidate = c;
        if (c == "ok") {
            a.result = AttemptResult::Match;
            a.evidence = "matched";
        } else {
            a.result = AttemptResult::Mismatch;
            a.evidence = "diff";
        }
        return a;
    };

    {
        const MatchOutcome o = run_match_loop("f", {"wip", "ok"}, 8, verifier);
        CHECK(o.verdict == LoopVerdict::Accepted);
        CHECK(o.attempts == 2);
        CHECK(o.accepted_candidate == "ok");
        CHECK(calls == 2);
    }
    {
        const MatchOutcome o = run_match_loop("g", {"wip1", "wip2"}, 8, verifier);
        CHECK(o.verdict == LoopVerdict::Rejected);
        CHECK(o.attempts == 2);
    }
    {
        const MatchOutcome o = run_match_loop("h", {"wip", "ok"}, 1, verifier);
        CHECK(o.verdict == LoopVerdict::Rejected);
        CHECK(o.attempts == 1);
    }
    {
        const MatchOutcome o = run_match_loop("i", {}, 8, verifier);
        CHECK(o.verdict == LoopVerdict::Rejected);
    }

    if (failures) {
        std::printf("%d forge test(s) failed\n", failures);
        return 1;
    }
    std::printf("forge_test: all checks passed\n");
    return 0;
}
