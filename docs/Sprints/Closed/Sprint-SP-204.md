---
sprint: SP-204
epic: EP-026
status: Closed
closed: 2026-11-05
platform: Linux
created: 2026-11-05
---

# SP-204 — `[Linux]` [EP-026] **S3**: branching — the branch chooser, its margin mark, the History section, the stale-branch purge, Undo History settings

**Status:** ✅ **CLOSED 2026-11-05** (user, 2026-11-05: *"yes to both."*) — activated 2026-11-05 (user: *"Activate SP-204 and run step 0"*), created 2026-11-05 (user: *"plan EP-026 S3"*). ✅ Q1–Q3 ruled 2026-11-05 (as recommended); ✅ step 0 measured 2026-11-05; ✅ Q4 ruled 2026-11-05 — every question ruled.
**Carries:** [T-0662] ✅ → [`../../Tasks/Verified/Task-verified-0662.md`](../../Tasks/Verified/Task-verified-0662.md) · ➕ [I-0350] ✅ (found 2026-11-05, added explicitly: Linux CI red since [I-0346]) → [`../../Issues/Verified/Issue-verified-0341-0350.md`](../../Issues/Verified/Issue-verified-0341-0350.md).
**Epic:** [EP-026] → [`../../Epics/Closed/Epic-EP-026.md`](../../Epics/Closed/Epic-EP-026.md) — its last open Sprint. ✅ [EP-048] stays open until
EP-026 lands (user, 2026-10-11).
**Builds on:** [SP-178] (S2: undo/redo, `HistoryCapture` over `scrivi_capture_*`) · [SP-182] (S5a: Linux's Project Settings
dialog, which gains the Undo History section — EP-026 Q4).
**Authority:** [EP-019] AC4, AC5 · design §10 T2 (`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`) · [I-0320]
(the chooser is a REDO question) · [I-0321] (branches with no text folded — in the core) · [T-0620] (the branch-point mark,
Q1–Q4 ruled 2026-10-24; its Linux half is this Sprint) · [SP-167] Q1 (purge is a Project-menu ACTION, not a setting).
**Shape:** ✅ Apple's (`feedback_linux_adopts_apple_shape`).
**Size:** L — six endpoints bound, four surfaces. ⚠️ Live pass on the rig or the VM → [EP-057] at close (Sprint-GUIDELINES).

---

## What reading the code found (2026-11-05)

- ✅ **The core already does the work.** `scrivi_capture_state` reports `redoFork` ([I-0320]: present only when the redo from
  here has ≥ 2 branches), with each child's `sceneID` and `changeOffsetUtf8` ([SP-192]) — where the mark goes
  (`scrivi.h:784–786`). Folding text-less branches ([I-0321]) is the core's. Nothing in ScriviCore changes.
- ⛔ **Linux binds none of the branching endpoints.** `ScriviBridge.hpp:522–525` lists them as S3's: `scrivi_history_get_tree`,
  `_list_stale_branches`, `_purge_branch`, `_select_branch`, `_get_settings`, `_set_settings`.
- ⛔ **Linux ignores the fork today.** `EditorShell.cpp:3254`: *"`forkAhead` (the branch chooser) is S3's — redo follows the
  primary branch meanwhile."* `HistoryCapture.cpp` never reads `redoFork`.
- **Apple, the reference:**
  - **Chooser** — `ManuscriptTextView.swift:385–398` (`performRedo`): with a `redoFork`, ⇧⌘Z opens the chooser instead of
    redoing, anchored at the mark nearest the caret (at the caret when that line is off screen). Choosing →
    `selectBranch` then redo (`redoBranch`, :550); dismissing redoes nothing. Any step closes it (:457). Keyboard: ↑/↓, ↩,
    Esc (`ForkPopover.swift`). Each row: preview + timestamp.
  - **Mark** — `ManuscriptTextView.swift:557+` ([T-0620]): left margin, one per line where a branch parts, only while Show
    Markup Hints is on, never in the text; a click opens the chooser; a tooltip and an accessibility label
    (*"⇧⌘Z will ask which branch to redo"*). Rescheduled after layout changes (:813, :841).
  - **History card** — `Inspector/HistoryCard.swift`, in the Writing stack: the windowed tree (primary spine + fork glyphs,
    *"Showing N of M events"*, a this-scene / whole-project toggle), the caret's event in bold, a click on an off-spine node
    re-primaries its fork (`select`, :175 — no redo), a passive stale-branch BADGE ([I-0108]: no purge in the card). The
    tree is the project's, re-fetched on a history revision, never on a scene change ([I-0331]).
  - **Purge** — Project ▸ Purge Stale History Branches… (`StaleBranchesSheet.swift`): scan, list, confirm each purge.
  - **Settings** — `ProjectSettingsSheet.swift:64–84`: Undo History ▸ Maximum undo events, Stale after (days).
