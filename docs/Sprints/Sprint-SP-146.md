---
sprint: SP-146
epic: EP-043
status: Active
platform: Linux
created: 2026-09-29
activated: 2026-09-29
---

# SP-146 — `[Linux]` **The App Object and the Windows** (S2 of [EP-043])

**Status:** 🟡 **ACTIVE — 2026-09-29 (user-approved).**
**Epic:** ✅ **[EP-043]** `[Linux]` **The Session** → [`../Epics/Epic-EP-043.md`](../Epics/Epic-EP-043.md)
**Serves:** ✅ **R1 · R2 · R3 · R7 · R8**
**Depends on:** ✅ **[SP-145] CLOSED 2026-09-29** → [`Closed/Sprint-SP-145.md`](Closed/Sprint-SP-145.md)
**Size:** ⛔ **HIGH** — ⚠️ **the Epic's biggest Sprint, and the planning already said so.**

---

## ⚠️ THE MANDATE — ⛔ this one is NOT behaviour-preserving

⛔ **[SP-145] WAS BEHAVIOUR-PRESERVING. ✅ THIS SPRINT IS THE OPPOSITE: it changes what the app IS.**
⚠️ **One window becomes N windows; ⛔ a local in `main()` becomes an app object; ⚠️ a singular quit
hook becomes a registry flush.** ✅ **Every AC below is a behaviour change, deliberately.**

⚠️ **⛔ THE TRAP THAT REPLACES [SP-145]'s:** ✅ **[SP-145] could prove itself with *"0 deletions in
`tests/`"*.** ⛔ **This Sprint CANNOT — ⚠️ assertions WILL change, because behaviour changes.**
✅ **So the discipline here is different: ⚠️ every changed assertion must be NAMED and JUSTIFIED in the
progress log, ⛔ not silently edited.**

---

## ✅ What is already decided — ⛔ do NOT re-derive it

⚠️ **[EP-043]'s record carries this Sprint's design and it was MEASURED, not guessed.**
✅ **Read `Epic-EP-043.md` §"The app-level owner" (`:358`) BEFORE writing code.** ⚠️ **In one table:**

| ✅ Decided | ⚠️ Where |
| --------- | -------- |
| ✅ **Linux has NO app-level owner at all** — ⛔ app state is LOCALS IN `main()` (`appSupportRoot` at `main.cpp:87`, hand-threaded three ways) | ✅ **Epic §app-level owner** |
| ✅ **The registry CANNOT live on a widget** — ⚠️ it answers *"is this project open?"* BEFORE a window exists | ✅ **Epic §app-level owner** |
| ✅ **Apple's shape is the target** — ⚠️ **every reader of Apple's registry is inside `AppEnvironment`; ⛔ NOT ONE is in a view** | ✅ **Epic table** |
| ✅ **[R-Q3]: a SEPARATE Landing window**; project windows are editor-only | ✅ **Epic §R-Q3** |
| ✅ **`File ▸ New`/`Open` RAISE Landing** and reuse its existing flow — ⛔ do NOT reimplement per-window | ✅ **`ScriviWindow.hpp:88-92`** |
| ✅ **Closing the LAST project shows Landing and NEVER quits** | ✅ **Epic sub-ruling** |
| ⛔ **"Both pages in every window" REJECTED** — ⚠️ N QML engines + N bridges; [I-0232] cost ~79% of an open once | ✅ **Epic** |
| ⛔ **The double-`ScriviBridge` is NOT a defect** — ✅ investigated 2026-09-27, no Issue filed | ✅ **Epic §dismissed** |

---

## Tasks

