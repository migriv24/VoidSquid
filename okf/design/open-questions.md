---
type: Design
title: Open questions
description: What is genuinely undecided in Void Squid, with a lean and what each blocks. The author decides.
tags: [status:planned, audience:dev, confidence:asserted]
timestamp: 2026-08-26T00:00:00Z
---

Convention inherited from Void Maiz: **the author decides.** Each question carries a lean,
and states **what it blocks** — a question that blocks nothing can wait.

Void Jelly's questions live in its own bundle (`VoidJelly:/design/open-questions.md`).
Three of them block work here: **§10** (does Jelly ship GL code?) blocks the
[viewport](/concepts/viewport.md); **§11** (who owns the DS host program?) blocks the
[build surface](/concepts/build-surface.md); **§5** (Jelly's language) settles this
bundle's §1 below.

---

## S1 — Does Void Squid need its own Void Core glyphs?

A `.ink` arrives with Void Jelly's glyphs (`sq-mesh`, `sq-light`, `sq-camera`,
`sq-anchor`). Does Void Squid register any of its own?

**Lean: no, not in v0.** Every glyph Void Squid registers is a piece of vocabulary that
exists only in the application, which means a `.ink` opened elsewhere loses meaning. If
something needs saying about a scene, it should be sayable in Void Jelly's vocabulary — or
it is a gap in Void Jelly worth reporting rather than routing around.

The first honest exception will arrive with gameplay
([engine horizon](/design/engine-horizon.md)), and by then there will be evidence.

*2026-10-09: the exception has arrived in the design.* The game model
([what a game is](/concepts/what-a-game-is.md)) is a vocabulary of glyphs, tag axes and
link kinds. The lean here stays **no** for the studio and v0; whether the game model
registers glyphs, and in whose namespace, is **S9**.

**Blocks:** nothing. Recorded because "just add a glyph" is the easiest boundary violation
available.

---

## S2 — Does Void Squid have a project concept, or only files?

Is there a "Void Squid project" (a `.ink` plus build settings plus target config), or does
it just open a `.ink` and take options on the command line?

**Options.** (a) Files only — open a `.ink`, pass `--nib ds`. (b) A project file holding
build settings. (c) Build settings live **inside the `.ink`**, as ordinary mantle content.

**Lean: (a) for v0, (c) as the destination.** (b) is a second file to keep in sync and a
second thing to version. (c) is more interesting than it looks: *"this scene targets the DS
with a 1800-polygon budget"* is a fact about the scene, it is exactly the kind of thing
Void Core's tags and rune content exist to hold, and putting it there means the build is
reproducible from the `.ink` alone. That is what makes a residual keyed by `.ink` hash
meaningful.

The caveat: it puts application policy into a library-owned format, so it needs Void Jelly's
agreement — a good first message to send them, once there is evidence rather than a
preference.

**Blocks:** nothing at rung 2.5. Decide before rung 6 (residuals as a corpus), which
depends on the answer.

---

## S3 — GUI first or headless first?

Void Maiz gives both, and building both at once is not the same as building both well.

**Lean: headless first, by a week or two.** Reasons: it is the smaller surface; it is what
CI and an agent use; a regression suite over residuals — the thing Void Jelly most needs —
wants a command line and not a window. And Maiz's own headless adoption path (~40 lines) is
easier to walk **toward** than to retrofit.

The counter-argument is real and worth stating: the [viewport](/concepts/viewport.md) is the
control experiment, and it exists only in the GUI. So the honest sequence is *headless
`build` first, GUI `view` immediately after* — not one long headless phase.

**Blocks:** rung 2.5's shape.

---

## S4 — How much of the residual does the GUI show?

A [residual](../../VoidJelly/okf/concepts/residual.md) for a large scene may have thousands
of entries, and Void Jelly's ASCII nib exists specifically to find out whether that drowns.

**Options.** (a) A flat table. (b) Grouped by cause (budget / not representable / quantised),
counts first, drill down. (c) Painted onto the viewport — click a crushed object and see
what happened to it.

**Lean: (b) first, (c) as the thing that actually justifies the concept.** (c) is the
feature that makes a residual *feel* like the point rather than like a build log — an artist
seeing the object that got crushed, in the scene, is a different experience from reading a
list. It is also rung 6 work and should not be attempted early.

**Blocks:** nothing. Recorded so (c) is not lost.

---

## S5 — Does Void Squid ever write a `.ink`?

v0 reads. Does it ever write — even just "save the camera position"?

**Lean: yes, and immediately, because it is unavoidable.** Camera state is model content
(`VoidMaiz:/concepts/views-as-projections.md`), so opening a scene and moving the view is
already a mutation. Which means the write path exists from day one and should be admitted
rather than discovered.

