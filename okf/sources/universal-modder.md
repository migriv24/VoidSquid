---
type: Source
title: universal-modder (rehan-remade/universal-modder)
description: "MIT-licensed skills, a Python CLI (`um`) and a reviewed knowledge base that let any coding agent mod games the person owns: recon, decompilers, asset generation, testing in the running game, two-game mashups, and written field notes. Its bridge contract and mashup case table are the most complete public account of what two unrelated games have in common; a trimmed reference copy is in third_party/."
tags: [status:planned, audience:dev, confidence:verified, source]
timestamp: 2026-10-10T00:00:00Z
---

**Verified 2026-10-10** by reading the repository at commit
`550651675153070cce68066e07c3c8f95df86d08` (2026-10-10 01:44 −0700): the README, the
`game-recon`, `mashup-mods` and retro-decompilation playbooks, the mashup case table, the
knowledge base's README and three technique notes, and the engine table in `um/scan.py`.
Nothing in it was run. The page's claims about *other* projects are universal-modder's,
which it labels as creator reports; this page does not upgrade them.

**What it is.** *"Skills, tools and a shared knowledge base that let any AI coding agent
mod almost any PC game you own."* Point an agent at an installed game; it finds the game,
picks a route, reads the code, builds a mod, generates art and audio, tests in the running
game, records a video and writes a field note. Installs as a Claude Code plugin, a Codex,
Gemini or Cursor extension, or a clone. Licence **MIT**.

**What is in it.**

- `skills/`: eleven playbooks (`mod-any-game` with twelve engine references, `game-recon`,
  `reverse-engineering`, `mashup-mods`, `asset-pipeline`, `fal-assets`, `game-automation`,
  `showcase-video`, `publish-mod`, `share-field-notes`, `game-research-websearch`).
- `um/`: a Python CLI. `um scan` fingerprints an install (engine and version, managed or
  native, anti-cheat, loaders, save folders, the community route); `um kb` searches and
  extends the knowledge base; `um fal` and `um comfy` generate assets (fal is a paid remote
  API, ComfyUI local); `um sprite`, `um render3d` (Blender), `um win`, `um video`,
  `um backup`; `um publish check` refuses to ship game files, decompiled code or keys.
- `knowledge/`: about 160 field notes, per game and per technique, written by agents and
  reviewed by pull request, each with versions, the route, the oracle and the gotchas.

**What it settles here** (argued in [decompilation](/design/decompilation.md)):

1. **The bridge contract is the shared structure of games**, learned from practice. Its
   fields: route and versions; an **ownership table** (player movement, camera, world
   geometry, collision each way, NPCs, health, inventory, saves, menus, scripted events,
   each with a hand-back trigger); **units and axes as a shared function**; channels;
   protocol identity; lifecycle; and *"not covered"*. That is a holiday's interface plus
   links plus a residual, in this family's terms (§4 of the design page).
2. **libsm64 is the opaque depth working.** *"Feed it collision and input, and it returns
   Mario's state and mesh."* A decompilation wrapped as a library with a contract was
   enough to put Mario into Garry's Mod and Elden Ring.
3. **"Don't advertise a universal adapter."** Its own conclusion after about thirty
   mashups: every bridge needed per-host reverse engineering. What generalises is the
   vocabulary, not an adapter.
4. **Evidence levels**: creator report, design or proposal, source inspection, derived
   comparison, synthetic test, real run. Only the last supports *"it works"*. A finer form
   of this bundle's `asserted` / `verified`.
5. **The decompilation loop that worked with agents**: one function at a time, a
   build-and-verify script that says how close, a cap of about ten attempts, commit every
   match, a shared learnings file, cheapest candidates first, several worktrees in
   parallel. *"Pure mechanical decompilation first; modernization afterwards."* And: *"a
   large decomp can take billions of tokens."*
6. **It covers PC installs, not console headers.** `um scan` has no reader for DS, N64,
   GBA or GameCube images; recon in this repository adds those and names `um` as the
   deeper tool for PCs ([recon](/concepts/recon.md)).

**The reference copy.** `third_party/universal-modder/`, from a shallow clone at the commit
above, with `.git` and media over 64 KB removed (5.1 MB kept of 28 MB), and the upstream
agent entry files renamed (`CLAUDE.upstream.md`, `AGENTS.upstream.md`,
`GEMINI.upstream.md`, `dot-claude.upstream/`) so their modding instructions never load into
a Void Squid session. See `third_party/README.md`. Nothing links it or runs from it.

Its rules are adopted as this bundle's ([decompilation](/design/decompilation.md) §10):
games the person owns, offline or self-hosted; never ship game files or decompiled code;
never inject into online games with anti-cheat; be honest about what was verified.
