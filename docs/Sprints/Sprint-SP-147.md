---
sprint: SP-147
epic: EP-043
status: Active
platform: Linux
created: 2026-09-29
activated: 2026-09-29
---

# SP-147 — `[Linux]` **The Persistence** (S3 of [EP-043])

**Status:** 🟡 **ACTIVE — 2026-09-29 (user-approved).**
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

- [ ] **AC1** — ✅ **A `SessionStore` owns `<appSupportRoot>/session.ini` via `QSettings`**
      (⚠️ **`QSettings::IniFormat`, [R-Q1]**), ⛔ **and nothing else reads or writes that file.**
- [ ] **AC2** — ⚠️ **[R-Q2]** ✅ **Keyed `[project/<projectID>]`, with `path` as a stored ATTRIBUTE** —
      ⛔ **NOT keyed by path.** ⚠️ **A moved project is skipped for one launch and KEEPS its geometry.**
- [ ] **AC3** — ✅ **Every write calls `settings.sync()`.** ⚠️ **Mirrors `onTimelineViewStateChanged`
      (`EditorShell.cpp:2906`) and its stated reason: ⛔ *"a --rm container may SIGKILL before Qt's lazy
      flush."***
- [ ] **AC4** — ⚠️ **[R4]** ✅ **On relaunch, every project open at quit is reopened**, ⛔ **skipping any
      whose path no longer resolves** (⚠️ **and a skip must NOT delete its record — R-Q2**).
      ⚠️ **Closes [I-0176].**
- [ ] **AC5** — ⚠️ **[R5]** ✅ **A restored window returns to its size, position and maximized state**,
      ✅ **AND its splitter proportions.** ⚠️ **Keyed BY PROJECT** — ⛔ **[EP-018] hit exactly this: one
      global autosave frame made every restored window stack at the default.** ⚠️ **Closes [I-0177].**
- [ ] **AC6** — ⛔ **[R6] THE GUARD.** ✅ **A test, smoke or headless/offscreen run NEVER restores the
      writer's windows**, ⚠️ **and the manifest is left INTACT while restore is suppressed** —
      ⛔ **suppressing must not be able to LOSE the open set.**
- [ ] **AC7** — ✅ **The guard is PROVEN BY BREAKING IT.** ⚠️ **A smoke asserts restore is suppressed;
      ⛔ removing the guard must make it go RED.** ✅ **Same discipline as [T-0553]'s and [T-0564]'s.**
- [ ] **AC8** — ⚠️ **No `QFile`/`QSaveFile` added under `platforms/linux/src/`.**
      ✅ **`scripts/check-package-boundary.sh` stays GREEN** — ⚠️ **`session.ini` is APP state under
      app-support, ⛔ NOT project data, so it does not cross the package boundary.**
- [ ] **AC-build** — ✅ **Docker build clean; `ctest` GREEN as NON-ROOT in the tests-on image**
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