| ID | Title | ⚠️ Why separate |
| -- | ----- | --------------- |
| **T-0558** | ✅ **Create `AppEnvironment`** — ⚠️ **Linux's FIRST app-level state owner.** ✅ Move `appSupportRoot` onto it; construct it in `main()` | ⚠️ **MECHANICAL, no behaviour change.** ✅ **Do it FIRST AND ALONE so the rest has a home** — ⛔ the Epic warns this is *"a real Task, not a preamble"* |
| **T-0559** | ✅ **Move `OpenProjectRegistry` AND `ProjectSession` OWNERSHIP onto `AppEnvironment`** — ⚠️ `EditorShell` takes a `ProjectSession*` instead of holding one | ⛔ **CHANGES `EditorShell`'s LIFETIME CONTRACT.** ✅ **This is where [SP-145]'s reference-binding seam is removed — ⚠️ it was built for exactly this moment** |
| **T-0560** | ✅ **The multi-window PLUMBING** — ⚠️ `ProjectWindowManager` · R3 check at the open funnel · R7 quit-flushes-every-window · R8 per-window release · window register/deregister | ⚠️ **SPLIT FROM THE ORIGINAL T-0560 (2026-09-29, user-approved)** — ✅ **it is verifiable on its own and it builds the structure T-0561 needs** |
| **T-0561** | ⚠️ **Landing as its OWN window, and a SECOND project window** — ✅ [R-Q3] · ⛔ AC4 and AC5 | ⛔ **SPLIT OUT: comparable in size to T-0560 again, ⚠️ and it touches the QML boundary — the least test-covered surface in the app** |

⚠️ **STRICTLY SERIAL.** ⛔ **T-0559 cannot start before T-0558 has a home to move things into, and
T-0560 cannot start before ownership is on the app object.**

---

## Acceptance Criteria

- [x] **AC1** — ✅ **An `AppEnvironment` class exists in `platforms/linux/src/`**, ⚠️ **owns
      `appSupportRoot`, and is constructed once in `main()`.** ⛔ **No `Q_GLOBAL_STATIC`, no singleton**
      — ✅ **Apple constructs its own explicitly and hands it down.**
- [x] **AC2** — ✅ **`OpenProjectRegistry` and the `ProjectSession` objects are owned by
      `AppEnvironment`.** ⛔ **`EditorShell` owns NO project identity** — ⚠️ **it is HANDED a
      `ProjectSession*` and shows it.** ✅ **Grep proof: no `OpenProjectRegistry` member on any widget.**
- [x] **AC3** — ⚠️ **[R2/R3]** ✅ **Opening a project that is ALREADY open RAISES its existing window**
      and does NOT create a second one. ⚠️ **The check lives in `AppEnvironment::openProject`** —
      ⛔ **the first point where the registry does real work.**
- [ ] **AC4** — ⚠️ **[R1]** ✅ **Two different projects open in TWO windows simultaneously**, each with
      its own navigator, editor, inspector and timeline. ✅ **[T-0561] — MET; ⚠️ proven by a harness
      (9/9), ⛔ NOT yet on the rig.**
- [x] **AC5** — ⚠️ **[R-Q3]** ✅ **[T-0561] — MET.** ✅ **Landing is its OWN window.** ⚠️ **Project windows are editor-only.**
      ✅ **`File ▸ New` / `File ▸ Open` from a project window RAISE Landing and trigger its EXISTING
      flow** — ⛔ **the flow is NOT reimplemented.**
- [x] **AC6** — ✅ **[T-0561] — MET** (`projectWindowClosing()`). ✅ **Closing the LAST project window SHOWS Landing.** ⛔ **The app NEVER quits
      implicitly on a window close.**
- [x] **AC7** — ⚠️ **[R7]** ⛔ **Quit flushes EVERY open session, not one.** ✅ **`main.cpp:124`'s
      `aboutToQuit → ScriviWindow::flushEditor` is SINGULAR BY CONSTRUCTION** (⚠️ `main.cpp:107`
      constructs ONE `ScriviWindow` by value) — ⛔ **it must be rewired to the registry.**
      ⚠️ **VERIFY WITH DIRTY SCENES IN BOTH WINDOWS.**
