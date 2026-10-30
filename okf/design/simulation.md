---
type: Design
title: Simulation — a world compiled to data, stepped in many instances, logged as events
description: "S17, decided 2026-10-10 by the author's delegation. A simulation is an artifact running under observation: the forge compiles a table (a scene and its Allomone rules) into inert data; a runtime that links nothing steps it deterministically; many instances share one static world and own only their dynamic state; a fork copies that state; the log is events, never positions. What the research on parallel and forkable simulation says, what was built, what it measured, and what comes next."
resource: src/sim/sim.cpp
tags: [status:current, audience:dev, audience:all, confidence:asserted, foundation]
timestamp: 2026-10-10T00:00:00Z
---

> The author, 2026-10-10: *"you decide right now, but also keep in mind if we want to
> simulate multiple instances at once, how might that happen. do research into
> simulation software to continue with this."* Decided and built the same day.
> `status:current` covers the runtime, the table compiler and the `effect sim` verbs.

# 1. The decision

**A simulation is an artifact running under observation, and its runtime links
nothing.**

```
   the forge (squid_app)                             the runtime (squid_sim)
   ─────────────────────                             ───────────────────────
   .ink ──► Void Jelly place()  ─┐                   World (read-only, shared)
                                 ├─► compile_table ─►   bodies: centre, half-extents,
   rules.allo ──► Void Allomone ─┘     (inert data)      dynamic / solid / region /
                  eval + merge                           gravity / speed / jitter / goal
                                                              │
                                                   Instance × K (each owns its state)
                                                     positions, velocities, flags,
                                                     seed, events
                                                              │ step(dt) — deterministic
                                                              ▼
                                                   events: enter · leave · contact
                                                           arrive · land
```

Three rules, each from the family's own pages:

1. **Rules compile to data** (railguard §6). The runtime never sees a rune, a tag or a
   rule. `squid_sim`'s link line is the standard library and nothing else, so the rule
   is enforced by the build, not by a comment.
