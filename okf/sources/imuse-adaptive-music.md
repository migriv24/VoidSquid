---
type: Source
title: iMUSE and adaptive music — music that reacts to the player
description: LucasArts' iMUSE (Land & McConnell, first shipped in Monkey Island 2, 1991; US patent 5,315,057, 1994): music composed as segments with decision points, so the score changes at musically sensible moments in response to game state. The lineage behind today's horizontal re-sequencing and vertical layering in Wwise and FMOD.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-09T00:00:00Z
---

**Claim this bundle leans on.** iMUSE's idea was that the game does not *switch tracks*;
it sets **conditions**, and the composer has marked **decision points** and **transition
segments** in the score so that the music moves between states on a beat or a phrase
boundary rather than at the moment of the event. Two techniques descend from it and are
now standard: **horizontal re-sequencing** (which segment next) and **vertical layering**
(which stems are audible). Both are authored by the composer and selected by game state.

**What it settles here.** The author wants *"music that reacts to the player."* The
structure that makes that possible is exactly the forge/artifact split: the composer
authors a **graph of segments and layers with conditions** (structure, the forge's), and
the runtime walks it against world state (dynamics, the runtime's). The conditions are
world facts the vocabulary already holds (the avatar's region, a stat crossing a
threshold, a beat beginning), so a music cue is a rune with links into the world, not a
file name in a script. The audio engine itself is Void Loops Studio's domain, which the
author has said is *"not at all ready for integration yet"*; this page records only the
shape of the seam. See [what a game is](/concepts/what-a-game-is.md) §5.

**Confidence.** `asserted`. Verify the patent number, the 1991 date and that Land and
McConnell are the named inventors.
