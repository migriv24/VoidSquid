# Void Squid — agent instructions

You are working on **Void Squid**: the game engine of the Void family, on **Void Core**
(`../VoidCore`) and **Void Maiz** (`../VoidMaiz`). Its first job is to be the host that
**Void Jelly** (`../VoidJelly`) cannot be for itself. Its shape after that, written down
on 2026-10-09, is three pillars: a pure **game model**, a **studio** of projections, and
**hosts** that differ only in their effect list.

Founded 2026-08-26. **0.1.0, the studio, is built (2026-10-12)**
([`okf/design/void-squid-0.1-the-studio.md`](okf/design/void-squid-0.1-the-studio.md)): a
docked Void Maiz window over a `.squid` project (a Void Core state document), a game
library's catalogue as mantles, Turkey/Table/Feast/Data/Scene workspaces, Load a game and
Setup ([`okf/design/load-a-game.md`](okf/design/load-a-game.md)). Rung 2.5's `.ink` harness
is its Scene workspace. Build with `cmake -S . -B build -G Ninja`, test with
`ctest --test-dir build -R squid_`; `build/bin/voidsquid` is the window. **The author,
2026-10-12: Void Squid is about constructing worlds and exporting games; decompilation is a
feature, and loading a game is step one.** The game model is still design only (S14).

## Ground rules

1. **OKF first.** `okf/` is the source of truth for design; code follows concepts, never the
   other way. Read [`okf/index.md`](okf/index.md) before doing anything. Every session that
   changes design or code captures it in `okf/log.md` and trues up the affected pages.

2. **v0 is a conformance harness that happens to look like an editor.** Not a hedge — the
   scope ([`okf/concepts/scope.md`](okf/concepts/scope.md)). The author's charter: *"like a
   BARE MINIMUM thing… purely to be testing Void Jelly."* Every proposed v0 feature answers
   one question: **does this exercise a Void Jelly seam that is not otherwise exercised?**
   v0 did not move on 2026-10-09; it became the first instance of the pillars.

3. **The three pillars separate, and the seams are Maiz's.**
   [`okf/design/the-three-pillars.md`](okf/design/the-three-pillars.md). The game model
   ([`okf/concepts/what-a-game-is.md`](okf/concepts/what-a-game-is.md)) is pure. The studio
   ([`okf/concepts/studio-surfaces.md`](okf/concepts/studio-surfaces.md)) is `project` /
   `compile` pairs over one document and **owns no pixels**, because on Latin OS windows
   never send pixels. The hosts ([`okf/concepts/hosts.md`](okf/concepts/hosts.md)) are the
   effect list, once per platform. Anything proposed for the game model passes three tests
   (railguard §8): sayable in Void Core's vocabulary; runs with no filesystem, network,
   display or clock; recoverable by a reader from a game nobody here made.

4. **Void Squid computes nothing about geometry.** Decimation, voxelisation, budget
   allocation, texture packing, fixed-point conversion, display-list encoding — all Void
   Jelly's, without exception. **If there is a loop over vertices in this repository,
   something has gone wrong.**

5. **Void Squid owns every effect.** Files, processes, the window, the GL context, the
   emulator, caches, the clock; later an ingress toolchain and a hypervisor.
   [`okf/concepts/build-surface.md`](okf/concepts/build-surface.md) is the list. The rule
   for anything new: **could it run in a unit test with no filesystem, network or
   display?** Yes → a library (Jelly, or the game model). No → a host.

