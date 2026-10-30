---
type: Concept
title: Construct and deconstruct
description: An engine that builds artifacts must also be able to take them apart. Egress (emit a document onto a target, with a residual for what the target could not hold) and ingress (read an artifact into a document, with a residual for what the reader could not recover) as one architecture; readers as the inverse of nibs; REA as the first ingress toolchain; and the design rule that every noun in the game model must survive both directions.
tags: [status:planned, audience:dev, confidence:exploratory, foundation]
timestamp: 2026-10-09T00:00:00Z
---

> The author, 2026-10-09: *"we must understand how to DECONSTRUCT as well. we need both
> ends here, where we have systems and architecture that can support both things at the
> same time... decompilation is just as important as compilation."* And:
> *"we should even INCLUDE 'reverse engineer anything'."* **Nothing here is built.** The
> packaging of REA is S10 in [open questions](/design/open-questions.md).

# 1. The symmetry the family already has half of

Void Jelly's whole subject is **egress with an honest account**: `emit(document, profile)
→ (blot, residual)`, where the residual names what the target could not hold. Reverse
the arrow and the shape is the same:

```
   egress   (construct)   emit : document  → artifact  + residual   what the target could not hold
   ingress  (deconstruct) read : artifact  → document  + residual   what the reader could not recover
```

The ingress residual is as real as the egress one. A decompiled binary has lost its
symbol names, its comments, its source structure; a game's progression recovered from
its data tables has lost the designer's intent behind each number. A reader that
pretends otherwise is lying in the way `VoidJelly:/concepts/nibs/trivial.md` warns an
empty residual lies. **REA's own output format already says this**: every conclusion
carries *"the evidence and limitations behind"* it ([REA](/sources/rea-reverse-engineer-anything.md)).

Jelly's **lens** law, `read(write(x)) == x`, is the degenerate case where the artifact is
our own format and both residuals are empty. Everything else is lossy in at least one
direction, and the architecture's job is to say where.

# 2. Readers are the inverse of nibs

A **nib** is a profile (what a target can hold) plus an emitter. A **reader** is a
profile (what an artifact kind contains) plus an ingester. The family has them already
without the name:

| artifact kind | reader | produces | residual names |
|---|---|---|---|
| a `.blend` | the Polyp (Jelly's Blender addon) | an `.ink` | what Blender held that `.ink` does not (Q13: armatures, for now) |
| a shipped game's files | a *community reader* (`VoidJelly:/design/rust-and-bevy.md`; iw4L's "point it at the copy you own") | an `.ink` with assetless references (Jelly Q17) | formats the reader could not parse |
| a vendor's device configuration | Chisme's *translation holidays* | a Chisme model | vendor features the model lacks |
| a native binary, an APK, an Electron app, a captured session | **REA** driving Ghidra, JADX and the rest | pseudocode, symbols, imports, IPC graphs, *with evidence and limitations* | stated per conclusion |
| a Void Core state document | Core's own reader | a mantle | nothing; this is the lens |
| *(added 2026-10-10)* a console ROM or disc, an install folder, any game file | **recon** ([built](/concepts/recon.md)), the stage before every reader | which reader: platform, engine, header, tools by depth | what recon did not determine; it is the only row with code |
| a game with a **matching decompilation** or a **static recompilation** | the community's own build (splat, objdiff, decomp-toolkit; N64Recomp) | source that compiles to identical bytes; or the original running natively | the unmatched functions: decomp.dev's percentage *is* this residual ([source](/sources/matching-decompilation-and-recompilation.md)) |

The last column is what makes a reader a reader and not a converter. A reader that does
not produce a residual belongs in the same bin as a nib that does not.

# 3. What a reader produces, for a game

Two documents, one per owner, joined by reference:

- **the scene**: an `.ink`, through Void Jelly, for everything with geometry and a place.
  Squid never parses or writes it directly ([railguard](/design/what-voidsquid-is-not.md) §4).
- **the game**: a mantle in [the game model](/concepts/what-a-game-is.md)'s vocabulary,
  for everything that is not geometry: characters and their stats, the upgrade graph, the
  beat graph, the feedback list, the cues. The first thing worth recovering is the
  **upgrade graph**, because it is the game's thesis in its most compressed form
  ([Bogost](/sources/bogost-procedural-rhetoric.md)).

And the ingress residual, stored like an egress one (Jelly Q8: keyed, diffable), so that
*"what did we fail to recover from this game"* is a report and not a feeling.

# 4. REA, and what "include" can mean

Three facts decide the shape, all from the verified source page:

1. **REA is a Node application that spawns other programs.** In the family's vocabulary
   that is a **process holiday**: the same kind of thing as devkitARM, `ndstool` and
   `cargo` on the egress side. The family does not vendor devkitARM; it *discovers* it on
   the machine, surfaces its failures legibly, and keeps it out of the library's
   dependency graph ([build surface](/concepts/build-surface.md), "the toolchain is the
   interesting one"). REA is the first **ingress toolchain**, and it goes in the same
   row.
2. **Rule 10 (vendor, don't depend) is about code the binary links.** A toolchain the host
   invokes is not that; `npx rea-agents` is the consumer's package manager on the
   consumer's machine, as `cargo` is for the Bevy nib (Jelly Q15's reasoning). Vendoring
   a Node application into a C++20 build would honour the letter of the rule and defeat
   its purpose.
3. **REA's results are evidence plus limitations.** A reader that turns them into a
   mantle keeps both: the conclusion as content, the evidence as links to the artifact's
   bytes (content-addressed, in Palabra's store), the limitations as the residual.

So the lean, recorded as S10: **REA is an ingress toolchain in the build surface, with
the same discovery, failure and effect-permission discipline as devkitARM, and a reader
(pure) that maps its output into the game model.** Nothing of REA is linked, and
`squid read game.apk --via rea` is refused by default like every other effect. The
author may prefer a tighter inclusion (a vendored pin of a specific REA version under
`tools/`, with its licence); that is also on the table and it is the author's call.

REA's disclaimer is adopted as this bundle's: deconstruction is for research, analysis
and reconstruction, and the person running it is responsible for authorisation. This
page says nothing about whether a given reading is permitted.

# 5. The design rule this adds

**Every noun in the game model must survive both directions.** For each glyph, tag axis
and link kind in [what a game is](/concepts/what-a-game-is.md):

- *emittable*: a nib can carry it to at least one target, or name it in the residual;
- *recoverable*: a reader can find it in at least one artifact kind nobody in this family
  made, or name it in the residual.

A noun that fails the second test is an editor convenience, and it moves to
[the studio](/concepts/studio-surfaces.md). This is the author's *"making choices with
both in mind"* made mechanical, and it is the test that keeps pillar 1 from becoming a
schema for games only this engine can make.

# 6. What is not being built

*2026-10-10: the author made decompilation central, and the program is now written down
in [decompilation](/design/decompilation.md): four depths of recovery (bytes, opaque,
source, structure) each with its oracle; opaque pieces as runes whose behaviour is a
holiday with a contract; the pipeline from recon to run; Void Bicho as a load-bearing
layer. The first stage, [recon](/concepts/recon.md), is built. Whether the rest moves
ahead of the order below is S20.*

No reader, no REA integration, no decompiler. Rung order is unchanged
([roadmap](/roadmap.md)): the first ingress is after the first two egresses have
residuals that can be compared, because a reader's residual needs a nib's to be
calibrated against. What this page buys now is that the build surface's "process" row,
the effect-permission discipline and the game model are all designed with the arrow
pointing both ways.
