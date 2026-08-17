# Verified Issues: I-0121 – I-0130

Archived Issues, ✅ **Resolved - Verified** by the user. Batched in decades of ten per
`Issue-GUIDELINES.md`.

| ID | Title | Severity | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| I-0121 | `[ScriviCore]` `rebalancedKeys(1)` divides by zero — CI red since 2026-07-30 | High | SP-106 | 2026-08-17 |
| I-0122 | `[ScriviCore]` `stack-use-after-scope` — test iterates a destroyed temporary `Result` | Medium | SP-106 | 2026-08-17 |

---

## I-0121 — ⚠️ `rebalancedKeys(1)` divides by zero; ScriviCore CI was red on every commit since 2026-07-30

**Severity:** High · **Sprint:** SP-106 · **Epic:** EP-031
**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**

**Symptom.** ScriviCore CI failed on **every commit from 2026-07-30 (1c42838) to 2026-08-16** — 17 days,
7 commits — while every local `ctest` run and every `macos-latest` CI leg reported green.

**Defect.** The `n == 0` ternary guard in `rebalancedKeys` is unreachable while the divisor is `n - 1`, so
`rebalancedKeys(1)` divides by zero.

⚠️ **Why it survived 17 days.** Integer division by zero is **UB, and UB is free to differ by target**:
arm64 `sdiv` quietly yields 0; x86-64 `idiv` raises `#DE` → SIGFPE. The developer's Mac and the
`macos-latest` runner were green; `ubuntu-latest` crashed. **"All tests pass" had meant "all tests pass on
arm64" since July**, and six sprints closed on that evidence.

**Fix.** Guard the single-key case explicitly. Per user ruling **R1**, a single key takes the **midpoint**,
not `lo`: `rebalancedKeys(1)` → `"H"` (`(lo + hi) / 2 == 17`, the generator alphabet being 0-indexed), with
`keyBefore("H") == "8"` and `keyAfter("H") == "Q"` — room on both sides.

**Verification.** Per ruling **R2**, a green arm64 run was explicitly **not** accepted as evidence. CI run
[`31975883684`](https://github.com/justgus/Scrivi/actions/runs/31975883684): test #172 passes on
**`ubuntu-latest` 523/523** and **`macos-latest` 516/516** — the first green ScriviCore CI since 2026-07-30.

**Files:** `ScriviCore/src/util/OrderKey.cpp`.

⚠️ **Consequence recorded for SP-100.** SP-093, SP-095, SP-096, SP-097, SP-098 and SP-099 all closed while
ScriviCore CI was failing; their "ctest N/N green" figures are **local arm64 results**. This does not
invalidate the work — the CI log showed **522/523 passing**, a single crashing test, not a rotten suite — but
re-running those suites on both architectures is the only way to know nothing else was hiding behind the
crash.

---

## I-0122 — ⚠️ `stack-use-after-scope`: a test iterated a destroyed temporary `Result`

**Severity:** Medium · **Sprint:** SP-106 · **Epic:** EP-031
**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**

**How it was found.** ⚠️ **By SP-106's own sanitizer leg (T-0413), on its very first CI run** — hours after
that leg was added, in the same sprint. The test had been passing green for weeks while reading freed stack
memory.

**Defect.** `Result::value()` returns a reference **into** the `Result`. Iterating
`listScenesByOrder(...).value()` directly leaves the loop walking a destroyed temporary: C++ lifetime
extension covers the temporary bound *directly* to the range variable, **not** the `Result` that owns the
vector behind it.

⚠️ **Same architecture-dependent invisibility as I-0121, one layer up.** The test passes on arm64 **both
before and after** the fix — the freed bytes happen to survive there. Only the **x86-64 sanitized** leg could
distinguish fixed from broken. Two defects in two days whose visibility depended on the host.

**Fix.** Bind the `Result` to a named variable before iterating its `value()`.

**Scope checked, not assumed.** A brace-matched scan of every range-for in `ScriviCore/src` and
`ScriviCore/tests` found **exactly one** iteration over a *temporary* `Result` — this one. The ~20 other
`for (... : xR.value())` sites bind a **named** `Result` and are correct. **Test code only; no shipping code
affected.**

**Verification.** ASan+UBSan **519/519** clean on macOS arm64; sanitized CI legs green.

**Files:** `ScriviCore/tests/` — `SceneSplitRepro.cpp`.

---

*Archived 2026-08-17 at the SP-106 close (user-approved). Sprint record:
[`../../Sprints/Closed/Sprint-SP-106.md`](../../Sprints/Closed/Sprint-SP-106.md).*
