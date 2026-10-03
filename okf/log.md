# Log

Dated history of this bundle. Reserved file (OKF v0.1): listing and history only.

---

## 2026-10-03 — The direction: a forge for many runtimes

The author brought the Rust/Bevy game-rewrite scene (iw4L, a from-scratch MW2 runtime; a
mashup of MW2, Skate 3 and Minecraft in one executable) and set a direction:

> *"eventually [Void Squid] will be the engine to tie everything together, combine it all
> into 1 thing, and be the ultimate generalizable game creation tool."*

### What was worked out

[Engine horizon](/design/engine-horizon.md)'s founding claim, *Void Core's contribution to
a game engine is the editor, not the runtime*, turns out to be exactly the gap in that
ecosystem. The community is building runtimes and has no forge: Bevy 0.19 has no editor,
and every rewrite invents its own asset IR. So the author's "ultimate tool" reads as **one
forge that pours into many runtimes**: gather, compose, see residuals, build, and live-edit
a running Bevy game over the **Bevy Remote Protocol**, which is a holiday. The full page is
[the forge for many runtimes](/design/the-forge-for-many-runtimes.md). It includes how each
sibling the author named would plug in (Palabra, Allomone, Verguenza, Chisme, Loops Studio).

### What was ruled out, as leans

- **No Rust half in Squid.** It reaches Rust through files and BRP. A Bevy-side plugin
  belongs to a Bevy host (`VoidJelly:/design/open-questions.md` §18).
- **Squid composes structure, never mechanics.** Mechanics compose in ECS at frame rate,
  inside the runtime.

### What did not change

**v0 is still a harness.** New [S6](/design/open-questions.md) records the lean for when the
charter widens: when one `.ink` has been emitted to two targets with two correct, different
residuals. The ecosystem moving fast is an argument *for* proving the pipeline first.

Written: [the forge for many runtimes](/design/the-forge-for-many-runtimes.md). Trued up:
[engine horizon](/design/engine-horizon.md), [open questions](/design/open-questions.md)
(S6), `index.md`.

---

## 2026-08-26 — Founded

Void Squid founded by the author on the same day the 3D library was renamed **Void Jelly**
and this name moved here. **The OKF comes before the code, as always** — no code exists,
and every page is `status:planned`.

### The rename that produced this bundle

| was | is |
|---|---|
| Void Squid (the renderer/library, founded 2026-08-25) | **Void Jelly**, `../VoidJelly` |
| *(did not exist)* | **Void Squid** — this bundle, a minimal game engine |

Void Jelly's [log](../../VoidJelly/okf/log.md) carries the full record.

### Why the client was founded before the library has code

The author's reasoning, and it is correct:

> *"the reason i think we need the game engine NOW, super early, is purely because the game
> engine will be the thing to actually UTILIZE Void Jelly to create the 'export'."*

It follows from a property stated in the same breath: **Void Jelly is a library like
Palabra, not an application — it cannot render or create windows by itself.** A library in
that shape *cannot demonstrate itself*. It can emit the bytes of a `.nds` and cannot write
them; it can emit a draw list and cannot draw it. Founded without a host it would be
**unfalsifiable** — every design decision would look fine, because nothing was pulling on
it.

The family has evidence in both directions. Void Maiz organises its phases around
**exit-test clients**; Void Palabra chose **forcing clients** deliberately, recording *"what
each proves and what each blocks on."* And Void Maiz's docking bug is the counter-case:
three code paths were dead for weeks because *"a checked-in layout file is a first-run test
nobody is running"* — found only when a client with no inherited config appeared.

### What was decided at founding

1. **v0 is a conformance harness that happens to look like an editor**
   ([scope](/concepts/scope.md)). Not a hedge — a scope. Every proposed feature answers one
   question: *does this exercise a Void Jelly seam that is not otherwise exercised?*

2. **Void Squid owns every effect; Void Jelly owns every computation.** *The transformation
   is Jelly's; the transaction is Squid's.* The list of holidays Void Squid implements is
   [build surface](/concepts/build-surface.md), and the mechanical rule for anything new is:
   *could it run in a unit test with no filesystem, network or display?* Yes → Jelly. No →
   here.

3. **A Void Maiz application, deliberately** — which brings three things that would
   otherwise be work: the model lives in Void Core so there is no second data model; every
   gesture is a visible command so humans, scripts and agents share one surface; and
   **headless costs ~40 lines, not a port**. That last one matters most: a regression suite
   over residuals wants a command line, not a window.

4. **`--dry-run-effects` is a gift from the boundary, not a nicety.** Because Void Jelly is
   pure, the entire pipeline runs and produces a complete residual **without writing
   anything or needing devkitARM installed** — so *"will this fit on a DS?"* is answerable
   in CI on a bare machine.

5. **The viewport is a control experiment, not a feature.** The GL residual should be nearly
   empty; if the picture is wrong, the loss is in the `.ink` and not in any target. It is
   also the most-exercised emitter in the system by two orders of magnitude, since it runs
   every frame where the DS and voxel nibs run on request.

6. **The engine part waits.** [engine horizon](/design/engine-horizon.md) records the
   direction and the three seams v0 must not foreclose (non-visual runes, cheap emission, a
   symbol table in the blot) — each of which v0 wants anyway, which is the test that they
   are not speculative work.

### The trap, named at founding

The pull will not be *"abandon the harness and build an engine."* It will be *"while we are
here, this would be easy."* A physics step is a weekend; an input map is an afternoon. Each
is small, and together they are how Void Jelly ends up with one host that is a game engine
and a shape that fits nothing else.

Two questions guard against it, both already written down:
[railguard](/design/what-voidsquid-is-not.md) §7 — *am I building this because Void Jelly
needs it proved, or because it would be fun?* — and, from Jelly's side, *would a Blender
in-viewport preview want this too?*

### Written at founding

`index.md`, `app.md`, [roadmap](/roadmap.md) (interleaved with Void Jelly's), this log;
concepts [scope](/concepts/scope.md), [viewport](/concepts/viewport.md),
[build surface](/concepts/build-surface.md); design
[what Void Squid is NOT](/design/what-voidsquid-is-not.md),
[engine horizon](/design/engine-horizon.md), [open questions](/design/open-questions.md);
plus `README.md` and `CLAUDE.md`.

**Everything `status:planned`.** Void Squid should not exist as *code* until Void Jelly's
rung 2.5 — its first three rungs need no host, and starting earlier would mean an
application with nothing to open.
