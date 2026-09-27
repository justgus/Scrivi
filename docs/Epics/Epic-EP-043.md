---
epic: EP-043
status: Active
platform: Linux
created: 2026-09-21
activated: 2026-09-25
---

# EP-043: `[Linux]` ⚠️ **The Session** — ✅ **many projects, each in its own window, restored where the writer left it**

**Status:** 🟡 **ACTIVE — activated 2026-09-25 (user-approved).** ⛔ **No Sprint is active yet.**
✅ **ALL FIVE OWED RULINGS ARE ANSWERED — 2026-09-26 (user-approved)** (✅ **§Rulings**).
✅ **[SP-145] IS UNBLOCKED and may now be created and activated.**
⚠️ **THREE OF THE FIVE QUESTIONS WERE FRAMED ON A PREMISE THE CODE CONTRADICTED**, and in each case the
correction changed the ANSWER, not just the wording — ✅ **the superseded text is kept in §Rulings
because the error is the instructive part.** ⚠️ **[R-Q4] FILED [I-0251] against [SP-145]**, and
✅ **[R-Q5] gives [SP-145] a NAMED CARVE-OUT** (the visibility flags move onto the session object).
✅ **ACTIVATION RATIONALE (2026-09-25): it blocks on nothing, it is fully planned, and Apple's [EP-018]
is FINISHED AND VERIFIED so this port reads an outcome rather than inventing one.**
⚠️ **IT WAS ALSO CHOSEN OVER [EP-035] ON A FILE COLLISION, MEASURED NOT ASSUMED:** ⛔ **[SP-145]
extracts per-project state out of `EditorShell.cpp` (2,783 lines), and [EP-035]'s open **AC4** (object
CRUD) attaches to that SAME owner** — ✅ **`EditorShell::onOpenObjectRequested` (`EditorShell.cpp:2214`,
wired from `SceneInspector` at `:131`, commented *"opening an object is requested here and OWNED by the
shell"*).** ⚠️ **Running [EP-035] first would write CRUD into `EditorShell` and have [SP-145]
immediately relocate it** — ✅ **the "paying the extraction cost twice" this Epic already predicted for
[I-0244].** ⚠️ **[EP-035]'s AC5 (thumbnails) does NOT collide** (✅ `SceneInspector.cpp`) — ⛔ **and its
EP-039 blocker EXPIRED when [EP-039] closed 2026-09-15.**
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
simply never built.** ⚠️ **The visibility flags are therefore RULED EXPLICITLY rather than
silently reversed** — ✅ **[R-Q4]: inspector persists THROUGH THE CORE (Apple parity), timeline stays
session-scoped.** ⛔ **And SP-078 is NOT reversed: it stands for timeline, and is superseded for
inspector by Apple's Doc 2 AC4** (see §*Rulings*).

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
| **[SP-145]** | **S1** | ✅ **The session split** — extract per-project state out of `EditorShell` into a `ProjectSession` equivalent; ⚠️ **BEHAVIOUR-PRESERVING, still one window.** ✅ **Introduce the registry (projectID → session), keyed by `projectID` per [R-Q2].** ⚠️ **[R-Q5] CARVE-OUT: take `inspectorVisible_` + `timelineVisible_` onto the session object** (⛔ per-project state, NOT per-widget). ⚠️ **PLUS [I-0251] as a SEPARATE, NAMED Task** — ⛔ **it is a real behaviour CHANGE inside a behaviour-preserving Sprint, so it must not be folded into the extraction Task.** | — (foundation) · ⚠️ **[I-0251]** | ⚠️ **MED-HIGH** | ✅ **nothing** |
| **[SP-146]** | **S2** | ⚠️ **THE APP OBJECT *AND* THE WINDOWS — renamed 2026-09-27, see §The app-level owner.** ⛔ **Linux has NO app-global state owner at all**, so S2 must FIRST create one (Apple's `AppEnvironment`) and move the registry + session OWNERSHIP onto it. ✅ Then: one window per open project; separate Landing window ([R-Q3]); **R3 focus-existing**; quit flushes the REGISTRY; close calls `scrivi_close_project`. | **R1 R2 R3 R7 R8** | ⛔ **HIGH** | **[SP-145]** |
| **[SP-147]** | **S3** | ✅ **The persistence** — open-session manifest + launch restore; **per-project geometry AND splitter state**; ⚠️ **the test guard lands HERE, with the first line of restore code.** | **R4 R5 R6** | ✅ **MEDIUM** | **[SP-146]** |
| **[SP-148]** | **S4** | ✅ **Verification** — AC sweep, Docker `ctest`, ⚠️ **the live pass on the real rig**, Epic close prep. | **AC-build AC-live** | ✅ **LOW** | **[SP-147]** |

⚠️ **THE CHAIN IS SERIAL, and that is stated up front because [EP-041] is currently paying for the same
shape.** ✅ **It is serial for a real reason — ⛔ you cannot persist per-window state before there are
per-window identities** — ⚠️ **but it means a stall in SP-145 stalls the Epic.**

⛔ **R6 IS NOT DEFERRED TO SP-148.** ⚠️ **It sits in SP-147 deliberately**: ✅ **the guard must exist in
the same commit as the restore, or the rig reopens the user's real work under whatever was just
compiled** — ⚠️ **which is [I-0176]'s own warning, and [I-0150]'s lived one.**

---

## Tasks — ✅ **planned; TASK IDs not yet issued**

⚠️ **Tasks are issued at Sprint ACTIVATION, not now** — ✅ **an Epic on the backlog with live Task rows
in `Task-backlog.md` is exactly the layer-discipline defect `feedback_task_layer_discipline` records.**
⚠️ **The shape below is planning, not a claim on IDs.**
⛔ **NO NEXT-AVAILABLE FIGURE IS STATED HERE** — ✅ **user ruling 2026-09-24: they live in
[`../tools/next-ids.json`](../tools/next-ids.json), allocated by `python3 docs/tools/next-id.py`.**
⚠️ **This section previously read *"Next available Task ID at the time of writing: T-0540"*** —
⛔ **exactly the restated counter that ruling removed, and it was already stale.**
✅ **ONE ID IS ALREADY ISSUED against this Epic:** ⚠️ **[I-0251] (an ISSUE, not a Task), allocated
2026-09-26 and assigned to [SP-145]** — ✅ **[R-Q4].**

| Sprint | Planned work |
| ------ | ------------ |
| **[SP-145]** | Extract `ProjectSession` from `EditorShell` (behaviour-preserving), ⚠️ **carrying `inspectorVisible_` + `timelineVisible_` with it ([R-Q5])** · Introduce `OpenProjectRegistry` equivalent, ✅ **keyed by `projectID` ([R-Q2])** · ⚠️ **[I-0251] — load `inspectorHidden` from the core and write it back on change (SEPARATE Task, [R-Q4])** |
| **[SP-146]** | ⚠️ **FIRST: create `AppEnvironment`** (⛔ **Linux has no app-level owner; `main()` hand-threads everything**) · ⚠️ **move `OpenProjectRegistry` AND `ProjectSession` OWNERSHIP onto it** — ✅ **this removes [SP-145]'s reference-binding seam** · ⚠️ **add the R3 check in `AppEnvironment::openProject`** · ✅ **THEN** one window per project + ✅ **SEPARATE Landing window ([R-Q3])**; ⚠️ **`File ▸ New`/`Open` in a project window RAISE Landing; closing the last project SHOWS Landing and never quits** · R3 focus-existing guard · ⚠️ **quit flushes THE REGISTRY, not a window** — ⛔ **`main.cpp`'s `aboutToQuit → ScriviWindow::flushEditor` is SINGULAR BY CONSTRUCTION and must be rewired here (R7)** · per-window `scrivi_close_project` · ⚠️ **budget N menu bars: `buildMenuBar()`/`updateMenuState()` become per-window, including the inspector/timeline check-state sync** |
| **[SP-147]** | Open-session manifest + launch restore · per-project geometry store (frame + maximized) · splitter-proportion persistence — ✅ **ALL THREE in `<appSupportRoot>/session.ini` via `QSettings`, `[project/<projectID>]` with `path` as an attribute ([R-Q1]/[R-Q2]); ⚠️ carry the `settings.sync()` habit** · ⚠️ **the test/headless guard** — ✅ **FIRST evaluate `XDG_DATA_HOME` redirection (see §Known traps) before inventing a `--no-restore` flag** |
| **[SP-148]** | AC sweep · Docker `ctest` as non-root · ⚠️ **live pass on the rig** · close prep |

---

## ✅ Rulings — ALL FIVE ANSWERED 2026-09-26 (user-approved)

✅ **Recorded before [SP-145] so they were not discovered mid-Sprint**, the way [SP-141]'s
endpoint-shape ruling was. ✅ **[SP-145] IS NOW UNBLOCKED.**

⚠️ **THREE OF THE FIVE QUESTIONS WERE FRAMED ON A PREMISE THE CODE DOES NOT SUPPORT**, and the
corrections changed what the answer had to be. ✅ **The original text is kept struck below, because the
error is the instructive part** — ⛔ **each was a fact restated from memory instead of measured, which
is this project's P7 class.**

---

### ✅ **R-Q1 — Session state lives in `<appSupportRoot>/session.ini` (`QSettings`, `IniFormat`).**

⛔ **THE QUESTION'S PREMISE WAS WRONG: *"Apple used `UserDefaults`"* is only HALF TRUE.** ✅ **Apple
SPLITS session state on a principled line** — ⚠️ **app-level window bookkeeping → `UserDefaults`**
(`OpenSessionManifest.swift:12`, `ProjectWindowFrameStore.swift:22`, `ProjectPreferences.swift:3`);
⚠️ **per-project DOCUMENT state → the core's schema** (`inspectorVisible`, `ProjectSession.swift:89`).
✅ **So there were THREE candidate homes, not two — and the third is what [R-Q4] takes.**

✅ **THE SHAPE, mirroring the EXISTING precedent** (`EditorShell.cpp:2756-2783`, which already keys
per-project state by `projectID` in an INI under `appSupportRoot`):

```ini
[openSet]
projectIDs=A,B,C

[project/A]
path=/projects/tintagael.scrivi
frame=@Rect(120 80 1400 900)
maximized=false
splitters=240,580,200
```

✅ **WHY:** same idiom as `timeline-view.ini`; ⚠️ **survives a `--rm` container restart**; ✅ **and needs
NO `check-package-boundary.sh` allow-list entry, because `QSettings` is not `QFile`/`QSaveFile`.**
⚠️ **CARRY THE `settings.sync()` HABIT** — ✅ **`EditorShell.cpp:2782` explains it: *"flush now — a
`--rm` container may SIGKILL before Qt's lazy flush"*; that reason applies verbatim here.**
⛔ **`recents.json` REJECTED:** ⚠️ **it conflates two lifetimes** (a recent is remembered ~forever, an
open-set is per-quit), ⛔ **needs a new allow-list entry, and would make a corrupt recents file lose
all geometry too.**
⛔ **A CORE DOCUMENT REJECTED:** ⚠️ **a window screen-rect stored in the project package would travel
between machines** — ✅ **wrong for a geometry.**

### ✅ **R-Q2 — Key by `projectID`; the `path` is a stored ATTRIBUTE, not the key.**

⛔ **THE QUESTION'S PREMISE WAS WRONG: *"keying by projectID needs a path resolver Linux does not
have."*** ✅ **Linux ALREADY keys persisted state by `projectID`** (`EditorShell.cpp:2087`,
`timeline/<projectID>`) — ✅ **and the resolver IS the stored `path` attribute.** ⚠️ **Apple's
`ProjectBookmarkStore` exists for the SANDBOX, not for identity** (its own header, `:8-12`) — ⛔ **and
Linux has no sandbox;** ✅ **`RecentsStore.hpp:14` already records that a plain absolute path is
Linux's equivalent.**

| Case | ✅ Ruled behaviour |
| --- | --- |
| project **moved** | ⚠️ path fails to resolve → **skipped for that launch** (✅ **R4 already says so**) — ✅ **but its geometry SURVIVES** and reattaches when reopened from the new path |
| project **copied** | ✅ **both copies share a `projectID`** → the **R3** registry treats them as ONE project, ✅ **which is correct** |
| path resolves, but reports a **DIFFERENT `projectID`** | ✅ **OPEN it and RE-KEY** — ⚠️ **the writer asked for that window; the ID is bookkeeping** |

⛔ **KEYING BY PATH REJECTED:** ⚠️ **geometry lost on EVERY move, dead entries kept forever, and two
copies would read as two projects** — ⛔ **letting R3 open two windows writing the same `projectID`.**

### ✅ **R-Q3 — A SEPARATE Landing window; project windows are editor-only.**

✅ **Exact [EP-018] shape.** ✅ **One `QMainWindow` for Landing** (keeping `Landing.qml`, its
`ScriviBridge` and `RecentsStore`), ✅ **one editor-only `QMainWindow` per open project.**
✅ **The `QStackedWidget` page-swap (`showEditor`/`showLanding`) DISAPPEARS.**

⚠️ **THREE THINGS ARE WELDED TO THE SINGLE WINDOW TODAY, and each is real work [SP-146] must budget:**

1. ⚠️ **`Landing.qml` owns the whole open/create flow AND its own `ScriviBridge`** — ✅ **`main.cpp`
   sets `appSupportRoot`, `defaultProjectsFolder` and `shell` as context properties on the LANDING
   widget**, and the QML instantiates `ScriviBridge`/`RecentsStore` directly via `QML_ELEMENT`.
2. ⚠️ **The menu bar belongs to `ScriviWindow`** (`buildMenuBar()`, `updateMenuState(bool)`) — ✅ **on
   Linux each `QMainWindow` carries its OWN, so N windows = N menu bars**, ⚠️ **including the
   `showInspectorAction_` / `showTimelineAction_` check-state sync now going per-window.**
3. ⚠️ **`File ▸ New` / `File ▸ Open` route through the landing page DELIBERATELY** —
   ✅ **`ScriviWindow.hpp:88-92`: *"the landing page hosts the full Open UI + the ScriviBridge, so the
   menu doesn't reimplement that plumbing."***

✅ **TWO SUB-RULINGS settle (3) and the lifecycle:**
- ✅ **`File ▸ New` / `File ▸ Open` in a project window RAISE the Landing window and trigger its
  EXISTING flow** — ⚠️ **reusing the plumbing, which is what `ScriviWindow.hpp:88` already argues for**
  (⛔ **not reimplementing it per-window**).
- ✅ **Closing the LAST project window SHOWS Landing and NEVER quits implicitly** — ⚠️ **R7 (quit
  flushes every session) becomes much harder to reason about if a window close can become a quit.**

⛔ **"BOTH PAGES IN EVERY WINDOW" REJECTED:** ⚠️ **N QML engines + N `ScriviBridge` instances** —
⛔ **the app already paid ~79% of an open for a duplicate once ([I-0232])** — ✅ **and "which window
shows landing" is ambiguous.**

### ✅ **R-Q4 — Inspector visibility persists THROUGH THE CORE; timeline stays session-scoped.**

⛔ **THE QUESTION'S PREMISE WAS WRONG, AND THIS IS THE RULING THAT CHANGED MOST.** ⚠️ **It was framed
as *"should Linux reverse SP-078?"*** — ⛔ **it is not a reversal at all.** ✅ **Apple ALREADY persists
`inspectorVisible`, and NOT in `UserDefaults`:** ✅ **`ProjectSession.swift:89-96` writes
`setInspectorHidden(!inspectorVisible)` into `inspector-layout.json` THROUGH THE CORE**, ⚠️ **with the
comment *"Doc 2 AC4 requires the hide/show state to persist. It was previously in-memory only, so the
inspector reappeared on every launch regardless of the writer's choice."***

| Flag | Apple | ⚠️ Linux today |
| --- | --- | --- |
| **inspector visible** | ✅ **PERSISTED** via the core document | ⛔ **`InspectorLayoutStore.cpp:55` writes the DEFAULT `inspectorHidden: false`; NOTHING ever reads it or writes it back** — ⚠️ **the only occurrence of the key in `platforms/linux/src/`** |
| **timeline visible** | ⚠️ **NOT persisted** (`ProjectSession.swift:98`) | ⛔ **session-scoped** — ✅ **parity, by accident** |

✅ **RULED:** persist **inspector** via the ALREADY-WIRED `scrivi_get/put_inspector_layout`.
⚠️ **`InspectorLayoutStore` already round-trips the WHOLE document** (*"PATCH, NEVER RECONSTRUCT"*,
its header `:36-42`) — ✅ **so the field is ALREADY PRESERVED ON DISK; only the READ and the WRITE-BACK
are missing.** ✅ **Leave TIMELINE session-scoped, where Apple agrees.**

✅ **SP-078 IS NOT REVERSED.** ⚠️ **It ruled BOTH flags together as *"a member, not persisted to
disk"*; ✅ Apple has since ruled inspector differently FOR A STATED REASON.** ✅ **THE HONEST RECORD:
SP-078 STANDS for timeline, and is SUPERSEDED for inspector by Apple's Doc 2 AC4.**

⚠️ **THE GAP IS FILED AS [I-0251], ASSIGNED TO [SP-145]** (✅ **user ruling 2026-09-26**) — ⛔ **NOT
absorbed silently into an AC.** ✅ **It is the same *"capability shipped, surface never built"* class as
[I-0215], [I-0241] and [I-0242]**, ⚠️ **and [I-0242] was FILED rather than absorbed.**
⚠️ **IT GOES TO [SP-145] BECAUSE THE EXTRACTION IS ALREADY MOVING `inspectorVisible_`** — ✅ **adding
the load-and-write-back there touches the flag ONCE.** ⛔ **NOTE THE TENSION, DELIBERATELY ACCEPTED:
[SP-145]'s mandate is BEHAVIOUR-PRESERVING and this is a real behaviour CHANGE inside it** — ✅ **so it
must be a NAMED, SEPARATE Task in that Sprint, not folded into the extraction Task.**

### ✅ **R-Q5 — [I-0244] becomes a SIBLING Epic, sequenced AFTER this one.**

✅ **EP-043 keeps its four Sprints and stays closable.** ⚠️ **[I-0244] is FOUR APPLE SPRINTS DEEP**
([SP-134] toolbar · [SP-135] bars · [SP-136] inspector column · [SP-137] detail sheet) — ⛔ **folding
that into a SESSION Epic is the [EP-035] AC1 error [I-0178] cites (nine ACs collapsed into one)**,
✅ **which this Epic's own Scope Notes say it was created to avoid.**

⚠️ **THE INSTRUCTION THAT MAKES THE SEQUENCING REAL RATHER THAN NOMINAL:** ✅ **[SP-145]'s extraction
MUST take `inspectorVisible_` and `timelineVisible_` onto the session object as it goes** — ⚠️ **they
are PER-PROJECT state, not per-widget state** — ✅ **so the shell Epic has somewhere to put pane state
instead of re-opening `EditorShell`.** ✅ **This costs [SP-145] NOTHING: both members are already in
`EditorShell`.**
⚠️ **Of [I-0244]'s three MEASURED gaps** (⛔ no `QToolBar`, ⛔ no navigator-visibility state, ⛔ no
`safeAreaBar` equivalents), ✅ **exactly ONE overlaps the extraction — pane-visibility state** —
✅ **and this carve-out is it.**

