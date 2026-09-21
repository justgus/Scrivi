---
epic: EP-043
status: Draft
platform: Linux
created: 2026-09-21
---

# EP-043: `[Linux]` ⚠️ **The Session** — ✅ **many projects, each in its own window, restored where the writer left it**

**Status:** 🔵 **DRAFT — on the [Epic backlog](Epic-backlog.md). ⛔ Not activated; no Sprint assigned.**
**Sprints (planned, IDs reserved):** [SP-145] · [SP-146] · [SP-147] · [SP-148]
**Primary Issues:** [I-0178] (multi-project) · [I-0176] (reopen at launch) · [I-0177] (geometry)
**Codebase:** `[Linux]` — ⚠️ **`platforms/linux/` ONLY.** ✅ **No ScriviCore change is expected.**
**Apple precedent:** [EP-018](Closed/Epic-EP-018.md) — *Per-Window / Per-Project Window Model*,
R1–R5 user-verified 2026-06-25, delivered in **3 Sprints (SP-048 → SP-050)**.
**Reference:** [`../Scrivi_Platform_Porting_Outline_v0_1.md`](../Scrivi_Platform_Porting_Outline_v0_1.md)
— ⚠️ **read BEFORE the first Sprint**, per the standing rule on that document.

---

## ✅ Why this Epic exists

⚠️ **All three Issues were found by the USER, on the REAL RIG, on 2026-08-29** — ✅ **the first day the
Linux app had ever run on real hardware** (SP-123 / T-0476). ⚠️ **None was found by a test suite**, and
that is the point `feedback_live_pass_finds_what_suites_cannot` already records: ✅ **what survives a
quit is not something a green suite can report on.**

⚠️ **They are not three defects. They are ONE missing concept — a SESSION —** and [I-0178] says so in
its own text: ✅ *"it is the natural parent of I-0176 and I-0177, since 'restore what was open' and
'restore geometry' are both per-window concepts that need a window registry to hang from."*

✅ **Apple already answered this exact question**, and the answer is a closed Epic with a named
architecture, not a sketch:

| Apple piece (`Scrivi/App/`) | What it owns | ⚠️ Linux equivalent today |
| --------------------------- | ------------ | ------------------------ |
| `OpenProjectRegistry.swift` | projectID → live session; **the authoritative non-reentrancy guard** | ⛔ **NONE** |
| `ProjectSession.swift` | ALL per-project state, one instance per window | ⚠️ **`EditorShell` — ONE instance, ONE `bridge_`, ONE `projectPath_`, ONE `sceneDoc_`** |
| `ProjectWindowManager.swift` | one `NSWindow`/controller per open project | ⛔ **NONE — `ScriviWindow` is a single `QMainWindow`** |
| `OpenSessionManifest.swift` | the set of projectIDs open at quit → the restore set | ⛔ **NONE — `RecentsStore` is a recents LIST, with no "was open" bit** |
| `ProjectWindowFrameStore.swift` | frame + full-screen, **keyed by projectID** | ⛔ **NONE — no geometry is persisted anywhere in `platforms/linux/`** |

⚠️ **Read that table in order and the Sprint sequence falls out of it**: ✅ the registry and the
session split are the foundation; ⛔ the manifest and the frame store CANNOT be built first, because
both are keyed by a per-window identity that does not yet exist.

---

## ⚠️ What the code actually looks like today — measured 2026-09-21, not recalled

✅ **`ScriviWindow` (`src/ScriviWindow.hpp:70`) is one `QMainWindow`** whose central widget is a
`QStackedWidget` with exactly two pages — page 0 the landing QML in a `QQuickWidget`, page 1 a single
lazily-built `EditorShell*  editor_`. ⚠️ **`showEditor()` / `showLanding()` SWAP between them.** ⛔ **A
second project has nowhere to go.**

✅ **`EditorShell` is 2,783 lines** (`src/EditorShell.cpp`) and owns the project singly. ⚠️ **This is
the single largest file in the Linux app** — ✅ **and the [EP-018] precedent says the extraction of a
per-project session from a single shared owner is its own Sprint** (SP-048 / T-0192, *"behavior-preserving"*).

✅ **`RecentsStore` (`src/RecentsStore.hpp`) is honest about its scope** in its own header: *"the recent-projects
store… an ordered list of {path, title, lastOpened}"*. ⚠️ **There is no session concept to extend.**

