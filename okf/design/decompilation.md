---
type: Design
title: Decompilation — a game is a modality, taken apart and put back together
description: "The author's 2026-10-10 direction: Void Squid decompiles games as well as compiling them, not to have source for its own sake but to recover game objects and their behaviours, shelve them like a toolbox, and compose new games from several decompiled ones in the studio. What exists in the world (matching decompilation, static recompilation, mashups, universal-modder, learned decompilers); four depths of recovery, each with its oracle; why holidays carry what two games do not share; the player taken apart; the pipeline and who owns each stage; auto and by-hand as one surface; Void Bicho as a load-bearing layer; what forking universal-modder would mean."
tags: [status:planned, audience:dev, audience:all, confidence:exploratory, foundation]
timestamp: 2026-10-10T00:00:00Z
---

> The author, 2026-10-10: *"just as void squid can COMPILE a video game, we should also be
> able to DECOMPILE a video game … we don't just want source code to have source code …
> we want to actually give STRUCTURE to the source code we get from some given binaries.
> we want to extract game objects themselves and their behaviors."* And: *"you plug in a
> game into void squid, and a loading progress bar begins like 'decompiling game'. and
> then you can get a whole series or set of things to choose from to build your world."*
> And: *"Void Bicho is going to be an EXTREMELY IMPORTANT layer to void squid."*
>
> *Later still: [seeing what was decompiled](/design/seeing-what-was-decompiled.md) built the
> identify stage's vision half (§6) and measured it; vision answers who, not what it does.*
>
> *Later on 2026-10-10 the whole pipeline was walked once, by hand and by agent, on three DS
> games: [the DS mashup exercise](/design/ds-mashup-exercise.md). Read it next; it tests
> every section below against real cartridges.*
>
> **Built today: recon only** ([recon](/concepts/recon.md), `status:current`), the first
> stage of §6. Everything else on this page is design. It extends
> [construct and deconstruct](/concepts/construct-and-deconstruct.md), which already holds
> the architecture (readers are the inverse of nibs; every reader returns a residual), and
> [three uses](/design/three-uses.md) §2, which named the decompile workspace.

# 1. A game is another modality

The author's framing: *"video game files are going to be very similar to video files or
.blend files. A video game is just another modality for data to exist in."* Blender is
the precedent. A person downloads models in FBX, OBJ and glTF from anywhere, imports them,
and once inside they are all the same kind of thing, to be organised and modelled with.
Nobody asks Blender to be the program that made them.

Void Squid's version: **readers are the importers** (the inverse of nibs), and what comes
in is a shelf of pieces in one vocabulary, the [game model](/concepts/what-a-game-is.md)'s.
Players, maps, enemies, items, rules. Roblox's toolbox is the interaction the author
pictures: drag a piece from the shelf into the world. In the family's terms that is one
dispatcher command that links or copies a rune into a [table](/concepts/turkey-table-feast.md),
which means it is logged, undoable, and available to an agent as well as a person.

What makes this harder than Blender's importers is stated once, up front: **a mesh is
data, and a game's most important parts are behaviour, which is code.** Blender never had
to import a mechanic. Everything below is about what to do with behaviour.

# 2. What exists in the world, 2026-10-10

Six traditions already take games apart. Each reaches a different depth (§3), and **each
already has an honest metric**, which is the most useful fact on this page: every one of
them is an ingress residual in this family's sense, under another name.