⛔ **RUNNING THE SHELL EPIC FIRST STAYS REJECTED** — ✅ **it was rejected at activation on a MEASURED
file collision:** ⚠️ **it would write chrome into `EditorShell` and have [SP-145] relocate it
immediately.**

---

### ⛔ The original question text — SUPERSEDED, kept because the errors are instructive

⚠️ **Three of these five restated a fact from memory that the code contradicts.** ✅ **In each case the
correction changed the answer, not just the wording** — ⛔ **which is why "surface the premise, don't
just answer the question" is the habit worth keeping.**

1. ⛔ ~~*"Q1 — Apple used `UserDefaults`."*~~ → ✅ **Apple SPLITS; there was a third home, and [R-Q4]
   needed it.**
2. ⛔ ~~*"Q2 — keying by projectID needs a path resolver Linux does not have."*~~ → ✅ **it has one, and
   already uses `projectID` as a persistence key.**
3. ✅ *"Q3 — what happens to the landing surface?"* → ⚠️ **the premise held; ⛔ but it UNDERSTATED the
   cost — three things are welded to the single window, not one.**
4. ⛔ ~~*"Q4 — SP-078 ruled them non-persistent; reversing it must be SAID."*~~ → ✅ **not a reversal:
   Apple already persists inspector through the core, and Linux is simply out of parity.**
