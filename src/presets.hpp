/* presets.hpp — small scenes built from Void Jelly's primitives, for trying things.
 *
 * Not an asset library (S18): the author's "basic shapes and primitives" for now, put
 * together into a handful of scenes that exercise a feature end to end. Each preset is
 * pure: it returns an Ink, and writing it to disk is `effect make`, the host's.
 *
 * Rune ids: Void Jelly never mints an id and neither does this file. Preset objects
 * carry fixed ids prefixed `rune_preset_`, the same honest compromise as Jelly's own
 * sample scene: a fixture whose ids moved would hash differently on every run. A scene
 * meant to be shared should be re-minted through Void Core (`rune new`) first.
 */
#pragma once

#include "voidjelly/ink.hpp"

#include <string>
#include <vector>

namespace squid::presets {

/* A two-lane road with a crossing zone (the goal), cars in both lanes, a wall in the
 * fast lane, and a crate that falls. Everything the default table rules talk about. */
voidjelly::Ink traffic();

/* Every primitive Jelly offers, in a row, for looking at. */
voidjelly::Ink shapes();

std::vector<std::string> names();
/* Empty Ink (no objects) for an unknown name. */
voidjelly::Ink by_name(const std::string& name);

}  // namespace squid::presets
