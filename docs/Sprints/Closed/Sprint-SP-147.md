---
sprint: SP-147
epic: EP-043
status: Closed
platform: Linux
created: 2026-09-29
activated: 2026-09-29
closed: 2026-09-30
---

# SP-147 — `[Linux]` **The Persistence** (S3 of [EP-043])

**Status:** ✅ **CLOSED 2026-09-30 (user-approved).** ✅ **All ACs met on the rig** — ⚠️ AC5's window POSITION
is RULED out on Wayland ([I-0264]). ✅ **[EP-043] S3 of 4 COMPLETE.**
**Epic:** ✅ **[EP-043]** → [`../Epics/Epic-EP-043.md`](../Epics/Epic-EP-043.md)
**Serves:** ✅ **R4 · R5 · R6**
**Depends on:** ✅ **[SP-146] CLOSED 2026-09-29** → [`Closed/Sprint-SP-146.md`](Closed/Sprint-SP-146.md)
**Size:** ✅ **MEDIUM**

---

## ⚠️ What this Sprint is for

⛔ **TODAY THE APP OPENS N WINDOWS AND REMEMBERS NOTHING ABOUT THEM.** ✅ **[SP-146] made multiple
project windows possible; ⚠️ this makes them come back.**

✅ **Three things persist, and they are ONE file:** ⚠️ **which projects were open, where each window
was, and how each window's splitters were proportioned.**

⚠️ **AND ONE THING MUST **NOT** HAPPEN: ⛔ a test, smoke, or headless run must NEVER restore the
writer's real windows.** ✅ **That is R6, and it is NOT deferrable** — ⚠️ **[I-0150] is what it pays for:
on Apple, `xcodebuild test` LAUNCHED the app and rewrote a real project.**

---

## Tasks

| ID | Title | ⚠️ Why separate |
| -- | ----- | --------------- |
| **T-0565** | ✅ **`SessionStore`** — ⚠️ `session.ini` via `QSettings`; read/write the open set, per-project geometry and splitter state | ✅ **Pure state, NO window code** — ⚠️ testable on its own, ⛔ which the restore path is not |
| **T-0566** | ✅ **Save on close/quit + the R6 guard** — ⚠️ record the open set and each window's geometry; ⛔ **the guard lands WITH the first line of restore code** | ⚠️ **Writing is safe on its own; ✅ restoring is what can hurt, so the guard must exist before it** |
| **T-0567** | ✅ **Restore at launch** (R4/R5) — ⚠️ reopen each recorded project, skipping any whose path no longer resolves | ⛔ **THE RISKY ONE.** ✅ Separable because T-0565/T-0566 stand without it |

⛔ **STRICTLY SERIAL.** ⚠️ **T-0567 must not land before T-0566's guard.**

---

## Acceptance Criteria

- [x] **AC1** — ✅ **A `SessionStore` owns `<appSupportRoot>/session.ini` via `QSettings`**
      (⚠️ **`QSettings::IniFormat`, [R-Q1]**), ⛔ **and nothing else reads or writes that file.**
- [x] **AC2** — ⚠️ **[R-Q2]** ✅ **Keyed `[project/<projectID>]`, with `path` as a stored ATTRIBUTE** —
      ⛔ **NOT keyed by path.** ⚠️ **A moved project is skipped for one launch and KEEPS its geometry.**
- [x] **AC3** — ✅ **Every write calls `settings.sync()`.** ⚠️ **Mirrors `onTimelineViewStateChanged`
      (`EditorShell.cpp:2906`) and its stated reason: ⛔ *"a --rm container may SIGKILL before Qt's lazy
      flush."***
- [x] **AC4** — ⚠️ **[R4]** ✅ **On relaunch, every project open at quit is reopened**, ⛔ **skipping any
      whose path no longer resolves** (⚠️ **and a skip must NOT delete its record — R-Q2**).
      ⚠️ **Closes [I-0176].**
