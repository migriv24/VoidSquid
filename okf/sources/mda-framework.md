---
type: Source
title: Hunicke, LeBlanc & Zubek — MDA: a formal approach to game design and game research
description: 2004, AAAI workshop on challenges in game AI. Mechanics → Dynamics → Aesthetics, read by the designer left to right and by the player right to left. The standard three-layer vocabulary, and a map onto forge / runtime / player.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-09T00:00:00Z
---

**Claim this bundle leans on.** **Mechanics** are the rules and data; **dynamics** are
the run-time behaviour of mechanics acting on player input; **aesthetics** are the
emotional responses evoked. The designer can only author mechanics; dynamics and
aesthetics emerge.

**What it settles here.** The three layers land on the family's lines exactly.
Mechanics are **the forge's** (authored structure and rule tables). Dynamics are **the
runtime's**, at frame rate, which is why Void Squid composes structure and never
mechanics in motion (the 2026-10-03 lean). Aesthetics are **the player's**, and the only
instrument the forge has on them is a question a game view can show and nothing can
compute: *what did the world answer?* See [the three pillars](/design/the-three-pillars.md) §2.

**Confidence.** `asserted`. Verify the paper's title and year.
