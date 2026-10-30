---
type: Source
title: Entity-component systems — Dungeon Siege (Bilas, 2002) and Overwatch (Ford, 2017)
description: Two GDC talks that fixed the vocabulary game runtimes now share: entities are ids, components are data, systems are the only code. Why a Void Core mantle reads as an ECS, and why that is a projection rather than an identity.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-09T00:00:00Z
---

**Bilas, "A Data-Driven Game Object System" (GDC 2002).** Dungeon Siege replaced a
class hierarchy of game objects with **components** composed at data level, so designers
could make new kinds of thing without a programmer. The talk is the usual origin cited
for the ECS pattern in games.

**Ford, "Overwatch Gameplay Architecture and Netcode" (GDC 2017).** A strict ECS:
components hold data and no behaviour, systems hold behaviour and no state of their own,
and the ordering of systems is the architecture's main design surface. Bevy follows this
shape with a scheduler that computes system order from declared data access.

**What it settles here.** Latin OS's reading (`LatinOS:/design/the-os-as-a-game.md` §5)
is that a **mantle is an ECS with a confluent scheduler**: runes are entities, tags and
content are components, rules are systems, and Reduce needs no ordering because
confluence makes it immaterial. This bundle adopts the reading with one precision: a
Bevy world is a **projection** of a mantle (Jelly's Bevy nib emits a spawn table), not
the mantle itself, because the runtime runs its systems at frame rate and the forge
never does (Void Unity's two-clocks law). Projecting one game's systems into another's
is then a nib plus a translation holiday, with a residual for what the second world
cannot hold. Recorded in [the forge for many runtimes](/design/the-forge-for-many-runtimes.md).

**Confidence.** `asserted`. Verify both talk titles and years on the GDC Vault.
