# Void Squid

**A minimal game engine on [Void Core](../VoidCore/README.md) and
[Void Maiz](../VoidMaiz/README.md) — and the host that
[Void Jelly](../VoidJelly/README.md) cannot be for itself.** Founded 2026-08-26. The OKF
comes before the code, as always.

One sentence: **Void Squid opens a `.ink`, shows it, and builds it into something a machine
can run — performing every effect that Void Jelly, being pure, refuses to.**

**Nothing is built yet.** Start reading at **[okf/index.md](okf/index.md)**.

## Read this first

**v0 is not a game engine. It is a conformance harness that happens to look like an
editor** — and reading it as anything else is how it eats its own schedule and Void Jelly's
with it. The charter, from the author at founding:

> *"like a BARE MINIMUM thing. we will NOT be focusing on all the cool stuff a game engine
> needs. this is purely to be testing Void Jelly."*

## Why it exists this early

Void Jelly is a library and it is **pure**: no files, no window, no processes. Which means
it *cannot demonstrate itself* — it can emit the bytes of a `.nds` and cannot write them; it
can emit a draw list and cannot draw it.

A library in that shape with no host is **unfalsifiable**. Every design decision looks fine
because nothing is pulling on it. Void Squid exists to pull.

## The three verbs

```
  open    Squid reads the file        →  jelly::read_ink(bytes)
  view    Squid owns window + GL      →  jelly::emit(ink, gl_profile)  -> drawlist
  build   Squid writes bytes,         →  jelly::emit(ink, ds_profile)  -> (blot, residual)
          runs devkitARM, packages,
          launches the emulator
```

Every line has the same shape: **Void Squid performs the effects on both ends; Void Jelly
does the computation in the middle.** *The transformation is Jelly's; the transaction is
Squid's.*

## What comes free from Void Maiz

- **The model lives in Void Core; Void Squid owns no truth.** No second data model to sync.
- **Every gesture is a visible, replayable command** — humans, scripts and agents share one
  surface.
- **Headless for ~40 lines, not a port:**
  ```
  squid build scene.ink --nib ds --dry-run-effects   # rehearse: full residual, writes nothing
  squid build scene.ink --nib ds --allow-effects=write,toolchain
  ```
  Because Void Jelly is pure, `--dry-run-effects` runs the *entire* pipeline and produces a
  complete residual **without devkitARM installed at all**. "Will this fit on a DS?" is
  answerable in CI on a bare machine.

## Scope

| in | out |
|---|---|
| a window, a viewport, a camera | selection, gizmos, an inspector, a scene editor |
| open `.ink`, pick a nib, build | input, physics, collision, audio, animation |
| show a residual | scripting, entity systems, prefabs, play mode |
| headless CLI | anything Blender already does better |

The test for any feature: **does this exercise a Void Jelly seam that is not otherwise
exercised?** See [the railguard](okf/design/what-voidsquid-is-not.md).

## Exit condition

Open a `.ink`, show it, build it for the DS, boot it in an emulator, display the residual —
**from both the GUI and the command line.** Only then is
[the actual game engine](okf/design/engine-horizon.md) worth a real conversation, and by
then it will be a conversation with evidence in it.

## Status

**Founding. No code.** Void Squid should not exist as code until Void Jelly's rung 2.5 —
its first three rungs need no host, and starting earlier would mean an application with
nothing to open. See the [roadmap](okf/roadmap.md), which is interleaved with Void Jelly's.

## Siblings

- **[Void Jelly](../VoidJelly/)** — the 3D library. Owns `.ink`, nibs, emitters, residuals,
  and all geometry. **Void Squid computes nothing about geometry.** Squid depends on Jelly;
  Jelly must build and test with this directory deleted.
- **[Void Core](../VoidCore/)** — rune, mantle, holiday, glyph, dispatcher, tags, links.
- **[Void Maiz](../VoidMaiz/)** — the UI library this is built with, and the source of the
  headless front-end.
- **[Void Palabra](../VoidPalabra/)** — versioning and persistence; relevant when `.ink`
  becomes a Palabra container.
