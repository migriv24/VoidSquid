# Void Squid — agent instructions

You are working on **Void Squid**: a minimal game engine on **Void Core** (`../VoidCore`)
and **Void Maiz** (`../VoidMaiz`), whose first and only job is to be the host that
**Void Jelly** (`../VoidJelly`) cannot be for itself.

Founded 2026-08-26, on the day the 3D library was renamed Void Jelly and this name moved
here. **Nothing is built.**

## Ground rules

1. **OKF first.** `okf/` is the source of truth for design; code follows concepts, never the
   other way. Read [`okf/index.md`](okf/index.md) before doing anything. Every session that
   changes design or code captures it in `okf/log.md` and trues up the affected pages.

2. **v0 is a conformance harness that happens to look like an editor.** Not a hedge — the
   scope ([`okf/concepts/scope.md`](okf/concepts/scope.md)). The author's charter: *"like a
   BARE MINIMUM thing… purely to be testing Void Jelly."* Every proposed feature answers one
   question:
   **does this exercise a Void Jelly seam that is not otherwise exercised?**
   Unglamorous features that pass are in. Exciting features that fail are out, and being
   obviously necessary *later* is not an argument for building them now.

3. **Void Squid computes nothing about geometry.** Decimation, voxelisation, budget
   allocation, texture packing, fixed-point conversion, display-list encoding — all Void
   Jelly's, without exception. **If there is a loop over vertices in this repository,
   something has gone wrong.**

4. **Void Squid owns every effect.** Files, processes, the window, the GL context, the
   emulator, caches, the clock. Void Jelly performs none of them.
   [`okf/concepts/build-surface.md`](okf/concepts/build-surface.md) is the list, and the rule
   for anything new is mechanical:
   **could it run in a unit test with no filesystem, network or display?** Yes → Void Jelly.
   No → here.

5. **Void Squid owns no truth.** Void Maiz's founding commitment, inherited: the model lives
   in Void Core, including view state like camera position. If a source-of-truth data
   structure appears here, stop and re-read
   `../VoidMaiz/okf/concepts/log-first-inheritance.md`.

6. **Void Squid does not own `.ink`.** It reads and writes it *through Void Jelly*, never
   parses it directly, never adds a field. A format with two owners drifts — the family
   precedent is Void Maiz deleting its own copies of Void Core's argument codec rather than
   keeping four in sync, one of which had a security hole nobody had found.

7. **Read the railguard before adding a feature.**
   [`okf/design/what-voidsquid-is-not.md`](okf/design/what-voidsquid-is-not.md). Summary
   test: **am I building this because Void Jelly needs it proved, or because it would be
   fun?** Both are legitimate reasons to build software; only the first is a reason to build
   it *now, here, before the library works*.

8. **Report findings upstream; do not fix Void Jelly from here.** When the boundary is
   awkward, that awkwardness *is* the finding — measured, from outside, in the pattern Void
   Hormiga and Void Maiz use. Draft it as a message file in the recipient's repo root:
   `MESSAGE_FOR_VOIDJELLY_voidsquid-<topic>-<YYYY-MM-DD>.md`, with a matching
   self-identifying H1. **Never** a bare `MESSAGE_FOR_<RECIPIENT>.md`. `okf/index.md` tracks
   open vs consumed; retire by the index, not by deletion.

9. **Honesty tags are not decoration.** No `status:current` without a `resource:` link to
   code. A recalled fact is `confidence:asserted`, and saying so is the point.

10. **Vendor, don't depend.** Family convention (Core, Maiz, Palabra): third-party pieces
    vendored with licences. No package managers, no frameworks. C++20, like Void Maiz.

11. **The author decides.** Open decisions go to
    [`okf/design/open-questions.md`](okf/design/open-questions.md) with a lean. Three of Void
    Jelly's block work here: its **§10** (does Jelly ship GL code?) blocks the viewport, its
    **§11** (who owns the DS host program?) blocks the build surface, and its **§5** (Jelly's
    language) settles S1.

## The sibling bundles

- `../VoidJelly/okf/` — the library. **Read `design/library-boundary.md` before writing any
  integration code**; it is the contract between these two repositories.
- `../VoidCore/okf/` — rune, mantle, holiday, glyph, tags, links, dispatcher. Its
  forge-not-artifact principle (`design/domains-and-guarantees.md`) is why Void Squid is
  the forge and the `.nds` is the artifact.
- `../VoidMaiz/okf/` — what this is built with. `concepts/headless-adoption.md` is the
  ~40-line path to a CLI; `concepts/views-as-projections.md` is the host-view seam the
  viewport is built against.

## What to do first

**Nothing, as code.** Void Squid should not exist as code until Void Jelly's rung 2.5 — its
first three rungs (verify the DS sheet, minimal `.ink`, the Polyp) need no host, and starting
earlier means an application with nothing to open. See [`okf/roadmap.md`](okf/roadmap.md).
