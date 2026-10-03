---
type: Design
title: What Void Squid is NOT — the railguard
description: The boundary that keeps a minimal harness from becoming a general-purpose game engine before its library works.
tags: [status:planned, audience:dev, confidence:asserted, foundation]
timestamp: 2026-08-26T00:00:00Z
---

# What Void Squid Is NOT — the Railguard

> Every project in this family opens with one of these, because each has a specific
> gravity well. Void Core's is *becoming a language with a runtime*. Void Jelly's is
> *becoming a 3D engine*. **Void Squid's is the strongest of the three, because it is the
> most enjoyable: building the game engine.**

The author set the charter and it is unusually blunt: *"like a BARE MINIMUM thing. we will
NOT be focusing on all the cool stuff a game engine needs. this is purely to be testing
Void Jelly."*

---

## 1. It is not (yet) a game engine

It is called one, and it will be one, and **it is not one now.** v0 is a
[conformance harness](/concepts/scope.md) for
[Void Jelly](../../VoidJelly/okf/index.md) that happens to look like an editor.

Out of scope, all of it, however obviously a game engine has it: input mapping, physics,
collision, audio, animation, scripting, entity/component systems, prefabs, scene
serialisation of its own, play mode, particle systems, navmeshes, networking.

The test for any proposed feature is one question:

> **Does this exercise a Void Jelly seam that is not otherwise exercised?**

Unglamorous features that pass (a residual panel, a nib picker, a `--dry-run-effects` flag)
are in. Exciting features that fail are out, and being obviously necessary *later* is not
an argument for building them *now*.

---

## 2. It is not a competitor to Unity, Godot or Blender

It will never be better than Godot at being Godot, and no part of this project's value
comes from trying.

Two consequences that are easy to state and hard to hold:

- **We do not rebuild Blender's editor.** Modeling, UV editing, material authoring, rigging
  — Blender does them, the artist already uses it, and
  [the Polyp](../../VoidJelly/okf/concepts/polyp.md) is how their work arrives. A scene
  editor inside Void Squid would be a worse Blender that also has to be maintained.
- **We do not chase feature parity.** The interesting thing here is the *pipeline*: a
  structured scene, a target that cannot hold it, and an honest account of the difference.
  Nothing in the mainstream engines does that, and nothing about matching their feature
  lists gets us closer to it.

---

## 3. It computes nothing about geometry

**Every mesh operation belongs to Void Jelly.** Decimation, voxelisation, winding numbers,
budget allocation, texture packing, fixed-point conversion, display-list encoding — all of
it, without exception.

If Void Squid ever contains a loop over vertices, something has gone wrong. The one thing
it does with geometry is **hand bytes to a GPU that Void Jelly already arranged**.

The drift here would be quiet and plausible: a small optimisation in the viewport, a quick
transform "while we have the data anyway." That is how a host acquires a second geometry
pipeline, and then two pipelines that disagree.

---

## 4. It does not own the `.ink` format

`.ink` is Void Jelly's (`VoidJelly:/concepts/ink-format.md`). Void Squid **reads and
writes it through Void Jelly** and never parses it directly, never adds a field, never
writes a "Void Squid section."

A format with two owners is a format that drifts. The family precedent is recent and
pointed: Void Maiz *deleted* its own copies of Void Core's argument codec rather than keep
four implementations in sync, and one of those copies had a security hole nobody had found.

If Void Squid needs to store something about a scene, it goes in the mantle as ordinary
rune content, through the dispatcher, like everything else.

---

## 5. It does not own truth

Void Maiz's founding commitment, inherited without modification: **the model lives in Void
Core; the application owns no source of anything** — including view state like camera
position, which is model content.

If a source-of-truth data structure appears in Void Squid, stop and re-read
`VoidMaiz:/concepts/log-first-inheritance.md`.

---

## 6. Its runtime is not Void Core

For when the engine part actually arrives ([engine horizon](/design/engine-horizon.md)):
**no Void Core ships to the target.** No runes, no tags, no dispatcher, and above all **no
interpreter of mantle rules.**

This is the specific error the superseded VoidDS proposal made
(`VoidJelly:/design/voidds-proposal-critique.md` §1), and it is Void Core's own drift test
failing — *"are we making Void Core compute the application's actual results?"* Authored
rules compile to **inert data tables** the runtime reads in its own language.

Void Squid is the **forge**; the `.nds` is the **artifact**
(`VoidCore:/design/domains-and-guarantees.md`). The line runs between them and not
somewhere convenient.

---

## 7. The summary test

> **Am I building this because Void Jelly needs it proved, or because it would be fun?**

Both answers are legitimate reasons to build software. Only the first one is a reason to
build it *now*, in *this repository*, *before the library works*.