| tradition | what it produces | how it is checked | depth | source |
|---|---|---|---|---|
| **matching decompilation** (Super Mario 64, Ocarina of Time, Metroid Prime, about 400 projects in one community list) | C that compiles back to the **identical** bytes | per-function byte match (objdiff); the whole game relinked byte-for-byte | source | [matching decompilation](/sources/matching-decompilation-and-recompilation.md) |
| **static recompilation** (N64Recomp and Zelda 64: Recompiled; Xbox 360, PS2, GameCube equivalents) | machine code translated to C, linked against a runtime that reimplements the console | the game runs natively and plays | opaque | same |
| **emulation and memory scripting** | nothing recovered; the original runs, addresses found by RAM search | the scripted effect happens in the running game | opaque | [universal-modder](/sources/universal-modder.md) |
| **engine recreation** (IW4L, benilla, a Skate 3 engine in Rust) | a new engine that reads the owned game's data | a round trip or byte match per format; side by side with a recompilation as oracle | structure, per game | same |
| **agent-driven modding and mashups** (universal-modder, REA) | mods, two-game mashups, written field notes | six evidence levels, from *creator report* to *real run* | any, per project | same; [REA](/sources/rea-reverse-engineer-anything.md) |
| **learned decompilers** (LLM4Decompile, SK2Decompile, D-LiFT, RlDecompiler) | C from a binary, by a model | re-executability: does the output compile and pass the original's tests | source | [learned decompilers](/sources/learned-decompilers.md) |

decomp.dev's progress bar (*"90.83% decompiled, 54.69% fully linked"*) is **exactly an
ingress residual**: the fraction of a binary for which the reader recovered something that
reproduces the original, and, implicitly, the list of functions for which it did not.

# 3. Four depths, each with its oracle

A piece of a game (a function, an asset, a mechanic, a level) is recovered to some
**depth**. Recon (§6) is the zeroth step; the four depths are a ladder. A piece does not
have to reach the top to be usable, which is the design's main freedom.

| depth | what is recovered | the oracle: how we know it is right | residual names |
|---|---|---|---|
| **bytes** | assets and binaries extracted and listed (meshes, textures, sounds, tables, code segments) | **round trip**: repacking what was extracted gives the original container. Void Jelly's lens law, `read(write(x)) == x`, applied to someone else's format | formats the reader could not parse |
| **opaque** | the original code *runs*, behind a contract (§4): an emulator, a recompilation, a loader in the game folder | **it behaves as the original**: a recorded scenario replays identically | what the contract does not expose |
| **source** | the code read as source: decompiled, matching where a community has done it | **matching**: `compile(source) == binary`, per function. The lens law at the source depth, and the reward a learning system can trust (§8) | unmatched functions; names and types that are guesses |
| **structure** | the piece lifted into the game model's nouns: an avatar, a character with stats, a region, a rule, an upgrade edge | **re-emission reproduces behaviour against the opaque oracle**: Mario's lifted jump arc, run in a new runtime, matches the arc the original produces, within a stated tolerance | the mechanic with no noun; the trigger with no rule ([three uses](/design/three-uses.md) §2) |

**The progress bar the author pictured is this table, counted.** *"Decompiling game…"*
fills as pieces reach each depth: perhaps 100% at bytes, 60% at opaque (everything that
runs under the emulator with a known address), 30% at source, 4% at structure. Each
number is honest because each has an oracle. A bar that said "100%" for a game whose
behaviour had only been extracted as bytes would be the lie
`VoidJelly:/concepts/nibs/trivial.md` warns an empty residual tells.

**The useful consequence: a piece at the opaque depth is already a piece.** Super Mario
64's movement has been recovered to structure by nobody. It has been recovered to the
opaque depth as a library, and that was enough for people to put Mario into Garry's Mod
and Elden Ring (§4). The shelf can hold pieces at any depth, as long as each says which.

# 4. Holidays, a lot more: what Minecraft and Mario 64 have in common

> *"what would a game like minecraft and mario 64 have in common? technically, they
> generate their worlds differently! technically, they have completely different
> systems. Yet, modders have figured out some fundamental baselines."*

At the level of code, nothing. At the level of **contracts**, a great deal, and the
mashup community has already written the contract down.

