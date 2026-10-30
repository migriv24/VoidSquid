---
type: Concept
title: What a game is
description: The author's conception of 2026-10-09, worked into a vocabulary — a world under a Dungeon Master, players with avatars, feedback, upgrades that carry the game's thesis, terrain that changes, a collage of modalities, narrative and living NPCs — and mapped onto Void Core's nouns. The first pillar of Void Squid, and the one that is pure.
tags: [status:planned, audience:dev, audience:all, confidence:exploratory, foundation]
timestamp: 2026-10-09T00:00:00Z
---

> **Status, honestly.** This page records a conception the author set on 2026-10-09 and
> works it into a vocabulary. **Nothing here is built, and v0's scope does not move**
> ([scope](/concepts/scope.md)). It is `confidence:exploratory` throughout: a position to
> argue with, stated so it can be wrong. The research it leans on is in
> [sources](/sources/index.md) and argued in
> [the epistemology of the game](/design/epistemology-of-the-game.md). **The author
> decides**; what is open is in [open questions](/design/open-questions.md) S9–S12.

# 1. What the author said

Five beliefs, carried into the design as the author stated them:

1. **Dungeons & Dragons is a foundational game.** *"I believe in the concept of a DM that
   is generating the narrative and orchestrating the world. There are characters with
   stats."* Even an operating system has *"narrative, someone telling their story, and
   people exploring a world."*
2. **A game is a collage of modalities.** Video, image, music, 3D, text. *"A game
   combines them all."*
3. **A game has an avatar** through which the player interacts with the world, a space
   in which the player **receives feedback** from the world, and **avenues of upgrades**
   that improve the player's *"traversal and dominion over the world."* Upgrades carry the
   world's themes: *"it's the game saying 'this is what it means to be BETTER.'"*
4. **Our games have terrain modification and world interaction.** *"Assets are not
   static, but dynamic."*
5. **Not Tetris, not Candy Crush.** *"We want narratives. We want music that reacts to
   the player. We want NPCs that feel alive. We want some sort of AI in this."*

And one method: the engine must be able to **deconstruct** as well as construct
([construct and deconstruct](/concepts/construct-and-deconstruct.md)), so every noun
below is tested both ways: can the forge emit it, and can a reader recover it?

# 2. The nouns, and where each one lives in Void Core

The oldest fully general game ontology is the one designed to be run by hand
([Braunstein to D&D](/sources/braunstein-to-dungeons-and-dragons.md)), which makes every
concept in it legible. Taking it as the starting vocabulary and placing each noun on the
family's substrate:

