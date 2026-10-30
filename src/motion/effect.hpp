/* motion/effect.hpp — `effect motion`: the runtime's judge, started and read (S34, lean (b)).
 *
 *   effect motion judge <library-folder> [out-folder]
 *        start the runtime under `--autotest judge` on the library, in out-folder (default
 *        <library>/judge), and return at once. The runtime opens its own window, renders every
 *        character's clips, writes judge.json, pace.json and a strip per clip, and closes. A
 *        previous judge.json and pace.json there are removed first, so a report is never stale.
 *   effect motion report <folder>
 *        read <folder>/judge.json (and pace.json when present) and answer motion/judge.hpp's
 *        summary. judge.json is the last file the runtime writes, so its presence means the run
 *        finished.
 *
 * The gallery is not an op: the runtime opens it with G while playing (`effect play`), and has
 * no flag that opens it on one character's clip. That flag is the runtime's to add.
 */
#pragma once

#include "voidmaiz/headless.hpp"

#include <string>
#include <vector>

namespace squid::motion {

maiz::EffectOp effect_op();

/* Fills `reply` (JSON) for `effect motion <args>`. */
void effect(const std::vector<std::string>& args, std::string& reply);

}  // namespace squid::motion