The consequence to accept openly: **`.ink` files will be edited by two applications**
(Blender via [the Polyp](../../VoidJelly/okf/concepts/polyp.md), and Void Squid), which is
`VoidJelly:/design/open-questions.md` §2 arriving from the other direction. It strengthens
the lean there — that this is a `VoidPalabra:/concepts/join.md` problem and not a
synchronisation problem.

**Blocks:** nothing technically, but it should be **said out loud before rung 2.5**, because
"it only reads" is a comfortable and false assumption to build on.

---

## S6 — When does the charter widen from harness to forge? *(new 2026-10-03)*

The author, 2026-10-03: Void Squid *"will be the engine to tie everything together,
combine it all into 1 thing, and be the ultimate generalizable game creation tool."* Read
through [engine horizon](/design/engine-horizon.md), that is **a forge for many runtimes**:
gather, compose, check residuals, and build to Bevy, the DS and others
([the forge for many runtimes](/design/the-forge-for-many-runtimes.md)).

The founding charter is still *"BARE MINIMUM … purely to be testing Void Jelly."* Both
statements are the author's, and they are not in conflict *if one comes after the other.*
The question is what marks "after".

**Lean: a test, not a date.** The charter widens when **one `.ink` has been emitted to two
targets with two correct, different residuals**. That means Void Jelly's rung 3 (DS) plus
either rung 2.5 (GL) or rung 2.5b (Bevy). Before that, every forge feature is a feature
built on an unproven pipeline. The railguard's question (*"am I building this because Void
Jelly needs it proved, or because it would be fun?"*) stays in force until then.

**Blocks:** nothing in v0. It decides when the railguard's §1 out-of-scope list starts to
shrink.

*2026-10-09:* the author has now described what the wider charter is
([the three pillars](/design/the-three-pillars.md)) and said *"we're changing a lot of
the foundations right now"* while agreeing that Void Jelly is not complete. The lean
stands as a test, with one refinement in S14: the game model is pure and can be built
before the S6 gate if the author rules so, because the gate's reason (an unproven
emission pipeline) does not touch it.

**Author, 2026-10-12: widened, for the studio.** *"right now let's actually create something, a real application i can launch with void maiz right now"*; and *"void squid is NOT about decompilation. It's about the creation of NEW things."* The studio was built before rung 3 ([Void Squid 0.1](/design/void-squid-0.1-the-studio.md)). What stays gated: the game model (S14) and the DS build (rung 3), which still waits on Void Jelly's DS nib.

---

## S7 — Three build targets in one repository, or three siblings? *(new 2026-10-09)*

The author: *"we may need to have separate packages, plugins, extensions, or whatever
you may call it."* [The three pillars](/design/the-three-pillars.md) §4 works out the
shape.

**Options.** (a) One target. (b) **Three targets in this repository** (`squid_model`,
`squid_studio`, `squid_host_*`) with a mechanical boundary check on what each may
include, after Jelly's `check_boundary.py` and Maiz's `voidmaiz` / `voidmaiz_view` /
`voidmaiz_headless` split. (c) Three sibling repositories now, each with a `void.json`.

**Lean: (b), promoted to (c) per pillar when a second consumer appears.** The precedent
is Void Allomone, extracted from Maiz on 2026-08-29 after it was already a clean unit
inside it. (a) makes the LITO host a port; (c) now is three manifests and three message
channels for code that does not exist. The game model is the pillar most likely to be
promoted first, because LITO's own system document and any Bevy host both want it.

**Blocks:** the shape of the first `CMakeLists.txt`, so it is decided before rung 2.5
even though only one target exists then.

---

## S8 — Does the railguard's §6 survive an operating-system artifact? *(new 2026-10-09; Latin OS's Q10)*

Latin OS proposes restating *"no Void Core ships to the target"* as **"the artifact
never ships the forge."** A DS cartridge stays Core-free; an OS image carries Core
because the OS is a rewriter; neither carries Squid.

**Lean: accept, with the drift test kept in the wording.** An artifact carries Void
Core only when Core is the artifact's *own* computation, never because it was easier
than compiling the forge's rules to data. The [railguard](/design/what-voidsquid-is-not.md)
§6 is already rewritten in that form, marked as a lean. If the author refuses, the
original wording is preserved beneath it. Jelly's agreement is also wanted, since the
same line runs through its nibs.

**Blocks:** nothing in v0. It decides what the LITO nib's emitter may contain, which is
Latin OS's rung, not ours.

---

## S9 — Does the game model register glyphs, and in whose namespace? *(new 2026-10-09)*

S1 says *no glyphs of Void Squid's own* and gives the reason: a `.ink` opened elsewhere
loses meaning. The game model needs a vocabulary: `character`, `region`, `beat`, `cue`,
`upgrade` and the rest of [what a game is](/concepts/what-a-game-is.md) §2.

**Options.** (a) Everything as tags on Jelly's `sq-anchor` (a rune with a transform and
no geometry). (b) **Glyphs in the game model's own namespace**, registered by pillar 1,
living in the *game* document, never in an `.ink`. (c) Ask Jelly to add them to `.ink`.

