---
type: Design
title: Three uses — decompile, create, simulate
description: "The author's 2026-10-09 reading of what the engine is for: decompiling existing games into Void Core, creating games from scratch around runes, mantles and holidays, and simulating inside a built or decompiled world, including as a sandboxed 'virtual box'. Each as a workspace over one document, on the precedent of Void Hormiga 0.2.0's data-defined kinds and workspaces; what each needs that exists, and what is missing."
tags: [status:planned, audience:dev, audience:all, confidence:exploratory, foundation]
timestamp: 2026-10-09T00:00:00Z
---

> **Nothing here is built.** The author named three directions the engine could take
> and asked to work toward all of them. This page says what each is in the family's
> terms, what already exists for it, and what is missing. The vocabulary the creation
> workflow uses is [Turkey, Table, Feast](/concepts/turkey-table-feast.md).

# 1. One engine, three workspaces

Void Hormiga's 0.2.0 (2026-10-09) is the precedent. It stopped owning the *kinds* of
thing a database holds (they became `kind` runes in a `kinds` mantle, with traits, that
travel with the database) and gained **workspaces**: *New database* asks what it is
for, and answers with a D&D 5e campaign, a cat grocery or a smart home, *each made
only of data-defined kinds*. The application did not grow three modes. It grew one
vocabulary and three starting documents.

Void Squid takes the same shape. **Decompile, create and simulate are three
workspaces over one kind of document** (the game document, S12), not three
applications. What differs is where the document's content comes from and which
surfaces open first.

| workspace | the document comes from | opens first | the arrow |
|---|---|---|---|
| **decompile** | an existing game, through readers | the Turkey list of recovered entities, and the ingress residual | artifact → document |
| **create** | a person, from nothing or a template | Turkey, Table, Feast | document → artifact |
| **simulate** | either of the above | the game view and the spatial log | document → running instance → log |

# 2. Decompile: games into Void Core

> The author: *"decompilation of video games, into the Void Core format (similar to how
> there is a rise of projects decompiling games into the Bevy ECS format)."*

**What exists.** The ingress architecture ([construct and deconstruct](/concepts/construct-and-deconstruct.md)):
a reader is the inverse of a nib, and returns a document plus a residual naming what
it could not recover. REA (vendored nowhere; a toolchain) drives Ghidra and friends.
Jelly's assetless `.ink` (its Q17) is how a decompiled scene can reference the
player's own copy of the game's bytes instead of carrying them. The ECS projection
(`VoidJelly:/design/rust-and-bevy.md`) says a Bevy world is one projection of a mantle,
so "decompile into Bevy" and "decompile into Void Core" differ in one step.

**What the author added today.** *"When decompiling, we should also attempt to
decompile a game, and sort of reconstruct it with our own structure."* That is two
passes with two residuals:

1. **Recover** what the game *has*: entities, meshes, tables, scripts, as faithfully as
   the reader can, residual naming what it could not parse.
2. **Restructure** it into Void Squid's nouns: entities become turkeys, levels become
   tables, the progression tables become the upgrade graph
   ([what a game is](/concepts/what-a-game-is.md) §4), dialogue trees become beats. The
   second residual names what did not map: a mechanic with no turkey, a trigger with no
   rule.

