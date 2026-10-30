---
type: Design
title: Goomba Galaxy 2 and the first turkey table — one physics engine, the game's own lights and cameras, and placing turkeys
description: "2026-10-11, second round. The author's notes on the first Goomba Galaxy (lighting, items and interactables, the camera, a castle world with no collision; music, sound, animation and run times good) and their asks: research physics and lighting, rebuild the level, support controllers the way Unity abstracts input, and start the turkey table. Built: a new engine on Jolt, with the game's own lights, cameras and lines, and a table editor whose saved tables the game plays. What it found, and the author's observation that every step adds more than the original game had."
resource: ../ds-mashup/src/g2/game.cpp
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-11T00:00:00Z
---

> The author: lighting (*"we may actually want to invent our own lighting"*), items and
> interactables, the camera (*"sometimes using the game's camera system is fine, but other
> times, we may want to have our own"*), and *"the mario 64 world wasnt able to collide with
> anything … consider universal physics engine"*. Music, sound effects, animation and run
> times were good. Then: *"we are always adding MORE to the original base game"*; begin the
> turkey table, *"placing assets on a 'table' and designing levels"*; and controllers,
> *"similar to Unity's controller abstraction"*.
>
> **Built and running**, outside this repository:
> `../ds-mashup/build/bin/goomba_galaxy2.exe` (`goomba_galaxy2.cmd`) and
> `../ds-mashup/build/bin/turkey_table.exe` (`turkey_table.cmd`), on a shared engine in
> `../ds-mashup/src/g2/`. The research is in
> [physics, lighting and input](/sources/physics-lighting-and-input.md).

# 1. The five notes, and what answered each

| note | answer | how it was checked |
|---|---|---|
| lighting | **the game's own lights first**: `LightData.arc` (13 settings, 33 light areas), actor and map sets per area and zone; then shading written here: linear-space lighting, a soft key light, a rim light, a shadow map around the player, a drop shadow along gravity, a starry sky, a filmic curve | a look test renders each view lit, textures-only, light-only and as normals |
| items and interactables | prompts that name the button of the device in use; coins, star bits, 1-ups, Power Stars; Launch and Sling Stars; **NPCs speak the game's own lines** (1,743 from `message.bmg`); Mario Kart DS item boxes that fall as physics bodies; a roulette of shells, banana, mushroom and star, all physics bodies under the level's gravity | the scripted run stomps, opens a box, throws, flies, talks |
| camera | **two cameras and a switch**: the game's camera areas (416 cameras, 76 areas) with five types implemented from Petari, and a camera written here (lazy follow, rises over walls, hides scenery it is inside) | the HUD names what drives the camera and whether it came from the disc |
| castle collision | **Super Mario 64 DS's own collision, decoded**: its KCL stores normals and triangle lengths at different fixed-point units from Mario Kart DS. With them, all 1,243 triangles land inside the model, and the ground matches the render mesh within 15 cm at 98% of points | a floor probe: rays, a standing test, a running test, dropped balls |
| a universal physics engine | **Jolt** for every world, with Galaxy's fields applied by the game ([research](/sources/physics-lighting-and-input.md) §1) | the same code in three worlds, and the orbit test on Jolt |

Controllers: an input layer shaped like Unity's (actions, bindings by control path,
composites, processors, schemes that follow the last device, rebinding from a file). It
drives the menu too. **It has not been tried with a physical controller.**

# 2. The turkey table, first build

The author's vocabulary ([turkey, table, feast](/concepts/turkey-table-feast.md)), made
into a tool:

- **The shelf** lists 215 turkeys from `toolbox/turkey_shelf.py`: 202 Super Mario Galaxy
  archives with Petari's class and category, Mario Kart DS's item box and items, and the DS
  shelf's characters and courses. Each has a kind (what the game will do with it), tags,
  a unit, and whether the game's own collision exists.
- **The table** is a 3D view. A picked turkey stands **upright to the world's gravity**
  where you click, not merely to the surface. Grab, turn, size, delete, duplicate, undo
  and redo all work.
