---
type: Design
title: Open questions
description: What is genuinely undecided in Void Squid, with a lean and what each blocks. The author decides.
tags: [status:planned, audience:dev, confidence:asserted]
timestamp: 2026-08-26T00:00:00Z
---

Convention inherited from Void Maiz: **the author decides.** Each question carries a lean,
and states **what it blocks** — a question that blocks nothing can wait.

Void Jelly's questions live in its own bundle (`VoidJelly:/design/open-questions.md`).
Three of them block work here: **§10** (does Jelly ship GL code?) blocks the
[viewport](/concepts/viewport.md); **§11** (who owns the DS host program?) blocks the
[build surface](/concepts/build-surface.md); **§5** (Jelly's language) settles this
bundle's §1 below.

---

## S1 — Does Void Squid need its own Void Core glyphs?

A `.ink` arrives with Void Jelly's glyphs (`sq-mesh`, `sq-light`, `sq-camera`,
`sq-anchor`). Does Void Squid register any of its own?

**Lean: no, not in v0.** Every glyph Void Squid registers is a piece of vocabulary that
exists only in the application, which means a `.ink` opened elsewhere loses meaning. If
something needs saying about a scene, it should be sayable in Void Jelly's vocabulary — or
it is a gap in Void Jelly worth reporting rather than routing around.

The first honest exception will arrive with gameplay
([engine horizon](/design/engine-horizon.md)), and by then there will be evidence.

**Blocks:** nothing. Recorded because "just add a glyph" is the easiest boundary violation
available.

---

## S2 — Does Void Squid have a project concept, or only files?

Is there a "Void Squid project" (a `.ink` plus build settings plus target config), or does
it just open a `.ink` and take options on the command line?

**Options.** (a) Files only — open a `.ink`, pass `--nib ds`. (b) A project file holding
build settings. (c) Build settings live **inside the `.ink`**, as ordinary mantle content.

**Lean: (a) for v0, (c) as the destination.** (b) is a second file to keep in sync and a
second thing to version. (c) is more interesting than it looks: *"this scene targets the DS
with a 1800-polygon budget"* is a fact about the scene, it is exactly the kind of thing
Void Core's tags and rune content exist to hold, and putting it there means the build is
reproducible from the `.ink` alone. That is what makes a residual keyed by `.ink` hash
meaningful.

The caveat: it puts application policy into a library-owned format, so it needs Void Jelly's
agreement — a good first message to send them, once there is evidence rather than a
preference.

**Blocks:** nothing at rung 2.5. Decide before rung 6 (residuals as a corpus), which
depends on the answer.

---

## S3 — GUI first or headless first?

Void Maiz gives both, and building both at once is not the same as building both well.

**Lean: headless first, by a week or two.** Reasons: it is the smaller surface; it is what
CI and an agent use; a regression suite over residuals — the thing Void Jelly most needs —
wants a command line and not a window. And Maiz's own headless adoption path (~40 lines) is
easier to walk **toward** than to retrofit.

The counter-argument is real and worth stating: the [viewport](/concepts/viewport.md) is the
control experiment, and it exists only in the GUI. So the honest sequence is *headless
`build` first, GUI `view` immediately after* — not one long headless phase.

**Blocks:** rung 2.5's shape.

---

## S4 — How much of the residual does the GUI show?

A [residual](../../VoidJelly/okf/concepts/residual.md) for a large scene may have thousands
of entries, and Void Jelly's ASCII nib exists specifically to find out whether that drowns.

**Options.** (a) A flat table. (b) Grouped by cause (budget / not representable / quantised),
counts first, drill down. (c) Painted onto the viewport — click a crushed object and see
what happened to it.

**Lean: (b) first, (c) as the thing that actually justifies the concept.** (c) is the
feature that makes a residual *feel* like the point rather than like a build log — an artist
seeing the object that got crushed, in the scene, is a different experience from reading a
list. It is also rung 6 work and should not be attempted early.

**Blocks:** nothing. Recorded so (c) is not lost.

---

## S5 — Does Void Squid ever write a `.ink`?

v0 reads. Does it ever write — even just "save the camera position"?

**Lean: yes, and immediately, because it is unavoidable.** Camera state is model content
(`VoidMaiz:/concepts/views-as-projections.md`), so opening a scene and moving the view is
already a mutation. Which means the write path exists from day one and should be admitted
rather than discovered.

The consequence to accept openly: **`.ink` files will be edited by two applications**
(Blender via [the Polyp](../../VoidJelly/okf/concepts/polyp.md), and Void Squid), which is
`VoidJelly:/design/open-questions.md` §2 arriving from the other direction. It strengthens
the lean there — that this is a `VoidPalabra:/concepts/join.md` problem and not a
synchronisation problem.

**Blocks:** nothing technically, but it should be **said out loud before rung 2.5**, because
"it only reads" is a comfortable and false assumption to build on.

---

## S6 — When does the charter widen from harness to forge? *(new 2026-10-03)*

The author, 2026-10-03: Void Squid *"will be the engine to tie everything together,
combine it all into 1 thing, and be the ultimate generalizable game creation tool."* Read
through [engine horizon](/design/engine-horizon.md), that is **a forge for many runtimes**:
gather, compose, check residuals, and build to Bevy, the DS and others
([the forge for many runtimes](/design/the-forge-for-many-runtimes.md)).

The founding charter is still *"BARE MINIMUM … purely to be testing Void Jelly."* Both
statements are the author's, and they are not in conflict *if one comes after the other.*
The question is what marks "after".

**Lean: a test, not a date.** The charter widens when **one `.ink` has been emitted to two
targets with two correct, different residuals**. That means Void Jelly's rung 3 (DS) plus
either rung 2.5 (GL) or rung 2.5b (Bevy). Before that, every forge feature is a feature
built on an unproven pipeline. The railguard's question (*"am I building this because Void
Jelly needs it proved, or because it would be fun?"*) stays in force until then.

**Blocks:** nothing in v0. It decides when the railguard's §1 out-of-scope list starts to
shrink.
