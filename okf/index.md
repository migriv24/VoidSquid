---
okf_version: "0.1"
---

# Void Squid — Knowledge Bundle (dev)

An Open Knowledge Format bundle describing **Void Squid**: the game engine of the Void
family, built on Void Core and Void Maiz, whose first job is to **be the host that
[Void Jelly](../../VoidJelly/okf/index.md) cannot be for itself**, and whose shape after
that is **a forge that pours one document into many artifacts** and can take artifacts
apart again.

One sentence, v0: **Void Squid opens a `.ink`, shows it, and builds it into something a
machine can run — performing every effect that Void Jelly, being pure, refuses to.**

One sentence, after: **a world under a Dungeon Master, authored as a Void Core document,
projected into a studio, emitted onto any runtime that can hold it, including an
operating system, and recovered from artifacts nobody here made.**

Founded 2026-08-26. **0.1.0, 2026-10-12: the studio**, the first real version
([Void Squid 0.1](/design/void-squid-0.1-the-studio.md)): a docked Void Maiz window over a
`.squid` project (a Void Core state document), a game library's catalogue as mantles,
Turkey, Table, Feast, Data and Scene workspaces, [Load a game](/design/load-a-game.md) on
four setups, and a desktop shortcut. The author, that day: *"void squid is NOT about
decompilation. It's about the creation of NEW things."* **First code 2026-10-09: rung 2.5.** A window opens a `.ink`, Void
Jelly's GL nib emits it, the viewport draws it, the residual panel accounts for it, and
the same application runs headless. Per the honesty convention inherited from the Void
Core bundle, a concept may not claim `status:current` without a `resource:` link to the
code that backs it; the [viewport](/concepts/viewport.md), the spatial log in
[smart logs](/design/smart-logs.md) and [simulation](/design/simulation.md) carry one. **Everything else is `status:planned`**,
including every page about the three pillars.

Build and run (MSYS2 UCRT64, Ninja; Void Core prebuilt, Void Maiz and Void Jelly beside):

```
cmake -S . -B build -G Ninja && cmake --build build
ctest --test-dir build -R squid_
build/bin/voidsquid                                           # the studio (the desktop shortcut's target)
build/bin/squid my-game.squid                                 # the studio on a project, from a console
build/bin/squid --state my-game.squid --import-library <dir>  # File > Open library, headless
build/bin/squid --allow-effects --json effect open scene.ink  # the same application, headless
```

People setting it up read [SETUP.md](../SETUP.md), not this bundle.

## Read this first

**Void Squid v0 is a conformance harness that happens to look like an editor**, and
reading it as anything else is how it eats its own schedule and Void Jelly's with it. The
author at founding: *"like a BARE MINIMUM thing. we will NOT be focusing on all the cool
stuff a game engine needs. this is purely to be testing Void Jelly."* That is written into
a [railguard](/design/what-voidsquid-is-not.md) because the pull the other way is
enormous and constant.

**On 2026-10-09 the author wrote down what it becomes**, and the pull now has a page to
pull toward instead of a shape to drift into. Three pillars
([the three pillars](/design/the-three-pillars.md)): a pure **game model**, a **studio**
of projections, and **hosts** that differ only in their effect list. The game model's
vocabulary is [what a game is](/concepts/what-a-game-is.md); the research behind it is
[the epistemology of the game](/design/epistemology-of-the-game.md) and the
[sources](/sources/index.md). **v0 did not move.** What moved is that v0 is now the first
instance of a pattern rather than a thing to be replaced.

## Why it exists at all

**Void Jelly is a library and it is pure** (`VoidJelly:/design/library-boundary.md`).
It can emit the bytes of a `.nds` and cannot write them; it can emit a draw list and
cannot draw it. A library in that shape with no host is **unfalsifiable**. Void Squid
exists to pull. The author: *"the game engine will be the thing to actually UTILIZE Void
Jelly to create the 'export'."*

# The three verbs (v0)

| verb | what Void Squid does | what Void Jelly does |
|---|---|---|
| **open** | read the file off disk | `read_ink(bytes) -> ink` |
| **view** | own the window, the GL context, the camera, the frame | `emit(ink, gl_profile) -> drawlist` |
| **build** | write the bytes, invoke devkitARM, package, launch the emulator, show the result | `emit(ink, ds_profile) -> (blot, residual)` |

