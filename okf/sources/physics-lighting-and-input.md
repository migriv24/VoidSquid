---
type: Source
title: Physics, lighting and input for decompiled worlds — Jolt, the game's own lights, Unity's actions
description: "A small research round for Goomba Galaxy 2 (2026-10-11): which physics engine can run every decompiled world (Jolt, against PhysX, Bullet and Rapier); what lighting a decompiled game already carries (Super Mario Galaxy's LightData) and what the field adds on top (hemisphere, rim, shadow maps, drop shadows, linear-space shading); how Unity's Input System separates actions from devices; and what Super Mario Galaxy's own cameras are. Each claim says whether it was verified here."
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-11T00:00:00Z
---

**Mostly `asserted`.** Written from web search summaries on 2026-10-11. Five claims were
**verified here** by building or decoding, and are marked so. The rest are the
verification queue: read Jolt's architecture documentation, the raylib shadow-map example
and Unity's Input System manual in full.

The author asked for a small round on physics and lighting before a new version of the
level ([Goomba Galaxy 2](/design/goomba-galaxy-2-and-the-turkey-table.md)), and for
controller support like Unity's.

# 1. Physics: one engine for every world

| engine | what the round found | fit here |
|---|---|---|
| **Jolt Physics** (Rouwe; MIT; C++17) | described as faster than Bullet in almost all cases and close to PhysX, multithreaded, used in *Horizon Forbidden West* (forum and secondary sources); MSYS2 packages it for MinGW | **chosen**. **Verified here:** v5.6.0 builds with this machine's MinGW GCC 15, and its `CharacterVirtual` takes a new "up" every step |
| **PhysX** (NVIDIA; open source) | mature; Cocos warns of about 5 MB of package size | heavier than needed |
| **Bullet** (zlib) | older, widely supported | superseded for new work |
| **Rapier** (Rust) | reported faster than Bullet; tested with Bevy and Godot | **worth knowing for the Bevy host** (Void Squid's planned runtime) |
| ReactPhysics3D | not found in this round | — |

**The lesson that matters.** Engines have one global gravity. Super Mario Galaxy has
fields. Jolt's own planet sample (`Samples/Tests/Character/CharacterPlanetTest.cpp`, **read
here**) solves it the way this exercise does: gravity factor zero, a force per body each
step towards the planet, and a character whose up is set every step. So **gravity fields
stay in the game model as data**, and the engine stays generic. One engine can then run
every decompiled world, as long as each world brings its own collision.

**Verified here:** the same Jolt code runs Good Egg Galaxy (106 fields, 71 collision
meshes), Super Mario 64 DS's castle grounds (one plane field, 1,243 decoded collision
triangles), and a table built from three games.

# 2. Lighting: what the game carries, and what goes on top

**Recovered (verified here).** Super Mario Galaxy keeps its lights in
`ObjectData/LightData.arc`. One table names each light setting; per-zone tables map light
ids to settings; `LightCtrlCube` areas pick a setting by place. Each setting holds two
directional lights and an ambient colour for each of four audiences: Player, Strong, Weak
and Planet (the map). Field names come from Petari's `LightDataHolder` and
`LightFunction`. Good Egg uses 13 settings and 33 light areas.

**What the field says Galaxy does on top** (forum accounts, **asserted**): shading set per
model rather than one lighting model, environment maps, and a hardware rim light. None of
this is the game's documented pipeline. The material data that would settle it (J3D
`MAT3` channel and TEV settings) is not decoded here.

**Techniques found, with what they cost:**

| technique | source | used |
|---|---|---|
| hemisphere ambient (sky and ground colours) | Hargreaves; three.js docs: very low cost, no shadows | the recovered ambient stands in |
| rim (Fresnel) light | gamedev.net shader notes; Galaxy discussions | yes, weak, in linear space |
| directional shadow map, orthographic | raylib 5.5 `shaders_shadowmap` example (listed, not read); MonoGame notes | yes, around the player |
| blob or drop shadow | the common stylised alternative to shadow maps | yes: Galaxy's drop shadow, cast along gravity |
| filmic tone curve | ACES fit (Narkowicz) | yes |