5. ✅ *"Q5 — sibling Epic or ACs inside?"* → ⚠️ **premise held** (✅ **added on activation**).

---

## ⚠️ The app-level owner — [SP-146]'s FIRST step (added 2026-09-27)

⚠️ **RAISED BY [SP-145] AND MEASURED, not inferred.** ✅ **[SP-145] put `OpenProjectRegistry` on
`EditorShell` because that Sprint changes no window code and one shell holds one project — ⛔ but that
is WRONG for more than one window, and the user agreed 2026-09-27.**

⛔ **THE REGISTRY CANNOT LIVE ON A WIDGET.** ⚠️ **Its whole purpose is to answer *"is this project
already open?"* BEFORE a window is created** — ✅ **a decision about the app's siblings, which no single
shell can make about the others.**

### ⛔ THE PROBLEM IS BIGGER THAN A MOVE: Linux has NO app-level owner

⚠️ **Measured 2026-09-27: there is no `AppEnvironment` equivalent anywhere in `platforms/linux/src/`.**
⛔ **No app singleton, no `Q_GLOBAL_STATIC`, no `qApp` property.** ✅ **App-global state is LOCAL
VARIABLES IN `main()`** — `appSupportRoot` is resolved at `main.cpp:87` and hand-threaded into the QML
context, `ScriviWindow`, and `ShellController` separately.
⚠️ **So [SP-146] does not "move a member." ✅ It CREATES Linux's first app-level state owner** — ⛔ **and
[SP-145]'s own note calling this SP-146's "first step" understated it: it is a real Task, not a
preamble.**

