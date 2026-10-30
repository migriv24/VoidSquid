---
type: Concept
title: The Forge — a device library and the cross-device match loop
description: "The author's 2026-10-28 direction: decompilation is not game-to-PC, it is game-to-any-device. A device library of per-device plugins — each a CPU, a runtime and its subsystems — a route that plans the bridges between a source device and a target device, and a match loop that decides, per function, whether a candidate translation is accepted against an oracle. Built: the pure library and `effect forge devices|route|match`. The verifier that compiles-and-runs is still a synthetic stand-in, and the report says so."
tags: [status:current, audience:dev, confidence:asserted, foundation]
timestamp: 2026-10-28T00:00:00Z
---

# The Forge

> The author, 2026-10-28: *"theoretically, the equation isn't 'how to get original game
> logic into PC' but rather, 'how to get original game logic into ANY OTHER device' … we
> want to create a more generalizable process … a library or plugin per device we are
> actually matching a game file to … any given user may not want ALL of the plugins."*

## 1. The arrow, generalized

[Decompilation](/design/decompilation.md) is written as "game → source". The forge writes
the same arrow with **both ends named**: a game runs on a **source device** (a Wii, an N64,
an operating system); a port re-expresses it for a **target device** (a PC, Android, another
console). "SMG → PC" is then one row in a table whose columns are devices, not a special case.

## 2. The device library

A **device plugin** is a manifest describing one device:

- `id`, `name` — how it is addressed and displayed;
- `cpu` — the instruction set architecture (the code must cross this boundary somehow);
- `kind` — `source`, `target`, or `both`;
- `toolchain` — how to build for it (a command to invoke, never a dependency);
- `runtime` — how to run and verify on it (native, or an emulator used as an **oracle**);
- `subsystems` — the canonical list (`cpu`, `gpu`, `audio`, `input`, `storage`, `os`, …),
  each with this device's implementation.

The library is a plain JSON document, `forge.device-library/0.1`
([`forge/fixtures/device-library.json`](../forge/fixtures/device-library.json)). Because each
device is one entry, a user installs exactly the devices their games need — the storage
concern the author named — and nothing stops the library growing to many consoles.

**Code:** [`src/forge/device.cpp`](../src/forge/device.cpp), schema and validation;
[`src/forge/forge.hpp`](../src/forge/forge.hpp) for the shape.

## 3. The route

`plan_route(source, target)` answers *"what must be bridged to move this game?"*. The cpu is
always bridged when the ISAs differ (recompiled or binary-translated); then every source
subsystem is either **remapped** onto the target's implementation or marked
**`reimplement`** when the target has none.

```
wii → pc-x86_64-windows
  cpu:    powerpc-broadway        → x86-64
  gpu:    GX FIFO                 → OpenGL / Vulkan
  audio:  AX + DSP                → WASAPI / XAudio2
  input:  EXI/SI (Wiimote…)       → keyboard / mouse / gamepad
  storage:DVD / FST               → filesystem
  os:     Starlet / IOS           → Windows
```

This is the platform layer made data: the same list this bundle has always called the
hardest stage of a port ([decompilation §6](/design/decompilation.md)), named per pair.

## 4. The match loop

For one function, candidates are proposed (by a person, a rule, or Void Bicho). The loop's
only policy is to **try in order, accept the first Match, and reject after the budget**. The
**verifier** — the compile-and-run that turns a candidate into a `Match` / `Mismatch` /
`Error` with evidence — is **injected**, never linked, because invoking a toolchain or a
runtime is an effect.

**Code:** [`src/forge/matchloop.cpp`](../src/forge/matchloop.cpp) (`run_match_loop`, pure);
the host seam [`src/forge/effect.cpp`](../src/forge/effect.cpp) supplies a **synthetic
verifier** today (candidate `"ok"` matches) and says so in every report.

The oracles are unchanged from [decompilation §8](/design/decompilation.md), strongest first:
a byte match, a behaviour match against a running original, re-executability, then a person's
acceptance. The loop is the place where those rewards are collected.

## 5. Built vs planned

| part | status | resource |
|---|---|---|
| device library (schema, validation, JSON round-trip) | current | [`src/forge/device.cpp`](../src/forge/device.cpp) |
| route planning (bridge steps) | current | same |
| match loop (policy, injected verifier) | current | [`src/forge/matchloop.cpp`](../src/forge/matchloop.cpp) |
| split stage (PowerPC → IR, Capstone invoked) | current | [`forge/toolbox/ppc_split.py`](../forge/toolbox/ppc_split.py) |
| `effect forge devices\|route\|split\|match\|compile\|shortcut` | current | [`src/forge/effect.cpp`](../src/forge/effect.cpp) |
| pure tests (7 of 7) | current | [`tests/forge_test.cpp`](../tests/forge_test.cpp) |
| a real verifier, compile-and-run (`match --verifier compile`) | current | [`src/forge/effect.cpp`](../src/forge/effect.cpp) |
| the proposer, a rule baseline in Bicho (`kind: propose`) | current | `../VoidBicho/python/bicho/adapters/propose_adapter.py` |
| per-target scene capacity, a pure `DeviceProfile` | current | `../VoidJelly/include/voidjelly/device_profile.hpp` |
| a learned proposer (Bicho `decompile`/`name` models) | planned | `../VoidBicho` |
| per-target runtime backends (the "run" row) | planned | `../VoidJelly` |

## 6. The operating-system corollary

The author's standing assumption — *an operating system is a game*
([what a game is](/concepts/what-a-game-is.md) §8) — means an OS is just another device row:
a source whose subsystems are the kernel, the compositor and the device drivers, and whose
"game" is the whole system image. The forge does not build those tools yet; the device
library is shaped so it can, which is the preparation the author asked for.

## 7. Rules kept

The forge carries the same rules as the rest of the ingress arrow: a toolchain is **invoked,
never vendored**; game bytes and decompiled code **never enter this repository**; and the
verifier's synthetic nature is **stated in its own output**, and the real one
(`match --verifier compile`) invokes a compiler and a runtime — which is exactly why it
sits behind the effect gate.
