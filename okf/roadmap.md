---
type: Roadmap
title: Roadmap
description: Void Squid's rungs, interleaved with Void Jelly's, because neither half proves anything alone.
tags: [status:planned, audience:dev, confidence:asserted, roadmap]
timestamp: 2026-08-26T00:00:00Z
---

**0.1.0, the studio, is built** (2026-10-12; [Void Squid 0.1](/design/void-squid-0.1-the-studio.md)),
ahead of rung 3 by the author's direction (S6). It is rung 8's first slice over the asset
library rather than the game document; what remains of rung 8 is below, with the studio's
own next steps. **Rung 2.5 is built** (2026-10-09; see [log](/log.md)). Two pages carry `status:current`
([viewport](/concepts/viewport.md) and, for one row, [build surface](/concepts/build-surface.md));
the rest are `status:planned`.

Convention inherited from Void Core: **this file lists only what is not yet done.** When
something ships it moves to [log.md](/log.md) and out of here.

Rungs marked **[J]** are [Void Jelly](../../VoidJelly/okf/roadmap.md)'s, listed because
Void Squid has nothing to do without them — the numbering is shared deliberately, so one
conversation covers both.

# Where Void Squid enters

Void Jelly's rungs 0–2 (verify the DS constraint sheet, minimal `.ink` and its round-trip
lens, the Polyp capturing structure) need **no host**. They are pure library and pure
plugin, and Void Squid should not exist as code until rung 2.5.

Starting earlier would mean an application with nothing to open.

---

# Rung 2.5 — Open a `.ink` and see it

**[J]** the GL nib: `emit(ink, gl_profile) -> drawlist`.
**Squid:** a Void Maiz window, a viewport, a camera. Read the file, hand the bytes to Void
Jelly, draw what comes back.

**This is the first execution of `VoidJelly:/design/library-boundary.md`,** and the point
of doing it here is that the boundary stops being an argument and becomes a build. Expect
it to be awkward somewhere; **the awkwardness is the finding**, and it goes back to the
Jelly agent as a measured report rather than a feature request.

Also the control experiment: the GL residual should be nearly empty. If it is not, the loss
is in `.ink` and not in any target ([viewport](/concepts/viewport.md)).

Decide `VoidJelly:` §10 (does Jelly ship GL code?) first. Settle S3 (headless or GUI first)
and say S5 out loud (yes, it writes).

**Deliverable:** a spinning scene from a real `.ink`, and one report to Void Jelly.

**Status 2026-10-09: built, with the remaining slice stated precisely.** The window, the
viewport, the camera, `effect open`, the residual panel and the headless CLI exist and
run (`src/`). S3 was settled by doing both in one day with the headless smoke test as the
first test; S5 is said out loud: the camera is written to the document on every release.
§10 was *not* decided; the uploader is in this repo for now. **Left of this rung:** a
`.ink` born in Blender rather than from Jelly's `sample` verb (waits on the Polyp, Jelly's
rung 2), and the emission-cost measurement, which needs that scene.

---

# Rung 3 — The build button

**[J]** the DS nib, deliberately dumb: keep the first 2048 polygons, report the rest as
`dropped`, produce **bytes only**.
**Squid:** the [build surface](/concepts/build-surface.md) — write the bytes, find and
invoke devkitARM, package the `.nds`, launch the emulator, show the residual.

Plus **headless**: `maiz::run_cli`, so `squid build scene.ink --nib ds` works from a
terminal, with effects refused by default and `--dry-run-effects` available.

**Why `--dry-run-effects` matters more than it sounds:** because Void Jelly is pure, the
entire pipeline runs and produces a complete residual **without writing anything or needing
devkitARM installed**. "Will this fit on a DS?" becomes answerable in CI on a bare machine.
That property is a gift from the boundary and should be exercised early.

Decide `VoidJelly:` §11 (who owns the DS host program?) before starting.

**Deliverable:** an emulator screenshot of a scene authored in Blender, and a residual
explaining everything missing from it. **This is v0's exit condition.**

---

# Rung 4 — More nibs, no new Squid

**[J]** charts, the oracle, the voxel nib.
**Squid:** the nib picker gains two entries and nothing else changes.

**That "nothing else changes" is the test.** If adding a target whose primitive is not a
triangle requires application work, the [build surface](/concepts/build-surface.md) is
wrong. This rung costs Void Squid almost nothing *if the boundary is right*, and it is
worth stating in advance so the cost is measured rather than absorbed.

---

# Rung 5 — Budgets become adjustable

**[J]** decimation with a cost curve, the greedy allocator.
**Squid:** a budget field, an importance override, a re-build.

The first rung where Void Squid does something a person would call *editing* — and note
what it is: editing **structure and policy**, never geometry
([railguard](/design/what-voidsquid-is-not.md) §3). Both are Void Core tags and mantle
content, dispatched like everything else.

---

# Rung 6 — Residuals as a corpus

**[J]** defines the residual key.
**Squid:** store residuals by `.ink` hash, diff two builds, paint the losses onto the
viewport, compare two nibs over one scene.

The rung where a residual stops being a report and becomes the thing the project is about.
Settle S2 (where build settings live) first — the diff depends on it. S4 (how much the GUI
shows) is answered here.

---

# After v0

**Only then** is [engine horizon](/design/engine-horizon.md) worth a real conversation, and
by then it will be a conversation with evidence in it. Nothing before rung 6 should be
justified by a gameplay feature.

*2026-10-09: the conversation's agenda is written in advance.* The rungs below are the
[three pillars](/design/the-three-pillars.md) in build order. They are unscheduled, they
wait on S6 (the charter) or the author's ruling (S14), and they are listed so that v0's
choices are made with them in view. **None moves rung 2.5 or 3.**

