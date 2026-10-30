---
type: Source
title: REA — Reverse Engineer Anything (morluto/rea)
description: An MIT-licensed MCP server and CLI that lets a coding agent reverse engineer local targets (native binaries, ELF, .NET, APKs, firmware, Electron apps, websites, process behaviour) by driving existing engines (Ghidra, IDA, Hopper, JADX, Binwalk, mitmdump, GDB). Its results carry "the evidence and limitations behind each conclusion." The author asked that it be included; this page records what it is and what shape "include" can take.
tags: [status:planned, audience:dev, confidence:verified, source]
timestamp: 2026-10-09T00:00:00Z
---

**Verified 2026-10-09** against the repository README at `https://github.com/morluto/rea`
(fetched; the README-level claims below are what was read, not the code).

**What it is.** *"Reverse engineer anything with agents, from app behavior down to
native binaries."* It runs as a **local MCP server** that an agent (Claude Code, Codex,
Cursor, Gemini CLI) calls, plus a CLI exposing the same workflows. Setup is
`npx rea-agents setup`; it requires Node.js 22 or newer. Licence **MIT**. At fetch time
the README reported about 47 thousand stars.

**Inputs.** Native binaries; ELF and EVM bytecode; recorded Linux crashes; JavaScript and
Electron apps (directories or ASAR); websites and HAR or mitmproxy captures; .NET
assemblies; Android APKs; firmware; packages and resources; process behaviour.

**Outputs.** Per target: pseudocode, assembly, symbols and references for native code;
modules, imports and IPC relationships for Electron; CIL for .NET. Every result includes
*"the evidence and limitations behind each conclusion."* Analysis runs locally; the
agent's model provider is whoever the agent uses.

**How it works.** It does not decompile anything itself. It **drives external engines**:
Hopper, Ghidra, IDA, pwntools, GDB with pwndbg, JADX, Binwalk and Unblob, mitmdump, and a
Chrome-family browser. It is an orchestration layer with a uniform contract over them.

**What it settles here.** Three findings, all recorded in
[construct and deconstruct](/concepts/construct-and-deconstruct.md):

1. **It is a toolchain, not a library.** It is a Node application that spawns other
   programs. In the family's vocabulary that is a **process holiday**, the same kind of
   thing as devkitARM on the egress side. It cannot be vendored into a C++ build in the
   sense rule 10 means, and it does not need to be; devkitARM is not vendored either.
2. **Its output shape is a residual.** *"Evidence and limitations behind each
   conclusion"* is what Void Jelly's residual is for emission, stated for ingestion. A
   reader that turns REA's findings into a mantle should carry the limitations as
   first-class content, not drop them.
3. **It is agent-shaped, and so is Void Squid.** REA is an MCP server; Squid is a Void
   Maiz application whose every gesture is a dispatcher command. The two meet in an
   agent's terminal without either linking the other.

Its disclaimer applies here as well: it is for research, analysis and reconstruction, and
the person running it is responsible for authorisation. Nothing in this bundle changes
that.
