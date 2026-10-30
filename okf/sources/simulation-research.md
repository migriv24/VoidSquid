---
type: Source
title: Running many simulations — vectorised environments, forks and replay, multi-instance traffic
description: Isaac Lab's vectorised DirectRLEnv; copy-on-write forks and checkpoint replay (filesystem model checking, reverse debugging, replay tooling, sparse checkpointing); SUMO's TraCI driving several simulator instances. Gathered 2026-10-10 for simulation.md.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-10T00:00:00Z
---

**Confidence.** `asserted`: from web-search summaries on 2026-10-10, except where a
primary page was in the results (Isaac Lab's source, SUMO's TraCI documentation).
Nothing here was run.

**Vectorised environments (Isaac Lab).** `DirectRLEnv` is *"designed to be used with
vectorized environments, i.e., the environment is expected to be run in parallel with
multiple sub-environments."* It does not subclass Gymnasium's `VectorEnv`, *"because
each RL library typically has its own definition for a vectorized environment,"* and
resets terminated sub-environments inside `step` because *"the simulator does not
support resetting individual sub-environments."* Lesson taken: one shared world, many
small states, one step over all of them.

**Forks, checkpoints and replay.** A 2026 filesystem model checker (arXiv 2608.01135)
reuses a copy-on-write fork as a checkpoint and replays an execution bit-exactly by
recording its nondeterministic inputs. Reverse-execution patents describe checkpoints as
suspended clones sharing memory until written. Replay.io's engineering blog notes forks
are cheap through copy-on-write but accumulate; a KIT thesis on sparse checkpointing
reports up to 79% less memory by restoring only each interval's working set, allowing
more concurrent simulations. Lesson taken: copy only the dynamic state, keep the step
deterministic, and a fork is a replayable branch.

**Many traffic simulators from one controller (SUMO / TraCI).** TraCI's Python client
can drive several SUMO instances from one script, each started with a `label` and
selected with `traci.switch()` (SUMO documentation, *Interfacing TraCI from Python*).
Each instance needs its own port; maintainers advise processes over threads for
parallel runs. Lesson taken: instances addressed by label or index; a process per
instance is the route for a runtime we do not own.

**Not covered:** MuJoCo MJX and Brax (GPU-batched physics), SimPy (discrete-event
simulation). Both are worth a reading pass before the runtime grows real physics or
event queues.
