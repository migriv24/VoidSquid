---
type: Design
title: The three pillars
description: The author's 2026-10-09 separation of Void Squid into three foundations — the game model, the studio surfaces, and the hosts — what each owns and must not own, the seams between them, how they package, and which of them the "no code before Jelly's rung 2.5" rule actually binds.
tags: [status:planned, audience:dev, confidence:exploratory, foundation]
timestamp: 2026-10-09T00:00:00Z
---

> The author, 2026-10-09: *"we may need to separate some foundational pillars... We may
> need to have separate packages, plugins, extensions, or whatever you may call it to
> define what we're even building."* This page is that separation. **Nothing is built;
> v0's scope does not move.** The packaging lean is S7 in
> [open questions](/design/open-questions.md); the author decides.

# 1. The three pillars, as the author named them

| # | pillar | the author's words | what it is in the family's terms |
|---|---|---|---|
| **1** | **the game model** | *"the 'game' definitions and architecture. how to add assets, modify, give logic"* | a **pure vocabulary** over Void Core: glyphs, tag axes, link kinds and rule shapes for what a game is ([what a game is](/concepts/what-a-game-is.md)) |
| **2** | **the studio** | *"the translation of that first layer into a GUI. assets, an inspector, a 'scene' view, a 'game' view"* | **projections** of a game document into surfaces, and gestures compiled back into dispatcher commands ([studio surfaces](/concepts/studio-surfaces.md)) |
| **3** | **the hosts** | *"run this software on desktop first, but also on Latin OS itself"* | the **effect list**, implemented once per platform ([hosts](/concepts/hosts.md)) |

The three are the Void Maiz architecture (`VoidMaiz:/concepts/views-as-projections.md`)
read at the scale of an engine: a model that owns truth (in Core), views that project it
and compile input back, and a host that performs effects. That is why they separate
cleanly: the seams already exist.

```
          ┌──────────────────────────────────────────────────────────┐
          │  pillar 3 · HOSTS      the effect list, once per platform │
          │  Windows desktop · LITO (Latin OS) · a build server       │
          │  files · processes · window · GL · input · clock · cache  │
          │  ┌────────────────────────────────────────────────────┐   │
          │  │  pillar 2 · STUDIO  project : document → surfaces  │   │
          │  │  hierarchy · inspector · assets · scene · game     │   │
          │  │                     compile : gesture → commands   │   │
          │  │  ┌──────────────────────────────────────────────┐  │   │
          │  │  │  pillar 1 · GAME MODEL   pure; no effects    │  │   │
          │  │  │  world · avatar · character · region · rule  │  │   │
          │  │  │  beat · cue · upgrade          (Void Core)   │  │   │
          │  │  └──────────────────────────────────────────────┘  │   │
          │  └────────────────────────────────────────────────────┘   │
          └──────────────────────────────────────────────────────────┘
                 computation the pillars call, never own:
                 Void Jelly (geometry) · Palabra (versions, store)
                 Allomone (derivation) · Bicho (models) · Loops (audio)
```

# 2. What each pillar owns, and must not

