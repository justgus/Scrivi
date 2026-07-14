# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| **SP-061** | **`[Linux]` Shell flip to Widgets host + Scene navigator + read-only continuous viewport** | **EP-022** | **🔵 Planning (next up)** |
| SP-062 | `[Linux]` Editable viewport + per-scene auto-save + in-editor ⌘↩/⌘⇧↩ Scene/Chapter creation | EP-022 | 🔵 Planning |
| SP-063 | `[Linux]` Scroll-driven scene switching + active-scene tracking | EP-022 | 🔵 Planning |
| SP-064 | `[Linux]` Cursor/focus + quit-reopen restore + EP-022 verify & close | EP-022 | 🔵 Planning |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning (parked) |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning (parked) |

Design reference for all: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06).
Epic-level ACs: `docs/Epics/Epic-active.md` (EP-019). Task detail: `docs/Tasks/Task-backlog.md` (T-0198–T-0217).

> **SP-051 activated 2026-07-06** (design signed off at activation) — moved to `Sprint-active.md`.
> **SP-052 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-052.md`.
> **SP-053 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-053.md` (EP-019 AC1 delivered & verified).
> **SP-054 activated 2026-07-07**, ✅ **closed 2026-07-09** — `Closed/Sprint-SP-054.md` (EP-019 AC3 + AC5 delivered & verified).
> **SP-055 activated 2026-07-10**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-055.md` (EP-019 AC4 + AC5's deferred branch clauses delivered & verified).
> **SP-056/SP-057 parked 2026-07-13** — EP-019 `[Apple]` was deferred to the Epic backlog; these sprints remain 🔵 Planning but are **not** next-up. The active line of work is the `[Linux]` app family (EP-020+).
> **SP-058 activated 2026-07-13**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-058.md` (EP-020 `[Linux]` App Foundation delivered & verified; EP-020 → Complete). Next `[Linux]` sprint is defined when EP-021 is drafted.
> **SP-059 activated 2026-07-13**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-059.md` (EP-021 `[Linux]` Project Lifecycle create-half: appSupportRoot + recents + landing + create-project; AC1/AC2/AC4/AC5 delivered & user-verified over VNC; T-0229 `EncryptedFileSecureStore` added mid-sprint).
> **SP-060 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-060.md` (EP-021 `[Linux]` open/close half: Open Project + 3 open modes + close→landing + full-loop smoke; AC3/AC6/AC7/AC8 delivered & VNC-verified). **This closed EP-021.** Next `[Linux]` Epic **EP-022** (Writing Surface) is Active; its first sprint (SP-061) is drafted at planning.
> **SP-061 drafted 2026-07-14** (🔵 Planning, next up) — EP-022 `[Linux]` first of 4. Front-loads the Widgets-host **shell flip** (`QApplication`+`QMainWindow` re-hosting the EP-020/021 QML via `QQuickWidget` — the consequence of the `QPlainTextEdit` editor decision on Qt 6.4) + **scene navigator** + **read-only continuous viewport** (one `QPlainTextEdit`/`QTextDocument`, all bodies via `open_project` + `scrivi_open_scene` loop). Delivers EP-022 **AC1 + AC2**. No ScriviCore work. Awaiting activation.
> **SP-062/SP-063/SP-064 drafted 2026-07-14** (🔵 Planning) — the rest of EP-022, so the whole Epic is planned before activation. **SP-062:** editable viewport + per-scene `scrivi_save_scene` (debounce+switch+close/quit) + in-editor ⌘↩ create-scene / ⌘⇧↩ create-chapter (the QPlainTextEdit payoff; Apple SP-023 parity) → **AC3**. **SP-063:** scroll-driven active-scene switching + navigator↔scroll sync (Apple SP-025 parity) → AC1/AC4 scroll portion. **SP-064:** cursor/focus + non-deletable separator + quit→reopen surface restore (Apple SP-033/EP-011 parity) + full EP-022 verification → **AC4/AC5/AC6/AC7**, **closes EP-022** (Claude drafts the completion summary; Epic close awaits user approval). Tasks T-0238–T-0248. All 🔵 Planning — none activated.

---

