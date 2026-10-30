---
type: Design
title: Seeing what was decompiled — vision models, an ID buffer, and the decompilers' own knowledge
description: "2026-10-10: the author found the DS mashup a jumble of triangles without meaning and asked for visual and semantic tools, with Void Bicho supplying the models. Built: a headless renderer with an ID buffer; Bicho's first runtime (SAM 2.1 Large for masks, CLIP for labels) behind its invocation contract; a lift from 2D masks and labels onto 3D triangles; evaluations against ground truth the decompilation itself supplies. Then Super Mario Galaxy, with the community decompilation Petari as a human-made dictionary of meaning. What was measured, and what it says about where meaning comes from."
resource: ../ds-mashup/toolbox/lift.py
tags: [status:current, audience:dev, audience:all, confidence:verified, foundation]
timestamp: 2026-10-10T00:00:00Z
---

> The author: *"we need a lot of tools to give MEANING to what you decompile … the finished
> product was a jumbled mess of triangles and textures … our top priority should be in
> getting visual information from the decompilation process."* And: *"void jelly is gonna
> need computer vision stuff. not within itself, but utilizing a library heavily (library
> provided by void bicho)."* Then: try Super Mario Galaxy, *"since they had humans the whole
> time, the decompilation they already did is extremely valuable."*
>
> **Built, and measured** (§3). The code lives where its rules allow. Models are in Void
> Bicho (`../VoidBicho/python/bicho/`); the renderer, the lift and the evaluators are in the
> toolbox (`../ds-mashup/toolbox/`), because they loop over vertices. No game data is in any
> repository.

# 1. The loop

```
 a decompiled scene ──render (headless, ID buffer)──▶ views + "which triangle drew this pixel"
        ▲                                                  │
        │                                     Void Bicho: segment-2d (SAM 2.1 Large)
        │                                                  │ masks: WHERE things are
   lift: votes per triangle ◀──────────── Void Bicho: label (CLIP ViT-B/16) per mask
        │                                                    WHAT they look like
        ▼
 semantics.json: per triangle a label, its share of the votes and its evidence;
 segments (connected triangles, one label); per part, vision's answer next to the name's
```

Each stage keeps a family rule. **Rendering is pure**: a scene and a camera give pixels and
IDs, with no window, so this is "simulate visually without a window", the author's ask from
the simulation side. **Models are effects** behind Bicho's contract. Every reply says which
model ran (resolved from a tag query), the parameters as applied, the cost, whether it
finished, and `deterministic: false`. That flag is true to fact: the same image gave 68 masks
on the CPU and 72 on the GPU. **Lifting is pure** again. **Every label is a proposal**:
`semantics.json` keeps the votes and the model ids, so nothing a model said becomes truth
until something accepts it (S13).

The **ID buffer** is the bridge. It is smart logs' layer 2 ([smart logs](/design/smart-logs.md)
§3), which was planned and is now built. It lets a 2D model's answer about pixels land on
3D triangles, so segmentation needs no 3D model at all: **3D segmentation by lifting
many 2D views**.

# 2. Two sources of ground truth, neither made by hand here

| oracle | where it comes from | what it labels |
|---|---|---|
| **a skeleton** | Super Mario 64 DS's Mario: the bone that owns each triangle (BMD) | head, torso, arm, hand, leg, shoe |
| **collision** | Mario Kart DS's KCL under each render triangle | road, off-road, wall |
| **a decompilation** | Super Mario Galaxy's level tables say which named object drew each triangle; **Petari** (SMGCommunity) says what each name is: its class, and the folder humans filed it in | Map, Enemy, MapObj, NPC, Ride … (1,183 names) |

The third is the one the author pointed at. Petari's source tree is itself a recovered
ontology of the game: `Enemy`, `MapObj`, `NPC`, `Player`, `Boss`, `Ride`, `Gravity`,
`Camera`, `AreaObj`. Its factory table maps every placeable name to a class and a model
archive. The toolbox reads names and folders only, never builds or copies the
decompilation, and respects that project's rule against AI-written decompiled code.

# 3. What was measured

| experiment | result | baseline | what it says |
|---|---|---|---|
| DS Mario, body parts, first try (masks greyed out of context) | **21%** of triangles right | 46% (always "head") | SAM 2 found the parts cleanly (cap, nose, ear, gloves, overalls, shoes); CLIP could not name a part shown alone |
| DS Mario, red-outline context crops + character prompts + specific masks weighing more | **71%** | 46% | naming needs context; the red outline matters (Shtedritski et al., ICCV 2023) |
| the same, but with isolated crops | 29% | 46% | the context crop is nearly the whole gain |
| Galaxy characters and items, identity from their own models (22, among 27 names) | **14/22 right, 17/22 in the top three** | 1 in 27 by chance | vision recognises *who*: Mario, Luigi, Peach, Rosalina, Bowser, Bowser Jr., Goomba, Piranha Plant, Bob-omb, Cheep Cheep; its errors look like a person's (Luma vs Boo, Toad vs 1-Up Mushroom) |
| Galaxy, Good Egg's opening zone (51 objects, 26,769 triangles), each triangle against Petari's category | 37% of labelled triangles | 40% (always "Map") | vision does not recover the decompilers' categories |
| … per object | planets 2/3, map objects 18/29, **enemies 1/17**, NPCs 0/2 | | |

