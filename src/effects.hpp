/* effects.hpp — the studio's effects: the library, the setup, Load a game, Play.
 *
 * Each is an `effect <op>` like `open` and `recon` (app.cpp), declared in the HostApp
 * so an agent arriving cold sees what it does and what it costs, and gated like every
 * effect. The effect handler is given only its arguments, never the document, so
 * whatever an effect needs from the project the front-end passes in: the library
 * folder, the world file, the feast as JSON. That keeps the handler re-entrancy-free
 * and makes each call a complete line in the journal.
 *
 *   effect library import <folder|assets.db>      read a library; answers the mantles to splice
 *   effect library export <folder> <feast-json> <worlds-json>
 *                                                 write the runtime's feast.json and worlds.json
 *   effect setup [deep]                           what this machine can do (library/setup.hpp)
 *   effect loadgame <file|folder>                 recon + catalogue + setups -> a plan
 *   effect catalogue refresh                      download the community decompilation list
 *   effect catalogue search <title> [platform]    match a title against both lists
 *   effect play <folder> <world-file> [feast-json]
 *                                                 write feast.json, start the runtime on a world
 *   effect table3d <folder> [world-file]          start the 3D table on a world
 *   effect reveal <path>                          show a file or folder in the system's file manager
 *   effect motion judge|report ...                the runtime's motion judge (motion/effect.hpp)
 */
#pragma once

#include "voidmaiz/headless.hpp"

#include <string>
#include <vector>

namespace squid {

/* The ops, for HostApp::effect_ops. */
std::vector<maiz::EffectOp> studio_effect_ops();

/* Handle `sub` when it is a studio effect: fills `reply` (JSON) and returns true. */
bool studio_effect(const std::string& sub, const std::vector<std::string>& args, std::string& reply);

/* Where the runtime lives (the program Play starts), or "" when it was not found.
 * VOIDSQUID_RUNTIME, else beside this binary, else the ds-mashup exercise's build. */
std::string runtime_dir();

/* Where the ingest routes are (the exercise's toolbox today): Settings, VOIDSQUID_TOOLBOX,
 * else beside this checkout. "" when not found. */
std::string toolbox_dir();

}  // namespace squid
