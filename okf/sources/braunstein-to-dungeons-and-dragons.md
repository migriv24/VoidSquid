---
type: Source
title: From Braunstein to Dungeons & Dragons — the referee as the first game engine
description: Wesely's Braunstein (1969), Arneson's Blackmoor (1971), Gygax & Arneson's Dungeons & Dragons (1974). A human referee runs the rules, adjudicates what the rules do not cover, and generates the world as the players explore it. The author's "DM" is this role, and a game engine is this role made mechanical.
tags: [status:planned, audience:dev, confidence:asserted, source, foundation]
timestamp: 2026-10-09T00:00:00Z
---

**The lineage, as usually told.** In 1969 David Wesely ran *Braunstein*, a Napoleonic
wargame in which each player took a single character with private goals in a town, and
the **referee** adjudicated whatever the players tried, including things no rule
covered. Dave Arneson took the format into a fantasy setting as *Blackmoor* (1971), with
persistent characters, a dungeon under a castle, and advancement over sessions. Gygax
and Arneson published *Dungeons & Dragons* in 1974, with the referee named the
**Dungeon Master**.

**What the DM actually is.** Three jobs in one person:

1. **Runs the rules.** Rolls dice, applies tables, tracks hit points. Mechanical,
   deterministic given the dice. A computer does this part well.
2. **Adjudicates the gaps.** Rules never cover everything; the DM decides what happens
   when a player does something unanticipated. This is *judgment* over the world's
   state, and it is where a game feels alive or dead.
3. **Generates the world.** Describes what the players see, voices the non-player
   characters, improvises what is behind the door nobody was expected to open, and
   keeps a narrative arc across all of it.

The published rulebooks are a **game engine for a human processor**: data tables (monster
stats, spell lists, treasure types), procedures (initiative, saving throws) and a
vocabulary (character, class, level, hit die, alignment). Everything a video game engine
later did in code, D&D did on paper with a person in the loop. That is why the author's
instinct to *"copy concepts and core philosophies from DnD"* is the right place to start
an ontology: it is the earliest fully general one, and it was designed to be run by hand,
so every concept in it is legible.

**What it settles here.** [What a game is](/concepts/what-a-game-is.md) §2 takes the
DM's three jobs as the three places computation lives in a Void Squid game: job 1 is the
**runtime** (rule tables compiled from the forge), job 2 is **the authored rules and
their derived state** (Void Core rules; Void Allomone for what is derived and never
authored), job 3 is **generative** and is the only one that wants a model (Void Bicho,
behind a holiday). The *character sheet* is the oldest example of a rune with typed
content and quantities. *Levels* and *experience* are the oldest upgrade graph.

**Confidence.** `asserted`, from recall of the standard histories (Peterson, *Playing at
the World*, 2012, is the one to check). Verify the dates and that Braunstein's referee
format is the acknowledged ancestor.