6. **Void Squid owns no truth.** Void Maiz's founding commitment, inherited: the model lives
   in Void Core, including view state like camera position. A model's output
   (the Dungeon Master's generative job, Void Bicho) is a proposal; the mantle is the
   record of what was accepted (S13).

7. **Void Squid does not own `.ink`.** It reads and writes it *through Void Jelly*, never
   parses it directly, never adds a field. The game model lives in **its own document**
   that references scenes (S12); no gameplay glyph goes into an `.ink` (S9).

8. **Both arrows.** The engine deconstructs as well as constructs
   ([`okf/concepts/construct-and-deconstruct.md`](okf/concepts/construct-and-deconstruct.md)).
   A reader is the inverse of a nib and produces a residual for what it could not recover.
   REA is an **ingress toolchain**, discovered and invoked like devkitARM, never linked
   (S10). Every noun in the game model must be emittable and recoverable.
   **Decompilation is central** (the author, 2026-10-10;
   [`okf/design/decompilation.md`](okf/design/decompilation.md)): the goal is structure,
   not source for its own sake. Every recovered piece states its depth (bytes, opaque,
   source, structure) and the oracle behind it. **Void Bicho is load-bearing** for it.
   No game bytes and no decompiled code ever enter this repository.
   `third_party/universal-modder/` is a reference: read it, never run it from there.

9. **Read the railguard before adding a feature.**
   [`okf/design/what-voidsquid-is-not.md`](okf/design/what-voidsquid-is-not.md). §6 is
   restated: **the artifact never ships the forge**; it carries Void Core only when Core is
   the artifact's own computation (an operating system), never for a game. §7's test: **am I
   building this because Void Jelly needs it proved, or because it would be fun?** §8 is the
   game model's test.

10. **Report findings upstream; do not fix siblings from here.** Draft a message file in
    the recipient's repo root: `MESSAGE_FOR_<RECIPIENT>_voidsquid-<topic>-<YYYY-MM-DD>.md`,
    with a matching self-identifying H1. **Never** a bare `MESSAGE_FOR_<RECIPIENT>.md`.
    `okf/index.md` tracks open vs consumed; retire by the index, not by deletion. Latin OS
    depends on nothing here and is never edited from here.

11. **Honesty tags are not decoration.** No `status:current` without a `resource:` link to
    code. A recalled fact is `confidence:asserted`, and saying so is the point. Twenty-three of
    the twenty-eight source pages in `okf/sources/` are `asserted`; raising one to
    `verified` means opening the work, not searching for it.

12. **Vendor, don't depend.** Family convention: third-party pieces vendored with licences.
    No package managers, no frameworks. C++20, like Void Maiz. A toolchain the host
    *invokes* (devkitARM, `cargo`, REA) is not a dependency and is not vendored.

13. **The author decides.** Open decisions go to
    [`okf/design/open-questions.md`](okf/design/open-questions.md) with a lean. S14 (when
    the game model is built) is the one that most wants a ruling. Three of Void Jelly's
    questions block work here: its **§10** blocks the viewport, its **§11** blocks the build
    surface, its **§19** decides whether the forge may hold a live scene.

## The sibling bundles

- `../VoidJelly/okf/` — the library. **Read `design/library-boundary.md` before writing any
  integration code**; it is the contract between these two repositories.
- `../VoidCore/okf/` — rune, mantle, holiday, glyph, tags, links, dispatcher. Its
  forge-not-artifact principle (`design/domains-and-guarantees.md`) is why Void Squid is
  the forge and the `.nds`, the Bevy app and the OS image are artifacts.
- `../VoidMaiz/okf/` — what this is built with. `concepts/views-as-projections.md` is the
  seam pillar 2 is made of; `concepts/headless-adoption.md` is the ~40-line path to a CLI.
- `../LatinOS/okf/` — `design/the-os-as-a-game.md` is the mirror of our
  `concepts/what-a-game-is.md` §8; `design/kernel.md` and `design/mesh-compositor.md` are
  what the LITO host is written against.

## What to do next

**The studio's next steps** are in the roadmap's rung 8: draw a turkey in 3D once Void
Jelly reads a glTF (asked); the local model's first job (S29); the ingest routes moving in
from `../ds-mashup`; the runtime and export (S30). Keep the studio's rules: every edit a
command through the Session, no geometry in the window, every effect declared, and no game
data, demo or model weights in the repository or a release. Coral is for sharing *play*;
editing together is Void Maiz's.


Rung 3: the build button, which is v0's exit condition and waits on Void Jelly's DS nib
(its rung 3) and devkitPro being installed. Rung 2.5's remaining slice is a `.ink` born in
Blender (Jelly's rung 2, the Polyp) and the emission-cost measurement that needs it. The
game model is pure and is the exception the author may rule on (S14); until then it is
designed, not built. On the evening of 2026-10-13 Load a game became one button (downloads, loading bars, the Minimum setup by default), the Games window replaced Progress, and **Void Onda** (`../VoidOnda`) was founded for motion: every character plays through it, `--autotest judge` judges every clip from pixels, and **G** opens the 3x3 gallery. Before that, on 2026-10-13 the studio gained 3D views through Jelly's new glTF reader, Profile and Settings, a Progress window measured against wikis and Wikipedia, and a small-model pilot (S29); twenty characters are playable in the exercise ([`okf/design/the-cast.md`](okf/design/the-cast.md)), and **Void Ocean** (`../VoidOcean`) was founded as the family's physics, research phase (S33). On 2026-10-18 `effect motion judge|report` gave the studio the runtime's Onda judge (S34 (b)) without linking Onda, and Ocean's gravity seam was measured, not linked ([`okf/design/motion-and-physics-in-the-studio.md`](okf/design/motion-and-physics-in-the-studio.md)). Decompilation's first stage, recon (`effect recon`), is built; its
next steps are rung 9a–9e in the roadmap, split by depth, with S20 asking the author
whether that order stands. See [`okf/roadmap.md`](okf/roadmap.md). Keep `mago doctor` at zero
errors on our line, and the Jelly and Squid test suites green together.
