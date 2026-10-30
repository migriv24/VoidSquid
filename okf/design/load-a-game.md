---
type: Design
title: Load a game — step one of making something new, and the four setups it runs on
description: "2026-10-12. The author: Void Squid is about the construction of worlds and the export of games, not decompilation; loading a game is step one. It must work for three kinds of person (an agent user who knows decompilation, an agent user who does not, someone with no agent at all) on machines from no GPU to a good one. So four setups: a minimum operational setup with no model and guardrails that assume nothing is smart, a local-model setup (Ollama) the author calls normal, a coding-agent setup, and a hosted-model setup that is planned only. What Load a game does today, the catalogue of known decompilations, the release rule (no game data, no demo), and what comes after loading."
resource: ../src/library/loadgame.cpp
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-12T00:00:00Z
---

> The author: *"ultimately though, void squid is NOT about decompilation. It's about the
> creation of NEW things. its about the EXPORT of games. so the 'load game' step, where you
> load the assets and logic from a game, is literally just step 1."*
>
> **Built:** the minimum setup's Load a game (recon, the catalogue, the plan), the setup
> inventory, and the community list download. **Designed:** the local-model and agent
> setups' use. **Planned only:** the hosted-model setup.

# 1. What Void Squid is for

Construction, then export. A person loads a game they own; what comes out becomes
**turkeys** (things), **tables** (worlds) and a **feast** (who plays, in what order);
they build something new and export it as a program (a Windows `.exe`, a Linux binary;
later a DS cartridge, later an operating system). Loading brings **logic** as well as
assets, eventually: the author's example is not only `goomba` but `goomba_ai`, behaviour
that can be reapplied to anything else, configured in Allomone (S16). None of the logic
half is built; Load a game is written so that its plan has a place for it.

# 2. Three people

| person | what Void Squid is to them | where they start |
|---|---|---|
| uses a coding agent, knows decompilation tools | a better place to see and edit what they recovered | the Shelf and the Turkey checks |
| uses a coding agent, does not know the tools | a library of tools their agent drives, and the window they check the agent's work in | `squid --describe` |
| no coding agent | everything, through the window; the minimum setup is built for them | Start > Load a game |

**The biggest thing for all three** (the author): loading assets separately, and designing
worlds with them.

# 3. Four setups (and vision)

Each is a strict superset of the one before. `effect setup` reports which are ready
(`library/setup.cpp`); the Setup window shows it.

| setup | adds | assumes | status |
|---|---|---|---|
| **minimum** (the MOSU) | recon, the catalogue, the routes, checks | **nothing is smart**: any step may be wrong, the person fixes by hand, tests catch what they can | built |
| **local model** ("normal") | Ollama (or Needle through Void Bicho): naming, tagging, testing, reading logs and CLI output | a model is helpful and still wrong sometimes; the minimum's checks still decide | detected; not called |
| **vision** | Void Bicho's CLIP and SAM: tags by what a thumbnail looks like | Python and PyTorch; a GPU is faster, the CPU works | used by the toolbox today |
| **agent** | a coding agent drives Void Squid from a terminal; decompiling a game no community has | the agent is capable; its work lands as reviewable commands | the CLI and briefing are built |
| **hosted model** | a model through the person's own API key (OpenAI, Anthropic, DeepSeek) | money per use; prompt work; shared projects must say who spent what | planned only, by the author's decision |

The author on the minimum: *"we are assuming it's stupid and can mess anything up at any
time … but honestly, that should NOT be the standard of doing things."* So the minimum is
the **floor**, built first because everything else stands on it, and the local model is
the setup most people with a decent machine should have. The author's machine is in that
range (an RTX A1000 with 6 GB, Ollama with three models pulled).

**Void Verguenza** is the candidate for the hosted setup's small storage (keys, spending)
when it is designed (S29).

# 4. Guardrails that rely on intelligence as little as possible

The author: *"we're gonna have to create ourselves a LOT of safety and guardrails for these
models. not because they're malicious, but because they're extremly likely to mess up.
These will have to be tests and other code designed to rely very little on the
intelligence of a model."*

The rules, held by what is built and binding on what is not:

1. **Every step of the plan is a rule a test can check**: a header read, a title match, a
   PATH lookup. Load a game makes no guess (`library_test.cpp` pins the plan).
