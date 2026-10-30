/* library/library.hpp — a game library, brought into a Void Squid project.
 *
 * A LIBRARY is a folder on the person's own machine that holds what was recovered from
 * games they own: models, textures, collision, audio, the worlds made from them, and
 * `assets.db`, the asset database in Void Hormiga's schema (runes, fields, tags,
 * links; built by the toolbox, okf/design/four-games-and-the-asset-database.md).
 * Nothing in a library is ever in this repository, and a release carries none of it.
 *
 * A PROJECT (`.squid`) is a Void Core state document. Opening a library brings the
 * library's CATALOGUE into the project as ordinary mantles, so from then on every edit
 * is a dispatcher command: undoable, journalled, reachable from the CLI, and shareable
 * through Void Maiz's collaborative editing when that is switched on. The library's
 * bytes (models, audio) stay in the library folder and are referred to by path.
 *
 *   mantle    glyph    one rune per
 *   turkeys   turkey   recovered thing (a Goomba, a planet, a banana bunch)
 *   games     game     game the library holds something from
 *   kinds     kind     kind of thing (what the game does with it), traits as tags
 *   tracks    track    piece of music a map may play
 *   tables    table    world you place turkeys on (the library's worlds.json, in order)
 *   feast     feast    the one feast: who the player is and how they move
 *
 * THE SPLICE KEEPS A PERSON'S WORK. Importing again (after the toolbox rebuilt the
 * database) replaces what the games, the wikis and the vision model said, and keeps
 * every tag whose source is `person`, every `sound.*` field, a kind a person chose,
 * the feast, and every mantle that is not a library mantle. That is the same promise
 * the toolbox's own rebuild keeps (asset_db.py), kept at the project's level.
 *
 * Reading the database and the folder is the effect; building mantles and splicing
 * them into a state document is pure, and tested without a file (tests/library_test.cpp).
 */
#pragma once

#include "json.hpp"

#include <string>
#include <vector>

namespace squid::library {

using json = nlohmann::json;

/* The mantles a library contributes. A splice replaces exactly these. */
const std::vector<std::string>& library_mantles();

/* Glyph descriptors (JSON), for HostApp::glyphs: turkey, game, kind, track, table,
 * feast. Their hints pick each field's editor in Void Maiz's widget registry and give
 * every field a label a person can read. */
std::vector<std::string> glyphs();

/* ── the effect half ── */

/* Read a library: `path` is the folder or its assets.db. Returns
 *   {"library": <folder>, "mantles": [<Core mantle objects>], "counts": {...}}
 * or {"error": "..."}. Reads assets.db (SQLite, read-only), worlds.json and feast.json
 * when present, and checks which thumbnails exist. Writes nothing. */
json read_library(const std::string& path);

/* ── the pure half ── */

/* One rune in Core's state shape. `id` is derived from (mantle, name), so importing
 * the same library twice gives every rune the same identity — which is what lets a
 * merge between two people's copies line the runes up. */
json make_rune(const std::string& mantle, const std::string& name, const std::string& glyph);
json make_mantle(const std::string& name);

/* Splice `imported` (read_library's answer) into the state document `state_json`,
 * keeping the person's work as described above. Returns the new state document, or ""
 * when `state_json` is not a state document. Sets config `squid.library`. */
std::string splice(const std::string& state_json, const json& imported);

/* The feast rune, as the runtime's feast.json. Pure. */
json feast_file(const json& feast_rune_content);

/* The tables, in order, as the runtime's worlds.json. Pure. */
json worlds_file(const json& tables_mantle);

}  // namespace squid::library
