# Verified Tasks: T-0412 – T-0414 (SP-106, EP-031 `[Cross]`)

All three tasks delivered SP-106 — **test integrity & CI trust** — and were ✅ **Verified 2026-08-17**
(user-approved). Sprint closed 2026-08-17 (Human-approved). Both Issues carried by the sprint
(**I-0121**, **I-0122**) were Verified in the same step and are archived to
`Issues/Verified/Issue-verified-0121-0130.md`.
Sprint detail + retrospective: `Sprints/Closed/Sprint-SP-106.md`.

---

## T-0412: Confirm the I-0121 fix on **x86-64**

**Status:** ✅ Verified (2026-08-17)
**Sprint:** SP-106 | **Epic:** EP-031 `[Cross]` | **Priority:** High

The `rebalancedKeys(1)` divide-by-zero guard was already applied when the sprint opened; this task was the
**verification the developer's machine cannot provide**. Integer division by zero is UB and differs by
target — arm64 `sdiv` yields 0 silently, x86-64 `idiv` raises `#DE` → SIGFPE — so every local run and every
`macos-latest` run had been green while `ubuntu-latest` crashed.

CI run [`31975883684`](https://github.com/justgus/Scrivi/actions/runs/31975883684): test #172 passes on
**`ubuntu-latest` 523/523** and **`macos-latest` 516/516** — **the first green ScriviCore CI since
2026-07-30**, 17 days and 7 commits of red.

⚠️ Per ruling **R2**, a green arm64 run was explicitly *not* accepted as evidence for this task.

**Files:** `ScriviCore/src/util/OrderKey.cpp` (the guard), `.github/workflows/scrivi-core-ci.yml`.

## T-0413: ⚠️ Sanitizer CI leg — `SCRIVI_ENABLE_SANITIZERS` + 2×2 matrix

**Status:** ✅ Verified (2026-08-17)
**Sprint:** SP-106 | **Epic:** EP-031 `[Cross]` | **Priority:** High

Makes UB fail **by diagnosis rather than by which instruction set happens to trap it**.

- `SCRIVI_ENABLE_SANITIZERS` (OFF by default) in the root `CMakeLists.txt`:
  `-fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer`.
- `scrivi-core-ci.yml` matrix becomes **2×2 (os × sanitizers) = 4 legs**. The plain legs stay — a sanitized
  binary is not the artifact users run.
- Each leg prints `os / arch / sanitizers` alongside its result, so no figure is quotable without its
  architecture.

**Proven RED before trusted (exit criterion 2):** reverting the `OrderKey` guard makes the sanitized
**arm64** build report `OrderKey.cpp:183:41: runtime error: division by zero` — reproducing on arm64 the
defect only x86-64 hardware trapped. A sanitizer that has never gone red proves nothing.

⚠️ **It then found a real, unknown defect on its first CI run** — **I-0122**, a `stack-use-after-scope` in
`SceneSplitRepro.cpp` that had been green for weeks while iterating freed stack memory. That is the sprint's
strongest result and it argues against the sprint's own framing: the tooling was scoped as insurance and
behaved as detection within hours.

**Files:** `CMakeLists.txt`, `.github/workflows/scrivi-core-ci.yml`.

## T-0414: macOS platform coverage — `platformDefault`'s Apple branch

**Status:** ✅ Verified (2026-08-17)
**Sprint:** SP-106 | **Epic:** EP-031 `[Cross]` | **Priority:** Medium

Linux had **7** platform-specific tests; macOS had **zero**. `platformDefault()`'s Apple branch was covered
only by a shared *"non-empty and ends in `Scrivi`"* assertion — which passes for **any** path ending in
`Scrivi`, including the Linux one. ⚠️ **The asymmetry was invisible precisely because the shared test looked
like coverage.**

Three Apple tests added: the documented shape (`~/Library/Application Support/Scrivi`), `HOME` handling, and
the previously untested `getpwuid()` fallback when `HOME` is unset.

**Verified RED before green:** mutating the Apple branch to the Linux rule fails **all 3** new tests while
**the pre-existing shared test stays green** — confirming the gap was real and that the new tests close it.

Per ruling **R4**, the 2 XDG tests stay Linux-only: the Apple rule is *different*, not absent, so this is new
coverage rather than a port. Per **R3**, the 5 `EncryptedFileSecureStore` tests were **not** ported to Swift —
they are Linux-only because the capability is.

**Files:** `ScriviCore/tests/unit/` (platform path tests), `ScriviCore/src/util/PathUtils.cpp` (untouched —
tests only).

---

## Suites at verification

| Configuration | Result |
| ------------- | ------ |
| macOS **arm64**, plain | **519/519** |
| macOS **arm64**, ASan+UBSan | **519/519** (clean — no leaks, no UB) |
| CI `ubuntu-latest` **x86-64** | **523/523** |
| CI `macos-latest` **arm64** | **516/516** |

519 = 516 + T-0414's 3 new Apple tests; the x86-64 total is higher because 7 tests are Linux-only.

---

*Archived 2026-08-17 at the SP-106 close (user-approved).*
