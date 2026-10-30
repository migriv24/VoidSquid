---
type: Concept
title: Hosts
description: The third pillar — Void Squid runs on a Windows desktop first and on Latin OS later, and the only thing that differs between the two is the effect list. What each host performs, what the LITO host changes (capabilities instead of ambient authority; surfaces instead of pixels; a hypervisor instead of an emulator), and the recursion the author named.
tags: [status:planned, audience:dev, confidence:exploratory]
timestamp: 2026-10-09T00:00:00Z
---

> **Nothing is built.** The desktop host is v0's build surface and waits for Void Jelly's
> rung 2.5 as before. The LITO host waits for LITO to boot
> (`LatinOS:/design/kernel.md`), which nothing here can move. This page exists so that the
> desktop host is written as *one host of two* from its first line.

# 1. The rule

A host is **the effect list, implemented for one platform**, and nothing else. The list
is [build surface](/concepts/build-surface.md): file read and write, process, window and
GL, emulator, cache, clock and progress, user choice. [Construct and deconstruct](/concepts/construct-and-deconstruct.md)
adds **ingress toolchains**; [studio surfaces](/concepts/studio-surfaces.md) adds **a
hypervisor** for the game view when the artifact is an operating system.

The mechanical test for whether something is a host's is unchanged: *could it run in a
unit test with no filesystem, network, display or clock?* No → here. And the test for the
whole pillar is: **two hosts, one transcript, one document.** The same dispatcher
transcript replayed on Windows and on LITO must produce the same document, byte for
byte, with only the effects' *implementations* differing.

# 2. The two hosts, side by side

| effect | Windows desktop (first) | LITO, Latin OS (later) |
|---|---|---|
| **file read / write** | the filesystem | there is no file system; a **capability** to a region of the capability graph, handed to Squid at launch, with no ambient authority (`LatinOS:/design/kernel.md` §3) |
| **process** (devkitARM, `cargo`, REA) | spawn, with discovery on `PATH` | a holiday to a service that holds the toolchain; the toolchain may run on another device in the mesh |
| **window / GL** | create the window, own the context, present | **no window and no pixels.** The studio's surfaces are runes in the compositor's scene; the compositor places and draws them (`LatinOS:/design/mesh-compositor.md`) |
| **input** | the window's events | capabilities to input, per seat |
| **emulator** (a DS game) | launch melonDS or similar on the `.nds` | the same, as a service |
| **hypervisor** (an OS artifact) | a desktop hypervisor, with Squid as the serial console and framebuffer viewer | LITO running LITO: a nested instance under the host's own capability regime; the recursion in §4 |
| **cache** | a directory | a Palabra store the host already has |
| **clock / progress** | the system clock; a progress bar | the same, through the host's sinks |
| **user choice** | dialogs, the command line | the same gestures, compiled to the same commands |

Read down the LITO column: nothing new is *computed*. Every row is an effect the desktop
host also performs, reached through a capability instead of a system call and placed as
a surface instead of drawn as pixels.

# 3. What the LITO host changes about Squid's own shape

Three things, each already true of a Void Maiz application and each made load-bearing:

1. **No ambient authority.** On LITO a program starts with nothing and holds only what it
   was handed. Squid's *effects refused by default, granted per run* discipline (inherited
   from Maiz headless) is the same model, so the LITO host does not add a permission
   system; it maps the existing grants onto capabilities.
2. **No pixels.** Every studio surface is `project : document → surface`
   ([studio surfaces](/concepts/studio-surfaces.md) §4). The desktop host turns that
   into ImGui draw lists; the LITO host hands it to the compositor as a rune. If any
   surface in pillar 2 can only be drawn, it cannot run on LITO, and that is the test
   the desktop host should be held to from the start.
3. **The build surface is a mesh, not a machine.** devkitARM may be installed on one
   device and REA on another; LITO's mesh computation
   (`LatinOS:/design/mesh-computation.md`) is what lets the forge's effects run where the
   tools are. Squid's "which nib" is already data rather than a mode; "which device"
   becomes the same kind of data.

# 4. The recursion, stated plainly

Latin OS runs Void Squid. Void Squid exports Latin OS. The author: *"recursion at its
core."* In the pillars' terms it is ordinary:

- the **studio** (pillar 2) opens the OS document as a scene;
- the **hosts** (pillar 3) perform the emission's effects and run the result under a
  hypervisor as the game view;
- the **game model** (pillar 1) describes the OS in the same nouns as a game
  ([what a game is](/concepts/what-a-game-is.md) §8);
- and **the artifact never ships the forge** ([railguard](/design/what-voidsquid-is-not.md)
  §6 as restated 2026-10-09): the exported OS carries Void Core, because it is a
  rewriter, and carries no studio, no viewport and no build surface.

What makes it feel vertiginous is only that forge and artifact share a vocabulary. That
is the family's point, not a problem.

# 5. Which host is built when

The **desktop host is v0** and is the only host for as long as v0 lasts; Windows because
it is the author's machine, with the toolchain-discovery problem the build surface
already names. A **headless host** (`maiz::run_cli`) arrives with it for ~40 lines and is
arguably the more important one, since CI and agents use it. The **LITO host** is built
when LITO can run a Void Maiz application at all, which is Latin OS's roadmap and Maiz's
Q23 seam, and it is listed in the [roadmap](/roadmap.md) as the rung with no date.