Every row has the same shape: **Void Squid performs the effects on both ends; Void Jelly
does the computation in the middle.** After v0 a fourth verb runs the other way:
**read** an artifact into a document, with a residual for what could not be recovered
([construct and deconstruct](/concepts/construct-and-deconstruct.md)).

# The three pillars (after v0)

| pillar | what it is | page | pure? |
|---|---|---|---|
| **1. the game model** | world, player, avatar, character, region, rule, beat, cue, upgrade; the Dungeon Master taken apart into three jobs | [what a game is](/concepts/what-a-game-is.md) | yes |
| **2. the studio** | hierarchy, inspector, assets, scene view, game view, console; each a projection of one document; *scene* and *game* abstract | [studio surfaces](/concepts/studio-surfaces.md) | yes, by the function-pair rule |
| **3. the hosts** | the effect list, once per platform: a Windows desktop first, Latin OS second | [hosts](/concepts/hosts.md) | no; this is where effects live |

# Concepts

* [Scope](/concepts/scope.md) - what v0 is and is not; the harness framing; the exit condition
* [Viewport](/concepts/viewport.md) - opening a `.ink` and showing it; the control experiment
* [Build surface](/concepts/build-surface.md) - the effect side: every holiday Void Jelly needs a host for, plus the two rows added for ingress and a hypervisor
* [What a game is](/concepts/what-a-game-is.md) - **pillar 1**; the author's five beliefs as a vocabulary over Void Core, and the same nouns read as an operating system
* [Studio surfaces](/concepts/studio-surfaces.md) - **pillar 2**; every engine surface as `project` / `compile`; why none may be pixels
* [Hosts](/concepts/hosts.md) - **pillar 3**; desktop and LITO side by side; the recursion
* [Construct and deconstruct](/concepts/construct-and-deconstruct.md) - readers as the inverse of nibs; the ingress residual; REA as the first ingress toolchain
* [Recon](/concepts/recon.md) - **built 2026-10-10**: the first stage of decompilation; what is this artifact, checked by checksum where the format has one, with tools by depth and a residual
<!-- forge: added 2026-10-28 -->
* [The Forge](/concepts/forge.md) - **device library + match loop current 2026-10-28; real compile-and-run verifier**: decompilation as game-to-any-device; per-device plugins, a route that plans the subsystem bridges between a source and a target, a match loop whose verifier is injected (compile-and-run built 2026-10-28, with a synthetic fallback), and build/package stages; `effect forge devices|route|match|compile|shortcut`
<!-- molerat: added 2026-10-18 -->
* [Void MoleRat](/concepts/molerat.md) - **first slice current 2026-10-18; the rest planned**: a decompilation and re-expression harness; the research phase is a first-class stage, its records are runes, and the toy-ledge synthetic target produces a validated `.squid` document through `effect molerat`; answer application, `resume`, `compose --commit` and the studio remain planned
* [Turkey, Table, Feast](/concepts/turkey-table-feast.md) - the author's creation workspaces: a thing and its presentations, a world and its rules (Allomone), the game laid out in time and across people; what Allomone can be today

# Design

