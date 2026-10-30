---
type: Design
title: Goomba Galaxy — a decompiled level, played as someone else and changed by meaning
description: "2026-10-11: the author asked for Good Egg Galaxy with its music and gameplay, played as a Goomba with Galaxy's physics, plus requests that test understanding: planets to orbit, Mario Kart DS item boxes from defeated enemies, a pause menu that loads Super Mario 64 DS's castle grounds, Galaxy's sound effects, more trees, a water planet and a penguin. What came from the disc, what was chosen by meaning, what was written by hand, and what it measured about how much of a game's physics is data."
resource: ../ds-mashup/src/galaxy.cpp
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-11T00:00:00Z
---

> The author: *"let's try creating something with the tools we have."* Good Egg Galaxy with
> all its music and gameplay, desktop controls, and: play as a Goomba under Galaxy's
> physics; a couple of planets around the level *"to see if we can test if we can easily
> orbit them (how much of the physics within the game itself can stand on its own?)"*; a
> Mario Kart DS item box when an enemy dies, with its sound; an Escape menu that loads
> Super Mario 64 DS's castle grounds *"as another world"*; all of Galaxy's sound effects;
> *"more trees … to see if we have the tools to understand this request"*; a water planet
> from somewhere in the game; a penguin at the start.
>
> **Superseded in part, 2026-10-11:** [Goomba Galaxy 2 and the turkey table](/design/goomba-galaxy-2-and-the-turkey-table.md)
> rebuilt the level on Jolt with the game's own lights and cameras, gave the castle its own
> collision, and added the turkey table. This page stays as the first round's record.
>
> **Built and running.** `../ds-mashup/build/bin/goomba_galaxy.exe`, launched by
> `../ds-mashup/goomba_galaxy.cmd`. Everything it loads is rebuilt from the author's own
> discs by `../ds-mashup/toolbox/build_goomba_world.py` into
> `Documents/games/smg-workbench/goomba_world`. No game data is in any repository.

# 1. Where each piece came from

Three origins, kept apart because Void Squid's claims depend on which is which. The game's
F1 panel says the same thing on screen.

| piece | origin | how |
|---|---|---|
| planets, objects, placement, collision | **the disc** | Good Egg Galaxy's placement tables (13 zones, 220 objects); KCL collision as float triangles |
| gravity | **the disc**, rules from **Petari** | 106 fields (point, disk, plane-in-box, cube, segment, plane) with Range, Distant, Priority and Inverse; highest priority in range wins |
| Launch Stars | **the disc** | 8 stars, each flying the level's own Bezier rail |
| music | **the disc**, mapping from **Petari** | `SMG_galaxy01_strm`, intro then loop at the stream's own loop point; Petari's stage table maps the galaxy to it |
| the Goomba | **the disc** | its model, its 6-joint skeleton and all 28 of its BCK animations (`walk`, `run`, `wait`, `hit`, `flatdown` …) |
| item box | **another cartridge** | Mario Kart DS's `box.nsbmd`, through apicula |
| castle grounds | **another cartridge** | Super Mario 64 DS's `main_castle_all.bmd`, through the DS mashup's converter |
| 24 trees | **chosen by meaning** | candidates by name, confirmed by CLIP, planted only where the level's gravity agrees with the ground's normal |
| water planet | **chosen by meaning** | 14 candidates by name, ranked by CLIP; `WaterSphereObject` won at 0.71 |
| two planets to orbit | **chosen by geometry** | roundness measured on each planet's collision mesh; `KoopaBattleMapPlanet` and `BlueStarCupsulePlanet` |
| penguin | **Petari's name** | `Penguin` is an NPC class with its own archive |
| sound effects | **chosen by meaning** | 1,682 Galaxy waves and 861 Mario Kart DS waves decoded, then ranked by CLAP against 12 descriptions |
| controls, enemy AI, items, hit rules, camera, menu | **written here** | none of it recovered; the game's own Goomba behaviour is in code we did not decompile |

# 2. How much of the physics stands on its own

