---
type: Concept
title: Viewport
description: Opening a .ink and showing it — the most-exercised path in the whole system, and the control experiment that tells us where a bug lives.
tags: [status:planned, audience:dev, confidence:asserted]
timestamp: 2026-08-26T00:00:00Z
---

The author's description: *"void squid will be able to open a `.ink` file normally,
basically like blender would. (just a 3d scene in basic opengl right now, not interactive
abilities yet)."*

Deliberately unambitious, and more load-bearing than it looks.

# It is the control experiment

The GL nib's [residual](../../VoidJelly/okf/concepts/residual.md) should be **nearly
empty**, because an unconstrained target refuses almost nothing. Which makes the viewport a
diagnostic instrument rather than a feature:

> **If the viewport looks wrong, the loss is in the `.ink`, not in any target.**

Without it, a mangled normal or a dropped UV map shows up in a DS build
indistinguishably from the forty things the DS legitimately could not carry, and a week
goes into blaming the wrong stage. `VoidJelly:/concepts/nibs/trivial.md` makes the same
argument from the library's side; this is the half that actually runs.

It is also, by a wide margin, **the most-exercised emitter in the system** — every frame,
against every scene anyone opens, where the DS and voxel nibs run only when someone asks.
Bugs surface here first, by volume.

# What it renders, and what renders it

The split follows `VoidJelly:/design/library-boundary.md` exactly:

- **Void Jelly** emits a **draw list**: buffers, indices, materials, transforms, draw calls
   — as data.
- **Void Squid** owns the window, the GL context, the frame loop, the camera, and issues
  the calls.

Whether Void Jelly also ships an optional, quarantined `voidjelly_gl` that issues those
calls into a **host-created** context is `VoidJelly:/design/open-questions.md` §10, and the
lean there is yes, mirroring `voidmaiz_view`. Either way **Void Squid creates the window
and Void Jelly never does.**

# What it is not

- **Not an editor.** No selection, no gizmos, no dragging objects, no inspector. Those are
  the parts of Blender we deliberately do not rebuild
  ([railguard](/design/what-voidsquid-is-not.md)).
- **Not a renderer with features.** No shadows, no post-processing, no PBR. Flat lit
  geometry. The point is *"is this the scene?"*, not *"is this beautiful?"*.
- **Not the definition of correct.** If the viewport and a blot disagree, **the blot is
  right.** A preview that becomes the reference is how a pipeline quietly acquires two
  truths.

# Camera state is model content

Void Maiz has already ruled on this and the ruling transfers without modification: view
state — camera position, zoom — is **model content dispatched through the core**, not a
private variable, and it lives in the undo-exempt `view.*` config tier
(`VoidMaiz:/concepts/views-as-projections.md`).

Maiz's `Camera` is three floats and deliberately *substrate-shaped*, which is why it
already served a geographic map without changing. A 3D camera needs more than three floats,
and this is the first client that does — worth a **message to the Void Maiz agent** once we
know precisely what the shape wants to be, rather than guessing now. Maiz's own substrates
page anticipates it: `[x,y,z]` is *"a compatible extension the day a 3D client exists."*
Void Squid is that day.

# The question it will force first

**Is emission cheap enough to sit in an interactive loop?**

A frame needs a draw list. If `emit(ink, gl_profile)` is a full scene walk, that is fine at
30 objects and not fine at 3000. The answers, in the order they should be tried:

1. **Cache the draw list**, and re-emit only when the `.ink` changes. Sufficient for v0,
   where nothing edits the scene from inside Void Squid.
2. **Incremental emission**, keyed by content hash — only the objects whose geometry blocks
   changed. This is where `.ink`'s content-addressing pays off in the interactive case
   rather than only the storage case.
3. Something cleverer, only if measurement demands it.

Deliberately not designed now. **Measure first** — this is precisely the class of question
the client was founded early to answer with numbers instead of opinions.

# Status

`status:planned`. Nothing built. Rung 2.5, jointly with Void Jelly's GL nib — neither half
proves anything alone.
