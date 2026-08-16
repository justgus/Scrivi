# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

**Currently: 3.** I-0017 and I-0018 are 🔴 Open; **I-0121** is 🟠 Resolved (code fix) - Not Verified, with its
CI and coverage work still outstanding and assigned to EP-031.

⚠️ **Verified Issues do not belong in this file.** I-0058 (Verified 2026-07-09) and I-0112 (Verified
2026-08-11) sat here for weeks after verification and were archived to `Verified/` on 2026-08-16. Archive an
Issue in the same step it is verified.

---

## I-0017: Window maximized state not restored on app relaunch

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `WindowFrameAutosave.swift`
**Severity:** Medium
**Sprint:** Not Assigned
**Related:** I-0051 (multi-window per-project frame/position restore — Verified 2026-06-29, subsumes the position/size part of this); **I-0055** (multi-window maximize-restore defect — the same zoom-restore problem on the per-window model; fix the two together)

**Description:**
Window position, size, and maximized state are not fully restored between app launches. Frame and position restore correctly. Maximized state does not — the window always relaunches un-maximized regardless of saved zoom state.

**Expected Behavior:**
On relaunch, the window appears at the same size, position, and maximized state as when the user last quit. The Landing View and Editor share the same window — no resize occurs when transitioning between them.

**Actual Behavior:**
Frame and position restore correctly. Maximized state does not restore.

**Steps to Reproduce:**
1. Maximize the window.
2. Quit the app.
3. Relaunch — window opens un-maximized.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`window.zoom(nil)` fires too early — SwiftUI's `WindowGroup` continues async layout passes after the call and overrides it. Current approach uses `NSApplication.didFinishLaunchingNotification` as the trigger, but this has not resolved the issue. Requires deeper investigation.

**Resolution:** TBD

---

## I-0018: Scene Navigator shows no selection on app load

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `ViewportSceneLoader.swift`
**Severity:** Low
**Sprint:** Not Assigned

**Description:**
When the app loads a project, no scene is selected/highlighted in the Scene Navigator. The Navigator self-corrects on first scroll.

**Expected Behavior:**
On load, the Navigator highlights the scene visible at the top of the manuscript viewport.

**Actual Behavior:**
No scene is highlighted until the first scroll event.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`viewportSceneID` is intentionally left nil during `loadAll()`. The scroll observer sets it on first scroll, but this hasn't fired at load time.

**Resolution:**
TBD — needs a mechanism to determine the top-of-viewport scene after `NSTextView` completes initial layout without triggering a spurious scroll notification.

---


## I-0121: [ScriviCore] `rebalancedKeys(1)` divides by zero — ScriviCore CI has failed on every commit since 1c42838 (2026-07-30)

