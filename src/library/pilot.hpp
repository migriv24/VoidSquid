/* library/pilot.hpp — a small model, held to account: the minimum setup's first model-assisted job.
 *
 * The author, 2026-10-12: test the MOSU "and see if i can run the smaller models and decompile a
 * game correctly", with guardrails that "rely very little on the intelligence of a model. as
 * little as possible." The job chosen (S29): naming. Many recovered things carry the game's
 * internal name (Galaxy's StarPiece, KinokoOneUp, Takobo) and no wiki page; the wiki knows them
 * as Star Bit, 1-Up Mushroom, Octoomba. A match makes the thing searchable, taggable and counted
 * in Progress.
 *
 * THE MODEL DECIDES ALMOST NOTHING. Every step but one is a rule:
 *   1. shortlist   a deterministic score (shared name parts, the game's own category against
 *                  the wiki's section, the wiki page's centrality) picks at most 12 candidates
 *                  from the same game. The model never sees the rest.
 *   2. ask         the model answers with a JSON schema whose `match` is an ENUM of the
 *                  shortlist plus "none" (Ollama's structured outputs): it cannot invent a name.
 *   3. agree       asked again with the shortlist reversed, it must give the same answer.
 *   4. fit         the answer's wiki sections must fit the turkey's kind (an enemy is not an
 *                  item); not already taken by another turkey of the same game.
 *   5. propose     what survives is a PROPOSAL (source "local:<model>"), accepted by a person.
 * And it is measured: `pilot test` hides the matches already made by name or Japanese
 * romanization (known right), asks the model to recover them, and scores it against the
 * rules-only baseline (the shortlist's first candidate). That number is the answer to "can a
 * small model do this", and it is printed, not assumed.
 */
#pragma once

#include "json.hpp"

#include <functional>
#include <string>
#include <vector>

namespace squid::library {

struct PilotTurkey {
    std::string id, name, cls, category, kind, game;
    std::string known;     // its current wiki match ("" for none); the test hides it
};
struct PilotEntity {
    std::string name, description;
    std::vector<std::string> games, sections;
    int mentions = 0;
};

/* Step 1, pure: at most `n` candidates for a turkey, best first, each with its score. */
std::vector<std::pair<std::string, double>> shortlist(const PilotTurkey& t, const std::vector<PilotEntity>& ents, size_t n = 12);

/* Step 4, pure: do the entity's sections fit the turkey's kind? */
bool kind_fits(const std::string& kind, const std::vector<std::string>& sections);

/* The model, as the host supplies it: a prompt and a JSON schema in, the model's JSON out
 * (or {"error"}). Ollama's /api/chat with `format` = the schema, through the effect. */
using AskModel = std::function<nlohmann::json(const std::string& prompt, const nlohmann::json& schema)>;

/* Steps 2-5 for one turkey: {"turkey", "shortlist", "answers": [a, b], "proposal" or null,
 * "guards": {"agreed", "fits", "free"}, "baseline"}. */
nlohmann::json propose(const PilotTurkey& t, const std::vector<PilotEntity>& ents, const AskModel& ask,
                       const std::vector<std::string>& taken);

/* The test: for up to `limit` turkeys whose match is known, hide it and see who recovers it.
 * {"model", "cases", "model_correct", "baseline_correct", "shortlist_recall", "abstained",
 *  "wrong_accepted", "results": [...]} */
nlohmann::json pilot_test(const std::vector<PilotTurkey>& turkeys, const std::vector<PilotEntity>& ents, const AskModel& ask,
                          const std::string& model, size_t limit);

}  // namespace squid::library
