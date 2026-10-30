/* forge.hpp — the pure half of the forge: the device library and the match loop.
 *
 * okf/concepts/forge.md.
 *
 * The author's direction (2026-10-28): decompilation is not "game -> PC". It is
 * "game -> any device". A game runs on a source device (a Wii, an N64, an OS); a
 * port re-expresses it for a target device (a PC, Android, another console). This
 * library names both ends of that arrow as first-class *devices*, plans the
 * subsystem bridges between them, and runs the *match loop* that decides, per
 * function, whether a candidate translation is accepted.
 *
 * ── Pure, like squid_read, squid_sim and squid_molerat ──
 *
 * This target links the standard library and the vendored JSON header, and nothing
 * else: no Void Core, no Void Jelly, no Void Maiz, no filesystem, no network, no
 * clock. The host (src/forge/effect.cpp) reads the device manifests and writes the
 * reports. The match loop's verifier is injected as a function: the compile-and-run
 * that actually checks a candidate lives outside this library, because it is an
 * effect (it invokes a toolchain or a runtime).
 */
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace squid::forge {

/* ── the device library ───────────────────────────────────────────────────── */

struct Subsystem {
    std::string name;   // canonical: cpu | gpu | audio | input | storage | os | ...
    std::string impl;   // this device's implementation ("GX FIFO", "OpenGL / Vulkan")
};

/* A logic register: one of the finite decomposition of game logic
 * (okf/design/logic-translation-layer.md). The code register is the subsystems
 * above; these are the behavioural ones. */
struct Register {
    std::string name;   // physics | motion | rules | presentation | narrative
    std::string impl;   // this device's implementation of it
};

struct Device {
    std::string id;         // "wii", "pc-x86_64-windows", "android-arm64", ...
    std::string name;
    std::string cpu;        // the instruction set architecture
    std::string kind;       // source | target | both
    std::string toolchain;  // how to build for it (a command, not a dependency)
    std::string runtime;    // how to run/verify on it
    std::vector<Subsystem> subsystems;
    std::vector<Register> registers;
};

struct DeviceLibrary {
    std::vector<Device> devices;
};

/* Parse a `forge.device-library/0.1` manifest. Refuses a wrong schema. */
bool parse_device_library(const std::string& json_text, DeviceLibrary& lib,
                          std::string& error);

std::string device_library_to_json(const DeviceLibrary& lib);

struct RouteStep {
    std::string subsystem;
    std::string source_impl;
    std::string target_impl;   // empty => reimplement on the target
};

struct Route {
    std::string source;
    std::string target;
    std::string cpu_source;
    std::string cpu_target;
    std::string toolchain;   // the target's
    std::string runtime;     // the target's
    std::vector<RouteStep> steps;
};

/* Resolve source -> target into a bridge plan. Returns nullopt and fills `error`
 * when either device is unknown. The cpu step is always emitted when the ISAs
 * differ, because the code must be recompiled or binary-translated either way. */
std::optional<Route> plan_route(const DeviceLibrary& lib,
                                const std::string& source,
                                const std::string& target,
                                std::string& error);

struct RegisterPlan {
    std::string name;
    std::string source_impl;
    std::string target_impl;   // empty => reimplement on the target
};

/* The behavioural half of a route: for each source register, the target's
 * implementation, or "reimplement" when the target has none. This is the
 * "compiler for a generic abstract device" at the logic level, per register. */
std::vector<RegisterPlan> plan_registers(const DeviceLibrary& lib,
                                         const std::string& source,
                                         const std::string& target,
                                         std::string& error);

/* ── the match loop ───────────────────────────────────────────────────────── */

enum class AttemptResult { Match, Mismatch, Error };

struct MatchAttempt {
    std::string candidate;
    AttemptResult result = AttemptResult::Error;
    std::string evidence;
};

enum class LoopVerdict { Accepted, Rejected, Continue };

struct MatchOutcome {
    std::string function;
    LoopVerdict verdict = LoopVerdict::Continue;
    int attempts = 0;
    std::string accepted_candidate;
    std::string evidence;
};

/* The verifier is injected: it compiles and compares one candidate and reports
 * Match / Mismatch / Error with evidence. The loop owns only the policy: try
 * candidates in order, accept the first Match, and reject when the budget is
 * spent or the candidate list runs out. A budget of 0 or less means "no cap". */
MatchOutcome run_match_loop(
    const std::string& function,
    const std::vector<std::string>& candidates,
    int budget,
    const std::function<MatchAttempt(const std::string& candidate)>& verifier);

}  // namespace squid::forge
