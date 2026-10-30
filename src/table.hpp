/* table.hpp — compile a table (a scene plus its rules) into a simulation World.
 *
 * The forge half of okf/design/simulation.md. A Table is a mantle and its rules
 * (okf/concepts/turkey-table-feast.md); for now the mantle is an `.ink` reached through
 * Void Jelly and the rules are one Void Allomone script. Compiling it is three steps,
 * none of which computes geometry here:
 *
 *   1. PLACE: Void Jelly's `place()` gives each object its world bounds.
 *   2. DERIVE: Void Allomone evaluates the script over the objects as subjects (id,
 *      kind from the `kind:` tag, name, tags) and merges the opinions. The properties a
 *      simulation reads are ordinary Allomone effects:
 *
 *        when tag "kind:car"   then dynamic true
 *        when tag "kind:car"   then speed 8
 *        when tag "kind:car"   then goal "crossing"
 *        when tag "kind:wall"  then solid true
 *        when tag "kind:zone"  then region true
 *        when tag "lane:fast"  then speed 14          # more specific: wins over speed 8
 *
 *   3. EMIT: one sim::Body per object, as plain data. The runtime never sees a rune, a
 *      tag or a rule. A property Allomone left CONFLICTED (two sources, equal
 *      strength) is not guessed: it is reported, and the body gets the default.
 *
 * This is the first place Void Squid uses Allomone as the Table's language, which is
 * the author's ruling of 2026-10-10 (S16): Allomone is a programming language for Void
 * Core, "a translation of interaction rules over a mantle into an interface that's
 * easier to understand."
 */
#pragma once

#include "sim/sim.hpp"
#include "voidjelly/ink.hpp"

#include <string>
#include <vector>

namespace squid {

struct TableCompile {
    sim::World world;
    std::vector<std::string> diagnostics;   // parse errors, unknown goals, conflicts
    size_t dynamic = 0, solid = 0, regions = 0;
};

/* Pure: (scene, rules text) → World. Never throws. */
TableCompile compile_table(const voidjelly::Ink& ink, const std::string& rules);

/* A default rule set for a scene whose author wrote none: everything tagged
 * `kind:car` drives to whatever is tagged `role:goal`, walls block, zones report. */
extern const char* const kDefaultRules;

}  // namespace squid