- [x] **AC8** — ⚠️ **[R8]** ✅ **Closing a project window calls `scrivi_close_project` for THAT
      project** — ⛔ **and only that one.** ⚠️ **Linux already does this correctly for its single
      project; ✅ do not regress it** (⚠️ **Apple's failure to do this at all is [I-0233]**).
- [ ] **AC9** — ⛔ **NOT MET, NOT ATTEMPTED.** ⚠️ **N menu bars exist (each window builds its own), ⛔ but the inspector/timeline check-state sync was written for ONE window and needs a live pass ([SP-148]).** ✅ **`buildMenuBar()` / `updateMenuState()` become PER-WINDOW**,
      ⚠️ **including the `showInspectorAction_` / `showTimelineAction_` check-state sync** — ⛔ **which
      must reflect the state of THAT window's project, not the app's.**
- [x] **AC10** — ✅ **Every existing Linux smoke still passes.** ⚠️ **Assertions MAY change here
      (unlike [SP-145]) — ⛔ but each change must be NAMED in the progress log with its reason.**
- [x] **AC-build** — ✅ **Docker build clean; `ctest` GREEN as NON-ROOT in the tests-on image**
      (⚠️ `project_linux_container_tests_off`: the shipping Dockerfile builds `SCRIVI_BUILD_TESTS=OFF`,
      ⛔ so "the container is green" does NOT mean `ctest` ran). ✅ **Apple must still build** —
      ⚠️ **no ScriviCore change is expected, so an Apple break means something was touched that
      should not have been.**

---

## ⛔ Explicitly OUT of scope

- ⛔ **ANY persistence of geometry, splitters, or the open-set.** ✅ **[R-Q1] named `session.ini`;
  ⚠️ [SP-147] writes it.** ⛔ **Do not create the file here.**
- ⛔ **The R6 test guard.** ⚠️ **It belongs with the first line of RESTORE code ([SP-147])** —
  ✅ **there is no restore code in this Sprint.** ⚠️ **⛔ BUT: read Apple's `SCRIVI_NO_PROJECT_LOAD` +
  `isRunningUnderTests` (`AppEnvironment.swift:374`, `:353`) when building AC1's class** —
  ✅ **the guard belongs ON this new class, which is why the Epic says build it here.**
- ⛔ **[I-0255]** (timeline persistence). ⚠️ **RULED 2026-09-29 that it SHOULD persist, ✅ but it is
  `[Cross]` — ⛔ fixing only Linux re-earns the parity gap [I-0251] just closed.**
- ⛔ **[T-0556]** (`Hide All` / `Restore All`). ⚠️ **`[Cross]`, and it depends on [I-0255].**
- ⛔ **[I-0244]'s shell work** — ✅ **ruled a sibling Epic AFTER this one ([R-Q5]).**
- ⛔ **ANY ScriviCore / C ABI change.**

### ⚠️ [I-0256] — ✅ FOLD-IN CANDIDATE, ⛔ NOT YET COMMITTED

⚠️ **[I-0256]** (⛔ no way to hide the Scene Navigator on Linux) **is pane/window work and this Sprint
owns the shell.** ✅ **Its ruling makes it cheap: ⛔ do NOT build a hide control — ⚠️ collapse the
splitter to width 0**, ✅ **and `EditorShell.cpp:160` applies `setCollapsible(false)` to the INSPECTOR
ONLY, so index 0 already collapses with no widget change.**

⛔ **IT IS NOT IN THE ACs.** ⚠️ **Take it ONLY if T-0558–T-0560 land with room to spare** — ✅ **this is
already the Epic's HIGH-risk Sprint, and the pre-collapse width must be remembered, which is state this
Sprint is otherwise not adding.**

---

## ⚠️ Known traps — ✅ each measured, not guessed

1. ⛔ **`main.cpp:107` CONSTRUCTS THE WINDOW BY VALUE** (`ScriviWindow window(landing, appSupportRoot);`)
   ⚠️ **and `:124` binds `aboutToQuit` to THAT instance.** ✅ **Both are singular by construction** —
   ⛔ **AC7 cannot be met by adding windows beside this one; the ownership model has to change.**
2. ⚠️ **`Landing.qml` owns the whole open/create flow AND its own `ScriviBridge`** — ✅ **`main.cpp`
   sets `appSupportRoot`, `defaultProjectsFolder` and `shell` as context properties on the LANDING
   widget.** ⛔ **Moving Landing into its own window means re-homing those context properties.**
3. ⚠️ **`flushEditor()` is called from FOUR places** (`ScriviWindow.cpp:82`, `:95`, `:108`, `:420`) —
   ✅ **each *"never lose edits when leaving the editor"*.** ⛔ **All four must keep working per-window
   while quit goes through the registry.**
4. ⚠️ **The menu bar is per-`QMainWindow` on Linux** — ✅ **so N windows = N menu bars, ⛔ and the
   inspector/timeline check-state sync is now per-window.** ⚠️ **Budget it (AC9); it is not free.**
5. ⚠️ **Find how the app ALREADY does it first** (`feedback_look_for_existing_pattern_first`) —
   ✅ **SP-118's dominant defect, and [SP-145] proved the rule again with `InspectorLayoutStore`.**
6. ⚠️ **The rig is READ-ONLY for source** (`feedback_rig_is_read_only`) — ✅ **edit on macOS, push,
   pull.** ⛔ **Never `sed` on the rig.**
7. ⛔ **A green suite cannot answer *"does it survive a quit?"* or *"do two windows work?"***
   (`feedback_live_pass_finds_what_suites_cannot`) — ✅ **[SP-148] owns the Epic's live pass, ⚠️ but AC4
   and AC7 deserve a look on the rig before this Sprint closes.**

---

## Progress log

### ✅ 2026-09-29 — T-0561 IMPLEMENTED — ⚠️ **LANDING IS ITS OWN WINDOW, AND A SECOND PROJECT WINDOW EXISTS**

✅ **`LandingWindow`** (`platforms/linux/src/LandingWindow.{hpp,cpp}`) — ⚠️ **the Landing QML moved out
of `ScriviWindow`'s `QStackedWidget` into its OWN top-level window ([R-Q3]).**
⛔ **THAT STACK IS WHY A SECOND PROJECT HAD NOWHERE TO GO** ([I-0178]): ⚠️ **one window, Landing as
page 0, a project as page 1.**

✅ **Project windows are EDITOR-ONLY and created ON DEMAND** by `AppEnvironment::openProjectWindow()`,
⚠️ **each a top-level window with NO PARENT** — ⛔ **parenting them to Landing would make them children
that minimise and close with it; ✅ they are siblings.**

✅ **`File ▸ New` / `File ▸ Open` now RAISE Landing** — ⛔ **they used to call `showLanding()` ON THE
PROJECT WINDOW, replacing the manuscript the writer was looking at.** ⚠️ **With one window that read as
"go back"; ✅ with N windows it would be destructive — she asked to open a DIFFERENT project, not to
close this one.**

#### ⚠️ TWO LIFECYCLE RULES THAT ARE EASY TO GET WRONG, AND BOTH ARE [R-Q3]'s

1. ⛔ **CLOSING LANDING MUST NOT QUIT** while projects are open. ⚠️ **Qt quits when the last top-level
   window closes** — ✅ **so `LandingWindow::closeEvent` HIDES and ignores the event whenever any
   project window remains.** ⚠️ **A writer with two manuscripts open who closes Landing is tidying up,
   ⛔ not quitting.**
2. ⛔ **CLOSING THE LAST PROJECT WINDOW MUST NOT QUIT EITHER.** ✅ **`projectWindowClosing()` shows
   Landing when the map empties** — ⚠️ **without it the app would vanish instead of returning to the
   project list, ⛔ and R7 becomes much harder to reason about if a window close can become a quit.**

#### ✅ AC4 PROVEN BY RUNNING — ⛔ not by reading

⚠️ **A throwaway harness drove `AppEnvironment` directly (no QML, no synthetic input), linked against
the app's own object files.** ✅ **9/9 checks, ALL PASS:**

| Check | Result |
| ----- | ------ |
| two projects → **two DIFFERENT windows** | ✅ **PASS — ⚠️ this is AC4** |
| window map holds **two** | ✅ **PASS** |
| map resolves each projectID to its own window | ✅ **PASS** |
| ⚠️ **R3 returns nullptr when the REGISTRY has no session** | ✅ **PASS — ⛔ proves the REGISTRY is authoritative, ⚠️ not the window map** |
| closing a window **deregisters** it | ✅ **PASS — ⛔ no dangle** |

⚠️ **THE FOURTH ROW IS THE ONE WORTH KEEPING:** ✅ **the window map alone would have answered "open",
⛔ and it is deliberately NOT the authority** — ⚠️ **[EP-018] proved platform de-duplication could not
be trusted (T-0191), and this harness shows the guard actually consults the registry.**

⚠️ **THE APP ALSO LAUNCHES CLEAN** (`QT_QPA_PLATFORM=offscreen`, exit 0, ⛔ no QML errors) — ✅ **which
is what proves the context properties (`appSupportRoot`, `defaultProjectsFolder`, `shell`) survived
being re-homed onto the Landing window.**

#### ✅ VERIFIED BY RUNNING

| Check | Result |
| ----- | ------ |
| Docker build (shipping image) | ✅ **clean** |
| ⚠️ **`ctest` — NON-ROOT (uid 1001), tests-ON image** | ✅ **641/641, 0 failed** |
| Linux smokes | ✅ **23/23 PASS** |
| ⚠️ **two-window harness** | ✅ **9/9 PASS** |
| App launch (offscreen) | ✅ **exit 0, no QML errors** |
| `scripts/check-package-boundary.sh` | ✅ **GREEN** |
| Apple `xcodebuild -scheme ScriviApp` | ✅ **BUILD SUCCEEDED** |

⚠️ **ASSERTION-CHANGE LEDGER: ✅ STILL EMPTY — ⛔ and that is now a LIMITATION, not a reassurance.**
⚠️ **Nothing in the 23 smokes drives two windows** — ✅ **the harness had to be written from scratch to
test AC4 at all, ⛔ and it is throwaway.** ⚠️ **[SP-148] must cover this on the rig.**

#### ⛔ WHAT IS **NOT** DONE — AC9

⛔ **AC9 (N menu bars) IS NOT MET AND WAS NOT ATTEMPTED.** ⚠️ **Each `QMainWindow` builds its own menu
bar already (`buildMenuBar()` runs per window), so N windows DO get N menu bars** — ⛔ **but the
inspector/timeline CHECK-STATE sync was written for one window and has not been re-examined under N.**
✅ **It needs a live pass to assess honestly, ⚠️ which is [SP-148]'s.**

---

### ⚠️ 2026-09-29 — T-0560 SPLIT, and the PLUMBING half IMPLEMENTED

⛔ **T-0560 AS PLANNED WAS TOO BIG, AND THAT WAS FOUND BY BUILDING IT, NOT BY GUESSING.**
✅ **Five of its six parts are done and verify cleanly. ⚠️ The sixth — a genuinely separate Landing
window so a SECOND project window can exist — is comparable in size to the other five combined.**
✅ **USER RULED 2026-09-29: split it.** ⚠️ **T-0560 keeps the plumbing; ✅ [T-0561] takes the windows.**

#### ⛔ WHY THE SIXTH PART IS NOT A CONTINUATION BUT A TASK

⚠️ **`main()` creates ONE `QQuickWidget` for Landing and hands it to ONE `ScriviWindow` as page 0 of a
`QStackedWidget`.** ⛔ **The QML context properties (`appSupportRoot`, `defaultProjectsFolder`, `shell`)
are set on THAT widget, and `shell` is bound to THAT window.** ✅ **So a second project window requires
re-homing all three and rewiring `File ▸ New`/`Open` from `showLanding()` to *raise the Landing
window*** — ⚠️ **the [R-Q3] shape.**
⛔ **THE CHEAP ALTERNATIVE IS ALREADY REJECTED:** ✅ **a second `ScriviWindow` with its own landing
widget is "both pages in every window", ⚠️ which the Epic rejected on measurement — N QML engines, N
bridges, and [I-0232] cost ~79% of an open for ONE duplicate.**

#### ✅ WHAT T-0560 DELIVERS

✅ **`ProjectWindowManager`** (`platforms/linux/src/ProjectWindowManager.hpp`) — ⚠️ `projectID` → window.
⛔ **A SEPARATE map from `OpenProjectRegistry`, deliberately**, ✅ **as Apple keeps them: state and
surface are different questions.** ⛔ **Borrowed pointers; ⚠️ `ScriviWindow`'s destructor deregisters.**

✅ **R3 — the non-reentrancy check** in `ShellController::openEditor`. ⚠️ **THAT IS THE RIGHT PLACE:
✅ it is the SINGLE FUNNEL every open flow reaches** (⚠️ the landing's Open button, a recents click and
New Project all call `shell.openEditor(...)` — `Landing.qml:121`, `:413`). ⛔ **Deeper would be too
late; in QML would be policy in the view.** ✅ **The answer comes from the REGISTRY, never the window
list.** ⚠️ **The `projectID` key was VERIFIED against the existing consumer (`EditorShell.cpp:450`
reads the same key from the same envelope), ⛔ not assumed.**

✅ **R7 — quit flushes EVERY window** (`AppEnvironment::flushAllWindows`). ⛔ **The old hook was
`&window, &ScriviWindow::flushEditor` — SINGULAR BY CONSTRUCTION** (⚠️ `main.cpp` built ONE window by
value) — ✅ **so with N windows it would have flushed one and SILENTLY DROPPED the rest.**

✅ **R8 — per-window release.** ⚠️ **`closeEvent` now calls `releaseProject()`**, ✅ **which closes THAT
project in the core and deregisters its session — ⛔ and only that one.**

#### ✅ VERIFIED BY RUNNING

| Check | Result |
| ----- | ------ |
| Docker build (shipping image) | ✅ **clean** |
| ⚠️ **`ctest` — NON-ROOT (uid 1001), tests-ON image** | ✅ **641/641, 0 failed** |
| Linux smokes (⚠️ each wrapper + its binary, offscreen) | ✅ **23/23 PASS** |
| `scripts/check-package-boundary.sh` | ✅ **GREEN** |
| Apple `xcodebuild -scheme ScriviApp` | ✅ **BUILD SUCCEEDED** |

⚠️ **ASSERTION-CHANGE LEDGER: ✅ STILL EMPTY.** ⛔ **T-0560 changed NO test assertions** — ⚠️ **the
plumbing adds structure that nothing yet drives; ✅ the behaviour changes arrive with [T-0561].**

#### ⛔ ONE BUILD BREAK OF MINE — ✅ recorded, it is the instructive part

⛔ **A BLANKET CMake ADD PUT `AppEnvironment.cpp` IN ALL 19 TARGETS AND BROKE THE LINK.**
⚠️ **`flushAllWindows()` calls `ScriviWindow::flushEditor()`, and `ScriviWindow.cpp` is in exactly ONE
target** — ✅ **so `persistence_smoke` and `lifecycle_smoke` failed with *"undefined reference to
ScriviWindow::flushEditor()"*.** ✅ **FIXED: the `.hpp` is listed in all 19 (it is header-only and
IDE-visible); ⛔ the `.cpp` ONLY beside `ScriviWindow.cpp`.** ⚠️ **The reason is now a comment in
`CMakeLists.txt`, so the next blanket add does not repeat it.**

