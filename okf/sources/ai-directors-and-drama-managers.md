---
type: Source
title: AI directors and drama managers — the DM as software
description: Façade's drama manager (Mateas & Stern, 2005), Left 4 Dead's AI Director (Booth, GDC 2009), and "experience management" (Riedl & Bulitko, AI Magazine 2013). Three ways software has taken the Dungeon Master's third job, and what each one computes over.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-09T00:00:00Z
---

**Façade (Mateas & Stern, 2005).** An interactive drama in one apartment with two
characters. A **drama manager** sequences authored *beats* against a desired tension
arc, choosing the next beat from what the player has done; a natural-language layer maps
typed input onto discourse acts. The authors' own account is that the beat library had
to be enormous for twenty minutes of play.

**Left 4 Dead's AI Director (Booth, 2009).** No story at all: the Director watches a
per-player *stress* estimate and spawns or withholds enemies and items to produce a
*pacing* curve (build-up, peak, relax). It is the DM's third job reduced to one variable
and one lever, and it shipped, which is the point.

**Experience management (Riedl & Bulitko, 2013).** The general framing: an agent that
manipulates the world to steer the player toward an authored experience, under the
constraint of preserving the player's sense of agency. Covers planners that rewrite the
story graph when the player breaks it, and the trade-off between authorial intent and
player freedom.

**What it settles here.** All three compute over **authored structure** (beats, a
tension curve, a story graph) plus **observed play**. None of them generates from
nothing; the modern move is to put a language model where Façade's beat library was. In
this bundle's terms the director is the one part of a game that is **generative at play
time**, so it is the one part that wants a model, and a model is a holiday (Void Bicho).
Everything it steers is structure the forge authored, which keeps the forge/artifact line
intact: the forge writes the beats and the arc; the director, in the artifact, chooses
among them. See [what a game is](/concepts/what-a-game-is.md) §2 (the DM's third job)
and §5.

**Confidence.** `asserted`. Verify Façade's beat architecture in the 2005 AIIDE paper,
Booth's talk title ("The AI Systems of Left 4 Dead"), and the Riedl & Bulitko citation.