### ✅ WHAT APPLE DOES — and why it is the shape to copy

⚠️ **I checked every reader of Apple's registry. ✅ ALL OF THEM are inside `AppEnvironment`;
⛔ NOT ONE is in a view.** ✅ **That is the load-bearing fact:**

| Apple piece | Owns | ⚠️ Note |
| --- | --- | --- |
| `AppEnvironment` | ✅ the engine, identity, **`openProjects` registry**, ⚠️ **and it CREATES the sessions** (`makeSession()`, `AppEnvironment.swift:378`) | ⛔ the registry never leaves this class |
| `ProjectWindowManager` | ✅ `[String: ProjectWindowController]` — **keyed by `projectID`** | ⚠️ a SEPARATE map from the registry |
| the window | ✅ shows a session it is HANDED | ⛔ **never owns one** |

⛔ **LINUX HAS THIS INVERTED TODAY:** ⚠️ **`EditorShell` owns the session AND the registry.**
✅ **That inversion IS the defect.**

### ✅ The target shape

```
AppEnvironment                     ← NEW (SP-146): app-global state
├── appSupportRoot                 ← today: a local in main()
├── OpenProjectRegistry            ← today: wrongly on EditorShell
├── owns the ProjectSession objects (one per open project)
└── openProject(path) → session*   ← THE R3 CHECK LIVES HERE

ProjectWindowManager               ← projectID → window
EditorShell                        ← is HANDED a session; owns no identity
```