- [x] **AC5** — ⚠️ **[R5]** ✅ **A restored window returns to its size, position and maximized state**,
      ✅ **AND its splitter proportions.** ⚠️ **Keyed BY PROJECT** — ⛔ **[EP-018] hit exactly this: one
      global autosave frame made every restored window stack at the default.** ⚠️ **Closes [I-0177].**
- [x] **AC6** — ⛔ **[R6] THE GUARD.** ✅ **A test, smoke or headless/offscreen run NEVER restores the
      writer's windows**, ⚠️ **and the manifest is left INTACT while restore is suppressed** —
      ⛔ **suppressing must not be able to LOSE the open set.**
- [x] **AC7** — ✅ **The guard is PROVEN BY BREAKING IT.** ⚠️ **A smoke asserts restore is suppressed;
      ⛔ removing the guard must make it go RED.** ✅ **Same discipline as [T-0553]'s and [T-0564]'s.**
- [x] **AC8** — ⚠️ **No `QFile`/`QSaveFile` added under `platforms/linux/src/`.**
      ✅ **`scripts/check-package-boundary.sh` stays GREEN** — ⚠️ **`session.ini` is APP state under
      app-support, ⛔ NOT project data, so it does not cross the package boundary.**
- [x] **AC-build** — ✅ **Docker build clean; `ctest` GREEN as NON-ROOT in the tests-on image**
      (⚠️ `project_linux_container_tests_off`). ✅ **Apple must still build.**

---

## ✅ R6 — ⛔ APPLE'S GUARD IS THE PRECEDENT. DO NOT INVENT A THIRD MECHANISM.

⚠️ **The Epic is explicit that this must be READ first** (`feedback_look_for_existing_pattern_first`).
✅ **READ 2026-09-29 — `Scrivi/App/AppEnvironment.swift:353-375`:**

| Apple mechanism | ⚠️ What it does |
| --------------- | --------------- |
| ✅ **`isRunningUnderTests`** | ⚠️ checks `XCTestConfigurationFilePath`, `XCTestBundlePath`, `XCTestSessionIdentifier` |
| ✅ **`suppressProjectRestore`** | ⚠️ `SCRIVI_NO_PROJECT_LOAD` — ⛔ **ENV VAR ONLY** |
| ✅ **both are stored `let`** | ⚠️ *"the decision must not change underneath the app mid-run"* |

⛔ **THE LOAD-BEARING PROPERTY, AND IT IS THE ONE TO COPY:** ✅ **Apple *"suppresses the RESTORE only…
the open-session manifest is left UNTOUCHED, so this cannot lose the writer's windows."***
⚠️ **A guard that CLEARED the manifest would be worse than no guard: ⛔ one test run would erase the
writer's session.**

