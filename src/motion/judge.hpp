/* motion/judge.hpp — the runtime's motion verdicts, read for the studio (S34, lean (b)).
 *
 * Void Onda (../VoidOnda) judges motion from pixels; the runtime (the exercise's
 * goomba_galaxy2) links it and, under `--autotest judge`, renders every character's clips
 * and writes two reports in its working directory:
 *
 *   judge.json   one row per clip: character, verb, clip, used_verb, confidence, and Onda's
 *                overall verdict as rate with a Wilson interval [lo, hi], plus every check's
 *                passed / trials / note (Onda's okf/design/judging-motion.md)
 *   pace.json    one row per (character, speed, rule): feet keeping pace on a treadmill
 *
 * The studio does not link Onda. It reads what the judge wrote and counts it: how many clips,
 * their mean and the lowest, and the checks under 80% the way the runtime prints them as
 * "weak". No verdict is computed here and no interval is recomputed; a pooled pace is
 * passed/trials, with no interval, because the interval is Onda's to compute. When the runtime
 * moves in (S30) the studio links Onda and this file goes (S34, lean (a)).
 *
 * Pure: the effect (motion/effect.cpp) reads the files and hands the JSON in.
 */
#pragma once

#include "json.hpp"

namespace squid::motion {

/* A check is "weak" below this rate: the runtime's own threshold when it prints a clip. */
constexpr double kWeak = 0.8;

/* {"clips", "mean", "lowest", "weak", "characters": [...], "pace": [...], "method"}.
 * `judge` is judge.json's array; `pace` is pace.json's array, or null when there is none.
 * Rows that do not name a character are skipped and counted in "skipped". */
nlohmann::json summarize(const nlohmann::json& judge, const nlohmann::json& pace);

}  // namespace squid::motion