### ⚠️ Three steps, in order

1. ✅ **Create `AppEnvironment`**; move `appSupportRoot` onto it; construct it in `main()`.
   ⚠️ **Mechanical, no behaviour change** — ✅ **do it first and alone, so the rest has a home.**
2. ⚠️ **Move the registry AND session OWNERSHIP onto it.** ✅ **`EditorShell` takes a `ProjectSession*`
   instead of holding one by value.** ✅ **THIS IS WHERE [SP-145]'s REFERENCE-BINDING SEAM IS REMOVED** —
   ⚠️ **it was built for exactly this moment.** ⛔ **This step changes `EditorShell`'s lifetime contract
   and deserves its own Task.**
3. ✅ **Add the R3 check in `AppEnvironment::openProject`** — ⚠️ **the first point where the registry does
   real work, because it is the first time two windows can exist.**

### ⚠️ ALSO ON THIS CLASS: Apple's R6 guard already exists — do NOT invent a third mechanism

✅ **`AppEnvironment.swift:374` has `SCRIVI_NO_PROJECT_LOAD`, plus `isRunningUnderTests`** (`:353`), ⚠️
**and it deliberately leaves the open-session manifest INTACT while suppressing restore** — ✅ **so a
test run cannot lose the writer's windows.** ⚠️ **That is a better-specified precedent than the
`XDG_DATA_HOME` redirection floated in [SP-145]'s notes.** ⛔ **[SP-147] must read Apple's guard before
planning R6** (⚠️ **`feedback_look_for_existing_pattern_first`**) — ✅ **and it belongs on this same new
class, which is another reason to build it in [SP-146] rather than later.**