* [What Void Squid is NOT](/design/what-voidsquid-is-not.md) - **the railguard**; §6 restated 2026-10-09 (*the artifact never ships the forge*), §8 added (the game model's own test)
* [The three pillars](/design/the-three-pillars.md) - the author's separation; seams; packaging lean; what the "no code before rung 2.5" rule actually binds
* [The epistemology of the game](/design/epistemology-of-the-game.md) - four positions on what a game is, what each leaves behind, and why the Dungeon Master is the hinge
* [Smart logs](/design/smart-logs.md) - **the CLI for worlds of any size**: logs are answers to bounded questions; four layers of perception; the spatial log, built and measured to 30,000 objects
* [Simulation](/design/simulation.md) - **built 2026-10-10**: a table compiled to data, stepped deterministically in many instances, forked, compared, logged as events
* [Three uses](/design/three-uses.md) - decompile, create, simulate, as workspaces over one document, on Hormiga 0.2.0's precedent
* [Decompilation](/design/decompilation.md) - *(2026-10-10)* a game is a modality: four depths of recovery each with its oracle, opaque pieces as holidays with contracts, the player taken apart, the pipeline, Void Bicho as load-bearing, what forking universal-modder would mean
* [The logic translation layer](/design/logic-translation-layer.md) - *(2026-10-28)* registers, translation holidays, and the abstract device: game logic decomposes into physics/motion/rules/presentation/narrative, a port is a per-register holiday with a commutation-defect residual, and a target device is a profile of per-register capabilities
* [Seeing what was decompiled](/design/seeing-what-was-decompiled.md) - **built 2026-10-10**: a headless renderer with an ID buffer, Bicho's SAM 2.1 + CLIP, lifting labels onto triangles, scored against a skeleton, a collision mesh and Super Mario Galaxy's community decompilation; vision answers *who*, the decompilation answers *what it does*
* [Four games and the asset database](/design/four-games-and-the-asset-database.md) - **built 2026-10-12, outside this repo**: Donkey Kong 64 as the fourth game; an asset database in Void Hormiga's schema, tagged from each game's own data, its wiki, names and vision; Turkey, Table and Feast as work set-ups; move, turn and size handles; music and sounds; dropping in your own models; the GPU actually used
* [Void Squid 0.1: the studio](/design/void-squid-0.1-the-studio.md) - **built 2026-10-12**: the first real application; Hormiga's docked shape on Maiz's Session, projection and widget registry; a `.squid` project; the library spliced in as mantles in 0.8 s, keeping a person's tags, kinds and sounds; no geometry in the window; two executables
* [Load a game](/design/load-a-game.md) - **built 2026-10-12**: loading is step one of making something new; three kinds of person; four setups (minimum, local model, agent, hosted) plus vision; guardrails that rely on intelligence as little as possible; the catalogue of known decompilations; the release rule (no game data, no demo)
* [The cast](/design/the-cast.md) - **built 2026-10-13, in the exercise**: 20 playable characters from three games, with skeletons, skin weights and animations in one rig format, switchable while playing; Donkey Kong 64's Kongs not decoded
<!-- ocean: added 2026-10-18 -->
* [Motion and physics in the studio](/design/motion-and-physics-in-the-studio.md) - **built 2026-10-18** (motion): `effect motion judge|report` starts the runtime's Void Onda judge and counts what it wrote (105 clips, mean 99.7%); **specified** (physics): Void Ocean stays unlinked (S33), and the gravity seam is measured (49 of Good Egg Galaxy's 109 fields have no exact Ocean primitive yet)
* [Music and Void Loops Studio](/design/music-and-void-loops-studio.md) - *(2026-10-13)* VLS cannot play game music yet (no realtime engine); its model fits; Donkey Kong 64's music is the N64's compressed MIDI, a small step from VLS's `midi-in` node
* [Decompilation progress](/design/decompilation-progress.md) - **built 2026-10-13**: (and the Games window, the evening) every wiki page is a thing, place, system or other game; matched to turkeys and tables; missing pages ranked by links; Wikipedia downloads; a small-model pilot whose guards took wrong accepts to zero
* [Coral](/concepts/coral.md) - *(2026-10-12, redrawn the same day)* sharing a game **while it is played**: hosts, servers, sessions. Editing together is Void Maiz's, and the studio gets it by dispatching every edit through a Session
* [Goomba Galaxy 2 and the turkey table](/design/goomba-galaxy-2-and-the-turkey-table.md) - **built 2026-10-11, outside this repo**: one physics engine (Jolt) for every world; the game's own lights, cameras and lines; controllers through actions; the first turkey table, whose saved tables the game plays; "the original is the floor"
* [Goomba Galaxy](/design/goomba-galaxy-exercise.md) - **built 2026-10-11, outside this repo**: Good Egg Galaxy played as a Goomba, changed by meaning (trees, a water planet, planets to orbit, a penguin), with Mario Kart DS item boxes and Super Mario 64 DS's castle as a second world; gravity is data, and it transplants
* [The DS mashup exercise](/design/ds-mashup-exercise.md) - **built 2026-10-10, outside this repo**: three DS cartridges taken apart and played as one PC program; what it found about games; the answers to the author's five questions
* [Engine horizon](/design/engine-horizon.md) - the 2026-08-26 sketch; now named by the pillars
* [The forge for many runtimes](/design/the-forge-for-many-runtimes.md) - the 2026-10-03 direction, with *systems* recorded as the horizon after it
* [Open questions](/design/open-questions.md) - S1–S36; S34 (Void Onda in the studio), S35 (small models), S36 (music) new 2026-10-13 evening; S29 measured and S32 (skins in `.ink`), S33 (linking Void Ocean) new 2026-10-13; the author widened S6 for the studio, ruled S23 (port now) and drew S25's line on 2026-10-12, and S12 has a provisional `.squid`; S28 (the shell into Maiz), S29 (the local model's first job), S30 (the runtime and the exported game), S31 (the community list) are new; S16 ruled (Allomone is Void Core's programming language), S17 decided, S18 accepted; S19–S22 new with decompilation; S23 (when the table moves onto Maiz) and S24 (a layer tag on every value) new 2026-10-11; S25 (Coral's polyps and players), S26 (wikis as a source), S27 (systems no other game has) new 2026-10-12; the author decides

