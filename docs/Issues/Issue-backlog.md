# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

**Currently: 2.** I-0017 and I-0018 are 🔴 Open. Neither is assigned to a Sprint.

⚠️ **Verified Issues do not belong in this file.** I-0058 (Verified 2026-07-09) and I-0112 (Verified
2026-08-11) sat here for weeks after verification and were archived to `Verified/` on 2026-08-16. Archive an
Issue in the same step it is verified.

✅ **I-0121 and I-0122 archived 2026-08-17**, in the same step SP-106 closed — to
[`Verified/Issue-verified-0121-0130.md`](Verified/Issue-verified-0121-0130.md). Both were Verified with the
sprint. Their full entries were removed from this file rather than left behind, per the rule above.

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


---

*Last Updated: 2026-08-17 (**I-0121 and I-0122 ✅ Verified and archived** to
`Verified/Issue-verified-0121-0130.md` at the SP-106 close — both full entries removed from this file in the
same step, which is the discipline the 2026-08-16 note below says was missed twice. **Backlog is now 2:**
I-0017 and I-0018, both 🔴 Open and unassigned. Prior note follows.)*

*2026-08-16, later same day (**I-0058 and I-0112 archived to `Verified/`; I-0121's code fix
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
