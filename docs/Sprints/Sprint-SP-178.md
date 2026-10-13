---
sprint: SP-178
epic: EP-026
status: Active
activated: 2026-10-11
platform: Linux
created: 2026-10-11
---

# SP-178 — `[Linux]` [EP-026] **S2**: undo/redo — Ctrl+Z / Ctrl+Shift+Z, persistence, session warning, barriers, grouped edits

**Status:** 🟡 **ACTIVE 2026-10-11** (user: *"activate SP-178"*) — ⚠️ IMPLEMENTED; ✅ [I-0287] found and fixed here (user: *"Fix I-0287 in SP-178."*) — created 2026-10-11 (user: *"We are ready to plan EP-026 S2."*). ✅ Q1–Q4 ruled 2026-10-11.
**Carries:** [T-0605] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-026] `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste →
[`../Epics/Epic-active.md`](../Epics/Epic-active.md). ➡️ Unblocks [EP-048] S4b (Replace / Replace All needs grouped undo).
**Builds on:** [SP-177] (S1) — WHEN an undo step ends is ScriviCore's (`scrivi_capture_*`); Linux gets Apple's granularity by
CALLING it, not by porting it. ✅ EP-026 Q3 (keys) ruled: undo **Ctrl+Z** · redo **Ctrl+Shift+Z** · ⛔ no Ctrl+Y.
**Authority:** [`../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`](../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md)
§4.a, §4.a.1, §4.5 (barriers), §4.d, §5 (sessions), §6.b (head-hash validation), §8. EP-019 AC1–AC3, AC7; [I-0270] grouped
edits; [I-0104]/[I-0259] save-path reporting.
**Shape:** ✅ Apple's, as it stands after SP-177 (`feedback_linux_adopts_apple_shape`): `HistoryCapture.swift` (316 lines,
plumbing only) and the apply path in `ManuscriptTextView.swift:374–560`.
**Size:** L. ⚠️ Needs a **rig live pass** (Ctrl+Z / Ctrl+Shift+Z measured against GNOME; how undo feels while typing).

---

## What reading the code found (2026-10-11)

✅ **The engine and the rules are done.** Linux needs the plumbing Apple keeps — and nothing on Linux reports to history today.

| Need | Linux today |
| ---- | ----------- |
| The endpoints | ⛔ **Not bridged.** `ScriviBridge.hpp:505` — *"19 `scrivi_history_*` / `scrivi_buffers_*` endpoints are DELIBERATELY ABSENT … never been ruled"*. ✅ EP-026 rules it now; the 12 `scrivi_capture_*` (SP-177) are absent too |
| Where an edit is seen | `EditorShell::onContentsChange` (`EditorShell.cpp:1329`) — one call per gesture (every gesture is ONE `QTextCursor` edit block, [EP-049]); the pending pair is already excluded (`pendingEditInProgress`) |
| Where the caret move is seen | `EditorShell::onCursorMoved` (`:1350`) → `promoteActiveScene` |
| Paste / in-scene cut | `ManuscriptEditor::insertFromMimeData` (`ManuscriptEditor.cpp:742`), `cutSelection` (`:831`) |
| A multi-scene edit ([I-0270]) | `ManuscriptEditor::deleteAcrossScenes` (`:854`) — ⌫ / ⌦ / Ctrl+X across a scene break, one `replaceRange` per scene, back to front. ⚠️ **Several `contentsChange` calls for ONE edit** → must be ONE grouped report, not N `note_edit`s |
| Autosave | 1.5 s debounce `saveTimer_` → `saveDirtyScenes` (`:1595`); scene-switch flush in `promoteActiveScene`; `saveScene` (`:1434`) |
| Structural ops (Apple records barriers) | split = `createSceneRequested` / `createChapterRequested` (`:338–341`); `mergeScene` / `mergeChapter` (`:2082`, `:2094`); `deleteSceneByID` (`:850`), `deleteChapterByID` (`:912`) |
| Every close | ✅ ONE funnel: `ScriviWindow::closeEvent` (`ScriviWindow.cpp:565`) — the X, File ▸ Close Project and Quit all arrive there |
| Ctrl+Z | `setUndoRedoEnabled(false)` (`ManuscriptEditor.cpp:38`, `EditorShell.cpp:136, 705`) — reserved for this Epic; the Edit menu (`ScriviWindow.cpp:201`) has no Undo/Redo |

⚠️ **Two traps, both found by reading:**
1. **A structural edit RELOADS the manuscript through `load()`** (`EditorShell.cpp:1081, 1131, 1735, 1741, 1898, 1914, 1974, 2053`).
   `scrivi_history_open` **mints a new session and replaces the store** (`scrivi_c_api.cpp:3692`). Opening history from
   `load()` would make every split / merge / delete start a new session — a spurious *"previous session"* warning on the
   next undo and a dropped pending edit. ✅ History opens ONCE per project open, never on a reload (Apple's
   `reloadManuscriptFromDisk` does not reopen it either).
2. **`QPlainTextEdit` claims Ctrl+Z through `ShortcutOverride`** before a menu shortcut sees it, and Qt's
   `QKeySequence::Redo` on X11 includes **Ctrl+Y**, which Q3 ruled out. ✅ The keys are matched explicitly (Ctrl+Z,
   Ctrl+Shift+Z) and proven through the REAL dispatch (`feedback_test_through_the_real_dispatch`), not by calling a slot.

## Goal

✅ **Apple's undo on Linux, event by event:** the same keystrokes make the same undo steps (the core decides), undo and redo
restore text and caret and save, history survives a quit, undoing into a previous session warns once, structural edits stop
undo with Apple's notice, and a cross-scene delete or cut is ONE step. ✅ **Same history file as the Mac** — a project
undoes the same on both.

## Plan

1. **Bridge the S2 endpoints** (`ScriviBridge`): `scrivi_history_open`, `_validate_scene`, `_note_scene_persisted`,
   `_close` and the 12 `scrivi_capture_*`. ⛔ Only these — `get_tree`, stale branches, purge, `select_branch` and settings
   are S3's; `scrivi_buffers_*` S5's. Replace the *"deliberately absent"* note with what is bound and which Sprint binds the
   rest. Extend `bridge_parity_smoke` for each new endpoint.
2. **`HistoryCapture` (Linux)** — `platforms/linux/src/HistoryCapture.{hpp,cpp}`, one per open project, owned by
   `EditorShell`; a port of Apple's PLUMBING only: `take()` (keep the state, re-arm a single-shot `QTimer` at
   `idleDeadlineMs`, call `scrivi_capture_idle` when it fires), a monotonic `nowMs` (`QElapsedTimer`), `isApplying`,
   `canUndo` / `canRedo`. ⛔ No rule logic. Added to `platforms/linux/CMakeLists.txt`.
3. **Lifecycle:** open + `validate_scene` per loaded scene in the load WORKER after the FIRST load of a project (Q1), never on
   a reload (trap 1). ✅ **Counted in the load progress (Q1):** after the scene reads, the bar's `total` grows by 1 (open the
   history log) + N (validate each scene) — still a COUNT known before the work starts ([SP-128] §2a), never an estimate; the
   label says which phase (*"Opening undo history…"*, *"Checking undo history… k of N scenes"*). ⚠️ `history_open` is ONE
   unit: replaying the log cannot report part-way without a core change, which this Sprint does not make. Close in `ScriviWindow::closeEvent`, in Apple's order: `flush` → `saveDirtyScenes` → `stampWritingSurface` →
   `history_close` → `releaseProject`.
4. **Reports** (Apple's call sites, one for one):
   - `onContentsChange` → `note_edit(scene, whole scene text, caret UTF-16, text before)` — `QTextDocument` positions are
     already UTF-16 (SP-177 Q2). "Text before" from a per-scene copy the shell keeps (set on load, after each report and
     each apply). ⛔ Not while `isApplying`, not for the pending pair, not during a grouped edit.
   - `onCursorMoved` → `caret_moved(scene, editInFlight, applying)`; `editInFlight` set by the edit, cleared after its turn.
   - Paste / in-scene cut → `begin("paste"|"cut")` before, `flush(kind)` after.
   - `deleteAcrossScenes` → ONE `grouped_edit([{scene, before, after, caret}], "delete"|"cut")`.
   - Autosave → `before_save(now)` before the writes (never seals unless idle, T-0396);
     `note_scene_persisted(scene, bytes)` **only after a write that succeeded** ([I-0259]).
5. **Barriers** — `capture_record_barrier` BEFORE each structural op, with Apple's kinds and notes verbatim: `sceneSplit`
   *"Can't undo past creating a scene"*, `chapterSplit`, `sceneMerge`, `chapterMerge`, `sceneDelete` (scene and chapter delete).
6. **Undo / Redo** — Edit ▸ **Undo** (Ctrl+Z) / **Redo** (Ctrl+Shift+Z) at the top of the Edit menu, enabled from
   `canUndo` / `canRedo` (refreshed on `aboutToShow`), and routed from `ManuscriptEditor` (trap 2). The apply, as Apple's
   `apply(step:)`:
   - `stoppedAtBarrier` → a modal notice with the barrier's note (Apple: `NSAlert`); no text change.
   - otherwise take the pending pair out, then replace each changed scene's body under `isApplying` — raw, ⛔ never through
     `replaceRange`'s re-balancing — caret from `cursorAfterUtf16`; one change places its caret, several place it once in the
     EARLIEST scene ([SP-164]); save immediately; `note_applied`.
   - `crossedSessionBoundary` → *"Undoing changes from a previous session"* / *"You are now undoing changes made
     {yesterday at 9:42 PM}."* (Apple's `HistoryTimestamp.friendly`, ported).
   - `structuralInverse` → Q2. `forkAhead` → Q3.
7. **Tests** (Linux smokes, offscreen, keys through `QTest` into the real widget):
   the reference sentence typed = ONE step, Return its own step; Ctrl+Z / Ctrl+Shift+Z restore text, caret and the file on
   disk; ⛔ Ctrl+Y does nothing; a paste = one step; a cross-scene ⌫ = one step restoring every scene; a split stops undo
   with its note; a split's reload does NOT start a new session; close → reopen → undo crosses the session boundary once;
   reopen raises no `externalChange` barrier (the save path reported the bytes). Mutation-checked. ctest on macOS and Linux
   (Docker, non-root — `project_linux_container_tests_off`).
8. **Rig live pass** (user), steps in the chat reply — a build check first (`--version`).

## ✅ Questions — Q1–Q4 RULED 2026-10-11

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | Where history opens: `history_open` replays the on-disk log and `validate_scene` runs per scene — on a slow mount that is unbounded, the [I-0195] class | ✅ **RULED: in the load WORKER** (*"background loader, but it needs to be included in the progress bar output during load"*) — after the scene bodies are read, before `loadFinished`; ✅ counted in the progress bar (plan 3) |
| **Q2** | A history made on the **Mac** can hold `structuredCut` / `structuredPaste` nodes (EP-029). An undo that crosses one returns `structuralInverse` — the pointer has ALREADY moved — and replaying it is S4's | ✅ **RULED (*"agreed"*): until S4, step back over it and say so:** redo at once to restore the pointer, then a notice *"This step is a cross-scene cut or paste. Undo it on the Mac for now."* — history and disk stay in step. Alternative: pull the structural replay (fragment paste / uncut + reload) forward from S4 into S2 |
| **Q3** | Forks before S3: undo then type starts a new branch (the new work is primary, design §5). Without the popover a Linux writer cannot reach the old branch until S3 — it stays in the file and the Mac can | ✅ **RULED (*"accept"*): accept for one Sprint;** `forkAhead` ignored, redo follows the primary; live-pass steps avoid leaving forks (SP-177 observation 2). Alternative: bring the fork popover forward from S3 |
| **Q4** | The live-pass project | ✅ **RULED (*"agreed. Already in place."*): a COPY of a project last edited on the Mac** — the copy is already on the rig — it also proves AC6, a Mac history undone on Linux. ⛔ Never the real manuscript |

⛔ **NOT in this Sprint:** the fork popover, History card, stale-branch purge, Undo History settings section (S3) ·
structured cut/copy/paste and its structural undo (S4) · copy buffers and View ▸ Show Buffers (S5) · Replace / Replace All
([EP-048] S4b) · Windows · any change to ScriviCore, `scrivi.history.v1` or Apple. ⚠️ None is expected; if one is found it is
surfaced, not slipped in. ✅ **Two were found and ruled IN: [I-0287] (2026-10-11) — a ScriviCore + `scrivi.history.v1`
(optional `barrier.sceneTexts`) + Apple + Linux change, AC9; and [I-0288] (2026-10-12) — a ScriviCore change, AC10.

## Acceptance Criteria

- [x] AC1 — Typing on Linux makes the steps the core decides (EP-019 AC1/AC2): one step per sentence, Return its own step, no step for a caret move or an autosave — ✅ rig live pass 2026-10-13 (steps 4–12)
- [x] AC2 — Ctrl+Z / Ctrl+Shift+Z undo and redo from the keyboard and Edit ▸ Undo / Redo, restoring text, caret and the scene file; enabled only when possible; ⛔ Ctrl+Y does nothing; measured against GNOME on the rig — ✅ rig live pass 2026-10-13 (steps 4–12)
- [x] AC3 — History survives a quit; the first undo into a previous session warns once with its time (EP-019 AC3); a clean reopen raises no external-change barrier — ✅ rig live pass 2026-10-13 (steps 4–12)
- [x] AC4 — Split, merge and delete of scenes and chapters stop undo with Apple's notice (EP-019 AC7); a structural reload does not start a new session — ✅ rig live pass 2026-10-13 (steps 4–12)
- [x] AC5 — A cross-scene ⌫ / ⌦ / Ctrl+X is ONE undo step restoring every scene ([I-0270]); a paste is one step — ✅ rig live pass 2026-10-13 (steps 4–12)
- [ ] AC6 — A history made on the Mac undoes on Linux (same file); a structural node from the Mac is handled per Q2
- [x] AC7 — Smokes green through the real key dispatch, mutation-checked; ctest green on macOS and Linux — ✅ (see the 2026-10-11/12 entries; re-run green 2026-10-13: ctest Linux 724/724, 32/32 smokes)
- [ ] AC8 — ✅ **Rig live pass (user)**
- [x] AC9 — [I-0287]: after a split or merge, undoing the next edit in an affected scene returns it to its text right AFTER the op (core + Apple + Linux; tested through `scrivi_*` and Linux's real keys) — ✅ rig live pass 2026-10-13 (steps 4–12)
- [x] AC10 — ✅ **VERIFIED 2026-10-12 (user)** — [I-0288]: after an outside edit, undo in a LATER session returns the scene to the outside text — never a mix with the old text; logs written before the fix open correctly (tested through `scrivi_*`)

---

## Progress log

### 🔵 2026-10-11 — Sprint CREATED in Planning

✅ User: *"We are ready to plan EP-026 S2."* ✅ SP-178 and [T-0605] issued by `next-id.py`. ✅ Read first: Apple's
`HistoryCapture.swift` and apply path (post-SP-177), the `scrivi_capture_*` / `scrivi_history_*` ABI, and Linux's edit, save,
reload and close paths. Two traps found (reload reopening history; `ShortcutOverride` and Ctrl+Y). Q1–Q4 await ruling.

### 🔵 2026-10-11 — Q1–Q4 RULED — ready to activate

✅ User: *"Q1: background loader, but it needs to be included in the progress bar output during load.  Q2: agreed.  Q3: accept.
agreed.  Q4: agreed.  Already in place."* Q1 in the load worker, **counted in the progress bar** (plan 3 amended: +1 history
open, +N scene validations, phase named in the label) · Q2 step back over a Mac structural node with a notice until S4 · Q3
forks unreachable on Linux until S3, accepted · Q4 a copy of a Mac-edited project, already on the rig.

### 🟡 2026-10-11 — Sprint ACTIVATED (user-approved)

✅ User: *"activate SP-178"* ✅ Left `Sprint-backlog.md`; [T-0605] → active.

### 🟡 2026-10-11 — IMPLEMENTED (plan 1–7); ⛔ [I-0287] found by the new smoke — ⏳ ruling owed before the live pass

✅ User: *"Implement SP-178"*. Built in Docker (`scrivi-linux-sp166` image, no warnings).

**Done (plan 1–7):**
- **Bridge** — `ScriviBridge` binds the 16 S2 endpoints as STATIC, best-effort calls (logged, ⛔ never `errorOccurred`); the
  *"deliberately absent"* note now says what S3 / S5 still bind.
- **`HistoryCapture`** (`platforms/linux/src/HistoryCapture.{hpp,cpp}`) — plumbing only: `take()`, the idle `QTimer` at the
  core's deadline, `isApplying`, can-state, `friendlyTimestamp` (Apple's `HistoryTimestamp.friendly`).
- **Lifecycle** — opened + every scene validated IN THE LOAD WORKER on a first load only (Q1), ✅ counted in the progress bar
  (+1 log, +N validations; label *"Opening undo history…"* / *"Checking undo history… k of N scenes"*). ⚠️ **Found on the way:**
  opening history is a project WRITE, so it must run BEFORE the worker's timeline read, and the load's own caret placement must
  not be reported — either made the timeline stale and re-read on the UI thread (`open_progress_smoke`, [I-0285] AC4). Closed in
  `releaseProject()`; `ScriviWindow::flushEditor` flushes first (Apple's flush → save → close).
- **Reports** — `onContentsChange` → `note_edit` (text before from `historyText_`; caret from the change itself); caret moves;
  paste / in-scene cut bracketed by new `ManuscriptEditor` signals; `deleteAcrossScenes` → ONE `grouped_edit`; `before_save` on
  autosave; `note_scene_persisted` after each SUCCESSFUL write (incl. the split's direct head/tail writes).
- **Barriers** — `sceneSplit`, `chapterSplit`, `sceneMerge`, `chapterMerge`, `sceneDelete` (scene and chapter), Apple's notes.
- **Undo / Redo** — Edit ▸ Undo (Ctrl+Z) / Redo (Ctrl+Shift+Z), enabled on `aboutToShow`; the editor catches both keys itself
  (ShortcutOverride) and leaves Ctrl+Y to the disabled document undo. Apply = Apple's `apply(step:)`: barrier notice; Q2 step-back
  + notice for a Mac structural node; every change applied raw (never re-balanced), saved at once, caret in the earliest scene;
  `note_applied`; the session-boundary warning.

**Tests:**
- ✅ New `undo_smoke` (CI step added): a real `EditorShell`, real keys, text read FROM DISK — a sentence is one step; Ctrl+Z /
  Ctrl+Shift+Z; ⛔ Ctrl+Y nothing; a paste one step; a cross-scene ⌫ one step restoring all three scenes; close → reopen: history
  persists, the session warning shows ONCE, no barrier; a split stops undo with Apple's note, its reload keeps the SAME history.
- ✅ **Mutation-checked:** no grouping → red (cross-scene undo); history reopened on every load → red (new session after a split).
- ✅ `bridge_parity_smoke` — 52 checks (all 16 S2 endpoints through the bridge, persistence by reopening, quiet failure).
- ✅ `open_progress_smoke` updated for the new total; ✅ **all 29 other Linux smokes green.** ScriviCore unchanged (ctest not
  re-run yet).

⛔ **[I-0287] — `undo_smoke` is RED on two checks, and they are a real defect, not a test fault:** after a scene **merge**, undoing
the next edit in the survivor DROPS the merged-in text (its scene file is already deleted); after a **split**, it brings the
pre-split text back (the tail is duplicated). ✅ **Proven through `scrivi_*` alone** — the core diffs against a per-scene head that
structural ops never update; the platform's correct "text before" is used only to seed. ⚠️ **So Apple has it too.** A fix is a
ScriviCore change (+ both platforms' split/merge paths), which this Sprint said it would surface, not slip in.
⏳ **Ruling owed:** fix [I-0287] in SP-178, or file it to its own Sprint (and how `undo_smoke` stays honest meanwhile).

### ✅ 2026-10-11 — [I-0287] RULED IN and FIXED (AC9) — ready for the live pass

✅ User: *"Fix I-0287 in SP-178."*

- **Core** — a hard barrier carries `sceneTexts` (sceneID → the text the op left there); the head is reset at the barrier on
  record AND wherever it is rebuilt (`rebuildHeadCache`, `pruneInconsistentNodes`, both eviction paths), persisted in the
  barrier record. A split's new scene gets its floor at the barrier, so its head hash is still checked at the next open.
  Optional `sceneTexts` on `scrivi_history_record_barrier` / `scrivi_capture_record_barrier`. ⚠️ Deliberately NOT a floor
  re-seed (that would replay pre-op diffs onto post-op text at the next open).
- **Both platforms record the split/merge barrier AFTER the op, with the texts** (before, a split's new scene ID did not exist
  yet). Linux: `EditorShell.cpp` split / chapter split / merge (survivor text read back with `readSceneTexts`). Apple:
  `ManuscriptTextView.swift` `handleCreateScene` / `handleCreateChapter` / `handleMergeScene`. Chapter merge carries none (it only
  relocates files).
- **Tests:** `CaptureCApiTests.cpp` (merge + reopen, split head + tail, eviction past the barrier in session and from the log) —
  ✅ **5 checks red with the core fix reverted, green with it.** ✅ `undo_smoke` fully green; ✅ **red again with Linux's texts
  removed**; ✅ **M3 re-run: section 6 red with the save-path report disabled.** ✅ ctest macOS **717/717**, Linux (non-root)
  **721/721**; ✅ interop **274 tests**; ✅ **all 30 Linux smokes**; ✅ Apple app builds.
- ⚠️ Apple's half is proven by build + the core test, ⛔ not by a step on the Mac.
- ⚠️ **Seen, not fixed:** the `externalChange` repair (`HistoryStore::validateSceneHead`) re-floors the scene — the shape avoided
  above. Unmeasured whether it corrupts at the next open; not filed.


### 🟡 2026-10-12 — Apple's half of [I-0287] tested on the Mac; ⛔ [I-0288] measured and filed — ⏳ rig offline, live pass waiting

✅ User: *"do both of those things."* (the Mac check of Apple's half, and measuring the `externalChange` repair).

- ✅ **AC9, Apple:** new `UndoAfterStructuralOpTests` (`ScriviInteropTests.swift`, 3 tests): real project through `loadAsync`,
  the real coordinator's `handleCreateScene` / `handleMergeScene`, typing through `textDidChange`, `performUndo`; text checked in
  the loader and on disk. ✅ Green; ✅ **red with Apple's `sceneTexts` dropped** (split head + merge). ⚠️ First version stayed
  green under the mutation: the defect needs an edit in the scene BEFORE the op, so the test now types one.
- ⛔ **[I-0288] filed (High, backlog, ruling owed):** after an outside edit, undo in a LATER session restores
  `"Zulu. one. Alpha two."` where `"Zulu."` is correct — the repair re-floors and the next open replays the pre-change diffs.
  Repro test in `CaptureCApiTests.cpp`, ⛔ red, HIDDEN as `[.I-0288]` until ruled (ctest still 717). Not this Sprint's scope
  unless ruled in.
- ✅ ctest macOS **717/717**; ✅ interop **277** (274 + 3).
- ⏳ **AC8 live pass** waits for the rig to come back on the network.

### ✅ 2026-10-12 — [I-0288] RULED IN and FIXED (AC10)

✅ User: *"fix it inside SP-178"*.

- **Core:** the `externalChange` repair's barrier carries the disk text (`sceneTexts`, [I-0287]'s mechanism) and writes no
  `floor` record; `reseedSceneFloor` removed. ✅ **Old logs:** a `floor` record with `reason: externalChange` that directly
  follows an `externalChange` barrier is read as that barrier's text, never as a floor (`attachLoadedBarrierText`). Nothing is
  rewritten on disk.
- **Docs:** `scrivi.h` (`scrivi_history_validate_scene`), the history design doc (A.1, §4, §6.b).
- **Tests:** `CaptureCApiTests.cpp` `[I-0288]` (no longer hidden): a log from this build and a log rewritten into the pre-fix
  shape. ✅ **Mutation-checked:** legacy attach off → legacy section red; repair without the text → both red.
- ✅ ctest macOS **718/718**; ✅ Linux (GCC, non-root) **721 test cases**, `[I-0288]` confirmed run there; ✅ interop **277**;
  ✅ **all 30 Linux smokes**.
- ⚠️ Not checked on `the-stairs-of-tintagael`, which took this repair per [I-0268]. The old-log path is what opens it, and it
  is proven only on a log rewritten by hand.
- ⏳ AC8 live pass still waits for the rig.

### ✅ 2026-10-12 — [I-0288] VERIFIED (AC10)

✅ User: *"mark I-0288 as verified."* → archived to `../Issues/Verified/Issue-verified-0281-0290.md` in the same step.

### 2026-10-13 — Q2's stopgap replaced by [SP-180]

A structural node from the Mac (`structuredPaste`, `structuredCut`) is now REPLAYED on Linux: [SP-180]'s
`applyStructuralInverse`, tested by `structured_paste_smoke` §7. Q2's step-back-over and its *"Undo it on the Mac for now"*
notice are gone. ⚠️ This Sprint's live pass (AC6) should expect the replay, not the notice.

### 🟡 2026-10-13 — Rig live pass (build 73), run together with [SP-179] and [SP-180]

- ✅ **Steps 4–12 pass** (user): one step per sentence and Return its own step; Ctrl+Z / Ctrl+Shift+Z; Ctrl+Y does nothing;
  the menu items' enabled state; a caret move or autosave makes no step; the previous-session warning once after a quit; the
  split and merge notices with [I-0287]'s undo; the cross-scene delete as one step.
  - The extra Ctrl+Z at the end is the user's trailing space, which undo holds as its own step until real text follows. As
    designed.
- **AC6** (a history made on the Mac): the Mac's structured paste undid and redid on Linux, but the session warning was
  missing ([I-0298]) and the UI froze ([I-0297]). Both are being fixed in [SP-180]; AC6 waits for the re-run.
