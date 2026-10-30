---
type: Concept
title: Turkey, Table, Feast — the creation workspaces
description: "The author's 2026-10-09 vocabulary for making a game in Void Squid. Turkey: a thing and its presentations (a rune and its glyphs). Table: a world and its rules (a mantle plus Void Allomone). Feast: the layout of the game in time and across people (narrative order, sessions, networking). Why new words, how each maps onto Void Core exactly, what Allomone can and cannot be today, and the same three words in a decompiled game."
tags: [status:planned, audience:dev, audience:all, confidence:exploratory]
timestamp: 2026-10-09T00:00:00Z
---

> **Nothing here is built.** The words are the author's. The mappings are a proposal
> to argue with; S15 and S16 in [open questions](/design/open-questions.md) hold what
> is undecided.

# 1. Why new words

Void Core already has rune, glyph, mantle and holiday, and they are exact. The author's
reason for new words is not that the old ones are wrong. It is that *"it makes things
clearer when programming, and for logs."* A game designer's sentence, *"put the goblin
on the forest table and give everything tagged hostile a patrol rule,"* should read the
same in the GUI, in the transcript and in a test. The new words name **workflows**; the
old words remain the **substrate**, and every new word must map onto exactly one old
construct, or it is a second ontology and will drift.

# 2. The three, mapped

| word | the author's description | in Void Core | owner of the computation |
|---|---|---|---|
| **Turkey** | the window where characters and objects are made; in a decompiled game, the list of every recovered entity. *"More similar to a glyph … the same turkey can represent itself in different ways on different tables."* | a **rune** (the thing, its stable identity and content) together with its **glyphs** (how it presents). One turkey, many presentations | the game model (pillar 1); geometry by reference to Jelly |
| **Table** | a world; *"where we are making the rules of the world. Where we can tag turkeys, and give certain tags rules."* | a **mantle**, plus a **rule set** over its tags | **Void Allomone** for what is true; see §3 for what happens |
| **Feast** | *"where we manage the LAYOUT of the game … linear narratives … multiplayer and networking"* | the **ordering and connection of tables**: a beat graph over tables, sessions, and the **holidays** that carry state between players | the game model for the graph; Void Palabra for sync; holidays for transport |

## Turkey: one thing, many tables

The author's key sentence is that a turkey *represents itself differently on different
tables*, *"almost exactly like how a glyph points to a rune."* So a turkey is **identity
plus a set of presentations**: the goblin is one rune (one `spirit.id`, one stat block,
one history), shown as a 3D model on the forest table, a sprite on the overworld map
table, a row in the bestiary table and a line of text in the narrative. This is also
Latin OS's cross-dimensional projection (*Master Chief into Link's Awakening*) at the
scale of one object: one identity, a presentation per target, a residual for what did
not survive.

It is also exactly what Void Hormiga 0.2.0 did with **kinds**: a kind is data in the
database, with traits (located, dated, listed) that front-ends query instead of names.
A turkey's presentations are the game version of a kind's traits. Hormiga's rule
carries over: *a kind may never take one of the application's glyph names*, so a
turkey's presentation names live in the game model's namespace (S9).

## Table: a world and its rules

A table is a mantle: turkeys placed on it (runes with `located-in`), tags on them, and
rules over the tags. The author: *"this will HEAVILY utilize Allomone … maybe it IS the
programming language of this game engine? … the allomone 'script' is a mantle in itself.
interactive."*

## Feast: the meal in order

A feast is the game laid out: which tables follow which (the beat graph of
[what a game is](/concepts/what-a-game-is.md) §5, with tables as its nodes), which
players sit at which table, and how state crosses between them. Networking enters here
and only here, as holidays: a table is shared by syncing its mantle (Palabra), and a
player's input reaches a table through a holiday like any other host effect.

# 3. What Allomone can be today, and what it cannot

Allomone's own pages answer the author's question precisely
(`VoidAllomone:/concepts/materialization.md`):

| | Voidscript | Allomone |
|---|---|---|
| every statement is | a dispatcher command | a constraint |
| effect | **mutates** the model | **derives** an annotation |
| terminates? | not necessarily | **always**, by theorem |

