# SP-106 — `[Cross]` Test integrity & CI trust

**Status:** ✅ **CLOSED 2026-08-17 (user-approved).** All three Tasks ✅ Verified; both Issues
(I-0121, I-0122) ✅ Resolved - Verified.
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Codebases:** `[ScriviCore]` + CI configuration
**Activated:** 2026-08-16 · **Closed:** 2026-08-17 · Ran before SP-102 and SP-100.

---

## Why this sprint existed

The user asked why GitHub had been reporting a CI error. **ScriviCore CI had been red on every commit since
2026-07-30 (1c42838) — 17 days, 7 commits** — on a one-line divide-by-zero in `rebalancedKeys(1)`.

⚠️ **The defect was not subtle; the reason it survived was.** Integer division by zero is UB, and UB is free
to differ by target: arm64 `sdiv` quietly yields 0, x86-64 `idiv` raises `#DE` → SIGFPE. The developer's Mac
and the `macos-latest` runner were **green**, `ubuntu-latest` **crashed**. So *"all tests pass"* had meant
*"all tests pass on arm64"* since July, and six sprints closed on that evidence.

---

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0412 | Confirm the I-0121 fix on **x86-64** | ✅ **Verified (2026-08-17)** |
| T-0413 | ⚠️ **Sanitizer CI leg** — `-fsanitize=undefined,address`, `-fno-sanitize-recover=all` | ✅ **Verified (2026-08-17)** |
| T-0414 | **macOS platform coverage** — `platformDefault`'s Apple branch | ✅ **Verified (2026-08-17)** |

## Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0121 | `rebalancedKeys(1)` divides by zero — CI red since 2026-07-30 | High | ✅ **Resolved - Verified (2026-08-17)** |
| I-0122 | ⚠️ **`stack-use-after-scope`** — test iterates a destroyed temporary `Result` | Medium | ✅ **Resolved - Verified (2026-08-17)** |

> ⚠️ **I-0122 was found BY T-0413, on the sanitizer leg's very first run** — hours after that leg was added,
> in this same sprint. The test had been green for weeks while reading freed stack memory.
>
> **`Result::value()` returns a reference INTO the Result.** Iterating `listScenesByOrder(...).value()`
> directly leaves the loop walking a destroyed temporary: lifetime extension covers the temporary bound
> *directly* to the range variable, not the `Result` that owns the vector behind it.
>
> ⚠️ **Same architecture split as I-0121, one layer up.** The test passes on arm64 **both before and after**
> the fix — the freed bytes happen to survive there. Only the **x86-64 sanitized** leg could tell the
> difference. Two defects in two days whose visibility depended on the host.
>
> **Scope checked, not assumed:** a brace-matched scan of every range-for in `ScriviCore/src` and
> `ScriviCore/tests` found **exactly one** iteration over a *temporary* `Result` — this one. The ~20 other
> `for (... : xR.value())` sites bind a **named** `Result` and are correct. Test code only; no shipping code
> affected.

---

## Delivery

### T-0412 — the fix confirmed where it actually fails

