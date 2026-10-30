---
type: Source
title: Destructible and modifiable worlds — GeoMod, Minecraft, Noita, Teardown, Dwarf Fortress
description: The existing instances of the author's fourth belief, that terrain and scenery should be modifiable. What representation each one chose, because the representation decides what can change.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-09T00:00:00Z
---

| game | year | representation | what can change | what cannot |
|---|---|---|---|---|
| **Red Faction** (GeoMod) | 2001 | polygon meshes with runtime CSG | blow a hole through a wall | anything the CSG cannot keep closed |
| **Minecraft** | 2011 (alpha 2009) | a uniform voxel grid | every block, by hand | shapes finer than a block |
| **Dwarf Fortress** | 2006– | a tile grid with per-tile material and a simulated history | the world, its geology and its past | its visual fidelity, by design |
| **Noita** | 2019 | a falling-sand simulation, *"every pixel is simulated"* | every pixel, with material rules (fire, acid, water) | scale: it is 2D |
| **Teardown** | 2020 | small voxels with a raytracer | every voxel, physically | anything that is not a voxel |

**The pattern.** A world is modifiable exactly to the extent that its representation is
**uniform and content-addressable**: a grid of cells with materials can change anywhere;
a hand-built polygon mesh can change only where someone wrote a special case. The games
that let the player change the most all chose a grid.

**What it settles here.** The author: *"the ability to actually change the terrain, and
modify the scenery. where assets are not static, but dynamic."* Void Jelly's **voxel nib**
(`VoidJelly:/concepts/nibs/voxel.md`) is the family's grid, and it already exists as a
target. The design consequence in [what a game is](/concepts/what-a-game-is.md) §5 is
that modifiable terrain is a **region rune whose geometry is a block the runtime may
rewrite**, with the forge seeing terrain state as data and the runtime owning its live
copy. Which half owns the live state at play time is Jelly's open question §19 arriving
in a game instead of a desktop, and it is raised with Jelly rather than settled here.

**Confidence.** `asserted`, from recall of the games and their developers' talks (Petri
Purho's GDC 2019 talk on Noita; Volition on GeoMod). Verify years and the Noita quote.
