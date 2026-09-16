# ⚠️ SP-130 RESTRUCTURE — one Sprint became FIVE (user-ruled 2026-09-15)

✅ **The user asked, in order: "is SP-130 completely planned?", "one Sprint or three?",
"three, four, or five?", and "are you including the test code and the anticipated 3-6 Issues?"**
⚠️ **Each question moved the answer, because each forced a measurement I had not taken.**

## ⚠️ What the original SP-130 assumed, and what was actually true

| assumption (2026-09-10) | ✅ measured (2026-09-15) |
| ----------------------- | ----------------------- |
| ⚠️ **35 hits, 11 files** | ✅ **10 hits, 6 files** — ⚠️ **SP-129 consumed the rest** |
| ⚠️ **T-0509 sweeps "the remaining hits"** | ⛔ **remainder is ZERO — T-0509 STRUCK** |
| ⚠️ **T-0508 is 3 blocking-stat sites** | ✅ **5 sites, ⛔ 2 already off-thread, 3 already degrade gracefully** |
| ⚠️ **T-0507 = "add endpoints, route Swift"** | ⚠️ **14 mutators, 6 synchronous saves, `@Bindable` UI state, TWO implementations** |
| ⚠️ **`inspector-layout.json` = "a correct exception"** (SP-129 T-0506) | ⛔ **RETRACTED — inside the package, already TWO owners** |
| ⚠️ **T-0510 can ship with T-0508** | ⛔ **NO — its allow-list changes when SP-141 lands** |

## ⚠️ The sizing error, and the correction

⚠️ **I sized this as "~450 lines" — the two `InspectorLayoutStore` files.** ⛔ **That counted the thing
being REPLACED, not the work.** ✅ **[SP-129]'s actual commit gives the real multiplier:**

| | lines | ratio |
| - | ----- | ----- |
| production | 421 | — |
| ✅ **tests** | 174 | **0.41×** |
| ✅ **docs/tracking** | 188 | **0.45×** |
| **total** | **783** | **1.86×** |

⚠️ **And [SP-129] was planned "Swift-only, no ScriviCore change expected."** ⚠️ **It delivered across
THREE codebases and TWO test suites, and surfaced [I-0214] mid-sprint — found by the USER, during the
live pass, on my own regression.**
⚠️ **T-0507 STARTS where SP-129 ENDED UP.**

## ✅ The five

| Sprint | Work | Blocks on | Decides |
| ------ | ---- | --------- | ------- |
| 🟡 **[SP-130]** | **T-0508** — rule the 5 `fileExists` sites | ⛔ nothing | ⛔ nothing |
| 🔵 **[SP-140]** | **T-0536** — [I-0215] unknown-key drop | ⛔ nothing | ⛔ nothing |
| 🔵 **[SP-141]** | **T-0507** — endpoints + Apple adoption | ⚠️ **a RULING** | ⚠️ **endpoint shape** |
| 🔵 **[SP-142]** | **T-0537** — retire Linux's duplicate | ⛔ **[SP-141]** | ⛔ nothing |
| 🔵 **[SP-143]** | **T-0510** — the regression guard | ⛔ **[SP-141] + [SP-142]** | ⛔ nothing |

✅ **SP-130 and SP-140 can run in PARALLEL — neither blocks on anything.**

## ⚠️ Why not three, and why not four

⚠️ **NOT THREE:** ✅ **T-0510 cannot ride with T-0508.** ⚠️ **Its allow-list must contain
`InspectorLayoutStore` today and NOT contain it after [SP-141]** — ⛔ **a guard written early passes
green across the change it polices.**

⚠️ **NOT FOUR:** ⚠️ **I argued [SP-141] and [SP-142] must land together, "or the drift survives."**
✅ **The dependency is real but the BUNDLING is not:** ⛔ **[SP-143]'s guard FAILS until both owners
are gone**, ⚠️ **so the guard — not the sprint boundary — is what prevents a half-finished retirement.**
⚠️ **Bundling them would let a Linux slip block a finished, verified Apple half** — ⛔ **[SP-132]'s
exact failure.**

## ⚠️ What this restructure does NOT claim

⛔ **It does not claim five sprints is cheap.** ⚠️ **It is more tracking overhead for ~450 lines of
production code.** ✅ **The defensible alternative is FOUR, folding [SP-143] into [SP-142].**
⚠️ **The case for keeping it separate is that a guard written by the same change it polices has no
independent witness** — ✅ **and this Epic has already paid for that lesson twice ([I-0214], [I-0215]).**
