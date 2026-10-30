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

*2026-10-09: what it becomes is now written down.* [The three pillars](/design/the-three-pillars.md)
names the game model, the studio and the hosts; [what a game is](/concepts/what-a-game-is.md)
is the vocabulary. **This section still governs v0 and the studio.** The game model is
pure and has its own test (§8 below); it is designed now and built when the author rules
the charter widened ([open questions](/design/open-questions.md) S6, S14).

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

## 6. The artifact never ships the forge

*Restated 2026-10-09 at Latin OS's request; the lean is S8 in
[open questions](/design/open-questions.md), and the author decides. The original
wording follows the restatement.*

> **The artifact never ships the forge.** It may ship Void Core; it never ships the
> studio, the viewport, the residual panel or the build surface.

For a game the original rule still holds in full: **no Void Core ships to the target.**
No runes, no tags, no dispatcher, and above all **no interpreter of mantle rules.**
Authored rules compile to **inert data tables** the runtime reads in its own language.
This is the specific error the superseded VoidDS proposal made
(`VoidJelly:/design/voidds-proposal-critique.md` §1), and it is Void Core's own drift test
failing — *"are we making Void Core compute the application's actual results?"*

What changed is that an **operating system** is now an artifact this forge may emit
(`LatinOS:/design/the-os-as-a-game.md`), and that artifact *is* Void Core all the way
down: its kernel's rewriter is the Reduce contract, its roster is runes. Shipping Core
there is not drift, because Core is the artifact's own domain. So the drift test is
kept and the wording is widened: **an artifact carries Void Core only when Void Core is
the artifact's own computation, never because it was easier than compiling the forge's
rules to data.** A DS cartridge stays Core-free. A Latin OS image carries Core. Neither
carries Squid.

Void Squid is the **forge**; the `.nds`, the Bevy app and the OS image are **artifacts**
(`VoidCore:/design/domains-and-guarantees.md`). The line runs between them and not
somewhere convenient.

---

## 7. The summary test

> **Am I building this because Void Jelly needs it proved, or because it would be fun?**

Both answers are legitimate reasons to build software. Only the first one is a reason to
build it *now*, in *this repository*, *before the library works*.

---

## 8. The game model has its own test, and it is not softer *(added 2026-10-09)*

The first pillar ([what a game is](/concepts/what-a-game-is.md)) is not work that proves
Void Jelly, so §7 would forbid it forever. It is instead work that makes a claim about
what a game is, and the test for anything proposed there is three questions, all of which
must be answered yes:

1. **Can it be stated in Void Core's vocabulary** (rune, tag, link, content, rule,
   holiday) without a new kind of thing?
2. **Does it run in a unit test with no filesystem, network, display or clock?**
3. **Can a reader recover it from a game nobody in this family made**, or name it in a
   residual ([construct and deconstruct](/concepts/construct-and-deconstruct.md) §5)?

A proposal that fails the third is an editor convenience and belongs to the studio, where
§1 and §7 apply to it. A proposal that fails the second is a host's or a runtime's. A
proposal that fails the first is a finding about Void Core, to report upstream.

What does not change: the game model computes nothing about geometry (§3), owns no
`.ink` (§4), owns no truth outside the mantle (§5), and ships no forge to any target (§6).
