/* sim.hpp — the model-rate simulation runtime. Links NOTHING but the C++ standard
 * library: no Void Core, no Void Jelly, no Void Maiz.
 *
 * okf/design/simulation.md is authoritative. The shape in one paragraph:
 *
 * A simulation is an ARTIFACT RUNNING UNDER OBSERVATION. The forge (squid_app)
 * compiles a world into inert data — places from Void Jelly's spatial queries,
 * behaviour from Void Allomone rules evaluated over the turkeys' tags — and hands
 * this runtime a `World`. The runtime steps it at a fixed dt, deterministically, and
 * reports EVENTS (entered a region, touched something, arrived, landed), never a
 * dump of positions. That is railguard §6 exactly: authored rules compile to data
 * the runtime reads in its own language, and no Void Core ships into it.
 *
 * ── Many instances at once ──
 *
 * The author: *"keep in mind if we want to simulate multiple instances at once."* The
 * design follows what parallel simulators converged on (Isaac Lab's sub-environments,
 * vectorised environments): ONE static `World`, shared and read-only, and many small
 * `Instance`s that own only their dynamic state (positions, velocities, flags, the
 * RNG, the event list). `step_all` steps every instance by the same dt. A `fork`
 * copies one instance's dynamic state — the copy-on-write idea, done by hand: the
 * world is never copied — so "what if this car were faster from tick 40" is one
 * fork and a parameter change.
 *
 * ── Determinism ──
 *
 * Same World, same seed, same dt, same tick count → the same events, byte for byte,
 * on any machine. There is no clock and no global RNG; each instance carries a
 * splitmix64 state. A fork with the same seed continues exactly as the original
 * would have; a fork with a new seed diverges only where the world uses randomness
 * (`jitter`). This is what makes a simulation a replayable experiment and makes two
 * instances comparable at all.
 */
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace squid::sim {

using Vec3 = std::array<double, 3>;

/* One body, as the forge compiled it. Every field is data; nothing here refers back
 * to a rune, a mantle or a script. `id` is carried only so events can be logged in
 * the forge's names. */
struct Body {
    std::string id;
    Vec3 center{};            // world, scaled units (metres)
    Vec3 half{};              // half-extents of its AABB
    bool dynamic = false;     // moves at all
    bool solid = false;       // blocks dynamic bodies
    bool region = false;      // a trigger volume: enter/leave events, never blocks
    bool gravity = false;     // falls until it rests on the ground or a solid
    double speed = 0;         // m/s toward `goal`
    double jitter = 0;        // per-instance speed variation, fraction (0.2 = ±20%)
    int goal = -1;            // index of the body it drives toward; -1 none
};

struct World {
    std::vector<Body> bodies;
    double ground_y = 0;      // the floor gravity bodies land on
    double g = 9.81;
};

struct Event {
    int64_t tick = 0;
    std::string kind;         // enter | leave | contact | arrive | land
    int body = -1;
    int other = -1;           // the region, solid or goal; -1 for land on the ground
    bool operator==(const Event&) const = default;
};

struct Instance {
    uint64_t seed = 0;
    uint64_t rng = 0;
    int64_t tick = 0;
    std::vector<Vec3> pos;              // per body
    std::vector<Vec3> vel;
    std::vector<double> speed;          // jittered once at creation
    std::vector<uint8_t> arrived, resting, blocked;
    std::vector<uint8_t> inside;        // bodies × bodies: dynamic i inside region j
    std::vector<Event> events;          // every event so far, in order
    bool operator==(const Instance&) const = default;
};

/* A fresh instance: positions from the world, speeds jittered from the seed. */
Instance make_instance(const World& world, uint64_t seed);

/* One fixed step of `dt` seconds. */
void step(const World& world, Instance& inst, double dt);

/* Every instance, `ticks` times. Instances are independent, so this is the loop a
 * future parallel or GPU runner replaces without changing anything else. */
void step_all(const World& world, std::vector<Instance>& instances, double dt, int ticks);

/* Copy one instance's dynamic state. `reseed` true re-rolls the jittered speeds from
 * `new_seed` (a different what-if); false continues bit-identically. */
Instance fork(const World& world, const Instance& from, uint64_t new_seed, bool reseed);

/* How many events of this kind, optionally only for one body. */
size_t count(const Instance& inst, const std::string& kind, int body = -1);

/* ── the log, bounded ── */

/* One instance: tick, counts per event kind, and the LAST `limit` events, in the
 * forge's names. Its size does not grow with the run's length. */
std::string instance_json(const World& world, const Instance& inst, size_t limit = 20);

/* Several instances side by side: per instance the tick, event counts and when each
 * body arrived; then where they diverge (the first tick their event lists differ).
 * The comparison is the reason to run many instances at all. */
std::string compare_json(const World& world, const std::vector<Instance>& instances);

}  // namespace squid::sim