# Sources

* [Sources index](/sources/index.md) - one page per external work the game model leans on, each tagged `asserted` or `verified`. Twenty-eight pages on 2026-10-12, five verified; the rest are the verification queue.

# Planned work

* [Roadmap](/roadmap.md) - rungs 2.5–6 are v0, interleaved with Void Jelly's; rungs 7–10 are the pillars, unscheduled
* [Log](/log.md) - what has changed in this bundle
* [App manifest](/app.md) - Void Squid's structured self-description; `void.json` at the repo root is the family registry's view of the same thing

# Sibling bundles

- **Void Jelly** — `../VoidJelly/okf` — the 3D library. Owns `.ink`, nibs, emitters,
  residuals, geometry. **Void Squid computes nothing about geometry.**
- **Void Core** — `../VoidCore/okf` — rune, mantle, holiday, glyph, dispatcher, tags,
  links. Void Squid owns none of it; the game model is a vocabulary over it.
- **Void Maiz** — `../VoidMaiz/okf` — the UI library Void Squid is built with; its
  view-as-projection seam is what pillar 2 is made of.
- **Void Palabra** — `../VoidPalabra/okf` — versions, joins, the content store; the game
  document's container (S12).
- **Void Allomone** — derive-only rules; the Dungeon Master's second job.
- **Void Bicho** — models behind a holiday; the Dungeon Master's third job, and since 2026-10-10 **a load-bearing layer for decompilation** (the author's decision; [decompilation](/design/decompilation.md) §8). **First code 2026-10-10**: SAM 2.1 and CLIP behind its invocation contract ([seeing what was decompiled](/design/seeing-what-was-decompiled.md)).
- **Void Loops Studio** — audio; the cue's engine. Not ready for integration.
- **Latin OS** — `../LatinOS/okf` — the operating system this forge will one day emit, and
  the second host it will run on. Its `the-os-as-a-game.md` is the mirror of our
  [what a game is](/concepts/what-a-game-is.md) §8.

Cross-bundle references are written `VoidJelly:/concepts/nib.md` (plain-text convention;
OKF links are bundle-root-relative, so foreign bundles cannot be linked directly).

## The dependency, stated once

**Void Squid depends on Void Jelly. Void Jelly does not depend on Void Squid**, and must
build, test and ship with this directory deleted from the disk. The same holds for Latin
OS in the other direction: Latin OS depends on nothing here, and this bundle never edits
it. Both are claims to check with a build, not with a promise.

# Inter-agent messages

Convention inherited from Void Maiz (2026-07-21): every message to another agent is a
uniquely titled file in the **recipient's** repo root,
`MESSAGE_FOR_<RECIPIENT>_<sender>-<topic>-<YYYY-MM-DD>.md`, with a matching
self-identifying H1. This index tracks open vs consumed; retire a thread here, not by
deleting the file.

**Consumed (received), all answered 2026-10-09:**

| from | file (in our root) | what it asked | what we did |
|---|---|---|---|
| Void Mago | `MESSAGE_FOR_VOIDSQUID_voidmago-you-need-a-void-json-and-a-catch-up-2026-10-04.md` | a `void.json`, a doctor run, a catch-up | all three; see [log](/log.md) |
| Void Jelly | `MESSAGE_FOR_VOIDSQUID_voidjelly-a-simulated-desktop-not-a-virtual-machine-2026-10-05.md` | nothing; tier 1 named as in scope | recognised in [studio surfaces](/concepts/studio-surfaces.md) |
| Latin OS | `MESSAGE_FOR_VOIDSQUID_latinos-the-operating-system-is-an-artifact-2026-10-09.md` | accept the restated §6; record *systems* as a horizon | both, as leans (S8); [railguard](/design/what-voidsquid-is-not.md) §6, [forge page](/design/the-forge-for-many-runtimes.md) |

