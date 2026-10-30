---
type: Design
title: Four games and the asset database — Donkey Kong 64, tags from the games' wikis, the three work set-ups, and Coral
description: "2026-10-12, third round of the Goomba Galaxy exercise. Fixes the author found (a menu that flickered shut, missing textures, the wrong GPU). Builds the author's asks: Turkey, Table and Feast as work set-ups; move, turn and size handles; music per map and sounds per turkey, with the person's own files; dropping in a model with collision made from it. Adds Donkey Kong 64 as the fourth game. Starts an asset database in Void Hormiga's schema, tagged from each game's own data, its wiki, names and vision. Lays Coral's foundation. And sets down how one game's systems translate into another's."
resource: ../ds-mashup/src/g2/table.cpp
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-12T00:00:00Z
---

> The author: the menu *"only flickers in for a single frame"*; *"textures for some things still
> don't load, or aren't present at all"*; Turkey, Table and Feast as *"different work set ups"*;
> *"the 3 arrows in 3d space"*; music per map and sounds per turkey, *"and even upload our own
> music"*; *"upload a custom obj, and have collisions and stuff automatically apply"*; Donkey Kong
> 64 next; a database like Void Hormiga's, with *"semantically accurate auto-given tags"*, and
> the games' wiki pages to help; *"start using GPU stuff"*. Then: a Coral, Void Squid's own
> Antfarm, for sharing with other players later.
>
> **Built, outside this repository** (`../ds-mashup`): the game, the turkey table, four
> toolbox steps, the asset database and the Coral journal. No game data and no wiki text are
> in any repository.

# 1. The three faults, and their causes

| fault | cause | fix, and the test that keeps it fixed |
|---|---|---|
| the menu closes the frame after it opens | the menu's own input map is switched on while Esc is still held, so its Cancel action sees a fresh press | an action in a newly enabled map ignores a key held over until it is released. A key-level script device holds the real Esc for six frames; the run checks the menu is still open |
| textures missing, things missing | (a) Galaxy's materials tint greyscale and compressed textures with a material colour register, which was not read (the Koopa planet's metal is red, the capsule planet blue, the water half-transparent blue); (b) star-bit rings and ocean spheres are built by the game's code, not loaded | (a) the converter reads the colour register; all 202 models converted again. (b) the toolbox builds them by Petari's rules: 12 star-bit rings (26 more star bits), 7 swimmable ocean spheres |
| the GPU | both programs ran on the laptop's Intel Iris Xe: Windows gives OpenGL programs the integrated GPU unless they ask | the programs export the two symbols NVIDIA's and AMD's drivers read; both now report the RTX A1000 at start |

# 2. The asks

- **Turkey, Table, Feast as work set-ups** ([turkey, table, feast](/concepts/turkey-table-feast.md) §6).
  The same windows in all three, each with a different job. Turkey inspects one thing: six
  checks, its kind, tags coloured by where they came from (click to remove), adding a tag
  with the shared vocabulary as suggestions, sounds per event (collect, defeat, touch,
  talk). Table builds a world. Feast sets the player (any turkey; speeds, jump, gravity,
  lives) and the order of the tables. The game reads the feast.
- **Handles**: arrows to move, rings to turn, boxes to size, and a centre box to size evenly.
  They use world or local axes, and Ctrl snaps to 1 m, 15 degrees or 0.1. Each drag is one
  undoable command.
- **Music and sounds**: a map plays any of the disc's 74 Galaxy streams, decoded as they play
  and looping at their own loop points, or a file the person drops in. A turkey can name its
  own sound per event, and the game plays it. Sequenced music (Super Mario 64 DS, Mario Kart
  DS, Donkey Kong 64) is not playable yet.
- **Dropping in a model** (.obj, .glb, .gltf) copies it with its materials and textures. It
  makes collision triangles from its shape, guesses its unit from its size, and adds it to
  the shelf and the database.

# 3. Donkey Kong 64, the fourth game

- **Map geometry and textures** come from dk64_lib, a GPL-3 library used as an invoked
  toolchain ([source](/sources/dk64-lib-and-the-game-wikis.md)). Jungle Japes and DK Isles are
  playable worlds.
- **Setups, read here**: per map, *n* "model two" objects, then an unknown block, then actors.
  The counts and record sizes add up exactly. Each model-two type's table entry carries
  **the game's own folder name**: plants, pickups, trees, torches, doors, pads, climb,
  buildings. Pickups are sprites, with animation frames as texture indices. The banana's
  frames are 168 to 175.
- **The scale, by oracle**: placed at three times their setup position, 71% of the bananas
  sit within 40 units of a floor (median 1.8); at one times, none do.