CI run [`31975883684`](https://github.com/justgus/Scrivi/actions/runs/31975883684): test #172 passes on
**`ubuntu-latest` 523/523** and **`macos-latest` 516/516** — **the first green ScriviCore CI since
2026-07-30.**

⚠️ **The x86-64 result is the only one that counts for this defect.** A green arm64 run is what hid the bug
for 17 days; it could never have distinguished fixed from broken.

### T-0413 — UB now fails by diagnosis, not by architecture accident

- `SCRIVI_ENABLE_SANITIZERS` (OFF by default) in the root `CMakeLists.txt`:
  `-fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer`.
- `scrivi-core-ci.yml` matrix is now **2×2 (os × sanitizers) = 4 legs**. The plain legs stay, because a
  sanitized binary is not the artifact users run.
- Each leg prints `os / arch / sanitizers` alongside its result.

> ⚠️ **The sanitizer was proven to work, not assumed.** Reverting the `OrderKey` guard makes the sanitized
> **arm64** build report `OrderKey.cpp:183:41: runtime error: division by zero` — i.e. it reproduces on arm64
> the exact defect only x86-64 hardware trapped. **A sanitizer that has never gone red proves nothing**, so
> this was demonstrated before being trusted (sprint exit criterion 2).

> ✅ **…and then it paid for itself immediately.** The sanitizer leg **failed on its very first CI run** —
> not on the seeded defect, but on a **real, unknown** one: **I-0122**, a `stack-use-after-scope` in
> `SceneSplitRepro.cpp` that had been passing green for weeks while iterating freed stack memory.
>
> ⚠️ **This is the sprint's strongest result, and it argues against its own framing.** SP-106 was scoped as
> *"fix a known defect and add tooling."* The tooling found a second defect within hours, with the same
> architecture-dependent invisibility. **The suite was hiding more than one thing, and the only reason we
> know is that we stopped trusting a single architecture.** What else is latent is now an empirical
> question, not a rhetorical one — which is worth weighing at SP-100.

### T-0414 — the macOS coverage gap

Linux had **7** platform-specific tests; macOS had **zero**. `platformDefault()`'s Apple branch was covered
only by a shared *"non-empty and ends in `Scrivi`"* assertion — which passes for **any** path ending in
`Scrivi`, including the Linux one. ⚠️ **The asymmetry was invisible precisely because the shared test looked
like coverage.**

Three Apple tests added: the documented shape (`~/Library/Application Support/Scrivi`), `HOME` handling, and
the previously untested `getpwuid()` fallback when `HOME` is unset.

> **Verified RED before green:** mutating the Apple branch to the Linux rule fails **all 3** new tests while
> **the pre-existing shared test stays green** — confirming the gap was real, and that the new tests close it.

---

## Suites at close

| Configuration | Result |
| ------------- | ------ |
| macOS **arm64**, plain | **519/519** |
| macOS **arm64**, ASan+UBSan | **519/519** (clean — no leaks, no UB) |
| CI `ubuntu-latest` **x86-64** (pre-T-0413/0414 commit) | **523/523** |
| CI `macos-latest` **arm64** (pre-T-0413/0414 commit) | **516/516** |

⚠️ **Figures name their architecture deliberately.** "516/516 green" without a platform is the habit that let
I-0121 run red for 17 days. 519 = 516 + T-0414's 3 new Apple tests; the x86-64 total is higher (523/526)
because 7 tests are Linux-only.

---

## Rulings carried from planning

**R1 — single-key position is the MIDPOINT** (user-ruled). `rebalancedKeys(1)` → `"H"`; `keyBefore("H")=="8"`,
`keyAfter("H")=="Q"`.

**R2 — a green arm64 run is NOT evidence for I-0121.** Acceptance required x86-64 or a sanitizer build.
**This is the rule that would have caught the bug 17 days earlier.**

**R3 — the 5 `EncryptedFileSecureStore` tests stay in C++, not Swift.** They are Linux-only because the
*capability* is: `scrivi_c_api.cpp:94-106` falls back to the in-memory `PrototypeSecureStore` on Apple, so
identity does not survive restart there. Four of the five assert the `SecureStore` **contract**, not OpenSSL —
if Apple regains a persistent store they belong against the interface in Catch2, run on both platforms, **not
duplicated in Swift**. *(Restoring Apple's persistent store was NOT in this sprint.)*

**R4 — the 2 XDG tests are correctly Linux-only.** The Apple rule is *different*, not absent; T-0414 added it
as new coverage rather than a port.

---

## Exit criteria

| # | Criterion | State |
| - | --------- | ----- |
| 1 | ScriviCore CI green on both matrix legs — first time since 2026-07-30 | ✅ Met (run `31975883684`) |
| 2 | Sanitizer **proven** to fail on reintroduced UB | ✅ Met — `OrderKey.cpp:183:41` reported on arm64 |
| 3 | macOS `platformDefault` has real coverage; asymmetry closed or documented | ✅ Met — 3 tests, verified RED first |
| 4 | `ctest` figures name their architecture | ✅ Met — in this record and in the CI leg names |

---

## Retrospective

**What the sprint proves.** The value delivered was not the one-line guard — that was already applied before
the sprint opened. It was **the ability to know**. Two defects, two days apart, both invisible on the
developer's machine and both surfaced only by widening what the suite runs on. The guard closed one bug; the
matrix closed the class.

**What it cost to learn.** 17 days of red CI on a defect a sanitizer reports in one line, and six sprint
closes whose "all tests pass" meant one architecture. Nothing is known to be wrong in those sprints — the CI
log showed 522/523, a single crashing test, not a rotten suite — but the evidence standard was weaker than it
read.

**The habit change worth keeping.** Quote the architecture with the number. `516/516` is not a result;
`516/516 macOS arm64` is.

---

## Carried forward — NOT done in this sprint

- ⚠️ **Apple has no persistent `SecureStore`.** Identity does not survive restart on Apple
  (`PrototypeSecureStore` is in-memory). Known and pre-existing, surfaced by R3's analysis; **needs its own
  Task** rather than being folded in here.
- **The Apple/Linux coverage asymmetry is narrowed, not closed.** T-0414 covered `platformDefault`; Linux
  still has 5 SecureStore tests with no Apple counterpart, and that is correct only while Apple has no
  persistent store.
- ⚠️ **The suite has been single-architecture for six sprints (SP-093 through SP-099).** Those closes quoted
  local arm64 figures. Nothing is known to be wrong, but **SP-100 should weigh what its verification rests
  on** — re-running those suites on both architectures is the only way to know nothing else was hiding
  behind the crash.

---

*Closed 2026-08-17 (user-approved). Tasks T-0412/T-0413/T-0414 archived to
`../../Tasks/Verified/Task-verified-0412-0414.md`; Issues I-0121/I-0122 archived to
`../../Issues/Verified/Issue-verified-0121-0130.md`. **Next: SP-102** (`[Apple]` pending presentation +
warning view + `sources` card), then **SP-100** (EP-031 verification & Epic close).*