**Lean: (b).** (a) makes a character a point in space with tags, which is wrong for an
NPC that has no place yet and absurd for an operating system's process. (c) is the
boundary violation Jelly's client page forbids (*no gameplay vocabulary*). Under (b) the
`.ink` stays Jelly's and the game document references scenes by content reference; S1's
reason survives because an `.ink` opened elsewhere still means exactly what it meant.
Which nouns are glyphs and which are tags or links is pillar 1's first design task and
is not decided here.

**Blocks:** the game document's shape (S12). Nothing in v0.

---

## S10 — How is REA included: as a toolchain, or as a vendored pin? *(new 2026-10-09)*

The author: *"we should even INCLUDE 'reverse engineer anything'."* The facts are on the
[source page](/sources/rea-reverse-engineer-anything.md) (verified): a Node application,
MIT, that drives Ghidra, JADX and others and reports evidence and limitations.

**Options.** (a) **An ingress toolchain** in the build surface, discovered on the
machine like devkitARM, invoked as a process holiday, refused by default like every
effect; a pure *reader* maps its output into the game model. (b) A vendored pin of a
specific REA release under `tools/`, with its licence, driven the same way. (c) Reimplement
its contracts in C++. (d) Link nothing and reference it only in the OKF.

**Lean: (a), and (b) is acceptable if the author wants a reproducible version.** (c)
gives a working MIT tool a second owner for no gain. (d) is not what the author asked.
Rule 10 is about code the binary links, and a toolchain is not that; `npx` is the
consumer's package manager, as `cargo` is for the Bevy nib. Argued in
[construct and deconstruct](/concepts/construct-and-deconstruct.md) §4.

**Blocks:** nothing before the first ingress rung. It decides one row of the build
surface.

---

## S11 — Text, narrative, image and video: siblings, or content on runes? *(new 2026-10-09)*

[What a game is](/concepts/what-a-game-is.md) §6 lists the family's modalities and finds
two gaps: there is no owner for text and narrative, and none for image and video.

**Options.** (a) **Content on runes** in the game model: a beat's text is a string, a
texture is a block, and the structure (beat graph, references) is pillar 1's. (b) Found
siblings now (a narrative library; a media library). (c) Ask Jelly to carry textures and
video as blocks in `.ink`.

