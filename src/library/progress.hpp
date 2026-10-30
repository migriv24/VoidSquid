/* library/progress.hpp — how much of a game has been recovered, measured against what is known
 * to be in it.
 *
 * The author, 2026-10-12: "we should have a tool to check the progress of decompilation ... not
 * only reference the actual project itself and the decompilation progress of it, but also (if
 * available) compare semantically to the wikipedia page of the game ... every wiki link should be
 * able to associate or link to some rune/turkey on the game."
 *
 * Three yardsticks, each named in the report with where it came from:
 *
 *   the wikis      every page the game's fan wiki lists (characters, enemies, items, places,
 *                  bosses) and every page its Wikipedia article's Gameplay and Plot sections
 *                  link to. Recovered = a turkey matched to that page. The missing ones are
 *                  ranked by how central they are in the wiki's own link graph (the page that
 *                  every other page mentions is the gap that matters most), the way Aesthetic
 *                  Atlas ranks a wiki's pages.
 *   the library    turkeys, playable characters, animations, worlds, and the depth each turkey
 *                  was recovered to (bytes, structure, ...).
 *   the project    the community decompilation, by link (its own progress page says how much of
 *                  the code is matched; Void Squid does not fetch it).
 *
 * A wiki match is a proposal (S26), so this is an estimate with its method stated, never a score
 * to trust blindly. Pure: the effect reads assets.db and passes the rows in.
 */
#pragma once

#include "json.hpp"

#include <string>
#include <vector>

namespace squid::library {

struct WikiEntity {
    std::string name;
    std::vector<std::string> games, sections, categories;
    std::string description;
    std::string redirect;
};

struct TurkeyRow {
    std::string id, game, wiki, kind, depth;
    bool rig = false;
    int animations = 0;
};

/* What a wiki page is to a game: "thing" (characters, enemies, objects, items: turkeys),
 * "place" (levels, locations: tables), "system" (mechanics, terms: logic, recovered later as
 * behaviour), or "game" (another game, mentioned; not counted). From its categories and
 * sections; a rule, so a reader can check it. */
std::string classify(const WikiEntity& e);

/* The report: {"games": [...], "method": "..."}. `mentions` are (from, to) wiki links. */
nlohmann::json progress_report(const std::vector<WikiEntity>& wiki, const std::vector<TurkeyRow>& turkeys,
                               const std::vector<std::pair<std::string, std::string>>& mentions,
                               const nlohmann::json& worlds, const nlohmann::json& known_games);

/* The effect half: read LIBRARY/assets.db (and worlds.json) and report. {"error"} on failure. */
nlohmann::json progress_of_library(const std::string& library_dir, const nlohmann::json& known_games);

/* The rows the pilot needs (library/pilot.hpp), read from LIBRARY/assets.db. */
struct PilotTurkey;
struct PilotEntity;
bool read_pilot_rows(const std::string& library_dir, std::vector<PilotTurkey>& turkeys, std::vector<PilotEntity>& ents,
                     std::string& error);

}  // namespace squid::library
