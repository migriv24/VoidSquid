---
type: Design
title: The DS mashup exercise — three cartridges taken apart and played as one
description: "2026-10-10: Super Mario 64 DS, Mario Kart DS and Mario Party DS, owned by the author, taken apart by an agent with universal-modder's playbooks, apicula, Void Squid's recon and converters written on the spot, then reassembled as a PC program: Super Mario 64 movement on a Mario Party board with its recovered space graph, and a Mario Kart DS race on recovered collision and checkpoints. What the exercise found about the nature of games, the answers to the author's five questions (touch screen, the many Marios, gameplay on another game's map, naming, a file type), and what Void Squid needs next."
resource: ../ds-mashup/src/game.cpp
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-10T00:00:00Z
---

> The author, 2026-10-10: *"lets look into the games here … and let's attempt to make a PC
> version of them … this decompilation project is mostly for gaining insights of the nature
> of games in general and what they mean."* And the end condition: *"a desktop application
> that i can run that is this meaningful 'mashup' of all the games (crucially, we are missing
> a 'narrative' … This will eventually come from the actual game engine and human
> interaction)."*
>
> **Built, and outside this repository**, because a game runtime is an artifact, not the
> forge (railguard §6), and because its converters loop over vertices (rule 4):
> `../ds-mashup/` holds the converters and the program. The game data lives only in a
> workbench beside the author's ROMs. `confidence:verified` covers what scripted runs and
> screenshots showed (§7). Nobody has played it with their hands yet.

# 1. What was built

| piece | from | depth reached | how |
|---|---|---|---|
| Wiggler's Garden board, 2,058 triangles, 46 textures | Mario Party DS, `bmap_01` entry 019 | bytes | **HBDF, reverse-engineered here**: no published spec was used |
| its 58-space graph with branches | `bmap_01` entry 003 | structure | found by shape; kinds counted, not named |
| Mario, body and capped head, 16 bones | Super Mario 64 DS, `mario_model.bmd` + `mario_head_cap.bmd` on bone `face_1` | bytes | its own BMD format, decoded here |
| Bob-omb Battlefield | `bombhei_map_all.bmd` | bytes | same decoder |
| Figure-8 Circuit, Mario and Luigi karts, item box | Mario Kart DS, Nitro formats | bytes | apicula, invoked as a toolchain |
| the course's collision with surface types | `course_collision.kcl` | structure | decoded here; road, off-road and walls read from the data |
| start, 26 checkpoints, 37-point AI path, item-box placements | `course_map.nkm` | structure | decoded here |
| Mario's movement | the N64 game's public decompilation (constants) | structure, by reimplementation | **a sibling's source standing in for opaque DS code** |
| kart handling, dice, coins, laps | nobody's | — | written for the mashup, and labelled as such on screen |

The program has three modes (board, kart, platform), a touch-screen panel driven by the
mouse, and an inspector (Tab) that shows each visible piece's source file, depth and
residual. One command, `tools/build_shelf.py`, rebuilds every piece from the ROMs and
writes `shelf.json` (§3, question 5).

# 2. What the exercise found about games

**1. The bytes depth is per developer, not per platform.** One console gave three model
formats: Nintendo's Nitro SDK (Mario Kart), Super Mario 64 DS's own older format, and
Hudson's chunked HBDF (Mario Party). All three end in the same DS geometry-engine command
lists, so one 130-line decoder serves every game. **Readers form a tree:** hardware (command
lists, texture formats, LZ, NitroFS) is shared, SDK formats are shared by many games, and
studio formats by a studio's games. A reader for "a DS game" is mostly the trunk.

**2. Structure lives in data more often than expected; behaviour lives in code.** The board's
graph, the course's checkpoints, start, AI path and surface types, and the skeleton's bone
names all came out of data files without touching any code. What a space *does*, how a
kart *handles*, how Mario *moves* and every one of Mario Party's minigames (108 overlays)
are code, so they stay at the opaque depth. The game model's split between structure
(data) and rules (code compiled to tables) is how these games were already built.

