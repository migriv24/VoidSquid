---
type: Source
title: Translating between games — learned projection, and structural analogy
description: "The author, 2026-10-12: the more games are added, the more abstract the mathematics of translating one game's systems into another; many people call their approach projection. A short search: learned shared representations (Atari transfer, world models) and structural analogy (Hinrichs and Forbus's structure mapping). Where Void Core's runes, tags, links and Allomone stand between them."
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-12T00:00:00Z
---

**`asserted`.** From search summaries, 2026-10-12. No paper was read in full. The search
did not find one established framework for projecting mechanics across arbitrary games.

# Two families

| family | how | examples found | limits reported |
|---|---|---|---|
| **learned projection** | train an encoder so different games land in one space; reuse what was learned there | transfer between Atari games (a shared model reached a reward about 50 times faster); frozen "processor" networks that transfer better in about 65% of game pairs; a Minecraft world-action model transferring zero-shot to VoxeLibre on basic tasks; latent-action world models | conflates representation and policy (the ALE survey); negative transfer when games differ; latent actions are "localised" without a common embodiment (ICML 2026) |
| **structural analogy** | find a mapping between the parts and relations of two games, then carry knowledge along it | Hinrichs and Forbus: transfer is *"fundamentally a problem of finding a good analogy"*, by structure mapping | needs the structure of both games stated |

# What it means here

Void Squid has the structure the second family needs, because decompilation reads it out of
the game rather than guessing it from pixels. A Galaxy gravity field, a Donkey Kong 64
"trees" folder and a Mario Kart DS item box are already runes with tags and links. So
**translation here is structure mapping first**, with learned models (Void Bicho) proposing
the mappings the structure cannot settle, and every mapping a proposal with a residual.

The gravity case shows the shape. Galaxy's field model (point, disk, box, segment, plane;
range; priority) contains the other games' gravity as one case: a constant downward pull is
a plane field with no range. A simpler game's system projects into the richer one with
nothing lost. The hard cases come when a game brings a system no other game has, such as
Minecraft's redstone. Then the shared vocabulary must grow, or the system stays that game's
own, with a residual saying so. That is the decision S27 asks about.

# Sources

- [GameWAM: a world action model for video games](https://arxiv.org/pdf/2608.26200)
- [Revisiting the Arcade Learning Environment](https://arxiv.org/pdf/1709.06009)
- [Reasoning-Modulated Representations](https://arxiv.org/pdf/2107.08881)
- [Hinrichs and Forbus: transfer learning through analogy in games](https://qrg.northwestern.edu/papers/Files/QRG_Dist_Files/QRG_2011/AIM_2011_TransferLearningThroughAnalogyInGames.pdf)
- [Entity embedding as game representation](https://arxiv.org/pdf/2010.01685)
- [Learning latent action world models in the wild (ICML 2026)](https://icml.cc/virtual/2026/poster/65056)