### ⛔ WHAT IS **NOT** PART OF THIS — the bridge duplication, INVESTIGATED AND DISMISSED

⚠️ **[SP-145] flagged that Linux constructs TWO `ScriviBridge` instances** (`Landing.qml:32` and
`EditorShell.cpp:69`), **each calling `bootstrap()`, and proposed filing it as an Issue.**
⛔ **MEASURED 2026-09-27, AND IT IS NOT A DEFECT. NO ISSUE WAS FILED.** ✅ **Three findings:**

1. ⚠️ **THE COST IS NOTHING.** ✅ **Benchmarked through the C ABI in the Linux container: the first
   `scrivi_ensure_local_identity` costs `0.78–5.0 ms`; ⛔ the SECOND costs `0.024–0.037 ms`, the third
   `0.018 ms`.** ⚠️ **The claim that each bridge "bootstraps identity separately" implied a cost that
   does not exist.**
2. ✅ **THERE IS NO DIVERGENCE RISK.** ⚠️ **`CoreSingleton` is ONE per process
   (`scrivi_c_api.cpp:157`), so both bridges share ONE `SecureStore` and resolve the SAME identity.**
3. ✅ **TWO INSTANCES IS THE CORRECT DESIGN.** ⚠️ **Their `errorOccurred` handlers are DELIBERATELY
   DIFFERENT surfaces: the landing sets `window.landingError`; the editor sets `errorLabel_` and must do
   so via a `QueuedConnection` because its calls run on a worker (T-0499).** ⛔ **Merging them would
   route worker-thread errors into the QML landing — a regression, not a cleanup.**