**Open (sent 2026-10-09):**

| recipient | file | the ask |
|---|---|---|
| Latin OS | `../LatinOS/MESSAGE_FOR_LATINOS_voidsquid-the-artifact-never-ships-the-forge-2026-10-09.md` | if the author names the OS document (their Q11) or the game document (our S12), tell us which, so one shape does not get two names |
| Void Jelly | `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-the-game-model-is-not-yours-and-two-questions-2026-10-09.md` | does Q19's lean cover a game's terrain edits in the forge's play surface? is a rune id stable across a content reference, or should it carry a symbol table? |
| Void Mago | `../VoidMago/MESSAGE_FOR_VOIDMAGO_voidsquid-void-json-written-and-what-doctor-said-2026-10-09.md` | none; three findings from the doctor run |
| Void Core | `../VoidCore/MESSAGE_FOR_VOIDCORE_voidsquid-needle-moves-to-bicho-2026-10-10.md` | mark your Needle plan as moved to Bicho, or keep it as the authority? |
| Void Allomone | `../VoidAllomone/MESSAGE_FOR_VOIDALLOMONE_voidsquid-a-game-engine-reads-your-ties-2026-10-10.md` | surface within-source equal-strength ties from the library? and: tier 1 has a forcing client |
| Void Jelly | `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-an-id-buffer-exists-and-ink-needs-textures-2026-10-10.md` | will `.ink` carry UVs and textures? render-with-ID-buffer and the lift as pure verbs? |
| Void Jelly | `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-a-ds-reader-exists-and-it-is-yours-2026-10-10.md` | own the DS reader (command lists, textures) as the DS nib's inverse, with a lens test? per-axis dequantisation in `.ink`? |
| Void Jelly | `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-normals-need-provenance-and-lights-are-scene-data-2026-10-11.md` | should `.ink` say where a normal came from (decoded, rebuilt, absent)? do light settings and light areas belong in the scene or the game document? |
| Void Hormiga | `../VoidHormiga/MESSAGE_FOR_VOIDHORMIGA_voidsquid-we-adopted-your-schema-and-coral-is-not-an-antfarm-2026-10-12.md` | is the four-table schema versioned? may a tag's source live in a `tag.<word>` field? is your identity work general enough for Coral's players? |
| Void Bicho | `../VoidBicho/MESSAGE_FOR_VOIDBICHO_voidsquid-decompilation-makes-you-load-bearing-2026-10-10.md` | the author made Bicho load-bearing for decompilation: is training an invocation (seconding Latin OS)? is a benchmark a mantle of functions with oracles? will you name a `classify` kind? |
| Void Maiz | `../VoidMaiz/MESSAGE_FOR_VOIDMAIZ_voidsquid-two-hosts-one-shell-2026-10-12.md` | would you take the shell two hosts now share (a console over a Session, toasts, workspaces as dock windows)? Plus: a lock that records a pid |
| Void Loops Studio | `../VoidLoopsStudio/MESSAGE_FOR_VOIDLOOPSSTUDIO_voidsquid-game-music-is-midi-and-your-model-fits-2026-10-13.md` | when your compile pipeline exists, would you open a patch Void Squid writes (midi-in nodes per game track) from a command line? |
| Void Jelly | `../VoidJelly/MESSAGE_FOR_VOIDJELLY_voidsquid-the-studio-needs-to-see-a-turkey-2026-10-12.md` | **acted on 2026-10-13**, by the author's permission, from here: the glTF reader, materials and textures are in Jelly. OBJ is not |
| Void Maiz | `../VoidMaiz/MESSAGE_FOR_VOIDMAIZ_voidsquid-first-3d-host-findings-2026-10-09.md` | should `Camera` grow a 3D sibling, or does each 3D host keep its own? Plus five measured findings from the first 3D host (Maiz gitignores its inbox, so this file is local only) |
