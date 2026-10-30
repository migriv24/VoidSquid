---
type: Design
title: Void Squid 0.1 — the studio, a real application on Void Maiz
description: "2026-10-12. The author asked for the first real version of Void Squid: an application to launch from the desktop, on Void Maiz properly (the menus, the windows, the widget registry), with a project file that opens the asset database, Turkey, Table and Feast as workspaces, Hormiga's look and Data tab, and the stars. What was built, how the library becomes Void Core mantles, what Void Maiz supplied and what the host still had to write, the rules it keeps, and what is still outside it."
resource: ../src/studio/studio.cpp
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-12T00:00:00Z
---

> The author: *"right now let's actually create something, a real application i can launch
> with void maiz right now"*; *"we're gonna want to more properly be using void maiz for the
> file, edit, etc ribon at the top, diffferent windows, and its real widget registry"*; *"the
> database we have right now, we should be able to load it up and find it. similar to how
> void hormiga has a .miga file"*; *"we can copy a LOT of the design stuff from hormiga.
> Windows and tabs. Also the data tab"*; *"i do like the stars in the background though"*;
> and *"i would want a desktop shortcut for void squid for me to test."*
>
> **Built** (`src/studio/`, `src/library/`, `src/host/`, `src/effects.cpp`), tested
> (`tests/library_test.cpp`, and a screenshot walk of every workspace on the four-game
> library), and on the author's desktop as *Void Squid*.

# 1. What it is

A docked window in the shape Void Hormiga proved (`VoidHormiga:/src/app/app.cpp`, its
`frame()`): a **menu bar** (File, Edit, View, Windows, Help), a row of **workspaces**, a
**dockspace** whose first layout puts the Shelf left, the Inspector right, the Console
below and the workspaces in the middle, and **toasts** for what just happened.

| workspace | job | file |
|---|---|---|
| Start | open a library, load a game, open a project, see this machine | `panels.cpp` |
| Turkey | inspect one thing: tags coloured by source, kind, checks, sounds, every field | `turkey.cpp` |
| Table | the tables in feast order, their music, a map from above, Play, the 3D table | `table.cpp` |
| Feast | the player (drop a turkey), speeds, jump, gravity, lives; write for the runtime | `table.cpp` |
| Data | Hormiga's Data section: mantles, a table of runes, the rune's form, tags, links | `data.cpp` |
| Scene | rung 2.5's harness: an `.ink`, Jelly's draw list, the residual, the look | `panels.cpp` |

Always-available windows: **Shelf** (the library, filtered by Void Maiz's one tag grammar
or plain words), **Inspector** (whatever was selected, through the widget registry),
**Console** (the transcript and a command line with history), **Load a game**, **Setup**.

**The look.** Hormiga's spacing and rounding, Lato with Font Awesome merged in, JetBrains
Mono in the Console. A deep-sea palette (navy, a cyan and a violet accent) on panels the
stars show through, and the starfield the author liked in the turkey table: a
deterministic field that twinkles, two faint nebulae, drawn on ImGui's background list.

# 2. The project: a `.squid` file is a Void Core state document

