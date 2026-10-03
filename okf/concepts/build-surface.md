---
type: Concept
title: Build surface
description: The effect side — every holiday Void Jelly needs a host for, enumerated, plus the rule that decides where a new one goes.
tags: [status:planned, audience:dev, confidence:asserted, foundation]
timestamp: 2026-08-26T00:00:00Z
---

**Void Jelly performs no effects.** No file I/O, no network, no subprocesses, no window, no
clock (`VoidJelly:/design/library-boundary.md`). Every one of those is a
[holiday](VoidCore:/concepts/holiday.md), and **Void Squid is the host that implements
them.**

This page is the list. It is short by design, and its shortness is the evidence that the
boundary is drawn in a sensible place: if this list were long or miscellaneous, Void Jelly
would be asking the host to do its thinking.

# The holidays

| holiday | what Void Squid does | Void Jelly's half |
|---|---|---|
| **file read** | read `scene.ink` off disk | `read_ink(bytes) -> ink` |
| **file write** | write the blot's bytes wherever the user asked | produced the bytes |
| **process** | invoke devkitARM, `ndstool`, the linker | produced everything they consume |
| **window / GL** | create the window, own the context, run the frame loop, present | `emit(ink, gl_profile) -> drawlist` |
| **emulator** | launch it on the finished `.nds` | nothing |
| **cache** | store derived [charts](../../VoidJelly/okf/concepts/charts.md) and residuals between runs | defines the **key**; owns no storage |
| **clock / progress** | turn the library's structured events into a progress bar | emits events into a host-registered sink |
| **user choice** | which nib, which budget, when to build | nothing — policy is never the library's |

# The rule for anything new

When something does not obviously belong to either side:

> **Could it run in a unit test with no filesystem, no network and no display?**
>
> Yes → **Void Jelly**. No → **Void Squid**.

That test is mechanical, and it has already settled the case most likely to be argued
about. *Producing the exact byte sequence of a packed GX display list* passes — pure
computation, testable in memory. *Linking those bytes into an ARM binary with devkitARM*
fails — a subprocess against a toolchain we do not own. **Void Jelly stops at the last byte
it can compute**, and everything after that is here.

# The toolchain is the interesting one

Building a `.nds` means calling out to devkitARM. That is a real dependency with real
failure modes, and Void Squid owns all of them:

- **Is it installed?** Where? Which version? This is a *discovery* problem that will be
  worse on Windows, which is the author's platform.
- **It fails.** Compiler errors, a missing linker script, a stale path. The host has to
  surface that legibly, and — importantly — **distinguish it from a Void Jelly failure**. A
  user staring at a red build should be able to tell whether the geometry was wrong or the
  compiler was missing.
- **It is slow.** Which is what makes a cached, incremental build worth having, and
  therefore what makes Void Jelly's content-addressing pay off downstream rather than only
  in theory.

Note who owns the C source that libnds compiles: **Void Jelly ships it as data** (a
resource in the DS nib), because it is the blot's *reader*, and a format with two owners
drifts. Void Squid compiles it and never edits it. This is
`VoidJelly:/design/open-questions.md` §11 and it is a lean, not yet a ruling.

# Effects are refused by default

Inherited from Void Maiz's headless design, and correct here without modification:
effects are *"the one thing `revert` cannot undo,"* so they are refused unless granted per
run.

```
squid build scene.ink --nib ds --dry-run-effects     # rehearse: emit, report, write nothing
squid build scene.ink --nib ds --allow-effects=write,toolchain
```

`--dry-run-effects` is more useful here than it sounds, and it falls directly out of Void
Jelly's purity: **you can run the entire pipeline and get a complete
[residual](../../VoidJelly/okf/concepts/residual.md) without producing a single artifact.**
"Will this fit on a DS?" is answerable in CI, on a machine with no devkitARM installed at
all. That is a genuinely good property and it exists only because the library refuses to
touch the disk.

# What this buys Void Jelly

Stated plainly, because it is the return on the boundary:

- Void Jelly's test suite needs **no window, no GPU, no emulator, no toolchain**.
- A second host — a Blender in-viewport preview, a build server, a future Maiz XR
  viewport — costs a new implementation of *this list* and nothing else.
- Someone working on the voxel nib does not need devkitARM installed.

# Status

`status:planned`. Nothing built. This is rung 3's work; the file-read half arrives at rung
2.5 with the [viewport](/concepts/viewport.md).
