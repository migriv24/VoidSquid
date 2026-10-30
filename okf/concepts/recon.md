---
type: Concept
title: Recon
description: "The first stage of the ingress arrow, built 2026-10-10: given a file or an install folder, say what kind of artifact it is, which platform and engine, what its header claims, which tools reach which depth of recovery and whether they are on this machine, and what recon itself could not determine. Console headers (DS, N64, GBA, Game Boy, GameCube, Wii) are read and checksum-verified; PC installs are fingerprinted by listing, with universal-modder's `um scan` named as the deeper tool. Pure in squid_read; the host performs the bounded read."
resource: src/read/recon.cpp
tags: [status:current, audience:dev, confidence:verified]
timestamp: 2026-10-10T00:00:00Z
---

> **Built 2026-10-10**: `src/read/recon.{hpp,cpp}` (the `squid_read` target, pure),
> `effect recon` in `src/app.cpp` (the effects), `tests/read_test.cpp` (the
> `squid_read` test) and two cases in the headless smoke test. `confidence:verified`
> means the tests run and pass, and the CLI was run against a real executable and a
> real folder on this machine. *Later the same day it ran on the author's own dumps of
> Super Mario 64 DS, Mario Kart DS and Mario Party DS: all three headers recomputed their
> CRC and read correctly (titles, game codes, 1,833 / 606 / 265 files, 103 / 4 / 108
> overlays). See the [DS mashup exercise](/design/ds-mashup-exercise.md).*

# 1. What it is for

Every reader is the inverse of a nib ([construct and deconstruct](/concepts/construct-and-deconstruct.md)),
and before a reader runs, something has to say **which** reader. That is recon: the
zeroth stage of [decompilation](/design/decompilation.md) §6, and the "Decompiling
game…" bar's first tick. universal-modder's playbook opens the same way (*"in about five
minutes, know what you're dealing with and which route to take"*).

```
build/bin/squid --allow-effects=recon --json effect recon "Super Mario 64 (U).z64"
build/bin/squid --allow-effects=recon --json effect recon "C:/Games/SomeUnityGame"
```

Refused by default like every effect; the refusal names what it would read.

# 2. What it answers

One JSON object, bounded: it does not grow with the size of the artifact (a
20,000-file listing answers in about 1.3 KB; [smart logs](/design/smart-logs.md)'s rule).

| field | what |
|---|---|
| `kind` | `rom`, `disc`, `install`, `archive`, `executable`, `unknown` |
| `platform`, `engine`, `title` | what recon concluded; `null` where it could not |
| `header` | what the header claims, in the format's own terms (game code, maker, file count, overlay count, byte order, entry point …) |
| `findings` | each conclusion with its **evidence** and a **confidence**: `verified` (a checksum the format defines was recomputed and agreed), `strong` (a magic number or signature file), `weak` (a name only) |
| `also` | other engines whose signals were present (launchers and helpers are common) |
| `routes` | for each depth (recon, bytes, opaque, source, structure), the tools that reach it, the executable name, and whether it is **on PATH** (`true`, `false`, or `null` when it cannot be found that way: a Python module, a GUI, a loader) |
| `residual` | what recon did not determine, in sentences |

The last route is always the structure depth, *"Void Squid reader + Void Bicho … NOT
BUILT"*. It is listed so the report says where the ladder ends today.

# 3. What it reads

| artifact | how it is recognised | checked by |
|---|---|---|
| Nintendo DS | header CRC-16 at 0x15E over 0x000–0x15D; logo CRC 0xCF56 | **recomputed** (`verified`) |
| Nintendo 64 | the first word, in all three dump byte orders (.z64, .v64, .n64), normalised before any field is read | magic (`strong`); the boot checksum covers 1 MiB and is not read |
| Game Boy Advance | 0x96 at 0xB2 and the complement byte at 0xBD | **recomputed** |
| Game Boy / Color | the logo at 0x104 and the header checksum at 0x14D | **recomputed** |
| GameCube, Wii | magic words at 0x1C and 0x18; RVZ and WIA compressed images recognised but not opened | magic |
| Godot pack, GameMaker data, Unity bundle, Ren'Py archive, ZIP (JAR, APK, LÖVE) | magic numbers | magic |
| Windows PE, ELF, Java class | signatures; PE machine; ELF class, byte order, machine | magic |
| an install folder | file names: Unity (Mono or IL2CPP), Unreal, Godot, GameMaker, Ren'Py, RPG Maker MV/MZ, Source and Source 2, XNA/FNA, LÖVE, Java, a bare executable | names only; the residual says so and suggests running recon on the signature file |

**The consoles are what recon adds.** universal-modder's `um scan` covers PC installs far
more deeply (engine versions, anti-cheat, loaders, save folders, a route per known game)
and recon does not try to match it: for a PC install the residual says *"`um scan` does"*
and the recon route lists `um`. Console headers are what `um` lacks, and they are the
platforms the family's own nibs target and decomp.dev tracks.

# 4. Where the line runs

- **Pure** (`squid_read`, links only the standard library): identifying from bytes and a
  listing, the route table, the report. Its test needs no Void Core DLL beside it; if it
  ever does, something effectful leaked in.
- **Effect** (`app.cpp`): the first KiB of one file, or a folder walk bounded at 20,000
  files and 8 levels, and one PATH lookup per tool. It writes nothing and runs nothing.
- **Derived, not truth**: recon holds nothing. Its answer is re-runnable from the path,
  so nothing goes into the document (rule 6). When a recon result is worth keeping, it
  will be because a piece was accepted onto a shelf, and that is a command.

# 5. What it found in its first run

Run against `third_party/universal-modder/` (a source tree, not a game), the first
version called it a Java game, because a Gradle wrapper `.jar` sat four folders deep.
Java and bare-executable signals now count only in the top two levels of a folder, and
the test carries the case.

# 6. Next

Done for the DS on 2026-10-10. Other consoles' dumps are still unread. Then the
ingress toolchain row (invoking `ndstool` under the gate), and the reader that turns its
output into an assetless `.ink` ([decompilation](/design/decompilation.md) §11).
