---
type: Source
title: ConceptGraphs and SceneGPT — open-vocabulary 3D scene graphs for language models
description: Gu et al., ConceptGraphs (arXiv 2309.16650, ICRA 2024); SceneGPT (arXiv 2408.06926, 2024). An object-centric 3D scene graph, with spatial relations, serialised as text, is what a language model reasons over best; dense per-point features do not scale. The design premise of Void Squid's smart logs.
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-09T00:00:00Z
---

**Claim this bundle leans on.** ConceptGraphs builds a 3D scene graph by fusing
class-agnostic 2D masks into 3D objects, tagging each with a vision-language model,
and inferring spatial relations between objects; the authors argue that dense
per-point feature maps *"do not scale well in larger environments"* and lack the
relations a planner needs, while a scene graph *"maps naturally to natural language
formats to interface with LLMs."* SceneGPT feeds that graph to a pre-trained language
model with in-context prompting, training-free, for object, geometric and spatial
questions. A later method (Sparse3DPR) reports large gains over ConceptGraphs by adding
hierarchy and **task-adaptive subgraph extraction**, which is to say: give the model
less of the graph, chosen for the question.

**What it settles here.** [Smart logs](/design/smart-logs.md) §2: a Void Core mantle is
already an object-centric graph, so a session should answer spatial questions with
bounded subgraphs (survey, near, look) rather than streaming the world. The
"task-adaptive subgraph" finding is the argument for §5.4, levels of attention.

**Confidence.** `asserted`: from search-result summaries of the papers on 2026-10-09,
not from reading them. Verify venue years and the quoted phrases against the PDFs. The
Sparse3DPR gains are its authors' own comparison.
