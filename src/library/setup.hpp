/* library/setup.hpp — what this machine can do, and which setup that makes it.
 *
 * Void Squid serves people with very different machines and very different help
 * (okf/design/load-a-game.md). Four setups, each a strict superset of the one before:
 *
 *   minimum      no model, no GPU. Recon reads the game's header; the catalogue says
 *                whether a community already decompiled it; a route turns it into
 *                assets; tests and checks catch what goes wrong; the person fixes the
 *                rest by hand. Always available. Assumes nothing is smart.
 *   local model  Ollama with a model pulled (or Needle through Void Bicho, later):
 *                naming, tagging and reading logs get better, and every answer is still
 *                checked by the same tests. The setup the author calls "normal".
 *   agent        a coding agent on this machine (Claude Code, Codex): Void Squid is a
 *                library of tools it drives (`squid --describe`), and decompiling a game
 *                nobody has done is possible.
 *   api          a hosted model through the person's own API key. PLANNED, not built.
 *
 * Vision (Void Bicho's CLIP and SAM) is a separate row: it needs Python, PyTorch and
 * ideally a GPU, and only the tagging step uses it.
 *
 * This file only LOOKS: PATH lookups and a few `--version` calls with short timeouts.
 * It installs nothing and changes nothing.
 */
#pragma once

#include "json.hpp"

#include <string>

namespace squid::library {

/* The GPU the window got, as OpenGL reported it. Set by the desktop host after it
 * makes its context (the headless host has none and says so). */
void set_gl_renderer(const std::string& renderer);

/* The inventory: {"machine":{...}, "tools":[...], "setups":[...], "checked":"..."}.
 * `deep` also asks Void Bicho's Python whether PyTorch sees CUDA (slow: seconds). */
nlohmann::json inventory(bool deep = false);

/* Where Ollama is: PATH, or its default install folder. "" when absent. */
std::string ollama_path();

}  // namespace squid::library