2. **Every match says why** ("same title, same platform", "one title contains the
   other"), and a different platform takes 45 of its 100 points away. A wrong match is visible.
3. **A model's answer is a proposal** (CLAUDE.md rule 6). It lands as a command a person
   can undo, with its source recorded (`tag_sources: "local:<model>"`, the same way
   `vision:0.42` is recorded today), and it is checked before it is shown: a tag against
   the vocabulary, a name against the game's own data and the wiki, a kind against the
   twelve kinds.
4. **Small models get tools and schemas, not prose.** Needle's decode grammar guarantees
   output that parses and returns an empty list rather than a guess
   (`VoidBicho:/design/needle.md`); Ollama's structured outputs do the same for a JSON
   schema. Either way, the model fills a form Void Squid checks.
5. **The person's work outranks every model's.** A re-import keeps person-sourced tags,
   kinds and removals (`library::splice`); a window outranks an agent on the same project
   (Void Maiz's Session).

# 5. Load a game today

`effect loadgame <file|folder>` (and the Load a game window):

1. **Pick the file**: a folder holding exactly one ROM or disc image means that file.
2. **Recon** reads its header (`squid_read`, the ingress arrow's first stage).
3. **Match** the header title and the file name against both lists (§6).
4. **Plan**: identify, find a decompilation, extract assets, name and tag, better names
   from a local model (planned), tag by vision, open the library. Each step says which
   setup it needs and whether it is done, ready, needs something, or is planned.

Measured on the author's games: Donkey Kong 64 (the `.z64` inside its folder; header
title `DONKEY KONG 64`, n64) matched the seed at 100 with the toolbox's route. Super
Mario Galaxy (an `.rvz`, which recon can only call `gamecube-or-wii`) matched at 100 once
either platform was allowed to count, then the community list's *Super Mario Galaxy 2* at
87 below it.

# 5b. Load a game, one button (2026-10-13, evening)

The author: *"i press a couple buttons, i get some loading bars (to download stuff) and it just
starts downloading and decompiling and stuff. If anything, the default should be the MOSU, but we
should be able to configure it differently."* Built:

- **Choosing a file reads it** (recon, the catalogue, the plan) with no second button.
- **Load it** starts the route with the setup chosen in Settings > Models (Minimum by default; Local
  model; Vision).
- **The route fetches its own tools** into the settings folder's `tools/` (never a repository), each
  an invocation: numpy and Pillow in a private Python; apicula (0BSD, a release download on Windows,
  cargo on Linux); nodtool (encounter/nod, for Wii and GameCube discs, so Dolphin is no longer
  needed); dk64_lib (GPL-3, its own environment); Petari's names (CC0, downloaded and read for names).
- **Loading bars**: the route writes `ingest.progress.json` (every step: waiting, running, done,
  skipped, failed; a fraction when known); the window draws a bar per step and one overall.
- **After**: Open the library; How complete is it (the Games window); the Local-model setup asks
  the model for names (the pilot) on its own.

Measured: apicula, nodtool and dk64_lib downloaded into an empty folder in 0.8 s, 1.1 s and 23 s;
Donkey Kong 64 from its ROM to a library through Void Squid's own effect in about a minute.

# 6. The catalogue of known decompilations

| list | where | what |
|---|---|---|
| the seed | `data/known-games.json`, shipped | written here: the six games Void Squid knows, the route for each, the community project it reads (for names, never copied) |
| the community list | downloaded to the settings folder by *Download the community list* (`effect catalogue refresh`) | [awesome-game-decompilations](https://github.com/CharlotteCross1998/awesome-game-decompilations), 485 entries on 2026-10-12, parsed with its platform headings, its AI marks and its work-in-progress lines |

**Why the community list is downloaded, not shipped**: it is someone else's curation and
has no licence file. Titles and links are facts; a copy of the curation is not ours to
make. The download is one request, made only when the person presses the button, and
declared as the `catalogue` effect's consequence.

# 7. Release

- **No game data, no decompiled code, no model weights, no demo.** Void Hormiga ships a
  demo dataset; Void Squid ships none (the author: *"Sadly i don't think we should even
  have demo data"*; a demo game the author makes may come later). The four-game library
  is a local test library and is never exported.
- **What a release carries**: the two executables, the shared Core, the fonts, the seed
  catalogue, the icon, `SETUP.md` (CMake stages them beside the binary).
- **Void Bicho is the heavy part**, so [SETUP.md](../../SETUP.md) splits it by need:
  CLIP on the CPU (about 1 GB), CLIP on an NVIDIA GPU (about 3 GB), SAM 2 and
  sentence-transformers only when wanted.
- **Windows and Linux.** The code has both halves (dialogs, processes, settings folders);
  only Windows has run it.

# 8. Open

S29 (the local model's first job), S30 (the runtime and the exported game) and S31 (the
community list) in [open questions](/design/open-questions.md).
