---
type: Manifest
title: Void Squid
name: Void Squid
id: voidsquid
version: 0.0.0
description: A minimal game engine on Void Core and Void Maiz — the host that opens a .ink, shows it, and builds it, performing every effect Void Jelly refuses to.
status: planned
authors: [migriv24]
icon: squid
theme: void
palette.primary: "#1f7a8c"
palette.accent: "#e0b0d5"
palette.bg: "#07131a"
palette.ink: "#dfeef2"
tags: [status:planned, audience:dev, confidence:asserted]
timestamp: 2026-08-26T00:00:00Z
---

The **app manifest** for this bundle — Void Squid's structured self-description (see
`VoidCore:/concepts/app-manifest.md`).

`version: 0.0.0` and `status: planned` are not placeholders. **Nothing is built.**

# What it is, in one line

**A conformance harness for [Void Jelly](../../VoidJelly/okf/index.md) that happens to look
like an editor** ([scope](/concepts/scope.md)). It is called a game engine because that is
what it becomes; it is not one yet, and treating it as one is the failure mode
[the railguard](/design/what-voidsquid-is-not.md) exists to prevent.

# Name

*Squid* was the author's name for the whole 3D project at its founding on 2026-08-25. On
2026-08-26 the author moved it here — to the game engine — and renamed the library **Void
Jelly**. The cephalopod stayed with the thing that *acts*; the jellyfish took the thing that
*flows through* everything. `.ink` stayed with Void Jelly, where it names the format.

Both bundles record the rename in their logs.