The author remembered the project file as `.ink`. **It is not**: `.ink` is Void Jelly's
scene format, and Void Squid never adds to it (CLAUDE.md rule 7). S12 already said the
game document is a separate thing whose **name is the author's**. Void Squid 0.1 uses
**`.squid`**, provisionally, in one place (`platform.cpp`'s filters and the default name),
so a rename is cheap.

A `.squid` is exactly what a `maiz::Session` reads and writes: Void Core's state document.
Beside it are the Session's journal (`.squid.log`, every command, who and when) and its
advisory lock (`.squid.lock`). Void Hormiga's `.miga` bundles its assets inside; a
`.squid` does not, because a library is game data and is never copied into anything a
person might share (§4).

| | `.miga` (Hormiga) | `.squid` (Squid) |
|---|---|---|
| holds | the whole state, plus every asset, base64 | the whole state; library files by path |
| why | an organisation's photos are irreplaceable and theirs to move | a library is rebuilt from a game the person owns, never shipped |

# 3. The library becomes mantles

`effect library import <folder>` reads `assets.db` (SQLite, read-only), `worlds.json` and
`feast.json`, and answers six mantles: **turkeys, games, kinds, tracks, tables, feast**,
one rune each, glyphs `turkey`, `game`, `kind`, `track`, `table`, `feast`. The front-end
then **splices** them into the project (`library::splice`, pure) and swaps the state in
with `Core::replace_state`.

**Why a splice and not a batch of commands.** Void Core measured 2,000 sequential `rune
new` calls at 9.9 to 16.4 seconds (SPEC §6.2), and a `set` costs time in proportion to
the document. The library is 235 turkeys with a dozen fields each: tens of seconds by
commands. The splice took **0.8 seconds** end to end. The cost is the undo history at
that point, the same trade Hormiga makes when it opens a database; the import itself is
one journalled effect line.

**What a re-import keeps** (pinned by `library_test.cpp`): every tag whose source is
`person`, every tag a person removed (`tags_removed`), every `sound_*` field, a kind a
person chose (`kind_source: person`), notes, placements, runes a person made
(`made_by: person`), the feast, and every mantle that is not a library mantle. **Rune ids
derive from (mantle, name)**, so two people's imports of the same library give the same
identities, which is what lets Void Maiz's merge line them up.

Tags keep their source exactly as the toolbox's database had it, in a `tag_sources`
object, and the Turkey workspace colours each chip by it: game, wiki, name, vision, you.

# 4. What Void Maiz supplied, and what the host wrote

**From Maiz, used as designed:** the `Session` (lock, journal, the person outranks an
agent, effects allowed in the window and declared for the briefing); `project_scene` and
`field_of`; `widget_form` and `WidgetRegistry::defaults()` plus `add_path` with the host's
dialog, so each glyph's `hints.editors` and `hints.labels` pick the editors on every form
(Turkey's every-field panel, Data, the Inspector, the Feast's sliders); `node_matches` for
the Shelf and Data filters; ImGui docking and `gl_context_hints`.

**Written by the host**, because Maiz's widget-registry page draws that line ("application-
level UI: menus, toolbars, status bars" are the host's): the menu bar, the workspace bar,
the dock layout, toasts, the busy overlay, the console. **Void Hormiga wrote the same
shell first.** Two hosts with one shell is the evidence Maiz asks for before it takes
something in, so it is reported upstream
(`../VoidMaiz/MESSAGE_FOR_VOIDMAIZ_voidsquid-two-hosts-one-shell-2026-10-12.md`) and S28
asks the question here.

# 5. The rules it keeps

- **No truth in the window.** Every change is a command through the Session; views
  re-project after it. The tag editor is three commands in one batch (the tag, its
  source, a removal record), one undo.
- **No geometry.** The studio draws thumbnails the toolbox rendered, a map of positions
  read from the world file, and Jelly's draw list. There is no loop over a mesh anywhere
  in `src/`. Placing turkeys in 3D therefore stays in the exercise's 3D table
  (`effect table3d` starts it) until Void Jelly can read a glTF, asked for in
  `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-the-studio-needs-to-see-a-turkey-2026-10-12.md`.
- **Every effect declared** (`effects.cpp`): `library`, `setup`, `loadgame`, `catalogue`,
  `play`, `table3d`, `reveal`, each with its consequence for `squid --describe`. On the
  command line each is refused unless granted.
- **Two executables, one main.** `squid` is the console program (the CLI, and the window
  with no command); `voidsquid` is the windowed build for the shortcut, so no console
  window opens. The window asks Windows for the discrete GPU (the Optimus and PowerXpress
  exports from the Goomba Galaxy exercise) and reports the renderer in Setup.

# 6. Measured

| what | result |
|---|---|
| tests | `squid_headless_smoke`, `squid_sim`, `squid_read`, `squid_library`: 4 of 4 pass |
| library import (CLI, `--import-library`) | 235 turkeys, 235 thumbnails, 5 games, 12 kinds, 74 tracks, 7 tables, in 0.8 s |
| setup inventory | 0.4 s; on this machine: Python 3.14, Ollama with three models, Void Bicho, Claude Code (inside the editor extension, not on PATH), the RTX A1000 |
| the screenshot walk (`SQUID_SHOTS`) | every workspace drawn on the four-game library, on the RTX A1000, from the shortcut's own binary |

# 7. Residual

- **No 3D table and no audio in Void Squid itself.** Both are the exercise's runtime,
  started as a process. Play writes the feast and starts it.
- **The runtime is the exercise's** (`goomba_galaxy2`), found beside the binary, through
  `VOIDSQUID_RUNTIME`, or in `../ds-mashup`. A release has no runtime until S30 is
  answered.
- **The ingest routes are the exercise's toolbox.** Load a game names them; it does not
  run them yet.
- **Collaborative editing is not switched on.** The project is ready for it (§3); turning
  it on is Void Maiz's networking.
- **The local model is detected, not used.** See [Load a game](/design/load-a-game.md).
- **Linux is in the code, not tested**: the dialogs (zenity, kdialog, yad, qarma), the
  process launcher (posix_spawn and a double fork), the settings folder. No Linux
  machine has run it.
- **The catalogue glyphs are registered by the host.** They describe the asset library,
  not the game model, which stays design-only (S9, S14).
- **A crash leaves a lock.** The lock file names no process, so the window asks the
  person: "Open it here anyway".