2. **The forge never steps physics at frame rate** (Void Unity's two-clocks law). The
   forge compiles, starts, forks and reads; the runtime steps.
3. **The log is events** ([smart logs](/design/smart-logs.md)). A run of any length
   reports counts and the last *n* events; a comparison of many runs reports when they
   part ways. Positions are queried, never streamed.

# 2. What the research said

Gathered 2026-10-10 ([sources](/sources/simulation-research.md)); the parts that
shaped the code:

| finding | from | what it decided here |
|---|---|---|
| many sub-environments stepped together in one process, reset as a batch | Isaac Lab's `DirectRLEnv` (vectorised environments) | **one shared `World`, many small `Instance`s**, one `step_all` |
| a fork is a cheap copy that shares everything unchanged (copy-on-write); a checkpoint plus recorded inputs replays bit-exactly | filesystem model checkers, reverse debuggers, replay tools | **`fork` copies only dynamic state**; determinism makes a fork a replayable branch |
| sparse checkpoints (only the working set) let more simulations run at once | checkpointing research | the instance *is* the working set: the world is never copied |
| one controller driving many simulator processes needs a label per instance and one port each; threads share a client badly, processes work | SUMO / TraCI practice | instances are **values in one process**, addressed by index; a process-per-instance runner is the escape hatch for a runtime that is not ours |

The common thread, and the reason this fits Void Core so well: **a simulation is
state plus a pure step function, and everything good about multi-instance simulation
follows from keeping it that way.** Void Core's model is the same claim about
documents.

# 3. Many instances, and the shapes of "many"

"Simulate multiple instances at once" means at least four different things, and the
same structure serves all of them:

| shape | what differs between instances | here |
|---|---|---|
| **Monte Carlo** | the seed (per-instance jitter in speeds) | `sim new K seed` — K seeds, one world |
| **what-if branch** | a parameter from a point in time onward | `sim fork i newseed` at tick t, then step both |
| **replay check** | nothing: must be identical | `sim fork i` without a seed; `compare` says `identical: true` |
| **variant worlds** | the rules (a different table) | a second `sim new` with another rules file; today one World per session (see §6) |

And at larger scale, where a single process stops being enough:

- **Threads over instances.** Instances share nothing mutable, so `step_all` is an
  embarrassingly parallel loop. Not done yet; measured below as the obvious next win.
- **A process per instance, or a peer.** For a runtime that is not ours (Bevy, SUMO,
  a physics engine) or a machine that is not this one. The forge addresses each by
  label, exactly as TraCI does, and reads events back over a holiday.
- **Forks as documents.** Void Palabra versions and branches documents with structural
  sharing. A long-lived what-if is a branch of the *game document*; an instance is the
  in-memory, throwaway version of the same idea.

# 4. Allomone as the Table's language, in use

The author ruled on 2026-10-10 that Allomone is a programming language for Void Core
(S16). This is its first job here. A table's rules are an ordinary Allomone script:

```
when tag "kind:car"                      then dynamic "true"
when tag "kind:car"                      then speed "8"
when tag "kind:car" and tag "lane:fast"  then speed "20"     # more specific: wins
when tag "kind:wall"                     then solid "true"
when tag "kind:zone"                     then region "true"
```

**One finding, measured and reported upstream.** Within one script, two rules of equal
strength that disagree are settled silently by Allomone, by the smaller value **as a
string**: `"false"` beats `"true"`, and `"10"` beats `"9"`. That is a deliberate,
order-free design, and it is a trap for a game: the designer is not told, and numbers
compare as text. Void Squid therefore looks at the opinions **before** merging and
reports every such tie as a diagnostic (`tie: roadworks.solid has equal-strength rules
(true vs false); … make one rule more specific`). Written to Allomone as
`../VoidAllomone/MESSAGE_FOR_VOIDALLOMONE_voidsquid-a-game-engine-reads-your-ties-2026-10-10.md`.

# 5. What it does, and what it measured

The traffic preset (`effect make traffic`): a road, a crossing zone that is the goal, three
cars nose to tail in the slow lane, one car in the fast lane with roadworks across it, and a
crate dropped from 6 m. Three instances, 200 ticks of 50 ms, default rules. Each run's
whole story is **six events**:

```
tick 21   land     crate
tick 24   contact  car_c → car_b       (the queue forms)
tick 66   arrive   car_a → crossing    (seed 2; 76 and 78 for seeds 1 and 3)
tick 66   enter    car_a → crossing
tick 76   contact  car_fast → roadworks
tick 101  contact  car_b → car_a       (the queue closes up at the crossing)
```

Two bugs the first runs found, both fixed with a test: cars aimed at the goal's centre
and cut across lanes (now they aim at the nearest point of the goal); and a faster car
behind a slower one bumped, stopped and bumped again, logging 25 contacts in 10 s (now
it advances to contact and follows, and one blocking episode is one event).

Throughput (`squid_scale_bench`, single thread, 100 ticks):

| bodies | instances | wall | body-steps/s | `compare` answer |
|---|---|---|---|---|
| 101 | 1 | 70 ms | 144,000 | 95 B |
| 101 | 64 | 4.8 s | 136,000 | 4.7 KB |
| 401 | 1 | 1.1 s | 36,000 | 97 B |
| 401 | 64 | 72 s | 36,000 | 4.9 KB |

Instances scale linearly, as they should. Bodies do not: collision is all-pairs, so
quadrupling the bodies quarters the rate. The comparison grows with the number of
instances and not with the run's length.

# 6. Next, in order

1. **A broad phase** (a uniform grid over the world's footprint) so collision is not
   all-pairs. Past a few hundred bodies this is the whole cost.
2. **Threads over instances** in `step_all`, the free factor on this machine's cores.
3. **"What changed in view" per tick**: the spatial log's look, diffed against the
   previous tick, for an instance a person is watching.
4. **Sim in the GUI**: draw an instance's current positions over the scene (the bodies'
   transforms, not new geometry), with the event list beside it.
5. **Variant worlds**: more than one compiled table per session, so two rule sets can
   race in one comparison.
6. **Snapshot back to a document**: an instance's state written out as a new `.ink`
   (through Jelly's `create`), so a moment in a simulation becomes a scene someone can
   open, edit and branch.

# 7. What this is not

Not a physics engine: no rotation, no friction, no impulses, axis-aligned boxes only.
The point at this rung is the **shape** (compile, step, fork, compare, log events) with
the cheapest motion that makes a story. A real physics library slots in behind the same
`World` / `Instance` / events boundary, as a runtime, and the forge does not change.
