# Toy Ledge — the first vertical slice's target

**Synthetic. No binary was disassembled, and no game data is here.** This fixture exists
so the MoleRat pipeline can be exercised end to end without a real target, a toolchain, a
model, or any copyrighted bytes.

`foreign.ir.json` is the normalized `molerat.foreign-ir/0.1` form a reader would produce:
three functions, two tables, a constant pool, and the residual the reader would state.

- `player_update` moves the player and calls `apply_gravity` and `door_check`.
- `apply_gravity` folds `stats.gravity` into `stats.player_vy`.
- `door_check` branches on `doors.count`.

The fixture's own `target.origin` says it is authored, and the emitted research marks every
claim `confidence:weak`. The point is the **shape** of the handoff, not the quality of a
decompilation.

```
squid --allow-effects=molerat effect molerat gather molerat/fixtures/toy-ledge \
    --ir molerat/fixtures/toy-ledge/foreign.ir.json --run toy1 --seed 42
squid --allow-effects=molerat effect molerat research toy1
squid --allow-effects=molerat effect molerat compose toy1
squid --allow-effects=molerat effect molerat residual toy1
squid --allow-effects=molerat effect molerat build toy1 --out molerat/out/toy1
squid --state molerat/out/toy1/toy1.squid --json validate
```
