---
type: Design
title: The epistemology of the game — what can be known about what a game is
description: The research behind the first pillar, read as a question of knowledge rather than taste. Four positions in the literature (no definition; a stance; rules plus fiction; meaning through rules), what this bundle adopts from each, why the Dungeon Master is the hinge between forge and artifact, and the method that keeps every claim falsifiable.
tags: [status:planned, audience:dev, audience:all, confidence:exploratory, foundation]
timestamp: 2026-10-09T00:00:00Z
---

> The author, 2026-10-09: *"this requires a lot more research, and it would be valuable to
> add into the OKF this philosophical and epistemological research."* This page is the
> argument; [what a game is](/concepts/what-a-game-is.md) is the vocabulary it produces;
> [sources](/sources/index.md) are the evidence, every one tagged with how far it has
> been checked. **Nothing here is built.**

# 1. The question

Before an engine can hold a game it has to hold *a model of what a game is*, and the
first honest thing to say is that **nobody has a definition that survives**. The history
of trying is short enough to state, and each failure left something usable.

# 2. Four positions, and what each one leaves behind

**No definition, only resemblance.** [Wittgenstein](/sources/wittgenstein-family-resemblance.md)
chose games as *the* example of a word with no common core. The consequence for an
engine is exact: a game model cannot be a schema (a fixed set of required fields), because
any schema excludes a game. It has to be an **open vocabulary** whose terms overlap the
way the games do. Void Core's glyphs and axis-typed tags are that shape; so the family's
substrate was already the right answer to a question nobody had asked it.

**A definition in the player's stance.** [Suits](/sources/suits-the-grasshopper.md)
answered Wittgenstein by moving the definition out of the artifact and into the player:
the **lusory attitude**, accepting inefficient means because they make the activity
possible. The engine's consequence is that *player* is a noun **outside the model**, like
a user is outside an application. A world with rules and nobody who has agreed to them is
a simulation. This is why the avatar (inside) and the player (outside) are two nouns and
the boundary between them is a holiday.

**Rules plus fiction.** [Juul](/sources/juul-half-real.md): the rules are real, the
world is fictional, and the two halves are separable, with Tetris as the limit where the
fiction is nearly empty. The engine's consequence is the family's own line: rules are
**data the runtime executes**, the fiction is **the mantle**. The author's *"we want
narratives"* makes the fiction half mandatory for the games Void Squid makes, and
[Caillois](/sources/caillois-man-play-and-games.md)'s *mimicry* names what Tetris lacks.
[Huizinga](/sources/huizinga-homo-ludens.md) is the oldest statement of the fiction half
as a **bounded world** with its own laws, which is a mantle.

**Meaning through rules.** [Salen & Zimmerman](/sources/salen-zimmerman-rules-of-play.md)
(meaningful play is discernable and integrated feedback), [Koster](/sources/koster-a-theory-of-fun.md)
(the game is a teacher), [Bogost](/sources/bogost-procedural-rhetoric.md) (the rules carry
the argument), and [MDA](/sources/mda-framework.md) (mechanics are authored; dynamics and
aesthetics emerge). Together they say what the author said in one sentence: the upgrade
path is the game telling you what *better* means. The engine's consequence is that **the
upgrade graph is authored structure the forge can query**, and that the forge cannot
author dynamics or aesthetics at all, only invite them.

# 3. The Dungeon Master is the hinge

The author's first belief is the one that organises the rest, and it is historically
literal: Dungeons & Dragons is a **game engine for a human processor**
([Braunstein to D&D](/sources/braunstein-to-dungeons-and-dragons.md)). The rulebooks are
tables and procedures; the DM runs them, fills their gaps, and generates the world.

Those three jobs are the three places computation lives in any game, and they fall on
different sides of Void Core's forge/artifact line:

- **Running the rules** is the artifact's, at frame rate, in the runtime's own language.
- **Adjudicating the gaps** is authored: rules in the document, derived consequences
  through Void Allomone. The forge's.
- **Generating** is the one job that is generative at play time. It wants a model, and a
  model is a holiday. It steers structure the forge authored
  ([AI directors and drama managers](/sources/ai-directors-and-drama-managers.md)).

What makes this the hinge is that **every attempt to put all three jobs in one place has
produced the wrong engine.** Put them all in the runtime and the forge is a file browser
(most engines). Put them all in the forge and the forge is a runtime, which is the
VoidDS error (`VoidJelly:/design/voidds-proposal-critique.md`) and the drift test Void
Core's railguard exists for. The DM, taken apart, is how the family's one principle
reaches gameplay.

# 4. Narrative, music, terrain, and the limits of authorship

The author's fifth belief is where the literature is most useful as a **list of what
cannot be authored**:

- [Jenkins](/sources/narrative-agency-and-architecture.md): *emergent* narrative cannot be
  written, only invited by systems. *Embedded* and *enacted* narrative can, and they are
  content on runes in places and a beat graph respectively.
- [iMUSE](/sources/imuse-adaptive-music.md): reactive music is authored as a graph of
  segments with conditions; the *reaction* is the runtime walking it. The composer
  authors a structure, not a response.
- [Destructible worlds](/sources/destructible-worlds.md): a world can change exactly to
  the extent its representation is uniform and content-addressable. The representation
  decides, which is a Jelly decision (the voxel nib), and the author's requirement is
  really a requirement on representation.

The lesson across all three is that **the forge authors structure with conditions, and
the artifact walks it.** That is the 2026-10-03 lean (*Squid composes structure, never
mechanics*) arriving from the humanities instead of from Bevy.

# 5. Deconstruction as an epistemic method

The author asks that the engine be able to deconstruct as well as construct, and names
[REA](/sources/rea-reverse-engineer-anything.md). Read epistemically this is a **method
for testing the vocabulary**: a noun in [what a game is](/concepts/what-a-game-is.md)
is well chosen if a reader can recover it from games nobody in this family made. The
upgrade graph should be recoverable from an existing game's progression tables; the beat
graph from its dialogue trees; the feedback list from what the engine does on a hit. A
noun that no reader could ever find is an editor convenience wearing a philosopher's
coat. [Construct and deconstruct](/concepts/construct-and-deconstruct.md) makes this a
design rule.

# 6. The method, so that none of this is decoration

- Every claim above is **tagged with what it leans on**, and every source is tagged with
  whether anyone has opened it. On 2026-10-09 every source but one is
  `confidence:asserted`: written from recall, not checked. The verification queue is the
  [sources index](/sources/index.md), and raising a page to `verified` is a task anyone
  can take.
- Every noun in the vocabulary has **three tests** ([what a game is](/concepts/what-a-game-is.md)
  §9): stateable in Core's terms, pure, and recoverable from an artifact.
- Every design consequence is **a lean, and the author decides**
  ([open questions](/design/open-questions.md)).
- And the oldest rule in the bundle still holds: **nothing is built before Void Jelly
  emits.** The research is a position the code will be measured against, not a licence
  to start.

# 7. The reading still owed

Peterson's *Playing at the World* (2012) for the D&D lineage; Bartle's player types
(1996) for whether *player* needs sub-kinds; Csikszentmihalyi's *Flow* for the feedback
loop's tempo; Laurel's *Computers as Theatre* (1991) for the OS-as-stage reading that
Latin OS's reframing wants; Costikyan's *I Have No Words & I Must Design* (1994) for a
designer's definition to set beside the philosophers'. None blocks anything.