**Status:** 🟠 **Resolved (code fix) - Not Verified (2026-08-16)** — the one-line guard is applied and proven
locally (see Resolution). ⚠️ **The Issue is NOT complete:** the `-fsanitize=undefined` CI work and the macOS
platform-coverage gap remain, and **the fix has not yet been observed green on x86-64**, which is the only
platform that traps it. Full closure belongs to the sprint that takes this Issue.
**Platform:** All platforms (UB — **traps** on x86-64 Linux; **silently survives** on arm64)
**Component:** `ScriviCore/src/util/OrderKey.cpp:175–183` (`rebalancedKeys`)
**Severity:** High — not for its runtime impact, but because **CI has been red for 17 days and 7 commits**, so
the suite has not gated anything since 2026-07-30.
**Sprint:** Not Assigned — **to be scoped into EP-031's next sprint at planning** (user ruling 2026-08-16)
**Epic:** EP-031 (by assignment; the defect itself predates and is independent of EP-031's subject matter)
**Date Identified:** 2026-08-16
**Reported by:** user — "the last workflow run on github.com reported a CI error on Commit 9333687… consistent
since Commit 1c42838"

**Description:**

`rebalancedKeys(n)` computes each key's position with:

```cpp
const int want = (n == 0)
    ? (lo + hi) / 2
    : lo + static_cast<int>((static_cast<long long>(hi - lo)
                             * static_cast<long long>(i))
                            / static_cast<long long>(n - 1));   // ← n == 1 ⇒ divide by zero
```

The ternary guards `n == 0`, but the divisor is `n - 1`, which is **zero when `n == 1`**. Integer division by
zero is undefined behavior.

⚠️ **The guard is also dead code.** The function already returned at line 164 (`if (n == 0) return out;`), so
the `n == 0` branch is unreachable. The guard was written for the wrong boundary — it should protect `n == 1`,
the case that actually divides by zero.

**Expected Behavior:**
`rebalancedKeys(1)` returns one valid, single-character caps key with room before and after it.

**Actual Behavior:**
On x86-64 the process receives **SIGFPE** and the test binary dies. Catch2 surfaces it as
`***Exception: Numerical`, which ctest reports as `(NUMERICAL)` — a crash, not a failed assertion.

**Steps to Reproduce:**
1. Build ScriviCore with tests on an **x86-64 Linux** host (the `ubuntu-latest` runner).
2. `ctest --test-dir build`
3. Test **#172** `rebalancedKeys returns short, spread, ascending, valid caps keys (T-0358)` aborts.
   `OrderKeyTests.cpp:234` iterates `n` over `{1, 2, 11, 34, 100}` — the **first** value trips it.

**Root Cause Analysis:**

Confirmed by compiling the exact expression with `-fsanitize=integer-divide-by-zero`:
`runtime error: division by zero` at that statement.

⚠️ **Why it never failed locally — this is the important part.** Integer division by zero is UB, so it need
not fail consistently, and here it does not:

| Host | Arch | Compiler | Result |
| ---- | ---- | -------- | ------ |
| Developer Mac + `macos-latest` runner | **arm64** | Clang | **Passes** — arm64 `sdiv` returns 0 for a zero divisor; no trap |
| `ubuntu-latest` runner | **x86-64** | GCC 13.3.0 | **Fails** — x86 `idiv` raises `#DE` → SIGFPE |

Both runners report arm64 in parts of their image metadata; the Ubuntu runner is confirmed **x86-64** by its
OpenSSL path (`/usr/lib/x86_64-linux-gnu/libcrypto.so`). **The bug is 100% reproducible where it traps and
100% invisible where it does not** — local `ctest` is **516/516 green** on the developer machine, which is why
this survived 17 days.

**Introduced by:** **1c42838** (2026-07-30, "SP-089 partly complete. Testing.") — established with
`git log -S "static_cast<long long>(n - 1)" -- ScriviCore/src/util/OrderKey.cpp`, which returns exactly one
commit. That commit rewrote `OrderKey.cpp` (+263/−71) and added both `rebalancedKeys` and the test that
exercises it, so **the defect and its detector arrived together.** This matches the user's report exactly.

**Impact:**

- **ScriviCore CI has been failing on every push since 2026-07-30** — SP-093, SP-095, SP-096, SP-097, SP-098
  and SP-099 all pushed red. `Scrivi Linux App CI` passed throughout, which is why the failure was easy to
  read as noise.
- ⚠️ **Every "ctest N/N green" claim in the sprint records for that window was a *local, arm64* result.** The
  suite did not gate any of those closes on x86-64. Nothing else is currently failing — the CI log shows
  **522/523 passing**, one crash — so this is a single defect, not a rotten suite.
- Runtime exposure is **low**: `rebalancedKeys(1)` means rebalancing a manuscript to a single scene/chapter.
  Reachable in principle, unreported in practice, and harmless on arm64 (Apple silicon) — but **wrong
  everywhere**, and a live SIGFPE on any x86-64 desktop build (Linux/Windows).

**Proposed Resolution:**

Guard the boundary that actually divides. For a lone key the midpoint matches the intent of the existing
(unreachable) `n == 0` branch:

```cpp
const int want = (n == 1)
    ? (lo + hi) / 2
    : lo + static_cast<int>(...);   // unchanged
```

This yields **`'H'`** for the single-key case — `(lo + hi) / 2 == 17`, and the generator alphabet is
0-indexed, so position 17 is `'H'`. (An earlier draft of this Issue said `'I'`, off by one; the *behavior* is
the intended true midpoint either way.) It satisfies the test's own requirements — verified
`keyBefore("H") == "8"` and `keyAfter("H") == "Q"`, so there is room on both sides, and it is one character so
the `n <= 34` length assertion holds. **The single-key position is a behavior choice** (`lo` would also be
defensible, and is more stable); the user ruled **midpoint** on 2026-08-16.

**Files Affected:** `ScriviCore/src/util/OrderKey.cpp` (the guard), `.github/workflows/scrivi-core-ci.yml`
(see below).

⚠️ **Fixing the divide alone leaves the class open.** A whole family of UB is invisible on the developer's
arm64 hardware and only trapped by chance on one runner. **Add a sanitizer configuration to CI**
(`-fsanitize=undefined,address` with `-fno-sanitize-recover=all`) so this class fails deterministically on
both platforms rather than depending on which instruction set happens to trap. The user has agreed to this
(2026-08-16) and noted it **may change the test configuration** — so it is sprint-planning work, not a
drive-by edit. Today the root `CMakeLists.txt` sets **no** sanitizer, `-Werror`, or build-type flags at all.

**Resolution (code fix applied 2026-08-16 — the guard only; the CI half is still open):**

`OrderKey.cpp:175` changed from `(n == 0)` to `(n == 1)`, with a comment recording why the boundary is `n == 1`
and why the bug is invisible on arm64. **Evidence:**

1. ⚠️ **The verification method matters more than the result here.** A plain arm64 build passes *with or
   without* the fix — it cannot distinguish them, and accepting it would have proved nothing. So the suite was
   rebuilt with `-fsanitize=undefined -fno-sanitize-recover=all`, which makes arm64 trap what x86-64 traps in
   hardware.
2. **Confirmed RED without the fix:** `git stash` + UBSan rebuild → test #172 dies with **SIGABRT** at
   `OrderKeyTests.cpp:232`. This reproduces the CI failure on the developer's machine for the first time.
3. **Confirmed GREEN with the fix:** UBSan build, full suite **516/516 passed, 0 failed** — so the fix works
   *and* nothing else in the suite has latent UB on this platform.
4. **Single-key output checked directly:** `rebalancedKeys(1)` → `"H"`; `keyBefore("H") == "8"`,
   `keyAfter("H") == "Q"` — room on both sides, one character, as the test requires.

**Verification (outstanding — for the sprint that takes this):**
1. ⚠️ Test #172 passes on **x86-64 Linux**, the platform that actually traps. **A green arm64 run is not
   evidence for this defect** — that is precisely what hid it for 17 days. The UBSan run above is the closest
   local substitute, not a replacement.
2. ScriviCore CI green on **both** matrix legs for the first time since 2026-07-30.
3. `-fsanitize=undefined` added to the CI matrix so this class fails deterministically rather than by
   architecture accident.

---

*Last Updated: 2026-08-16, later same day (**I-0058 and I-0112 archived to `Verified/`; I-0121's code fix
applied.** Both Issues had been Verified — 2026-07-09 and 2026-08-11 — and left sitting in this backlog.
⚠️ **The consistency audit earlier the same day did not catch them because it never opened this file**: it
audited `Issue-active.md`, `Issue-Documentation.md` and the `Verified/` archives, and only *grepped*
`Issue-backlog.md`. **An audit scoped to the files that usually go stale will miss the file nobody looks at**
— which is exactly where a Verified Issue goes to be forgotten. I-0121's `rebalancedKeys` guard is now applied
and proven RED-then-GREEN under UBSan (516/516); its CI and coverage halves remain open. Prior note follows.)*

*2026-08-16 (**I-0121 opened — ScriviCore CI has been red on every commit since 2026-07-30.**
`rebalancedKeys(1)` divides by zero (`OrderKey.cpp:179`): the ternary guards `n == 0` — already unreachable —
while the divisor is `n - 1`. ⚠️ **Invisible locally by hardware, not by luck**: arm64 returns 0 for integer
division by zero, x86-64 raises SIGFPE, so the developer Mac and the `macos-latest` runner are green while
`ubuntu-latest` crashes. Introduced by **1c42838**, confirmed by `git log -S`, matching the user's report to
the commit. **Assigned to EP-031, to be scoped at the next sprint's planning** (user ruling) together with
adding `-fsanitize=undefined` to CI, which may change the test configuration. Prior note follows.)*

*2026-08-11 (I-0112 opened, then **root cause corrected**. Filed as a suspected live-appearance-switch
staleness bug; the user disproved that within minutes — Dark Mode had been active for hours and the app was
launched minutes before the defect was seen, so no switch was involved. Confirmed cause is static: body-text
attribute dictionaries omit `.foregroundColor` (`:517`, `:296-298`) and `textColor` is never set, so AppKit
renders body runs as literal `NSColor.black` against an adaptive dark background. Manuscript-only, as the sole
AppKit text surface. Sprintless/unassigned. I-0017/I-0018/I-0058 unchanged.)*