⚠️ **RECORDED BECAUSE THE INVESTIGATION IS THE RESULT.** ✅ **"Two instances of a boundary class" LOOKS
like the duplication class this Epic's siblings keep finding ([I-0215], [I-0241], [I-0242])** — ⛔ **and
it is not one.** ⚠️ **A measurement is what told the difference, and filing it on the resemblance would
have cost a Sprint's attention for no defect.**

---

## ⚠️ Known traps — each already paid for once

- ⚠️ **The test guard is the expensive one.** ✅ **[I-0150]:** `xcodebuild test` launched the app and
  **rewrote a real project**. ⛔ **A restore feature without a guard is a data-loss feature.**
  ✅ **IT MAY ALREADY BE HALF-BUILT — measured 2026-09-26, and worth checking BEFORE inventing a
  flag:** ⚠️ **`AppSupport.cpp:14-20` honours `XDG_DATA_HOME`**, and ✅ **every smoke already exports
  `QT_QPA_PLATFORM=offscreen`** (e.g. `tests/quit_smoke.sh:17`). ⚠️ **A run that points
  `XDG_DATA_HOME` at a temp dir CANNOT SEE the writer's `session.ini` at all** — ✅ **the strongest
  form of the guard, because it REMOVES the state rather than suppressing a code path.**
  ⛔ **Evaluate that before adding a `--no-restore` switch** (⚠️ **`feedback_look_for_existing_pattern_first`**).
- ⚠️ **R7 is a one-line problem today and will NOT stay one.** ✅ **`main.cpp` wires
  `aboutToQuit → ScriviWindow::flushEditor`, SINGULAR BY CONSTRUCTION.** ⛔ **Once sessions are
  registry-held, that connection must flush THE REGISTRY, not a window** — ✅ **written into
  [SP-146]'s row so it is not rediscovered**, ⚠️ **per R7's own framing: *a multi-window app whose quit
  path saves one window is a data-loss defect, not a polish item*.**
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

## ✅ Adjacent, and SEQUENCED — [I-0244] is a SIBLING Epic, AFTER this one ([R-Q5], 2026-09-26)

