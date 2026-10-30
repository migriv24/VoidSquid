/* matchloop.cpp — the cross-device match loop, pure.
 *
 * The verifier is injected, so this library owns only the policy: candidates are
 * tried in order, the first Match is accepted, and a function is rejected when the
 * budget is spent or the candidate list runs out. The compile-and-run that turns a
 * candidate into a MatchAttempt is an effect and lives in the host.
 */
#include "forge/forge.hpp"

namespace squid::forge {

MatchOutcome run_match_loop(
    const std::string& function,
    const std::vector<std::string>& candidates,
    int budget,
    const std::function<MatchAttempt(const std::string& candidate)>& verifier) {
    MatchOutcome out;
    out.function = function;

    const int cap = budget > 0 ? budget : static_cast<int>(candidates.size());
    const int n = static_cast<int>(candidates.size());
    for (int i = 0; i < n && i < cap; ++i) {
        const std::string& c = candidates[static_cast<size_t>(i)];
        const MatchAttempt a = verifier(c);
        out.attempts = i + 1;
        if (a.result == AttemptResult::Match) {
            out.verdict = LoopVerdict::Accepted;
            out.accepted_candidate = c;
            out.evidence = a.evidence;
            return out;
        }
        out.evidence = a.evidence;
    }

    out.verdict = LoopVerdict::Rejected;
    if (out.evidence.empty()) out.evidence = "no candidate matched within budget";
    return out;
}

}  // namespace squid::forge
