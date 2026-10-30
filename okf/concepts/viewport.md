---
type: Concept
title: Viewport
description: Opening a .ink and showing it — the most-exercised path in the whole system, and the control experiment that tells us where a bug lives.
resource: src/viewport.cpp
tags: [status:current, audience:dev, confidence:asserted]
timestamp: 2026-08-26T00:00:00Z
---

> **Built 2026-10-09** (`src/viewport.cpp`, `src/main.cpp`): a Void Maiz window opens a
> `.ink`, Void Jelly's GL nib emits a draw list, the host draws it with fixed-function GL
> as the window's background and floats the Scene, Residual and Log panels over it.
> Orbit, pan and zoom; the camera is `config view.camera3d` in the document, flushed as
> one command on release. The sample scene draws as expected and its residual reads
> **fits, 18 entries, 0 dropped**. What was measured and what was found is in the
> [log](/log.md). The interactive-emission question below is **not yet measured**: the
> sample scene is too small to say anything, so the list is emitted once per open.

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

# What the first cut is, honestly

Fixed-function GL 1.1 (client-side arrays, `glDrawElements`, one directional light, no
loader, no shader) against the GL 3.0 compatibility context Maiz asks for on Windows.
Zero vendored GL code, which is the right size for *"is this the scene?"* and the wrong
size for macOS, where Maiz asks for a 3.2 core context and none of this runs. That is a
measured argument for Jelly's open question §10 lean (b), a quarantined `voidjelly_gl`
owning the GL3 path, and the day it moves, this file shrinks to a camera and a call.

The camera is six numbers under `view.camera3d`, with this host's own serialise and
parse, because Maiz's `Camera` is three floats and its `compile_camera` cannot carry a
3D orbit. Reported to Maiz (`../VoidMaiz/MESSAGE_FOR_VOIDMAIZ_voidsquid-...-2026-10-09.md`).

# Status

`status:current` (`resource: src/viewport.cpp`). Rung 2.5, jointly with Void Jelly's GL
nib, which is also built. The emission-cost measurement waits for a scene with thousands
of objects, which waits for the Polyp.
