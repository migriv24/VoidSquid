/* effect.hpp — the host half of Void MoleRat.
 *
 * The pure library (src/molerat/molerat.hpp) computes. This header is the seam the
 * effect handler calls: it reads the IR, keeps molerat/out/<run>/, writes the state
 * document and its reports, and refuses malformed agent answers. Every op is reached
 * as `effect molerat <op> ...`, so it is gated and journalled by the existing holiday
 * boundary. No model is embedded and no game bytes leave the machine.
 */
#pragma once

#include "voidmaiz/headless.hpp"

#include <string>
#include <vector>

namespace squid {

std::vector<maiz::EffectOp> molerat_effect_ops();

/* args[0] is the op: gather | research | compose | residual | build | status |
 * task | answer | verify. Returns true when the op was recognised (even on a
 * handled error), and fills `reply` with JSON. */
bool molerat_effect(const std::vector<std::string>& args, std::string& reply);

}  // namespace squid
