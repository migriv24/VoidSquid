/* library/loadgame.hpp — Load a game: from a file the person owns to a plan.
 *
 * The author, 2026-10-12: loading a game is STEP ONE of making something new, not the
 * point of Void Squid. So this answers the person's question in the plainest terms —
 * what is this, has anyone already recovered it, what can Void Squid do with it on
 * this machine, and what would need a smarter helper — and then gets out of the way.
 *
 * Deterministic on purpose. The minimum setup assumes no model at all, so every step
 * here is a rule a test can check: recon's header reading, the catalogue's title match,
 * the inventory's PATH lookups. A model (local or an agent) may PROPOSE later steps;
 * it never decides this one. Pure: recon's answer, the catalogue and the inventory come
 * in as values.
 */
#pragma once

#include "library/catalogue.hpp"

#include "json.hpp"

#include <string>
#include <vector>

namespace squid::library {

/* A folder that holds exactly one game file (a ROM, a disc image) means that file;
 * anything else means the folder itself. Reads a directory listing (the effect). */
std::string pick_game_file(const std::string& path);

/* The titles worth matching: recon's header title, then the file or folder name. */
std::vector<std::string> candidate_titles(const nlohmann::json& recon, const std::string& path);

/* The plan: {"target", "recon", "titles", "matches", "steps": [{step, setup, state, how}], "summary"}. */
nlohmann::json plan(const std::string& path, const nlohmann::json& recon,
                    const std::vector<KnownGame>& games, const nlohmann::json& inventory);

}  // namespace squid::library