**Lean: (a) until a second consumer wants the computation.** A narrative library earns
its existence when something *computes* over text (a director choosing beats against an
arc, a dialogue lens to and from a writer's tool); until then the structure is links and
the text is content. (c) is partly true already for textures through the Polyp, and
video is a question for Jelly, not a decision here. Founding a sibling is the author's
call, and this bundle does not found one.

**Blocks:** nothing. Recorded because a collage with two missing media should say so.

---

## S12 — What is the game document, and what is it called? *(new 2026-10-09)*

An `.ink` is a scene. A game is a world that contains scenes, plus characters, rules,
beats, cues and an upgrade graph that have no geometry. S2 asked whether Squid has a
project concept; the game model answers that it has a **document**.

**Options.** (a) Everything in one `.ink`, by asking Jelly to widen it. (b) **A Palabra
container holding the game mantle, which references its scenes (`.ink`s) and its other
media by content reference.** (c) A directory of files with a manifest.

**Lean: (b).** It keeps `.ink` Jelly's (railguard §4), gives versions and joins for
free (Jelly's own destination for `.ink`, its Q4), and makes "a game" a single
shareable, forkable thing, which is what the author's and Latin OS's *document* framing
wants. S2's lean (build settings as mantle content) is unchanged and lands in this
container. **The name is the author's**, as `.ink` and `.miga` were; Latin OS's Q11 asks
the same question for the operating-system document, and one answer may serve both.

**Blocks:** the first `squid open` that opens something other than a bare `.ink`. Not v0.

**2026-10-12: a provisional name, `.squid`.** The author asked whether the project file "was a .ink file". It is not: `.ink` is Jelly's scene. Void Squid 0.1 writes a Void Core state document under the extension `.squid` ([the studio](/design/void-squid-0.1-the-studio.md) §2), held in one constant so the author's own name costs a rename. Lean (b) is unchanged for the game document; the `.squid` project is the step before it, a Core document with no Palabra container yet. **The name is still the author's.**

---

## S13 — Where does the generative part of play run, and may the forge ever hold a model? *(new 2026-10-09)*

The Dungeon Master's third job, generating, is the only part of a game that wants a
model at play time ([what a game is](/concepts/what-a-game-is.md) §2). The author:
*"we want some sort of AI in this."*

**Options.** (a) **In the artifact only**, behind a Void Bicho holiday, steering
structure the forge authored (beats, an arc, NPC relationships). (b) Also in the forge,
as an authoring assistant that proposes beats, stats and cues through the same holiday,
with every proposal landing as an ordinary dispatcher command the author can revert. (c)
In the forge as a source of truth (the model *is* the DM at authoring time).

**Lean: (a) now, (b) when Bicho exists, never (c).** (b) is what Void Maiz's
agent-drivable dispatcher was built for and costs the model nothing new. (c) breaks
*Void Squid owns no truth*: a model's output is a proposal, and the mantle is the record
of what was accepted. The two-clocks law holds throughout: the artifact's director runs
at play rate; the forge's assistant runs at model rate and never reaches into a running
artifact.

**Blocks:** nothing. Void Bicho is design-only.

---

## S14 — When is the game model built, relative to the S6 gate? *(new 2026-10-09)*

S6 gates the charter's widening on *one `.ink`, two targets, two correct residuals*. The
gate's reason is that forge features built on an unproven emission pipeline are the
*while-we-are-here* trap. The game model is pure, emits nothing and opens nothing, so the
reason does not touch it ([the three pillars](/design/the-three-pillars.md) §5).

**Options.** (a) Hold it to S6 regardless. (b) **Design it now; build it when the author
rules the charter widened, or at S6, whichever comes first**, as a pure target with its
own tests and no studio around it. (c) Build it now.

**Lean: (b).** (c) is what the railguard's §7 exists to catch: it would be built because
it is the most interesting thing in the bundle, which is a fine reason and not a reason
for *now*. (a) ignores that the author has already asked for the design and that a pure
library with no host is exactly what the founding log said Void Jelly's first rungs could
be. The author decides, and this is the question that most wants that ruling.

**Blocks:** the first line of code in pillar 1.

---

## S15 — Are Turkey, Table and Feast new verbs, or aliases over Core's? *(new 2026-10-09)*

The author's creation vocabulary ([Turkey, Table, Feast](/concepts/turkey-table-feast.md))
is meant to make programming and logs clearer. Each word maps to one Core construct
(rune + glyphs, mantle + rules, the ordering of tables + holidays).

**Options.** (a) New dispatcher verbs registered by the game model. (b) **Aliases that
lower to Core commands**, with the journal recording both. (c) GUI names only; the log
stays in Core's words.

**Lean: (b).** (a) is a second command language, which the family has refused every
time (Maiz deleted its argument-codec copies; Jelly adds no command language of its own).
(c) loses the author's stated reason, clearer logs. Under (b) `turkey new goblin` and
`rune new character goblin` are one journal line in two vocabularies.

**Blocks:** the game model target's command surface (S14).


**Author, 2026-10-10:** *"turkey, table, and feast are new things, however they share a great amount of meaning with rune, mantle, and holiday. to the point where im still deciding if it makes sense to make them separate things."* **Open, with the author.** The lean (aliases) still holds for the command surface either way: if they become separate things, they are separate *workflow* nouns whose commands still lower to Core's, so the journal has one language. Code uses the words where a workspace is meant (`table.cpp` compiles a table) and Core's words where the substrate is meant.
---

## S16 — Is Allomone the programming language of the engine? *(new 2026-10-09)*

The author: *"maybe it IS the programming language of this game engine?"* Allomone's own
pages split the job: Allomone derives (built, always terminates), Voidscript mutates, and
*"Allomone compiles to Voidscript"* when tier 1 opens.

**Lean: Allomone for what is true on a table, now; for amendment when its tier 1 lands;
Voidscript for what happens until then; the runtime's own code at frame rate in a shipped
game.** Argued in [Turkey, Table, Feast](/concepts/turkey-table-feast.md) §3. The part of
game logic that is declarative (alertness, visibility, music cues, moods, who may see
what) is larger than it looks and is Allomone's today. Squid is a strong forcing client
for Allomone's tier 1, which is worth a message to Allomone once a table exists to test it.

**Blocks:** nothing built. Decides what the Table window edits.


**Ruled by the author, 2026-10-10:** *"yes allomone is a programming language, sort of, more like a translation of interaction rules over a mantle into an interface that's easier to understand (so a programming language but for void core)."* **Settled.** First use: a table's simulation rules ([simulation](/design/simulation.md) §4), where Allomone decides who moves, who blocks, who triggers and where they go. One finding sent upstream: equal-strength ties within a script are settled silently by text order.
---

## S17 — What steps time in a simulation? *(new 2026-10-09)*

[Three uses](/design/three-uses.md) §4. The forge never steps physics at frame rate.

**Options.** (a) An existing runtime the forge drives and observes (Bevy over its remote
protocol; a physics library in a small Squid-owned runner process). (b) **A model-rate
simulation in the document**: discrete events as dispatcher commands, a few steps a
second, fully logged and replayable. (c) Both, chosen per simulation.

**Lean: (b) first, as the cheapest proof, then (c).** A car on a road or a D&D encounter
at model rate needs no new dependency and exercises the spatial log's "what changed in
view". Frame-rate physics is (a) and is a separate process, never inside the forge.

**Blocks:** the simulate workspace. Nothing in v0.


**Delegated by the author and decided, 2026-10-10: (b) first, with the multi-instance shape built in from the start.** A model-rate runtime (`squid_sim`) that links nothing; the forge compiles a table into data; many instances share one world; fork copies dynamic state; the log is events. Built and measured: [simulation](/design/simulation.md). (a), an external runtime, stays the route for real physics, behind the same World / Instance / events boundary.
---

## S18 — The asset library: a catalogue mantle plus an embedding model? *(new 2026-10-09)*

The author: *"we might need a LIBRARY of assets, or methods of asset generation … presets
attached to some textual embedding model maybe."*

**Lean:** a catalogue mantle of asset runes (content-addressed blocks, tags, thumbnail),
searched by tag query first and by an `embed` model in Void Bicho second; generation
(image-to-3D) as a later Bicho adapter whose output enters as a proposal. The catalogue
shape is the one Bicho already uses for models, so it costs no new idea. Where asset
blocks are stored is Palabra's question, not ours.

**Blocks:** nothing now.


**Accepted by the author, 2026-10-10**, with the scope for now: *"right now we're fine with basic shapes and primitives … eventually i would wanna add things with more semantic meaning."* First shelf built: Void Jelly's primitives (box, plane, cylinder, sphere, cone) and two preset scenes in Void Squid (`effect make traffic|shapes`). The semantic library (embeddings, Needle's embedding head is a candidate) waits.
---

## S19 — universal-modder: a reference, a toolchain, or a fork, and where would a fork live? *(new 2026-10-10)*

The author: *"i essentially want to fork this universal modder in such a way where we can
eventually get to complete local decompilation of games."* The facts are on the
[source page](/sources/universal-modder.md) (verified): MIT; Python CLI, agent skills and a
knowledge base; PC games; no console headers.

**Options.** (a) **Reference only**: the trimmed copy in `third_party/`, read, never run.
(b) **A toolchain**: the person installs `um`; Void Squid discovers it and invokes it
through the ingress row of the [build surface](/concepts/build-surface.md), like REA (S10).
(c) **A fork in its own family repository**: a Python toolchain whose agent-driven stages
are replaced one at a time by tools and Bicho's small models, local-first, emitting a game
document instead of a mod folder; invoked as in (b). (d) Fold it into Void Squid's C++.

**Lean: (a) now, done; (b) when the ingress row exists; (c) when the author says go, with a
name the author gives.** (d) would put a Python application inside a C++20 build and give a
working MIT tool a second owner, which is rule 12's purpose defeated. A fork is honest only
once there is a first stage to replace with something measured against the agent baseline
([decompilation](/design/decompilation.md) §8, §9), and recon is the first such stage,
built here in C++ because it is pure and small.

**Blocks:** nothing in v0. Decides where decompilation's tool code lives.

---

## S20 — Does decompilation move the order: the first ingress, and S14? *(new 2026-10-10)*

The author, 2026-10-10: *"decompilation is going to be key for this sort of project."* The
[roadmap](/roadmap.md) puts the first ingress at rung 9, after the first two egresses have
residuals to calibrate against, and the game model behind S14.

**Options.** (a) Keep the order. (b) **Split the ingress by depth**: recon, the toolchain row
and readers to the bytes depth proceed now, because they are host and reader work and need
neither the game model nor a second egress; *identify* and *lift* wait for the game model
(S14). (c) Move all of decompilation, including the game model, ahead of rung 3.

**Lean: (b).** It is what was done today (recon) and costs v0 nothing: the DS nib still
gates rung 3, and the first useful ingress test (*read back the `.nds` our own nib wrote*)
needs rung 3 anyway. (c) would build the game model because decompilation wants it, which is
a real reason, but it is S14's ruling to make, and this question only says that
decompilation is now a second client asking for it.

**Blocks:** the identify and lift stages; whether S14's ruling should come sooner.

---

## S21 — Is an opaque piece's contract a Void Core holiday, or a game-model noun? *(new 2026-10-10)*

[Decompilation](/design/decompilation.md) §4: a piece recovered only to the opaque depth
(libsm64's Mario; a recompiled function; an emulated subsystem) enters the document as a
rune whose behaviour runs behind a contract: ownership, units, channels, lifecycle, *not
covered*.

**Options.** (a) **A holiday in Core's sense**, with the contract as content and links on the
rune that stands for it: ownership as `controls`/`owns` links, units as data on a link,
hand-back as rules, *not covered* as the residual. (b) A new game-model noun, *contract*,
with its own glyph. (c) A new kind of thing in Core.

**Lean: (a).** It passes railguard §8's first test (sayable in Core's vocabulary) without a
new kind of thing, and the recoverability test by construction (the mashup community
recovers contracts from games nobody here made, by hand, today). If (a) turns out to need a
holiday to carry a declared interface Core cannot express, that is a finding for Void Core
and goes there as a message, which is (c) arrived at honestly.

**Blocks:** the lift stage for opaque pieces; nothing before it.

---

## S22 — Is the decompile shelf the game document, rune for rune? *(new 2026-10-10)*

The [DS mashup exercise](/design/ds-mashup-exercise.md) needed a document no format
provided. For every piece it records the game, the file inside the cartridge, the depth, the
oracle, the residual and the units. It also holds the characters, each one identity with
presentations by role, and a glossary from internal names to canonical and display names.
It was written as `shelf.json`.

**Options.** (a) **Adopt it as the game document's decompile-workspace form (S12)**, mapped
onto runes: a piece is a rune whose heavy content is content-addressed blocks; depth and
oracle are tags; the residual is content; an identity is a rune with `presents-as` links to
presentation runes keyed by role; the glossary is a mantle of name runes with
`internal-name-in` links. (b) Keep it as an interchange file the studio imports. (c) A
new container format.

**Lean: (a).** It passes railguard §8's three tests: sayable in Core's vocabulary, pure,
and recovered from three games nobody here made. (b) gives the studio a second source of
truth (rule 6). (c) is the format-with-two-owners problem the railguard's §4 warns about.

**Blocks:** the studio's shelf surface; S12's shape, which this would partly answer.

## S23 — When does the turkey table move into Void Squid, on Void Maiz? *(new 2026-10-11)*

The first table ([Goomba Galaxy 2 and the turkey table](/design/goomba-galaxy-2-and-the-turkey-table.md))
is an exercise in `../ds-mashup`, in raylib with raygui. It edits one world document, writes
a transcript in the table's words, and its saves are played unchanged by a game. It shows
the vocabulary works. It also breaks two rules a studio surface keeps: it owns pixels, and
it loops over vertices (to pick surfaces and rebuild normals).

**Options.** (a) **Port it now** as Void Squid's first studio surface on Void Maiz, with
Void Jelly picking surfaces and Bicho's shelf as the turkey source. (b) **Keep growing it in
the exercise** until S14 rules on the game model, then port the shape, not the code.
(c) Make it a Void Maiz sample, owned there.

**Lean: (b).** The table's document is the world file, which is not yet the game
document (S12, S22). Porting before S14 would fix a format by accident. The exercise
costs nothing to keep, and every finding in it is already written here.

**Blocks:** pillar 2's first surface; S12 and S22.

**Author, 2026-10-12: port now, onto Void Maiz.** The studio's shape (Turkey, Table, Feast as workspaces; Hormiga's windows, tabs and Data section) moved into Void Squid. The 3D table's placement did not: it would bring raylib and a loop over vertices, so it stays in the exercise and the studio starts it as a process until Void Jelly reads a glTF (message sent). The world files are still the exercise's format, so S22 stays open.

