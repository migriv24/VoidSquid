---
type: Source
title: Matching decompilation and static recompilation (decomp.dev, the community list, N64Recomp)
description: "The two community traditions that recover console games. Matching decompilation writes C that compiles back to the identical binary, measured per function (decomp.dev tracks the projects); static recompilation translates machine code to C against a runtime that reimplements the console, so the original runs natively without being understood (N64Recomp). One is the source depth with a byte-exact oracle; the other is the opaque depth."
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-10T00:00:00Z
---

**Confidence, per claim.** decomp.dev's front page and the community list were **fetched
2026-10-10** and summarised by a reading tool, not read line by line; the tool chain
named below comes from universal-modder's retro-decompilation playbook (read in full) and
web search summaries of the decomp-toolkit and N64Recomp READMEs (not opened). Tagged
`asserted` until someone opens N64Recomp's README and one decomp project's build.

## Matching decompilation

**decomp.dev** (`https://decomp.dev/`) tracks projects that *"attempt to write source code
(C, C++) that compiles to the same binary as the original"*, and notes *"all source code
is written from scratch."* Projects are grouped by platform (GameCube, Wii, N64,
PlayStation, Switch and others) and each shows a **decompiled** percentage and, for some,
a **fully linked** percentage. On the day it was fetched: Metroid Prime 90.83% / 54.69%,
Super Mario Galaxy 100% / 68.75%, Pikmin 2 99.63% / 98.09%, Star Fox Adventures 100% /
100%.

**SamidyFR/Game-Decompilations** (`https://github.com/SamidyFR/Game-Decompilations`) is an
alphabetical community list of roughly 400 entries, mixing decompilations, disassemblies
(NES, Game Boy), recompilations and ports, labelled in parentheses. Its introduction:
*"Below is a list of game de-compilations I could gather."* It is a work in progress and
says so.

**The tools**, as universal-modder describes them: **splat** splits a ROM into assembly and
data; **m2c** gives a first-pass C decompile; **asm-differ** and **objdiff** compare a
compiled function with the original; **decomp-permuter** searches for code shapes that
match; **decomp.me** shares scratches. For GameCube and Wii, **decomp-toolkit** (`dtk`)
splits the original binary into relocatable objects and generates the linker script, so
the decompiled code relinks *byte for byte*.

**What it settles here.** Matching is **the lens law at the source depth**:
`compile(source) == binary`, per function. The percentage is an ingress residual. It is
also the strongest reward a learning system could ask for, because it cannot be argued
with ([decompilation](/design/decompilation.md) §3, §8).

## Static recompilation

**N64Recomp** takes a binary plus symbols and metadata, splits it into functions and
translates each instruction literally into C, which is compiled for a modern platform and
linked against **N64ModernRuntime**; Zelda 64: Recompiled runs Majora's Mask natively that
way, with the RT64 renderer, from the person's own ROM. Equivalents named by
universal-modder: XenonRecomp (Xbox 360), PS2Recomp, DolRecomp (GameCube/Wii),
psxrecomp (PS1).

**What it settles here.** Recompilation is **the opaque depth**: the original's behaviour,
running, without being understood. universal-modder calls a recompilation *"the best
oracle for a clean reimplementation"*, which is exactly its role in this design: the
behaviour a structure-depth lift must reproduce.

## Both, together

**libsm64** (from the Super Mario 64 decompilation) shows what a decompilation becomes
when it is wrapped with a contract: a library any host can feed collision and input to.
The community norm is the same across all of these: the person supplies their own dump;
no project ships game assets.
