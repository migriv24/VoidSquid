---
type: Concept
title: Studio surfaces
description: The second pillar — the hierarchy, inspector, assets, scene view, game view and console that every engine has, each as a projection of one document and a compiler of gestures into commands. Why "scene" and "game" are abstract here, and why every surface must be describable as a mantle rather than as pixels if Void Squid is to run on Latin OS.
tags: [status:planned, audience:dev, confidence:exploratory]
timestamp: 2026-10-09T00:00:00Z
---

> **Nothing here is built, and none of it is v0.** v0's surface is one screen
> ([scope](/concepts/scope.md)): a viewport, a nib picker, a build button, a residual
> panel. This page says what the surfaces become after the charter widens
> ([open questions](/design/open-questions.md) S6), so that v0's one screen is the first
> instance of the pattern and not a dead end.

# 1. One rule, inherited

Void Maiz owns no truth, so a view is a function pair
(`VoidMaiz:/concepts/views-as-projections.md`):

```
project : document → surface      (what to draw)
compile : gesture  → commands     (what the user meant)
```

Every surface below is that pair over **the same document**. There is no editor state
that is not either a projection or `view.*` config content in the document. That is the
whole architecture of pillar 2, and the reason it is one page.

# 2. The surfaces an engine has, as projections

| surface (Unity's name) | projects | compiles | Maiz already has |
|---|---|---|---|
| **hierarchy** | the mantle's structure: runes, their `located-in` and parent links, filtered by tag | select, reparent, rename, tag | the tree/outline view, named as a candidate |
| **inspector** | one rune: its glyph's fields, content, quantities, links | edit a field; stage locally, flush as one command on release | `draw_inspector` |
| **assets** | the content-addressed store: every block, who references it, its provenance | import (an effect), replace, dedupe | nothing yet; Palabra's store is the model |
| **scene view** | a mantle with places, drawn: `emit(ink, gl_profile)` through Jelly, into a host-built view | fly, pick (`(scene, ray) → (surface, u, v)`, pure, Jelly's), move a rune | v0's viewport, exactly |
| **game view** | **an artifact running**, shown as a surface: an emulator's framebuffer, a Bevy process observed over the Bevy Remote Protocol, LITO under a hypervisor | nothing into the artifact at frame rate; commands go to the *document*, and a re-emit follows | nothing; this is the one genuinely new surface |
| **console / transcript** | the dispatcher's log: every command, every reply, every effect granted or refused | type a command | the log strip; the CLI transcript *is* this view |
| **residual panel** | what the current nib could not hold, grouped by cause, painted onto the scene view on request | pick a loss, see its rune | v0, S4 |

Two rows carry the author's instruction that *scene* and *game* be very abstract.

# 3. "Scene" and "game", abstractly

**A scene is any mantle with a projection.** An `.ink` is one: runes with places and
geometry, projected by a nib. But a mantle with no geometry at all has projections too
(the hierarchy is one), and an operating system's *plan* mantle
(`LatinOS:/design/the-os-as-a-game.md` §2) is a scene the moment something can draw it.
The scene view does not ask what kind of thing it is drawing; it asks for a projection.

**A game is any artifact running under observation.** The forge emitted it; it runs in
its own process, its own clock, its own language; the forge may *read* it (a
framebuffer, a protocol, a serial console) and may never dispatch into it per frame. That
is Void Unity's two-clocks law (*a frame may read the model; a frame may never dispatch
into it*) seen from the forge's side. Under that definition a DS game in an emulator, a
Bevy game over BRP and **LITO under a hypervisor with Squid as its console** are the
same surface with different holidays behind it, which is what Latin OS's "virtual box"
request amounts to, and why it needs no emulator written here.

Play mode, then, is not a mode. It is the game view open on an artifact built from the
current document, and *"edit while playing"* is: dispatch to the document, re-emit
incrementally, the artifact reloads what changed. The incremental re-emission is Jelly's
seam 2, which v0 already forces.

# 4. Why every surface must be a mantle, not pixels

On a desktop host the studio draws into a window and nobody asks how. On Latin OS the
unit of display is a mantle and **Void windows never send pixels**
(`LatinOS:/design/mesh-compositor.md`). A studio whose surfaces were only pixel
routines could not run there as a native Void window at all; it would be a foreign
application in a surface, like a GTK app.

The function-pair rule is what makes the studio portable without a port: a surface that
is `project : document → surface` is already a description the compositor can place,
and the host decides whether "surface" means ImGui draw lists in a GL window or a rune
in LITO's compositor scene. This is the concrete reason pillar 2 must not own pixels,
and it is the reason [hosts](/concepts/hosts.md) is a separate pillar rather than a
build flag.

# 5. What the studio must not own

- **A definition of a game.** The glyphs, tag axes and link kinds are pillar 1's
  ([what a game is](/concepts/what-a-game-is.md)). A surface that needs a noun pillar 1
  lacks has found a gap to report there, not a field to add here.
- **Geometry.** The scene view hands Jelly's draw list to a GL context the host created.
  No loop over vertices ([railguard](/design/what-voidsquid-is-not.md) §3).
- **Truth.** Selection, camera, panel layout: `view.*` content in the document, undo-exempt,
  dispatched like everything else.
- **Effects.** Import, save, build, launch, read-a-binary: asked for by name, granted by
  the host per run.

# 6. What v0 proves for this pillar

v0's one screen is the scene view, the residual panel, the console (headless) and a
nib picker. If those four obey the function-pair rule and the host test in §4, the other
surfaces are more of the same. If they do not, this page is wrong and should be rewritten
with the evidence.