---

# Rung 7 — The game model, pure *(pillar 1)*

A build target with no effects and a boundary check: the glyphs, tag axes and link kinds
of [what a game is](/concepts/what-a-game-is.md), as a Void Core domain module, tested
as `(document, command) → document`. Its first fixture is a **character sheet** and an
**upgrade graph**, because they are the oldest instances and the easiest to be wrong
about. Settle S9 (glyphs, whose namespace) and S12 (the game document) first.

**Falsifies:** that a game's non-geometric structure can be said in Core's vocabulary at
all. **Deliverable:** a game document with no scene in it that `ls`, `tag` and `link`
can work on, headless, and a query that answers *"what does this game say it means to be
better?"*

---

# Rung 8 — The studio surfaces *(pillar 2)*

**First slice shipped as 0.1** (the log, 2026-10-12): the shell, the inspector, the Data
table and the asset library, as projections of one Core document. Still to come for the
studio, in the order the author's asks make most useful:

0. **Done the evening of 2026-10-13**: Load a game in one button with downloads and loading bars;
   the Games window; motion through Void Onda, judged from pixels; Donkey Kong 64's Kongs.
1. **See a character move in the studio**: Jelly reads glTF and the studio draws turkeys and
   tables in 3D (2026-10-13), but `.ink` has no skins (S32), so characters stand in bind pose.
2. **The pilot's proposals land in Turkey** (S29): **built 2026-10-14** (Accept / Not this on the
   card, Ask for one turkey). Next: cases with no English name, and the progress report counting
   accepted proposals (it reads the library, not the project's decisions yet).
3. **The ingest routes move in from the exercise's toolbox**: Load a game runs them from the
   studio now (2026-10-13), but the routes are still the exercise's Python.
4. **Donkey Kong 64's animations** (table 11), replacing Onda's synthesized gait; its music as MIDI
   (S36).
5. **The runtime and export** (S30): Play without the exercise; a game as a Windows `.exe`
   and a Linux binary.
6. **Collaborative editing switched on** through Void Maiz's networking (Coral stays for play).
7. **A Linux machine runs it** in front of a person.
8. **Motion in the studio** (S34 (b); ocean: added 2026-10-18): `effect motion judge|report`
   **built 2026-10-18**. Next: a Motion window over the report; the gallery on one clip (a runtime
   flag first). Void Ocean stays unlinked until S30 (S33); its gravity seam is measured
   ([motion and physics in the studio](/design/motion-and-physics-in-the-studio.md)).

The rest of the rung, as first written:

Hierarchy, inspector and assets over the game document, as `project` / `compile` pairs
([studio surfaces](/concepts/studio-surfaces.md)), beside v0's scene view and residual
panel. Then the **game view**: an artifact observed, first an emulator, then a Bevy
process over the Bevy Remote Protocol.

**Falsifies:** that every surface can be a projection with no truth of its own, and that
*play mode is not a mode*. **Deliverable:** edit a stat in the inspector, re-emit, see the
running artifact pick it up, with the transcript showing exactly one command.

---

# Rung 9 — The first ingress *(construct and deconstruct)*

An **ingress toolchain** row in the build surface, REA first (S10), with a pure reader
that turns its evidence and limitations into a game document and a stored ingress
residual. The target is a game nobody in this family made.

**Falsifies:** the game model's nouns, by the third test in railguard §8: a noun no
reader can recover is an editor convenience. **Deliverable:** an existing game's upgrade
graph, recovered, with a residual naming what was not.

*2026-10-10: split by depth* ([decompilation](/design/decompilation.md) §11; the lean in
S20, the author decides). Stage zero, recon, is built and moved to the [log](/log.md).
What is left, in order:

- **9a:** recon on real dumps. *Done for the DS on 2026-10-10 (the [DS mashup exercise](/design/ds-mashup-exercise.md)); other consoles wait for dumps.*
- **9b, unblocked:** the ingress toolchain row, invoked once under the gate: `ndstool` on a
  `.nds`. Best once rung 3 exists, so the first `.nds` read back is one our own nib wrote.
- **9c, needs Void Jelly:** a reader to the bytes depth: a DS cartridge's file system and
  meshes into an assetless `.ink` with a residual. *A working Python reader exists in
  `../ds-mashup/tools/` and was offered to Jelly as the DS nib's inverse (message, 2026-10-10).*
- **9d, needs Void Bicho** (Phase 0 and 1, and a `classify` kind): identify, by hand, in the
  studio, with proposals and acceptances in the transcript.
- **9e, needs the game model (S14):** lift to structure; the upgrade-graph deliverable above.

---

# Rung 10 — The LITO host *(pillar 3; no date)*

Void Squid as a Void window on Latin OS: the effect list reimplemented over capabilities,
the surfaces placed by the mesh compositor, the build surface spread over the mesh
([hosts](/concepts/hosts.md)). Then the recursion: open an operating-system document,
emit it through a device nib, run it under a hypervisor as the game view.

**Falsifies:** that the Windows host and the LITO host differ in pillar 3 and in no
other. **Deliverable:** one transcript, two hosts, one document, byte for byte. Waits on
LITO booting and on Maiz's Q23.

---

# What is deliberately NOT on this roadmap for v0

Input, physics, collision, audio, animation, scripting, entity systems, prefabs, play mode,
a scene editor, a material editor, an asset browser.

Every one belongs to a game engine and none belongs to a harness. Absence from rungs 2.5–6
is a decision, and the reasoning is [the railguard](/design/what-voidsquid-is-not.md).
Several reappear above, in order, after the charter widens; the ones that do not (physics,
collision, input, animation in motion) are **the runtime's**, in its own language, and
never arrive here at all.
