/* scale_bench.cpp — how the pipeline and the spatial log behave as a world grows.
 *
 * Two questions this bundle had deferred, answered with one measurement:
 *
 *   1. Scope claim 4: is emission cheap enough to sit in an interactive loop?
 *   2. The author's 2026-10-09 question: in a world of thousands of objects, what
 *      does a LOG cost? Not milliseconds only — bytes, because a log an agent reads
 *      is context, and context is the scarce resource.
 *
 * It builds a synthetic world of N objects (a few instanced meshes on a grid, tagged
 * by region and kind), then times write/read of the .ink, the GL emission, and the
 * three spatial queries, and reports the size of each answer. Synthetic on purpose:
 * no real scene this size exists until the Polyp does, and a synthetic world is the
 * honest stand-in as long as it is labelled as one.
 *
 *     squid_scale_bench [N ...]        default: 100 1000 10000
 */
#include "camera.hpp"
#include "sim/sim.hpp"

#include "voidjelly/format.hpp"
#include "voidjelly/nibs/gl.hpp"
#include "voidjelly/spatial.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace voidjelly;
using Clock = std::chrono::steady_clock;

namespace {

Mesh cube() {
    Mesh m;
    m.positions = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
                   {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};
    m.faces = {{{0, 3, 2, 1}}, {{4, 5, 6, 7}}, {{0, 1, 5, 4}}, {{2, 3, 7, 6}}, {{0, 4, 7, 3}}, {{1, 2, 6, 5}}};
    return m;
}

/* A rough sphere-ish "tree" with many quads, so not every mesh is tiny. */
Mesh lump(int rings) {
    Mesh m;
    const double pi = 3.14159265358979323846;
    for (int r = 0; r <= rings; ++r)
        for (int s = 0; s < rings; ++s) {
            const double th = pi * r / rings, ph = 2 * pi * s / rings;
            m.positions.push_back({std::sin(th) * std::cos(ph), std::cos(th) * 2, std::sin(th) * std::sin(ph)});
        }
    for (int r = 0; r < rings; ++r)
        for (int s = 0; s < rings; ++s) {
            const uint32_t a = r * rings + s, b = r * rings + (s + 1) % rings;
            m.faces.push_back({{a, b, b + static_cast<uint32_t>(rings), a + static_cast<uint32_t>(rings)}});
        }
    return m;
}

Ink world(size_t n) {
    Ink ink;
    ink.name = "bench-world";
    ink.unit_scale = 1.0;
    const BlockRef meshes[3] = {ink.put_mesh(cube()), ink.put_mesh(lump(8)), ink.put_mesh(lump(16))};
    const char* kinds[3] = {"rock", "bush", "tree"};
    const size_t side = static_cast<size_t>(std::ceil(std::sqrt(static_cast<double>(n))));
    for (size_t i = 0; i < n; ++i) {
        Object o;
        o.id = "obj_" + std::to_string(i);
        o.rune_id = "rune_bench_" + std::to_string(i);
        o.name = o.id;
        const size_t x = i % side, z = i / side;
        o.transform.m[3] = static_cast<double>(x) * 4.0;
        o.transform.m[11] = static_cast<double>(z) * 4.0;
        o.mesh = meshes[i % 3];
        o.material = kinds[i % 3];
        o.tags = {{"kind", kinds[i % 3]}, {"region", "r" + std::to_string((x / 25) + 10 * (z / 25))}};
        ink.objects.push_back(std::move(o));
    }
    return ink;
}

double ms_since(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

template <class F>
double time_ms(F&& f, int reps = 3) {
    double best = 1e300;
    for (int i = 0; i < reps; ++i) {
        const auto t0 = Clock::now();
        f();
        best = std::min(best, ms_since(t0));
    }
    return best;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<size_t> sizes;
    for (int i = 1; i < argc; ++i) sizes.push_back(static_cast<size_t>(std::strtoull(argv[i], nullptr, 10)));
    if (sizes.empty()) sizes = {100, 1000, 10000};

    std::printf("synthetic world: instanced cube / 64-quad lump / 256-quad lump on a 4 m grid; best of 3\n\n");
    std::printf("%8s %9s %10s %9s %9s %9s %9s %9s %9s %9s | %10s %9s %10s\n",
                "objects", "polygons", "ink_bytes", "write_ms", "read_ms", "emit_ms", "emit_h_ms",
                "survey_ms", "near_ms", "look_ms", "survey_B", "near_B", "look20_B");

    for (const size_t n : sizes) {
        const Ink ink = world(n);
        std::vector<uint8_t> bytes;
        const double w = time_ms([&] { bytes = write_ink(ink); });
        ReadResult rr;
        const double r = time_ms([&] { rr = read_ink(bytes); });
        GlEmission e;
        const double em = time_ms([&] { e = emit_gl(ink); });
        const std::string hash = e.drawlist.ink;
        const double em_h = time_ms([&] { e = emit_gl(ink, GlProfile::standard(), hash); });

        Survey sv;
        const double su = time_ms([&] { sv = survey(ink, 8); });
        const std::string sj = survey_json(sv, ink.unit_scale);

        const Vec3 mid = sv.bounds.center();
        std::vector<Nearby> nb;
        const double ne = time_ms([&] { nb = near(ink, mid, 10.0, 20); });
        const std::string nj = near_json(nb, mid, 10.0);

        const squid::Camera3D cam = squid::frame_all(e.drawlist, squid::Camera3D{});
        const LookCamera lc = cam.to_look(16.0 / 10.0);
        LookResult lr;
        const double lo = time_ms([&] { lr = look(ink, lc, 20); });
        const std::string lj = look_json(lr, lc);

        std::printf("%8zu %9zu %10zu %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f | %10zu %9zu %10zu\n",
                    n, ink.polygon_count(), bytes.size(), w, r, em, em_h, su, ne, lo, sj.size(), nj.size(), lj.size());
        if (!rr.ok) { std::printf("read failed: %s\n", rr.error.c_str()); return 1; }
    }
    std::printf("\nlook20_B is one `effect look` answer truncated to 20 objects; it does not grow with the world.\n");

    /* ── Many instances (okf/design/simulation.md) ──
     * A world of N cars in N/10 lanes driving to one goal strip, K instances, 100
     * ticks each. Reports body-steps per second and the bytes of one `sim compare`. */
    std::printf("\nsimulation: K instances x N bodies x 100 ticks, single thread\n");
    std::printf("%8s %10s %12s %16s %12s %12s\n", "bodies", "instances", "wall_ms", "body_steps/s", "compare_B", "events_i0");
    for (const size_t bodies : {100, 400}) {
        squid::sim::World w;
        squid::sim::Body goal;
        goal.id = "goal";
        goal.center = {200, 1, 0};
        goal.half = {5, 1, 1000};
        goal.region = true;
        w.bodies.push_back(goal);
        for (size_t i = 0; i < bodies; ++i) {
            squid::sim::Body c;
            c.id = "car_" + std::to_string(i);
            c.center = {-static_cast<double>(i % 10) * 7.0, 0.7, static_cast<double>(i / 10) * 4.0};
            c.half = {2, 0.7, 1};
            c.dynamic = c.solid = true;
            c.speed = 8;
            c.jitter = 0.2;
            c.goal = 0;
            w.bodies.push_back(c);
        }
        for (const size_t k : {1, 16, 64}) {
            std::vector<squid::sim::Instance> runs;
            for (size_t s = 0; s < k; ++s) runs.push_back(squid::sim::make_instance(w, s + 1));
            const auto t0 = Clock::now();
            squid::sim::step_all(w, runs, 0.05, 100);
            const double ms = ms_since(t0);
            const double steps = static_cast<double>(k) * static_cast<double>(w.bodies.size()) * 100.0;
            std::printf("%8zu %10zu %12.1f %16.0f %12zu %12zu\n", w.bodies.size(), k, ms, steps / (ms / 1000.0),
                        squid::sim::compare_json(w, runs).size(), runs[0].events.size());
        }
    }
    std::printf("\nCollision is all-pairs (O(N^2) per tick); a broad phase is the next step past a few hundred bodies.\n");
    return 0;
}
