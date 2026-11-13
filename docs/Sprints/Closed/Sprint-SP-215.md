---
sprint: SP-215
epic: EP-054
status: Closed
platform: Cross
created: 2026-11-13
---

# SP-215 — `[Apple]` + `[Linux]` [EP-054] **File ▸ Import / Export**: one place for every way in and out

**Status:** ✅ **CLOSED** 2026-11-13 (user, 2026-11-13: *"close SP-215 and update CLAUDE.md to use the simulator id."*) · activated 2026-11-13 (user, 2026-11-13: *"Q3 Current Chapter…, activate and implement SP-215"*) · planned 2026-11-13 (user, 2026-11-13: *"Close SP-214 and plan the import/export and Linux start window
sprints"*). The shape is ruled (below); ✅ Q1, Q2, Q4 ruled as recommended; Q3: "Current Chapter…".
**Carries:** [T-0676] ✅ Verified → [`../../Tasks/Verified/Task-verified-0676.md`](../../Tasks/Verified/Task-verified-0676.md); [T-0678] (AC7) ✅ Verified → [`../../Tasks/Verified/Task-verified-0678.md`](../../Tasks/Verified/Task-verified-0678.md).
**Epic:** [EP-054] → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Builds on:** [SP-206] (Export / Import Archive…), [SP-207]–[SP-214] (Export Manuscript…, its formats and the scope
question, Copy as HTML), the timeline import/export (EP-021).
**Followed by:** [SP-216] — Linux's start window gets the project window's menu bar, so it mirrors THIS File menu.
**Size:** M — menus on three platforms, the scope question removed, and the iPad's timeline import/export built.

---

## Ruled before planning (user, 2026-11-13)

The user's proposal (*"…'Export Manuscript' would be misleading if the writer only wants to export a single chapter … she
would start hunting around for an 'Export Chapter' menu item"*), with the five points as recommended (*"I agree with your
recommendations for Points 1-5"*):

- **The File menu:** … **Copy as HTML** · **Import ▸** Timeline…, Archive… · **Export ▸** Manuscript…, Current Chapter…, Timeline…,
  Archive… — Copy as HTML above Import and Export.
- **Point 1:** Export ▸ **Manuscript…** and **Current Chapter…** as two items — **the scope question goes away**. Current Chapter… is the
  caret's chapter, greyed out when there is none.
- **Point 2:** the toolbar's Export button: tooltip **"Export"**; its menu matches File ▸ Export, plus Copy as HTML.
- **Point 3:** the Timeline panel keeps its own Import / Export Timeline… items.
- **Point 4:** **"Timeline"**, singular (one file in, the project's one timeline out).
- **Point 5:** Import ▸ Archive… works with no project open; Import ▸ Timeline… and every Export item are greyed out.
- ✅ **The iPad's timeline import/export is part of this work** (*"Let's make the iPad's import/export timeline functionality
  part of this work."*).

## What reading the code found (2026-11-13)

| | File menu today | Timeline import / export today |
|---|---|---|
| **Mac** | Export Archive…, Export Manuscript…, Copy as HTML, Import Archive… (`ScriviApp.swift:324`) | the Timeline panel only (`TimelineStripView.swift:1266`, `:1514`) |
| **iPad** | the same (`ScriviApp.swift:578`) + the toolbar's Export menu (`ProjectToolbar.swift:124`) | the panel's items **do nothing**: `triggerImport` / `triggerExport` are `#if os(macOS)` with no iOS branch (`TimelineStripView.swift:2122`, `:2148`) |
| **Linux** | those four + Import / Export Timeline… (`ScriviWindow.cpp:305–331`) | the File menu and the panel's menu (`TimelinePanel.cpp:1499`) |

- ⚠️ **On Apple the timeline flow lives inside the panel's view**: the open panel, the peek at the file and the epoch-offset
  sheet are `TimelineStripView` state (`:819`, `:1462`, `:2122`). The File menu cannot reach it unless it moves (→ Q1).
  Linux already holds it in the shell (`EditorShell::importTimeline` / `exportTimeline`).
- ⚠️ **The Mac's timeline export is probably silent when it fails, and may always fail:** it writes
  `atomically: true` with `try?` (`TimelineStripView.swift:2156`). Export Manuscript… found that an atomic write goes through
  a file beside the destination, which the sandbox does not grant (`ProjectArchiveFlow.swift:187`). The import's read is
  `try?` too. To be checked first, then fixed with the move (plan step 2) — a failure reported, never silent.
- **The scope question** is `askScope` (`ProjectArchiveFlow.swift:224`) on Apple and `askManuscriptScope`
  (`EditorShell.cpp:4561`) on Linux; Export Manuscript… and Copy as HTML both ask it (→ Q2).
- **The format prompt** lists ePub, which is whole-manuscript only (SP-213 Q7) — Export ▸ Current Chapter… leaves it out.
- Linux has no toolbar Export button; Point 2 is Apple's.

## Goal

Every way a writer takes work in or out is in one place, named for what it does: **File ▸ Import** and **File ▸ Export**,
the same on the Mac, the iPad and Linux. A writer who wants one chapter finds **Export ▸ Current Chapter…**; the iPad can import
and export a timeline.

## Plan

1. **Apple menus** (`ScriviApp.swift`, the Mac's and the iPad's command groups): Copy as HTML, then `Menu("Import")`,
   `Menu("Export")` per the ruling; disabled per Point 5.
2. **Apple timeline flow, out of the panel** (Q1): open → peek → epoch-offset sheet → import, and export → save; on the Mac
   the panels, on the iPad the document picker and the share sheet (as the other exports); errors shown. The panel's items,
   the File menu and the toolbar all call it.
3. **Export ▸ Manuscript… / Current Chapter…** (Apple `ProjectArchiveFlow.exportManuscript`, Linux `EditorShell::exportManuscript`):
   the scope is the item; the format prompt without ePub for a chapter; Current Chapter… greyed when the caret is in no chapter.
4. **The toolbar** (`ProjectToolbar.swift`): tooltip "Export"; Manuscript…, Current Chapter…, Timeline…, Archive…, Copy as HTML.
5. **Linux** (`ScriviWindow.cpp`): the same File menu, order and wording, as `QMenu` submenus; the panel's menu kept.
6. **Checks:** macOS build and interop; the iOS build; Linux ctest and a smoke that walks File ▸ Import / Export through the
   real menu bar (titles, order, enabled with and without a chapter at the caret) and exports a chapter without a scope
   question.
7. **Live passes (user):** the menus on the Mac, iPad and Linux; a chapter and a manuscript exported; a timeline exported and
   imported back on each, the iPad included.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where Apple's timeline flow lives** once the File menu needs it. (a) A window-level flow beside the archive and manuscript (`ProjectArchiveFlow`), the epoch sheet presented by the window: the File menu, toolbar and panel call it, and it works with the panel closed — Linux's shape. (b) The File menu opens the Timeline panel and asks it to run its own flow — less to move, but exporting would open a panel as a side effect. | **(a).** | ✅ **RULED as recommended** (user, 2026-11-13: *"SP-215 Q1, Q2, Q4 as recommended."*) |
| **Q2** | **Copy as HTML's scope question.** You ruled Copy as HTML stays a single item, so it would be the only command still asking "the whole manuscript, or only Chapter N?" (AO3 is posted a chapter at a time, so the chapter is its usual answer). (a) Keep the question; (b) Copy as HTML ▸ Manuscript / Chapter, as Export. | **(a)** — your ruling, and the question names the chapter it will copy. | ✅ **RULED as recommended** (user, 2026-11-13: *"SP-215 Q1, Q2, Q4 as recommended."*) |
| **Q3** | **The chapter item's label.** (a) "Chapter…", greyed when the caret is in none; the save panel's name says which (as now: `Chapter 2.pdf`). (b) The chapter's own name, "“Chapter 2”…", which changes as the caret moves. | **(a)** — a menu item that renames itself is harder to find, and the save panel already names it. | ✅ **RULED: (c) "Current Chapter…"** (user, 2026-11-13: *"Q3 Current Chapter…, activate and implement SP-215"*) — the user's proposal, recommended in reply. |
| **Q4** | **The iPad's timeline export** goes out through the share sheet (Save to Files, AirDrop …) as every other iPad export does; import through the document picker, filtered to `.json`. | As stated. | ✅ **RULED as recommended** (user, 2026-11-13: *"SP-215 Q1, Q2, Q4 as recommended."*) |

⛔ **NOT in this Sprint:** Linux's start window (→ [SP-216]), new formats, keyboard shortcuts for these items, world
import/export, any change to what the exports write.

## Acceptance Criteria

- [x] AC1 — **Mac and iPad:** File holds Copy as HTML, **Import ▸** Timeline…, Archive… and **Export ▸** Manuscript…,
      Current Chapter…, Timeline…, Archive…, in that order; with no project, only Import ▸ Archive… is enabled
- [x] AC2 — **Export ▸ Manuscript…** exports the whole manuscript and **Current Chapter…** the caret's chapter, with no scope
      question; Current Chapter…'s format prompt has no ePub; Current Chapter… is greyed when the caret is in no chapter
- [x] AC3 — **The toolbar's Export button** (Mac, iPad): tooltip "Export"; Manuscript…, Current Chapter…, Timeline…, Archive…, Copy as
      HTML
- [x] AC4 — **Timeline import / export** run from the File menu, the toolbar and the Timeline panel through one flow (Q1),
      **working on the iPad** (Q4); a failure is reported, never silent
- [x] AC5 — **Linux:** the same File menu, order and wording; the panel's items kept; a smoke through the real menu bar;
      ctest green
- [x] AC6 — **Live passes (user):** the menus, a chapter and a manuscript exported, a timeline exported and imported back —
      on the Mac, the iPad and Linux
- [x] AC7 — ➕ *(added 2026-11-13, [T-0678])* **The format is a submenu, not a dialog** (user: *"On Mac and Linux we present the
      export options as a horizontal row of buttons … Is there any way we can make it a vertical list of buttons like the iPad
      displays"*; option B ruled (user, 2026-11-13: *"B, add it to SP-215 as a new AC"*)): Export ▸ **Manuscript ▸** Markdown…, HTML…, Word Document…, ePub…,
      PDF… and **Current Chapter ▸** the same without ePub — on the Mac, the iPad (menu bar and toolbar) and Linux; the format
      dialog is gone; live passes on all three

## Progress log

- **2026-11-13** — 🔵 **Created in Planning** (user, 2026-11-13: *"Close SP-214 and plan the import/export and Linux start
  window sprints"*), issued via `next-id.py`; [T-0676] filed. The menu shape, Points 1–5 and the iPad's timeline ruled in
  chat; Q1–Q4 to rule.
- **2026-11-13** — ✅ **Q1, Q2, Q4 ruled as recommended** (user, 2026-11-13: *"SP-215 Q1, Q2, Q4 as recommended. For Q3, what do
  you think about 'Current Chapter...'?"*). Q3: "Current Chapter…" recommended in reply; awaiting the user's word.
- **2026-11-13** — 🟡 **Activated, Q3 ruled "Current Chapter…"** (user, 2026-11-13: *"Q3 Current Chapter…, activate and implement SP-215"*).
- **2026-11-13** — ✅ **Built** — AC5 met; AC1–AC4 built and checked in the live passes (menus and file panels have no
  harness on Apple).
  **Apple:** File's items are one view for the Mac's and the iPad's menu bar — `ImportExportCommands` (Copy as HTML, Import ▸,
  Export ▸) and `ExportMenuItems` (`ProjectArchiveFlow.swift`), the latter also the toolbar's Export menu (tooltip "Export",
  then Copy as HTML; `ProjectToolbar.swift`). `exportManuscript(_:currentChapter:)`: the item is the scope; the chapter's
  format prompt has no ePub; Current Chapter… disabled when `caretChapter` is nil. **The timeline flow left the panel (Q1):**
  `exportTimeline` / `importTimeline` / `readTimeline` / `finishTimelineImport` beside the archive; the epoch-offset sheet is
  the project window's (`EditorView.swift`, `ProjectSession.pendingTimelineImport`); the panel's items call it
  (`TimelineStripView` `onImportTimeline` / `onExportTimeline`; its dialog now serves only Adjust Epoch Offset…). The iPad:
  the document picker (`ArchivePicker.timeline`, `.json`) and the share sheet (Q4). ⚠️ **Fixed on the way:** the Mac's
  timeline export wrote atomically (refused by the sandbox beside the chosen file) and swallowed every error, as did the
  import's read — now non-atomic, and every failure is shown.
  **Linux:** File ▸ Copy as HTML, Import ▸ Timeline…, Archive…, Export ▸ Manuscript…, Current Chapter…, Timeline…, Archive…
  (`ScriviWindow.cpp`); Current Chapter… enabled as the menu opens (`EditorShell::caretChapter`); `exportManuscript(bool)`
  asks no scope; Copy as HTML still asks (Q2). `settings_smoke` §6: the File menu's order and both submenus through the real
  menu bar; Current Chapter… enabled at a chapter, its prompt without ePub, then straight to the save dialog — **each check
  fails on its mutation** (ePub put back; never enabled).
  **Evidence:** macOS build, iOS build; interop **348 tests** passed; package boundary, stub parity and ABI guards clean.
  scrivi-vm (Qt 6.10.2): ctest **847/847**; settings, authorship, buffers, chapter-titles, find, format and indent smokes
  pass. ⏳ AC6: the live passes.
- **2026-11-13** — ✅ **AC1–AC4, AC6 — live passes** (user, 2026-11-13: *"Mac steps 1-8 pass. iPad steps 1-5 pass. Linux steps 1-6 pass."*). AC1–AC6 met; T-0676 → ✅
  Verified, archived. Close awaits approval.
- **2026-11-13** — ➕ **AC7 added** (user, 2026-11-13: *"B, add it to SP-215 as a new AC"*): the export formats as submenus (option B — the platforms' own vertical list, Pages'
  *Export To ▸* convention; the per-format descriptions are dropped). [T-0678] filed, active.
- **2026-11-13** — ✅ **AC7 built** (live passes pending). **Apple:** `ExportMenuItems` — Export ▸ **Manuscript ▸** and
  **Current Chapter ▸** submenus of `ExportFormat` (Markdown…, HTML…, Word Document…, ePub…, PDF…; the chapter's without
  ePub), the toolbar's Export menu with them; `exportManuscript(_:format:currentChapter:)` asks nothing (`ProjectArchiveFlow.swift`).
  **Linux:** the same submenus (`ScriviWindow.cpp`), `EditorShell::exportManuscript(format, currentChapter)`; the format
  dialog is gone on both. `settings_smoke` §6 rewritten: both submenus' items, Current Chapter enabled at a chapter, its PDF…
  straight to the save dialog — fails on its mutations (ePub in the chapter's; never enabled). **Evidence:** macOS and iOS
  builds; interop 348 passed; package boundary clean; scrivi-vm ctest 847/847; settings, authorship, buffers, chapter-titles,
  find, format and bridge-parity smokes pass. ⏳ AC7: the live passes.
- **2026-11-13** — ✅ **AC7 — live passes** (user, 2026-11-13: *"mac ipad linux all pass."*). AC1–AC7 met; T-0678 → ✅ Verified, archived.
  Close awaits approval.
- **2026-11-13** — ✅ **CLOSED** (user, 2026-11-13: *"close SP-215 and update CLAUDE.md to use the simulator id."*). Next: [SP-216], Linux's start window gets this menu bar.
