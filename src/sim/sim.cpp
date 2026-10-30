/* sim.cpp — see sim.hpp. Standard library only. */
#include "sim.hpp"

#include <algorithm>
#include <cmath>
#include <map>

namespace squid::sim {
namespace {

uint64_t splitmix(uint64_t& s) {
    uint64_t z = (s += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
double unit(uint64_t& s) { return (splitmix(s) >> 11) * (1.0 / 9007199254740992.0); }

bool overlap(const Vec3& ca, const Vec3& ha, const Vec3& cb, const Vec3& hb) {
    for (int k = 0; k < 3; ++k)
        if (std::fabs(ca[k] - cb[k]) >= ha[k] + hb[k] - 1e-9) return false;
    return true;
}
bool inside_xz(const Vec3& p, const Vec3& c, const Vec3& h) {
    return std::fabs(p[0] - c[0]) <= h[0] && std::fabs(p[2] - c[2]) <= h[2];
}
bool inside3(const Vec3& p, const Vec3& c, const Vec3& h) {
    return inside_xz(p, c, h) && std::fabs(p[1] - c[1]) <= h[1];
}

std::string str(const std::string& s) {
    std::string o = "\"";
    for (char c : s) { if (c == '"' || c == '\\') o += '\\'; o += c; }
    return o + "\"";
}
std::string name(const World& w, int i) { return i >= 0 && i < (int)w.bodies.size() ? w.bodies[i].id : ""; }

void jitter_speeds(const World& world, Instance& inst) {
    uint64_t s = inst.seed ^ 0xA5A5A5A5DEADBEEFull;
    inst.speed.resize(world.bodies.size());
    for (size_t i = 0; i < world.bodies.size(); ++i) {
        const Body& b = world.bodies[i];
        const double r = unit(s) * 2 - 1;    // drawn for every body, so indices stay aligned
        inst.speed[i] = b.speed * (1 + b.jitter * r);
    }
}

}  // namespace

Instance make_instance(const World& world, uint64_t seed) {
    const size_t n = world.bodies.size();
    Instance inst;
    inst.seed = seed;
    inst.rng = seed;
    inst.pos.resize(n);
    inst.vel.assign(n, Vec3{0, 0, 0});
    inst.arrived.assign(n, 0);
    inst.resting.assign(n, 0);
    inst.blocked.assign(n, 0);
    inst.inside.assign(n * n, 0);
    for (size_t i = 0; i < n; ++i) inst.pos[i] = world.bodies[i].center;
    jitter_speeds(world, inst);
    /* Regions already containing a body at tick 0 count as entered, silently: an
     * `enter` means a change, and nothing changed yet. */
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            if (i != j && world.bodies[i].dynamic && world.bodies[j].region &&
                inside3(inst.pos[i], world.bodies[j].center, world.bodies[j].half))
                inst.inside[i * n + j] = 1;
    return inst;
}

void step(const World& world, Instance& inst, double dt) {
    const size_t n = world.bodies.size();
    ++inst.tick;

    for (size_t i = 0; i < n; ++i) {
        const Body& b = world.bodies[i];
        if (!b.dynamic) continue;
        Vec3 next = inst.pos[i];

        /* Drive toward the NEAREST POINT of the goal's footprint, in the ground plane.
         * Aiming at the centre made a car in the far lane cut diagonally across the
         * near one (found 2026-10-10 by the walls-off test); the nearest point is lane
         * keeping for free when the car is already level with the goal. */
        if (b.goal >= 0 && !inst.arrived[i] && inst.speed[i] > 0) {
            const Body& gb = world.bodies[b.goal];
            const double tx = std::clamp(next[0], gb.center[0] - gb.half[0], gb.center[0] + gb.half[0]);
            const double tz = std::clamp(next[2], gb.center[2] - gb.half[2], gb.center[2] + gb.half[2]);
            const double dx = tx - next[0], dz = tz - next[2];
            const double d = std::sqrt(dx * dx + dz * dz);
            const double stride = std::min(d, inst.speed[i] * dt);
            if (d > 1e-12) { next[0] += dx / d * stride; next[2] += dz / d * stride; }
        }

        /* Fall. */
        if (b.gravity && !inst.resting[i]) {
            inst.vel[i][1] -= world.g * dt;
            next[1] += inst.vel[i][1] * dt;
        }

        /* Land on the ground. */
        if (b.gravity && !inst.resting[i] && next[1] - b.half[1] <= world.ground_y) {
            next[1] = world.ground_y + b.half[1];
            inst.vel[i][1] = 0;
            inst.resting[i] = 1;
            inst.events.push_back({inst.tick, "land", (int)i, -1});
        }

        /* Solids block. A blocked body advances to the point of contact (a short
         * bisection along its move) instead of stopping short, so a faster car behind
         * a slower one FOLLOWS it rather than bumping, stopping and bumping again. It
         * stays `blocked` while it follows, and one blocking episode is one `contact`
         * (found 2026-10-10: before this, one queue logged 25 contacts in 10 s). */
        auto first_hit = [&](const Vec3& at) {
            for (size_t j = 0; j < n; ++j) {
                if (j == i || !world.bodies[j].solid) continue;
                if (overlap(at, b.half, inst.pos[j], world.bodies[j].half) &&
                    !overlap(inst.pos[i], b.half, inst.pos[j], world.bodies[j].half))
                    return (int)j;
            }
            return -1;
        };
        const int hit = first_hit(next);
        if (hit >= 0) {
            if (b.gravity && next[1] < inst.pos[i][1]) {   // landed on top of a solid
                inst.vel[i][1] = 0;
                inst.resting[i] = 1;
                inst.events.push_back({inst.tick, "land", (int)i, hit});
                continue;
            }
            double lo = 0, hi = 1;
            for (int k = 0; k < 12; ++k) {
                const double mid = (lo + hi) / 2;
                Vec3 p;
                for (int a = 0; a < 3; ++a) p[a] = inst.pos[i][a] + (next[a] - inst.pos[i][a]) * mid;
                (first_hit(p) < 0 ? lo : hi) = mid;
            }
            for (int a = 0; a < 3; ++a) inst.pos[i][a] += (next[a] - inst.pos[i][a]) * lo;
            if (!inst.blocked[i]) {
                inst.blocked[i] = 1;
                inst.events.push_back({inst.tick, "contact", (int)i, hit});
            }
        } else {
            /* Free: the episode ends only when the body has moved a full stride with
             * nothing in the way. */
            inst.blocked[i] = 0;
            inst.pos[i] = next;
        }

        if (b.goal >= 0 && !inst.arrived[i] &&
            inside_xz(inst.pos[i], world.bodies[b.goal].center, world.bodies[b.goal].half)) {
            inst.arrived[i] = 1;
            inst.events.push_back({inst.tick, "arrive", (int)i, b.goal});
        }
    }

    /* Regions: enter and leave are changes of containment. */
    for (size_t i = 0; i < n; ++i) {
        if (!world.bodies[i].dynamic) continue;
        for (size_t j = 0; j < n; ++j) {
            if (j == i || !world.bodies[j].region) continue;
            const uint8_t now = inside3(inst.pos[i], world.bodies[j].center, world.bodies[j].half) ? 1 : 0;
            uint8_t& was = inst.inside[i * n + j];
            if (now != was) inst.events.push_back({inst.tick, now ? "enter" : "leave", (int)i, (int)j});
            was = now;
        }
    }
}

void step_all(const World& world, std::vector<Instance>& instances, double dt, int ticks) {
    for (Instance& inst : instances)
        for (int t = 0; t < ticks; ++t) step(world, inst, dt);
}

Instance fork(const World& world, const Instance& from, uint64_t new_seed, bool reseed) {
    Instance out = from;
    if (reseed) {
        out.seed = new_seed;
        out.rng = new_seed;
        jitter_speeds(world, out);
    }
    return out;
}

size_t count(const Instance& inst, const std::string& kind, int body) {
    size_t c = 0;
    for (const Event& e : inst.events) if (e.kind == kind && (body < 0 || e.body == body)) ++c;
    return c;
}

std::string instance_json(const World& world, const Instance& inst, size_t limit) {
    std::map<std::string, size_t> counts;
    for (const Event& e : inst.events) ++counts[e.kind];
    std::string j = "{\"seed\":" + std::to_string(inst.seed) + ",\"tick\":" + std::to_string(inst.tick);
    j += ",\"event_total\":" + std::to_string(inst.events.size()) + ",\"counts\":{";
    bool first = true;
    for (const auto& [k, c] : counts) { if (!first) j += ','; first = false; j += str(k) + ":" + std::to_string(c); }
    j += "},\"last\":[";
    const size_t from = inst.events.size() > limit ? inst.events.size() - limit : 0;
    for (size_t i = from; i < inst.events.size(); ++i) {
        const Event& e = inst.events[i];
        if (i > from) j += ',';
        j += "{\"tick\":" + std::to_string(e.tick) + ",\"kind\":" + str(e.kind) + ",\"body\":" + str(name(world, e.body));
        if (e.other >= 0) j += ",\"other\":" + str(name(world, e.other));
        j += "}";
    }
    j += "]}";
    return j;
}

std::string compare_json(const World& world, const std::vector<Instance>& instances) {
    std::string j = "{\"instances\":" + std::to_string(instances.size()) + ",\"runs\":[";
    for (size_t k = 0; k < instances.size(); ++k) {
        const Instance& inst = instances[k];
        if (k) j += ',';
        j += "{\"index\":" + std::to_string(k) + ",\"seed\":" + std::to_string(inst.seed) +
             ",\"tick\":" + std::to_string(inst.tick) + ",\"events\":" + std::to_string(inst.events.size()) + ",\"arrived\":{";
        bool first = true;
        for (const Event& e : inst.events) {
            if (e.kind != "arrive") continue;
            if (!first) j += ',';
            first = false;
            j += str(name(world, e.body)) + ":" + std::to_string(e.tick);
        }
        j += "},\"contacts\":" + std::to_string(count(inst, "contact")) + "}";
    }
    j += "]";
    /* Where the runs part ways: the first event index at which any differs from run 0. */
    if (instances.size() > 1) {
        int64_t first_diff_tick = -1;
        const auto& base = instances[0].events;
        for (size_t k = 1; k < instances.size(); ++k) {
            const auto& ev = instances[k].events;
            const size_t m = std::min(base.size(), ev.size());
            size_t i = 0;
            while (i < m && base[i] == ev[i]) ++i;
            if (i < m || base.size() != ev.size()) {
                const int64_t t = i < m ? std::min(base[i].tick, ev[i].tick)
                                        : (i < base.size() ? base[i].tick : ev[i].tick);
                if (first_diff_tick < 0 || t < first_diff_tick) first_diff_tick = t;
            }
        }
        j += ",\"identical\":" + std::string(first_diff_tick < 0 ? "true" : "false");
        if (first_diff_tick >= 0) j += ",\"first_divergence_tick\":" + std::to_string(first_diff_tick);
    }
    j += "}";
    return j;
}

}  // namespace squid::sim
