---
type: Source
title: Computer vision in the age of language models — concepts, parts, marks and scene graphs (2023–2026)
description: "A short survey for the seeing tools: SAM 3's concept prompts; zero-shot 3D part segmentation by lifting 2D masks (PartSLIP++, SAMPart3D, Find3D, ALIGN-Parts); visual prompting by marking instead of cropping (the red circle, Set-of-Mark); open-vocabulary and functional 3D scene graphs (Open3DSG, OpenFunGraph, Octree-Graph); and game-video benchmarks (PhysGame, VideoGameQA-Bench). Each claim is set beside what the toolbox measured on decompiled DS and Wii scenes."
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-11T00:00:00Z
---

**`asserted`.** Written on 2026-10-11 from search summaries of abstracts, listings and
proceedings pages. No paper was read in full. Two of the claims were also tested here, on
our own data, and those results are marked **measured** with the page that holds them.
Raising this page means reading SAM 3, Find3D and Set-of-Mark in full.

The question the author asked: *what can modern computer vision, now built around language,
teach a tool that has to give meaning to decompiled game scenes?* The field moved in four
directions at once, and each one has a counterpart in [seeing what was decompiled](/design/seeing-what-was-decompiled.md).

# 1. Concepts instead of clicks: SAM 3

| work | what it does | reported |
|---|---|---|
| **SAM 3: Segment Anything with Concepts** (Meta, arXiv 2511.16719, Nov 2025) | Promptable Concept Segmentation: a short noun phrase or an image exemplar returns masks and identities for **every** matching instance, in images and video. A detector and a memory tracker share one backbone. A *presence head* decides whether the concept is in the image at all, separately from where it is. | about double the accuracy of earlier systems on concept segmentation; a 4M-concept dataset with hard negatives; the SA-Co benchmark |

**What it means here.** SAM 2 needed a point or a box per object, so the toolbox had to
know where to look. SAM 3 takes the word. It is behind Void Bicho's `segment-text` kind
since 2026-10-11.

**Measured** on a Super Mario Galaxy render (Bicho's log): "toy" found the Octoomba at
0.88, while "enemy" and "octopus" found nothing. The presence head is doing its job: it
refuses a concept it cannot see. *Enemy* is a role, not an appearance, and no picture
carries it. That is the same finding as the category test in
[seeing what was decompiled](/design/seeing-what-was-decompiled.md) §3, now from a second
model.

# 2. Parts in 3D by lifting 2D

| work | method | reported |
|---|---|---|
| **PartSLIP++** (Zhou, Gu et al., UCSD, arXiv 2312.03015) | SAM masks on multi-view renders, lifted to 3D with a modified EM instead of PartSLIP's heuristic | better than PartSLIP in low-shot semantic and instance part segmentation |
| **SAMPart3D** (Yang et al., HKU and VAST, arXiv 2411.07184) | a 3D backbone distilled from text-free 2D features, parts at several granularities, then named by a VLM from multi-view renders | PartObjaverse-Tiny, 200 shapes |
| **Find3D** (Ma, Yue, Gkioxari, ICCV 2025, arXiv 2411.13550) | a 46M-parameter point-cloud model trained on 27K objects that **SAM and Gemini annotated automatically**; per-point features queried by free text | up to 3x mIoU over the next method; 6x to 300x faster |
| **ALIGN-Parts / "Name That Part"** (Paul et al., JHU, arXiv 2512.18003) | parts as *partlets*, matched to text descriptions by bipartite assignment; uses LLM-written affordance descriptions | a third-party summary reports 58.8% LA-mIoU gain and 100x speed; unchecked |

**What it means here.** The toolbox's lift (render with an ID buffer, mask in 2D, vote onto
triangles) is the PartSLIP family's loop. Two lessons transfer:

1. **Find3D's data engine is our situation reversed.** It used 2D foundation models to label
   3D assets, then trained a small 3D model on those labels. A decompiled game already
   carries better labels than SAM and Gemini can give: joint names, archive names,
   animation names, and a community decompilation's classes. A small 3D part model trained
   on decompiled games would have *ground truth* where Find3D had pseudo-labels. That is a
   Bicho training-data story, in line with the author's *"smaller and more specialized
   models"*.
2. **Naming is its own problem.** ALIGN-Parts treats naming as set alignment, not
   per-part classification. The toolbox's CLIP step picks a word per part independently,
   which is why a thing with no word gets the nearest one (residual in seeing §5).

# 3. Mark, don't crop

| work | finding |
|---|---|
| **"What does CLIP know about a red circle?"** (Shtedritski, Rupprecht, Vedaldi, ICCV 2023, arXiv 2304.06712) | a red circle drawn on the image steers CLIP to that region **and keeps the context**; marking beats cropping |
| **Set-of-Mark prompting** (Yang et al., Microsoft, arXiv 2310.11441) | number every SAM region on the image, then ask a large VLM about the numbers; zero-shot GPT-4V beat a fine-tuned specialist on RefCOCOg |