✅ **`EditorShell.cpp:124` hardcodes the splitter sizes `{240, 580, 200}` on every launch.** ⚠️ **So
the navigator/viewport/inspector proportions a writer sets are lost at every quit** — ⚠️ **the user
reported the WINDOW, but [I-0177] already records that the panel layout has the same defect and is
"arguably more annoying."** ✅ **This Epic fixes both; ⛔ it does not treat the splitter as optional.**

⚠️ **Session-scoped-only WAS a deliberate choice for VISIBILITY flags** (SP-078 / T-0320 — *"a member,
not persisted to disk"*). ⛔ **Nothing ever ruled that GEOMETRY should be discarded** — ✅ **it was
simply never built.** ⚠️ **AC5 must therefore rule the visibility flags explicitly rather than
silently reversing an earlier decision** (see §*Rulings owed*).

---

## Acceptance Criteria

⚠️ **Deliberately numbered R1–R5 to match [EP-018]**, so a reviewer can read the two Epics side by
side and see what parity means. ✅ **R6–R8 are Linux-specific and have no Apple counterpart.**

- [ ] **R1** — Multiple **distinct** projects can be open simultaneously. *(Mirrors EP-018 R1.)*
- [ ] **R2** — Each open project lives in **its own window**. *(Mirrors EP-018 R2.)*
- [ ] **R3** — The same project is **non-reentrant**: opening an already-open project **raises and
      focuses its existing window** instead of opening a second copy. ⚠️ **An app-side registry is
      authoritative** — ✅ **EP-018 proved the platform's own de-duplication was not race-safe (T-0191)
      and abandoned it on evidence.** *(Mirrors EP-018 R3.)*
- [ ] **R4** — On relaunch, the app **restores every project window that was open at quit**, skipping
      any whose path no longer resolves. ⚠️ **Closes [I-0176].** *(Mirrors EP-018 R4.)*
- [ ] **R5** — A restored or reopened project window returns to **the size, position and maximized
      state it had when last closed**, ✅ **and to its SPLITTER PROPORTIONS.** ⚠️ **Closes [I-0177].**
      ⚠️ **Geometry is keyed BY PROJECT**, not one global frame — ✅ **EP-018 hit exactly this: the
      pre-EP-018 single autosave stored ONE frame, so every restored window stacked at the default.**
- [ ] **R6** — ⚠️ **THE TEST GUARD.** A test run, a smoke run, or a headless/offscreen run **NEVER
      restores the writer's real project windows.** ⚠️ **[I-0176] names this as mandatory**, and
      `feedback_never_drive_synthetic_input_at_real_work` / [I-0150] are what it is paying for: ✅ **on
      Apple, `xcodebuild test` LAUNCHED the app and rewrote a real project.** ⛔ **This AC is not
      optional and is not deferrable to the end.**
- [ ] **R7** — **Quit flushes EVERY open project**, not just the front one. ⚠️ **`main.cpp` wires
      `aboutToQuit → ScriviWindow::flushEditor`, which is singular by construction** — ✅ **a
      multi-window app whose quit path saves one window is a data-loss defect, not a polish item.**
- [ ] **R8** — ⚠️ **`scrivi_close_project` is called for every window that closes.** ✅ **Linux already
      does this correctly for its single project** (`project_apple_index_leak` records Apple as the
      side that does NOT) — ⛔ **and a multi-window rework is exactly where that correctness gets
      dropped silently.** ✅ **Guard it while the code is being touched, not afterwards.**
- [ ] **AC-build** — Docker build clean; `ctest` green on Linux; the existing Linux smokes still pass;
      ⚠️ **no regression to open / save / close.** ✅ **Run the tests as a NON-ROOT user in the second
      image** (`project_linux_container_tests_off` — ⚠️ **the Dockerfile builds `SCRIVI_BUILD_TESTS=OFF`,
      so "the container is green" does NOT mean `ctest` ran**).
- [ ] **AC-live** — ⚠️ **A LIVE PASS ON THE REAL RIG, by the user.** ✅ **All three Issues were found
      that way and none can be verified any other way** — ⚠️ **"was it still there after a quit?" is
      not a question a headless smoke can ask.** ⚠️ **Confirm the build under test first**
      (`scrivi_linux --version`, `feedback_confirm_the_build_under_test`).

---

## Sprints — ✅ **4 planned**, ⚠️ **sized against EP-018's actual 3**

⚠️ **EP-018 took 3 Sprints on Apple and this is scoped at 4.** ✅ **The extra one is SP-145**, and the
reason is concrete rather than padding: ⚠️ **Apple's `ProjectSession` was extracted from an
`AppEnvironment` that was already a separate object; ✅ Linux's per-project state lives inside a
2,783-line `EditorShell` that is ALSO the widget.** ⚠️ **Separating "the project's state" from "the
widget showing it" is the whole risk of this Epic, and it deserves its own Sprint with a
behaviour-preserving mandate.**

| Sprint | Step | Scope | ACs | ⚠️ Risk | ⛔ Blocked by |
| ------ | ---- | ----- | --- | ------ | ------------ |
| **[SP-145]** | **S1** | ✅ **The session split** — extract per-project state out of `EditorShell` into a `ProjectSession` equivalent; ⚠️ **BEHAVIOUR-PRESERVING, still one window.** ✅ **Introduce the registry (projectID → session).** | — (foundation) | ⚠️ **MED-HIGH** | ✅ **nothing** |
| **[SP-146]** | **S2** | ✅ **The windows** — one window per open project; landing/editor relationship reworked; **R3 focus-existing**; quit flushes all; close calls `scrivi_close_project`. | **R1 R2 R3 R7 R8** | ⛔ **HIGH** | **[SP-145]** |
| **[SP-147]** | **S3** | ✅ **The persistence** — open-session manifest + launch restore; **per-project geometry AND splitter state**; ⚠️ **the test guard lands HERE, with the first line of restore code.** | **R4 R5 R6** | ✅ **MEDIUM** | **[SP-146]** |
| **[SP-148]** | **S4** | ✅ **Verification** — AC sweep, Docker `ctest`, ⚠️ **the live pass on the real rig**, Epic close prep. | **AC-build AC-live** | ✅ **LOW** | **[SP-147]** |

⚠️ **THE CHAIN IS SERIAL, and that is stated up front because [EP-041] is currently paying for the same
shape.** ✅ **It is serial for a real reason — ⛔ you cannot persist per-window state before there are
per-window identities** — ⚠️ **but it means a stall in SP-145 stalls the Epic.**

⛔ **R6 IS NOT DEFERRED TO SP-148.** ⚠️ **It sits in SP-147 deliberately**: ✅ **the guard must exist in
the same commit as the restore, or the rig reopens the user's real work under whatever was just
compiled** — ⚠️ **which is [I-0176]'s own warning, and [I-0150]'s lived one.**

---

## Tasks — ✅ **planned, IDs NOT yet issued**

⚠️ **Tasks are issued at Sprint ACTIVATION, not now** — ✅ **an Epic on the backlog with live Task rows
in `Task-backlog.md` is exactly the layer-discipline defect `feedback_task_layer_discipline` records.**
✅ **Next available Task ID at the time of writing: T-0540.** ⚠️ **The shape below is planning, not a
claim on IDs.**

| Sprint | Planned work |
| ------ | ------------ |
| **[SP-145]** | Extract `ProjectSession` from `EditorShell` (behaviour-preserving) · Introduce `OpenProjectRegistry` equivalent |
| **[SP-146]** | One window per project + landing rework · R3 focus-existing guard · quit flushes all sessions · per-window `scrivi_close_project` |
| **[SP-147]** | Open-session manifest + launch restore · per-project geometry store (frame + maximized) · splitter-proportion persistence · ⚠️ **the test/headless guard** |
| **[SP-148]** | AC sweep · Docker `ctest` as non-root · ⚠️ **live pass on the rig** · close prep |

---

## ⚠️ Rulings owed BEFORE [SP-145] activates

✅ **Recorded now so they are not discovered mid-Sprint**, the way [SP-141]'s endpoint-shape ruling was.

1. ⚠️ **Q1 — Where does session state live on disk?** ✅ **Apple used `UserDefaults`.** ⚠️ **Linux has
   `QSettings` AND an existing `recents.json` under `appSupportRoot`, and [I-0177] calls both
   plausible.** ⛔ **Picking one late means writing the persistence twice.**
2. ⚠️ **Q2 — What is the window identity key?** ✅ **Apple keys by `projectID`, resolving the path
   separately via `ProjectBookmarkStore`.** ⚠️ **Linux `RecentsStore` entries are PLAIN ABSOLUTE PATHS
   — there are no security-scoped bookmarks.** ⚠️ **Keying by path is simpler and WRONG the moment a
   project moves; keying by projectID needs a path resolver Linux does not have.**
3. ⚠️ **Q3 — What happens to the landing surface?** ✅ **Apple keeps a separate Welcome window.**
   ⚠️ **Linux stacks landing and editor in ONE window.** ⛔ **Does the landing become its own window, or
   does closing the last project return that window to landing?** ✅ **This is a visible UX decision,
   not an implementation detail.**
4. ⚠️ **Q4 — Do the SESSION-SCOPED VISIBILITY FLAGS become persistent?** ⚠️ **SP-078 / T-0320 ruled
   them deliberately non-persistent.** ✅ **R5 persists geometry and splitters.** ⛔ **Leaving inspector
   and timeline visibility session-scoped while their SIZES persist is defensible but must be SAID** —
   ⚠️ **otherwise the next live pass files it as a defect.**

---

## ⚠️ Known traps — each already paid for once

- ⚠️ **The test guard is the expensive one.** ✅ **[I-0150]:** `xcodebuild test` launched the app and
  **rewrote a real project**. ⛔ **A restore feature without a guard is a data-loss feature.**
- ⚠️ **Do not mirror a placeholder.** ✅ **`feedback_mirror_the_finished_surface_not_the_placeholder`** —
  ⛔ **Linux once copied an Apple stub Apple had already deleted.** ✅ **Mirror EP-018's FINAL
  architecture (AppKit windows + authoritative registry), ⛔ NOT the approved design it abandoned
  (`WindowGroup(for:)`, which cached dismissed windows and hung on reopen).**
- ⚠️ **Find how the app ALREADY does it first.** ✅ **`feedback_look_for_existing_pattern_first`** —
  ⚠️ **SP-118's dominant defect; four Issues each broke a rule already written down in the repo.**
- ⚠️ **The rig is READ-ONLY for source.** ✅ **`feedback_rig_is_read_only`** — ⚠️ **edit on macOS, push,
  pull.** ⛔ **Never `sed` on the rig.**
- ⚠️ **Right-Shift+D silently disconnects the RDP session** — ✅ **`Scrivi_Linux_Rig_Setup_v0_1.md`.**
- ⚠️ **"The Linux container is green" ≠ `ctest` ran.** ✅ **`project_linux_container_tests_off`.**

---

## ⛔ Explicitly OUT of scope

- ⛔ **Any ScriviCore / C ABI change.** ✅ **EP-018 was Swift-layer only and this is expected to be
  `platforms/linux/`-only.** ⚠️ **If a core change proves necessary, that is a finding worth
  surfacing, not a quiet addition.**
- ⛔ **Apple-side work of any kind.** ✅ **EP-018 is closed and verified.**
- ⛔ **[I-0181]** (unmounted volume → false `missing`) — ✅ **it is `[ScriviCore]` and already
  Resolved-Not-Verified under T-0498.**
- ⛔ **Deep links** (EP-018 R5's Apple counterpart). ⚠️ **Linux has no deep-link surface to rebuild**,
  ✅ **so R5 here is geometry instead — the numbering match is deliberate but not literal.**
- ⛔ **Undo/redo, menus and settings parity** — ✅ **that is [EP-026], still 🔵 Draft.**

---

## Scope Notes

✅ **This Epic is the SECOND half of what the real-rig day produced.** ⚠️ **The first half — the drive-loss
and open-cost findings — became [EP-042], now closed.** ✅ **These three Issues were held back because
[I-0178] correctly refused to be sized as one Issue**, ⚠️ **citing the EP-035 AC1 error where nine ACs
were collapsed into one.**

⚠️ **`project_linux_no_session_persistence` has recorded "wants its own Epic" since the day it was
found.** ✅ **This is that Epic.**

✅ **Per `feedback_linux_adopts_apple_shape`, Apple is the reference architecture** — ⚠️ **but note the
direction here is unusually clean: ✅ Apple is FINISHED and VERIFIED, so this port reads an outcome
rather than inventing one.**
