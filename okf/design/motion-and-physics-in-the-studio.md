---
type: Design
title: Motion and physics in the studio — Void Onda's judge read through an effect, Void Ocean's seam measured
description: "2026-10-18. Void Onda and Void Ocean are sibling libraries; the studio links neither. S34's lean (b) is built: `effect motion judge` starts the runtime's judge, `effect motion report` counts what it wrote (105 clips, mean 99.7%, lowest 97.0%, measured that day). S33's lean (b) stands, so Ocean is not linked; the seam it will cross is measured instead: of Good Egg Galaxy's 109 gravity fields, 49 have no exact Ocean primitive as Ocean is today."
resource: ../src/motion/judge.hpp
tags: [status:current, audience:dev, confidence:verified]
timestamp: 2026-10-18T00:00:00Z
---

# Motion and physics in the studio

[Void Onda](../../../VoidOnda/okf/index.md) (motion, judged from pixels) and
[Void Ocean](../../../VoidOcean/okf/index.md) (physics as fields) were founded from this session
on 2026-10-12 and 2026-10-13, and live in their own repositories. **The studio links neither.**
The runtime (the exercise's `goomba_galaxy2`, in `../ds-mashup`) links Onda; nothing links Ocean.
Mago agrees: `mago pair voidsquid voidonda` and `mago pair voidsquid voidocean` both answer
`seam library, cost retest`, from the siblings' own `interacts_with`. Squid's `void.json` names
neither, which is accurate for the binary this repository builds.

This page has two halves with different statuses: the motion half is **built** (the resource
above); the physics half is **specified, not built** (§2), on S33's lean.

## 1. Motion: S34's lean (b), first half — built

S34 ([open questions](/design/open-questions.md)) leaned *"(b) first: the studio shows the
runtime's `judge.json`"*, because the judge already writes everything the studio would show.
Nothing persisted from the judge's runs of 2026-10-13 and 14, so the studio has to be able to
start one as well as read one.

| op | what | cost |
|---|---|---|
| `effect motion judge <library> [out]` | starts `goomba_galaxy2 <library> --autotest judge` in `out` (default `<library>/judge`), returns at once; first removes that folder's old `judge.json` and `pace.json`, so a report is never stale | a program with its own window; writes two reports and a strip per clip |
| `effect motion report <folder>` | reads `judge.json` (written last, so its presence means the run finished) and `pace.json`, and answers the summary | reads two files |

**What the summary is** (`src/motion/judge.hpp`, pure; `tests/motion_test.cpp`): clips, their
mean and lowest overall rate, the checks under 80% (the runtime's own "weak"), per character the
same, and pace pooled as passed/trials per rule and speed. **What it is not:** a verdict. Every
rate and interval is Onda's, computed by the runtime from pixels, and is passed through unchanged;
no interval is recomputed here, so pooled pace carries none. That keeps Onda the one place that
judges, and makes this file deletable when S34's (a) links Onda.

`squid_motion` links the JSON header and nothing else, like `squid_read`: if its test ever needs
the Core DLL beside it, something effectful leaked in.

### Measured, 2026-10-18

One run through the studio's headless front-end, on the smg-workbench library (written to a
scratch folder, not the library):

| | |
|---|---|
| clips | **105** over 24 characters (Galaxy's two, Super Mario 64 DS's four, Mario Kart DS's thirteen, Donkey Kong 64's five) |
| mean overall rate | **99.7%** |
| lowest | `dk64.tiny` `air.land` (`onda_land`, synthesized, `confidence:asserted`): **97.0%** [93.6–98.6] |
| weak checks | **0** |
| feet keep pace, nearest pace (11 walkers, pooled) | 75.0% at 3 m/s, 74.4% at 6, 54.5% at 9.5 |
| the same, the rule before | 28.2%, 5.0%, 45.4% |

Consistent with Onda's [judging motion](../../../VoidOnda/okf/design/judging-motion.md) (75 clips,
mean 99.9%, lowest 98.7% on 2026-10-13, before the Kongs' 30 clips): the Kongs' made motion is
where the lowest clips now are.

### What is not built

- **A Motion window.** The studio's console runs both ops today; a dock window (a row per
  character, the lowest clip and its strip, the weak checks) is the next increment, and it is a
  projection of the report, so it owns no pixels beyond the strips the runtime drew.