- ⚠️ **An Apple string is stale.** `HistoryCard.swift:207` says *"Across the whole project — manage in Project Settings…"*, but
  purge left Project Settings for the Project menu in [SP-167]. Linux must not copy it (→ Q3).
- ⚠️ **Linux's Writing tab is fixed sections, not a card stack** (`SceneInspector.cpp:164–195`: Tags, Outline, To-do). There is
  no card to add (→ Q1).
- ⚠️ **Linux has no left margin on the manuscript** to draw a mark in. `QPlainTextEdit::setViewportMargins` makes one; how it
  sits with the presenter's layout and the Orca reading ([EP-050]) is measured before the mechanism is ruled (→ Q4).

## Step 0 — measured (2026-11-05)

**How:** a throwaway probe (`margin_probe`, built in a COPY of the tree at `a86ac37`; nothing added to the repo) on the REAL
`ManuscriptEditor` + `SceneDocument` + `ManuscriptPresenter`, wired as `EditorShell` wires them, with
`ManuscriptAccessibility::install()` so the accessible interface is the one Orca reads ([SP-202]'s route). Docker
`scrivi-linux` (Ubuntu 24.04, Qt 6.4.2), offscreen. A 28 px gutter (a 14 px mark, Apple's 6 px gap, slack) under two
mechanisms against no margin: **VM** = `setViewportMargins(28, 0, 0, 0)` · **DM** = `QTextDocument::setDocumentMargin(4 + 28)`.

| Measured | No margin | VM (viewport margin) | DM (document margin) |
| -------- | --------- | -------------------- | -------------------- |
| Wrap at the same text width | — | ✅ identical, line for line | ⛔ differs: the margin is on all four sides (the right too) |
| Text origin | x 4, y 4 | x 4, y 4 (inside the viewport, which moved right) | ⛔ x 32, **y 32**: the top inset grows too |
| Presenter formats per block | — | ✅ unchanged (0 blocks differ) | ✅ unchanged |
| Click → caret (9 spots, across a hidden escape and markers) | 7/9 match the keyboard snap | ✅ the same 7/9 | ✅ the same 7/9 |
| A click where the mark sits | — | ✅ outside the viewport: the caret never moves | ⚠️ goes to the text: the caret jumps to the line start |
| Orca: `characterRect` → `offsetAtPoint` | 18/18 | ✅ 18/18; no rect left of the viewport | ✅ 18/18 |
| Scene divider (painted over the viewport) | spans 0..698 | ✅ spans the narrowed viewport | ✅ unchanged |
| Turning it on over ~1.86 MB, 1,186 scenes (open + show 735–752 ms) | — | ✅ **6 ms** (off: 6 ms) | ⛔ **1,693–1,720 ms**: a whole-document relayout, on the GUI thread |
| Placing marks | — | ⛔ **12 spread marks ×100: 1 ms — taken on a document the DM row had already relaid; not valid.** Re-measured on the real code in step 3 (progress log) | — |

⚠️ **Found:** `QPlainTextEdit` lays out lazily, so a block that has not been on screen has no lines. Placing a mark through
`QTextLayout::lineForTextPosition` before `blockBoundingGeometry` (which lays the block out) read an invalid line and
**crashed the probe**. The order that works: block geometry → cull off-screen → the line → `isValid()`. The code in step 3
must do the same, and `branch_smoke` checks a mark whose line has never been shown.
⚠️ **Found in step 3 (2026-11-05):** `blockBoundingGeometry` itself WALKS from the first visible block to the one asked for,
laying out every block between — so "geometry first" alone lays out the manuscript up to a far branch. ✅ The code culls by
block NUMBER against the visible range before any geometry (`ManuscriptEditor::placeBranchMarks`).


✅ **Branching undo on Linux as on the Mac:** Ctrl+Shift+Z at a branch point asks which branch, a mark shows where it will
ask, the History section shows and re-primaries branches, stale branches are purged from the Project menu, and the two
Undo History settings are in Project Settings — over the same history file, so a project branches the same on both.

## Plan

0. **Measure** (recorded here before Q4 is ruled): a left viewport margin on `ManuscriptEditor` — its effect on the
   presenter's line layout, the caret snap, the find highlight and what Orca is given; the cost of placing marks over
   dumas (~1,186 scenes) on a layout change.
1. **Bind** the six endpoints in `ScriviBridge` (envelopes as Apple decodes them); `HistoryCapture` keeps `redoFork` from
   each capture report. ⚠️ Tree fetch and stale scan run off the GUI thread (`AsyncCall.hpp`; `feedback_main_thread_is_for_ui_only`).
2. **The chooser:** Ctrl+Shift+Z (and Edit ▸ Redo) with a `redoFork` opens it, anchored at the nearest mark or the caret
   (Q2). Choose → `selectBranch` → redo through the existing apply path; Esc / click-away → nothing. Any step closes it.
   Never shown as an undo lands ([I-0320]). Replaces the stopgap at `EditorShell.cpp:3254`.
3. **The mark** ([T-0620] Q1–Q4 as ruled): left margin, one per line where a branch parts, only with Show Markup Hints
   (Ctrl+Shift+H) on; never in the text, caret stops, Find, copy or word count; click opens the chooser; tooltip + accessible
   name (*"Ctrl+Shift+Z will ask which branch to redo"*). Re-placed after an edit, a scroll and a layout change.
4. **The History section** (Q1) in the Writing tab: the windowed tree, *"Showing N of M"*, the scope toggle, the caret's event
   bold, click on an off-spine node → `selectBranch` (no redo), the stale badge with Q3's wording. Re-fetched on a history
   revision only ([I-0331]).
5. **Project ▸ Purge Stale History Branches…**: scan (off thread), list, confirm each purge; disabled with no project.
6. **Project Settings ▸ Undo History**: Maximum undo events, Stale after (days), through `_get_settings`/`_set_settings`,
   between Typography and the buttons as Apple orders it; the hint line naming the Project-menu action.
7. **Tests** — a new `branch_smoke` (`.cpp` + `.sh`), through the real key dispatch (`feedback_test_through_the_real_dispatch`):
   - undo → type → undo → Ctrl+Shift+Z opens the chooser; choosing each branch restores its text, in the editor and on disk;
     Esc redoes nothing; a plain undo never opens it;
   - a text-less branch is not offered ([I-0321]); the mark appears only with hints on, on the right line, and never enters
     the text or a save; a click on it opens the chooser;
   - History section: click re-primaries, then Ctrl+Shift+Z follows it; a scene switch does not re-fetch the tree;
   - purge removes the branch from the log; settings round-trip through the core (`project.json` is canonical — `scrivi.h`);
   - a branch made on the Mac is chosen on Linux from the same log.
   - **Mutations** (in a COPY of the repo — `feedback_no_git_stash_in_mutation_loops`): redo ignores the fork · the chooser
     opens on undo · the mark is drawn with hints off · the tree re-fetches per scene.
8. **Docs:** `ScriviBridge.hpp:522–525` comment; the ABI binding count.

## Questions — ✅ Q1–Q4 RULED 2026-11-05 (as recommended; Q4 after step 0)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Where the History card goes on Linux.** Apple's is a card in the Writing stack; Linux's Writing tab is three fixed sections. | **A fourth section, "History", at the foot of the Writing tab**, collapsible, closed by default — the same stack, the same order, no new tab (Apple has three). Alternative: build Linux a card stack first — a separate Epic's size, not this one · ✅ **RULED** (user, 2026-11-05) |
| **Q2** | **The chooser widget.** | **A `QMenu` popped at the anchor**: ↑/↓, Return and Esc, and dismiss-on-click-away come native, as Apple's popover gives them; each item *preview · timestamp*, the primary checked. Alternative: a custom popup frame — more code for the same behaviour · ✅ **RULED** (user, 2026-11-05) |
| **Q3** | **The stale badge's wording** (Apple's names Project Settings, which no longer purges). | **Both platforms say *"Across the whole project — Project ▸ Purge Stale History Branches…"***, with Apple's one-line string fixed in this Sprint (an `[Apple]` change, Mac-checked). Alternative: Linux only, and an Apple Issue filed · ✅ **RULED** (user, 2026-11-05; no Issue exists for the wording — the fix is carried by [T-0662]) |
| **Q4** | **The mark's mechanism** (step 0, above). | **VM: a 28 px viewport margin, always present, with a gutter widget the editor paints and that takes the mark's clicks.** It changes nothing in the document, the presenter, the caret or what Orca gets, costs 6 ms, and a click beside a mark can never move the caret. Always present because Apple's inset is (`textContainerInset` 60 pt, `ManuscriptTextView.swift:54`), so Show Markup Hints shows the marks and never rewraps the text. ⛔ Not DM: a 1.7 s relayout on the GUI thread, and it insets all four sides. Alternative: the margin only while hints are on (6 ms, but every toggle rewraps the page by 28 px) · ✅ **RULED** (user, 2026-11-05: *"I agree the viewport margin is the way to go."*) |

⛔ **NOT in this Sprint:** the whole-manuscript history view ([T-0621], an exploration) · any change to `scrivi.history.v1` or
to the core · Windows.

## Acceptance Criteria

- [x] AC1 — The six branching endpoints bound; `redoFork` read from every capture report; no history call on the GUI thread
      beyond what S2 already makes — ✅ `ScriviBridge` ×6; the tree, the stale scan and the settings read/write run on workers
      (`AsyncCall`); select / purge are capture calls, as Apple's (`HistoryCapture`)
- [x] AC2 — Ctrl+Shift+Z at a branch point opens the chooser (keyboard-first, preview + timestamp); choosing redoes onto that
      branch, in the editor and on disk; dismissing redoes nothing; never shown on an undo ([I-0320]); text-less branches not
      offered ([I-0321]) — ✅ `branch_smoke` §1–§2 (the folding is the core's: not re-tested here)
- [x] AC3 — The branch-point mark per [T-0620] Q1–Q4: left margin, hints on only, drawn never stored, click opens the chooser,
      tooltip and accessible name — ✅ `branch_smoke` §1, §3
- [x] AC4 — The History section (Q1): tree, scope toggle, caret's event, click re-primaries, stale badge (Q3); re-fetched on a
      history revision, never on a scene change — ✅ `branch_smoke` §4
- [x] AC5 — Project ▸ Purge Stale History Branches…: scan, list, confirmed purge — ✅ `branch_smoke` §6 (Cancel keeps, Purge
      takes, Purge never the default)
- [x] AC6 — Project Settings ▸ Undo History: Maximum undo events, Stale after (days), shared with the Mac through the project —
      ✅ `branch_smoke` §5 (through the core; `project.json` is canonical)
- [x] AC7 — `branch_smoke` in CI, mutation-checked; ctest green on macOS and Linux (second image, non-root); all Linux smokes
      green; `run-interop-tests.sh` green — ✅ (log, 2026-11-05) · ✅ **the Mac check of Apple's corrected string passed (user,
      2026-11-05)**
- [x] AC8 — Live pass (user) on the VM or the rig, against a log branched on the Mac — ✅ **passed on scrivi-vm, all steps
      (user, 2026-11-05)**, build 1.0.357 (`6c5f3ab`); log kept at `~/evidence-2026-11-05/sp204-scrivi.log` on the VM. No
      [EP-057] Task needed.

## Progress log

### 🔵 2026-11-05 — Sprint CREATED in Planning

User: *"plan EP-026 S3"*. SP-204 and [T-0662] issued via `next-id.py`. Q1–Q4 await rulings (Q4 after step 0).

### 🔵 2026-11-05 — Q1–Q3 RULED (as recommended); Q4 after step 0

User: *"There is no issue for the stale wording. SP-204 Q1–Q3: agreed. Q4 rule after the Step 0 measurement."* Q1: a
History section at the foot of the Writing tab, collapsed by default. Q2: a `QMenu`. Q3: both platforms name Project ▸
Purge Stale History Branches…; Apple's `HistoryCard.swift:207` string is fixed here, under [T-0662].

### 🟡 2026-11-05 — ACTIVATED; step 0 measured

User: *"Activate SP-204 and run step 0"*. Measured in Docker on the real editor (above): a viewport margin changes nothing
but the viewport's width and costs 6 ms; a document margin costs a 1.7 s relayout. Found the lazy-layout trap for marks on
lines never shown. ⏳ Q4.

### 🟡 2026-11-05 — Q4 RULED: the viewport margin

User: *"I agree the viewport margin is the way to go."* Built as recommended: 28 px, always present (Apple's inset is), a
gutter the editor paints that takes the mark's clicks. Steps 1–8 next.

### 🟡 2026-11-05 — Steps 1–8 BUILT; AC1–AC7 met (AC7's Mac string check and AC8 are the user's)

User: *"no no, always present is correct. yes, start step 1. In fact implement SP-204 completely if you are able."*

**Built** (`[Linux]` unless marked):
- `ScriviBridge` — `historySelectBranch`, `historyGetTree`, `historyListStaleBranches`, `historyPurgeBranch`,
  `historyGetSettings`, `historySetSettings`.
- `HistoryCapture` — `redoFork()` (fresh) / `redoForkNow()` (the last report's), `selectBranch`, `purgeBranch`; signals
  `reported` (Apple's `onReport`) and `historyChanged` (Apple's `revision`, bumped where Apple bumps it).
- `EditorShell` — `redo()` asks at a branch point ([I-0320]); `showBranchChooser` (a `QMenu`, Q2: header row, rows *preview —
  primary · when*, the primary checked and highlighted first, the keys' hint row); `redoBranch`; any step closes it;
  `refreshBranchMarks` after every report and on a Markup Hints change; `purgeStaleBranches` (Apple's sheet: scan off the GUI
  thread, list, Rescan, confirm each — Cancel the default).
- `ManuscriptEditor` — the 28 px gutter (Q4), `BranchMark` buttons (tooltip + accessible name, no focus), `placeBranchMarks`
  culled by visible block NUMBER before any geometry (see step 0's added finding).
- `HistorySection` (new) — at the foot of the Writing tab (Q1); per-device "This scene only" (Apple's `@AppStorage` key);
  fetched off the GUI thread on `historyChanged` only, while open; a caret move only moves the bold ([I-0106]).
- `ProjectSettingsDialog` — Undo History (Apple's labels and hints), read on a worker, written on close when changed,
  `idleRolloverHours` kept. `ScriviWindow` — Project ▸ Purge Stale History Branches….
- `[Apple]` `HistoryCard.swift:207` — the badge names Project ▸ Purge Stale History Branches… (Q3).
- Tests: `branch_smoke.cpp` + `.sh`, in CMake and Linux CI. ⚠️ `undo_smoke`, `structured_paste_smoke` and
  `structural_baseline_smoke` redid at what are REAL branch points (undo, then a new edit) and expected the primary — the S2
  stopgap. ✅ Each now answers the chooser with Return (the primary), as Apple's `structuralEditsUndoAndRedo` answers
  `onRedoFork` ([I-0320]).

**Found while building:**
- ⛔ `blockBoundingGeometry` walks from the first visible block (step 0's added finding, above); step 0's mark-cost row was
  taken on a relaid document. ✅ **Re-measured on the real `setBranchMarkPositions`** over ~1.86 MB, nothing relaid first
  (throwaway `mark_cost_probe`, Docker Qt 6.4.2): a mark at the end while viewing the top 0 ms · 12 spread marks ×100: 0–1 ms ·
  50 scroll steps: 110 ms with no marks, 111–115 ms with 12.
- ⛔ My own first History section rebuilt its list on every caret move (the list jumped to its top per keystroke). ✅ Only the
  bold moves (`markCaretRow`).
- ➕ [I-0350]: Linux CI red on `main` since [I-0346] — not this Sprint's (it fails with HEAD's editor), fixed in the
  accessibility smoke's harness. Added to this Sprint explicitly.

**Evidence (2026-11-05, working tree on `a86ac37`):**
| Check | Result |
| ----- | ------ |
| Docker `scrivi-linux` (Ubuntu 24.04, Qt 6.4.2): full Linux build | ✅ no errors |
| All 40 Linux CI smoke steps, non-root, `SCRIVI_VERIFY_SCENE_TEXT=1` | ✅ 0 failed (first run: 4 — the three stopgap smokes and [I-0350]) |
| `branch_smoke` mutations, in a COPY of the repo | ✅ 5/5 caught: redo ignores the branch point · the chooser on an undo · the mark with hints off · the tree re-fetched per scene · no block-number cull; the restored copy green |
| ScriviCore ctest, macOS | ✅ 806/806 |
| ScriviCore ctest, Linux (Docker, non-root) | ✅ 810/810 |
| `scripts/run-interop-tests.sh` (macOS) | ✅ 337 tests in 45 suites |
⚠️ This is evidence, not a live pass (Sprint-GUIDELINES). ⏳ AC7's Mac look at the corrected string.

### ✅ 2026-11-05 — AC8: the Linux live pass passed

The user ran the pass on scrivi-vm against a branch point made on the Mac (steps A–C as given in chat: the chooser on
Ctrl+Shift+Z, ↓/Return and Esc, no chooser on undo, the margin mark with hints on and off and its click, the History
section's re-primary, Undo History settings round-trip, purge with Cancel as default): **all steps pass**. Build 1.0.357
(`6c5f3ab`); the log has no errors and is kept at `~/evidence-2026-11-05/sp204-scrivi.log`. ✅ The Mac pass (AC7's string
check) ran first and passed too (user, 2026-11-05). ✅ Linux CI on `main` for `6c5f3ab`: run 42089629645, **success** (the
first green run since [I-0350]). **All eight ACs met.** ✅ T-0662 and [I-0350] verified; **closed 2026-11-05** (user, 2026-11-05: *"yes to both."*).