| noun | what it is | in Void Core's terms | who owns the computation |
|---|---|---|---|
| **world** | the bounded space the game happens in ([Huizinga](/sources/huizinga-homo-ludens.md)); its laws replace the ordinary ones inside it | a **mantle**: runes, tags, links, rules, with the outside reachable only through holidays. A world may *contain* scenes; an `.ink` is one scene | Core (the model), Palabra (versions and joins of it) |
| **player** | a person who has accepted the rules because they make the activity possible ([Suits](/sources/suits-the-grasshopper.md)) | **not a rune.** A player is outside the mantle, like a user is outside a Void Maiz application. The player reaches in through a holiday (input) and the world reaches out through others (display, sound, haptics) | the host |
| **avatar** | the rune through which the player's input enters the world, and through which the world answers (§3) | a rune with a `controls` link from a player holiday; its capabilities are content and links | pillar 1's vocabulary; the runtime moves it |
| **character** | a thing with **stats** and a role; the avatar is a character, so are NPCs | a rune whose content is typed quantities (Core's quantity annotations, 0.2.14) and whose tags give role, class, allegiance | pillar 1 |
| **thing / asset** | an object, a prop, a sound, a text, a texture: anything placed or referenced | a rune whose heavy content is a **content-addressed block** (Palabra's store; Jelly's `.ink` already does this for meshes) | the modality's library (§6) |
| **region / terrain** | a part of the world with extent that things are *in*, and which may change (§5) | a rune with geometry (a block) and `located-in` links from what it contains | Jelly for the geometry; the runtime for its live state |
| **rule** | what happens when; the half of a game that is real ([Juul](/sources/juul-half-real.md)) | Core's **rules** for structure; **Allomone** for anything derived and never authored (*who should see this*, *what colour is danger*); compiled to **inert data tables** for the runtime | Core, Allomone; the runtime reads the tables |
| **beat** | a unit of authored narrative with conditions ([Jenkins](/sources/narrative-agency-and-architecture.md)) | a rune in a graph of `then`, `requires`, `excludes` links; its text is content | pillar 1; a director chooses among beats at play time |
| **cue** | a unit of authored music or sound with conditions ([iMUSE](/sources/imuse-adaptive-music.md)) | a rune with links into world facts (a region, a stat threshold, a beat) | Loops Studio for the audio; pillar 1 for the links |
| **upgrade** | a change to a character's capabilities that the world grants (§4) | an `unlocks` link from a world fact to a capability, with `requires` and `costs` | pillar 1; the runtime applies it |
| **the DM** | the one who runs the rules, adjudicates the gaps and generates the world | **a role, not a rune**, split three ways below | three owners |

**The Dungeon Master, taken apart.** The referee has three jobs, and they are the three
places computation lives in a Void Squid game:

| the DM's job | in a Void Squid game | who |
|---|---|---|
| **runs the rules** (dice, tables, hit points) | the rule tables the forge compiled, executed at frame rate | the **runtime** (a Bevy app, a C program on the DS, LITO) |
| **adjudicates the gaps** (what the rules do not say) | authored rules and their derived consequences, authored in the forge and queryable there | **Core rules + Allomone**, in the document |
| **generates the world** (describes, voices, improvises, keeps the arc) | the part that is generative at play time: a director choosing among authored beats, an NPC finding words | a **model behind a holiday** (Void Bicho), in the artifact, steering structure the forge authored ([AI directors](/sources/ai-directors-and-drama-managers.md)) |

That split is the forge/artifact line applied to the one concept that looks like it
straddles it. The forge is the DM *preparing*: the maps, the tables, the beats, the
arc. The artifact is the DM *at the table*. The author's *"some sort of AI"* is the third
row and only the third row, and it is a holiday because a model is one
(`VoidCore:/concepts/holiday.md` names LLM backends; `VoidBicho:` is the vocabulary).

# 3. The avatar, and feedback

A player reaches the world through an avatar and the world answers. The answer has to
be **discernable** (the player can tell the action had a result) and **integrated** (the
result matters later): [Salen & Zimmerman](/sources/salen-zimmerman-rules-of-play.md)'s
two conditions for meaningful play, [Murray](/sources/narrative-agency-and-architecture.md)'s
*agency*.

What this does to the vocabulary: **feedback is part of the avatar concept, not a
presentation detail.** Every modality the world answers in (a sound, a particle, a line of
text, a change in the music, a controller rumble) is a holiday the host performs, and the
*decision* to answer is a rule in the document. So the forge can ask *"what does the
world do when the avatar does X?"* and get a list, and a
[deconstruction](/concepts/construct-and-deconstruct.md) of an existing game should be
able to recover the same list. Void Maiz already routes haptics and logging through
host-registered sinks; the pattern transfers.

# 4. Upgrades are the game's thesis

The author's strongest claim, and the literature agrees with it from three directions:

- **Valorisation of outcomes** ([Juul](/sources/juul-half-real.md)): a game says some
  outcomes are better than others.
- **Procedural rhetoric** ([Bogost](/sources/bogost-procedural-rhetoric.md)): a game
  argues through what it rewards, costs and forbids, whatever its text says.
- **The game as teacher** ([Koster](/sources/koster-a-theory-of-fun.md)): the upgrade
  path is the curriculum.

So **the upgrade graph is where the game's values are written**, and it is authored
structure: `unlocks`, `requires`, `costs` links between world facts and capabilities,
each carrying the theme it serves as a tag. Three consequences:

1. It is **queryable in the forge.** *"What does this game say it means to be better?"*
   is `ls` over the upgrade graph.
2. It is **the first thing a reader should recover** from an existing game, because it
   is the game's argument in its most compressed form.
3. The narrative is a second place the values are written, and the forge holds both, so
   it can **compare them**. [Hocking](/sources/hocking-ludonarrative-dissonance.md)'s
   dissonance becomes a structural check: *does any `unlocks` edge reward what a `beat`
   condemns?* That is a claim to test when there is a document to test it on, not a
   feature to build.

# 5. The living world

The author's fifth belief is four things, and each has a known shape.

**Narrative.** [Jenkins](/sources/narrative-agency-and-architecture.md)'s modes map
onto three ways the document holds a story. *Embedded* narrative is content on runes in
places: a letter on a desk is text attached to a rune in an `.ink`, and it exists today
in everything but the glyph. *Enacted* narrative is the beat graph of §2. *Emergent*
narrative is the runtime's dynamics and cannot be authored, only invited, which is the
MDA split ([MDA](/sources/mda-framework.md)) and a limit the forge should state rather
than hide.

**Music that reacts.** [iMUSE](/sources/imuse-adaptive-music.md)'s structure: the
composer authors segments and layers with transition points; the runtime selects among
them against world state. The cue is a rune with links into world facts; the audio
engine is Void Loops Studio's, and is not ready. The seam is recorded; nothing is built.

**NPCs that feel alive.** A character rune with stats, a role, relationships (links to
other characters and to beats), and, for the generative part, access to the director's
model. What makes an NPC feel alive in the literature is **memory and consequence** (the
Nemesis system, Dwarf Fortress's histories, `confidence:asserted`): the NPC's state is
*integrated* in Salen & Zimmerman's sense. That is a property of the document (the NPC's
history is content that persists, which Palabra gives by default), not of the model.

**Terrain that changes.** The games that let players change the most all chose a
uniform, content-addressable representation ([destructible worlds](/sources/destructible-worlds.md)).
The family's grid is Void Jelly's voxel nib. A modifiable region is a rune whose geometry
is a block the runtime may rewrite; the forge sees terrain state as data, and the
runtime owns its live copy at frame rate. Whether the *forge* may also hold a live,
discretely edited scene (a play surface inside the studio) is Jelly's open question §19
arriving in a game instead of a desktop, and it goes to Jelly as a message rather than
being settled here.

# 6. A collage of modalities

A game combines every medium, and each medium has (or lacks) an owner in the family.
Void Squid composes documents from these by reference, through Palabra's content
references, and computes nothing in any of them.

| modality | owner | its document | status, honestly |
|---|---|---|---|
| 3D scene, geometry, placement | Void Jelly | `.ink` | rung 1 built; no nib emits yet |
| music, sound | Void Loops Studio | its node-graph project | *"not at all ready for integration"* (the author) |
| models (the generative DM, NPC speech) | Void Bicho | its model catalogue | design only |
| derived annotation (what follows from what) | Void Allomone | rule sets | built, extracted from Maiz 2026-08-29 |
| versions, joins, the content store | Void Palabra | containers | built |
| **text and narrative** (beats, dialogue, lore) | **nobody** | | **a gap.** Pillar 1 holds the *structure* (beat graph); the text itself is content on runes. Whether that needs a sibling is S11 |
| **image and video** | **nobody** | | **a gap.** Textures reach the family through Blender and the Polyp today; video has no path at all |

The gaps are listed because a collage with two missing media is not a collage. They are
not work for now; they are the honest shape of the family on 2026-10-09.

# 7. What our games are, and what the vocabulary must still describe

The author rules Tetris and Candy Crush out of the games Void Squid makes. On
[Caillois](/sources/caillois-man-play-and-games.md)'s axes they are pure agon-ludus with
no mimicry: rules without a world or a role; in [Juul](/sources/juul-half-real.md)'s
terms their fiction half is nearly empty. **Void Squid's games require both halves**: a
world with a story and rules that are real.

But the vocabulary is **not allowed to be unable to say Tetris.** [Wittgenstein](/sources/wittgenstein-family-resemblance.md)
is right that no definition holds, and [deconstruction](/concepts/construct-and-deconstruct.md)
will bring in games the author would never make. So the rule is: *our games* are a
constraint on what the **forge's templates and defaults** produce; the **vocabulary** is
open, and a document with no beats, no avatar and one region is a valid, boring game.
This is also why the model is a vocabulary (glyphs, tags, links) and not a schema.

# 8. The same nouns, read as an operating system

Latin OS's 2026-10-09 reframing (`LatinOS:/design/the-os-as-a-game.md`) says an operating
system is a constructed artifact and Void Squid is its forge. The author's instruction
that *scene* and *game* be very abstract is what makes that readable in this vocabulary
without changing a noun:

| game noun | in an operating system |
|---|---|
| world | the machine, or the mesh of a person's devices |
| player | a person with a seat |
| avatar | the session: the bundle of capabilities the person acts through |
| character, stats | a process (a mantle) and its quantities (memory, time, consent held) |
| the DM running the rules | the kernel's reducer (LITO) |
| the DM adjudicating gaps | init reconciling the *plan* mantle against the *observed* one |
| the DM generating | the system's agents (Bicho runes) |
| upgrades | capabilities handed, delegated, restricted (Verguenza): *this is what it means to be trusted* |
| beats, narrative | the log. An OS's story is its command transcript, and Palabra keeps it |
| terrain | the capability graph that replaces the file system; it changes as things are created |
| NPCs | services and other people's agents |

None of this is built and none of it moves v0. It is recorded because the author asked
that the abstraction hold at this altitude, and because it is the test of whether the
nouns in §2 are the right ones: a vocabulary that could not say *operating system*
would be a vocabulary for one genre.

# 9. The test for anything proposed in this pillar

Three questions, all mechanical:

1. **Can it be stated in Void Core's vocabulary?** A rune, a tag, a link, content, a rule,
   a holiday. If it needs a new kind of thing, that is a finding about Core, to report.
2. **Does it run in a unit test with no filesystem, network, display or clock?** Yes is
   required. Pillar 1 is pure; everything that is not pure is a host's
   ([hosts](/concepts/hosts.md)) or a runtime's.
3. **Can it be recovered from an existing game as well as emitted into a new one?** If a
   reader could never find it in any artifact, it is probably an editor convenience, and
   it belongs in [the studio](/concepts/studio-surfaces.md), not here.

And two rules carried over unchanged from 2026-10-03: **Squid composes structure, never
mechanics in motion**, and **authored rules compile to inert data the runtime reads in
its own language** ([railguard](/design/what-voidsquid-is-not.md) §6, as restated).

*2026-10-10: the third test, run from the other side.* [Decompilation](/design/decompilation.md)
§5 takes **player** and **avatar** apart into the rows the mashup community's ownership
tables already use (control, body, locomotion, viewpoint, presentation, stats, inventory,
hand-back), compares Minecraft and Super Mario 64 row by row, and turns *"make the pig the
player"* and *"make Bowser into Peach"* into link moves and presentation swaps with a
residual. It is the strongest evidence so far that §2's split (player outside, avatar a
rune, feedback a holiday) is recoverable from games nobody here made. It also suggests
two nouns §2 lacks: a **viewpoint** (a camera rune that `follows` a body) and a
**contract** for behaviour recovered only as running code (S21).

# 10. What this page does not decide

The names of the glyphs, which of them are tags instead, whether pillar 1 is a build
target in this repository or a sibling library, and what the game document is called.
All in [open questions](/design/open-questions.md) S9–S12. The author names things.
