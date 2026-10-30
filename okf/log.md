# Log

Dated history of this bundle. Reserved file (OKF v0.1): listing and history only.

---
<!-- forge: added 2026-10-28, deep dive + logic translation -->

## 2026-10-28 — Deep dive: Ocean, Onda, Core's interaction nets, and the logic translation layer

The author pushed back on "impossible" and was right. Read the whole family before judging:
Void Ocean (fields, gravity primitives, the commutation-defect translation), Void Onda (a motion
meaning vocabulary), and Void Core (interaction nets; Reduce; mantle composition via box).
Three findings that change the plan:

- **The world half of SMG is already native.** The ds-mashup runtime (`goomba_galaxy2`) plays
  real Good Egg Galaxy geometry, collision, 106 gravity fields, Launch Stars, music and Mario's
  rig, all recovered from the owned disc. The gap is the **behavior** half, not the world.
- **"Logic" is not one thing.** It decomposes into registers — physics, motion, rules,
  presentation, narrative — each with a built pure home (Ocean, Onda, Core+Allomone, Jelly).
  A port is a per-register translation holiday whose residual is a commutation defect, and a
  target device is a profile of per-register capabilities.
- **AI proposes; the verifier decides.** Bicho classifies/proposes; the match loop and the
  defect hold correctness. Embeddings are for the shelf, not for translation.

New page: [The logic translation layer](/design/logic-translation-layer.md). SMG's remaining
work is the rules register and the camera, carried one function at a time through that model.

| tests | Squid 7 of 7 |
|---|---|

<!-- forge: added 2026-10-28, deep dive + logic translation -->

<!-- forge: added 2026-10-28, split stage -->

## 2026-10-28 — The split stage is real: main.dol disassembled into the Molerat IR

The author asked to work the remaining stages with real tools adapted to the structure. The
split stage — PowerPC bytes into the normalized foreign IR — is now real.

- **Built `forge/toolbox/ppc_split.py`** (Capstone 5.0.7, PowerPC 32-bit big-endian, installed
  as an invoked toolchain, never vendored), reached as `effect forge split <main.dol>
  [--out <ir.json>] [--limit N]`. A linear sweep plus recursive traversal from the entry point,
  following direct branch and call targets; honest residual: indirect branches and paired-single
  instructions are not followed, and names are addresses until a decompilation names them.
- **Ran it on the real `main.dol`** (Super Mario Galaxy, RMGP01): 32 functions from entry
  `0x8000403C`. The first is the entry itself — calls into `0x800041A8`, `0x800042E0`,
  `0x80004238`, then stack saves. Molerat consumed it: 32 models, 32 mappings, 80 residuals,
  3 holidays; compose produced 181 runes and 148 links (the call graph survives); build wrote a
  **validated `smg1.squid`** into `~\Documents\SMG-Native-Port\smg-recon`.
- **Fixed the test artifact.** `--windowed` now adds `-mwindows -municode`, and the rebuilt
  `plant-demo.exe` is PE subsystem 2 (GUI): the Desktop shortcut opens a window that stays
  instead of a console that flashes and closes.

| tests | Squid 7 of 7 |
|---|---|

<!-- forge: added 2026-10-28, split stage -->

<!-- forge: added 2026-10-28, plant run -->

## 2026-10-28 — The plant runs: a real executable from the forge's assembly line

The author asked for the exe to be made "the long way" — the manufacturing plant, not the
product. Built the remaining stages and ran the whole line on a real input.

- **Built: the real verifier and two build stages.** `effect forge match --verifier compile
  --candidate-dir <dir>` now compiles each candidate with a real compiler, runs it, and
  compares its stdout to the oracle (through a pipe, after `cmd /c` mangled the first
  shell-redirection attempt). `effect forge compile <src.cpp> <out.exe>` builds; `effect
  forge shortcut <exe> <name>` places a Desktop `.lnk`. The `c-host` device joined the
  fixture library.
- **Ran the whole line end-to-end** on a synthetic-but-real input (three C++ candidates,
  two functions with stdout oracles): route `c-host → pc-x86_64-windows`, match (`greet`
  accepted after 2 attempts, `camera_yaw` rejected after 2), compile `ok.cpp` to a 252 KiB
  `.exe`, shortcut to the Desktop, run → "hello from the forge".