- **The gallery from the studio** (S34 (b)'s second half). The runtime opens it with **G** while
  playing (`effect play`) and has no flag to open it on one character's clip; a
  `--gallery <character> <verb>` flag is the runtime's to add, and `effect motion gallery` would
  then be one more `launch`.
- **Onda linked** (S34 (a)), when the runtime moves in (S30).

## 2. Physics: S33's lean (b), and the seam measured — specified, not built

S33 leans *(b): link Ocean when the runtime moves in (S30)*, because a view of a field the runtime
does not use would be a picture of nothing. That stands. What can be done without linking is to
measure the translation Ocean's rung 2 will make, from the runtime's gravity onto Ocean's
`GravityPrimitive`, by reading both sides' code (`../ds-mashup/src/g2/world.cpp` `Gravity::load` /
`field_at`; `../VoidOcean/src/gravity.cpp`).

The runtime has seven field types (Petari's names); Ocean has five kinds. Field by field:

| runtime type | Ocean kind | carries? | residual |
|---|---|---|---|
| `point` | `point` (`a` = centre) | yes, but | `distant`: the runtime subtracts it from the distance before the range test and the tie; Ocean has no such field. The toolbox sets it to 500 × scale when a point's own `Distant` is missing |
| `segment` | `segment` (`a`, `b` = the ends) | yes, but | `distant`, as for points |
| `disk`, `disktorus` | `disk` (`a`, `b` = up, `radius`) | yes | the torus is already drawn as a disk by the runtime, and the runtime ignores a disk's `distant`: both the runtime's residuals, not the translation's |
| `plane` | `plane` (`a`, `b` = up) | yes | none |
| `planeinbox` | none | **no** | a plane that pulls only inside an oriented box; Ocean's plane is unbounded |
| `cube` | `box` | **only when axis-aligned** | Ocean's box is `a ± b`, axis-aligned; the runtime's is oriented by its matrix. Inside the box the runtime pulls toward the nearest face; Ocean's closest point is the point itself, so its pull there is zero |

And three things every type shares:

- **Strength.** The runtime's field gives a direction; the magnitude is one multiplier for the
  whole world (the feast's `gravity`). Ocean gives each primitive its own `strength`. Carrying the
  world means setting every primitive to that one value.
- **The rule.** Both pick the highest priority in range and the nearest on a tie (Petari's
  rule); with `distant` missing, "nearest" differs for points and segments.
- **Units and zones.** The runtime converts game units to metres on load, and the toolbox has
  already composed each zone's placement into the field's matrix; a translation should start from
  the loaded values, so neither is a residual.

**Counted on the workbench's worlds (2026-10-18):** Good Egg Galaxy's `world.json` has **109
fields**: 31 `cube`, 28 `planeinbox`, 27 `disk`, 14 `point`, 8 `segment`, 1 `plane`. So **49 of
109 have no exact Ocean primitive as Ocean is today**: the 28 `planeinbox` and the 21 `cube`s
whose matrix is rotated. The other 10 cubes are axis-aligned and carry everywhere except inside
the box. **14 more carry only up to `distant`**: 6 of the 14 points and all 8 segments have one.
8 of the 27 disks have a `distant` the runtime does not read. Super Mario 64 DS's castle and both
Donkey Kong 64 maps are one `plane` each and carry exactly. (One field, a segment, is `inverse`;
both sides have it.)

**What follows, and whose it is.** The translation and its defect are Ocean's (rung 2, its
`translation.hpp`): an oriented box, a bounded plane and a `distant` offset on Ocean's primitive
would make the 49 and the 14 carry, or Ocean can measure them as the residual of a lift into a rasterised
field. Either is Ocean's choice, made in Ocean's repository, so this finding is addressed to
Ocean and not acted on here. The Squid side, when S33 is ruled: an effect hands Ocean a world's
`gravity` array as bytes and shows the defect it answers; Squid evaluates no field itself
(railguard §3's spirit: the computation is the library's).

## 3. Beside MoleRat

Void MoleRat ([its page](/concepts/molerat.md), agent A, the same day) is a decompilation harness
with its own `effect molerat` ops. Nothing here overlaps it: `effect motion` reads a runtime's
reports and MoleRat recovers structure from artifacts. Where they will meet is Ocean's rung 2:
a recovered level's physics (MoleRat's *mapping* and *residual* records) is what Ocean's
translation would consume, and the residual table in §2 is the shape such a record would take
for gravity.