⚠️ **[I-0244] (filed 2026-09-22) records that Linux has NONE of [EP-040]'s editor shell** — ⛔ **no
toolbar, no navigator-visibility state, no `safeAreaBar` equivalents.**

✅ **RULED 2026-09-26 (user-approved): [I-0244] BECOMES ITS OWN `[Linux]` EPIC, SEQUENCED AFTER
[EP-043].** ⛔ **NOT ACs inside this Epic.** ✅ **THE TWO RECORDS NOW AGREE** — ⚠️ **they previously did
not: this section called it *"a different Epic's subject"* while [I-0244]'s own row said *"sequenced
WITH or after"*.** ✅ **It is AFTER, ruled** — ⛔ **and [I-0244]'s row has been corrected to match, so
this is no longer two sources of truth drifting.**

✅ **WHY NOT FOLDED IN:** ⚠️ **[I-0244] is FOUR APPLE SPRINTS DEEP** ([SP-134] toolbar · [SP-135] bars ·
[SP-136] inspector column · [SP-137] detail sheet) — ⛔ **folding that into a SESSION Epic is the
[EP-035] AC1 error [I-0178] cites (nine ACs collapsed into one)**, ✅ **which this Epic's own Scope
Notes say it was created to avoid.** ⚠️ **EP-043 stays shippable and closable; ⛔ the accepted cost is
that Linux stays visibly behind Apple's chrome for one more Epic.**

⚠️ **THE CARVE-OUT THAT MAKES THE SEQUENCING REAL RATHER THAN NOMINAL** — ✅ **this is the whole
operative content of [R-Q5], and it lands in [SP-145]:** ⚠️ **the extraction MUST take
`inspectorVisible_` and `timelineVisible_` onto the session object as it goes**, ⛔ **because they are
PER-PROJECT state, not per-widget state.** ✅ **Then the shell Epic has somewhere to put pane state
instead of re-opening `EditorShell`.** ✅ **It costs [SP-145] NOTHING — both members are already in
`EditorShell`.**

### ⚠️ AMENDMENT 2026-09-27 — [R-Q5]'s MECHANISM was wrong; its INTENT stands

⛔ **[R-Q5] said the extraction should "take `inspectorVisible_` and `timelineVisible_` onto the session
object."** ⛔ **THOSE MEMBERS DO NOT EXIST AND NEVER DID.** ✅ **Measured 2026-09-27: visibility is read
STRAIGHT OFF THE WIDGETS** — `inspector_->isVisible()` / `timeline_->isVisible()`
(`EditorShell.cpp:1966`, `:1983`). ⚠️ **`EditorShell.cpp:124` NAMES `inspectorVisible_` in a comment**
— ⛔ **and the member was never added**, ✅ **so the ruling was written from a comment rather than from
the code.**

✅ **THE INTENT IS UNAFFECTED:** ⚠️ **pane visibility must become per-project state with a home, so the
shell Epic does not re-open `EditorShell` for it.** ⛔ **THE MECHANISM CHANGES:** ✅ **[SP-145] must
INTRODUCE that state, not relocate it** — ⚠️ **slightly MORE work than ruled, not less.**
⚠️ **AND `InspectorLayoutStore` IS `SceneInspector`'s, not `EditorShell`'s** (`SceneInspector.hpp:257`)
— ✅ **so [I-0251] touches `SceneInspector`.**
⚠️ **THIS IS THE STANDING-RULE CLASS `CLAUDE.md` DESCRIBES** — ✅ ***"a list rots without being
edited"*** — ⛔ **a restatement that reads correctly while the code has moved underneath it.**

⚠️ **OF [I-0244]'s THREE MEASURED GAPS, EXACTLY ONE OVERLAPS THE EXTRACTION:** ⛔ no `QToolBar` (✅ that
is `ScriviWindow`'s, and [R-Q3] is reworking the window layer anyway) · ⛔ no `safeAreaBar` equivalents
(✅ also the window layer) · ⚠️ **pane-visibility state — THIS is the overlap, and the carve-out above
is it.** ✅ **So "both rework `EditorShell`" was true but too coarse: the collision is ONE concern
wide, and it is now assigned.**

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