## S24 — Should every value in a world say which layer it came from? *(new 2026-10-11)*

The author, 2026-10-11: *"we are always adding MORE to the original base game."* Every
world built so far mixes five layers: the game read; other games read; chosen by meaning;
written here; placed by a person ([Goomba Galaxy 2](/design/goomba-galaxy-2-and-the-turkey-table.md) §4).
Today the layer is recorded on objects (`source`, `added`, scores) but not on every
value (a light colour, a camera distance, a gravity range widened by the orbit test).

**Options.** (a) **A layer tag on every rune and every changed field**, in the game model
(Void Core tags make it cheap). (b) Per object only, as now. (c) A separate provenance
document.

**Lean: (a).** The orbit test's widened ranges are exactly the edit that disappears
under (b): a value from the disc, changed for a reason, with nothing on the value saying
so. Core's tags already carry this kind of fact. It also answers the honesty rule (11) for
data, not only for pages.

**Blocks:** nothing now; it shapes S12 and S22 when they are ruled.

## S25 — In the Coral, is a polyp a rune, a mantle, or a file? And who is a player? *(new 2026-10-12)*

[Coral](/concepts/coral.md) is the author's name for Void Squid's sharing layer. Its journal is
built and local. Two things decide its shape before any network: what a shared thing is,
and what identifies a player.

