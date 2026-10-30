---
type: Design
title: The forge for many runtimes
description: Void Squid's purpose in the Rust/Bevy game-rewrite ecosystem and in the Void family, read through the author's 2026-10-03 direction; the mashup workbench, Bevy as a runtime it pours into, and why none of this changes v0.
tags: [status:planned, audience:dev, confidence:exploratory]
timestamp: 2026-10-03T00:00:00Z
---

> **Nothing here is being built, and v0's scope does not move.** This page records a
> direction the author set on 2026-10-03, and works out what it means using the claims the
> bundle already makes. The scope in force is still [scope](/concepts/scope.md) and
> [the railguard](/design/what-voidsquid-is-not.md). Void Jelly's side is
> `VoidJelly:/design/rust-and-bevy.md`.

# What the author said

> *"eventually [Void Squid] will be the engine to tie everything together, combine it all
> into 1 thing, and be the ultimate generalizable game creation tool."*

It came with news about the open-source scene. People are rewriting old games in **Rust on
Bevy** over assets they already own, then **combining them**: one executable that is MW2's
multiplayer, Skate 3's skating and a Minecraft world. The author named five siblings Squid
could one day tie together: Void Allomone, Void Chisme, Void Palabra, Void Verguenza and
Void Loops Studio.

# The claim this bundle already makes, and why it fits

[Engine horizon](/design/engine-horizon.md) opens with one structural claim:

> **Void Core's contribution to a game engine is the editor, not the runtime.**

The rewrite ecosystem is that claim playing out in public. The community is producing
**runtimes**: Bevy apps, one per rewritten game, mashed together through ECS. What it does
not have is a **forge**. Bevy 0.19 has no editor (`VoidJelly:/sources/bevy-0-19.md`), and
every rewrite invents its own asset IR. So "the ultimate generalizable game creation tool"
does not need to be read as *a runtime that runs everything*. Read through this bundle, it
means **one forge that can pour into many runtimes**: a Bevy app, a DS `.nds`, and Void
Unity's Unity.

That reading keeps every rule this bundle already has, and it puts Squid where nobody else
is standing.

# What the forge does, in this ecosystem

Each row is one verb in the shape of the [three verbs](/index.md). Squid performs the
effects, and a library does the computation.