*"Voidscript is the mechanism; Allomone is the intent,"* and the seam, when it opens, is
**Allomone compiles to Voidscript**. Only Allomone's tier 0 (derive) is built. Tier 1
(amend fields and tags on existing runes) is planned and safe if the effect graph is
acyclic; tier 2 (create runes) needs a budget; tier 3 (scripts that write scripts) is
refused by theorem.

For a game that means a clean split:

- **What is true on a table** is Allomone today, and it is most of what a designer
  means by "rules": *everything tagged `hostile` within 5 m of the player is `alert`*;
  *the music cue `tension` is live when any `alert` exists*; *an NPC's mood follows its
  relationship links*; *which turkeys a player may see*. Derived, order-free, with
  conflicts surfaced as values. The table's rule set is a mantle and is edited
  interactively, as the author wants.
- **What happens** (a door opens, a goblin takes damage, a quest advances) is a state
  change. In the forge and in a model-rate simulation it is Voidscript, or Allomone
  tier 1 compiled to Voidscript once it exists. In a shipped runtime it is the
  runtime's own code reading compiled tables (railguard §6).

*Ruled by the author 2026-10-10 (S16): Allomone is a programming language for Void Core. First in use as a table's simulation rules ([simulation](/design/simulation.md)).* The split below still describes what each half can do today.

So the lean for S16: **Allomone is the Table's language for truth now, and for
amendment when tier 1 lands; Voidscript carries what happens until then.** Calling
Allomone *the* programming language of the engine is right about the part of game logic
that is declarative, which is larger than it looks, and premature about the part that
changes state.

# 4. In a decompiled game

The same three words, filled from the other direction
([three uses](/design/three-uses.md) §2):

- **Turkey** is the list of every entity a reader recovered, each with the
  presentations it was found in (a model, a sprite, a data row), and the residual of
  what could not be recovered for it.
- **Table** is each level or map, with the rules the reader could infer, written as
  Allomone where they are derivations and flagged where they are mechanics with no
  Allomone form.
- **Feast** is the order the game presents its levels and the session structure it
  shipped with.

# 5. In logs

The reason for the words, made concrete. A transcript line names its workspace and its
noun, so a person scanning an agent's work can tell which surface it touched:

```
> turkey new goblin --kind character
> table place goblin forest --at 12 0 4
> table rule forest 'when tag "hostile" and near "player" 5 then alert true'
> feast after forest cave
> effect look
```

These are illustrative, not built; whether they are new verbs or aliases over Core's
(`rune new`, `place`, `rule`, `link`) is S15, and the lean is **aliases that lower to
Core commands**, so the journal records the Core command and the alias in one line and
no second command language exists.

# 6. Work set-ups (the author, 2026-10-12)

> *"turkey, table, and feast as sort of different 'work set ups'. Like, the windows are always
> available for all of them, but they accomplish different things."*

The three words name **work set-ups**, not windows. Every window is always there (the
shelf of turkeys, the 3D view, the inspector, the transcript). Each set-up gives them a
different job:

| set-up | the job | the view | the inspector |
|---|---|---|---|
| **Turkey** | inspect each recovered thing: is it set up right, are its tags accurate? | the thing alone, turning, beside a Goomba's height | checks (texture, collision, wiki match, tags, size, named by the game or by eye), kind, tags with their sources, sounds per event |
| **Table** | build a world: drag turkeys onto it, as now | the world, with handles to move, turn and size | the selected thing, or the map: its music and gravity fields |
| **Feast** | for now, the player: who you play and how they move; the order of the tables | the world | the player's numbers, the tables in order |

**Built** as an exercise in `../ds-mashup/src/g2/table.cpp`
([Goomba Galaxy 2 and the turkey table](/design/goomba-galaxy-2-and-the-turkey-table.md),
[four games and the asset database](/design/four-games-and-the-asset-database.md)). Turkey
edits go to an asset database in Void Hormiga's schema. Every shareable change goes to the
[Coral](/concepts/coral.md) journal.
