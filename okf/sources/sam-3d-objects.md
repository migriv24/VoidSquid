---
type: Source
title: SAM 3D Objects — single image to 3D objects with pose, shape, texture and layout
description: Meta, "SAM 3D: 3Dfy Anything in Images" (arXiv 2511.16624, 2025-11-19). From an image and an object mask, reconstructs the object in 3D (Gaussian splat or mesh) with pose and layout. SAM License. Needs Linux and a 32 GB GPU. Vendored as code only into Void Bicho on 2026-10-09; cannot run on this machine.
tags: [status:planned, audience:dev, confidence:verified, source]
timestamp: 2026-10-09T00:00:00Z
---

**Verified 2026-10-09** against the repository (`github.com/facebookresearch/sam-3d-objects`,
commit `f91db41`, 2026-06-02), its README, `doc/setup.md` and `LICENSE`.

**What it is.** It *"can convert masked objects in an image, into 3D models with pose,
shape, texture, and layout,"* single or multiple objects. The quick start saves a
Gaussian splat (`.ply`); the notebooks also produce `.glb` meshes. Input is an image plus
a mask, which is to say it sits **after** a 2D segmenter.

**Requirements.** *"A linux 64-bits architecture"* and *"A NVIDIA GPU with at least 32
Gb of VRAM."* Weights are gated on Hugging Face (`facebook/sam-3d-objects`; request
access, then authenticate). Licence: the **SAM License** (Meta, 2025-11-19), which
permits use, modification and redistribution under its terms; it is not MIT, so it is
recorded separately from the family's code.

**Where it lives.** `../VoidBicho/third_party/sam-3d-objects/`, code and documentation
only (6.8 MB): the git history, the sample photographs (stock images) and a demo GIF were
removed to save disk; no weights. See that folder's `README` in Bicho.

**What it settles here.** The author wanted the engine to have *"a better understanding
of 3D space."* SAM 3D is the clearest current instance of **layer 3** in
[smart logs](/design/smart-logs.md) §3 going all the way to geometry: from a picture to
objects with places. For Void Squid it is a **decompilation and asset-generation** tool
([three uses](/design/three-uses.md)): a screenshot of a game becomes proposed turkeys
with meshes and positions, each a proposal with provenance, never truth. It is not a
runtime dependency of anything, and on this laptop (6 GB, Windows) it can only be a
reference until a larger machine or a peer runs it (Bicho's `peer` backend).
