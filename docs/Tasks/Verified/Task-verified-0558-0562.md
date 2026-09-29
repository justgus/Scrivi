# Verified Tasks — T-0558 · T-0559 · T-0560 · T-0561 · T-0562

**Sprint:** ✅ **[SP-146]** → [`../../Sprints/Closed/Sprint-SP-146.md`](../../Sprints/Closed/Sprint-SP-146.md)
**Epic:** ✅ **[EP-043]** `[Linux]` **The Session** — ⚠️ **S2 of 4**
**Platform:** `[Linux]`
**Implemented:** 2026-09-29 · ✅ **USER-VERIFIED 2026-09-29 by live pass on the rig**
**Archived:** 2026-09-29, in the same step [SP-146] closed (`feedback_archive_on_close`).

---

## ✅ The verification

✅ **THE USER, 2026-09-29:** ***"I tested it on the rig and it passed."***

⚠️ **AND THE PASS BEFORE IT DID THE REAL WORK.** ✅ **The user's FIRST rig pass confirmed AC9 and found
[I-0257]** — ⛔ **a quit that left windows open non-deterministically** — ⚠️ **which no suite could have
reported: ✅ nothing in the 23 smokes drives two windows.**

---

## The Tasks

| ID | Title | ✅ Status |
| -- | ----- | --------- |
| **T-0558** | ✅ **Create `AppEnvironment`** — ⚠️ **Linux's FIRST app-level state owner**; `appSupportRoot` moves onto it | ✅ **VERIFIED** |
| **T-0559** | ✅ **Move `OpenProjectRegistry` OWNERSHIP onto it** — ⚠️ `EditorShell` holds a non-owning `AppEnvironment*` | ✅ **VERIFIED** |
| **T-0560** | ✅ **The multi-window PLUMBING** — `ProjectWindowManager` · R3 · R7 · R8 | ✅ **VERIFIED** |
| **T-0561** | ✅ **Landing as its OWN window + a SECOND project window** ([R-Q3]) | ✅ **VERIFIED** |
| **T-0562** | ⛔ **[I-0257]** — ⚠️ **`File ▸ Quit` left windows open**; ✅ explicit `quitApplication()` | ✅ **VERIFIED** |

⚠️ **T-0560 WAS SPLIT MID-SPRINT (user-approved).** ⛔ **As planned it was too big — ✅ and that was
found by BUILDING it, not by guessing.** ⚠️ **Five of its six parts verified cleanly; the sixth became
[T-0561].**

---

## ⚠️ What this Sprint actually changed

⛔ **BEFORE: ONE window, with Landing as page 0 of a `QStackedWidget` and a project as page 1.**
✅ **That stack IS why a second project had nowhere to go ([I-0178]).**

✅ **AFTER: an app-level owner, a separate Landing window, and N editor-only project windows.**

| ⚠️ Concern | ⛔ Before | ✅ After |
| ---------- | -------- | ------- |
| App-global state | ⛔ **LOCALS IN `main()`**, hand-threaded three ways | ✅ **`AppEnvironment`, constructed once, handed down** |
| *"Is this project open?"* | ⛔ **a registry on a WIDGET** | ✅ **the app's registry — ⚠️ authoritative, consulted before a window exists** |
| Landing | ⛔ **page 0 of the one window** | ✅ **its own window** |
| A second project | ⛔ **IMPOSSIBLE** | ✅ **its own window** |
| Quit | ⛔ **flushed ONE window; ⚠️ later left windows open** | ✅ **flushes every session, then tears down deterministically** |

---

## ⛔ [I-0257] — the defect the live pass found, and why a suite could not

⚠️ **USER:** ***"sometimes only closes one or two of the open windows… its not 'last clicked' and its
not 'last opened'."*** ✅ **THE "NO PATTERN" WAS THE CLUE — ⛔ it pointed at an ordering race, not a rule.**

