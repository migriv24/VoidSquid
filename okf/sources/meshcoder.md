---
type: Source
title: MeshCoder — point clouds to editable, part-structured Blender scripts
description: Dai et al., "MeshCoder: LLM-Powered Structured Mesh Code Generation from Point Clouds," NeurIPS 2025 poster. A multimodal language model turns a point cloud into an executable Blender Python script decomposed into semantic parts; the code representation makes 3D shapes editable and helps language models reason about them. Linked by the author on 2026-10-09.
tags: [status:planned, audience:dev, confidence:verified, source]
timestamp: 2026-10-09T00:00:00Z
---

**Verified 2026-10-09** against the NeurIPS 2025 poster page
(`neurips.cc/virtual/2025/loc/san-diego/poster/117054`), abstract level only. Project
page: `daibingquan.github.io/MeshCoder`. No code repository was listed.

**Claim.** Existing shape-to-program methods rely on *"limited domain-specific
languages (DSLs) and small-scale datasets."* MeshCoder builds a set of Blender Python
APIs for detailed geometry, a large *"paired object-code dataset, where the code for each
object is decomposed into distinct semantic parts,"* and trains a multimodal model to
translate point clouds into executable scripts. *"Our code-based representation enhances
the reasoning capabilities of LLMs in 3D shape understanding tasks."*

**What it settles here.** Three things, all in the direction the family already leans:

1. **Structure is the representation a model can reason over.** The same finding as
   [ConceptGraphs](/sources/conceptgraphs-scenegpt.md), one level down: a shape as named
   parts beats a shape as points. Void Jelly's founding argument (structure survives;
   decimation does not) is this claim from the authoring side.
2. **It is a reader.** Point cloud in, structured program out, with whatever it could
   not express lost: an ingress in [construct and deconstruct](/concepts/construct-and-deconstruct.md)'s
   sense, whose output lands in Blender, which the Polyp already reads. A decompiled
   object could go *model → MeshCoder script → Blender → Polyp → .ink* with each step's
   loss named.
3. **It is a model, so it is Bicho's** if it is ever run here, behind a holiday, as a
   proposal with provenance.

**Not established:** quantitative results (the abstract gives none), hardware needs, or
whether weights are released.