- **The inspector** sets kind and tags, gives a planet its own point gravity field (at
  least ten metres deep, the orbit test's lesson), and sets the start.
- **Save** writes an ordinary world file and lists it in the game's worlds. **Play** saves
  and starts the game on it.
- **The transcript** writes every edit in the table's words: *"table: placed turkey
  KoopaBattleMapPlanet (smg.KoopaBattleMapPlanet)"*, *"table: gave KoopaBattleMapPlanet a
  point gravity field"*. That is the logging the concept page asked for (§5).

**Verified:** the editor's autotest builds *Goombas on a Koopa planet* from three games: a
Galaxy planet, two Galaxy Goombas, a Mario Kart DS item box, a palm tree, the penguin and a
coin. The game then plays it. The Goomba runs all the way round the planet, grounded the
whole time, and stomps a Goomba for a Mario Kart DS item box.

# 3. What it found

1. **Every recovered layer was there to be read.** Lights, light areas, cameras, camera
   areas and every line of dialogue were in the disc's own tables, named by Petari. None of
   it needed a model, and all of it came out the same day.
2. **Two game formats lie quietly about one thing each.** Super Mario 64 DS's collision
   uses other fixed-point units than Mario Kart DS's. Super Mario Galaxy's models, as
   converted here, carry one placeholder normal. Neither fault shows until something new
   is built on top. That is the strongest argument yet for depth and oracle on every piece
   ([decompilation](/design/decompilation.md)).
3. **Gravity fields belong to the world, not the engine.** The engine pulls each body
   along whatever field the world's data picks. A table made from three games orbits
   correctly with nothing changed in the engine.
4. **The table works because the world file is the game model in miniature.** One format
   is read by the game and written by the editor, with a provenance line on each thing.
   This is S22's lean in practice.

# 4. "We are always adding more"

The author's observation is right, and it is the shape of the work. Each step left the
original game as it was and added a layer:

| layer | example | in the file as |
|---|---|---|
| **the game, read** | placement, gravity, lights, cameras, lines | the disc's own values, with their field names |
| **other games, read** | an item box from Mario Kart DS, a castle from Super Mario 64 DS | a `source` naming the cartridge and file |
| **chosen by meaning** | trees, a water planet, sounds | the choice, its score, its runners-up |
| **written here** | movement, AI, the Squid camera, rim light, roulette weights | named on screen (F1) and in code comments |
| **placed by a person** | a table | `added: placed on the turkey table`, and the transcript |

The original is the floor, never the ceiling. That only stays honest if every layer says
which one it is. S24 asks whether that becomes a rule of the game model.

# 5. Residual

- **Not tried with a real controller.** The bindings follow GLFW's Xbox layout.
- **Camera types not implemented:** charmed, tower, object-parallel, front-and-back and
  others. They fall back to the Squid camera, and the HUD says so.
- **Galaxy's material lighting** (`MAT3` channels, TEV, environment maps) is not decoded.
  The rim and shading are invented.
- **Normals for Galaxy models** are rebuilt from faces, because `j3d.py` writes a
  placeholder.
- **Mario Kart DS's item table** (`ItemSlotList.dat`) is not decoded; the roulette weights
  are invented.
- **Super Mario 64 DS's shadow decals** draw as dark ovals, and its sky and music are
  stand-ins.
- **The table** has no gizmo handles, no free (non-gravity) rotation, no disk or box fields,
  and no feast (no ordering of tables).

# 6. What Void Squid takes from it

1. **Lights, light areas, camera areas and lines are game-model nouns.** Each passed
   railguard §8: sayable, pure, recovered from a game nobody here made.
2. **The table is a studio surface prototype.** It is a `project`/`compile` pair over one
   world document, with a transcript, and it owns pixels only because it is an exercise.
   S23 asks when it moves onto Void Maiz.
3. **Physics is a host effect with world data**: one engine, the fields in the document.
   This is the effect-list shape of [hosts](/concepts/hosts.md).
4. **Debug views per lighting term and a probe per world** are the oracles here. They found
   every fault this round, as the evaluators did last round.
