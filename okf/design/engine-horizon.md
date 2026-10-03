---
type: Design
title: Engine horizon
description: The actual game engine, later — what a Void Core game engine would and would not be, and the three seams v0 must not foreclose.
tags: [status:planned, audience:dev, confidence:exploratory]
timestamp: 2026-08-26T00:00:00Z
---

> **Nothing here is being designed or built.** This page exists so the direction is not
> lost and so no v0 decision quietly forecloses it. `confidence:exploratory` throughout —
> a sketch, not a position. The scope that *is* in force is
> [scope](/concepts/scope.md) and [the railguard](/design/what-voidsquid-is-not.md).

# The one structural claim

Void Core's forge-not-artifact principle decides the shape, and it is not the obvious one:

> **Void Core's contribution to a game engine is the editor, not the runtime.**

`VoidCore:/design/domains-and-guarantees.md` works this exact example — *"the running game
the player plays"* is the artifact; *"the level / dialogue / asset editor"* is the forge.

So a "Void Core game engine" is really **a Void Core game *editor* plus a conventional
runtime**, and the interesting engineering is the seam between them. Void Squid v0 is
already an instance of that seam for one aspect — the visual one — which is why the author's
instinct that the renderer is *"fundamental and first"* is right. The visual aspect has the
hardest constraints and the least room to fake it.

**2026-10-03: the author's direction makes this claim the plan, not a sketch.** Squid
*"will be the engine to tie everything together."* Read with the claim above, that means a
forge that pours into many runtimes, including the Rust/Bevy ones the open-source scene is
now building. Worked out in [the forge for many runtimes](/design/the-forge-for-many-runtimes.md).

# What the runtime would and would not be

**Would:** eat a blot and some data tables; run a loop; do input, physics, collision, audio;
be written in whatever the platform demands (C on the DS, and a *different* runtime per
platform is fine — they share data, not code).

**Would not:** contain Void Core. No runes, no tags, no dispatcher, no rule interpreter on
the target ([railguard](/design/what-voidsquid-is-not.md) §6).

Authored rules compile to **inert data the runtime reads in its own way**. A trigger volume
becomes a row in a table, not a net to reduce. The rule is authored in the vocabulary of
the forge and *lands* in the vocabulary of the artifact.

# The three seams v0 must not foreclose

Cheap disciplines, and the test that they are not speculative work is that **v0 wants each
of them anyway**:

1. **Non-visual runes must be expressible.** A trigger volume, a spawn point, a waypoint, a
   sound emitter — objects a [nib](../../VoidJelly/okf/concepts/nib.md) mostly *ignores*.
   Void Jelly's glyph set must not assume every rune is drawable. It needs this regardless:
   a camera is exactly that shape.

2. **Emission must be cheap or incremental.** Iteration speed is the entire value
   proposition of a data-driven pipeline. The [viewport](/concepts/viewport.md) forces this
   at rung 2.5, long before any game needs it.

3. **Structure must survive to the target in some form.** The blot is dead, but a runtime
   needs to know which display list is the crate and which crate the player can push. So a
   blot carries a **symbol table** — stable names from rune names, mapping to offsets. Small,
   dumb, static; not a runtime. v0 wants it anyway, so a
   [residual](../../VoidJelly/okf/concepts/residual.md) can be traced back to something the
   artist recognises.

# What is genuinely unresolved, and stays that way

- **Where does gameplay authoring live?** In Blender alongside the scene (the Polyp already
  goes there), in a separate Void Maiz surface over the same mantle, or in text? All three
  are defensible; nothing forces a choice.
- **What is the rule vocabulary?** Void Core's `rules` are interaction-net rewrite rules
  over *structure*, which is not obviously the right shape for *"when the player touches
  this, open that."* It might be a good fit; it might be a category error. Worth finding out
  before committing, and not now.
- **Does 3D platformer gameplay on a DS want anything unusual from the pipeline?** The
  author's stated goal is 3D platformers. Collision geometry (`role:collision`, never drawn,
  excluded from the polygon budget) is the one thing already anticipated in
  `VoidJelly:/concepts/budget.md`. Whether more is needed is unknown.
- **One engine, or one per target?** The DS runtime and a desktop runtime share almost
  nothing except the blot format. Pretending otherwise is how cross-platform engines get
  their worst abstractions.
  *(2026-10-03: the Rust/Bevy ecosystem supplies an answer from outside. Desktop runtimes
  increasingly **are** Bevy apps, and Squid does not need to write one. It needs a nib that
  feeds one. See [the forge for many runtimes](/design/the-forge-for-many-runtimes.md).)*

None of these blocks anything in v0, which is why this page can stop here.

# The trap worth naming

The pull will not be *"let us abandon the harness and build an engine."* It will be
*"while we are here, this would be easy."* A physics step is a weekend. An input map is an
afternoon. Each is genuinely small, and together they are how the library ends up with one
host that is a game engine and a shape that fits nothing else.

The discipline that prevents it is already written down in two places, and both are one
question:

- [railguard](/design/what-voidsquid-is-not.md) §7 — *am I building this because Void Jelly
  needs it proved, or because it would be fun?*
- `VoidJelly:/design/client-void-squid.md` — *would a Blender in-viewport preview want this
  too?*

**The engine gets built after Void Jelly works**, and it will be a better engine for having
a proven pipeline underneath it.