## SP-061: [Linux] Shell flip to Widgets host + Scene navigator + read-only continuous viewport

**Status:** 🔵 Planning (next up)
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (first of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change.** The scene-load
endpoints already exist and were verified against source: `scrivi_open_project` returns the ordered
`scenes[]` list + `activeScene` body + `restored{anchor,focus,scroll}`; `scrivi_open_scene(projectRootPath,
appSupportRoot, projectID, sceneID)` returns `{scene{sceneID,chapterID,title,slug,metadataPath,
contentPath}, markdown}` (`scrivi.h:43` / `scrivi_c_api.cpp:430`). `scrivi.h` stays untouched — AC7's
additive-only clause holds trivially.

**Goal:** Turn EP-021's placeholder project window into the **editor shell**, on the architecture EP-022
committed to. Three moves: (1) **flip the app to a Qt Widgets host** — `QApplication` + `QMainWindow`
re-hosting the EP-020/021 QML (Landing / create / open / close / recents) via **`QQuickWidget`**; (2) a
**scene navigator** listing scenes in manuscript order (chapter grouping + live titles, display + select
only); (3) a **read-only continuous viewport** — every scene body in **one `QPlainTextEdit` /
`QTextDocument`** with scene-boundary markers, initial active scene applied. Delivers **EP-022 AC1 + AC2**.
Editable surface + auto-save is **SP-062**; scroll-driven active-scene switching is **SP-063**.

**Why the shell flip is here (and first):** EP-022 locked **`QPlainTextEdit`** (the `NSTextView` analogue)
for parity with the Apple writing surface — in-editor ⌘↩ createScene / ⌘⇧↩ createChapter, custom undo/redo
(`document()->setUndoRedoEnabled(false)`), cut/copy/paste hooks. On the pinned **Qt 6.4** a `QWidget`
can't embed cleanly in a QML `ApplicationWindow` (`WidgetsInQuick` is Qt 6.7+; `QGraphicsProxyWidget` is
too fragile for a live editor), so QML must live **inside** Widgets, not the reverse. Refactoring the
verified EP-021 bootstrap (`QQmlApplicationEngine`-loads-`Landing.qml`) into a `QMainWindow` shell is the
sprint's first and riskiest piece — front-loaded and re-verified before the navigator/viewport go on top.

**Design reference:** `docs/Epics/Epic-active.md` (EP-022 Design decisions + Open Questions #1/#2/#5
resolved); `ScriviCore/include/scrivi/scrivi.h` (`scrivi_open_project`, `scrivi_open_scene`);
`docs/Scrivi_Project_Package_Structure_v0_1.md` (on-disk scene layout). Apple analogues (behavior to
re-create, **not** port): `ManuscriptTextView.swift` (`NSTextView`, `sceneStartMap`/
`sceneStorageOffsetMap`, continuous document), the EP-009 scene navigator sidebar.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0234 | **Shell flip** — refactor `main.cpp` from the `QQmlApplicationEngine` bootstrap to a `QApplication` + `QMainWindow` host; embed the landing QML via `QQuickWidget`; keep the context props (`appSupportRoot`, `defaultProjectsFolder`), the `QFileDialog` folder picker, and the Quit path working; re-verify the full EP-021 landing→create→open→close→recents loop over VNC before building anything on top | High | 🔵 Backlog |
| T-0235 | **Editor shell + read-only continuous viewport** — `QMainWindow` central area housing one `QPlainTextEdit` (`setReadOnly(true)`, `document()->setUndoRedoEnabled(false)`); on open, load ordered `scenes[]` + `activeScene` body from `openProject`, loop `scrivi_open_scene` for the remaining bodies, assemble into one `QTextDocument` with scene-boundary markers; record per-scene start offsets (the `sceneStartMap` seed). Becomes the "opened project" destination, replacing `ProjectWindow.qml` | High | 🔵 Backlog |
| T-0236 | **Scene navigator** — sidebar (native `QListView`/`QTreeView` preferred; QML-via-`QQuickWidget` fallback) listing scenes in manuscript order with chapter grouping + live titles from the `openProject` scene list; clicking a scene selects it and scrolls the viewport to that scene's start offset; keyboard + pointer usable. Display + selection only (structure editing = EP-023) | High | 🔵 Backlog |
| T-0237 | **Verify (AC1/AC2) + headless smoke** — add/extend a headless smoke that opens a multi-scene fixture and asserts every scene body loaded (concatenated length / per-scene offsets), wire into CI; developer VNC click-through: open a multi-scene project → navigator lists scenes in order → click scrolls the viewport → initial active scene applied. New `.cpp/.hpp/.qml` are CMake-tracked (Linux files are **not** in the Apple pbxproj) | High | 🔵 Backlog |