| # | ⛔ Cause | ✅ Measured |
| - | -------- | ---------- |
| **1** | ⛔ **`QApplication::quit()` DOES NOT CLOSE WINDOWS** — ⚠️ it exits the event loop | ⚠️ **2 of 2 project windows left standing** |
| **2** | ⛔ **`closeAllWindows()` walks a SNAPSHOT**, ⚠️ and [T-0561]'s handlers MUTATE the window set mid-walk | ⚠️ **closed 1 of 2** |

✅ **FIX: flush FIRST (R7), then close from the app's OWN map — ⛔ not Qt's snapshot** — ⚠️ **with a
`quitting_` flag suppressing the Landing re-show and Landing's close-ignore.**
⚠️ **ALL THREE quit paths route through it.** ⛔ **The Landing QML's Quit button had the SAME defect**
— ✅ **the user reported it as working, ⚠️ and it usually is, ⛔ but only because Landing is typically
the LAST window, so `quit()` closing nothing is invisible.**
✅ **RE-MEASURED: 4 project windows + Landing, 5 runs, ZERO left open every time.**

---

## ✅ What was RUN

| Check | Result |
| ----- | ------ |
| ⚠️ **`ctest` — NON-ROOT (uid 1001), tests-ON image** | ✅ **641/641, 0 failed** |
| Linux smokes (⚠️ each wrapper + its binary, offscreen) | ✅ **23/23 PASS** |
| ⚠️ **two-window harness** (throwaway) | ✅ **9/9 PASS** |
| ⚠️ **quit harness, 4 windows × 5 runs** (throwaway) | ✅ **5/5 all closed** |
| App launch (offscreen) | ✅ **exit 0, no QML errors** |
| `scripts/check-package-boundary.sh` | ✅ **GREEN** |
| Apple `xcodebuild -scheme ScriviApp` | ✅ **BUILD SUCCEEDED** |
| ✅ **LIVE PASS ON THE RIG** | ✅ **PASSED** |

⚠️ **NO TEST ASSERTION WAS CHANGED IN THE ENTIRE SPRINT** — ⛔ **and that is a LIMITATION, not a
reassurance.** ✅ **Nothing in the 23 smokes drives two windows**, ⚠️ **so both harnesses had to be
written from scratch and are throwaway.** ⛔ **The two-window and quit paths have NO standing
regression guard.**

---

## ⚠️ Two errors of mine, recorded because they are the instructive part

1. ⛔ **A BLANKET CMake ADD broke the link.** ⚠️ **`AppEnvironment.cpp` went into all 19 targets, but
   `flushAllWindows()` calls `ScriviWindow::flushEditor()` and `ScriviWindow.cpp` is in exactly ONE.**
   ✅ **The `.hpp` belongs in all 19; ⛔ the `.cpp` only beside `ScriviWindow.cpp`.** ✅ **The reason is
   now a comment in `CMakeLists.txt`.**
2. ⛔ **`ctest` "failed" 40+ ScriviCore tests with *"No space left on device"*.** ⚠️ **NOT a code
   defect: my own repeated image builds had filled Docker's disk.** ✅ **Pruned; the SAME binary then
   passed 641/641** — ⚠️ **which is what proved it was the disk.**

---

## ⚠️ What this Sprint did NOT do

⛔ **NO persistence.** ✅ **[R-Q1] named `session.ini`; ⚠️ [SP-147] writes it.** ⛔ **No geometry, no
splitters, no open-session manifest, no launch restore.**
⚠️ **So today's app opens N windows and remembers NOTHING about them across a quit** — ✅ **which is
[SP-147]'s entire job.**
⛔ **The R6 test guard is [SP-147]'s too** — ⚠️ **it belongs with the first line of restore code, and
✅ Apple's `SCRIVI_NO_PROJECT_LOAD` + `isRunningUnderTests` (`AppEnvironment.swift:374`, `:353`) is the
precedent to read first.**
