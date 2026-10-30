---
type: Design
title: Smart logs — the CLI as context management for worlds of any size
description: "How a Void Squid session tells a person or an agent what is in a world of thousands of objects without drowning either: logs are answers to bounded questions, not streams; four layers of perception (structure, geometry, render, learned), each optional above the first; the spatial log (survey, near, look) built and measured; what the 3D scene-understanding literature says about feeding a world to a language model; and what is next."
resource: src/app.cpp
tags: [status:current, audience:dev, audience:all, confidence:asserted, foundation]
timestamp: 2026-10-09T00:00:00Z
---

> The author, 2026-10-09: *"the void core is based on the cli, which should be able to
> log what is on the screen. … with a 3d space, we can maybe log spatial position
> stuff, but we should also be able to log visual stuff … Assume that there are
> massive worlds, with thousands of objects to load. How are we going to manage the
> logs? I feel like we might need spatial logs as well, and abilities to understand
> what's happening in any given scene."*
>
> `status:current` covers the **spatial log** (§4), built and measured the same day.
> The render layer and the learned layer (§3) are `planned`.

# 1. The problem, stated as a budget

Void Core's founding promise is total observability: every gesture is a logged,
replayable command. That promise was written for documents with tens of runes. A
world has tens of thousands, and the reader of the log is increasingly an agent with
a context window. **A log that grows with the world is a log nobody can read.** So
the question is not "what do we log?" but "**what does it cost to know something?**",
and the answer has to be bounded.

Measured on 2026-10-09 (`tests/scale_bench.cpp`, a synthetic world of instanced
meshes on a grid, best of three, this laptop):

| objects | polygons | `.ink` | full GL emit | emit, hash supplied | survey | near | look | survey answer | near answer | look answer (20) |
|---|---|---|---|---|---|---|---|---|---|---|
| 100 | 10,764 | 54 KB | 5.7 ms | 1.8 ms | 1.2 ms | 0.7 ms | 0.8 ms | 10.9 KB | 1.0 KB | 2.9 KB |
| 1,000 | 108,564 | 402 KB | 46 ms | 14 ms | 9.5 ms | 9.4 ms | 7.8 ms | 14.5 KB | 1.0 KB | 3.0 KB |
| 10,000 | 1,086,564 | 3.9 MB | 477 ms | 156 ms | 76 ms | 58 ms | 83 ms | 15.1 KB | 1.1 KB | 3.0 KB |
| 30,000 | 3,260,000 | 11.8 MB | 1,442 ms | 476 ms | 260 ms | 210 ms | 217 ms | 15.5 KB | 1.0 KB | 3.0 KB |

Two findings, both load-bearing:

1. **The answers stay the same size while the world grows three hundredfold.** A
   `look` is ~3 KB at 100 objects and at 30,000. That is the property a smart log
   needs, and it holds because each query is truncated by design (a grid of fixed
   resolution; the top 20 by screen size; the nearest 20).
2. **Full re-emission is not interactive past about a thousand objects.** Even with
   the hash supplied (the first optimisation the bench forced; see the log) it is 156
   ms at 10,000. Scope claim 4 is answered: *no*, not by full re-emission. Jelly's
   seam 2 (incremental emission) is now a measured requirement, not a precaution.

# 2. The principle: a log is the answer to a question

> **Do not stream the world into the log. Ask it questions, and log the answers.**

Every entry in a smart log is a command and its reply, like every other Void Core
log line. What changes is which commands exist. Instead of "here are 30,000 objects,"
a session asks:

| question | command | answer is bounded by |
|---|---|---|
| what is this world, roughly? | `effect survey [cells]` | the grid: at most `cells³` cells, 5 ids and 3 tags each |
| what is near this point? | `effect near x y z r [limit]` | `limit` |
| what is on screen? | `effect look [camera] [aspect] [limit]` | `limit`, largest first |
| what is this one thing? | `get <rune>`, `describe <rune>` (Core) | one rune |
| what changed? | `diff`, `status` (Core) | the edit, not the world |

