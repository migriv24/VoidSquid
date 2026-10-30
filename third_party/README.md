# third_party — code Void Squid holds but does not own

Each folder here is someone else's work, kept as a **reference**. Nothing here is
compiled into anything, linked by anything, required by any build, or run from this
folder. A toolchain Void Squid *invokes* is found on the person's machine, not here
(CLAUDE.md rule 12; `okf/concepts/build-surface.md`, the ingress row).

| folder | upstream | commit | licence | why |
|---|---|---|---|---|
| `universal-modder/` | https://github.com/rehan-remade/universal-modder | `550651675153070cce68066e07c3c8f95df86d08` (2026-10-10) | MIT (see its `LICENSE`) | the reference for decompilation and mashups: `okf/design/decompilation.md`, `okf/sources/universal-modder.md` |

## `universal-modder/`

Added 2026-10-10 at the author's request (*"this is going to be very important to
download and use as reference"*), and as the starting point if the author decides to fork
it (S19 in `okf/design/open-questions.md`). Taken from a shallow clone, then:

- removed `.git/`;
- removed media files over 64 KB (generated images, `.glb` models, a GIF): 28 MB → 5.1 MB.
  Code, skills, the knowledge base and the examples' code are all kept;
- **renamed the upstream agent entry files**: `CLAUDE.md` → `CLAUDE.upstream.md`,
  `AGENTS.md` → `AGENTS.upstream.md`, `GEMINI.md` → `GEMINI.upstream.md`, `.claude/` →
  `dot-claude.upstream/`. Upstream's instructions tell an agent how to mod games and add a
  hook that puts `um` on PATH; under their original names a coding agent working in this
  repository could load them as if they were Void Squid's. They were read before
  renaming and are benign. Restore the names in a fork, not here.

The Windows checkout of upstream needs `git config core.longpaths true`; some knowledge
base paths exceed 260 characters.

To use `um` itself, install it the upstream way (`uv tool install
git+https://github.com/rehan-remade/universal-modder`); recon (`effect recon`) reports
whether it is on PATH.
