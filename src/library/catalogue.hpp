/* library/catalogue.hpp — has somebody already decompiled this game?
 *
 * The cheapest way to recover a game is to find that a community already did, and the
 * MINIMUM OPERATIONAL SETUP (okf/design/load-a-game.md) is built on that: no model, no
 * GPU, just a game the person owns, recon's reading of its header, and a list of known
 * decompilations, disassemblies and recompilations to match it against.
 *
 * Two lists, kept apart on purpose:
 *
 *   the seed     data/known-games.json, shipped with Void Squid and written here: the
 *                games Squid has an ingest route for, and the projects those routes read
 *                (for names and structure, never copied). Small, curated, ours.
 *   the cache    the community list, github.com/CharlotteCross1998/awesome-game-
 *                decompilations, downloaded on the person's machine when they ask
 *                (`effect catalogue refresh`, a network effect) into config_dir. Not
 *                shipped and not copied into this repository: it is someone else's
 *                curation and has no licence file.
 *
 * Matching is deterministic: titles are folded to letters and digits and compared
 * whole, then by containment, and the platform must agree when both sides state one.
 * Every match carries the reason it matched, so a person (or a small model) can see
 * why, and a wrong match is a visible proposal rather than a silent one.
 */
#pragma once

#include "json.hpp"

#include <string>
#include <vector>

namespace squid::library {

struct KnownGame {
    std::string title;
    std::vector<std::string> aliases;   // header titles, other regions' names
    std::string platform;               // recon's vocabulary: n64, ds, wii, gc, gba, gb, nes, snes, pc, ...
    std::string project;                // what the community project is called
    std::string url;
    std::string kind;                   // decompilation | disassembly | recompilation | reimplementation | toolkit
    std::string route;                  // what Void Squid can do with it today ("" = nothing yet)
    std::string route_id;               // the ingest route that does it (toolbox/route.py): dk64, ds, smg
    std::string notes;
    std::string from;                   // "seed" | "community list"
    bool ai_flagged = false;            // the community list marks AI-involved projects
    bool work_in_progress = false;
};

/* Fold a title for comparison: lowercase letters and digits only, with region and
 * language parentheses removed ("Donkey Kong 64 (Europe) (En,Fr)" -> "donkeykong64"). */
std::string fold_title(const std::string& t);

/* Map a community-list heading ("Nintendo 64", "Nintendo DS") to recon's platform word. */
std::string platform_of_heading(const std::string& heading);

/* Parse the seed file's JSON. */
std::vector<KnownGame> parse_seed(const nlohmann::json& j);

/* Parse the community list's Markdown (headings, then "- [Title](url)" lines). Pure. */
std::vector<KnownGame> parse_community_list(const std::string& markdown);

struct Match {
    KnownGame game;
    std::string why;      // "same title", "title contains", ...
    int score = 0;        // 100 exact title on the same platform, lower for weaker evidence
};

/* Match candidate titles (recon's header title, the file name) against the lists. Pure.
 * Best first; at most `limit`. */
std::vector<Match> match(const std::vector<std::string>& titles, const std::string& platform,
                         const std::vector<KnownGame>& games, size_t limit = 8);

nlohmann::json to_json(const KnownGame& g);

}  // namespace squid::library
