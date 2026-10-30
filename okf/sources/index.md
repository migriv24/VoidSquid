---
okf_version: "0.1"
---

# Sources — external work Void Squid leans on

Per Void Core's convention (`VoidCore:/references/okf-spec.md`, source provenance) and
Latin OS's practice: one page per external work, `type: Source`, cited by ordinary body
link so that a page's *linked from* list is the blast radius of a wrong attribution.
Confidence means: `asserted` = written from recall or a secondary account, **not**
checked against the artifact; `verified` = someone opened the artifact and confirmed
author, year and claim. The verification queue is every `asserted` page below.

Gathered 2026-10-09 for [what a game is](/concepts/what-a-game-is.md) and
[the epistemology of the game](/design/epistemology-of-the-game.md). Every page here is
one the author's five beliefs lean on; nothing was added for completeness.

## What a game is, and whether that can be known

- [Wittgenstein — family resemblance](/sources/wittgenstein-family-resemblance.md) —
  no definition; an open vocabulary instead of a schema
- [Huizinga — Homo Ludens](/sources/huizinga-homo-ludens.md) — the bounded world
- [Caillois — Man, Play and Games](/sources/caillois-man-play-and-games.md) — the four
  kinds and the paidia–ludus axis; where Tetris sits
- [Suits — The Grasshopper](/sources/suits-the-grasshopper.md) — the lusory attitude; why
  *player* is a noun
- [Juul — Half-Real](/sources/juul-half-real.md) — real rules, fictional world; the
  classic game model

## Feedback, learning, and what the rules say

- [Salen & Zimmerman — Rules of Play](/sources/salen-zimmerman-rules-of-play.md) —
  meaningful play: discernable and integrated
- [Koster — A Theory of Fun](/sources/koster-a-theory-of-fun.md) — the game as teacher
- [MDA](/sources/mda-framework.md) — mechanics, dynamics, aesthetics on forge, runtime,
  player
- [Bogost — procedural rhetoric](/sources/bogost-procedural-rhetoric.md) — the rules
  carry the thesis; the upgrade graph is where values are written
- [Hocking — ludonarrative dissonance](/sources/hocking-ludonarrative-dissonance.md) —
  when the upgrade graph and the story disagree

## The Dungeon Master, narrative, and the living world

- [Braunstein to D&D](/sources/braunstein-to-dungeons-and-dragons.md) — the referee's
  three jobs; the rulebook as an engine for a human processor
- [Murray, Jenkins, Aarseth](/sources/narrative-agency-and-architecture.md) — agency;
  embedded, enacted and emergent narrative; ergodic traversal
- [AI directors and drama managers](/sources/ai-directors-and-drama-managers.md) —
  Façade, Left 4 Dead, experience management; the DM's third job as software
- [iMUSE and adaptive music](/sources/imuse-adaptive-music.md) — a score as a graph
  with conditions
- [Destructible worlds](/sources/destructible-worlds.md) — the representation decides
  what can change

## Architecture

- [ECS — Dungeon Siege and Overwatch](/sources/ecs-dungeon-siege-overwatch.md) — why a
  mantle reads as an ECS and why a Bevy world is a projection of one
- [REA — Reverse Engineer Anything](/sources/rea-reverse-engineer-anything.md) —
  **verified**; the first ingress toolchain

## 3D understanding and AI (added 2026-10-09 for [smart logs](/design/smart-logs.md))

- [ConceptGraphs and SceneGPT](/sources/conceptgraphs-scenegpt.md) — an object-centric
  scene graph as text is what a language model reasons over best
- [MeshCoder](/sources/meshcoder.md) — **verified** (abstract); shape as part-structured
  code; a reader whose output is structure
- [SAM 3D Objects](/sources/sam-3d-objects.md) — **verified**; image + mask to 3D objects;
  vendored as code in Void Bicho; needs 32 GB, cannot run here
- [Running many simulations](/sources/simulation-research.md) — vectorised environments, copy-on-write forks and replay, SUMO's multi-instance control; the basis of [simulation](/design/simulation.md)

## Decompilation (added 2026-10-10 for [decompilation](/design/decompilation.md))

- [universal-modder](/sources/universal-modder.md) — **verified** (repository read at a
  pinned commit); agent-driven modding and mashups; the bridge contract is the shared
  structure of games; a trimmed reference copy is in `third_party/`
- [Matching decompilation and static recompilation](/sources/matching-decompilation-and-recompilation.md)
  — decomp.dev, the community list of about 400 projects, objdiff and decomp-toolkit,
  N64Recomp; the source depth and the opaque depth, each with its oracle
- [Learned decompilers](/sources/learned-decompilers.md) — LLM4Decompile and the 2025–26
  reinforcement-learning papers; small models are competitive, nobody classifies, and
  recompiling is not preserving

## Vision (added 2026-10-11 for [seeing what was decompiled](/design/seeing-what-was-decompiled.md))

- [Computer vision in the age of language models](/sources/vision-in-the-age-of-language-models.md)
  — SAM 3, part segmentation by lifting, marking instead of cropping, functional scene
  graphs, game-video benchmarks; set beside what the toolbox measured
- [Physics, lighting and input](/sources/physics-lighting-and-input.md) — Jolt against
  PhysX, Bullet and Rapier; the game's own lights and what goes on top; Unity's input
  actions; Super Mario Galaxy's cameras as data
- [dk64_lib and the game wikis](/sources/dk64-lib-and-the-game-wikis.md) — **verified** (used):
  Donkey Kong 64's geometry through a GPL-3 toolchain; 896 wiki pages as a layer of meaning
- [Translating between games](/sources/cross-game-transfer.md) — learned projection and
  structural analogy; where Void Core's structures stand