**libsm64** turns the Super Mario 64 decompilation into a library: *"feed it collision
and input, and it returns Mario's state and mesh."* That sentence is a holiday's
signature. Mario's movement code is opaque to the host; what the host knows is what goes
in (collision queries, controller input), what comes out (a pose, a mesh, a state), and
nothing else. Two hosts with nothing else in common (Garry's Mod, Elden Ring) both ran
Mario, because both could answer *"what is solid near this point?"*.

universal-modder's **bridge contract** ([source](/sources/universal-modder.md)) is the
generalisation, learned from about thirty published mashups. Its fields, and where each
lives in Void Core's vocabulary:

| contract field | what it says | in Void Core |
|---|---|---|
| route and exact versions | which build of each program | content on the piece's rune; part of its provenance |
| **ownership table** | who owns player movement, camera, world geometry, collision each way, NPCs, health, inventory, saves, menus, scripted events; the hand-back trigger for each | **links**: who `controls`, who `owns`; hand-back triggers are **rules** |
| units and axes | *"1 GTA metre = 1 block; (x, y, z) → (x, z + yOffset, −y)"* | **data** on the link between two pieces: a function both sides share, never code in either |
| channels | snapshot, queue, ring, image; direction, rate, what happens when full or late | the **holiday's** declared interface |
| protocol identity and lifecycle | versions, byte order; start, pause, load, death, crash | the holiday's interface, and rules over its state |
| not covered | what the bridge leaves alone and what the player will see | **the residual** |

So the author's *"use the holiday structure a LOT more"* has a precise reading. **A piece
recovered only to the opaque depth enters the game document as a rune whose behaviour is
a holiday**: the rune carries the contract as content and links, the holiday is the
running original behind it. The game model can place it, link it, swap it and reason
about it without ever reading its code. A piece recovered to structure has no holiday;
its behaviour is rules in the document. Most pieces of most games will sit at the first
for a long time, and that is fine.

**What generalises is the vocabulary, not an adapter.** universal-modder says it plainly:
*"Don't advertise a universal adapter. Every bridge needed per-host reverse engineering."*
This page agrees. What is general is the language the adapters are written in (who owns
what, in which units, handed back when), which is the [family resemblance](/sources/wittgenstein-family-resemblance.md)
argument from [what a game is](/concepts/what-a-game-is.md) §7 arriving from practice.

**Worlds, both ways.** Minecraft generates its terrain from a seed, chunk by chunk; Super
Mario 64's levels are authored geometry. In the game model both are **regions**. One
region's geometry comes from a generator behind a holiday (seed and chunk coordinate in,
blocks out); the other's is a block in an `.ink`. Both answer the same questions (*what
is solid at this point? what is in view?*), so a mashup can set Peach's Castle inside a
generated world without either system knowing about the other. The questions are
Void Jelly's spatial queries (`VoidJelly:/concepts/spatial.md`); the answers come from
whichever owner each region has.

# 5. The player, taken apart

> *"it's easy for a human developer to understand 'oh yeah, we have a player in both of
> them, so we can just like, swap players, easy.' but they don't really think about the
> fundamental structure of a player."*

The game model already refuses to make *player* a rune ([what a game is](/concepts/what-a-game-is.md)
§2: the player is outside the mantle; the avatar is the rune they act through). The
mashup ownership table splits it further, and every row is a decision two games make
differently and modders make silently:

| part | in Void Core | Minecraft | Super Mario 64 |
|---|---|---|---|
| **the person** | outside the mantle; reaches in through an input holiday | keyboard and mouse | an analog stick and buttons |
| **control** | a `controls` link from the input holiday to one rune | to Steve | to Mario |
| **body** | a rune with a collision shape and a place | a 0.6 × 1.8 block box on a voxel grid | a cylinder against triangle collision |
| **locomotion** | a capability: input and collision in, pose out. A contract (§4) at the opaque depth, rules at structure | walk, sprint, jump, swim, creative flight | run, triple jump, long jump, wall kick, dive, swim |
| **viewpoint** | a camera rune with a `follows` link to the body | first person by default, third person as an option | third person behind a camera character (Lakitu), with a first-person look that cannot move |
| **presentation** | the turkey's glyph: mesh, skeleton, animations (a scene through Void Jelly) | a blocky skin | a rigged, animated model |
| **stats** | quantities on the character rune | 20 health points, hunger, experience | an 8-segment power meter, lives |
| **inventory** | runes linked `holds` | 36 slots, 9 of them the hotbar | none; stars and coins are counters |
| **hand-back** | rules: who takes control during a cutscene, a door, a menu, death | the game owns menus and death | the game owns doors, cannons, star dances |