**Pillar 1, the game model.** Owns the nouns and their relations. Must not own: any
geometry (Jelly's), any pixel (the studio's), any effect (a host's), any mechanic in
motion (the runtime's). Its test is [what a game is](/concepts/what-a-game-is.md) §9:
stateable in Core's vocabulary, pure, recoverable from an artifact. In
[MDA](/sources/mda-framework.md)'s terms it holds **mechanics** as data and nothing of
dynamics.

**Pillar 2, the studio.** Owns the function pair `project` and `compile` for each
surface, and nothing else: no truth, no second data model, no camera variable that is not
`view.*` config in the document. Must not own: a definition of what a game *is* (that is
pillar 1, and a surface that needs a noun pillar 1 lacks has found a gap to report, not a
field to add), or any effect. "Scene" and "game" are abstract here by the author's
instruction: a scene is **any mantle with a projection**, a game is **any artifact
running under observation**. That is what lets the same studio open an operating-system
document.

**Pillar 3, the hosts.** Owns every effect: the list in
[build surface](/concepts/build-surface.md), extended by [hosts](/concepts/hosts.md) with
ingress toolchains and a hypervisor. Must not own: anything a unit test could run
without a filesystem, network, display or clock. A host is thin by construction, and
the test of the whole separation is that **the Windows host and the LITO host differ in
this pillar and in no other.**

# 3. The seams between them

| seam | shape | already exists as |
|---|---|---|
| pillar 1 ↔ pillar 2 | the studio projects a document whose glyphs pillar 1 declared; gestures become dispatcher commands in pillar 1's vocabulary | Maiz's `project_scene` in, command lines out (`VoidMaiz:/concepts/views-as-projections.md`, host-built views) |
| pillar 2 ↔ pillar 3 | the studio asks for effects by name; the host grants or refuses them per run | Maiz's effect handler and the headless `--allow-effects` discipline |
| pillar 1 ↔ the libraries | content by reference: a region's geometry is a block Jelly owns; a cue's audio is Loops Studio's; a character's history is Palabra's | Palabra content references; Jelly's `.ink` blocks |
| the forge ↔ the artifact | the forge emits; the artifact runs; the forge may *read* the running artifact and never dispatch into it per frame | Void Unity's two-clocks law; the Bevy Remote Protocol as a holiday |

The last row is the one that keeps the engine from becoming a runtime, and it is the
same line in all three pillars.

# 4. How they package

**Lean: three build targets in this repository, with a mechanical boundary check,
promoted to siblings when a second consumer appears.** Modelled on Void Jelly, whose
`tools/check_boundary.py` fails the build if a second translation unit includes
`voidcore.h`, and on Void Maiz, whose `voidmaiz` / `voidmaiz_view` / `voidmaiz_headless`
split is exactly pillars 1–3 for a node editor.

| target | pillar | links | may include |
|---|---|---|---|
| `squid_model` *(name provisional)* | 1 | `voidcore` | nothing with an effect; a boundary check enforces it |
| `squid_studio` | 2 | `squid_model`, `voidmaiz`, `voidmaiz_view`, `voidjelly` (for the scene view's draw list) | no `<fstream>`, no process spawn, no window creation |
| `squid_host_desktop`, later `squid_host_lito` | 3 | everything above, plus the platform | the effects, and only the effects |

Why not siblings now: a sibling is a repository, a manifest, a version and a message
channel, and pillar 1 has no second consumer yet. Why not one target: because the day
pillar 1 has a second consumer (LITO wanting the game vocabulary for its own system
document; a Bevy host wanting it as a crate) the split must already be mechanical, or it
will be a port. The precedent is Allomone, extracted from Maiz on 2026-08-29 after it was
already a clean unit inside it.

The author may prefer siblings from the start; that is S7.

# 5. What the "no code before rung 2.5" rule binds, pillar by pillar

The founding rule was written for one reason: *starting earlier would mean an
application with nothing to open.* Read against the pillars, it binds unevenly, and
saying so is a finding rather than a loophole:

- **Pillar 2 and the desktop host are v0.** They are the viewport, the nib picker and
  the build button, and they wait for Jelly's GL nib exactly as before.
- **Pillar 1 is pure and opens nothing.** It needs no nib, no window and no geometry;
  its tests are `(document, command) → document`. The founding rule does not bind it,
  but **the railguard's question still does**: it is not work that proves Jelly, so it
  is not work for *this* repository's v0 either. The honest position is that pillar 1
  can be *designed* now (this bundle is doing that) and *built* when the author says the
  charter has widened ([open questions](/design/open-questions.md) S6), whichever comes
  first of that ruling and the S6 gate.
- **The LITO host** waits for LITO to boot, and no page here can move that.

# 6. Where the earlier pages now sit

[Engine horizon](/design/engine-horizon.md) (2026-08-26) sketched *a game editor plus a
conventional runtime*; this page is that sketch given three names. [The forge for many
runtimes](/design/the-forge-for-many-runtimes.md) (2026-10-03) is pillar 3's build
surface seen from the ecosystem. Latin OS's *forge for many systems*
(`LatinOS:/design/the-os-as-a-game.md`) is pillar 2's abstraction of *scene* and *game*
taken at face value. None is superseded; each is one pillar seen before the others had
names.