- The plant is the deliverable; the demo exe is its test artifact. The SMG-specific stage —
  PowerPC decompilation plus the GX/AX/DVD platform layer — is the part a real toolchain
  (Ghidra/splat, then AstroCore's runtime) plugs into; the forge reaches it and names it,
  it does not skip it.

| tests | Squid 7 of 7 |
|---|---|

<!-- forge: added 2026-10-28, plant run -->

<!-- forge: added 2026-10-28 -->

## 2026-10-28 — The forge: a device library and the cross-device match loop

The author's direction: decompilation is game-to-any-device, not game-to-PC. First slice built.

- **Built: `effect forge`** (`src/forge/`, a pure `squid_forge` target, `tests/forge_test.cpp`).
  `devices <manifest>` lists the installed device plugins; `route <manifest> <source> <target>`
  plans the subsystem bridges (the cpu always, then each source subsystem remapped or marked
  `reimplement`); `match <task.json> [--manifest ...]` runs the match loop with a **synthetic
  verifier** — no real toolchain is invoked yet, and the report says so.
- **Pure, like MoleRat:** the verifier is injected, never linked; the host does the reads.
- Fixtures: `forge/fixtures/device-library.json` (wii, pc-x86_64-windows, android-arm64) and
  `forge/fixtures/match-task.json`. The wii→pc route names every bridge the platform layer
  needs, as data.
- [The Forge](/concepts/forge.md) is new; the index. Then, at the author's permission, the
  two halves were built in the siblings: Void Bicho gained a `propose` rule baseline
  (`python/bicho/adapters/propose_adapter.py`, `kind: propose`, tests green), and Void Jelly
  gained a pure `DeviceProfile` for scene capacity
  (`include/voidjelly/device_profile.hpp`). Both are deterministic baselines and say so;
  a real verifier and a learned proposer remain planned.
- The SMG recon project lives at `~\Documents\SMG-Native-Port`, outside this repo;
  no game bytes enter the repository.

| tests | Squid 7 of 7 (`squid_forge` new) |
|---|---|

### Did not change

No sibling edited; no game data in the repository; nothing committed.

<!-- forge: added 2026-10-28 -->

---
<!-- ocean: added 2026-10-18 -->

## 2026-10-18 — Motion in the studio (S34 b), and Void Ocean's gravity seam measured (S33)

Agent B, concurrently with agent A's MoleRat. Void Onda and Void Ocean are sibling repositories,
read-only from here; this session did their Squid side only.

- **Built: `effect motion`** (`src/motion/`, a pure `squid_motion` target, `tests/motion_test.cpp`).
  `judge <library> [out]` starts the runtime's `--autotest judge`; `report <folder>` counts the
  `judge.json` and `pace.json` it wrote, passing Onda's verdicts through unchanged. **Measured**
  through the headless front-end: 105 clips over 24 characters, mean 99.7%, lowest Tiny Kong's
  synthesized landing at 97.0% [93.6–98.6], no weak checks; feet keep pace 75/74/55% at 3/6/9.5 m/s
  (were 28/5/45%).
- **Specified, not built: Void Ocean's seam.** S33's lean (b) stands, so nothing links Ocean. Read
  from both codebases and counted on Good Egg Galaxy: 49 of 109 gravity fields have no exact Ocean
  primitive (28 bounded planes, 21 rotated boxes), and 14 carry only up to `distant`. Ocean's to
  answer.
- [Motion and physics in the studio](/design/motion-and-physics-in-the-studio.md) is new; S33 and
  S34 annotated; rung 8 item 8; the index.

| tests | Squid 5 of 5 (`squid_motion` new) |
|---|---|

### Did not change

No sibling edited; no game data in the repository (the judge's strips went to a scratch folder);
`void.json` unchanged (the op uses effects it already lists: process, file-read, file-write).
Nothing committed.

<!-- ocean: added 2026-10-18 -->
---
<!-- molerat: added 2026-10-18 -->

## 2026-10-18 — Void MoleRat's first vertical slice (toy-ledge) is built

Agent A, continuing the same day's design work. **The synthetic slice is current; the
answer-application seam is not.** `squid_molerat` (pure: `ir.cpp`, `research.cpp`,
`emit.cpp`) parses a normalized foreign IR, recovers reads/writes/calls and a call order,
states model/mapping/residual/holiday records, maps them onto `<run>:game` and
`molerat:<run>`, computes the normal form with no scaffolding, and emits a `.squid`
document. `effect molerat` provides gather, research, compose, residual, build, status,
task, answer and verify.

Measured: `squid_molerat_test` passes; `ctest --test-dir build -R squid_` is 6 of 6,
including the headless smoke's gather → research → compose → build → verify path. On
`molerat/fixtures/toy-ledge/foreign.ir.json`: 5 models, 9 mappings, 10 residuals, 3
holidays; the document validates (`{"ok":true}`); `verify` passes all four checks. A claim
with no evidence was refused with `ok:false` and no `answers/` write.

Corrected honestly in the same pass: `void.json`'s `provides.effects` lists effect kinds,
not op names, so `"molerat"` was removed and the ops named in `effects_note` (they use
file-read, file-write and clock, all already listed); the concept page's "an accepted
answer becomes one `batch`" is now marked planned, because the slice stores validated
answers and does not yet apply them; and design §5/§8.2/§8.3 were aligned with the code
that landed (one ruleset source in `emit.cpp`, no `resume`/`--commit`).

Not built: applying an accepted answer as one dispatcher `batch` with undo/journal
attribution, `resume`, `compose --commit`, the studio workspace, and any real
disassembler or toolchain invocation. No Core, Maiz, Mago or sibling change.

<!-- molerat: added 2026-10-18 -->
---
<!-- molerat: added 2026-10-18 -->

## 2026-10-18 — Void MoleRat: a decompilation-and-re-expression harness (design)

Agent A, concurrently with agent B's Ocean/Onda work. **Nothing built.** A new concept page,
[Void MoleRat](/concepts/molerat.md), and `molerat/design.md`: the forge-not-artifact
railguard applied; the pipeline ingest → disassemble → IR → structure → **research** →
re-author → emit; the research phase's four records (model, mapping, residual, holiday) as
runes in one mantle per run, with a generated OKF projection; the agent harness's bounded
task/answer protocol (no model inside); and the first vertical slice on a synthetic IR
fixture, so no game data is involved.

Two findings recorded rather than worked around: Void Core has no host-verb registration
seam, so MoleRat adds `effect molerat` ops (gated, and effectful by classification); and
Reduce's `patch` is content-only, so retyping is the pure mapper's job and rule-driven
retyping is a proposed Core extension, not a fork. Mago was not built in this checkout, so
`mago scan`/`matrix` were not available; no manifest was added for MoleRat.

<!-- molerat: added 2026-10-18 -->
---

## 2026-10-14 — Proposals on the Turkey card, Donkey Kong 64's music, feet that keep pace

The author: *"continue with development (find some stuff in the roadmap and continue)"*. Three roadmap
items and one fix:

- **The pilot's proposals land in Turkey** (rung 8, item 2). `effect pilot propose <library> [model]
  [count] [turkey]` keeps every proposal in the library (`pilot_proposals.json`); a turkey with no wiki
  page shows the proposal, the guards it passed and the shortlist, with **Accept** (sets `wiki`, source
  `local:<model>`) and **Not this** (recorded on the turkey, never proposed again), both commands in the
  journal; or **Ask the local model** for one turkey. Measured: gemma4:e2b proposes "Star Bit" for
  Galaxy's `StarPiece`, both answers agreeing, every guard passed.
- **Donkey Kong 64's music as MIDI** (S36, step 1): `tools/dk64_music.py` in the exercise reads the
  N64's compressed MIDI (back-references, durations instead of note-offs, loop markers) and writes
  Standard MIDI Files. 174 songs (table 0's entries 1–174; entry 0 not tried), 165,821 notes; Void Loops Studio's own reader opens all 174;
  one song (60) stops early on an escape the format as read does not explain (residual). The DK64 route
  writes them into the workbench.
- **Feet that keep pace** (Void Onda's rungs 1 and 2): Galaxy's implausible speeds were an eye joint
  read as a foot; a pixel check of foot sliding; the runtime now plays the walk, run or dash whose
  measured pace is nearest the speed, which cut sliding at 6 m/s from 85% of contact frames to 35%.
- **Fixed**: a crash in the new proposal box (a JSON temporary iterated after it died), found by the
  screenshot walk; its stale lock on the author's project removed.

- **Made gaits that plant** (Void Onda's rung 2b): the Kongs' walks and runs now keep pace with the
  ground 83/78/57% of contact frames at 3/6/9.5 m/s (were 26/30/29%); Lanky Kong's long arms the residual.

### Did not change

No music, game data or model weights in a repository (the MIDI files are in the person's workbench).
Nothing committed.

---

## 2026-10-13 (evening) — Load a game in one button, the Games window, Void Onda, the Kongs

The author: Load a game should *"fully work"*: a couple of buttons, loading bars, downloading and
decompiling, the minimum setup by default and configurable; Mario Kart DS's triangles do not match
up; Mario's animations do not fully load; Super Mario 64 DS's timings are off; *"we might need
another void thing… Void Onda"* for motion, with tags, blending, and a layer games map their own
animation systems onto; Donkey Kong 64's characters; a window to manage loaded games and their
progress; whether Void Loops Studio can carry music. Then, mid-session: many statistical and
pixel-level tools for judging motion over time, verdicts as rates (*"passed with about 85%"*), the
game's own code as the reference, a 3x3 gallery to fix motion by choosing, and small local models
rather than one large one.

### Built

- **Load a game, one button** ([load a game](/design/load-a-game.md) §5b): choosing a file reads it;
  **Load it** runs the route with the Settings tier (Minimum by default); the route downloads its
  own tools (apicula, nodtool, dk64_lib, Python packages, Petari's names) into the settings folder
  and writes `ingest.progress.json`; the window draws a loading bar per step.
- **The Games window** ([decompilation progress](/design/decompilation-progress.md)): per game, the
  community decompilation's figures (decomp.dev, `effect decomp`), wiki coverage, the library, where
  it was loaded from, **load again**.
- **[Void Onda](../../VoidOnda/okf/index.md)** founded: clips with real loop modes and interpolation,
  a vocabulary of verbs and per-game dictionaries, a mixer with crossfades and speed matching,
  analysis, retargeting, synthesis from a skeleton, a computer-vision toolkit with verdicts, the 3x3
  gallery. The runtime plays every character through it; **G** opens the gallery.
- **The judge** (`--autotest judge`): every clip of every character rendered unlit and judged from
  pixels, including pixels predicted by ray casting the runtime's own mesh.
- **[The cast](/design/the-cast.md)**: 25 characters. The texture-wrap bug (the "two heads"), Super
  Mario 64 DS's eye scale, Galaxy's frozen rotations and forced loops fixed; Donkey Kong 64's five
  Kongs read from the ROM by a new actor reader and moved by Onda's synthesized gait.
- **Void Jelly**: materials carry a texture wrap per axis (written only when not repeat, so hashes
  hold); the glTF reader reads samplers; the studio's views honour it.

### Measured

| what | result |
|---|---|
| tests | Squid 4 of 4; Jelly 9 of 9 (a wrap test added); Onda all checks; Ocean all checks |
| the judge | 105 clips over 24 skinned characters: lowest 97.0%, mean 99.7%. With wraps switched off on purpose, the racers fall to 55–88%: the oracle catches the bug it was built after |
| Load a game | tools into an empty folder: apicula 0.8 s, nodtool 1.1 s, dk64_lib 23 s; Donkey Kong 64 ROM to library through `effect ingest` in about a minute |
| the gallery | a simulated person reaches a hidden target in 7 rounds (Onda's tests) |
| decomp.dev | Super Mario Galaxy 100% code, 88% data; Mario Kart DS 1.5% code; the other two not tracked |

### Found

- **The bug the author saw as broken triangles was texture wrapping**: three formats state it, four
  of our readers dropped it. A judge that predicts each pixel from the mesh catches it at once.
- **A clip's name is not its meaning**: the fall had been mapped to a landing. Meanings are now
  claims with a confidence, and the ones that matter were watched.
- **Some clips are not played in time**: Mario Kart DS's `drive` is a steering lean.
- **Knuckle-walkers defeat height rules**: finding limbs needs the hierarchy.
- **No small general optimiser exists to drop in**; the classical preferential methods need none
  ([small models](../../VoidOnda/okf/design/small-models.md)).
- **Void Loops Studio cannot play game music yet** (no realtime engine; a feature freeze), but its
  model fits, and Donkey Kong 64's music is the N64's compressed MIDI
  ([music and Void Loops Studio](/design/music-and-void-loops-studio.md)).

### Written

Pages: the cast, load a game §5b, decompilation progress (the Games window), music and Void Loops
Studio; S32 annotated; S34 (Onda in the studio), S35 (small models), S36 (music) new. Void Onda's
whole bundle. A message to Void Loops Studio.

### Did not change

No game data, rigs, textures, music, model weights or decompiled code in any repository. The tools
Load a game downloads go to the person's settings folder. Nothing committed.

---

## 2026-10-13 — The cast, progress, a small-model pilot, Void Jelly's reader, Void Ocean founded

The author, 2026-10-12: permission to write in Void Jelly *"to get it up to speed for void
squid right now"*; all repositories in a state that could go on GitHub; a Profile and a
Settings tab; no "Edit in 3D table" button, but views to switch between; *"why cant i play as
donkey kong, mario from galaxy, all the playable characters in mario 64 ds, or any of the
racers from the mario kart ds?"*; a tool for the progress of decompilation, compared with the
game's wiki and Wikipedia (*"every wiki link should be able to associate or link to some
rune/turkey"*), learning from AestheticAtlas; Wikipedia downloads; Load a game that continues
inside Void Squid; the MOSU tested with small models under heavy guardrails; and Void Ocean
begun, in a research phase.

### Built

- **[The cast](/design/the-cast.md)** (in the exercise): 20 playable characters with skeletons,
  skin weights and animations in one rig format, switched with Q or chosen in the Feast's
  Play as grid. Galaxy's Mario and Luigi, Super Mario 64 DS's four, all thirteen Mario Kart DS
  racers, a Goomba. Donkey Kong 64's Kongs are not decoded (residual).
- **In Void Jelly, by permission**: a glTF reader (`read_gltf`, on vendored cgltf 1.14, MIT)
  with a residual for what it flattens and omits (skins, animations, cameras, lights); a
  material table (`sq-material` runes, textures as content-addressed blocks); vertex colours;
  `.ink` format 3, written only when a scene has materials, so every older hash holds; the GL
  nib's `textured()` profile. Nine of nine Jelly tests pass.
- **3D views in the studio** (`src/studio/view3d.cpp`, `src/viewport.cpp`): a turkey and a table
  drawn through Jelly's GL nib inside ImGui, with textures, vertex colours and alpha. The Table
  switches between **From above**, **Objects** and **In 3D**; the Turkey between **Card** and
  **In 3D**. The "Edit in the 3D table" button is gone. `effect world` opens a world file as one
  scene (480 objects, 173k triangles for Good Egg Galaxy, sky skipped).
- **Profile and Settings** (`src/studio/settings.cpp`, `src/host/paths.cpp`): a name, a colour and
  a line about the person, who signs edits as `human:<name>`; settings for look, places (the
  runtime, Bicho, the toolbox, projects), models (the Ollama model) and data.
- **[Decompilation progress](/design/decompilation-progress.md)** (`src/library/progress.cpp`):
  each wiki page a thing, place, system or other game; matched to turkeys and tables; missing
  pages ranked by how many pages link to them (10,670 "mentions" links, AestheticAtlas's idea).
  **Wikipedia** (`ds-mashup/toolbox/wikipedia_fetch.py`): four articles' key-section links and
  summaries, into the person's cache.
- **Load a game continues inside the studio**: `effect ingest` runs a route (`toolbox/route.py`:
  dk64, ds, smg) detached, the window follows its log, and the new library opens when it is done.
  Donkey Kong 64 into a new library in 59 s.
- **The pilot** (`src/library/pilot.cpp`, `effect pilot`): a small local model proposes a turkey's
  wiki page from a deterministic shortlist, under seven guards.

### Measured

| what | result |
|---|---|
| tests | Squid 4 of 4; Jelly 9 of 9; Void Ocean's one binary, all checks |
| progress | Galaxy 19 of 433 wiki pages; DK64 5 of 148; MKDS 16 of 138; SM64DS 7 of 202 |
| the pilot, 25 known cases | the rule alone 18 right, 7 wrong; qwen3:0.6b 7 right, 0 wrong; llama3.2:1b 14 right, 0 wrong; gemma4:e2b 17 right, 0 wrong (after the exact-name guard; before it, 2 and 3 wrong) |
| the cast | 20 characters, each drawn and animated by `--autotest cast` |

### Found

- **A rule beat a better prompt.** Every wrong accept was a near name beating the exact one
  (Banana → Banana Coin). The guard that fixes it is four lines.
- **The progress ranking finds the matcher's gaps**: Peach is playable and Star Bits are a kind,
  but `Peach` and `StarPiece` reach no page by name.
- **MSYS2's linker crashes when it should report an error** (binutils 2.45.1): an undefined
  symbol gives exit 5 and no message. A missing function looks like a broken toolchain.
- **Rebuilt normals can point inward**, and nobody notices on a textured model until it is lit.

### Written

[The cast](/design/the-cast.md); [decompilation progress](/design/decompilation-progress.md);
S29 answered with measurements; S32 (does `.ink` grow skins?) and S33 (when does Squid link
Void Ocean?) new; the roadmap's rung 8 trued up; the index. Void Jelly's log, index and
reader page. **[Void Ocean](../../VoidOcean/okf/index.md)** founded beside it: fields, gravity
primitives, the terrain lift, the commutation defect, and Minecraft Ring and the GTA V
passthrough read as lessons.

### Did not change

No game data, no wiki or Wikipedia text, no rigs, no model weights in any repository. Nothing
committed. The game model is still design only (S14).

---

## 2026-10-12 (evening) — Void Squid 0.1: the studio, a real application

The author: build the first real version, *"a reall application i can launch with void maiz
right now"*, properly on Void Maiz (menus, windows, the widget registry); a project file that
opens the asset database the way `.miga` opens Hormiga's; Hormiga's look and Data tab; keep
the stars; a desktop shortcut. And the frame around it: Coral is for sharing *play* (hosts,
servers, sessions) while editing together is Maiz's; Load a game must serve three kinds of
person on four setups (a minimum with no model, a local model, an agent, a hosted model
later); no game data and no demo in a release; Linux as well as Windows; and *"void squid is
NOT about decompilation. It's about the creation of NEW things."*

### Built

- **The studio** (`src/studio/`): a docked window with Start, Turkey, Table, Feast, Data and
  Scene workspaces, and Shelf, Inspector, Console, Load a game and Setup windows; Hormiga's
  spacing, Lato with Font Awesome, a deep-sea palette over the starfield. Every edit is a
  command through a `maiz::Session`; forms are Maiz's `widget_form` over six host glyphs.
- **Projects**: a `.squid` is a Void Core state document (name provisional, S12), with its
  journal and lock beside it; recent projects, layout and settings live in the person's
  settings folder.
- **The library** (`src/library/library.cpp`): `assets.db`, `worlds.json` and `feast.json` read
  into six mantles and spliced into the project, keeping a person's tags, removals, kinds and
  sounds; ids derived from names so copies merge.
- **Load a game** (`loadgame.cpp`, `catalogue.cpp`): recon, the seed catalogue
  (`data/known-games.json`), the community list downloaded on request, and a plan whose every
  step is a checkable rule. **Setup** (`setup.cpp`): the machine, the tools, the setups.
- **Effects** declared for the briefing: `library`, `setup`, `loadgame`, `catalogue`, `play`,
  `table3d`, `reveal`. Play writes the feast and starts the exercise's runtime.
- **Host pieces** (`src/host/`): paths and processes with Windows and POSIX halves; file
  dialogs from Hormiga (zenity, kdialog, yad, qarma on Linux).
- **Release pieces**: two executables (`squid` console, `voidsquid` windowed); an icon drawn
  here (`packaging/`); `SETUP.md`; `vendor/` (SQLite, nlohmann/json, stb, fonts) with
  `THIRD-PARTY-NOTICES.md`; `LICENSE`; version 0.1.0 in CMake and `void.json`. A shortcut on
  the author's desktop, and a `four-games.squid` project in their Documents made from the
  local test library (never exported).

### Measured

| what | result |
|---|---|
| tests | 4 of 4: `squid_headless_smoke`, `squid_sim`, `squid_read`, and the new `squid_library` (title folding, the community list's Markdown, matching, the plan, the splice keeping a person's work, the runtime files) |
| library import | 235 turkeys, 235 thumbnails, 5 games, 12 kinds, 74 tracks, 7 tables in 0.8 s, through `replace_state` (commands would take tens of seconds, by Core's own `rune new` measurement) |
| setup inventory | 0.4 s: Python 3.14, Ollama with three models, Void Bicho, Claude Code inside the editor extension, the RTX A1000 |
| Load a game | Donkey Kong 64 matched at 100 with a route; Super Mario Galaxy's `.rvz` at 100 once `gamecube-or-wii` counted as either; the community list: 485 entries |
| the screenshot walk | every workspace drawn from the shortcut's own binary on the four-game library |

### Found

- **The project file is not an `.ink`.** The author remembered it so; the notes say `.ink`
  is Jelly's scene and the game document's name is the author's (S12). `.squid` is
  provisional.
- **A lock cannot tell a crash from a window.** The first test run crashed (an ImGui
  assertion: a row moved the cursor and submitted nothing), and its lock kept the project
  shut. The window now asks the person; Maiz is told a pid would let a host check.
- **A level's far scenery hides the level.** Galaxy's sky objects sit 20,000 units out; the
  Table's map fits the middle 90%.
- **The plan nearly claimed a feature that does not exist** (a local model naming things).
  It now says *planned*.
- **Claude Code is often not on PATH**: the editor extension ships its own binary. Setup
  looks there too.

### Written

[Void Squid 0.1](/design/void-squid-0.1-the-studio.md); [Load a game](/design/load-a-game.md);
[Coral](/concepts/coral.md) redrawn; open questions S6, S12, S23, S25 annotated with the
author's direction, S28–S31 new; the roadmap's rung 8; [the index](/index.md); `README.md`,
`SETUP.md`, `void.json`. Messages: Void Maiz (two hosts, one shell; a lock with a pid), Void
Jelly (a glTF reader so the studio can draw a turkey).

### Did not change

No game data, no wiki text, no model weights in the repository. The game model is still
design only (S14); rung 3 still waits on Jelly's DS nib. Nothing committed.

---

## 2026-10-12 — Four games and the asset database: Donkey Kong 64, wiki tags, work set-ups, Coral

The author on Goomba Galaxy 2: the menu flickered shut; textures were missing or things
absent; Turkey, Table and Feast should be *"different work set ups"*; handles in 3D; music
per map and sounds per turkey, with their own files; their own models with collision; then
Donkey Kong 64; a database like Void Hormiga's, with tags made from each game's wiki; the
GPU. Mid-session: Void Squid's own Antfarm, named **Coral**, for sharing with players later.

### Built (in `../ds-mashup`, not here)

- **Fixes:** a key held over a newly enabled input map no longer counts as a press (a
  key-level script device tests it); Galaxy's material colour registers are read (202 models
  converted again); star-bit rings and ocean spheres are built by Petari's rules; both
  programs ask for the discrete GPU (they had run on the Intel Iris Xe).
- **The turkey table's three set-ups**, move/turn/size handles, music per map from the disc's
  74 streams or the person's files, sounds per turkey, dropping in models (collision from
  their shape) and audio, the feast read by the game.
- **Donkey Kong 64:** Jungle Japes and DK Isles, playable; geometry by dk64_lib (GPL-3,
  invoked); setups, the game's folder names and the banana sprites read here.
- **The asset database** (`assets.db`, Void Hormiga's schema) with tags from the games, 896
  wiki pages, names and CLIP; a device inventory (`devices.json`); the Coral journal.

### Measured

| what | result |
|---|---|
| the menu | a scripted Esc held for six frames: the menu stays open (it closed the next frame before) |
| the GPU | both programs report the NVIDIA RTX A1000 (they had run on the Intel Iris Xe) |
| Galaxy textures | 202 models converted again with their material colours; 12 star-bit rings (26 more star bits), 7 swimmable ocean spheres |
| Donkey Kong 64 | Jungle Japes and DK Isles playable; 74 banana sprites in Jungle Japes; the probe test grounded in both (161 of 180) |
| the asset database | 235 turkeys, 1,006 tags (633 game, 181 vision, 112 wiki, 93 name); 773 wiki entities from 896 cached pages; 24 matched, including Kuribo to Goomba by romanization; 115 vocabulary words; 74 tracks |
| the editor | its autotest builds a table across two games, gives it music, saves it and removes it again |

### Found

**Each fault had one cause hidden in data:** a held key, an unread colour register, a GPU
choice made by Windows, a Donkey Kong 64 vertex colour that is really a normal. **Tags
launder** unless what a builder writes back is kept apart from what the game said. **A wiki
lists things in sections, but mentions others in the same lines;** its categories name a
class when plural and a game when not, and a match counts only from the thing's own game's
page. **Galaxy's gravity contains every other game's so far**; the first system no game here has will come with
Minecraft (S27).

### Written

[Four games and the asset database](/design/four-games-and-the-asset-database.md);
[Coral](/concepts/coral.md); [turkey, table, feast](/concepts/turkey-table-feast.md) §6 (work
set-ups); sources [dk64_lib and the game wikis](/sources/dk64-lib-and-the-game-wikis.md) and
[translating between games](/sources/cross-game-transfer.md); S25 to S27; a message to Void
Hormiga (its schema adopted; Coral is not an Antfarm).

### Residual

- Donkey Kong 64: its collision and model-two geometry are not decoded, its actors are
  markers, and its lit meshes draw black on the lit path (cause not found), so its levels
  draw unlit.
- Sequenced music (Super Mario 64 DS, Mario Kart DS, Donkey Kong 64) does not play yet.
- The controller has not been tried by a person.
- Wiki matches and vision tags are proposals (S26). Hub pages bring franchise-wide categories.
- The castle's probe test fails because the author gave it a point gravity through the table,
  which pulls the player up off the lawn. The edit is kept; the table now offers point
  gravity only for planets, with a warning.

### Did not change

No game data, wiki text or model weights in any repository. dk64_lib is not vendored or
imported. Nothing committed.

---

## 2026-10-11 (afternoon) — Goomba Galaxy 2 and the first turkey table

The author on the first Goomba Galaxy: lighting, items and interactables, the camera, and a
castle world that collided with nothing needed work; music, sound, animation and run times
were good. Asks: a small research round on physics and lighting; a new version of the
level; controllers *"similar to Unity's controller abstraction"*; and the turkey table,
*"placing assets on a 'table' and designing levels"*. And an observation: *"we are always
adding MORE to the original base game."*

### Built (in `../ds-mashup`, not here)

- **A shared engine** (`src/g2/`): Jolt Physics 5.6.0 (vendored, MIT) with the level's
  gravity applied per body; a renderer on the game's own lights; the game's own cameras
  beside one written here; an input layer of actions, bindings and schemes.
- **Goomba Galaxy 2**, on that engine, with prompts, NPC lines from the game, Mario Kart DS
  items as physics bodies, and the castle grounds on their own decoded collision.
- **The turkey table**: 215 turkeys from three games, placement upright to the world's
  gravity, an inspector, undo, a transcript, Save and Play.
- **Toolbox steps:** `galaxy_extras.py` (lights, light areas, cameras, camera areas),
  `messages.py` (1,743 lines), `goomba_rig.py`, `turkey_shelf.py`; `kcl.py` now reads
  Super Mario 64 DS's units; `build_goomba_world.py` runs everything.

### Measured

| | result |
|---|---|
| Good Egg's own data newly read | 13 light settings, 33 light areas, 416 cameras, 76 camera areas (all resolve), 1,743 lines |
| castle collision decoded | 1,243 triangles; ground within 15 cm of the model at 98% of points |
| floor probe, standing 3 s | Good Egg 126 of 180 steps grounded; castle 162; the table 162 |
| a table built in the editor, played | the Goomba runs all the way round its planet, grounded throughout |

### Found

**Two formats each hid one fault.** Super Mario 64 DS's collision uses different
fixed-point units from Mario Kart DS's (normals 1/1024, lengths 1/2^22). Super Mario Galaxy
models, as converted here, carry one placeholder normal. Nothing in either game shows it.
**Lighting must be linear**, and it must know where its normals came from. **The world
file is the game model in miniature**: the game reads what the table writes. **The
original is the floor**: every step adds a layer, and each layer must say which one it is
(S24).

### Written

[Goomba Galaxy 2 and the turkey table](/design/goomba-galaxy-2-and-the-turkey-table.md);
[physics, lighting and input](/sources/physics-lighting-and-input.md); S23 and S24 in
[open questions](/design/open-questions.md); a message to Void Jelly about normals'
provenance and light data.

### Did not change

No game data in any repository. Jolt and raygui are vendored with licences in
`../ds-mashup/vendor/`. Petari was read for names and camera semantics; nothing was copied.
Nothing committed.

---

## 2026-10-11 — Goomba Galaxy: a decompiled level played as someone else; Bicho's models moved; vision research

The author: move the cached models *"somewhere more useful"*; *"do some modern academic
research into computer vision in the age of llms"*; and build Good Egg Galaxy with its music
and gameplay, played as a Goomba under Galaxy's physics, with planets to orbit, Mario Kart DS
item boxes from defeated enemies, a pause menu that loads Super Mario 64 DS's castle grounds,
Galaxy's sound effects, more trees, a water planet and a penguin.

### Built

- **Goomba Galaxy** (`../ds-mashup/src/galaxy.cpp`, `goomba_galaxy.cmd`): every request
  above, running. Worlds are rebuilt from the author's discs by
  `../ds-mashup/toolbox/build_goomba_world.py`; new toolbox steps are `galaxy_build.py`,
  `galaxy_dress.py`, `goomba_rig.py`, `castle_world.py` and `find_sounds.py`.
- **Void Bicho**: eight cached models moved into its store; SAM 3, CLAP, MiniLM and BLIP
  behind the contract (Bicho's own log).

### Measured

| | result |
|---|---|
| Good Egg Galaxy read from its tables | 13 zones, 220 objects, 106 gravity fields, 8 Launch Star rails |
| added by meaning | 24 trees, a water planet, 2 planets to orbit, a penguin |
| orbit test, three added planets, two headings | held by their own field 100% of 30 s, after one fix |
| sound effects | 12 picked by CLAP from 2,543 decoded waves; not checked by ear |

### Found

**Gravity is data, and it transplants.** The level's fields, read whole, carried a new
character on new planets with twenty lines of movement code. The one failure (a running jump
leaving a three-metre field) was fixed in data. Fields pass railguard §8's three tests
([Goomba Galaxy](/design/goomba-galaxy-exercise.md) §2).

**Editing by meaning works when the data says where up is.** Vision picked the trees, and
the level's own gravity decided where they could stand. The first attempt, without that
check, planted them upside down.

**The research agrees with the measurements.** Marking beats cropping (our 21% to 71%), and
concept segmentation still cannot see a role (SAM 3 found "toy", not "enemy").
A decompiled level is already the functional scene graph that 3D vision papers try to
infer from pixels ([vision in the age of language models](/sources/vision-in-the-age-of-language-models.md)).

### Written

[Goomba Galaxy](/design/goomba-galaxy-exercise.md);
[vision in the age of language models](/sources/vision-in-the-age-of-language-models.md);
updates to [seeing what was decompiled](/design/seeing-what-was-decompiled.md) §6, the
sources index and this bundle's index. In Void Bicho: its log, index and perception page.

### Did not change

No game data, model weights or decompiled code in any repository. Petari read for names and
folders only. Nothing committed.

---

## 2026-10-10 (night) — Seeing what was decompiled: vision tools, Bicho's first code, Super Mario Galaxy

The author, on the mashup: *"a jumbled mess of triangles and textures"*; without semantic
information, more animation, sound and enemies would be useless. Top priority: visual
information from decompilation, with Void Bicho providing the models (*"1gig or 2 is
fine"*). Midway: try **Super Mario Galaxy** too, and learn from its community decompilation
**Petari**, *"since they had humans the whole time."*

### Built

- **Void Bicho, first code:** a Python runtime with the catalogue, tag-query selection, the
  invocation contract, refusal as a value, `fetch`, `serve`, and adapters for **SAM 2.1
  Large** and **OpenCLIP ViT-B/16**, on the GPU (Bicho's own log has it).
- **The toolbox** (`../ds-mashup/toolbox/`): a headless renderer with an ID buffer, a lift
  from masks and labels onto triangles, segments, semantic models, and evaluators that score
  against a skeleton, a collision mesh or a decompilation.
- **Super Mario Galaxy:** disc via `nodtool`; Yaz0, RARC and BCSV readers; a **J3D converter**
  (joints, envelopes, GX lists, eleven texture formats); zones assembled from their own
  placement tables with per-triangle object identity; **Petari's 1,183 names** indexed to
  class, archive and category.

### Measured

| | result | baseline |
|---|---|---|
| DS Mario body parts, first try | 21% | 46% |
| … with red-outline context crops (the main lever) | **71%** | 46% |
| Galaxy identity from a model alone (22 characters and items, 27 names) | **14/22 top-1, 17/22 top-3** | 1 in 27 |
| Galaxy zone, triangles vs Petari's categories | 37% | 40% |
| … enemies found as enemies | **1 of 17** | |

### Found

**Vision and the decompilation answer different questions.** Vision recognises what things
look like and who they are. Petari's categories say what they *do* ("Enemy" is a code role:
it hurts Mario), and no picture shows that. Meaning needs both arrows, plus a link from
identity to role (a text model: Needle, as the author suggested). Animation names already
carry behaviour (`walk`, `flatdown`, `hipdropdown`, `swoon` in a Goomba's archive).

### Written

[Seeing what was decompiled](/design/seeing-what-was-decompiled.md). Messages to Void Jelly
(`../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-an-id-buffer-exists-and-ink-needs-textures-2026-10-10.md`:
UVs and textures in `.ink`; render-with-ID-buffer and lift as pure verbs).

### Did not change

No game data in any repository: the Galaxy disc, its shelf and Petari's clone live in
`Documents/games/smg-workbench`, and model weights in `%USERPROFILE%/.voidbicho/models`.
Petari is read for names and folders only, never built, never copied.

---

## 2026-10-10 (evening) — The DS mashup exercise: three cartridges taken apart and played as one

The author: work a decompilation project for real, *"mostly for gaining insights of the
nature of games,"* on their own Super Mario 64 DS, Mario Kart DS and Mario Party DS. Use
universal-modder as is, take notes, and end with *"a desktop application that i can run
that is this meaningful 'mashup'"*. They named five pitfalls: the touch screen, the many
Marios, Super Mario 64 DS gameplay on a Mario Party map, naming, and a file type.

### Built (outside this repository: `../ds-mashup/`, an artifact, not the forge)

- **Converters:** NitroFS, NARC, LZ10/LZ11 and Mario Party DS's own container; one decoder
  for the DS geometry engine's command lists; all seven DS texture formats; **Mario Party
  DS's HBDF models, reverse-engineered with no spec**; Super Mario 64 DS's BMD models with
  bones and head attachment; KCL collision with surface types; Mario Kart DS course maps;
  Mario Party DS board space graphs. apicula invoked as a toolchain for Nitro models.
- **`tools/build_shelf.py`:** ROMs to a shelf of 11 pieces and `shelf.json`, in one command.
- **`ds_mashup.exe`:** Super Mario 64 movement on Wiggler's Garden with its 58-space graph
  and a touch-screen dice roll; Figure-8 Circuit against Luigi on decoded collision,
  checkpoints and AI path; Bob-omb Battlefield. A mouse-driven touch panel, and an inspector
  that shows each piece's source, depth and residual. `run.cmd` starts it.
- **universal-modder, as is:** `um` installed in a private environment at the pinned commit.
  `kb search` found nothing for these games, `scan` saw only the emulator, `publish check`
  reported 0 failures, and `kb check` passed the exercise's field note.

### Found (the full list is [the exercise](/design/ds-mashup-exercise.md) §2)

The bytes depth is per developer, not per platform: three model formats on one console, one
shared trunk. Structure lives in data (graphs, checkpoints, surface types, bone names);
behaviour lives in code. A game's best oracle can be its ancestor (the N64 Super Mario 64
decompilation stands in for the DS game's opaque code). Every game has its own units, so
a mashup is mostly calibration. **Scale is narrative**: Mario Party DS shrinks its players,
and normalising the board would erase that. The camera belongs to the mode, not to the
character. Two rule systems can own one body in turns, with hand-back triggers. A cartridge
is already a mashup of tables.

### Answered

The author's five questions, in [the exercise](/design/ds-mashup-exercise.md) §3: the touch
screen is an input holiday plus a second projection; Mario is one identity with
presentations by role; Super Mario 64 DS gameplay on a Mario Party map needed calibration, a
collision source, a camera change and an ownership rule; names need internal, canonical and
display layers; and the "file type" is a document, not a binary format, which is `shelf.json`
and is proposed as the game document's decompile form (**S22**).

### Checked

`ds_mashup --race-test` finishes three laps on decoded checkpoints (39 s each, kart speed
calibrated to that, not recovered). `--autotest` passes through all three modes with seven
screenshots looked at.

### Recon on real dumps

All three cartridge headers verified by CRC, recon's first run on real games. Rung 9a is done
for the DS.

### Did not change

v0's exit condition is rung 3. No game bytes or decompiled code are in any repository. The
workbench is local to the author's machine. Squid still computes no geometry: every vertex
loop is in `../ds-mashup/`, and the DS reader was offered to Void Jelly instead.

### Messages

Sent to Void Jelly: `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-a-ds-reader-exists-and-it-is-yours-2026-10-10.md`
(own the DS reader as the DS nib's inverse? per-axis dequantisation in `.ink`?).

---

## 2026-10-10 (later) — Decompilation: recon built, the program written, Bicho made load-bearing

The author: decompiling is as important as compiling, and the point is **structure**, not
source for its own sake: recover game objects and their behaviours, shelve them like
Roblox's toolbox behind a *"decompiling game"* progress bar, and compose new games from
several decompiled ones in the studio (*"making the pig from minecraft be the player"*).
A game is *"just another modality for data to exist in,"* like a video or a `.blend`. Use
holidays far more, because two games share no systems yet modders have found baselines.
Fork universal-modder toward fully local decompilation, with smaller specialised models,
a human in the loop, perhaps RL, federated one day. And: **Void Bicho is an extremely
important layer to Void Squid.**

### The author's decision

- **Void Bicho is load-bearing for Void Squid** (decompilation's identify, decompile and
  lift stages, and the shelf's search). Recorded in [decompilation](/design/decompilation.md)
  §8 and sent to Bicho as a message, not an edit.

### Built

- **[Recon](/concepts/recon.md)** (`status:current`): `squid_read`, a pure target that links
  only the standard library, and `effect recon <file-or-folder>`. It reads Nintendo DS,
  Game Boy Advance and Game Boy headers and **recomputes their checksums**. It reads N64
  headers in all three dump byte orders, GameCube and Wii disc headers, Godot, GameMaker,
  Unity and Ren'Py containers, and PE, ELF and Java executables. It fingerprints PC
  installs by listing. Every finding carries evidence and a confidence; routes list the
  tools for each depth and whether each is on PATH; the residual names what was not
  checked and points to `um scan` for PC installs. `tests/read_test.cpp` (the `squid_read`
  test) and two headless smoke cases.
- **A reference copy of universal-modder** in `third_party/` (pinned commit, MIT, 5.1 MB of
  28 MB). Upstream's agent entry files are renamed so they never load here; see its README.

### Measured, and found

All three Squid suites pass; the build has no warnings. A 20,000-file install answers in
**1,322 bytes**. The first real run, against universal-modder's own folder, called it a
Java game because of a Gradle wrapper four folders deep. Java and bare-executable signals
now count only in the top two levels, with a test. No real game dump was recon'd; that is
rung 9a.

### Written

[Decompilation](/design/decompilation.md): six traditions that already take games apart,
each with an honest metric (decomp.dev's percentage is an ingress residual). **Four
depths** (bytes, opaque, source, structure), each with an oracle: matching is the lens law
at the source depth. **Opaque pieces as runes whose behaviour is a holiday with a
contract**, after libsm64 and universal-modder's bridge contract. **The player taken
apart** into nine rows, Minecraft against Super Mario 64, with the author's two examples
as commands and residuals. The pipeline from recon to run, with owners. Auto and by-hand
as one surface, where every acceptance is a labelled example. The model kinds Bicho
needs, and the rewards that can be trusted. What a fork of universal-modder would change.

Also: [recon](/concepts/recon.md); sources [universal-modder](/sources/universal-modder.md)
(verified), [matching decompilation and recompilation](/sources/matching-decompilation-and-recompilation.md)
and [learned decompilers](/sources/learned-decompilers.md) (both asserted); open questions
**S19** (reference, toolchain or fork), **S20** (does decompilation move the order; lean:
split by depth), **S21** (is a contract a holiday; lean: yes). Trued up:
[construct and deconstruct](/concepts/construct-and-deconstruct.md),
[three uses](/design/three-uses.md), [what a game is](/concepts/what-a-game-is.md) (two
nouns §2 lacks: a viewpoint, and a contract), [build surface](/concepts/build-surface.md),
[roadmap](/roadmap.md) (rung 9 split into 9a–9e), `index.md`, `void.json`, `CLAUDE.md`.

### Findings worth keeping

1. **What two unrelated games share is a contract, not code.** The mashup community has
   written it down (ownership with hand-back, units as a shared function, channels,
   lifecycle, *not covered*), and it is a holiday's interface plus links plus a residual.
   *"Don't advertise a universal adapter"* is universal-modder's own conclusion, and this
   bundle's: the vocabulary generalises, the adapters do not.
2. **Decompilation has verifiable rewards**, and matching (byte-exact) is stronger than
   anything the learned-decompiler papers use. That is where this family could contribute
   to the field, not only borrow from it.
3. **Nobody classifies.** Every learned decompiler turns a function into C, and none says
   *"this is the player's jump."* That is the `classify` gap, and it is the stage that
   turns source into structure.
4. **A correction to the conversation, recorded so it is not repeated:** Super Mario 64
   plays in third person behind Lakitu's camera. It has a first-person look that cannot
   move. The viewpoint row of §5 exists because two games differ exactly here.

### What did not change

v0's exit condition is rung 3. Squid computes no geometry; recon reads headers, not
meshes. The game model is still design, behind S14. No game bytes and no decompiled code
are in this repository, and none will be.

### Messages

Sent to Void Bicho: `../VoidBicho/MESSAGE_FOR_VOIDBICHO_voidsquid-decompilation-makes-you-load-bearing-2026-10-10.md`
(training as an invocation, seconding Latin OS; a benchmark as a mantle of functions with
oracles; a `classify` kind).

---

## 2026-10-10 — Simulation, primitives, the author's rulings, and Needle to Bicho

### The author's rulings

- **S16, settled:** Allomone is *"a programming language for Void Core,"* a translation of
  interaction rules over a mantle into an easier interface.
- **S15, open with the author:** Turkey, Table and Feast are new things that share much
  of the meaning of rune, mantle and holiday; whether they become separate is undecided.
- **S17, delegated and decided:** see below, with research and multiple instances in mind.
- **S18, accepted:** the asset library will take forever and be worth it; basic shapes and
  primitives for now, semantic assets later.
- Needle belongs to Void Bicho rather than Void Core.

### Built

- **[Simulation](/design/simulation.md)** (`status:current`): `squid_sim`, a runtime that
  links nothing but the standard library; `table.cpp`, which compiles a scene and an
  Allomone rule set into a World (places from Jelly, behaviour from Allomone, inert data
  out); deterministic stepping, many instances over one shared world, forks that copy only
  dynamic state, bounded event logs and cross-instance comparison; `effect sim new | step |
  fork | log | compare`; `tests/sim_test.cpp`; a multi-instance section in the scale bench.
- **Presets** (`effect make traffic | shapes`) from **Void Jelly's new primitives** (box,
  plane, cylinder, sphere, cone; every face wound outward and tested).

### Measured

The traffic preset tells its whole story in **six events per run** across three seeds. Many
instances scale linearly (about 140,000 body-steps per second at 100 bodies, single
thread); bodies do not, because collision is all-pairs (36,000 at 400). A comparison of 64
runs is under 5 KB.

### Found, and fixed with a test

1. Cars aimed at the goal's centre and cut across lanes; they now aim at its nearest point.
2. A faster car behind a slower one logged 25 contacts in 10 s; it now follows to contact,
   and one blocking episode is one event.
3. **Allomone settles equal-strength ties within one script silently, by text order**
   (`"false"` over `"true"`, `"10"` over `"9"`). The table compiler now reports every such
   tie; the measurement went to Allomone.

### Elsewhere

Void Bicho: [Needle](../../VoidBicho/okf/design/needle.md) as its plan of record (Needle 3 is
121M, Apache-2.0, with embeddings, and its **telemetry is on by default**); Phase 1's first
runtimes named (ONNX Runtime, Needle's engine); a `void.json`. Void Jelly: primitives.
Messages to Void Core (Needle moved) and Void Allomone (ties; tier 1's forcing client).

---

## 2026-10-09 (evening) — Smart logs, the spatial log, and the three uses

The author asked for four things at once: SAM 3D Objects downloaded where it fits best;
research into 3D modelling mixed with AI; a far richer idea of what the CLI logs for
worlds of thousands of objects ("spatial logs"); and three directions for the engine
(decompile, create, simulate) with a creation vocabulary of Turkey, Table and Feast,
leaning on Void Allomone. Permission to write in Jelly, Maiz and Bicho.

### Where SAM 3D went, and why

Into **Void Bicho**, `third_party/sam-3d-objects/`, code only (6.8 MB after removing the
git history, 214 MB of stock sample photos and a GIF; no weights). Bicho's boundary page
already says a new model architecture is *another model in Bicho*; Jelly is pure and
Squid owns effects, not models. **It cannot run here**: it needs Linux and a 32 GB GPU,
and this laptop has a 6 GB RTX A1000 on Windows with a CPU-only PyTorch. Disk was at 98%
(24 GB free); nothing else was downloaded.

### Built

- **Jelly: spatial queries** (`src/spatial/`, `VoidJelly:/concepts/spatial.md`):
  `world_matrices` (now shared with the GL nib, so the parent-chain rule has one owner),
  `place`, `survey`, `near`, `look`, three verbs, a test suite. Pure; "what is on screen"
  answered by geometry with no GPU, and every answer says occlusion is not computed.
- **Jelly: `emit_gl` takes a precomputed hash.** The bench showed half of emission was
  re-serialising the Ink to hash it.
- **Squid: `effect survey | near | look`**, the camera moved to a GL-free `camera.cpp` so
  the headless build has it, a **Look** panel fed by one debounced `effect look` per
  settled camera, smoke-test coverage for all three, and `squid_scale_bench`.

### Measured (synthetic world, best of three)

| objects | full emit | emit, hash supplied | look | look answer | survey answer |
|---|---|---|---|---|---|
| 1,000 | 46 ms | 14 ms | 8 ms | 3.0 KB | 14.5 KB |
| 10,000 | 477 ms | 156 ms | 83 ms | 3.0 KB | 15.1 KB |
| 30,000 | 1,442 ms | 476 ms | 217 ms | 3.0 KB | 15.5 KB |

**Log answers stay fixed in size as the world grows; full re-emission is not interactive
past about a thousand objects.** Scope claim 4 is answered no, and Jelly's incremental
emission is now a measured requirement.

### Written

[Smart logs](/design/smart-logs.md) (`status:current` for the spatial log: four layers of
perception, structure / geometry / render / learned, each optional above the first);
[three uses](/design/three-uses.md); [Turkey, Table, Feast](/concepts/turkey-table-feast.md);
sources for ConceptGraphs/SceneGPT, MeshCoder (verified, abstract) and SAM 3D (verified);
open questions S15–S18. In Bicho: `perception.md`, `3d-ai-landscape.md`,
`third_party/README.md`. In Jelly: `spatial.md`.

### Findings worth keeping

1. **The 3D-understanding literature and the family agree.** ConceptGraphs and SceneGPT
   feed language models an object-centric scene graph as text; a mantle already is one.
   Smart logs are bounded subgraphs on demand, not a new representation.
2. **Allomone is the Table's language for truth, not yet for change.** It derives (built,
   terminating by theorem); Voidscript mutates; Allomone's tier 1 (amend) is the seam a game
   engine forces. Squid is a strong forcing client for it (S16).
3. **Hormiga 0.2.0 is the precedent for the three uses**: data-defined kinds and workspaces
   (one of which is a D&D 5e campaign). Decompile, create and simulate are three workspaces
   over one document, not three applications.
4. **Decompilation is what makes perception models matter.** A decompiled world arrives
   untagged; segmentation and captioning are how its turkeys get found.

### What did not change

Squid computes no geometry; the spatial queries are Jelly's. Models are Bicho's and
optional. v0's exit condition is still rung 3. Maiz got no new message: a 3D inspector
is not needed (its inspector edits a rune's fields whatever they describe), and the 3D
camera ask is already with them.

---

## 2026-10-09 (later) — First code: rung 2.5, the viewport and the headless front-end

The author: *"let's just keep developing,"* with permission to write in Void Jelly and to
message Void Maiz. The roadmap's first rung for this repository is 2.5, and it needs
Jelly's GL nib, which did not exist. So the session built both halves and ran them
against each other, which is what *neither half proves anything alone* meant.

### Built, in Void Jelly (its log has the detail)

The `Residual` type, the GL nib (`emit_gl`, pure, every decision a residual entry), the
`emit` verb, a test suite of nine cases, and two build changes a parent project needs.
Six of six Jelly tests pass. On the sample scene the control reads **fits, 18 entries,
0 dropped**.

### Built, here

- **`CMakeLists.txt`**: Maiz and Jelly as subdirectories; **one** Void Core, the prebuilt
  shared library wrapped as an IMPORTED target before Jelly is added. Three targets in
  the shape [the three pillars](/design/the-three-pillars.md) §4 leans toward:
  `squid_app` (the application declared once, no window system), `squid` (the desktop
  host, the only target that includes a GL header), `squid_headless_smoke`.
- **`src/app.cpp`**: the `maiz::HostApp`. Effects `open` (the one file read; then Jelly's
  `read_ink` and `emit_gl`), `residual`, `inspect`. The scene is held as **derived** data
  keyed by the `.ink` hash; the document holds only `squid.ink` and `view.camera3d` as
  config. Dropping the held scene and re-opening the path rebuilds it, which the smoke
  test checks: Void Squid owns no truth.
- **`src/viewport.cpp`**: fixed-function GL 1.1 over Maiz's 3.0 compatibility context,
  client-side arrays straight from Jelly's buffers, one light, orbit/pan/zoom. Row-major
  to column-major once per draw. `frame_all` reads vertices to aim a camera and that is
  the only place this repository reads one.
- **`src/main.cpp`**: one binary. No arguments: the window. A `.ink` argument: the window
  with it open. Anything else: `maiz::run_cli`. The GUI is a `Human` session so a person
  outranks an agent on the same document.
- **`tests/headless_smoke.cpp`**: refused by default; allowed opens, emits and reports the
  verdict; the document remembers path and camera across sessions; the scene is not in
  the document; a bad path is a sentence. Passes.

### Measured

| claim | reading |
|---|---|
| the boundary is workable (scope claim 1) | three calls (`read_ink`, `emit_gl`, draw); nothing asked Jelly for a window, a UI concept or a gameplay noun |
| the GL residual is nearly empty (claim 2) | fits, 0 dropped; 18 entries all caused by profile lines (triangulated ×3, derived ×4, defaulted ×8, omitted ×3) |
| a residual is presentable (claim 3) | grouped by kind with counts, drill-down per subject; readable at 18, unknown at thousands |
| emission is cheap enough for a loop (claim 4) | **not measured**; the sample is five objects and nothing edits a scene from inside Squid yet |
| the CLI costs ~40 lines | `main.cpp`'s CLI branch is one line; the HostApp is the cost, and the GUI pays it too |

### Found, and what was done about each

1. **The MinGW runtime.** The binary loaded a different toolchain's `libstdc++-6.dll`
   from the PATH and died before `main` (exit `0xc0000139`). The runtime is now linked
   statically into every executable, the decision Jelly already made for its DLL.
2. **Two Cores in one process.** Maiz links the prebuilt Core by file; Jelly builds Core
   from source. Settled with an IMPORTED target here and a `TARGET` guard in Jelly.
3. **The effect handler's `op`.** For `effect open x.ink`, Core 0.2.14 hands the handler
   `op == "open"`, not `op == "effect"` with `open` in the arguments, which is the other
   reading of Maiz's `headless.hpp` comment. The handler accepts both; reported to Maiz.
4. **Dispatching past the Session.** The first GUI cut called `Core::dispatch` directly and
   replaced the Session's log sink, so the effect gate, journal and lock saw nothing: the
   journal's silence was the finding. Fixed: every command goes through
   `Session::dispatch`, the panel reads `on_command`. Reported to Maiz as a seam that is
   easy to get wrong.
5. **A 3D camera does not fit Maiz's `Camera`.** Three floats cannot carry an orbit; six
   numbers under `view.camera3d` with this host's own serialise/parse. The viewport page
   predicted this message on founding day; it is now sent.
6. **CTest sees the siblings' trees.** `add_subdirectory` registers Maiz's and Jelly's tests
   (and a name collision on `headless_smoke`). Ours are prefixed `squid_`; run
   `ctest --test-dir build -R squid_`. Reported to Maiz as a build-option suggestion.
7. **Jelly's sample cube had two inward faces.** With culling on, they disappeared from
   behind. Fixed in Jelly with a test that fails on the old cube; and the viewport now
   draws without culling and with two-sided lighting, because a viewer meant to show
   losses must not silently hide a face an artist wound backwards.
8. **A remembered camera with stray quotes re-framed silently.** The parser now strips
   quotes of either kind; a camera survives a restart (checked by screenshot from the
   back-left side).
9. **Jelly's §10 was not decided** and did not need to be: the pure half is needed under
   either answer. The uploader lives here, cannot run on macOS, and is the measured
   argument for Jelly's lean (b) when a second host appears.

### What did not change

Nothing about geometry is computed here (one read of vertices, to aim a camera). `.ink`
is read through Jelly. The game model is design only. v0's exit condition is rung 3.

### Trued up

[Viewport](/concepts/viewport.md) → `status:current`. [Scope](/concepts/scope.md),
[build surface](/concepts/build-surface.md), [roadmap](/roadmap.md) (rung 2.5 status),
`index.md`, `README.md`, `CLAUDE.md`, `void.json` (version source, provides, status).
Message sent: `../VoidMaiz/MESSAGE_FOR_VOIDMAIZ_voidsquid-first-3d-host-findings-2026-10-09.md`.

---

## 2026-10-09 — The foundations: three pillars, what a game is, construct and deconstruct

The author reframed Void Squid, the same day Latin OS reframed itself as an artifact this
forge emits. *"This was not Void Squid's original conception, however, this is what it's
evolving into."* Three pillars, five beliefs about what a game is, a requirement to
deconstruct as well as construct, and a request that the philosophical and
epistemological research go into the OKF. The author also said Void Jelly is not complete
and the demo is not built, and asked for the overhaul anyway. **Nothing was built. v0 did
not move.**

### What was worked out

- **[The three pillars](/design/the-three-pillars.md).** The game model (pure), the
  studio (projections), the hosts (the effect list). They separate along seams Void Maiz
  already has (`project` / `compile`, the effect handler). Packaging lean: three build
  targets in this repository with a mechanical boundary check, promoted to siblings per
  pillar when a second consumer appears (S7). A finding: the founding "no code before
  rung 2.5" rule binds the studio and the desktop host, and does **not** bind the game
  model, which opens nothing; but the railguard's question still does, so the game model
  is designed now and built on the author's ruling or at S6 (S14).
- **[What a game is](/concepts/what-a-game-is.md).** The author's five beliefs as a
  vocabulary over Void Core: world (a mantle), player (outside, reached through
  holidays), avatar, character with stats, region, rule, beat, cue, upgrade. **The
  Dungeon Master taken apart into three jobs** that land on three owners: running the
  rules (the runtime), adjudicating gaps (Core rules and Allomone), generating (a model
  behind a holiday, Bicho). The upgrade graph is where the game's thesis is written and is
  authored structure. Feedback is part of the avatar concept. The living world's four
  parts each have a known shape. A modality table with two honest gaps (text and
  narrative; image and video). The same nouns read as an operating system (§8). Three
  tests for anything proposed (§9).
- **[The epistemology of the game](/design/epistemology-of-the-game.md).** Four positions
  (no definition; a stance; rules plus fiction; meaning through rules), what the engine
  takes from each, and why the Dungeon Master is the hinge between forge and artifact.
  Deconstruction read as an epistemic method for testing the vocabulary.
- **[Sources](/sources/index.md).** Seventeen pages in the family's one-page-per-work
  convention. **Sixteen are `confidence:asserted`** (written from recall, unverified) and
  say so; one, [REA](/sources/rea-reverse-engineer-anything.md), is `verified` against
  its README today. The verification queue is the index.
- **[Construct and deconstruct](/concepts/construct-and-deconstruct.md).** A reader is
  the inverse of a nib: `read(artifact) → (document, residual)`. REA is a Node
  application that drives Ghidra and others and reports evidence and limitations; that
  makes it a **process holiday**, the ingress twin of devkitARM, not a library to vendor
  (S10). Design rule: every noun in the game model must be emittable and recoverable.
- **[Studio surfaces](/concepts/studio-surfaces.md)** and **[hosts](/concepts/hosts.md).**
  *A scene is any mantle with a projection; a game is any artifact running under
  observation.* Play mode is not a mode. On Latin OS Void windows never send pixels, so
  no surface may be pixels, and the desktop host must be held to that from its first line.
  The Windows and LITO hosts differ in the effect list and nowhere else.

### Trued up

[Railguard](/design/what-voidsquid-is-not.md): §1 dated, **§6 restated at Latin OS's
request** as *the artifact never ships the forge* with the drift test kept in the
wording (original preserved beneath), **§8 added** (the game model's own three-question
test). [Open questions](/design/open-questions.md): S1 and S6 annotated, **S7–S14
added**. [Roadmap](/roadmap.md): rungs 7–10 added after v0, unscheduled; the
not-on-this-roadmap list scoped to v0. [Build surface](/concepts/build-surface.md): two
after-v0 rows (ingress toolchain, hypervisor). [Scope](/concepts/scope.md),
[engine horizon](/design/engine-horizon.md), [the forge for many runtimes](/design/the-forge-for-many-runtimes.md)
(*systems* as the horizon after runtimes; the ECS projection sentence): dated notes.
`index.md`, `app.md`, `README.md`, `CLAUDE.md` rewritten to match.

### The inbox, consumed

- **Void Mago (2026-10-04).** `void.json` written: `application`, `0.0.1`,
  `version_sources: []`, requires Jelly (static), Core (shared via Maiz), Maiz (static),
  Allomone (static via Maiz). `mago doctor`: **zero errors, zero warnings on our line**;
  one note (Jelly described twice, as `requires` and `interacts_with`), fixed by
  describing the seam once on the edge. `mago blast voidcore`: Squid under RESHIP;
  `mago blast voidjelly`: Squid under REBUILD. Both correct. Three findings about other
  manifests (Latin OS's `link: "c-abi"` fails the enum; Core and Palabra name their seam
  differently) reported to Mago, not fixed from here.
- **Void Jelly (2026-10-05).** Tier 1 of the desktop testing ladder recognised as in
  scope; it is the first instance of the scene view over any mantle with a projection.
- **Latin OS (2026-10-09).** Both asks accepted as leans (S8); the OS-as-game mapping
  written into [what a game is](/concepts/what-a-game-is.md) §8 and the LITO host into
  [hosts](/concepts/hosts.md).

### Messages sent

Replies to Latin OS, Void Jelly and Void Mago in their roots, listed in `index.md`. Jelly
gets two questions (its Q19 against a game's terrain edits; rune-id stability across a
content reference). Latin OS gets one (whether its Q11 and our S12 share a name).

### What did not change

The harness. Rung 2.5 waits for Jelly's GL nib; rung 3 is still the exit condition;
Squid computes nothing about geometry, owns no `.ink`, owns no truth, ships no forge to
any target. The author decides S7–S14, and S14 (when the game model is built) is the one
that most wants a ruling.

---

## 2026-10-03 — The direction: a forge for many runtimes

The author brought the Rust/Bevy game-rewrite scene (iw4L, a from-scratch MW2 runtime; a
mashup of MW2, Skate 3 and Minecraft in one executable) and set a direction:

> *"eventually [Void Squid] will be the engine to tie everything together, combine it all
> into 1 thing, and be the ultimate generalizable game creation tool."*

### What was worked out

[Engine horizon](/design/engine-horizon.md)'s founding claim, *Void Core's contribution to
a game engine is the editor, not the runtime*, turns out to be exactly the gap in that
ecosystem. The community is building runtimes and has no forge: Bevy 0.19 has no editor,
and every rewrite invents its own asset IR. So the author's "ultimate tool" reads as **one
forge that pours into many runtimes**: gather, compose, see residuals, build, and live-edit
a running Bevy game over the **Bevy Remote Protocol**, which is a holiday. The full page is
[the forge for many runtimes](/design/the-forge-for-many-runtimes.md). It includes how each
sibling the author named would plug in (Palabra, Allomone, Verguenza, Chisme, Loops Studio).

### What was ruled out, as leans

- **No Rust half in Squid.** It reaches Rust through files and BRP. A Bevy-side plugin
  belongs to a Bevy host (`VoidJelly:/design/open-questions.md` §18).
- **Squid composes structure, never mechanics.** Mechanics compose in ECS at frame rate,
  inside the runtime.

### What did not change

**v0 is still a harness.** New [S6](/design/open-questions.md) records the lean for when the
charter widens: when one `.ink` has been emitted to two targets with two correct, different
residuals. The ecosystem moving fast is an argument *for* proving the pipeline first.

Written: [the forge for many runtimes](/design/the-forge-for-many-runtimes.md). Trued up:
[engine horizon](/design/engine-horizon.md), [open questions](/design/open-questions.md)
(S6), `index.md`.

---

## 2026-08-26 — Founded

Void Squid founded by the author on the same day the 3D library was renamed **Void Jelly**
and this name moved here. **The OKF comes before the code, as always** — no code exists,
and every page is `status:planned`.

### The rename that produced this bundle

| was | is |
|---|---|
| Void Squid (the renderer/library, founded 2026-08-25) | **Void Jelly**, `../VoidJelly` |
| *(did not exist)* | **Void Squid** — this bundle, a minimal game engine |

Void Jelly's [log](../../VoidJelly/okf/log.md) carries the full record.

### Why the client was founded before the library has code

The author's reasoning, and it is correct:

> *"the reason i think we need the game engine NOW, super early, is purely because the game
> engine will be the thing to actually UTILIZE Void Jelly to create the 'export'."*

It follows from a property stated in the same breath: **Void Jelly is a library like
Palabra, not an application — it cannot render or create windows by itself.** A library in
that shape *cannot demonstrate itself*. It can emit the bytes of a `.nds` and cannot write
them; it can emit a draw list and cannot draw it. Founded without a host it would be
**unfalsifiable** — every design decision would look fine, because nothing was pulling on
it.

The family has evidence in both directions. Void Maiz organises its phases around
**exit-test clients**; Void Palabra chose **forcing clients** deliberately, recording *"what
each proves and what each blocks on."* And Void Maiz's docking bug is the counter-case:
three code paths were dead for weeks because *"a checked-in layout file is a first-run test
nobody is running"* — found only when a client with no inherited config appeared.

### What was decided at founding

1. **v0 is a conformance harness that happens to look like an editor**
   ([scope](/concepts/scope.md)). Not a hedge — a scope. Every proposed feature answers one
   question: *does this exercise a Void Jelly seam that is not otherwise exercised?*

2. **Void Squid owns every effect; Void Jelly owns every computation.** *The transformation
   is Jelly's; the transaction is Squid's.* The list of holidays Void Squid implements is
   [build surface](/concepts/build-surface.md), and the mechanical rule for anything new is:
   *could it run in a unit test with no filesystem, network or display?* Yes → Jelly. No →
   here.

3. **A Void Maiz application, deliberately** — which brings three things that would
   otherwise be work: the model lives in Void Core so there is no second data model; every
   gesture is a visible command so humans, scripts and agents share one surface; and
   **headless costs ~40 lines, not a port**. That last one matters most: a regression suite
   over residuals wants a command line, not a window.

4. **`--dry-run-effects` is a gift from the boundary, not a nicety.** Because Void Jelly is
   pure, the entire pipeline runs and produces a complete residual **without writing
   anything or needing devkitARM installed** — so *"will this fit on a DS?"* is answerable
   in CI on a bare machine.

5. **The viewport is a control experiment, not a feature.** The GL residual should be nearly
   empty; if the picture is wrong, the loss is in the `.ink` and not in any target. It is
   also the most-exercised emitter in the system by two orders of magnitude, since it runs
   every frame where the DS and voxel nibs run on request.

6. **The engine part waits.** [engine horizon](/design/engine-horizon.md) records the
   direction and the three seams v0 must not foreclose (non-visual runes, cheap emission, a
   symbol table in the blot) — each of which v0 wants anyway, which is the test that they
   are not speculative work.

### The trap, named at founding

The pull will not be *"abandon the harness and build an engine."* It will be *"while we are
here, this would be easy."* A physics step is a weekend; an input map is an afternoon. Each
is small, and together they are how Void Jelly ends up with one host that is a game engine
and a shape that fits nothing else.

Two questions guard against it, both already written down:
[railguard](/design/what-voidsquid-is-not.md) §7 — *am I building this because Void Jelly
needs it proved, or because it would be fun?* — and, from Jelly's side, *would a Blender
in-viewport preview want this too?*

### Written at founding

`index.md`, `app.md`, [roadmap](/roadmap.md) (interleaved with Void Jelly's), this log;
concepts [scope](/concepts/scope.md), [viewport](/concepts/viewport.md),
[build surface](/concepts/build-surface.md); design
[what Void Squid is NOT](/design/what-voidsquid-is-not.md),
[engine horizon](/design/engine-horizon.md), [open questions](/design/open-questions.md);
plus `README.md` and `CLAUDE.md`.

**Everything `status:planned`.** Void Squid should not exist as *code* until Void Jelly's
rung 2.5 — its first three rungs need no host, and starting earlier would mean an
application with nothing to open.
