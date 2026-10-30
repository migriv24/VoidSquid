/* sim_test.cpp — the runtime and the table compiler, as pure functions.
 *
 * In the order each would be most expensive to be wrong:
 *   1. Determinism: same world, same seed → identical instances, byte for byte.
 *   2. Fork: a fork with the same seed continues exactly as the original; a reseeded
 *      fork diverges, and compare_json says when.
 *   3. The rules: Allomone decides who moves, who blocks and where they go, and a more
 *      specific rule beats a broader one.
 *   4. The story the traffic preset tells: the slow lane reaches the crossing and
 *      queues; the fast car hits the roadworks; the crate lands; nobody passes through
 *      a wall.
 */
#include "presets.hpp"
#include "sim/sim.hpp"
#include "table.hpp"

#include <cstdio>
#include <string>

using namespace squid;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { ++failures; std::printf("  FAIL  %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static int body(const sim::World& w, const std::string& id) {
    for (size_t i = 0; i < w.bodies.size(); ++i) if (w.bodies[i].id == id) return (int)i;
    return -1;
}
static bool contains(const std::string& h, const std::string& n) { return h.find(n) != std::string::npos; }

int main() {
    const voidjelly::Ink ink = presets::traffic();
    const TableCompile tc = compile_table(ink, kDefaultRules);
    for (const std::string& d : tc.diagnostics) std::printf("  diag: %s\n", d.c_str());
    CHECK(tc.diagnostics.empty());
    CHECK(tc.dynamic == 5);       // four cars and the crate
    CHECK(tc.regions == 1);       // the crossing
    const sim::World& w = tc.world;
    const int car_a = body(w, "car_a"), car_c = body(w, "car_c"), fast = body(w, "car_fast");
    const int wall = body(w, "roadworks"), crate = body(w, "crate"), crossing = body(w, "crossing");
    CHECK(w.bodies[car_a].goal == crossing);     // the scene's role:goal, by default
    CHECK(w.bodies[wall].solid && !w.bodies[wall].dynamic);
    CHECK(w.bodies[crossing].region && !w.bodies[crossing].solid);
    CHECK(std::abs(w.ground_y - 0.0) < 1e-9);     // the road's top

    /* 1. Determinism. */
    std::vector<sim::Instance> runs = {sim::make_instance(w, 7), sim::make_instance(w, 7)};
    sim::step_all(w, runs, 0.05, 200);           // 10 simulated seconds
    CHECK(runs[0] == runs[1]);

    /* 4. The story. */
    const sim::Instance& r = runs[0];
    CHECK(sim::count(r, "arrive", car_a) == 1);
    CHECK(sim::count(r, "enter", car_a) == 1);
    CHECK(sim::count(r, "contact", fast) == 1);  // the roadworks, and nothing else:
    for (const sim::Event& e : r.events)
        if (e.kind == "contact" && e.body == fast) CHECK(e.other == wall);
    CHECK(sim::count(r, "arrive", fast) == 0);
    CHECK(sim::count(r, "land", crate) == 1);
    CHECK(r.pos[fast][0] + w.bodies[fast].half[0] <= w.bodies[wall].center[0] - w.bodies[wall].half[0] + 1e-9);  // never through the wall
    CHECK(std::abs(r.pos[crate][1] - 0.5) < 1e-9);   // resting on the road
    CHECK(sim::count(r, "contact", car_c) >= 1);     // the queue behind the leader

    /* 2. Fork: same seed continues bit-identically; a reseed diverges. */
    std::vector<sim::Instance> a = {sim::make_instance(w, 3)};
    sim::step_all(w, a, 0.05, 40);
    sim::Instance same = sim::fork(w, a[0], 0, false);
    sim::Instance other = sim::fork(w, a[0], 99, true);
    std::vector<sim::Instance> cont = {a[0], same, other};
    sim::step_all(w, cont, 0.05, 160);
    CHECK(cont[0] == cont[1]);
    CHECK(!(cont[0].events == cont[2].events));
    const std::string cmp = sim::compare_json(w, cont);
    CHECK(contains(cmp, "\"identical\":false"));
    CHECK(contains(cmp, "\"first_divergence_tick\""));
    CHECK(sim::compare_json(w, {cont[0], cont[1]}).find("\"identical\":true") != std::string::npos);

    /* The bounded log. */
    const std::string log = sim::instance_json(w, r, 5);
    CHECK(contains(log, "\"counts\""));
    CHECK(contains(log, "\"body\":\"car_a\"") || contains(log, "\"kind\":"));

    /* 3. Rules: a more specific rule wins; Allomone resolves it, not this host. */
    const std::string faster = std::string(kDefaultRules) +
        "when tag \"kind:car\" and tag \"lane:fast\" then speed \"20\"\n"
        "when tag \"kind:car\" and tag \"lane:fast\" then jitter \"0\"\n";
    const TableCompile tf = compile_table(ink, faster);
    CHECK(tf.world.bodies[fast].speed == 20 && tf.world.bodies[car_a].speed == 8);

    /* Equal-strength opposite opinions in ONE script: Allomone settles them silently by
     * text order ("false" < "true"), so this host must say so rather than let it pass. */
    const std::string tied = std::string(kDefaultRules) + "when tag \"kind:wall\" then solid \"false\"\n";
    const TableCompile tn = compile_table(ink, tied);
    CHECK(tn.diagnostics.size() == 1 && contains(tn.diagnostics[0], "tie: roadworks.solid"));

    /* Walls off, said properly: a MORE SPECIFIC rule. No tie, and the fast car arrives. */
    const std::string no_walls = std::string(kDefaultRules) +
        "when tag \"kind:wall\" and not tag \"kind:car\" then solid \"false\"\n";
    const TableCompile tw = compile_table(ink, no_walls);
    CHECK(tw.diagnostics.empty());
    CHECK(!tw.world.bodies[wall].solid);
    std::vector<sim::Instance> nw = {sim::make_instance(tw.world, 7)};
    sim::step_all(tw.world, nw, 0.05, 200);
    CHECK(sim::count(nw[0], "arrive", fast) == 1);
    CHECK(sim::count(nw[0], "contact", fast) == 0);

    /* A goal that is not in the scene is a sentence, not a crash. */
    const TableCompile bad = compile_table(ink, std::string(kDefaultRules) + "when tag \"kind:car\" then goal \"nowhere\"\n");
    CHECK(!bad.diagnostics.empty());

    if (failures == 0) { std::printf("ok    squid sim\n"); return 0; }
    std::printf("FAIL  squid sim (%d)\n", failures);
    return 1;
}