**Options for a polyp.** (a) **A rune** (a turkey's record) or **a mantle** (a table), in Void
Core's words, so sharing is syncing runes and mantles (Void Palabra). (b) A file (the world
JSON as it is today). (c) Both: files now, runes once the game model exists (S14).

**Lean: (c).** The journal already speaks in mantles and runes, so nothing written today
has to be rewritten. The files stay the transport until S12 and S22 settle the document.

**Who is a player:** for now, the account name on the machine. Void Hormiga's identity
work (`VoidHormiga:/concepts/platform/identity.md`) is the obvious place to borrow from.

**Blocks:** nothing now. It shapes multiplayer.

**Author, 2026-10-12: the line drawn.** Coral is game data shared between players *while a game is active*: hosts, servers, sessions. Sharing the **editing** is Void Maiz's networking, which a project built on a Maiz Session gets without Coral ([Coral](/concepts/coral.md)). So a polyp is play-rate state, and the edit journal is the project's own `.squid.log`.

## S26 — Wikis as a layer of meaning: how far, and with what credit? *(new 2026-10-12)*

The author proposed using each game's wiki for meaning. About 1,000 pages for four games
now tag turkeys ([dk64_lib and the game wikis](/sources/dk64-lib-and-the-game-wikis.md)).
Wiki text is CC BY-SA, and nothing from it leaves the local cache and database.