This is the same move [Void Maiz's headless design](../../VoidMaiz/okf/concepts/headless.md)
made for applications (*the briefing is assembled from registries, never authored*)
and that [Void Jelly's residual](../../VoidJelly/okf/concepts/residual.md) made for
losses (*summarise by cause, drill down by subject*). **Summaries first, detail on
request, every step a command an agent can repeat.**

It is also what the 3D scene-understanding literature converged on independently
([ConceptGraphs and SceneGPT](/sources/conceptgraphs-scenegpt.md)): give a language
model an **object-centric graph with spatial relations as text**, not dense
per-point features, because dense maps *"do not scale well in larger environments."*
A Void Core mantle is already that graph. The spatial log adds the spatial relations
on demand.

# 3. Four layers of perception

"Log what is on the screen" means four different things, with four different owners
and four different costs. Each layer is **optional above the first**, and a session
says which it used.

| layer | answers | computed by | effect? | status |
|---|---|---|---|---|
| **0 · structure** | what exists, what it is called, how it is tagged and linked | Void Core (`ls`, `find`, `tree`, `links`) | none | built (Core) |
| **1 · geometry** | where things are, what is near, what is inside the view frustum | **Void Jelly** (`survey`, `near`, `look`), pure | none | **built 2026-10-09** |
| **2 · render** | what is actually visible (occlusion), how many pixels each rune covers, a screenshot | a **host** draws an ID buffer, reads it back | GPU, readback | planned |
| **3 · learned** | what it *looks like*: segments, labels, captions, a 3D reconstruction of an image | a **model**, through **Void Bicho** | model inference | planned; optional by construction |

The layers are ordered by cost and by honesty. Layer 1 is exact about what it
computes and says what it does not: `look` reports `"occlusion":"not computed"`, and
on the sample scene it lists the statue and the pivot, which are inside crate_0 and
cannot be seen. Layer 2 is where that becomes true: render each rune's id into an
offscreen buffer, count pixels per id, and the log can say *crate_0 covers 31% of the
screen; statue 0 px, occluded*. That is a host effect (the GPU), it needs no model,
and it is the next thing to build here.

Layer 3 is the author's *"segmentation models, or models that can enrich the logs …
they should be optional."* A model never produces truth here. It produces an
**annotation with provenance** attached to runes the host already knows (the ID
buffer from layer 2 is how a 2D mask gets back to a rune), recorded the way Bicho's
invocation contract records every model call. Its owner is Bicho, not Jelly and not
Squid; see `VoidBicho:/concepts/perception.md`. It matters most where layer 0 is
missing, which is **decompilation** ([three uses](/design/three-uses.md)): a game
nobody tagged, whose objects have to be found before they can be named.

# 4. The spatial log, as built

All three queries are Void Jelly's (`VoidJelly:/concepts/spatial.md`), pure, in scaled
units (Ink units × `unit_scale`), and deterministic: the same scene and the same
question produce the same bytes, so two log entries can be diffed.

- **`survey`** places every object (world AABB through its parent chain; empties as
  points), buckets centres into a grid, and reports per occupied cell the count, five
  sample ids and the three most common tags. It is the *"what is this world?"* entry
  an agent reads first.
- **`near`** is distance from a point to each object's bounds, nearest first.
- **`look`** tests each bounding sphere against the camera's frustum, and reports the
  visible ones largest-on-screen first with their screen position and distance, plus
  counts off screen and behind.

In Void Squid they are `effect survey|near|look`, granted and journalled like every
effect. The GUI dispatches `effect look` **once per settled camera** (debounced 0.4 s,
so a wheel spin is one entry) and shows the answer in a **Look** panel; the transcript
therefore carries a line for every view the person actually stopped on. A headless
agent with no window can do the same with the camera stored in the document, or with
no camera at all (`effect look` frames the whole scene).

# 5. What a world-sized session needs next, in order

1. **Layer 2: the ID buffer.** True occlusion and pixel coverage per rune, and a
   screenshot effect, in the desktop host. No model, no new dependency.
2. **A spatial index that survives between questions.** Every query today re-places
   the whole scene (~2.5 µs per object). A host can cache `place()` keyed by the
   `.ink` hash; past that, Jelly wants a BVH or a grid it can hand back. Pure, so
   Jelly's.
3. **"What changed in view."** A look diffed against the previous look: *crate_7
   entered, door_2 left the frame.* This is the log line a simulation produces every
   tick (see [three uses](/design/three-uses.md), simulate), and it is a diff of two
   values the log already holds.
4. **Levels of attention.** An agent's focus as a query, not a camera: a region, a tag
   filter, a rune and its neighbourhood. `survey` restricted to a cell or a tag is
   the first step; the shape is Core's filter grammar, not a new one.
5. **Incremental emission** (Jelly seam 2), forced by §1's table.
6. **Layer 3 adapters in Bicho**, starting with something that runs on this machine
   (a small 2D segmenter on CPU; see `VoidBicho:/references/3d-ai-landscape.md`).

# 6. What a smart log must never do

- **Summarise without saying so.** Every truncated answer carries its total
  (`visible_total`, `count`, `occupied_cells`) so the reader knows what it is not
  seeing.
- **Claim more than it measured.** `occlusion: not computed` until layer 2 exists.
- **Let a model's answer become the record.** A layer-3 annotation is a proposal with
  provenance; the mantle is what was accepted.
- **Grow with the world.** A query whose answer is proportional to the scene is a
  dump, and belongs behind a flag a person sets on purpose.