The author's question had a measurable answer. Galaxy's physics splits into **data** (which
field pulls where, how strongly it is preferred, how far it reaches) and **code** (how Mario
moves under it). The data was read whole. The code was not: the Goomba's movement is twenty
lines written here.

That was enough. Every planet in Good Egg Galaxy works under the level's own fields, and
planets taken from *other galaxies* work once given one field each. The orbit test
(`goomba_galaxy --autotest orbit`) holds "forward" and "run" for 30 simulated seconds on
each added planet, in two headings, with periodic jumps:

| planet | radius | held by its own field | grounded | swept round the centre |
|---|---|---|---|---|
| water sphere | 26 m | 100% | 0% (swimming 96%) | 669–1000° |
| `KoopaBattleMapPlanet` | 12.4 m | 100% | 41–56% | 693–1203° |
| `BlueStarCupsulePlanet` | 5.5 m | 100% | 13–53% | 1484–2371° |

The first run found a real fault. On the smallest planet, a running jump sideways left a
field three metres deep, and the Goomba drifted 818 m away. Galaxy's gravity has constant
strength inside its range, so any bounded jump comes back if the range is deep enough. The
fix was a data change, not a physics change: added fields now reach at least ten metres.

**What it says.** A game's gravity, written as data, survives transplanting into a new
engine, a new character and new planets. Its movement code does not come along, but it is
small, and the data constrains it tightly. This is the strongest evidence so far that the
[game model](/concepts/what-a-game-is.md) can hold *fields* as nouns: sayable, runnable with
no display, and recoverable by a reader from a game nobody here made.

# 3. Did the tools understand the requests?

| request | understood by | result | weakness |
|---|---|---|---|
| more trees | name, then vision, then gravity | 24 trees, upright on their planets | the first try planted trees upside down under disks; the fix asks the level's own gravity which way is up |
| a water planet | name, then vision | a water sphere, swimmable | two candidates tied until placeholder cubes ("Temporary HighModel") were labelled and rejected |
| planets to orbit | geometry | two round planets | a box scored perfectly round until roundness was measured on triangle centres |
| a penguin | a human-made name | one penguin facing the start | it does not move; its behaviour is code |
| Galaxy's sounds | audio-text retrieval | 12 effects, each with its CLAP score | **not checked by ear**; a named effect needs the sequence bytecode, which was not decoded |
| item box sound | audio-text retrieval | a sparkle from Mario Kart DS's waves | same caveat |

Every choice is written into the world file with its score, its runners-up and why, so a
human can overrule it. That is the shape the [Dungeon Master](/concepts/what-a-game-is.md)'s
proposals already have: a model's output is a proposal, and the record is what was accepted.

# 4. Residual

- **Behaviour.** Enemies wander and chase by code written here. The Goomba's own Galaxy AI,
  the penguin's, and every enemy's are not recovered.
- **Rendering.** Environment-mapped materials still render wrong (the coin). Only each
  material's first texture is used.
- **Castle grounds.** Collision is the render mesh, because Super Mario 64 DS's collision
  variant is not decoded. Its music is sequenced, so a Galaxy stream stands in. Walking off
  the mesh's edge respawns the Goomba.
- **Sounds** were found by meaning, not by the game's sound table.
- **14 object classes** in Good Egg Galaxy are placed but not modelled (star-bit groups,
  switches, the ocean sphere's controller and others), and are listed in the world file.

# 5. What Void Squid takes from it

1. **Gravity fields belong in the game model** as nouns with Range and Priority. They passed
   all three of railguard §8's tests here.
2. **Editing by meaning works when the data says where *up* is.** Vision chose the trees.
   The level's gravity decided where they could stand.
3. **A world file that records origin per piece** (disc, other cartridge, chosen by meaning,
   written by hand) is the shelf (S22) in practice. The game reads it, and the F1 panel
   shows it.
4. **Oracles were the whole method.** The orbit test, the autotest's screenshots and the
   world file's scores found every fault above. None were found by looking at the game.
