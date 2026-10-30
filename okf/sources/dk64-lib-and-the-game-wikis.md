---
type: Source
title: dk64_lib, and the games' wikis as a layer of meaning
description: "Two outside sources used directly on 2026-10-12. dk64_lib (Python, GPL-3, 0.2.1): reads Donkey Kong 64's pointer tables and exports map geometry with textures; invoked as a toolchain, never imported. Super Mario Wiki and the Donkey Kong Wiki: each game's page and every thing linked from its lists, downloaded once as wikitext and used for categories, Japanese romanizations, wiki categories and first sentences, which tag the turkeys."
tags: [status:current, audience:dev, confidence:verified, source]
timestamp: 2026-10-12T00:00:00Z
---

**`verified`**: both were used here, not read about. What each claim rests on is said
below.

# dk64_lib

- **What:** a Python library that reads a big-endian Donkey Kong 64 ROM, walks its pointer
  tables, and exports map geometry (GLB, glTF, OBJ, DAE) with decoded textures, plus text.
  Documentation at dk64-lib.readthedocs.io; installed from PyPI, version 0.2.1.
- **Licence: GPL-3.** So it is used the way apicula is: installed in its own environment
  (`../ds-mashup/.tools/dk64-venv`), driven by one small script (`tools/dk64_export.py`),
  never vendored and never imported by the exercise's own code. Void Squid's rule 12 calls
  this an invoked toolchain.
- **Verified here:** it reads the PAL ROM (region `pal`, pointer table at 0x1038D0) and
  exports Jungle Japes and DK Isles with their textures.
- **What it leaves undecoded** (its own table): setups, collision, model-two geometry and
  actors. The exercise reads setups itself: three counts and three record sizes that add up
  exactly. It also reads model two's category names and the pickups' sprite frames.
- **Found here:** in its export, 385 of Jungle Japes' 635 meshes have vertex colours of zero.
  On the N64 those meshes are lit by normals; the colour bytes carry normals instead.

# The games' wikis

- **What:** Super Mario Wiki's pages for Super Mario Galaxy, Super Mario 64 DS, Mario Kart DS
  and Donkey Kong 64, and the Donkey Kong Wiki's (through its API; its pages answer a
  browser check). The author's idea: *"a way to help you could be to actually reference the
  wikipedia pages for these games."*
- **How:** each game page's list sections (enemies, obstacles, bosses, items, power-ups,
  objects, characters, levels) give 773 linked things. Each thing's page is fetched
  once, as raw wikitext, one request every 1.5 seconds, under a User-Agent that says who
  is asking. They are cached in `Documents/games/wiki-cache`.
- **Used for:** the section a thing is listed in, its Japanese romanization (which matches
  Petari's internal names: Kuribo for Goomba), its wiki categories (each one's last word:
  *"Super Mario Galaxy enemies"* gives *enemy*), and its first sentence as a description.
- **Licence:** the wikis' text is CC BY-SA. It stays in a local cache and in a local
  database; nothing from it is in a repository.
- **Found:** category words need a filter. A category often records where a thing
  appeared (*"Super Smash Bros. Brawl stickers"*), not what it is; those words are kept out.
  A category naming a class is plural, and one naming a game is not. Super Mario Wiki writes
  the romanization as `JpnR`, not `JapR`. 896 pages are cached, for 773 entities.
