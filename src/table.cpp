/* table.cpp — see table.hpp. */
#include "table.hpp"

#include "allomone/allomone.hpp"
#include "allomone/annotate.hpp"
#include "voidjelly/mantle.hpp"
#include "voidjelly/spatial.hpp"

#include <algorithm>
#include <cstdlib>
#include <map>

namespace squid {

const char* const kDefaultRules =
    "# The default table: cars drive to the goal, walls block, zones report.\n"
    "when tag \"kind:car\" then dynamic \"true\"\n"
    "when tag \"kind:car\" then speed \"8\"\n"
    "when tag \"kind:car\" then jitter \"0.2\"\n"
    "when tag \"kind:wall\" then solid \"true\"\n"
    "when tag \"kind:car\" then solid \"true\"\n"
    "when tag \"kind:zone\" then region \"true\"\n"
    "when tag \"kind:crate\" then gravity \"true\"\n"
    "when tag \"kind:crate\" then dynamic \"true\"\n";

namespace {
bool truthy(const std::string& v) { return v == "true" || v == "1" || v == "yes"; }
}  // namespace

TableCompile compile_table(const voidjelly::Ink& ink, const std::string& rules) {
    TableCompile out;
    const std::vector<voidjelly::Placement> placed = voidjelly::place(ink);

    /* Subjects, in the host's address space: rune ids. `kind` is the domain kind from
     * the `kind:` tag (Allomone's own advice: project Core's rune kind as something
     * else and leave `kind` to the host). */
    std::vector<allomone::Subject> subjects;
    std::map<std::string, int> index;
    std::string goal_id;
    for (size_t i = 0; i < placed.size(); ++i) {
        allomone::Subject s;
        s.id = placed[i].id;
        s.name = placed[i].name;
        s.mantle = ink.name;
        s.tags = placed[i].tags;
        for (const std::string& t : placed[i].tags) {
            if (t.rfind("kind:", 0) == 0) s.kind = t.substr(5);
            if (t == "role:goal" && goal_id.empty()) goal_id = s.id;
        }
        index[s.id] = static_cast<int>(i);
        subjects.push_back(std::move(s));
    }

    const allomone::Script script = allomone::parse("table", rules);
    for (const allomone::Diagnostic& d : script.diagnostics)
        out.diagnostics.push_back("rules line " + std::to_string(d.line + 1) + ": " + d.message);
    const allomone::ConstraintMap map = allomone::eval(script, subjects);

    /* Within ONE source, Allomone settles equal-strength disagreements silently, by
     * the smaller value as a string (measured 2026-10-10: "false" beats "true", "10"
     * beats "9"). That keeps the merge order-free, which is right, and it hides from a
     * game designer that two of their rules disagreed, and compares numbers as text.
     * So this host looks before merging and says so. Reported to Allomone. */
    {
        std::map<std::pair<std::string, std::string>, std::map<int, std::vector<std::string>>> seen;
        for (const allomone::Annotation& a : map.cells) {
            auto& vals = seen[{a.subject, a.property}][a.strength];
            if (std::find(vals.begin(), vals.end(), a.value) == vals.end()) vals.push_back(a.value);
        }
        for (const auto& [key, by_strength] : seen) {
            const auto& top = by_strength.rbegin()->second;   // the strongest opinions
            if (top.size() > 1) {
                std::string list;
                for (const std::string& v : top) list += (list.empty() ? "" : " vs ") + v;
                out.diagnostics.push_back("tie: " + key.first + "." + key.second + " has equal-strength rules (" + list +
                                          "); Allomone settles it by text order, so make one rule more specific");
            }
        }
    }

    const allomone::Merged merged = allomone::merge({map});
    for (const allomone::MergedCell* c : merged.conflicts())
        out.diagnostics.push_back("conflict: " + c->subject + "." + c->property + " has equal-strength opinions; default used");

    for (size_t i = 0; i < placed.size(); ++i) {
        const voidjelly::Placement& p = placed[i];
        sim::Body b;
        b.id = p.id;
        const voidjelly::Vec3 c = p.bounds.center(), e = p.bounds.extent();
        b.center = {c[0], c[1], c[2]};
        b.half = {e[0] / 2, e[1] / 2, e[2] / 2};
        auto prop = [&](const char* k) { return merged.value(p.id, k); };
        b.dynamic = truthy(prop("dynamic"));
        b.solid = truthy(prop("solid"));
        b.region = truthy(prop("region"));
        b.gravity = truthy(prop("gravity"));
        b.speed = std::atof(prop("speed").c_str());
        b.jitter = std::atof(prop("jitter").c_str());
        std::string goal = prop("goal");
        if (goal.empty() && b.dynamic && b.speed > 0) goal = goal_id;   // the scene's goal, if any
        if (!goal.empty()) {
            const auto it = index.find(goal);
            if (it == index.end()) out.diagnostics.push_back(p.id + ": goal '" + goal + "' is not an object in this scene");
            else if (it->second != static_cast<int>(i)) b.goal = it->second;
        }
        if (b.region) b.solid = false;   // a trigger volume never blocks
        out.dynamic += b.dynamic;
        out.solid += b.solid;
        out.regions += b.region;
        out.world.bodies.push_back(std::move(b));
    }

    /* The floor: the top of the lowest static object (a road, a terrain slab), else
     * y = 0. Static, not solid: a road is something cars drive ON, and making it solid
     * would make it block them. */
    double floor = 0;
    bool found = false;
    for (const sim::Body& b : out.world.bodies)
        if (!b.dynamic && !b.region) {
            const double top = b.center[1] + b.half[1];
            const double bottom = b.center[1] - b.half[1];
            if (!found || bottom < floor - 1e-12) { floor = top; found = true; }
            (void)bottom;
        }
    out.world.ground_y = found ? floor : 0.0;
    return out;
}

}  // namespace squid
