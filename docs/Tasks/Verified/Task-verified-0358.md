# T-0358 — [ScriviCore] OrderKey caps-only generation + dotted keys + rebalance

**Status:** ✅ Implemented - Verified
**Date Verified:** 2026-07-31
**Sprint (worked):** SP-089 (EP-027 lineage — not EP-029 scope)
**Epic:** EP-027 (chapter-folder / OrderKey identity line)

## Summary

Replaced the base-62 (`0-9 A-Z a-z`) fractional order-key generator with **base-36
CAPITALS-ONLY** (`kGenDigits`, `util/OrderKey.cpp`) joined by single-digit dot-segments
(`"A.5"`, `"Q.3.T"`) so byte-sort == Finder/Explorer natural-sort on every filesystem.

## Design decisions (settled during implementation)

- `'0'` and `'Z'` are **reserved open-bound sentinels** — never real keys. The top real
  single-digit key is `'Y'`.
- `keyBetween` uses a **bounded** fractional descent (loop capped at `max(loD,hiD)+1`, with a
  `belowHi` flag) — no unbounded growth (the earlier infinite-loop risk is gone).
- Out-of-order bounds **auto-repair** (swap); only **equal** bounded bounds return empty;
  `""` / `"0"` / `"Z"` normalize to the open sentinels.
- `rebalancedKeys(n)` spreads short caps keys within `1..'Y'`.
- Legacy lowercase keys stay accepted by `isOrderKey` (migration compatibility).

## Verification

1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON && cmake --build build --parallel`
2. `./build/ScriviCore/tests/ScriviCoreTests "[OrderKey],[T-0358]"` → all pass (14 cases).
3. `./build/ScriviCore/tests/ScriviCoreTests` → full suite green, incl. the 500-iteration
   collision test, natural-sort agreement, and `SceneSplitRepro` create-in-place (now
   `001/A/M/S`, in-place insert `G`). (Suite later grew to **361 cases** with the I-0096
   `beforeSceneID` tests.)

## Files

- `ScriviCore/src/util/OrderKey.cpp`
- `ScriviCore/src/util/OrderKey.hpp`
- `ScriviCore/tests/unit/OrderKeyTests.cpp`
- `ScriviCore/tests/integration/SceneSplitRepro.cpp`

ScriviCore-only; no `pbxproj` change (CMake-built core, not Xcode-compiled).
