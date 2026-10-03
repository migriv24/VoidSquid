---
okf_version: "0.1"
---

# Void Squid — Knowledge Bundle (dev)

An Open Knowledge Format bundle describing **Void Squid**: a minimal game engine built on
Void Core and Void Maiz, whose first and only job is to **be the host that
[Void Jelly](../../VoidJelly/okf/index.md) cannot be for itself.**

One sentence: **Void Squid opens a `.ink`, shows it, and builds it into something a
machine can run — performing every effect that Void Jelly, being pure, refuses to.**

Founded 2026-08-26. **No code exists yet.** Per the honesty convention inherited from the
Void Core bundle, a concept may not claim `status:current` without a `resource:` link to
the code that backs it — so **every page here is `status:planned`**.

## Read this first

**Void Squid v0 is not a game engine yet. It is a conformance harness that happens to look
like an editor**, and reading it as anything else is how it eats its own schedule and Void
Jelly's with it.

The author was explicit at founding:

> *"void squid will be made using maiz, and like a BARE MINIMUM thing. we will NOT be
> focusing on all the cool stuff a game engine needs. this is purely to be testing Void
> Jelly."*

That is the whole charter, and it is written into a [railguard](/design/what-voidsquid-is-not.md)
because the pull the other way is enormous and constant.

## Why it exists at all

**Void Jelly is a library and it is pure** — no files, no window, no processes
(`VoidJelly:/design/library-boundary.md`). Which means Void Jelly *cannot demonstrate
itself*: it can emit the bytes of a `.nds` and cannot write them; it can emit a draw list
and cannot draw it.

A library in that shape, with no host, is **unfalsifiable**. Every design decision looks
fine because nothing is pulling on it. The author's reasoning:

> *"the reason i think we need the game engine NOW, super early, is purely because the game
> engine will be the thing to actually UTILIZE Void Jelly to create the 'export'."*

So Void Squid exists to pull. Everything in this bundle follows from that and from nothing
else.

# The three verbs

That is the entire v0 surface. Each is one call into Void Jelly wrapped in the effects
Jelly refuses to perform.

| verb | what Void Squid does | what Void Jelly does |
|---|---|---|
| **open** | read the file off disk | `read_ink(bytes) -> ink` |
| **view** | own the window, the GL context, the camera, the frame | `emit(ink, gl_profile) -> drawlist` |
| **build** | write the bytes, invoke devkitARM, package, launch the emulator, show the result | `emit(ink, ds_profile) -> (blot, residual)` |

Every row has the same shape: **Void Squid performs the effects on both ends; Void Jelly
does the computation in the middle.**

## The direction, 2026-10-03

The author: Void Squid *"will be the engine to tie everything together … the ultimate
generalizable game creation tool."* Through [engine horizon](/design/engine-horizon.md)'s
claim that Void Core's contribution to a game engine is *the editor, not the runtime*, that
means **one forge that pours into many runtimes**. The Rust/Bevy rewrite community is
building runtimes and has no forge. **v0 stays a harness until one `.ink` reaches two targets
with two correct residuals** ([S6](/design/open-questions.md)). See
[the forge for many runtimes](/design/the-forge-for-many-runtimes.md).

# Concepts

* [Scope](/concepts/scope.md) - what v0 is and is not, in one page; the harness framing
* [Viewport](/concepts/viewport.md) - opening a `.ink` and showing it; why this is the most-exercised path in the whole system
* [Build surface](/concepts/build-surface.md) - the effect side: every holiday Void Jelly needs a host for, enumerated

# Design

* [What Void Squid is NOT](/design/what-voidsquid-is-not.md) - **the railguard**; read before adding anything
* [Engine horizon](/design/engine-horizon.md) - the actual game engine, later, and the three seams v0 must not foreclose
* [The forge for many runtimes](/design/the-forge-for-many-runtimes.md) - the author's 2026-10-03 direction: Squid as the forge that ties the family together and pours into Bevy, the DS and others; why v0 does not move
* [Open questions](/design/open-questions.md) - what is genuinely undecided

# Planned work

* [Roadmap](/roadmap.md) - interleaved with Void Jelly's, because neither half proves anything alone
* [Log](/log.md) - what has changed in this bundle
* [App manifest](/app.md) - Void Squid's structured self-description

# Sibling bundles

- **Void Jelly** — `C:\Users\migri\Documents\Projects\VoidJelly\okf` — the 3D library.
  Owns `.ink`, nibs, emitters, residuals, geometry. **Void Squid computes nothing about
  geometry.**
- **Void Core** — `C:\Users\migri\Documents\Projects\VoidCore\okf` — rune, mantle, holiday,
  glyph, dispatcher, tags, links. Void Squid owns none of it.
- **Void Maiz** — `C:\Users\migri\Documents\Projects\VoidMaiz\okf` — the UI library Void
  Squid is built with, and the source of its headless front-end.
- **Void Palabra** — `C:\Users\migri\Documents\Projects\VoidPalabra\okf` — versioning and
  persistence, relevant when `.ink` becomes a Palabra container.

Cross-bundle references are written `VoidJelly:/concepts/nib.md` (plain-text convention —
OKF links are bundle-root-relative, so foreign bundles cannot be linked directly).

## The dependency, stated once

**Void Squid depends on Void Jelly. Void Jelly does not depend on Void Squid**, and must
build, test and ship with this directory deleted from the disk. That is a claim to check
with a build, not with a promise.

# Inter-agent messages

Convention inherited from Void Maiz (2026-07-21): every message to another agent is a
uniquely titled file in the **recipient's** repo root,
`MESSAGE_FOR_<RECIPIENT>_<sender>-<topic>-<YYYY-MM-DD>.md`, with a matching
self-identifying H1. This index tracks open vs consumed; retire a thread here, not by
deleting the file.

**None open.** Void Squid is the client agent in the Jelly relationship, so most traffic
will run the other way: **report what the library made hard**, in the pattern Void Hormiga
and Void Maiz use — a measured finding, not a feature request.
