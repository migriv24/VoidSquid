---
type: Source
title: Learned decompilers (LLM4Decompile, SK2Decompile, D-LiFT, RlDecompiler, AutoDecompiler)
description: "Models that turn binaries into C, from 2024 to 2026: LLM4Decompile's open 1.3B–33B models and its re-executability benchmark; then reinforcement learning with compiler, symbolic-execution and test rewards (SK2Decompile, D-LiFT, RlDecompiler, AutoDecompiler); and a 2026 warning that recompiling is not the same as preserving behaviour. The starting point for Void Bicho's decompile and name model kinds."
tags: [status:planned, audience:dev, confidence:asserted, source]
timestamp: 2026-10-10T00:00:00Z
---

**`asserted`.** Written from web search summaries of the papers' abstracts and listings on
2026-10-10. No paper was opened in full and no model was run. Raising this page means
reading at least LLM4Decompile and SK2Decompile and running one of their small models on
a function with a known answer.

| work | year | what it does | measure | reported |
|---|---|---|---|---|
| **LLM4Decompile** (Tan, Luo, Li, Zhang; EMNLP 2024; `albertan017/LLM4Decompile`) | 2024 | open decompilation models, 1.3B to 33B parameters, end to end and refining Ghidra's output | **re-executability**: the C compiles and passes the original's assertions (HumanEval-Decompile, ExeBench) | the 6.7B model about 45% on HumanEval-Decompile against GPT-4o's 16% |
| **SK2Decompile** (arXiv 2509.22114) | 2025 | two phases: structure (*skeleton*), then identifier names (*skin*), each tuned with RL | compiler reward for structure; semantic similarity for names | 21.6% re-executability gain over GPT-5-mini on HumanEval |
| **D-LiFT** (arXiv 2506.10125) | 2025 | RL fine-tuning of a decompiler back end | *D-Score*: compiler feedback, symbolic execution against the original binary, readability | — |
| **RlDecompiler** (ICPC 2026) | 2026 | Ghidra context in the prompt; RL over syntax, AST similarity, compilability and tests | re-runnable rate | 1.3B: 28%; 3B: 41% |
| **AutoDecompiler** (arXiv 2606.16162) | 2026 | multi-turn RL: the model revises against compiler errors, execution results and failing tests | validity, recompilability, execution consistency, semantic fidelity | — |
| **"When LLM Decompilers Recompile More and Preserve Less"** (arXiv 2609.05370) | 2026 | a critique: surface and recompilation rewards do not measure behavioural agreement | — | only the motivation was seen |

**What it settles here** ([decompilation](/design/decompilation.md) §8):

1. **Small models are competitive.** A 1.3B or 3B model trained for the task beats a
   general frontier model on re-executability. That is the author's *"smaller and more
   specialized models"* with numbers behind it, at the reported level.
2. **Decompilation has verifiable rewards**, and the field has started using them for RL.
   Matching (byte-exact, [matching decompilation](/sources/matching-decompilation-and-recompilation.md))
   is stronger than any reward in this table and none of these papers use it, because
   their benchmarks are not games with matching projects. That gap is where this family
   could contribute.
3. **Recompiling is not preserving.** The 2026 critique is why the design asks for a
   behaviour oracle (the opaque depth) as well as compilation.
4. **None of these classify.** Every one turns a function into C. None says *"this
   function is the player's jump."* That `classify` kind is the gap in Bicho's table.

**Games are not HumanEval.** Every benchmark above is small, compiler-generated C. A game
binary built by an old proprietary compiler (IDO for the N64, MWCC for the GameCube) is
a different distribution, and nothing here says how these models do on it.
