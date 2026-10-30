---
type: Concept
title: Coral — sharing a game while it is being played (host, servers, sessions); editing together is Void Maiz's
description: "The author, 2026-10-12, twice. First: Void Squid needs its own layer like Void Hormiga's Antfarm, about sharing game information with other players, named Coral. Then, the same day, the line drawn: Coral is game data shared between players while a game is active (who hosts, which servers, the session); sharing the EDITING, collaborative editing of a game, is Void Maiz's networking, which a studio built properly on Maiz gets without writing anything. The exercise's local journal was the foundation; what it recorded now belongs on the editing side."
resource: ../ds-mashup/src/g2/coral.hpp
tags: [status:planned, audience:dev, audience:all, confidence:exploratory]
timestamp: 2026-10-12T00:00:00Z
---

> The author, first: *"we might actually need our own version of an 'antfarm' but for this
> sort of game … its about sharing game information with other players … We could name it
> a 'coral'."*
>
> Then: *"coral is mostly for game data sharing between players, when a game is active and
> stuff. determining the host of the game, servers, etc. that's coral. Sharing the EDITING,
> like colaborative editing for games, that's with void maiz's provided networking stuff.
> So a lot of that should be easily integrated and carry over (if we've been properly using
> void maiz to register tools and stuff properly)."*

# 1. Two kinds of sharing, two owners

| | **editing together** | **playing together** |
|---|---|---|
| what moves | changes to a project: a tag corrected, a table reordered, a feast set | the state of a running game: where everyone is, what happened, who scored |
| clock | model rate: a person's edit, whenever | play rate: many times a second |
| owner | **Void Maiz** (presence, LAN and Reticulum links, the merge of state documents) | **Coral** |
| what Void Squid must do | dispatch every edit as a Void Core command in a Maiz Session, and nothing else | everything: Coral is Squid's to design |

**Editing together comes free when the studio is built properly on Void Maiz**, and that
is the test the author set. Void Squid 0.1's studio
([the studio](/design/void-squid-0.1-the-studio.md)) does it: the project is a Void Core
state document, opened through a `maiz::Session`; every change, from the window, the
console or an agent, is one dispatcher command; library runes have stable identities, so
two people's copies line up when Maiz merges them. Switching Maiz's collaborative editing
on for a project is Maiz's work, not a Coral feature.

# 2. Why Coral is not the Antfarm

Void Hormiga's Antfarm is about an organisation's backends: services, credentials, data
flowing in from outside. Coral is about **players in the same game at the same time**.

| | Antfarm (Hormiga) | Coral (Squid) |
|---|---|---|
| on the other end | services | players |
| what moves | records from outside, in | a running game's state, between players |
| trust question | is this service allowed this key? | is this move from the player it says, and is it legal in this world? |
| failure that matters | a dead service | two players who disagree about what just happened |

# 3. The parts (designed, not built)

| word | what |
|---|---|
| **reef** | one player: who they are and how to reach them |
| **session** | one game being played: its feast, its tables, its players |
| **host** | who runs the authoritative copy of a session: a player's machine, or a server |
| **polyp** | one piece of shared play state: a player's avatar, a table's live objects, the score |

Questions Coral answers that nothing else in the family does: who hosts, how a player
finds a session, what crosses each tick, what happens when the host leaves, and what a
latecomer receives. **No game data crosses between players**: a session names its turkeys
by recipe (the archive, the route, the depth) and each player's runtime builds them from
their own copy of the game.

# 4. What the exercise built, and where it went

The turkey table (`../ds-mashup`) wrote a local journal, `coral/journal.jsonl`, of every
shareable change: tags, kinds, sounds, saved tables, the feast. Those are **edits**, so
under the author's line they belong to editing together, and in Void Squid the project's
own Void Core journal (`.squid.log`, written by the Maiz Session) already records each of
them as a command. The exercise's file stays as it is; Void Squid does not write one.

# 5. Open

S25 in [open questions](/design/open-questions.md), narrowed: is a polyp a rune, a mantle,
or something with no Core shape at play rate? Who is a player (Void Hormiga's identity
work is a candidate)? And where does a session's host run: on a player's machine first,
a server later?
