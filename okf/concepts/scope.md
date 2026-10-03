---
type: Concept
title: Scope
description: What Void Squid v0 is — a conformance harness for Void Jelly that happens to look like an editor — and the exit condition that lets it become more.
tags: [status:planned, audience:dev, confidence:asserted, foundation]
timestamp: 2026-08-26T00:00:00Z
---

# The framing that makes everything else easy

**Void Squid v0 is a conformance harness for [Void Jelly](../../VoidJelly/okf/index.md)
that happens to look like an editor.**

That sentence is worth more than a feature list, because it settles arguments before they
start. Every proposed feature gets one question:

> **Does this exercise a Void Jelly seam that is not otherwise exercised?**

If yes, it is in scope even if it is unglamorous. If no, it is out of scope even if every
game engine has it. Undo, gizmos, material editing, a scene outliner, asset browsers, play
mode — all excellent, all out.

The author set this directly: *"a BARE MINIMUM thing… purely to be testing Void Jelly."*

# What v0 actually is

A **Void Maiz application** (`VoidMaiz:/okf/index.md`) with:

- a window and a 3D **[viewport](/concepts/viewport.md)**;
- a file open dialog for `.ink`;
- a nib picker (GL / DS / voxel / ASCII);
- a **build** button;
- a panel that shows a [residual](../../VoidJelly/okf/concepts/residual.md);
- and, for free, a **headless front-end** (see below).

That is it. Roughly one screen.

# Why a Void Maiz application, specifically

Not an arbitrary choice — three things arrive with it that would otherwise be work:

1. **The model lives in Void Core, and Void Squid owns no truth.** Maiz's founding
   commitment. A `.ink` is a mantle; the application is a projection of it. There is no
   second data model to keep in sync, which is the failure mode every editor has.

2. **Every gesture is a visible, replayable command.** Maiz's commitment 2 — humans,
   scripts and agents share **one** interaction surface. So "open this, build it for DS" is
   the same thing whether a person clicks it or an agent types it.

3. **Headless comes for ~40 lines, not a port.** `maiz::run_cli(app, argc, argv)` is the
   whole front-end (`VoidMaiz:/concepts/headless-adoption.md`). Which means this exists
   almost immediately:

   ```
   squid build scene.ink --nib ds --out build/
   squid describe                       # what can you do?
   squid status ; squid diff            # what did it change?
   ```

   That is a scriptable, CI-able, agent-drivable build pipeline, and it matters more than
   the GUI for Void Jelly's purposes: **a regression suite over residuals needs a command
   line, not a window.**

   One caveat inherited from Maiz: **effects are refused by default and granted per run**,
   because they are the one thing `revert` cannot undo. Writing a `.nds` and invoking
   devkitARM are effects. That default is correct here and should not be relaxed.

# The four things v0 must prove

These are the reasons it exists. Each corresponds to a Void Jelly claim that is currently
unfalsifiable.

| # | claim under test | how v0 tests it |
|---|---|---|
| 1 | **the library boundary is workable** — a pure library plus an effectful host is not merely tidy but *usable* | build the thing; notice every place the seam is awkward and report it |
| 2 | **the GL residual is nearly empty** | the viewport runs it every frame; if the picture is wrong, the loss is in `.ink`, not in a target |
| 3 | **a residual is presentable**, not just complete | put one on screen and see whether it can be read |
| 4 | **emission is cheap enough** to sit in an interactive loop | move an object, redraw, and find out |

Claim 4 is the one most likely to come back with bad news, and finding that out in week
three is the entire value of founding the client early.

# The exit condition

v0 is done when it can open a `.ink`, show it, build it for the DS, boot the result in an
emulator, and display the residual — **from both the GUI and the command line.**

At that point Void Jelly has a real host, and the question *"what should Void Squid become"*
becomes answerable with evidence instead of imagination. See
[engine horizon](/design/engine-horizon.md). Until then it is not a question worth having
an opinion about.

# What it is in Void Core's terms

The **forge, not the artifact** (`VoidCore:/design/domains-and-guarantees.md`). Void Core's
own worked example is exactly this domain: *"the running game the player plays"* is the
artifact; *"the level / dialogue / asset editor"* is the forge.

So Void Squid is the forge, the `.nds` is the artifact, and **no Void Core ships to the
target**. When Void Squid eventually grows a runtime
([engine horizon](/design/engine-horizon.md)), that runtime is on the artifact side of the
line and inherits none of this.

# Status

`status:planned`. Nothing built.