**Measured.** This was the main lever in the toolbox. Super Mario 64 DS body parts went from
**21%** to **71%** when bare crops were replaced by red-outline crops in context
([seeing what was decompiled](/design/seeing-what-was-decompiled.md) §3). The paper
predicted the direction. Our data agrees at the size the paper reports for its own tasks.

**What it means here.** Set-of-Mark is the shape of the next step. Number every object in a
render of a decompiled zone and give one marked image to a local VLM, instead of asking CLIP
one crop at a time. It also gives the answer a stable reference: mark *7* is object id 7 in
the ID buffer.

# 4. Scenes as graphs a language model can read

| work | what it does |
|---|---|
| **Open3DSG** (Koch et al., CVPR 2024) | open-vocabulary object classes and open-set relationships from point clouds, by aligning a 3D backbone with 2D VLM features; no labelled scene graphs needed |
| **OpenFunGraph** (Zhang et al., CVPR 2025, arXiv 2503.19199) | **functional** 3D scene graphs: objects, the interactive elements on them, and what each does; VLMs and LLMs supply the functional knowledge; beats adapted Open3DSG and ConceptGraphs |
| **Octree-Graph** (Wang et al., ICCV 2025, arXiv 2411.16253) | each node an adaptive octree carrying occupancy, edges for spatial relations; argues point clouds are costly and poor at occupancy |

The earlier half of this line is on [ConceptGraphs and SceneGPT](/sources/conceptgraphs-scenegpt.md).

**What it means here.** A decompiled level *is* a functional scene graph before any vision
runs. Super Mario Galaxy's placement tables give every object, its zone, its path and its
links, and the gravity tables give a relation the papers have to infer: *this field holds
that planet*. The field is trying to recover from pixels what a decompilation reads from
bytes. Vision's job in Void Squid is narrower and still essential: name the things the
tables do not, and check the decompiler's guesses. OpenFunGraph's split (object, the
interactive part on it, its function) maps onto Petari's split (archive, joint, class).

# 5. Games as a test bed for vision-language models

| work | what it tests |
|---|---|
| **PhysGame** (arXiv 2412.01800; TMLR follow-up 2601.16471) | 880 gameplay videos with physics-violating glitches; open video LLMs trail proprietary ones; glitches used as training signal |
| **VideoGameQA-Bench** (arXiv 2505.15952, NeurIPS 2025 D&B) | glitch detection, visual regression and bug reports on game images and video |
| **RefGlitch-Bench** (arXiv 2604.11082, 2026) | whether a reference image helps a VLM find glitches |

**What it means here.** These benchmarks judge a game from its video. Void Squid holds the
game's own data, so it can judge a rendering against the data that produced it. The
residual is then exact: a coin rendered black is a converter fault (an environment-mapped
material), not a vision fault. That is an oracle these benchmarks do not have.

# 6. Lessons for Void Bicho and Void Jelly

1. **Prompt with words when you have them, with marks when you do not.** SAM 3 for
   "find every tree", Set-of-Mark for "what is mark 7".
2. **Vision answers *who*; the decompilation answers *what it does*.** Two models
   (CLIP, SAM 3) failed the same way on *enemy*. Role needs code knowledge or a text model
   between identity and role.
3. **Distil, don't call.** Find3D shows a small 3D model can be trained from big 2D
   models' labels. Decompiled games supply better labels, so Bicho's small specialised
   models have a credible path.
4. **Keep the oracle.** Every paper above reports against human labels. The toolbox reports
   against the game's own skeleton, collision and decompilation, and should keep doing so.

# Sources

- [SAM 3: Segment Anything with Concepts](https://arxiv.org/abs/2511.16719v1)
- [PartSLIP++](https://arxiv.org/abs/2312.03015v1)
- [SAMPart3D](https://arxiv.org/abs/2411.07184v1)
- [Find Any Part in 3D](https://arxiv.org/abs/2411.13550v2)
- [Name That Part (ALIGN-Parts)](https://arxiv.org/abs/2512.18003v2)
- [What does CLIP know about a red circle?](https://arxiv.org/abs/2304.06712v2)
- [Set-of-Mark prompting](https://arxiv.org/abs/2310.11441v2)
- [Open3DSG, CVPR 2024](https://openaccess.thecvf.com/content/CVPR2024/papers/Koch_Open3DSG_Open-Vocabulary_3D_Scene_Graphs_from_Point_Clouds_with_Queryable_CVPR_2024_paper.pdf)
- [Open-Vocabulary Functional 3D Scene Graphs, CVPR 2025](https://openaccess.thecvf.com/content/CVPR2025/html/Zhang_Open-Vocabulary_Functional_3D_Scene_Graphs_for_Real-World_Indoor_Spaces_CVPR_2025_paper.html)
- [Open-Vocabulary Octree-Graph, ICCV 2025](https://openaccess.thecvf.com/content/ICCV2025/html/Wang_Open-Vocabulary_Octree-Graph_for_3D_Scene_Understanding_ICCV_2025_paper.html)
- [PhysGame](https://arxiv.org/abs/2412.01800v1), [PhysGame follow-up](https://arxiv.org/abs/2601.16471v1), [RefGlitch-Bench](https://arxiv.org/pdf/2604.11082)
