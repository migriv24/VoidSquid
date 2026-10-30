/* effect.hpp — the host half of the forge.
 *
 * The pure library (src/forge/forge.hpp) computes. This seam is what the effect
 * handler calls: it reads the device manifests and the match task, runs the loop
 * with a synthetic verifier, and returns JSON. Every op is reached as
 * `effect forge <op> ...`, so it is gated and journalled by the existing holiday
 * boundary. No toolchain is invoked yet, and the report says so.
 */
#pragma once

#include <string>
#include <vector>

namespace squid {

/* args[0] is the op: devices | route | match. Returns true when the op was
 * recognised (even on a handled error), and fills `reply` with JSON. */
bool forge_effect(const std::vector<std::string>& args, std::string& reply);

}  // namespace squid
