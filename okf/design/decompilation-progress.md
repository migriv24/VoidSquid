---
type: Design
title: Decompilation progress
description: How complete is each game? Every page of a game's wiki and its Wikipedia article is a thing, a place, a system or another game; each thing should map to a turkey and each place to a table. The Progress window counts the matches, ranks the missing pages by how often the wiki links to them, and a small local model proposes matches under guards that rely little on its intelligence.
resource: ../../src/library/progress.cpp
tags: [status:current, audience:dev, confidence:verified]
timestamp: 2026-10-13T00:00:00Z
---

# Decompilation progress

The author, 2026-10-12: a tool to check the progress of decompilation that compares
*semantically* to the game's wiki and Wikipedia; *"every wiki link should be able to associate
or link to some rune/turkey."* And from AestheticAtlas (CornersOfTheInternet): a wiki read as a
graph of pages and links.

## The idea

A game's wiki is a list of what the game contains, written by people who played it. If the
library recovered the game, every page about a thing in it should map to a turkey, and every
page about a place to a table. The gap is the progress.

`src/library/progress.cpp` classifies each wiki entity of a game:

| class | how | maps to |
|---|---|---|
| thing | characters, enemies, items, objects, bosses | a turkey |
| place | the wiki's categories say level, galaxy, world, course (categories only: one game's "location" section listed characters) | a table |
| system | power-ups as mechanics, game modes, controls | logic, later |
| game | the page is another game (category Games) | excluded |

and counts how many have a turkey or a table, by name, by Japanese internal name, or by an
accepted proposal. **Missing pages are ranked by in-degree**: how many other pages of the wiki
link to them (the asset database stores 10,670 such "mentions" links). That is AestheticAtlas's
lesson: a page many pages point at is central, and missing it matters more.

## Wikipedia

`ds-mashup/toolbox/wikipedia_fetch.py` downloads each game's Wikipedia article as raw wikitext
into the person's wiki cache (never this repository), with the links of its Gameplay, Plot,
Story, Synopsis, Premise, Setting and Characters sections, and each linked page's summary. Rate
limited to one request every 1.5 s. Fetched: Galaxy 61 links, SM64DS 28, MKDS 24, DK64 28. The
asset database merges them as sections `wikipedia:<section>`, and Progress counts them
separately: a game's Wikipedia article is the short list of what matters most.

## Measured (the four-game library, 2026-10-13)

| game | wiki pages matched | Wikipedia links matched | most-linked missing |
|---|---|---|---|
| Super Mario Galaxy | 19 of 433 (4%) | 2 of 59 | Princess Peach, Star Bit, Bob-omb, Piranha Plant, Boo, Warp Pipe |
| Donkey Kong 64 | 5 of 148 (3%) | 1 of 28 | Tiny Kong, Diddy Kong, King K. Rool, Health Meter, Lanky Kong, Chunky Kong |
| Mario Kart DS | 16 of 138 (12%) | 6 of 29 | Princess Peach, Bob-omb, Piranha Plant, Boo, Warp Pipe, Bullet Bill |
| Super Mario 64 DS | 7 of 202 (3%) | 4 of 27 | Princess Peach, Bob-omb, Piranha Plant, Boo, Nintendo, cannon |

Low numbers, and honest: the library holds what three exercises needed, not the games. The
ranking says what to decode next, and for Donkey Kong 64 it agrees with the
[cast](/design/the-cast.md)'s residual: the Kongs.

**The ranking also finds the matcher's own gaps.** Mario Kart DS's Peach is playable and
Galaxy's Star Bits are a kind, yet "Princess Peach" and "Star Bit" are missing: the turkey is
called `Peach` or `StarPiece`, and a match by name or romanisation does not reach. That is the
job the pilot below proposes for, and a reason to accept its proposals. "Nintendo" and
"cannon" come from Wikipedia's links and are noise the classifier does not yet drop.

## The Games window (2026-10-13, evening)

The author: *"a window where we manage the games we have loaded in our datasets, and their
decompilation progress… I would assume that most games would only have like 80% decompilation
rates."* The Progress window became **Games**: per game, three measures that count different
things, side by side.

| measure | what it counts | where it comes from |
|---|---|---|
| Decompilation | code and data matched to the original | decomp.dev's project list (`effect decomp refresh`, one request, on a button) |
| The wiki | pages of the game's wiki the library has a turkey or table for | the progress report above |
| The library | turkeys, characters, animations, worlds | the library |

On 2026-10-13 decomp.dev listed Super Mario Galaxy (Petari) at 100% of code and 88% of data, and
Mario Kart DS (RonyxDumb/mkds) at 1.5% of code; Super Mario 64 DS and Donkey Kong 64 are not
tracked there (both have decompilations elsewhere: tangosdev/sm64ds-decomp, 0xGlitchbyte/dk64re).
Each game shows where it was loaded from (the route writes `games.json`) and **load again**.

## The pilot: can a small model help, safely?

The author asked to *"test the MOSU"* with small models (Needle-class) and heavy guardrails
that rely little on intelligence. `src/library/pilot.cpp` is that test.

**The task:** given a turkey (internal name, class, category, kind), name its wiki page.

**The guards** (most of the work is rules, not the model):

1. **A deterministic shortlist** of 12 candidates from this game's pages, scored by shared
   words through a 31-entry glossary of Nintendo's internal words (`kuribo` → goomba,
   `teresa` → boo, `tico` → luma), kind fit and centrality. The model only chooses.
2. **Structured output**: Ollama's JSON schema with the shortlist as an enum plus "none";
   `think:false`, temperature 0. It cannot answer outside the list.
3. **Self-agreement**: asked twice, the shortlist reversed the second time. Both must agree.
4. **Kind fit**: an enemy cannot match an item page.
5. **Not taken**: another turkey does not hold it.
6. **Exact name outranks the model**: a candidate whose name *is* the turkey's (brackets
   aside) is the answer, and a model that picks another is overruled.
7. **The result is a proposal**, landing with source `local:<model>` for a person to accept
   (railguard: a model's output is a proposal; the mantle records what was accepted).

**The test** (`effect pilot test`): 25 turkeys whose page is already known, the answer hidden.

| model | correct | wrong accepted | abstained |
|---|---|---|---|
| shortlist's first name, no model | 18 | 7 | 0 |
| qwen3:0.6b | 7 | 0 | 18 |
| llama3.2:1b | 14 | 2 → **0** with guard 6 | 9 → 11 |
| gemma4:e2b (≈2B effective) | 17 | 3 → **0** with guard 6 | 5 → 8 |

**What it says.** With these guards a 2-billion-parameter model is *safe* (no wrong accepts in
25) and nearly as productive as the rule, and the rule alone is *productive and unsafe* (7
wrong). A sub-billion model is safe and mostly abstains. The wrong accepts before guard 6 were
all "a near name beat the exact name" (Banana → Banana Coin; Luigi → Luigi (character)), which
is why the guard is a rule and not a better prompt.

**On the Turkey card (2026-10-14).** A turkey with no wiki page shows the model's proposal and the
guards it passed; **Accept** writes it with source `local:<model>`, **Not this** is remembered.

**What it does not say.** The 25 cases are the easy ones: their page was already found by
name. The hard cases (an internal name with no English in it) have no ground truth yet, so
the test cannot score them. Needle itself was not run (S29).