**Options.** (a) Keep wikis as a **proposal source** like vision: tags marked `wiki:`, a person
confirms. (b) Treat a wiki match as truth. (c) Stop at game pages and lists.

**Lean: (a).** A wiki is human-made and usually right about *what a thing is*, but a match
is made here by name and romanization, and a wrong match is easy (two things with one
name). It stays a proposal with its source on every tag, as rule 6 asks.

## S27 — When a game brings a system no other game has, does the shared vocabulary grow? *(new 2026-10-12)*

The author: *"the more games we add, the more abstract the mathmatics is going to get."* So
far one system subsumes the others: Galaxy's gravity fields contain every other game's
gravity as a case. Minecraft (terrain generation, redstone) and Skyrim are next on the
author's list, and each brings systems nothing here has.

**Options.** (a) **Grow the shared vocabulary** whenever a new game's system can be stated in
Void Core's words, and keep a residual for what cannot. (b) Keep each game's systems its own
and translate only on demand. (c) Learn a shared space ([cross-game transfer](/sources/cross-game-transfer.md)).

**Lean: (a), with (c) as a proposer.** Structure mapping is what decompilation makes
possible. A learned model may suggest a mapping, and the mantle records what was accepted
(S13).

**Blocks:** the game model's noun list (S14).

---

## S28 — Should the studio shell move into Void Maiz? *(new 2026-10-12)*

Void Maiz draws its line at application chrome: menus, toolbars, status bars are the host's
(`VoidMaiz:/concepts/widget-registry.md`). Void Hormiga wrote a docked shell (a menu bar,
sections as dock windows, a console, toasts, a busy overlay); Void Squid 0.1 wrote the same
one again ([the studio](/design/void-squid-0.1-the-studio.md) §4).

**Options.** (a) Keep it per host. (b) **A small optional Maiz component** for the shell:
the dock layout seeded once, workspaces as dock windows with focus, a transcript console
over a Session, toasts. (c) Wait for a third host.

**Lean: (b), Maiz's decision.** Two hosts with one shell is the evidence Maiz asks for, and a
shared console over a Session is what makes every host's transcript look alike. Sent as
`../VoidMaiz/MESSAGE_FOR_VOIDMAIZ_voidsquid-two-hosts-one-shell-2026-10-12.md`.

**Blocks:** nothing; Void Squid keeps its own until Maiz answers.

## S29 — What is the local model's first job, and how is it held to account? *(new 2026-10-12)*

The author calls the local-model setup "normal" ([Load a game](/design/load-a-game.md) §3).
Void Squid detects Ollama and its models and asks them nothing yet.

**Options.** (a) **Names and tags in the Turkey workspace**: for a turkey with few tags or no
wiki match, the model proposes from the game's own data, the vocabulary and the thumbnail's
vision tags, as a JSON schema; each proposal is checked (against the vocabulary, the twelve
kinds) and lands with source `local:<model>`. (b) Reading logs and CLI output for the person.
(c) Writing ingest routes.

**Lean: (a) first.** It is the job with the most checks already built, and its failures are
visible chips a person can click away. (c) is the agent setup's. The hosted-model setup and
its storage (Void Verguenza is the candidate) wait, by the author's decision.

**Blocks:** nothing.

**Measured, 2026-10-13** ([decompilation progress](/design/decompilation-progress.md)): the
author asked that the minimum setup be tested with small models, so (a)'s matching half was
built as a pilot with seven guards and scored on 25 turkeys whose page is known. With the guards,
llama3.2:1b and gemma4:e2b accept **no wrong answer** and find 14 and 17 of 25; qwen3:0.6b
finds 7. The rule alone finds 18 and gets 7 wrong. **Lean, updated:** the MOSU may include a
model of about 2B parameters, never as the authority: a proposal, from a shortlist, twice, under
rules. Needle itself was not run; the hard cases (no English in the name) have no ground truth.

## S30 — The runtime, and the exported game *(new 2026-10-12)*

Play starts the exercise's runtime (`goomba_galaxy2`, raylib and Jolt). The author wants games
exported as programs, for Windows and Linux.