- **Lit and baked meshes**: 385 of Jungle Japes' 635 meshes have all-zero vertex colour. On
  the N64 those vertex bytes carry normals, and the game lit those meshes. The levels draw
  unlit, as the original did.

# 4. The asset database and its tags

Void Hormiga's schema (runes, fields, tags, links), in `WORLD_DIR/assets.db`. Mantles: games,
kinds (with traits), wiki, turkeys, vocabulary, tracks, maps. Every tag on a turkey says
where it came from:

| source | example |
|---|---|
| `game:` | Petari's category (Enemy), Donkey Kong 64's folder (trees), the kind |
| `wiki:` | the wiki section a thing is listed in, its categories' head nouns |
| `name` | a word of its name that the vocabulary already has (Coco**Tree**) |
| `vision:` | CLIP on a thumbnail against a shared visual vocabulary, with its score |
| `person` | added in the Turkey set-up; survives every rebuild |

**Measured** (final build, 2026-10-12):

| | result |
|---|---|
| turkeys | 235 from five games, 1,006 tags (633 from the games, 181 vision, 112 wiki, 93 names; the rest by person) |
| wiki entities | 773 listed on the four game pages (Galaxy 390, Super Mario 64 DS 185, Donkey Kong 64 128, Mario Kart DS 118); 896 pages cached |
| matched to a wiki entity | 24, among them Kuribo to Goomba and Takobo to Octoomba by romanization, DinoPackun to Dino Piranha |
| tagged by vision | 212 |
| vocabulary | 115 words |

Tags shared by two or more games include enemy, boss, item, coin, power-up, lava, barrel,
bomb, flower, water, shell, banana, fruit, plant, mushroom, wood, bridge, switch and block.
Most turkeys are not matched because 153 of them are Galaxy planets, whose internal names
the wiki does not list.

**Found while building it:**

1. **Tags launder.** The builder wrote its tags back into the catalogue, and the next run read
   them as the game's own (a coin became an enemy). The shelf's own tags are now kept apart.
2. **A wiki section is not a category of everything linked in it.** Links in a section's prose
   are mentions; only the first link on a list or table row is the entity.
3. **A category's head noun is its last word, and a category that names a class is plural.**
   One that names a game is not (*"Mario Kart 8 Deluxe"*), and gives no word. A title-cased
   prefix names a game or an episode (*"Bowser Jr.'s Journey allies"*), so it counts only when
   it is this turkey's own game. Some plural categories still record where a thing appeared
   (trophies, stickers), and a short list keeps those out.
4. **"Tree" is not yet shared across games.** Galaxy has tree models. Donkey Kong 64's trees
   are model-two geometry, which is not decoded yet, so they are not turkeys. Its folder name
   is waiting for them.
5. **A match must come from the turkey's own game's page.** At first Galaxy's ShootingStar
   matched Mario Kart DS's kart. Another game's entity counts only when the turkey's game has
   no page fetched (Mario Party DS).
6. **Super Mario Wiki writes the romanization as `JpnR`**, inside a template that nests
   others. Reading `JapR` matched none of 810 pages; reading `JpnR` gave Kuribo to Goomba.
7. **Hub pages are noisy.** Mario and Goomba are the first link on rows in many sections, so
   section tags count only when the thing is in at most two of its game's sections. Their
   franchise-wide categories (Mario: chef, pirate, referee) remain, as proposals.

# 5. Coral

[Coral](/concepts/coral.md): a reef per player and a journal of every shareable change.
Local only. The turkey table writes it.

# 6. Translating one game's systems into another

The author: *"the more games we add, the more abstract the mathematics is going to get."*
The field has two families ([cross-game transfer](/sources/cross-game-transfer.md)): learned
projection into a shared space, and structural analogy. Decompilation gives Void Squid the
structure the second needs. So far one system has always contained the others: Galaxy's
gravity fields include a constant pull as a plane field with no range, which is every other
game's gravity. Minecraft's redstone and terrain will be the first systems no game here has.
S27 asks what happens then.

# 7. Residual

- **Donkey Kong 64:**
  - Collision is the render mesh; the collision tables are not decoded.
  - Model-two geometry and actors (enemies, the Kongs) are markers only.
  - The lit path draws its normal-lit meshes black, cause not found, so the levels draw unlit.
  - DK Isles has no collision on its water.
- **Galaxy:** materials still use only their first texture and colour register. TEV stages
  and environment maps are not read.
- **The menu fix** is tested with a scripted key. The controller is still untested by a
  person.
- **Tags:** wiki matches are by name and romanization, so they are proposals (S26). Vision tags
  are CLIP's best of a fixed list.
- **Coral** has no network, by design for now.