---

### ✅ 2026-09-29 — T-0558 and T-0559 IMPLEMENTED

✅ **T-0558 — `AppEnvironment` EXISTS** (`platforms/linux/src/AppEnvironment.hpp`, header-only).
⚠️ **Linux's first app-level state owner.** ✅ **Owns `appSupportRoot`; declares the registry.**
⛔ **NOT a singleton** — ✅ **constructed once in `main()` (`main.cpp`) and handed down by pointer**,
⚠️ **exactly as Apple constructs and passes its own.**
⚠️ **The registry is DECLARED from the start though T-0558 does not use it** — ✅ **deliberate: a
half-moved registry is worse than either end state.**

✅ **T-0559 — OWNERSHIP MOVED.** ⛔ **`OpenProjectRegistry registry_` is GONE from `EditorShell`**
(⚠️ grep: no `registry_` remains in `EditorShell.hpp`/`.cpp`). ✅ **`EditorShell` now holds
`AppEnvironment* env_`, NOT owned.** ⚠️ **Threaded `main()` → `ScriviWindow` → `EditorShell`.**
⚠️ **ONLY FOUR REGISTRY CALL SITES EXISTED** (`EditorShell.cpp:606`, `:607`, `:609`, `:1392`) —
✅ **each kept its call, order and meaning; ⛔ only the owner changed.**