⚠️ **THE LINUX SHAPE — ✅ same two questions, ⛔ different signals:**
- ✅ **`SCRIVI_NO_RESTORE=1`** — ⚠️ the operator's switch. ✅ **An env var, like Apple's** (⛔ and
  `[I-0201]`'s launch-argument hazard is Apple-specific, ⚠️ but an env var is simpler regardless).
- ✅ **`QT_QPA_PLATFORM=offscreen`** — ⚠️ **every smoke wrapper already exports it**, ✅ so a headless
  run is self-identifying with NO new plumbing.
- ⛔ **`XDG_DATA_HOME` redirection is REJECTED** — ⚠️ **[SP-145] floated it; ✅ the Epic already calls
  Apple's guard *"a better-specified precedent."*** ⛔ **Redirection makes the guard depend on the test
  harness remembering to redirect, ⚠️ and a harness that forgets touches the real file.**

---

## ⛔ Explicitly OUT of scope

- ⛔ **[I-0255]** (timeline visibility persistence). ⚠️ **RULED to persist, ✅ but `[Cross]`** —
  ⛔ **fixing only Linux re-earns the parity gap [I-0251] closed.**
- ⛔ **[T-0556]** (`Hide All` / `Restore All` focus mode). ⚠️ **`[Cross]`, and it depends on [I-0255].**
- ⛔ **Navigator visibility persistence.** ✅ **[T-0563] left it session-scoped DELIBERATELY** —
  ⚠️ **Apple's `columnVisibility` is `@State`, so persisting Linux's alone invents a parity gap.**
- ⛔ **[I-0244]'s shell work** — ✅ **a sibling Epic AFTER this one ([R-Q5]).**
- ⛔ **ANY ScriviCore / C ABI change.**

---

## ⚠️ Known traps

1. ⛔ **THE GUARD IS NOT A CLEANUP.** ⚠️ **Suppressing restore must leave `session.ini` INTACT** —
   ✅ **Apple says so explicitly, and a guard that clears it would lose the writer's session on the
   first test run.**
2. ⛔ **A SKIPPED PROJECT MUST KEEP ITS RECORD.** ⚠️ **[R-Q2]: a moved project is skipped for ONE
   launch and keeps its geometry** — ✅ **deleting the row on a failed resolve would punish a writer
   who moved a folder.**
3. ⚠️ **`settings.sync()` ON EVERY WRITE.** ⛔ **The container SIGKILLs before Qt's lazy flush** —
   ✅ **`EditorShell.cpp:2906` already carries this habit and says why.**
4. ⚠️ **GEOMETRY IS PER PROJECT, NOT GLOBAL.** ⛔ **[EP-018] shipped one autosave frame once and every
   restored window stacked at the default.**
5. ⚠️ **RESTORE OPENS WINDOWS CONCURRENTLY** — ✅ **which is precisely the race
   `OpenProjectRegistry`'s header says it exists for.** ⛔ **Ask the registry, never the window list.**
6. ⚠️ **The rig is READ-ONLY for source** (`feedback_rig_is_read_only`).
7. ⛔ **A green suite cannot answer *"does it survive a quit?"*** — ✅ **[SP-148] owns the live pass,
   ⚠️ but AC4/AC5 deserve a rig look before this Sprint closes.**

---

## Progress log

### ✅ 2026-09-30 — CLOSED (user-approved)

✅ **Close approved by the user 2026-09-30** after the final rig pass: [I-0264] (Wayland: no `0,0` recorded) and
[I-0265] (Landing size) — *"they pass."* ✅ **[T-0565], [T-0566], [T-0567] VERIFIED** →
`../../Tasks/Verified/Task-verified-0565-0567.md`. ✅ **[I-0264], [I-0265] VERIFIED** →
`../../Issues/Verified/Issue-verified-0261-0270.md`. ✅ **Closes [I-0176] and [I-0177]'s substance on Linux** —
⚠️ their records are [SP-148]'s AC sweep to reconcile.
⚠️ **Carried forward, not in scope here:** [I-0255] (Timeline visibility persistence, `[Cross]`); the desktop-logout
path (untested); Landing staying up beside restored windows (Apple dismisses its Welcome — a parity question).
➡️ **[SP-148] (S4, verification) is NEXT.**


### ✅ 2026-09-30 — THE RIG PASS (build on the rig, GNOME **Wayland** session over RDP)

| # | Test | Result |
| - | ---- | ------ |
| 1 | Projects open at Quit reopen | ✅ PASS |
| 2 | Size / position / splitters per window | ✅ size, maximized, splitters — ⛔ **POSITION NOT RESTORED** → [I-0264] |
| 3 | Maximized restores maximized ([I-0177]'s own symptom) | ✅ PASS |
| 4 | Closed-before-quit does not return; reopening it keeps its size | ✅ PASS |
| 5 | Project on an unplugged drive skipped, returns with its geometry | ✅ PASS — *"through multiple restarts"* |
| 6 | `SCRIVI_NO_RESTORE=1` → Landing only; nothing lost | ✅ PASS |
| 7 | Hidden pane → sane layout | ✅ PASS (inspector) |

⚠️ **[I-0264] — CAUSE ESTABLISHED:** the session is **Wayland** (`loginctl` `Type=wayland`) and every saved
frame reads `0,0,W,H`: ⛔ a Wayland client can neither learn nor set its window position; GNOME centres
new windows. ⚠️ A platform limit, not a T-0567 defect — **ruling needed**.
⚠️ **[I-0265] (new):** the LANDING window's size is never remembered (Apple's Welcome is).
✅ **Expected, not defects:** a hidden Timeline or Navigator reappears after relaunch — Timeline
visibility persistence is [I-0255] (`[Cross]`, out of scope), and Navigator visibility is session-scoped
by ruling (T-0563). ✅ Both came back at their last saved size.
✅ **RULINGS (user, same day):** [I-0264] **(a) ACCEPT** position on Wayland; [I-0265] **INTO [SP-147]**; hidden
Timeline/Navigator reappearing is **expected**.
✅ **[T-0565], [T-0566] AND [T-0567] VERIFIED and ARCHIVED** → `../Tasks/Verified/Task-verified-0565-0567.md`.

### ✅ 2026-09-30 — [I-0264] + [I-0265] IMPLEMENTED

✅ **[I-0264]:** `AppEnvironment::recordableFrame` — when the platform does not report position (Wayland),
keep the PREVIOUSLY stored position and take only the new size, ⛔ so the meaningless `0,0` never overwrites a
real X11 position. Used by project windows and Landing.
✅ **[I-0265]:** `SessionStore` `[landing]` group (frame + maximized); `LandingWindow::showRestored()` at launch,
`recordGeometry()` on every close path (hide / close / quit).
✅ `ctest` **645/645** NON-ROOT; smokes **25/25**; new checks **proven red** (Wayland rule broken →
1 FAIL; `recordLanding` disabled → 3 FAIL); `check-package-boundary.sh` GREEN.
⚠️ **Awaiting the rig:** resize/maximize Landing → quit → relaunch; project-window sizes still restore.

---

### ✅ 2026-09-30 — T-0567 IMPLEMENTED (restore at launch) — ✅ SPRINT IMPLEMENTATION COMPLETE

✅ **`AppEnvironment::restoreSession()`, called ONCE from `main()`** after Landing and the shell
controller exist. ⚠️ **Each project goes through `openProjectWindow()` — the SAME funnel as a Landing
open**, ⛔ no second open path.

| Piece | ⚠️ What it does |
| ----- | --------------- |
| ✅ **`resolvableProjectsToRestore()`** | ⚠️ The guarded set (through `projectsToRestore()`, ⛔ so R6 still holds), minus paths that do not resolve. ⛔ **A skip writes nothing** — [R-Q2] |
| ✅ **No envelope on restore** | ⚠️ Each editor performs the project's ONE open (the reload path) — ⛔ opening first would re-earn [I-0232]'s double open |
| ✅ **Geometry on EVERY open** | ⚠️ `openProjectWindow()` applies the project's saved frame, maximized state and splitters **whenever the identity is known** — ✅ **Apple's shape** (`ProjectWindowFrameStore`, I-0051), ⛔ not restore-only |
| ✅ **`clampedOnscreen()`** | ⚠️ Apple's rule ported as-is: keep a frame overlapping a screen by ≥ 80×80, else re-centre on the primary |
| ✅ **Splitters: all-positive or nothing** | ⚠️ **Resolves T-0566's surfaced item 1:** a pane HIDDEN at close recorded a 0; ⛔ applying it to a now-visible pane would collapse it, so such a list is ignored and defaults stay |
| ✅ **`title` in `session.ini`** | ⚠️ Restore has no Landing envelope to take a title from |
| ⚠️ **`pendingOpens_` — R3 while a load is in flight** | ⛔ **The registry learns a project only when its load FINISHES.** A writer clicking a recent while restore is still loading that same project would get a SECOND window on it — two editors on one project. ✅ An open whose identity is known up front is held until it settles; `existingWindowFor()` consults it. ⚠️ **This also closes the same gap for ordinary Landing opens** |

⚠️ **A project that is no longer `ready` (needs repair) FAILS its restore load and its window closes
back to Landing** — ⛔ **restore NEVER repairs without the writer's consent**; the repair prompt lives on
Landing. ⚠️ Its record is untouched (the failed load never gets an identity), so it is retried next launch.

#### ✅ VERIFIED BY RUNNING — the smokes, AND the real app under Xvfb

⚠️ **The smokes CANNOT exercise the reopen — by design:** every smoke is `offscreen`, which is exactly
what R6 suppresses. ✅ So the **real `scrivi_linux`** was run under **Xvfb (no `offscreen`)** in a
throwaway container, against two generated fixture projects and a hand-seeded `session.ini`
(⛔ never real work — `feedback_never_drive_synthetic_input_at_real_work`):

| Step | Observed |
| ---- | -------- |
| Launch 1 — seeded Alpha `900x650+50+60`, Beta `800x600+1000+100`, `gone-id` at a missing path | ✅ **Both windows at EXACTLY those frames**; ✅ `skipping gone-id — path does not resolve; record kept` |
| Ctrl+W on Beta | ✅ Beta `open=false`, ✅ its geometry and splitters RECORDED |
| Ctrl+Q | ✅ Alpha stays `open=true` (the quit freeze); ✅ **seeded splitters APPLIED**: navigator 200 / inspector 250 / timeline 150 (⚠️ defaults are 240 / 400 / 120) |
| Launch 2 — `SCRIVI_NO_RESTORE=1` | ✅ **Landing only**; ✅ **`session.ini` BYTE-IDENTICAL** afterwards |
| Launch 3 | ✅ **Alpha ONLY**, at `900x650+50+60`; ✅ `gone-id` record still present |

| Check | Result |
| ----- | ------ |
| ⚠️ **`ctest` — NON-ROOT (uid 1001)** | ✅ **641/641** |
| Linux smokes | ✅ **25/25** — ⚠️ `restore_guard_smoke` now also covers **AC4** (skip + record kept); `session_store_smoke` covers `title` and **`setOpen` directly** (⚠️ T-0566 added it untested) |
| Docker build | ✅ **clean** (⚠️ only the pre-existing `DeterministicUUIDProvider.hpp` warning) |
| `check-package-boundary.sh` | ✅ **GREEN** |
| Apple `xcodebuild build` | ✅ **SUCCEEDED** (no Apple source touched) |

⚠️ **WHAT IS STILL UNPROVEN — for the rig (trap 7, [SP-148]):**
1. ⛔ **MAXIMIZED RESTORE.** ⚠️ Xvfb has no window manager, so maximize could not be observed.
   ✅ Code path: `setGeometry(normal frame)` then `showMaximized()`. ⚠️ **[I-0177]'s own symptom.**
2. ⚠️ **A HUNG network mount at launch.** `QFileInfo::exists` is a synchronous stat on the UI thread;
   ✅ an absent path or unplugged drive answers at once, ⛔ a hung mount would block launch
   ([I-0193]'s class). Unmeasured.
3. ⚠️ **Desktop logout** (carried from T-0566): a close that bypasses `quitApplication()` marks each
   window closed.
4. ⚠️ **Landing stays up beside restored windows.** ⚠️ **Apple DISMISSES Welcome when a project opens**
   (`WelcomeWindowRoot`); ⛔ Linux never has, for ANY open ([SP-146] shipped it that way). ✅ Not changed
   here — **a parity gap to rule, not a T-0567 regression.**
5. ⚠️ **Both project windows are titled `Scrivi — Linux (alpha)`**, not by project — pre-existing, seen
   in `xwininfo` during the run.

---

### ✅ 2026-09-30 — T-0566 IMPLEMENTED (save on close/quit + the R6 guard)

✅ **`AppEnvironment` now OWNS the `SessionStore`** (`platforms/linux/src/AppEnvironment.hpp`) —
⛔ **the only owner of `session.ini` (AC1).**

⚠️ **WHAT IS WRITTEN, AND WHEN:**
- ✅ **On a SUCCESSFUL load** — `SessionStore::setOpen(id, path)` (**new**), from `ScriviWindow`'s
  `loadFinished` handler, beside the window registration. ⚠️ **Only `open` + `path`:** ⛔ `record()`
  writes `maximized` unconditionally, so recording a fresh window's DEFAULT state would clobber the
  geometry the project was last closed with. ✅ **Writing at load, not only at quit, means a SIGKILLed
  session still knows what was open.**
- ✅ **In `ScriviWindow::closeEvent`, BEFORE `releaseProject()`** — frame, maximized, both splitters,
  handed to `AppEnvironment::projectWindowReleasing()`. ⚠️ **Every project-window exit arrives there:**
  the X, `Close Project` (`showLanding()` → `close()`), and Quit (`quitApplication()` → `close()`).
- ⛔ **`setClosed` is SKIPPED while quitting** — ✅ **Apple's `isTerminating` freeze, via the existing
  `quitting_` flag.** ⚠️ A quit closes every window; ⛔ marking them closed would leave R4 nothing.
- ⚠️ **A maximized window records `normalGeometry()`** — ✅ what un-maximizing after a restore returns
  to, ⛔ not the screen-sized rect.

⛔ **THE R6 GUARD — `AppEnvironment::projectsToRestore()`, the ONE route from `session.ini` to a
reopened window.** ✅ **Returns nothing when `QT_QPA_PLATFORM=offscreen` OR `SCRIVI_NO_RESTORE` is
set; ⛔ WRITES NOTHING (AC6).** ✅ Both signals are function-local `static const` — Apple's stored-`let`
rule. ⚠️ **T-0567 MUST start its restore there**, ⛔ never at `sessionStore().openProjectIDs()`.

⚠️ **The WRITE side is deliberately unguarded — as on Apple.** ✅ **Checked: no smoke constructs an
`AppEnvironment` or a `ScriviWindow`**, so only the real app writes `session.ini`. ✅ And because the
manifest is per-project `open` FLAGS (not Apple's whole-set rewrite), ⚠️ a `SCRIVI_NO_RESTORE` launch
that opens project A marks A open **without touching B's flag** — ✅ the un-restored session survives.

#### ✅ AC7 — PROVEN BY BREAKING IT (twice, in a throwaway container, never the repo)

✅ **`restore_guard_smoke` — three passes over one seeded temp root:** ⚠️ **(1) NO signal — the
NEGATIVE CONTROL: the funnel MUST return the open project**, which is what makes (2) and (3) mean
anything; **(2) `QT_QPA_PLATFORM=offscreen`; (3) `SCRIVI_NO_RESTORE=1`.** ✅ **Each suppressed pass
asserts `session.ini` is BYTE-IDENTICAL afterwards.**

| Break | Result |
| ----- | ------ |
| ⛔ **Guard removed** (`if (false)`) | ✅ **RED** — `[R6] a guarded run restores NOTHING` |
| ⛔ **Guard CLEARS the manifest** (`clearAll()` before return) | ✅ **RED** — `[AC6] BYTE-IDENTICAL`, `[AC6] the open set SURVIVES suppression` |

| Check | Result |
| ----- | ------ |
| ⚠️ **`ctest` — NON-ROOT (uid 1001)** | ✅ **641/641** |
| Linux smokes | ✅ **25/25** (⚠️ **was 24 — the new one is included**) |
| Docker build | ✅ **clean** (⚠️ only the pre-existing `DeterministicUUIDProvider.hpp` `-Wformat-truncation`) |
| `check-package-boundary.sh` | ✅ **GREEN** (AC8) |
| Apple `xcodebuild build` | ✅ **SUCCEEDED** (⚠️ no Apple source touched) |

⚠️ **TWO THINGS FOR T-0567, SURFACED NOT SOLVED:**
1. ⚠️ **A HIDDEN pane records a ZERO in its splitter sizes** (`QSplitter::sizes()` reports 0 for a
   hidden widget). ⛔ Navigator/inspector visibility is session-scoped (T-0563), so restoring
   `{0, …}` into a VISIBLE navigator would collapse it. ✅ **T-0567 must decide how to apply them.**
2. ⚠️ **Only the Quit paths set `quitting_`.** ⛔ An end-of-desktop-session close that bypasses
   `quitApplication()` would close each window as a *writer* close and mark it closed. ✅ **Unmeasured
   — worth a rig look in [SP-148]'s live pass.**

⛔ **ONE MESS OF MINE CLEANED UP:** ⚠️ **T-0565 spliced the `session_store_smoke` CMake block INTO THE
MIDDLE of the persistence smoke's comment header**, leaving that comment describing the wrong target.
✅ **Restored.**

---

### ✅ 2026-09-29 — T-0565 IMPLEMENTED (`SessionStore`)

✅ **`platforms/linux/src/SessionStore.{hpp,cpp}`** — ⚠️ **`<appSupportRoot>/session.ini` via
`QSettings`, keyed `[project/<projectID>]` with `path` as an ATTRIBUTE ([R-Q1]/[R-Q2]).**
⚠️ **ONE class, not Apple's two** (`OpenSessionManifest` + `ProjectWindowFrameStore`) — ✅ **because
[R-Q1] ruled ONE FILE, and two readers of one file is the [I-0215] class.**
⛔ **It owns NO widgets and does NO restoring** — ✅ **which is why its smoke needs no display.**

⚠️ **THREE DECISIONS WORTH THE COMMENT THEY CARRY:**
- ✅ **Sizes/frames stored as comma-joined STRINGS**, ⛔ not `QVariantList` — ⚠️ **a list writes as an
  opaque `@Variant(...)` blob nobody can inspect or repair.**
- ✅ **A malformed size list yields NOTHING, not a partial one** — ⚠️ **half a set of splitter sizes
  would lay the panes out wrongly, ⛔ which is worse than falling back to defaults.**
- ✅ **`sync()` on every write** — ⚠️ **`EditorShell.cpp:2906` already carries this habit and says why:
  ⛔ a `--rm` container may SIGKILL before Qt's lazy flush.**

#### ✅ VERIFIED BY RUNNING — ⛔ and the test was PROVEN RED first

✅ **`session_store_smoke` — 24 assertions, TWO passes over one temp root** (⚠️ **the second pass is the
"restart" case: it proves the FILE carries the session, ⛔ not process memory**). ✅ **ALL PASS.**

⚠️ **TWO ASSERTIONS ARE RULINGS, NOT CONVENIENCES, and both were proven failable:**
✅ **`setClosed` was temporarily changed to `remove()` the whole group, and the smoke went RED on
exactly `[R-Q2] geometry SURVIVES a close`, `splitter sizes SURVIVE a close` and `path survives a
close`** — ⚠️ **then green on restore.**

| Check | Result |
| ----- | ------ |
| ⚠️ **`ctest` — NON-ROOT (uid 1001)** | ✅ **641/641** |
| Linux smokes | ✅ **24/24** (⚠️ **was 23 — the new one is included**) |
| ⚠️ **`session_store_smoke` guard** | ✅ **PROVEN RED with [R-Q2] broken** |
| Docker build | ✅ **clean** |
| `check-package-boundary.sh` | ✅ **GREEN** — ⚠️ **AC8: `session.ini` is APP state under app-support, ⛔ not project data** |

⛔ **ONE BUILD ERROR OF MINE:** ⚠️ **the new target missed
`target_include_directories(... PRIVATE src)`, so the harness could not find its own header.**
✅ **Every sibling target has that line; ⛔ I added the target without copying it.**

---

⚠️ **2026-09-29 — Sprint created and ACTIVATED (user-approved).**
✅ **Apple's R6 guard was READ BEFORE PLANNING** (`AppEnvironment.swift:353-375`), ⚠️ **per the Epic's
standing instruction** — ✅ **and its *"manifest left UNTOUCHED"* property is now AC6.**