*2026-10-10: this workspace now has its own design page, [decompilation](/design/decompilation.md):
the shelf the author pictured (Roblox's toolbox, filled by a "decompiling game" bar), four
depths of recovery, the player taken apart, and Void Bicho as load-bearing. Its first stage,
[recon](/concepts/recon.md), is built.*

**What is missing.** A reader for any real game; the game document (S12); and
**layer-3 perception** ([smart logs](/design/smart-logs.md) §3), because a decompiled
world arrives *untagged*. Its objects have to be found and labelled before they can
be turkeys, and that is exactly what segmentation and captioning models are for. This
is the strongest reason Bicho's perception adapters matter to Squid.

# 3. Create: games from scratch

> *"Focusing on Objects (runes), Worlds (mantles), and networking/multiplayer
> (holiday)."*

The creation workflow is three windows, named by the author:
[Turkey, Table, Feast](/concepts/turkey-table-feast.md). In short:

- **Turkey** is where things are made: a character, an object, any rune, and how it
  presents itself (its glyph).
- **Table** is a world: a mantle with rules. Tags on turkeys, and rules over tags,
  in **Void Allomone**.
- **Feast** is the layout of the game over time and across people: the narrative
  order, the sessions, and the networking (holidays).

**What exists.** The pillars ([the three pillars](/design/the-three-pillars.md)), the
game model's nouns ([what a game is](/concepts/what-a-game-is.md)), Allomone's derive
half (built, terminating by theorem), Palabra's sync for multiplayer state, the
viewport and the spatial log. **What is missing:** the game document, the game model
target (S14), the studio surfaces, and **a write path for rules** (Allomone tier 1;
see [Turkey, Table, Feast](/concepts/turkey-table-feast.md) §3).

# 4. Simulate: let it play out

> *"Use void squid to simulate a car driving on the road … suppose we already have a
> world built, or maybe we have a decompiled game, we should be able to simulate things
> within it and let stuff play out. … when it comes to a virtual box, this is basically
> saying that you want a sandboxed instance of some system."*

**What simulation is, in the family's terms.** A simulation is an **artifact running
under observation** ([studio surfaces](/concepts/studio-surfaces.md) §3): the forge
emits a runnable instance, the instance steps its own clock, and the forge reads it and
logs what happened. The "virtual box" is the same thing with an operating system as the
artifact ([hosts](/concepts/hosts.md)). One shape, three scales: a car on a road, a
decompiled level, a whole OS.

*2026-10-10: decided and built as [simulation](/design/simulation.md) (option 2 below, multi-instance from the start).*

**The line that does not move.** Void Core and the forge do not step physics at frame
rate (railguard §6; Void Unity's two-clocks law). A simulation therefore needs a
**runtime** that steps. Two honest options, recorded as S17:

1. **An existing runtime** (Bevy over its remote protocol; a physics library inside a
   small Squid-owned runner), which the forge drives and observes.
2. **A model-rate simulation in the document**: discrete events, each a dispatcher
   command, at a few steps per second, which is enough for a traffic intersection or a
   D&D encounter and is fully logged and replayable for free. This is the DM's first
   job ([what a game is](/concepts/what-a-game-is.md) §2) run slowly, and it is the
   cheapest thing that makes "simulate a car" answerable today.

**What the spatial log adds.** A simulation's output is a log, and the log must not
grow with the world: each tick logs **what changed in view** and **what entered or left
a region**, not every object ([smart logs](/design/smart-logs.md) §5.3). An agent told
"simulate a car driving on the road" should be able to run it, read twenty lines, and
answer "did it reach the intersection?" without the world in its context.

**The asset library.** The author: *"we might need a LIBRARY of assets, or methods of
asset generation … a bunch of presets attached to some textual embedding model maybe."*
In family terms that is a **catalogue mantle of asset runes** (each a content-addressed
block, tags, a thumbnail) plus an **embedding model in Bicho** that turns "a red car" into
a vector and returns the nearest presets. Bicho's own founding already makes a model
catalogue a mantle selected by tag query; an asset catalogue is the same shape. Generation
(image-to-3D) is a later, heavier layer-3 adapter (SAM 3D, TripoSR; see
`VoidBicho:/references/3d-ai-landscape.md`), and its output enters as a proposal like any
other model output. Recorded as S18.

# 5. Order

The three uses share their first steps, so the order is by what unblocks the most:

1. **The game document** (S12) and **the game model target** (S14): every workspace
   needs somewhere to put turkeys and tables.
2. **Layer 2 of the spatial log** (the ID buffer): simulate and decompile both need to
   know what is actually visible.
3. **A model-rate simulation** of one tiny scene (a car on a road, as dispatcher events),
   logged with "what changed in view". The cheapest end-to-end proof of the simulate
   workspace, and it needs no physics library.
4. **One perception adapter in Bicho** that runs on this machine, enriching a look with
   labels. The first step of decompile.
5. v0's rung 3 (the DS build) continues in parallel; it is the create workspace's first
   real artifact.

Nothing here moves rung 3, and nothing here is built before the author rules on S14.