The author's two examples become **commands**, and the hard part becomes **a residual**:

**"Make the pig from Minecraft the player."** Move the `controls` link from the input
holiday to the pig rune. Move the camera's `follows` link to the pig. The pig has a body
and walks already, but its walking is driven by its AI's goals, not by input, so give it
a locomotion contract: Steve's, which reads input and moves a body through the same
voxel collision. The residual is what a human modder knows without saying: *the pig has
no hands, so place-block and attack have no presentation; the pig's animation set has no
jump; the first-person camera sits at Steve's eye height, not a pig's.* Each line is a
proposal to accept, fix, or leave.

**"Make Bowser into Princess Peach."** Two different commands, and the
[Turkey](/concepts/turkey-table-feast.md)'s split between a thing and its presentations is
what makes them different. (a) **Swap the presentation**: Bowser's rune keeps his AI and
his boss-fight rules, and takes Peach's mesh; the residual names the skeleton mismatch
(Bowser's animations on Peach's rig is a retargeting job, geometry, Void Jelly's). (b)
**Swap the role**: Peach's rune takes Bowser's rules. In Super Mario 64 Peach appears only
in the ending and has no gameplay behaviour, so (b) gives her a fight she has no
animations for, and (a) is the reading that plays. A person asking for "Bowser into
Peach" means one of these; the studio should ask which, and the transcript records the
answer.

Every swap is a link move or a presentation swap. **The intelligence is in filling the
residual**, and that is where the models come in (§8).

# 6. The pipeline, and who owns each stage

| stage | what happens | depth reached | owner | effect or pure | exists |
|---|---|---|---|---|---|
| **recon** | what is this artifact: platform, engine, header, which tools reach which depth | (zeroth) | Squid host + `squid_read` | the read is an effect; identifying is pure | **built 2026-10-10** ([recon](/concepts/recon.md)) |
| **extract** | assets and binaries out of the container | bytes | an ingress toolchain (`ndstool`, AssetRipper, splat, Dolphin); a pure reader into an `.ink` with assetless references (`VoidJelly:` Q17) | toolchain is an effect; reader is pure (Jelly's for geometry) | no |
| **split** | code into functions, symbols, sections | bytes → source | toolchain (Ghidra, splat, decomp-toolkit) | effect | no |
| **decompile** | functions into C; matched where possible | source | toolchain + Bicho `decompile` models + the matching oracle | effect | no |
| **identify** | which function is the player's update, the camera, the health, the save | (labels on any depth) | **Bicho** `classify` models, and a person | proposals; acceptance is a command | no |
| **lift** | labelled pieces into nouns and contracts | structure | a pure reader + Bicho `extract` | pure, with model proposals | no; needs the game model (S14) |
| **accept** | a person or a rule accepts proposals | | the dispatcher; the mantle records (S13) | pure | the mechanism exists (Core) |
| **shelve** | the toolbox: a catalogue mantle of the game's pieces, searchable by tag and by embedding | | the game model + Bicho `embed` (S18's shape) | pure | no |
| **compose** | drag pieces into a table; new logic in Allomone and Voidscript | | the studio | pure | partly: tables compile today ([simulation](/design/simulation.md)) |
| **run** | the composed game runs somewhere | | a runtime: each opaque piece's own (emulator, recompilation) behind its contract; structure pieces compiled to data for any runtime | effect | no; the hardest stage |

The last row is said plainly: **running a mashup is the stage the community does by hand,
per pair of games, today.** The design does not make it easy. It makes the parts that
are the same every time (ownership, units, hand-back, residuals) into data the forge can
check, so the per-pair work is smaller and written down.

# 7. Auto and by hand are one surface

> *"maybe there could be an 'auto decompile' tool, but there could also be one where the
> human is going through and manually giving values and such with it."*

In Void Maiz every gesture is a dispatcher command, so there is no second mechanism to
build. **By hand**: a person sees a proposal (*"function 0x80245A10 looks like the player's
jump; confidence 0.71; evidence: it writes Mario's vertical velocity and is called from
the action dispatcher"*) and accepts it, edits it, or rejects it. **Auto**: a rule accepts
every proposal above a threshold and queues the rest for a person. Both write the same
commands to the same transcript, so an auto run can be inspected, replayed, or partly
undone by hand.

**Every decision is a labelled example.** The mantle is the record of what was accepted
(S13), with the model's provenance on the proposal and the person's or rule's on the
acceptance. Training data for §8 accrues as a side effect of using the tool, without a
separate labelling step and without the game's bytes leaving the machine.

# 8. Void Bicho is load-bearing

The author's decision, 2026-10-10: Bicho is *"an EXTREMELY IMPORTANT layer to void
squid"*, because *"it's going to be very difficult to make this generalizable tool work
WITHOUT llms or ai models."* Today the best practice is a large model driving tools
(universal-modder's loop; one public N64 decompilation reports 2,145 functions matched in 84 days
that way). The author's direction is **away from that**: more specialised tools, smaller
local models, a human in the loop, and eventually everything on the person's own machine.

**The model kinds this needs**, each an ordinary Bicho model rune selected by tag
(`VoidBicho:/concepts/model.md`), each returning a proposal with a confidence:

| kind | in | out | stage | what exists to start from |
|---|---|---|---|---|
| `decompile` | assembly for one function | C | decompile | LLM4Decompile (1.3B–33B, open); RlDecompiler (1.3B and 3B) |
| `name` | pseudocode | names for functions, variables, fields | decompile | SK2Decompile's naming phase |
| `classify` | pseudocode, call graph, memory writes | a role: player update, camera, AI, render, physics, save | identify | nothing published that we found; the gap |
| `extract` | text tables, dialogue, item data | typed fields | lift | Needle's structured extraction (`VoidBicho:/design/needle.md` §4, job 3) |
| `segment-2d`, `caption` | screenshots taken while a function runs | what is on screen | identify | MobileSAM (`VoidBicho:/concepts/perception.md`) |
| `embed` | a piece, or a request like *"a small enemy that flies"* | a vector | shelve | Needle's embedding head |

And one baseline: **a large agent with tools**, per stage, against which every small
model is measured on the same task. A small model earns its place by matching the agent
on a stage at a fraction of the cost, locally.

**Rewards a learning system can trust.** Decompilation is unusual in having verifiable
answers. In order of strength: a byte match (the function compiles to the original), a
behaviour match against the opaque oracle (the recompiled game plays the recorded
scenario the same way), re-executability (compiles and passes tests), and a person's
acceptance. The literature is moving the same way: SK2Decompile and RlDecompiler train
with compiler and test rewards, and a 2026 paper warns that recompilation alone is an
incomplete proxy for preserved behaviour ([learned decompilers](/sources/learned-decompilers.md)).
So the behaviour oracle matters, and it is the opaque depth that supplies it.

**Federated, when there are users.** The author: *"federated learning would be ideal, if
we had a large user base, which we currently don't."* The shape is already on Bicho's
desk from Latin OS (*a device learns from its own data and sends weight deltas, never
data*). For decompilation it has a second reason: **a game's bytes and its decompiled
code must not leave the person's machine** (the community's rule, §10), and deltas are
the only thing that would. Whether deltas can leak what they were trained on is an open
research question (membership inference), recorded here and not answered.

**What Bicho needs to change for this**: training in scope as an invocation (Latin OS
asked on 2026-10-09; this is a second, independent client), and evaluation (Bicho's open
question 6) made concrete: *a benchmark is a mantle of functions, each with its oracle.*
Sent as `../VoidBicho/MESSAGE_FOR_VOIDBICHO_voidsquid-decompilation-makes-you-load-bearing-2026-10-10.md`.

# 9. universal-modder, and what forking it would mean

[universal-modder](/sources/universal-modder.md) (MIT) is skills, a Python CLI (`um`) and a
reviewed knowledge base that let any coding agent mod games the person owns. A trimmed
reference copy is in `third_party/universal-modder/` (pinned commit, see its README); it is
read, not run, and nothing links it.

| universal-modder has | in Void Squid's terms |
|---|---|
| `um scan` (engine, version, anti-cheat, loaders, saves) | **recon** for PC installs; ours names it as the deeper tool instead of re-implementing it |
| the knowledge base: field notes per game and per technique, with versions, gotchas, oracles | a **mantle of notes**, searchable by tag: what a reader for a given engine should already know |
| six evidence levels (*creator report* … *real run*) | a finer form of this bundle's `asserted` / `verified`; worth adopting in residuals |
| the bridge contract | the **contract** of an opaque piece (§4) |
| seven mashup routes | the depth at which the guest is used: frame compositing and passthrough are opaque; conversion is bytes; recreation is structure |
| `um publish check` (no game files, no decompiled code, no keys) | a **rule** over the document before anything is shared |
| fal for generated assets (a paid remote API) | Bicho's generation adapters, local where possible |

**What a fork changes**, read from the author's direction: (1) agent-driven stages are
replaced one at a time by tools and small models, each measured against the agent on the
same task; (2) everything runs locally; (3) the output is a **document** (pieces at known
depths, contracts, residuals), not a mod folder. The knowledge base and the safety rules
are kept, credited.

Where a fork would live is S19. The lean: it is a **toolchain** (Python, invoked), so it
belongs in its own family repository and reaches Void Squid through the ingress row of the
[build surface](/concepts/build-surface.md), like REA. Its name is the author's.

# 10. Rules carried over, unchanged

From universal-modder, REA and the decompilation communities, adopted as this bundle's:

- **The person owns the game and dumps it themselves.** No ROMs, ISOs or leaked builds
  are downloaded by anything in this family.
- **Game bytes and decompiled code are never shipped.** A shared document carries
  references and converters, and the receiver's own copy fills them (Jelly's assetless
  `.ink`, its Q17). The shelf lives on the person's machine.
- **Never into online games or anti-cheat.** Offline, single-player, or servers the person
  runs.
- **Respect each community's rules**, including those that refuse AI-written
  contributions or anything built on leaked material.
- **Be honest about what was verified**, at the evidence level actually reached.

This page says nothing about whether a given reading is permitted where the person lives;
the person running the tools is responsible for that.

# 11. Order

1. **Recon.** Built ([recon](/concepts/recon.md)).
2. **The ingress toolchain row, once**: invoke one tool under the effect gate. `ndstool`
   on a `.nds` is the natural first, because rung 3's DS nib will produce our own `.nds`,
   and reading back what we wrote is the lens between our egress and our ingress.
3. **A reader to the bytes depth**: a DS cartridge's file system and assets into an
   assetless `.ink` plus residual, with Void Jelly owning the geometry.
4. **Bicho's Phase 0 and 1** (catalogue, one invocation), then the `classify` gap.
5. **Identify, by hand**, in the studio: proposals, accept, reject, the transcript.
6. **Lift**, which needs the game model, and therefore S14.

Steps 1 to 3 are host and reader work and do not need the game model. Steps 5 and 6 do.
Whether decompilation's weight changes the order of S14 and the first ingress rung is S20.
