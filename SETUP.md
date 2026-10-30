# Setting up Void Squid

Void Squid builds new worlds out of the games you own. This guide says what you need,
what is optional, and how to add each piece. **Start with the minimum.** Everything after
it makes Void Squid smarter or faster, and nothing after it is required.

> Void Squid ships **no game, no game data, no decompiled code and no model weights**,
> and has no demo data. What you load comes from games you own and stays on your
> machine. Void Squid sends nothing anywhere unless you press a button that says it
> will (today there is one: *Download the community list*).

## Contents

1. [Which setup is yours](#1-which-setup-is-yours)
2. [The minimum: Void Squid itself](#2-the-minimum-void-squid-itself)
3. [Your first library](#3-your-first-library)
4. [A local model (Ollama)](#4-a-local-model-ollama)
5. [Vision (Void Bicho)](#5-vision-void-bicho)
6. [A coding agent](#6-a-coding-agent)
7. [A hosted model with your own key (planned)](#7-a-hosted-model-with-your-own-key-planned)
8. [Where Void Squid keeps things](#8-where-void-squid-keeps-things)

Void Squid can check your machine for you: **Setup** (top right), then *Check this
machine*. It only looks; it installs nothing.

---

## 1. Which setup is yours

| you have | your setup | what you get |
|---|---|---|
| any computer, no GPU needed | **Minimum** | Load a game tells you what a game is and whether a community already recovered it; the routes bring its assets in; you check and fix names and tags in the Turkey workspace, build worlds in the Table, play them |
| a few GB of free memory | **+ Local model** | better names, tags and log reading, still held to the same checks. *Detected today; Void Squid does not ask it anything yet.* |
| Python, ideally an NVIDIA GPU | **+ Vision** | thumbnails tagged by what they look like (CLIP); shapes cut out (SAM) |
| Claude Code or Codex | **+ Agent** | an agent drives Void Squid's tools from a terminal, and can decompile games no community has |
| an API key you pay for | **Hosted model** | planned, not built |

Three kinds of person, three ways in:

- **You already use a coding agent and know decompilation tools.** Void Squid is the
  place to see and edit what you recovered: the Shelf, the Turkey checks, the Table.
  Skip to [§6](#6-a-coding-agent).
- **You use a coding agent but not decompilation tools.** Void Squid is a library of
  tools your agent can drive *and* the window you check its work in. Do §2, then §6.
- **You have no coding agent.** Do §2 and §3. Add §4 and §5 when you want more help.
  Expect to check things by hand: the minimum setup assumes no help is smart, and the
  checks in the Turkey workspace exist for exactly that.

---

## 2. The minimum: Void Squid itself

**Windows 10 or 11 (x64)** is where Void Squid is built and tested today. **Linux (x64)**
is supported by the code and not yet tested on a real machine.

Until there is a release to download, Void Squid is built from source.

### Windows

1. Install [MSYS2](https://www.msys2.org/), open the **UCRT64** shell, and install the
   compiler and build tools:
   ```sh
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja git
   ```
2. Clone Void Squid and its siblings **side by side** in one folder:
   ```sh
   git clone https://github.com/migriv24/void-core VoidCore
   git clone https://github.com/migriv24/VoidMaiz
   git clone https://github.com/migriv24/VoidJelly
   git clone https://github.com/migriv24/VoidAllomone
   git clone https://github.com/migriv24/VoidSquid
   ```
3. Build Void Core first (its own README), then Void Squid:
   ```sh
   cd VoidSquid
   cmake -S . -B build -G Ninja
   cmake --build build
   ctest --test-dir build -R squid_
   ```
4. Run `build/bin/voidsquid.exe` (the window) or `build/bin/squid.exe --help` (the
   command line). To make a desktop shortcut, point it at `voidsquid.exe` with
   `packaging/squid.ico` as its icon.

### Linux

1. Install the build tools, the X11 headers GLFW needs, and a file dialog helper:
   ```sh
   sudo apt install build-essential cmake ninja-build git \
        libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl-dev zenity
   ```
   (`zenity` draws the Open and Save dialogs. `kdialog`, `yad` or `qarma` work too.)
2. Clone and build exactly as on Windows (steps 2 and 3). Run `build/bin/voidsquid`.

### What the minimum can already do

- **Load a game** (top right): point at a ROM, a disc image or an install folder. Void
  Squid reads its header, matches it against known decompilations, and lays out what
  happens next on this machine. Every step is a rule a test can check, not a guess.
- **Download the community list** (in Load a game): fetches
  [awesome-game-decompilations](https://github.com/CharlotteCross1998/awesome-game-decompilations)
  once, so Load a game knows about hundreds more projects. Needs `curl` (built into
  Windows 10 and later).
- Open a library, inspect and tag turkeys, order the tables, set the player, play.

---

## 3. Your first library

A **library** is a folder on your machine holding what was recovered from your games:
models, textures, collision, music, the worlds, and `assets.db`, the catalogue.
**File > Open library...** brings the catalogue into your project.

**Today, libraries are built by the toolbox of the Goomba Galaxy exercise**, a separate
repository (`ds-mashup`) where the routes for the first four games were worked out:
Super Mario Galaxy, Super Mario 64 DS, Mario Kart DS and Donkey Kong 64. Those routes
move into Void Squid one at a time. Until then, Load a game names the route and what it
needs. Each needs Python 3.10 or newer, plus:

| game | you need | the route reads |
|---|---|---|
| Super Mario Galaxy (Wii) | the disc extracted with [Dolphin](https://dolphin-emu.org/) (right-click > Properties > Filesystem > Extract Entire Disc) | placements, models and materials, collision, gravity, music |
| Super Mario 64 DS, Mario Kart DS, Mario Party DS (DS) | the ROM; `apicula` (Rust, through `cargo install`) | models, collision |
| Donkey Kong 64 (N64) | the ROM; `dk64_lib` in its own Python environment (GPL-3, invoked, never imported) | map geometry, textures, pickups |

After the toolbox runs, open its output folder as your library.

---

## 4. A local model (Ollama)

**What it is for:** better names and tags, and reading long logs. **Status:** Void Squid
detects Ollama and the models you pulled, and says so in Setup. It does not ask a model
anything yet. Every answer a model gives will be a proposal, checked by the same rules
the minimum uses.

1. Install Ollama from [ollama.com/download](https://ollama.com/download).
2. Pull a model that fits your memory:

   | free memory | try |
   |---|---|
   | 1 to 2 GB | `ollama pull qwen3:0.6b` |
   | 2 to 4 GB | `ollama pull llama3.2:1b` |
   | 6 GB of GPU memory, or 8+ GB of RAM | a 2 to 4 billion parameter model, such as `gemma4:e2b` |

3. In Void Squid: **Setup > Check this machine**. *Local model* turns green.

---

## 5. Vision (Void Bicho)

**What it is for:** tagging each thumbnail by what it looks like (a tree, water, a
mushroom) with CLIP, and cutting shapes out of pictures with SAM. A GPU makes it much
faster; the CPU works. Only the tagging step uses it.

Void Bicho is a sibling repository. Clone it beside Void Squid, then make its Python
environment:

```sh
git clone https://github.com/migriv24/VoidBicho
cd VoidBicho
python -m venv .venv
# Windows: .venv\Scripts\activate      Linux: source .venv/bin/activate
```

Then install, from smallest to largest, only what you need:

| for | install | download size |
|---|---|---|
| tagging, on the CPU | `pip install torch --index-url https://download.pytorch.org/whl/cpu`, then `pip install open_clip_torch pillow` | about 1 GB |
| tagging, on an NVIDIA GPU | instead of the CPU line: `pip install torch torchvision --index-url https://download.pytorch.org/whl/cu128` | about 3 GB |
| cutting out shapes (SAM 2) | `pip install sam-2` (see SAM 2's own instructions) | adds about 1 GB of weights on first use |
| matching words to words | `pip install sentence-transformers` | about 0.5 GB |

Model weights are downloaded on first use into each library's own cache, outside every
repository. **Setup > Check vision deeply** asks PyTorch whether it sees your GPU.

Void Squid looks for Void Bicho beside its own checkout, or wherever `VOIDBICHO_ROOT`
points.

---

## 6. A coding agent

**What it is for:** everything the minimum cannot do alone, above all decompiling a game
nobody has decompiled. Void Squid is built so an agent can drive it from a terminal,
with every action logged in the same journal the window writes:

```sh
squid --describe                                    # what Void Squid is and what it can do, for an agent arriving cold
squid --state my.squid --import-library <folder>    # File > Open library, from a terminal
squid --state my.squid --allow-effects=loadgame effect loadgame <game file>
squid --state my.squid --allow-effects=setup effect setup
```

Effects (anything that touches files, programs or the network) are **refused** unless
the command grants them, and a person's window always outranks an agent on the same
project.

- **[Claude Code](https://claude.com/claude-code)** or **[Codex](https://github.com/openai/codex)**.
  Void Squid also finds the copy of Claude Code inside the VS Code extension.
- **[universal-modder](https://github.com/rehan-remade/universal-modder)**, skills and a
  knowledge base for modding PC games with an agent (its `um` command).
- **[REA](https://github.com/morluto/rea)**, agent-driven reverse engineering over
  Ghidra, JADX and others (`npx rea-agents`).
- **[Ghidra](https://ghidra-sre.org/)**, for decompiling native code.

---

## 7. A hosted model with your own key (planned)

A hosted model (OpenAI, Anthropic, DeepSeek) through your own API key, for people with no
local GPU and no coding agent. **Not built.** It costs money per use, and it needs careful
work on what is sent and how a shared project records who spent what, so it waits.

---

## 8. Where Void Squid keeps things

| what | Windows | Linux |
|---|---|---|
| your projects (`.squid`, with a `.log` journal and a `.lock` beside each) | `Documents\Void Squid` | `~/Documents/Void Squid` |
| settings, recent projects, window layout, the downloaded community list | `%APPDATA%\VoidSquid` | `~/.config/voidsquid` |
| libraries | wherever you put them | wherever you put them |

To remove Void Squid, delete the build folder and the settings folder. Your projects and
libraries are yours; they stay until you delete them.