**Measured here, and the most useful findings of the round:**

1. **Linear space is not optional.** Lighting sRGB texture values directly washed every
   scene out. Converting textures, vertex colours and the game's light colours to linear,
   lighting, then encoding gave back the textures' own saturation.
2. **Decoded normals can be placeholders.** Every Galaxy model this toolbox converts
   carries the same normal (`toolbox/j3d.py` does not decode `VTX1` normals). The game
   lights maps by vertex colour, so nothing in the game shows it. Any lighting written on
   top must know where its normals came from. The renderer now rebuilds them from faces,
   welded by position and oriented outwards, and that is a residual.
3. **A debug view per term** (lit, textures only, light only, normals) found both faults in
   minutes. The look test (`goomba_galaxy2 --autotest look`) keeps it.

# 3. Input: actions, not keys

Unity's Input System (manual, versions 1.0 to 1.8, **asserted**): an action has any number
of **bindings**, each a control path. **Composites** build a value from several controls
(the 2D vector from WASD). **Processors** transform values; a deadzone on both a binding
and its control applies twice, as the manual warns. **Interactions** decide when an action
fires. **Control schemes** group bindings by device. Bindings have ids so user
**overrides** can be saved.

GLFW, which raylib uses, ships SDL's GameControllerDB and maps known pads to an Xbox-like
layout (GLFW forum and SDL README, **asserted**). That is why one set of gamepad bindings
can serve many controllers.

**Built here** (`ds-mashup/src/g2/input.cpp`): action maps (Player, Camera, UI), control
paths, 2D composites, processors (deadzone, scale, invert, normalize, per-second), schemes
that follow the last device used, prompts that name the right button, a script device for
autotests, and a bindings file written on first run for rebinding. **Not verified with a
physical controller:** no pad was connected in this session.

# 4. Cameras: the game's own

Super Mario Galaxy's cameras are data too (**verified here**). Each zone has
`camera/cameraparam.bcam`, a table of 416 cameras across Good Egg's zones. Its field names
(`camtype`, `angleA`, `angleB`, `dist`, `wpoint`, `axis`, `camint` and others) come from
Petari's `CameraParamChunk`. `CubeCameraBox`, `CubeCameraCylinder` and `CubeCameraSphere`
areas name a camera by id. All 76 area ids resolve. Types in Good Egg: parallel 224,
planet 31, stay-put 31, charmed 29, fixed eye 28, object-parallel 24, follow 20, tower 11,
and others. Petari's `CameraHolder` maps each type to a camera class.

# Sources

- [Jolt Physics build README](https://raw.githubusercontent.com/jrouwe/JoltPhysics/master/Build/README.md); [MSYS2 package](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-jolt-physics); [jMonkeyEngine forum on Jolt, Bullet and Rapier](https://hub.jmonkeyengine.org/t/new-physics-engines-replacing-bullet-engine-for-game-dev/47330); [Cocos physics choices](https://docs.cocos.com/creator/3.2/manual/en/physics/physics-item.html)
- [Catlike Coding: custom gravity](https://catlikecoding.com/unity/tutorials/movement/custom-gravity/)
- [Hargreaves: hemisphere lighting](https://shawnhargreaves.com/radiosity_lighting.pdf); [three.js lighting notes](https://smithery.ai/skills/cloudai-x/threejs-lighting); [raylib and shadows](https://noino.substack.com/p/raylib-graphics-shading); [gamedev.net: Fresnel and rim lighting](https://gamedev.net/shaderlab/61-fresnel-and-rim-lighting/); [Polycount on Galaxy's shading](https://polycount.com/discussion/comment/997495/)
- [Unity Input System: bindings](https://docs.unity3d.com/Packages/com.unity.inputsystem@1.8/manual/ActionBindings.html); [GLFW and SDL_GameControllerDB](https://discourse.glfw.org/t/gamepad-mappings-and-sdl-gamecontrollerdb/1621)
