---
type: Design
title: The cast
description: Every playable character of the four games, decoded with its skeleton, skin weights and animations into one rig format, and switchable at play time: 25 characters (Galaxy's Mario and Luigi, Super Mario 64 DS's four, Mario Kart DS's thirteen racers, Donkey Kong 64's five Kongs, a Goomba), played through Void Onda and judged from their pixels.
resource: ../../../ds-mashup/toolbox/build_cast.py
tags: [status:current, audience:dev, confidence:verified]
timestamp: 2026-10-13T00:00:00Z
---

# The cast

The author, 2026-10-12: *"why cant i play as donkey kong, mario from galaxy, all the playable
characters in mario 64 ds, or any of the racers from the mario kart ds?"* Now one can, and, since
the evening of 2026-10-13, as Donkey Kong 64's five Kongs too (their own models, Void Onda's motion). The code lives in the DS-mashup exercise, like the rest of the
pipeline that reads game files; nothing from a game enters this repository, and the rigs live in
the person's workbench (`goomba_world/rigs/`, 8.7 MB for twenty).

## What was decoded

| game | characters | from | animations |
|---|---|---|---|
| Super Mario Galaxy | Mario, Luigi (and the Goomba, as before) | `Mario.arc`, `Luigi.arc`: J3D `.bmd` with EVP1 envelopes (true skin weights) | `MarioAnime.arc`: 11 states (idle, walk, run, jump, spin, fall, land and more), Luigi sharing Mario's skeleton |
| Super Mario 64 DS | Mario, Luigi, Wario, Yoshi | NSBMD-era `.bmd` per character, with the head as a separate model attached to the head bone | `.bca` per character, 7 states; Mario's as the fallback where bone names agree |
| Mario Kart DS | all 13 racers: Mario, Luigi, Peach, Yoshi, Toad, Donkey Kong, Wario, Bowser, Daisy, Waluigi, Shy Guy, R.O.B., Dry Bones | `P_XX.nsbmd`, converted by apicula to skinned glTF | drive, lose, spin, win and more, 9 states; each in their standard kart |

## One format

**Rig v2, "skin"** (`tools/rigfile.py`): `rig.json` with joints (parent, bind transform),
inverse bind matrices, material groups (texture, alpha, colour, a range of vertices) and
animations (per joint: scale, rotation as Euler or quaternion, translation, per frame, at an
fps), plus `skin.bin`, 19 floats a vertex (position, normal, uv, colour, four joints, four
weights). Three readers write it: `bmd_rig.py` (SM64DS: BCA's nine channels per bone, s32/4096
scale and translation, s16 rotation with 4096 a turn, bone = T·Rz·Ry·Rx·S), the J3D reader in
`toolbox/j3d.py` (skin mode), and `skin2rig.py` (any skinned glTF, sampled with slerp).

The runtime skins on the CPU: v = Σ w · (pose[j] · ibm[j]) · v_bind, uploaded per frame. A
character's states map game-neutral names (idle, walk, run, jump, fall) to its own animations,
and its height is scaled to a size in metres from the game, so a Mario Kart racer and a Galaxy
Mario stand at sensible heights side by side.

## Switching

**Q** (or pressing the left stick) cycles the cast while playing; the Feast's **Play as** grid
picks one before starting, by game. `--autotest cast` cycles every character and checks each one
draws and animates.

## What was found

- **Rebuilt normals pointed inward** on Galaxy's skins (the file's normals are per face on some
  groups). Now smoothed by position and turned outward by a vote against the centroid, only
  where a group's normals are visibly flat.
- **Galaxy's tint is in the material**, not the texture: carried as the group colour.
- **Yoshi's head is a separate model at a different scale shift**: the root translation is
  rescaled by the ratio of the two shifts, and the tongue (always extended at bind) is dropped.
- **A cast run with one game overwrote the others.** Always pass every source.
- **A turkey that becomes a character keeps its kind** (the Goomba stays an enemy).

## The evening of 2026-10-13: what the author saw, and what it was

The author: Mario Kart DS's models' *"triangles don't really match up"*; Mario's animations *"don't
really fully load"*; Super Mario 64 DS's *"timings are off"*; and the Kongs were missing. Each had a
cause that could be measured:

| what was seen | cause | fix |
|---|---|---|
| a face drawn twice, "two heads" | the DS mirrors half-textures (a face from half an image) and the formats say so per texture (glTF samplers; the DS's TEXIMAGE_PARAM; GX's wrap); nothing on our side honoured it, so the half repeated | wrap modes carried through every converter, the rig format, the runtime (rigs and every glTF it loads) and Void Jelly to the studio's views |
| Super Mario 64 DS's one stretched eye | the eye material scales its texture coordinates by 2 and mirrors: two eyes from one eye texture | `bmd2gltf.py` applies the material's texture matrix |
| Galaxy's rotations frozen | the BCK reader scaled rotation keys' *times* by the angle factor, so every rotation sat at its last key | times kept as frame numbers |
| a jump looping in the air; clips snapping between frames and states | every Galaxy clip forced to loop; no interpolation, no blending | [Void Onda](../../../VoidOnda/okf/index.md): loop modes from the files, interpolation, crossfades |
| timings off; feet sliding | the wrong clips (the fall was `jmped`, a landing) and one fixed playback rate | Onda's dictionaries, watched; rate from each cycle's measured ground speed |
| racers popping at a seam | Mario Kart DS's `drive` is a steering lean, not a loop | played from the kart's turning (`param:steer`) |
| no Kongs | Donkey Kong 64's actor format undocumented in dk64_lib | `tools/dk64_actor.py`, written from the DK64 Randomizer's notes (MIT, read): bones, rigid binding, F3DEX2, textures from table 25; items held in hand left out |

Every fix was checked by the judge (`--autotest judge`, [Void Onda's judging motion](../../../VoidOnda/okf/design/judging-motion.md)):
105 clips, each rendered unlit and checked frame by frame, including pixels predicted by ray casting
the runtime's own mesh. With wraps switched off on purpose, the racers fall to 55–88%; with them,
every clip passes at 97% or above.

## Residual

- **Donkey Kong 64's animations are not decoded** (table 11). The Kongs move by Void Onda's
  synthesized gait, tagged as made; the gallery (**G**) tunes it.
- **Void Jelly has no armatures**, so the studio's 3D views show a character in bind pose or
  not at all; animation is only in the runtime. Asked of Jelly: does `.ink` grow skins (S32)?
- **Only some states per character**: swimming, climbing and item animations are not mapped.
- **Feet slide at the runtime's top speed.** Galaxy's speeds were an eye joint read as a foot (fixed
  2026-10-14); the runtime now plays the cycle whose pace is nearest the speed, and Void Onda's pace
  check measures the rest: at 9.5 m/s the games' runs would need more than 5x their speed.
- **Some thumbnails render black** (Galaxy's `StarPiece`): the toolbox's renderer and that material.