**Exit criteria:** app launches as a Widgets `QMainWindow` and the **entire EP-021 flow still works**
through the `QQuickWidget`-hosted landing (create, open all 3 modes, close→landing, recents, identity,
`QFileDialog` picker, Quit) — **re-verified over VNC** (AC7 guard). Opening a `ready` project lands in the
editor shell: navigator lists every scene in order; the read-only continuous viewport shows **all** bodies
in one `QPlainTextEdit`/`QTextDocument` with scene-boundary markers; the initial active scene is applied;
clicking a navigator scene scrolls the viewport. No editing yet (read-only — that's SP-062). CI green:
ScriviCore `ctest` green, the new scene-load smoke green, macOS + EP-020/021 untouched (`scrivi.h`
unchanged). **AC1 + AC2 user-verified over VNC.**

**Open items (decide during-sprint):**
- **Navigator host (T-0236):** native Widgets list vs. a second `QQuickWidget` QML list. Lean
  native-Widgets (simpler in the new shell; EP-023 structure editing wants native drag/drop anyway).
- **Scene-boundary marker representation (T-0235):** block user-state vs. a separator block vs. a
  side-kept offset map — must survive becoming **editable/splittable** in SP-062/SP-063, not a read-only
  shortcut. This is the `sceneStartMap`/`sceneStorageOffsetMap` seed.
- **`ProjectWindow.qml` fate:** superseded by the `QMainWindow` editor shell — delete or keep transient;
  remove dead QML from the qml module if unused.

---

## SP-062: [Linux] Editable continuous viewport + per-scene auto-save + in-editor Scene/Chapter creation

**Status:** 🔵 Planning
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (second of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change.** Uses the already-present
`scrivi_save_scene` (`scrivi.h:49` — projectID, paths, markdown, selection anchor/focus, scroll,
authorship), and `scrivi_create_scene` / `scrivi_create_chapter` for the in-editor creation keystrokes
(confirm signatures at activation; the Apple app already drives them). `scrivi.h` stays untouched (AC7).
**Goal:** Make the read-only viewport from SP-061 **editable** and close the **write→save→reopen loop** —
the heart of EP-022. Typing edits the active scene's segment in the single `QTextDocument`; edits persist
per scene via `scrivi_save_scene` on a **debounce (~1.5s idle) + on scene-switch + on close/app-quit**
cadence (mirrors Apple `ProjectSession.saveAllDirty`, SP-023's 1s debounce + scene-exit + resign saves).
And — the reason `QPlainTextEdit` was chosen — the **in-editor Scene/Chapter creation keystrokes** land
here: **⌘↩** saves the current scene, calls create-scene, inserts a scene boundary, moves the caret into
the new empty segment; **⌘⇧↩** does the same for a new chapter. Delivers **EP-022 AC3**.

**Parity target (Apple SP-023):** `ManuscriptTextView`'s auto-save (1s debounce `Task`, scene-exit
immediate save, app-resign save) and its `⌘↩`/`⌘⇧↩` bindings. The Linux surface must feel identical: same
keystrokes, same "author never thinks about saving." Per-scene `scrivi_save_scene` writes individual `.md`
files at scene boundaries even though the viewport edits one document.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0238 | **Editable viewport + dirty tracking** — `setReadOnly(false)`; map caret/edits to the owning scene via the SP-061 offset map; per-scene dirty flags; keep the boundary markers non-editable/non-deletable (the separator-integrity rule) | High | 🔵 Backlog |
| T-0239 | **Per-scene auto-save** — `scrivi_save_scene` on ~1.5s idle debounce **+** on scene-switch **+** on close/app-quit (must fire on the Docker/VNC foreground-quit path); writes the real `.md`; surfaces save failures | High | 🔵 Backlog |
| T-0240 | **⌘↩ create scene** — save current scene, call create-scene, insert a boundary, move caret into the new empty segment, update the offset map + navigator | High | 🔵 Backlog |
| T-0241 | **⌘⇧↩ create chapter** — save current scene, call create-chapter, insert a boundary, move caret into the new chapter's first scene, update the offset map + navigator | High | 🔵 Backlog |
| T-0242 | **Verify AC3 + headless save smoke** — headless: edit → `save_scene` → reopen asserts the new bytes on disk; VNC: type across scenes → auto-save → switch → ⌘↩/⌘⇧↩ create → reopen shows edits + new scenes/chapters; **quit-path save** confirmed | High | 🔵 Backlog |

**Exit criteria:** the viewport is editable; edits to any scene persist via per-scene `scrivi_save_scene`
(debounce + scene-switch + close/quit) with the real `.md` updated on disk and **no data loss** on switch
or quit; **⌘↩** creates a scene and **⌘⇧↩** creates a chapter in-editor, caret landing in the new segment,
navigator + offset map updated; scene-boundary markers stay non-editable/non-deletable. CI green (new save
smoke + `ctest` unchanged; macOS/EP-020/021 untouched). **AC3 user-verified over VNC**, including the
container foreground-quit save path.

**Open items (decide at activation):** confirm `scrivi_create_scene`/`scrivi_create_chapter` C-ABI
signatures + envelopes; decide undo behavior for this sprint (document-level undo stays **disabled** —
`setUndoRedoEnabled(false)` — reserving ⌘Z for the future custom history; no in-sprint undo).

---

## SP-063: [Linux] Scroll-driven scene switching + active-scene tracking

**Status:** 🔵 Planning
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (third of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — no ScriviCore change.
**Goal:** Make the **visible/caret scene** the "active" scene and keep **navigator selection ↔ viewport
scroll** in sync — the behavior Apple gave its own sprint (SP-025), kept separate here so it doesn't
destabilize the SP-062 write loop. Scrolling the continuous `QTextDocument` past a scene boundary promotes
the neighbor to active (saving the departing scene, reusing SP-062's save path); the navigator highlight
follows the scroll; a navigator click still scrolls the viewport (from SP-061) **without** moving the caret.
Delivers the scroll-driven portion of **AC1/AC4**.

**Parity target (Apple SP-025 + SP-033):** scroll-past-boundary promotes next/previous scene + saves the
departing scene; navigator highlight updates on scroll-driven switch; navigator tap scrolls to a scene
without moving the cursor; scroll-driven tracking is distinct from cursor position.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0243 | **Scroll → active-scene promotion** — map the viewport's visible region (via the offset map) to the active scene; crossing a boundary promotes next/previous and saves the departing scene (reuse SP-062 save) | High | 🔵 Backlog |
| T-0244 | **Navigator ↔ scroll sync** — navigator highlight follows scroll-driven active scene; navigator click scrolls the viewport without moving the caret; no feedback loop between the two | High | 🔵 Backlog |
| T-0245 | **Verify** — VNC: scroll through a multi-scene project → active scene + navigator highlight track the scroll; departing scenes saved; click-to-scroll leaves the caret put; CI green | High | 🔵 Backlog |

**Exit criteria:** scrolling past a boundary changes the active scene and saves the departing one; the
navigator highlight tracks scroll position; navigator click scrolls without moving the caret; no
scroll↔selection feedback loop. CI green. **AC1/AC4 (scroll portion) user-verified over VNC.**

---

## SP-064: [Linux] Cursor/focus behavior + quit-reopen restore + EP-022 verify & close

**Status:** 🔵 Planning
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (fourth of 4 sprints — **closes EP-022**)
**Codebase:** `[Linux]` (`platforms/linux/`) only — no ScriviCore change.
**Goal:** The EP-011-equivalent **cursor/focus polish** and **quit→reopen restore fidelity**, then the
full EP-022 verification pass and Epic close. On open, the editor takes focus and the caret lands sensibly;
crossing a scene boundary places the caret correctly (no jump-to-start); the scene-boundary separator is
non-editable/non-deletable. Closing and reopening the project (via recents) restores the **last active
scene + cursor + scroll** — proving the write→save→reopen loop end-to-end, using the `restored{anchor,
focus,scroll}` payload `scrivi_open_project` already returns and the `selectionAnchor/focus/scroll` args
`scrivi_save_scene` already persists. Delivers **AC4 + AC5 + AC6 + AC7** and **closes the Epic**.

**Parity target (Apple SP-033 / EP-011):** all-in-memory viewport cursor map, correct cursor placement
after navigate/switch (I-0010 fix), reliable first-responder transfer (Apple `takeFocus()` → Qt
`setFocus`/`activateWindow`), non-editable/non-deletable virtual separator, restored active scene + surface
state on relaunch.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0246 | **Cursor placement + focus** — editor takes focus on open; sensible initial caret; correct caret across boundary crossings (no jump-to-start); enforce non-editable/non-deletable boundary markers | High | 🔵 Backlog |
| T-0247 | **Quit→reopen surface restore** — persist active scene + selection anchor/focus + scroll on close (via `save_scene`); on reopen (recents), apply `openProject`'s `restored{anchor,focus,scroll}` to select/scroll/caret the last active scene | High | 🔵 Backlog |
| T-0248 | **EP-022 verification + close prep** — full write→save→switch→scroll→quit→reopen loop over VNC; real `.md` confirmed on disk / from macOS via the shared mount; AC1–AC7 checklist; extend the lifecycle/headless smoke for surface-state restore; CI green; draft EP-022 completion summary for user close approval | High | 🔵 Backlog |

**Exit criteria:** editor takes focus on open with a sensible caret; caret is correct across boundary
crossings; boundary markers are non-editable/non-deletable; closing + reopening (recents) restores the last
active scene with its cursor + scroll; the full write→save→switch→scroll→quit→reopen loop is VNC-verified
with real `.md` on disk; **all EP-022 ACs (AC1–AC7) pass user verification**; CI green (restore smoke +
`ctest`; macOS/EP-020/021 untouched). EP-022 completion summary drafted — **Epic close awaits user
approval** (Claude does not self-close Epics).

---

## SP-056: Copy buffers — store, ABI, HUD/palette UX, history integration

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Multiple copy buffers usable for the two-buffer replacement-scan CONOPS (design §9.a); pastes are history events.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | High | 🔵 Backlog |
| T-0214 | Buffer UX: keyboard HUD + palette + Edit-menu items (T4); paste/cut history integration | High | 🔵 Backlog |

**Exit criteria:** CONOPS walkthrough verifiable; each buffer paste is one undo step; system pasteboard untouched; buffers persist across relaunch.

---

## SP-057: Undo/Redo — history panel, perf fixtures, verification & Epic close

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Management surface (Trade T2 option B), performance validation, and EP-019 acceptance-criteria verification.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0215 | History panel (T2 management surface) | Medium | 🔵 Backlog |
| T-0216 | Perf/integration fixtures (100k events, 500 KB scene); gitignore migration | Medium | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | High | 🔵 Backlog |

**Exit criteria:** all EP-019 ACs pass user verification; docs updated (`Scrivi_Project_Package_Structure`, repair matrix); Epic ready for close approval.

---

*Last Updated: 2026-07-14 (SP-060 ✅ closed; EP-021 ✅ closed. **All of EP-022 drafted** (🔵 Planning) —
SP-061 (shell flip + navigator + read-only viewport, T-0234–T-0237, AC1/AC2), SP-062 (editable +
per-scene auto-save + ⌘↩/⌘⇧↩ create, T-0238–T-0242, AC3), SP-063 (scroll-driven switching, T-0243–T-0245,
AC1/AC4 scroll), SP-064 (cursor/focus + restore + verify/close, T-0246–T-0248, AC4/AC5/AC6/AC7 — closes
EP-022). No ScriviCore work across the Epic. **None activated** — awaiting go-ahead. SP-056–SP-057 remain
🔵 Planning, parked behind deferred EP-019.)*