**Options.** (a) Move the runtime into Void Squid as the first **artifact host**: an exported
game is that runtime plus the feast, the tables and the person's own library, built for a
platform. (b) Keep the runtime outside and only write files it reads. (c) Wait for the game
model (S14) and write the runtime against it.

**Lean: (a), shaped by (c).** Railguard §6 holds either way: the artifact never ships the
forge, and a game ships no Void Core. The open part is what an exported game may carry of a
library made from someone's own disc: by today's rule, nothing they could not rebuild.

**Blocks:** a release that can Play without the exercise.

## S31 — The community decompilation list: download, or ask for a licence? *(new 2026-10-12)*

Load a game matches against awesome-game-decompilations, downloaded on the person's machine
because the list has no licence file ([Load a game](/design/load-a-game.md) §6).

**Options.** (a) Keep downloading on request. (b) Ask its author for a licence and ship a
snapshot. (c) Grow our own list.

**Lean: (a) now, (b) if its author agrees.** A snapshot would let Load a game work offline;
the seed already covers the games Void Squid has routes for.

## S32 — Does `.ink` grow skins? *(new 2026-10-13)*

Twenty characters now have skeletons, weights and animations ([the cast](/design/the-cast.md)),
in the exercise's rig format. Void Jelly's glTF reader records skins and animations as
omitted. So the studio shows a character in bind pose and only the runtime animates it.

**Options.** (a) Jelly grows armatures: joints as runes, weights and inverse binds as blocks,
animations as blocks a rune names; the GL nib emits a pose. (b) Skins stay outside `.ink`, in
the game document (S12), and the runtime is the only skinner. (c) The rig format stays the
exercise's until a nib needs skins (the DS has them).

**Lean: (a), asked of Jelly, not done from here.** A skeleton is structure, which is Jelly's
job, and the DS nib will need it. Posing is a loop over vertices, which railguard §4 says is
never Squid's.

**Blocks:** a character that moves in the studio.

**Pressure, the evening of 2026-10-13:** Void Onda now owns poses (S2 there, the same lean): Jelly the
skin as structure, Onda the pose, the host the draw.

## S33 — When does Void Squid link Void Ocean? *(new 2026-10-13)*

[Void Ocean](../../VoidOcean/okf/index.md) was founded beside Void Squid: fields, gravity
primitives, a terrain lift, and the commutation defect that measures a physics translation.
Pure; nothing links it.

**Options.** (a) Now, for a Physics view in the Table (the gravity field drawn over a level).
(b) When the runtime moves in (S30), so the runtime's gravity is Ocean's. (c) When Void Core
says what a field is (Ocean's S1).

**Lean: (b).** A view of a field the runtime does not use would be a picture of nothing;
Ocean's rung 2 (Galaxy's gravity read from a level and rasterised) can be tested from its own
repository first.

**Blocks:** nothing now.

**Measured, 2026-10-18** (lean unchanged): the seam Ocean's rung 2 will cross, read from both
sides' code and counted on Good Egg Galaxy's world. 49 of its 109 gravity fields have no exact
Ocean primitive (bounded planes, rotated boxes), and 14 carry only up to `distant`. That is
Ocean's to answer, in its repository
([motion and physics in the studio](/design/motion-and-physics-in-the-studio.md) §2).

## S34 — Void Onda in the studio *(new 2026-10-13)*

[Void Onda](../../../VoidOnda/okf/index.md) was founded for motion and its judging; the runtime links
it. The studio does not.

**Options.** (a) The studio links Onda for a Motion workspace: clips by verb, the judge's verdicts per
character, the gallery in the studio. (b) The studio shows the runtime's `judge.json` and opens the
runtime's gallery as a process. (c) Wait for the game model (S14).

**Lean: (b) first, (a) when the runtime moves in (S30).** The judge already writes everything the
studio would show.

**Progress, 2026-10-18:** (b)'s first half is built as `effect motion judge|report`
([motion and physics in the studio](/design/motion-and-physics-in-the-studio.md) §1): 105 clips,
mean 99.7%, lowest 97.0%, through the studio's headless front-end. Still open in (b): a Motion
window, and the gallery opened on one clip, which needs a runtime flag the runtime does not have.

## S35 — Small models: which first, and in what? *(new 2026-10-13)*

The author: many small local models, not one large one; a small model behind the 3x3 gallery;
C and ONNX rather than Python. Void Onda's [small models](../../../VoidOnda/okf/design/small-models.md)
found no small general optimiser ready to drop in, and built the gallery without one.

**Lean:** a preference model fitted on the gallery's pairs (C++), then a network through ONNX
Runtime's C API only when pooled data exists. The naming pilot (S29) stays the first model in the
studio.

## S36 — Music and Void Loops Studio *(new 2026-10-13)*

[Music and Void Loops Studio](/design/music-and-void-loops-studio.md). **Lean:** describe now (game
sequences as MIDI in the workbench), open VLS as a process at its milestone 3, react through its
engine at milestone 4.