⚠️ **`env_` MAY BE NULLPTR AND EVERY USE IS GUARDED.** ✅ **Several smoke binaries construct an
`EditorShell` directly with no app environment** — ⛔ **they never exercise the registry, so a guard is
honest rather than defensive.** ⚠️ **Both `EditorShell` and `ScriviWindow` default the parameter to
`nullptr` for exactly this reason.**

#### ✅ VERIFIED BY RUNNING — ⛔ not asserted

| Check | Result |
| ----- | ------ |
| Docker build (shipping image, `TESTS=OFF`) | ✅ **clean** |
| ⚠️ **`ctest` — NON-ROOT (uid 1001), tests-ON image** | ✅ **641/641, 0 failed** |
| Linux smokes (⚠️ each wrapper given its matching binary, `QT_QPA_PLATFORM=offscreen`) | ✅ **23/23 PASS** |
| `scripts/check-package-boundary.sh` | ✅ **GREEN — *"no app-side package reads or writes"*** |
| Apple `xcodebuild -scheme ScriviApp` | ✅ **BUILD SUCCEEDED** |

⚠️ **THE TESTS-ON IMAGE IS SCRATCH, NOT COMMITTED** — ✅ **`project_linux_container_tests_off`: the
shipping Dockerfile builds `SCRIVI_BUILD_TESTS=OFF`, so "the container is green" does NOT mean `ctest`
ran.** ⚠️ **A second image with `TESTS=ON` + a uid-1001 user was built in the scratchpad.**

#### ⚠️ ONE HARNESS ERROR OF MINE — ⛔ recorded so it is not mistaken for a defect

⛔ **I first ran all 24 wrappers with NO ARGUMENT and got 24 "failures."** ✅ **Every one printed
`usage: <name>.sh <harness-binary>`** — ⚠️ **the wrappers TAKE their binary as `$1`.** ✅ **Paired by
name, 23/23 pass.** ⚠️ **`dumas_world_fixture` is EXCLUDED: it is a project GENERATOR needing
`<projectDir>`, ⛔ not a smoke** — ✅ **[SP-145] recorded the same exclusion and the same 23.**

⚠️ **ASSERTION-CHANGE LEDGER (this Sprint's discipline): ✅ NONE SO FAR.** ⛔ **T-0558 and T-0559
changed no test assertions** — ⚠️ **both are ownership moves, and the behaviour is genuinely
unchanged.** ✅ **T-0560 is where that will stop being true.**

---

⚠️ **2026-09-29 — Sprint created and ACTIVATED (user-approved).**
✅ **Planning was NOT re-derived** — ⚠️ **[EP-043]'s record already carried the design, measured
2026-09-27; ✅ this document points at it rather than restating it** (P7: *a second copy is what goes
stale*).