**The finding that matters most: vision and the decompilation answer different questions.**
Petari's categories are functional. *Enemy* means "its code hurts Mario", which no picture
shows: a Piranha Plant looks like a plant, and the spiky `NeedlePlant` that looks dangerous
is filed as a map object. Vision is good at *what something looks like* and *who it is*,
and poor at *what it does*. Meaning needs both arrows. Vision supplies identity and parts.
The decompilation and the data supply role and behaviour: a Goomba archive ships animations
named `walk`, `run`, `flatdown` (stomped), `hipdropdown` and `swoon`. Knowledge about the
identity joins them: a Goomba is an enemy in every Mario game. That last link is a
text-model job, and Needle (`VoidBicho:/design/needle.md`) is the candidate the author named.

**The second: higher-fidelity assets help, but are not the main lever.** The low-poly DS
Mario went from 21% to 71% by changing how the question was put to CLIP, not by changing the
model. Galaxy's identity results show that once a model is detailed enough to be a
recognisable character, an off-the-shelf model knows the franchise.

# 4. Super Mario Galaxy, taken apart (what exists now)

| piece | how |
|---|---|
| the disc (RVZ, RMGP01) | `nodtool` (the decomp-toolkit author's disc library), invoked as a toolchain |
| Yaz0, RARC, BCSV/JMap placement tables | `toolbox/nintendo_gc.py`, written here; field names resolved by their JMap hashes |
| J3D models (BDL/BMD) with joints, envelopes, GX lists, materials, eleven texture formats | `toolbox/j3d.py`, written here; a Goomba, Mario and a zone of 51 objects render correctly |
| a zone assembled from its own tables, with per-triangle object identity | `toolbox/smg_scene.py` (Good Egg Galaxy's SoramameZone, DinoPackunBattleZone, DorayakiFortressZone, PeanutZone) |
| what every name is | `toolbox/petari_index.py` over Petari's `NameObjFactory.cpp`: 1,183 names → class, model archive, category |

# 5. Residual

- **J3D:** materials with GPU-generated texture coordinates (environment maps) render wrong.
  The coin comes out near black, so the identity test's coin miss is the converter's fault,
  not vision's. Only each material's first texture is used, and no animations (BCK, BTP,
  BRK) are converted yet, though their names are read.
- **Placement:** the rotation order is a guess that looks right on four zones. Only scenario
  layers `common` and `layera` are used.
- **CLIP** chooses only among the words it is given. A thing with no word gets the nearest
  one. The vocabularies were written by hand here.
- **SAM 3D** was not attempted. SAM 2.1 Large (898 MB) runs on this laptop's 6 GB GPU in about
  6 seconds per view; SAM 3D Objects needs 32 GB.
- Mario Party's characters and the DS boards were not re-run through the loop.

# 6. What Void Squid needs next

1. **Join the three sources of meaning in one document.** For each piece: vision's identity
   and parts (proposals), the decompilation's class and category (Petari-like knowledge,
   where a community has it), and the data's behaviour vocabulary (animation names, table
   fields). That is the shelf (S22) gaining a *meaning* column.
2. **A text model for the gap between identity and role**: Needle, or a larger local model
   behind the same Bicho contract, answering "is a Goomba an enemy?" with a confidence.
3. **The renderer and the lift belong to Void Jelly** once `.ink` carries UVs and textures.
   Jelly has no texture today, so the toolbox holds them (message to Jelly).
4. **Animations next**, now that their names are known to carry meaning: J3D BCK and Super
   Mario 64 DS's BCA.
5. **Set-of-Mark next, then a small 3D part model.** Number every object in a render and ask
   one local vision-language model about the marks; later, train a small part model on
   decompiled games, whose labels are ground truth
   ([vision in the age of language models](/sources/vision-in-the-age-of-language-models.md) §2, §3).
   Since 2026-10-11 Bicho also has SAM 3 (`segment-text`) and CLAP (`label-audio`), used in
   [Goomba Galaxy](/design/goomba-galaxy-exercise.md).
6. **The evaluators stay.** Every change to the loop is scored against a skeleton, a
   collision mesh or a decompilation, never against impressions.
