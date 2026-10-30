# Void Squid

**Build new worlds out of the games you own.** Void Squid is the game engine of the Void
family, on [Void Core](../VoidCore/README.md) and [Void Maiz](../VoidMaiz/README.md):
a studio where what you recovered from your games becomes things you can inspect, tag,
lay out and play, and later a forge that exports new games. It began as the host
[Void Jelly](../VoidJelly/README.md) cannot be for itself, and still is.

**Version 0.1.0 (2026-10-12): the first real version of the studio.** Windows today,
Linux in the code and not yet tested. **Setting it up: [SETUP.md](SETUP.md).** The design
is in **[okf/index.md](okf/index.md)**.

> Void Squid ships **no game, no game data and no demo**. Everything you load comes from
> games you own and stays on your machine.

## What it is for

The author, 2026-10-12: *"void squid is NOT about decompilation. It's about the creation
of NEW things. its about the EXPORT of games."* Loading a game is step one. After it come
the turkeys (the things a game is made of), the tables (worlds to put them on), the feast
(who plays, in what order), and later the logic: not only a Goomba but the Goomba's
behaviour, reapplied to anything else.

## The window

A docked studio in the shape [Void Hormiga](../VoidHormiga/) proved: a menu bar, a row of
**workspaces**, and windows that are always there.

| workspace | for |
|---|---|
| **Start** | open a library, load a game, open a project |
| **Turkey** (F5) | careful inspection of one thing: every tag and where it came from, its kind, its checks, its sounds |
| **Table** (F6) | the worlds: their order, their music, a map of what is on them; Play |
| **Feast** (F7) | the player: who, how fast, how high, how many lives |
| **Data** (F8) | every mantle of the project as a table, edited through Void Maiz's widget registry |
| **Scene** (F9) | the original harness: an `.ink` through Void Jelly, drawn, with its residual |

Plus the **Shelf** (everything in the library, searchable with one tag grammar), the
**Inspector**, the **Console**, **Load a game** and **Setup**.

**Every edit is one Void Core command.** The window, the console and an agent on the
command line make the same change in the same journal, which is why undo works, why an
agent's work can be reviewed, and why Void Maiz's collaborative editing will carry it.

## Projects and libraries

- A **project** (`.squid`) is your work: a Void Core state document, with its journal
  (`.squid.log`) beside it.
- A **library** is a folder of what was recovered from your games, with `assets.db`, its
  catalogue (Void Hormiga's schema). *File > Open library* brings the catalogue into the
  project; a re-import keeps every tag, kind and sound you set.

## Load a game, and four setups

Point Void Squid at a ROM, a disc image or an install folder. It reads the header, checks
whether a community already decompiled the game, and lays out what this machine can do
next. That much works on any computer, with no model and no GPU: the **minimum** setup.
A **local model** (Ollama), **vision** (Void Bicho) and a **coding agent** each add more;
a **hosted model** with your own key is planned. [SETUP.md](SETUP.md) has the steps;
[okf/design/load-a-game.md](okf/design/load-a-game.md) the reasons.

## Build

```
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build -R squid_
build/bin/voidsquid            # the window
build/bin/squid --describe     # the briefing, for an agent
```

Needs a C++20 compiler (MSYS2 UCRT64 on Windows), Ninja, a built Void Core beside this
repository, and Void Maiz, Void Jelly and Void Allomone checked out beside it.
`void.json` is the family registry's manifest.

## The rules it keeps

- **Void Squid owns no truth.** The model lives in Void Core; the window projects it.
- **Void Squid computes nothing about geometry.** Thumbnails are rendered by the toolbox,
  the world map is positions read from a file, the scene is Jelly's draw list.
- **Void Squid owns every effect**, and declares each one: reading a library, starting the
  runtime, the one network request. Effects are refused on the command line unless granted.
- **Vendor, don't depend.** Third-party code is in `vendor/` with its licence
  ([THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md)).

## Siblings

- **[Void Jelly](../VoidJelly/)**: the 3D library; owns `.ink`, nibs, residuals and all geometry.
- **[Void Core](../VoidCore/)**: rune, mantle, holiday, glyph, dispatcher, tags, links.
- **[Void Maiz](../VoidMaiz/)**: the UI library this is built with.
- **[Void Hormiga](../VoidHormiga/)**: the database schema and the studio's shape came from it.
- **[Void Bicho](../VoidBicho/)**: the models (vision now; more later).
- **[Latin OS](../LatinOS/)**: the operating system this forge will one day emit, and a second host.
