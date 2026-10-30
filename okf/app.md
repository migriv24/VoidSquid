---
type: Manifest
title: Void Squid
name: Void Squid
id: voidsquid
version: 0.1.0
description: The game engine of the Void family, on Void Core and Void Maiz — first the host that opens a .ink, shows it, and builds it, performing every effect Void Jelly refuses to; then a forge that pours one document into many artifacts and can take artifacts apart again.
status: alpha
authors: [migriv24]
icon: squid
theme: void
palette.primary: "#1f7a8c"
palette.accent: "#e0b0d5"
palette.bg: "#07131a"
palette.ink: "#dfeef2"
tags: [status:planned, audience:dev, confidence:asserted]
timestamp: 2026-10-09T00:00:00Z
---

The **app manifest** for this bundle — Void Squid's structured self-description (see
`VoidCore:/concepts/app-manifest.md`). The family registry's view of the same thing is
`void.json` at the repository root, written 2026-10-09 at Void Mago's request; the two
agree on the version.

`version: 0.1.0` (2026-10-12): the studio is built ([Void Squid 0.1](/design/void-squid-0.1-the-studio.md)); the game model and the build button are not.

# What it is, in one line

**A conformance harness for [Void Jelly](../../VoidJelly/okf/index.md) that happens to look
like an editor** ([scope](/concepts/scope.md)), and, after that, **three pillars**: a pure
game model, a studio of projections, and hosts that differ only in their effect list
([the three pillars](/design/the-three-pillars.md)). It is called a game engine because
that is what it becomes; treating v0 as one is the failure mode
[the railguard](/design/what-voidsquid-is-not.md) exists to prevent.

# Name

*Squid* was the author's name for the whole 3D project at its founding on 2026-08-25. On
2026-08-26 the author moved it here — to the game engine — and renamed the library **Void
Jelly**. The cephalopod stayed with the thing that *acts*; the jellyfish took the thing that
*flows through* everything. `.ink` stayed with Void Jelly, where it names the format.

Both bundles record the rename in their logs. The game model's own name, and the game
document's, are the author's to give ([open questions](/design/open-questions.md) S12).