| what the forge does | the effect (Squid's) | the computation (whose) |
|---|---|---|
| **gather** scenes from many producers: Blender through the Polyp, a community reader that turns a game's files into ink, another mod's `.ink` | read files, run a reader the player installed | `.ink` validate and inspect: **Void Jelly** |
| **compose** them into one mashup | none; it is all dispatch | structure join: **Void Palabra**. Rule and property merge, so two mods disagreeing is a **named conflict, not a load order**: **Void Allomone** |
| **see what did not fit**, per target | show it | the residual (`VoidJelly:/concepts/residual.md`), per nib: **Void Jelly** |
| **build** to a runtime | write bytes, invoke the toolchain (devkitARM, `cargo`) | `emit(ink, profile)`: **Void Jelly** (the DS nib, the Bevy nib) |
| **live-edit** a running Bevy game | talk to the game over the **Bevy Remote Protocol**, JSON-RPC: a **holiday** in Void Core's exact sense | re-emit what changed: **Void Jelly** (incremental emission, already a seam v0 must keep) |
| **share** the workbench with collaborators (*"multi user on single instance, tracking and version history"*) | the transport | history, joins, sync: **Void Palabra** |
| **administer** the shared server: who may wipe the world, who holds the mod-signing key | prompt the approvers | *k*-of-*n* consent, sealed secrets: **Void Verguenza** |
| **diagnose** the LAN party or the self-hosted server | the sensor | device and traffic analysis: **Void Chisme** |
| *later:* score the game with music that reacts to model-rate events | the audio device | the node-graph audio model: **Void Loops Studio**, *"not at all ready for integration yet"* (the author) |

And because Squid is a Void Maiz application, **every one of those rows is a dispatcher
command.** That ties back to the author's first point, that *"llms and ai in general is
making programming easier and more accessible to EVERYONE."* A forge whose every gesture
is a visible, replayable command is a forge an agent can drive. Bevy's community already
wants that: `bevy_brp_mcp` exists to let coding agents poke running Bevy games. Squid
would give agents the *authoring* side.

# The two lines that do not move

**1. Squid stays C++ on Void Maiz, with no Rust half.** It reaches the Rust world through
**files** (`.ink`, and the Bevy nib's spawn table) and through **BRP** (a protocol). Linking
Rust into Squid would make it a Bevy app, and then it would be a runtime.
`VoidJelly:/design/open-questions.md` §18 leans the same way: the Bevy-side plugin belongs to
a Bevy host, possibly a future *Void Bevy* sibling on Void Unity's pattern, and never to
Squid.

**2. Squid composes structure; it never composes mechanics.** The mashup composes skating
and gunplay as ECS systems, at frame rate, inside Bevy. That is the runtime's job, and Bevy
is good at it. What Squid adds sits one level up and at model rate: **which** mods are on,
**what each claims** about the world, and **where they disagree**. Void Unity's founding
law covers it: *"a frame may read the model; a frame may never dispatch into it."*

# Why this makes the harness *more* important, not less

The ecosystem moves fast. Bevy breaks its API roughly every four months, and the
rewrites are weeks old. The temptation will be to chase it: a Bevy viewport here, a mod
loader there. [Engine horizon](/design/engine-horizon.md) already names the trap: *"while we
are here, this would be easy."*

The scarce thing in that ecosystem is not one more runtime. It is **a pipeline that
provably works**: one `.ink`, two targets, two correct residuals. That is precisely what
v0 exists to prove. **The forge gets built after Void Jelly emits.** It will matter to the
rewrite community for the same reason it matters to us: by then there will be something
under it.

# What this adds to the seams v0 must not foreclose

[Engine horizon](/design/engine-horizon.md) names three seams. This direction adds no new
build work. It adds one constraint on the existing ones:

- **Seam 2 (cheap or incremental emission)** now has a second customer: live-edit over BRP.
  A full re-emit per edit is no more viable for a running Bevy game than for the viewport.
- **Squid must not assume one target per session.** The forge's whole value is one scene
  feeding many runtimes, so the [build surface](/concepts/build-surface.md) should treat
  "which nib" as data, not as a mode. v0 wants this anyway for the GL and DS nibs.

# The horizon after runtimes: systems *(added 2026-10-09, at Latin OS's request)*

Latin OS reframed itself on 2026-10-09 as a constructed artifact with Void Squid as its
forge (`LatinOS:/design/the-os-as-a-game.md`): the same four verbs (gather, compose, see
the residual, build) over an **operating-system document**, with LITO as a build target
beside Bevy and the DS, and *a device is a nib*. Recorded here as the horizon **after**
runtimes, in that order, and **it waits on S6 like everything else**: an OS forge on an
unproven emission pipeline is the while-we-are-here trap at its largest, which is Latin
OS's own reading too. The author's "virtual box" is the game view
([studio surfaces](/concepts/studio-surfaces.md) §3) open on LITO under a hypervisor, with
Squid as the console; no emulator is written. What it changes in the railguard is §6, as
restated ([open questions](/design/open-questions.md) S8).

**ECS projection, in one sentence.** A mantle is an entity-component system with a
confluent scheduler ([ECS sources](/sources/ecs-dungeon-siege-overwatch.md)), so a Bevy
world is one *projection* of a mantle, which is what the Bevy nib already emits; projecting
one game's systems into another's across dimensionality (*Master Chief into Link's
Awakening*) is a nib with a residual plus a translation holiday for the parts that are not
geometry. Nothing new is built to say it; what is needed is the nibs.

# Open

Recorded as [open questions](/design/open-questions.md) S6: *when does Squid's charter
widen from harness to forge?* The lean is a test, not a date. The shape of the forge once
it widens is [the three pillars](/design/the-three-pillars.md).