**3. A game's best oracle can be its ancestor.** Super Mario 64 DS's movement code was not
read. The N64 game's matching decompilation is public and complete, so its constants stand
in: run 32 units/frame, gravity 4, jumps 42/52/69. That's family resemblance as a
decompilation route: a franchise is a lineage, and Mario Kart DS ships remade Mario Kart 64
and SNES courses (`old_choco_64`, `old_donut_sfc`) as data.

**4. Every game has its own units, and the mashup is mostly calibration.** Three pieces
needed three calibrations, each with its evidence. The board was scaled from Mario Party's
own Mario (16.7 units tall = 1.6 m). The level from Bob-omb Battlefield's width in Mario
heights. The race from a kart's length, with the collision at 1/16 of the render model,
proved by the floor heights agreeing exactly. The space graph sat at half the board's scale,
found only by drawing it over the board from above. **Units and their evidence are part of
the document**, which is the bridge contract's "units and axes" field (`decompilation.md` §4).

**5. Scale is narrative.** Mario stands among leaves six times his height on the board,
because Mario Party DS's story shrinks the players with Bowser's Minimizer, and Wiggler's
Garden is a garden at insect scale. Calibrating from the game's own character kept that
story; normalising the board to a human scale would have erased it. **A mashup that
normalises units can delete meaning.**

**6. The camera belongs to the mode, not to the character.** Super Mario 64's chase camera
on a Mario Party board hid Mario behind leaves. Mario Party looks down on its boards from
high above. The fix was a per-mode camera, so **the viewpoint noun (`decompilation.md` §5)
belongs to the table, not to the avatar.**

**7. Two rule systems can own one body in turns.** On the board Mario walks under Super
Mario 64's rules until the dice are rolled; then the board owns his position and he hops
space to space; then control comes back. That is exactly the mashup community's ownership
table with a hand-back trigger (S21): who owns the player is a rule with a condition, not
a property of the character.

**8. A cartridge is already a mashup.** Mario Party DS holds a board game and dozens of
minigames with their own physics and controls. Mario Kart DS holds racing, battle and
mission modes. A "game" is a set of tables with transitions, which is
[Turkey, Table, Feast](/concepts/turkey-table-feast.md) found in the wild: pieces are
turkeys, play styles are tables, and the way one leads to another is the feast. The
mashup did the same at the next level up.

**9. Universal-modder, as is, covers PC games and not DS ones.** `um scan` fingerprints
DeSmuME as "unknown native engine" and cannot see the games. `um kb search` finds nothing
for Mario Kart DS, Mario Party DS or "nintendo ds". `um kb check` and `um publish check`
worked unchanged and were useful: the field note passes, and the project has 0 failures
against the ROM folder. Recon (`effect recon`) verified all three cartridge headers by
checksum, its first run on real dumps.

# 3. The author's five questions

**1. The touch screen.** The bottom screen is a second view whose role differs per game:
camera buttons and a map in Super Mario 64 DS, a course map in Mario Kart DS, dice and
minigames in Mario Party DS. On a PC it became a panel beside the 3D view, with the mouse
as the stylus. Every touch action also has a key, because in a Void Maiz application every
gesture is a command. Stylus-native mechanics (drawing, rubbing, tapping targets) map onto
mouse drags well, since the DS had one touch point. **In Void terms the touch screen is an
input holiday plus a second projection;** what it *shows* in each mode is a studio-surface
question, not a hardware one.

**2. The many Marios.** There are three: `mario_model` + head (Super Mario 64 DS),
`E_MR` in a kart (Mario Kart DS) and `p01_mario` (Mario Party DS). The shelf holds **one
identity with presentations keyed by role** (`platform`, `kart`, `board`), plus each game's
internal name for it. Nothing is duplicated, because a mode asks the identity for a
presentation rather than loading "a Mario". Super Mario 64 DS already did this
internally: Mario, Luigi, Wario and Yoshi share one skeleton and animation set, and only a
few animations and the head models differ. Luigi is the rival in the race, so the race
never shows two Marios. Mismatches are recorded, not hidden: Mario Party's Mario is on
the shelf with wrong colours and marked as such.

