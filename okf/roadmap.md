---
type: Roadmap
title: Roadmap
description: Void Squid's rungs, interleaved with Void Jelly's, because neither half proves anything alone.
tags: [status:planned, audience:dev, confidence:asserted, roadmap]
timestamp: 2026-08-26T00:00:00Z
---

**Nothing is built.** Every page in this bundle is `status:planned`.

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

# What is deliberately NOT on this roadmap

Input, physics, collision, audio, animation, scripting, entity systems, prefabs, play mode,
a scene editor, a material editor, an asset browser.

Every one belongs to a game engine and none belongs to a harness. Absence here is a
decision, and the reasoning is [the railguard](/design/what-voidsquid-is-not.md).
