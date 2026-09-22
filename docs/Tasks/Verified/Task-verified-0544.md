# Verified Task: T-0544

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0544** | ✅ **[I-0243]** — ⚠️ **the project title rendered THREE times; leave ONE** | ⚠️ **none — [EP-040] direct** | [EP-040] | **2026-09-22** |

✅ **VERIFIED by the user's live pass 2026-09-22:** *"I can verify the triplication is removed, the tab
bar is removed, and file open when fullscreen now opens a new window."*

---

## ⚠️ The defect

⛔ **ONE FACT, THREE INDEPENDENT MECHANISMS**, which is why no single deletion fixed it:

| # | Where | Source |
| - | ----- | ------ |
| 1 | the **tab header** lozenge | `window.title` (AppKit) |
| 2 | the **nav bar** title | `.navigationTitle(projectTitle)` (SwiftUI) |
| 3 | pinned above the **scene list** | `Text(prefs.projectTitle)` — ⚠️ **did not scroll, so always visible** |

⚠️ **FOUND BY THE USER'S LIVE PASS ON [SP-134]** — ✅ **adding a toolbar made it obvious.**

## ⚠️ The tab bar was never a decision

⛔ **`tabbingMode` was NEVER SET ANYWHERE**, ✅ **so macOS's default `.automatic` applied** —
⚠️ **which tabs windows IN FULL SCREEN and not outside it.** ⛔ **The result was ONE COMMAND doing TWO
DIFFERENT THINGS: *Open Project* made a new WINDOW when windowed and a new TAB when full-screen.**
✅ **Nobody chose that; it was the platform showing through an unset property.**

## ✅ The ruling — OPTION A, windows only (user, 2026-09-22)

✅ **`window.tabbingMode = .disallowed`.** ⚠️ **Full screen now behaves like windowed, and [EP-018]'s
per-window model is the only model.**

✅ **WHY:** ⚠️ **a tab bar teaches *"these are peers within one workspace"*, which is what a
MULTI-document app looks like** — ⛔ **and the app-shape study ruled Scrivi a SINGLE-DOCUMENT editor**
(⚠️ **one `NSTextView` holds the whole manuscript; a scene is a structural unit of the document, not a
document**). ✅ **One manuscript per window says that honestly.**

⚠️ **THIS IS NOT A REVERSAL OF [EP-018], and the distinction matters:** ✅ ***"one manuscript"* and
*"one window"* are DIFFERENT CLAIMS.** ⚠️ **EP-018 (closed, verified) deliberately delivered
*"multiple distinct projects open at once, each in its own window"*** — ⛔ **tabs were a THIRD way of
showing multiple projects, arriving by ACCIDENT beside the two that were designed.**

## ✅ What shipped

- ✅ **`window.tabbingMode = .disallowed`** (`ProjectWindowManager.swift`) — ⛔ **removes source (1).**
- ⛔ **`projectHeader` DELETED** (`SceneNavigatorView.swift`) — ✅ **a TOMBSTONE marks the site** —
  ⛔ **removes source (3).**
- ✅ **`.navigationTitle` survives as the ONE title.**

⚠️ **THE ESTIMATE WAS "one or two lines" AND THE EDITS ARE** — ⛔ **but `projectHeader` was the ONLY
READER of `SceneNavigatorView.prefs`.** ✅ **RULED: KEEP `prefs`** — ⚠️ **removing it means changing the
initialiser and BOTH platform call sites in `EditorView`, which is churn beyond this Issue's scope**,
✅ **and a navigator that knows its project's preferences is a plausible near-term need.**
✅ **The build is clean with no unused-property warning.**

## How it was verified

✅ **USER LIVE PASS 2026-09-22 — all three claims confirmed:** ✅ **the triplication is gone** ·
✅ **the tab bar is gone** · ✅ ***Open Project* in full screen now opens a NEW WINDOW.**
✅ **Build clean · `TEST SUCCEEDED` 132/132 in 12 suites · `ctest` 626/626 · boundary guard clean.**

⚠️ **MY OWN RUNNING-APP CHECK WAS BROKEN AND GAVE A FALSE ALL-CLEAR.** ⛔ **`pgrep -c "Scrivi.app"`
returns 0 WHILE THE APP RUNS** — ✅ **`-c` matches the process NAME, not the command line.**
✅ **CORRECT: `pgrep -f "Scrivi.app/Contents/MacOS"`.** ⚠️ **Three test runs in this Epic were blocked
by a running app; twice I had a check that should have caught it and did not.**

---

*Verified 2026-09-22 by the user's live pass. [EP-040] AC1 is now fully met.*