**3. Super Mario 64 DS gameplay on a Mario Party board.** Done, and it took four things:
calibration (§2.4), collision from the board's render mesh (Mario Party has no collision
file, since its characters never walk freely), a camera change (§2.6), and an ownership rule
between free movement and the dice (§2.7). The board's own graph became an optional layer
over free roaming. What did not carry over is the meaning of each space, because that is
code.

**4. Naming.** The games' internal vocabulary is Japanese romaji, and it collides with
English: `koopa` is Bowser while `nokonoko` is Koopa Troopa, `heyho` is Shy Guy, `teresa`
Boo, `killer` Bullet Bill, `kinopio` Toad. Texture names describe their own use (`kabe`
wall, `ashiba` footing, `taki` waterfall, `YazirusiBunki` branch arrow). Three layers are
needed: **internal** (per game), **canonical** (the identity, `mario`) and **display** (the
player's language). The shelf holds a glossary. The games' own message tables (Mario Kart
DS's MESG files) would give the display layer from the source and were not read.

**5. A new file type.** The exercise answered it by needing one, and it is not a binary
format. Geometry is fine as glTF, or as `.ink` through Void Jelly. What was missing was a
document that says, for every piece, **which game, which file inside the cartridge, how
deep it was recovered, by what oracle, what was not recovered, and in what units**. It also
says who the characters are, across presentations, and what the words mean. `shelf.json`
is that document, about 200 lines for 11 pieces. **It is the game document (S12) in its
decompile-workspace form**, and every field maps onto a rune: a piece is a rune with
content-addressed blocks, depth and oracle are tags, the residual is content, identities
are runes with `presents-as` links. S22 asks the author to adopt that mapping.

# 4. What Void Squid needs next, in order of what the exercise forced

1. **The shelf as a mantle.** `shelf.json` translated one to one into runes, opened in the
   studio, so the inspector the mashup grew by necessity becomes a projection (S22).
2. **A DS reader trunk owned by the right sibling.** Command-list and texture decoding are
   geometry, so Void Jelly's (rule 4). They are also the inverse of Jelly's coming DS nib,
   and a lens test is possible: emit, decode, compare. Sent to Jelly as a message.
3. **Units as data with evidence**, on every piece and every link between two games'
   pieces (S21's contract).
4. **The ingress toolchain row for real.** apicula was invoked as a toolchain here from
   Python; Squid's `effect` gate should do it (rung 9b).
5. **`classify` at the file level first.** The identifying work in this exercise was
   file-level: which entry is the board, which holds the space table, which is Mario. It
   was done by chunk census, size and names, never by reading a function. Bicho's
   `classify` kind should start there.
6. **Narrative, which nothing recovered.** No piece carries why. The program says so on
   screen, and the author named where it comes from: the engine and people.

# 5. Residual of the exercise

Space kinds named only by guess (most common kind = blue, by frequency). Mario Party DS's
character colours are wrong (material binding off by one) and its minigames are untouched.
No animations (BCA, ANMF) are converted, and there is no sound (SDAT). Super Mario 64 DS's
KCL variant is not decoded, so its level's ground is the render mesh. Level objects and
enemies live in overlays and were not placed. Kart speed is calibrated to a typical lap
time (about 39 s), not recovered. Nobody has played it with a controller yet.

# 6. Where the work is

| what | where |
|---|---|
| program, converters, field note | `../ds-mashup/` (README, `fieldnotes/`) |
| the person's pieces | `%USERPROFILE%\Documents\games\ds-mashup-workbench\shelf\` (never shared) |
| launcher | `../ds-mashup/run.cmd` |

# 7. How it was checked

`ds_viewer` rendered every converted model from three angles, compared with the games by
eye. `ds_mashup --autotest` played a scripted session through all three modes and saved
seven screenshots: the dice roll and a branch landing, the race reaching the far loop at
about 80 km/h with Luigi alongside, and Mario in Bob-omb Battlefield's trench. `ds_mashup
--race-test` drives three laps on the course's own AI path: every checkpoint in order, laps at
39, 78 and 117 s, a win over Luigi paid to the board as coins and a star. `um publish
check . --game <ROMs>` reported 0 failures, and `um kb check` passed the field note.
