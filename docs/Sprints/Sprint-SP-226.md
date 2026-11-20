---
sprint: SP-226
epic: EP-055
status: Active
platform: Cross
created: 2026-11-19
---

# SP-226 — `[Cross]` [EP-055] S6: collapsible chapters in the Navigator

**Status:** 🟡 **Active** (user, 2026-11-19: *"Q1-Q6 as recommended.  Activate SP-226 now."*). Q1–Q6 ruled as recommended.
**Carries:** [T-0657] (rulings of 2026-11-03 in the Task; moved into EP-055 AC6 by Q11, 2026-11-19).
**Epic:** [EP-055] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — the last of AC6. S5's iPad pass ([SP-225]) found nothing,
so S6 is T-0657 alone.
**Builds on:** T-0569 (the Navigator's search), SP-073 (chapter and scene drag on both platforms), SP-144 / I-0234 (the workspace-state
write budget), SP-199 (the iOS Navigator measurement).
**Size:** L — a core field and endpoint, the Apple Navigator (Mac and iOS), and the Linux Navigator (Linux adopts Apple's shape in the
same work).

---

## What reading the code found (2026-11-19)

**The core.**
- `workspace-state.json` is per device, under app support (`state/projects/<projectID>/`), not in the package
  (`WorkspaceStateService.cpp:11`). It holds the last writing surface, and every open-scene call rewrites it
  (`WorkspaceStateService::save`, read-modify-write of the whole struct).
- It reaches the app inside the open-project result (`OpenProjectResult.workspaceState`, `Results.hpp:45`). ⚠️ **No endpoint writes it
  on its own.** The only writer is `scrivi_open_scene` (`scrivi.h:166`), as a side effect.
- So T-0657 needs: one field (`expandedChapterIDs`) in `WorkspaceState`, its JSON, the open result's envelope, and **one new endpoint**
  to set it. ⚠️ Every save must carry the field, or the next open-scene write drops it (the struct passes through `load` → `save`, so it
  survives once both copy it). ABI 135 → 136; the stub-parity and project-guard checks cover the new one.

**Apple (`SceneNavigatorView.swift`, 880 lines).**
- One flat `List` over `flatRows`: chapter headers interleaved with scene rows, so a single `.onMove` reorders both (`:5`, `:120`,
  `:710`). Collapsing is a filter in `flatRows` that drops a collapsed chapter's scene rows; the header row stays.
- Search (T-0569) already filters `flatRows` (`:724`) and turns reorder off while filtering (`:129`).
- The list follows `loader.viewportSceneID` with `scrollTo(anchor: nil)` (`:241`). ⚠️ A scene in a collapsed chapter has no row to
  scroll to, so the viewport's chapter must be open before that scroll fires. The rulings already require it (the current chapter is
  always open).
- A drop reads its predecessor from `flatRows` (`performMove`, `:469`). A drop just under a header makes the scene that chapter's
  **first** scene, which stays true for a collapsed header (Q4).

**Linux (`EditorShell::rebuildNavigator`, `EditorShell.cpp:2008`).**
- Already a real tree: `QTreeView`, chapter items with scene children, and `navigator_->expandAll()` (`:2049`) after **every** rebuild.
  Every structural edit rebuilds. Collapsing is native; the work is to stop `expandAll`, restore the set after each rebuild, and record
  the writer's `expanded` / `collapsed`.
- ⛔ Linux's Navigator has no search, so Q3 is Apple-only.

## Goal

The Navigator shows the chapters, with the current chapter open and any the writer opened herself. Her choices survive a relaunch on
that device. Expand All and Collapse All are one action each. On dumas the iPad's first Navigator show is no longer a multi-second stall.
The same on the Mac, iPad, iPhone and Linux.

## Plan

1. **Core:** `expandedChapterIDs` in `WorkspaceState` and `scrivi.workspaceState.v1` (absent = none); returned in the open-project
   envelope; `scrivi_set_expanded_chapters(projectRootPath, appSupportRoot, projectID, chapterIDsJSON)` writes it, behind a project
   guard. Unit tests: round trip; an `open_scene` write keeps the field; a missing field reads as empty. A boundary test through
   `scrivi_*`, not the facade.
2. **Apple:** the stored set is loaded at open. Shown open = stored ∪ {the viewport's chapter}. A chevron on each header toggles the
   stored set (Q5), written off the main thread on each toggle (Q2). `flatRows` drops a closed chapter's scenes. Expand All and
   Collapse All per Q6.
3. **Linux:** the same: no `expandAll`; after each rebuild expand stored ∪ current; `expanded` / `collapsed` update the set and write it;
   the current chapter re-expands if collapsed; Expand All and Collapse All as on Apple.
4. **Measure:** T-0657's method (an iOS test hosting `SceneNavigatorView` on a copy of dumas, `layoutIfNeeded` timed); the 2026-11-03
   baseline is ~75 ms on the simulator against 12 ms for a bare List.
5. **Checks:** ctest (macOS; Linux in Docker as non-root); Mac interop (`scripts/run-interop-tests.sh`, N > 0); iOS tests; macOS and iOS
   builds; the Linux build and smokes in Docker; the CI guard scripts; CI after the push.
6. **Live passes (user):** the Mac, iPad, iPhone, Linux (the VM). The relaunch steps kill the app first.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **A project opened on a device with no stored choice** (every project today, and every new one). | **All chapters closed except the current one.** It is the case that is slow today, and Expand All is one action away. || ✅ As recommended (2026-11-19) |
| **Q2** | **When a toggle is written.** | **On every toggle, off the main thread,** through the new endpoint. One small file; nothing to lose on a kill; the main thread never waits on it. || ✅ As recommended (2026-11-19) |
| **Q3** | **Search with collapsed chapters** (Apple only). | **Search ignores collapse:** while there is a query, every chapter with a match shows its matching scenes, and the writer's set is untouched; clearing the query restores it. A match hidden in a closed chapter would defeat the search. || ✅ As recommended (2026-11-19) |
| **Q4** | **Dragging onto a collapsed chapter.** | **The current rules, unchanged:** a scene dropped just under a header (open or closed) becomes that chapter's first scene; a chapter drags as a unit, open or closed. No auto-expand while dragging (SwiftUI's List has none). || ✅ As recommended (2026-11-19) |
| **Q5** | **The current chapter's chevron.** The ruling keeps it open, but she may want it to *stay* open after she leaves. | **Its chevron marks it as hers:** tapping it on the current chapter adds it to (or removes it from) her set, and the chevron shows which. It never closes while she is in it. Without this, the only way to keep the chapter she is writing in open is to leave and reopen it. || ✅ As recommended (2026-11-19) |
| **Q6** | **Where Expand All and Collapse All live.** | **The chapter header's context menu** (both platforms) **and View ▸ Expand All Chapters / Collapse All Chapters** (Mac and iPad menu bar; Linux View menu). Collapse All keeps the current chapter open, per the ruling. || ✅ As recommended (2026-11-19) |

⛔ **NOT in this Sprint:** rules for auto-collapsing chapters (the user's to define later, T-0657); a search for Linux's Navigator; the
Navigator's compact-width layout beyond what the iPhone already does (S7).

## Acceptance Criteria

- [x] AC1 — **Core:** the expanded set round-trips through `workspace-state.json`, survives `scrivi_open_scene`'s writes, and reads as
      empty when absent; the new endpoint is guarded and tested through the C ABI; ctest green on macOS and Linux (Docker)
- [x] AC2 — **Apple:** only stored ∪ the current chapter show their scenes; the current chapter is always open, and the list still
      follows the viewport into a closed chapter (it opens); the chevron toggles (Q5); Expand All and Collapse All (Q6); search per Q3;
      drag per Q4
- [x] AC3 — **Linux:** the same shape; a structural edit (which rebuilds the tree) keeps the set
- [ ] AC4 — The set survives killing and relaunching the app, per device (Q1 for a project with none)
- [x] AC5 — **Measured:** the iOS Navigator's first show on dumas, before and after, by T-0657's method
- [ ] AC6 — iOS tests, Mac interop, macOS and iOS builds, the Linux build and smokes, the guard scripts, CI green
- [ ] AC7 — **Live passes (user):** the Mac, iPad, iPhone, Linux (the VM)

## Live-pass steps (AC4, AC7)

Run on the Mac, the iPad, the iPhone and the Linux VM (pull and build on the VM first).

**A. First open (Q1)**
1. Open a project with several chapters that you have not opened since this build.
2. Only the chapter you are writing in shows its scenes. Every other chapter shows its header only.

**B. Opening and following**
3. Click or tap the ▸ on another chapter. Its scenes appear.
4. Scroll the manuscript into a third chapter. The Navigator opens that chapter and follows you there.
5. The chapter you left closes. The chapter you opened in step 3 stays open.

**C. The current chapter's chevron (Q5)**
6. Click or tap the chevron on the chapter you are in now. It stays open. On Apple the chevron turns to the accent colour; on Linux the italics go away.
7. Scroll back into the chapter from step 3. The chapter from step 6 stays open after you leave it.

**D. Search (Q3, Apple only)**
8. Type a word from a scene in a closed chapter into the Navigator's search field. The matching scene shows under that chapter.
9. Clear the search. Your open and closed chapters come back as they were.

**E. Expand All and Collapse All (Q6)**
10. Choose View ▸ Collapse All Chapters. Every chapter closes except the one you are in.
11. Choose View ▸ Expand All Chapters. Every chapter opens.
12. Right-click (Mac, Linux) or long-press (iPad, iPhone) any chapter header. The menu offers Expand All Chapters and Collapse All Chapters.

**F. Dragging (Q4, Mac and Linux)**
13. Close a chapter with its chevron. Drag a scene onto that closed chapter's header row. The scene becomes that chapter's first scene, and the chapter does not open while you drag.

**G. Kill and relaunch (AC4)**
14. Collapse two chapters with their chevrons, so a mix of open and closed remains. Note which are open.
15. Stop the app in Xcode or force-quit it (on Linux, quit it). Relaunch and reopen the project.
16. The same chapters are open as before the quit, plus the one you are writing in.

**H. Per device and Linux only**
17. On a second device, open the same project. It starts with only the current chapter open: the choices are kept per device.
18. Linux only: split a scene (a structural edit). The open and closed chapters stay as they were.
19. Linux only: on a chapter open only because you are in it, the title is in italics, and hovering shows a tooltip saying so.

## Progress log

- **2026-11-19** — 🔵 **Created in Planning** (user, 2026-11-19: *"plan SP-226"*), issued via `next-id.py`; carries [T-0657]. Q1–Q6 to
  rule.
- **2026-11-19** — 🟡 **Activated** (user, 2026-11-19: *"Q1-Q6 as recommended.  Activate SP-226 now."*); Q1–Q6 ruled as recommended. [T-0657] → `Task-active.md`.
- **2026-11-19** — 🔨 **Built** (user, 2026-11-19: *"implement SP-226"*). [T-0657] → 🟠 `Task-unverified.md`.
  - **Core (AC1):** `expandedChapterIDs` in `WorkspaceState` / `scrivi.workspaceState.v1` (omitted when empty) and in the open
    envelope; `scrivi_set_expanded_chapters` under `SCRIVI_PROJECT_WRITE` (ABI 136 project endpoints, all guarded).
    ⚠️ **Found while building:** `scrivi_save_scene` (every autosave, through the write queue) wrote `workspace-state.json` from
    a FRESH struct, not read-modify-write — it would have erased the set on the next keystroke's save. `SceneWriter` now starts
    from the stored state (`storedOrEmpty`). No race with the queue: a queued save runs `scrivi_save_scene` under the project
    lock, and the new endpoint waits for earlier queued jobs (W3). Tests through the C ABI: round trip, survives `open_scene`
    and `save_scene`, `[]` clears, non-arrays refused; schema round trip and absent = none. ctest macOS **879/879**.
  - **Apple (AC2; AC4 by reopen in the test — the kill and relaunch is the live pass):** the set on `ViewportSceneLoader` (`setChapter`, `expandAllChapters`, `collapseAllChapters`; each write
    detached, chained after the last). `flatRows` gives a closed chapter its header only; open = hers ∪ the viewport's
    (Q1); search ignores collapse (Q3); `performMove` unchanged (Q4). `ChapterChevron` (Q5): ▸/▾, on the current chapter tinted
    when hers, dimmed when open only because she is in it; disabled while searching; 44 pt on iOS. The follow-scroll is
    deferred one turn so a newly opened chapter's row exists. Expand / Collapse All in the header's context menu and View on
    Mac and iPad (Q6). iOS test `NavigatorChapterTests` (toggle, Collapse All, Expand All each reach the next open).
  - **Linux (AC3):** `expandAll()` removed; `applyChapterExpansion()` after every rebuild (hers ∪ current; italic + tooltip
    when open only because current); `QTreeView::expanded` / `collapsed` record her choice (collapsing the current chapter
    toggles hers, Q5); `selectNavigatorScene` follows the chapter; writes on a worker, one at a time, the latest last; a
    structural reload keeps the window's set. Context menu and View ▸ Expand All / Collapse All Chapters.
    `chapter_titles_smoke` §6.
  - **AC5 measured** (a temporary probe, removed, as SP-199's was): the Navigator's first show on dumas-prose (1,153 scenes,
    49 chapters) in the iPhone 18 Pro simulator, median of 5 — **every chapter open 65 ms → collapsed 23 ms** (the bare-List
    floor was 12 ms in SP-199).
  - **AC6:** iOS **36/36**; Mac interop **352/352**; macOS and iOS builds ✅; the six guard scripts ✅; Linux (Docker, Ubuntu 24.04, non-root): ctest **884/884**, all **44** CI smokes ✅ (`chapter_titles_smoke` §6 included). ⏳ CI on the push.
  - ⏳ **AC7:** the live passes (Mac, iPad, iPhone, Linux VM).

*Last Updated: 2026-11-19 (🔨 **Built**; ⏳ AC7 live passes.)*

*Last Updated: 2026-11-19 (🟡 **Activated** (user, 2026-11-19: *"Q1-Q6 as recommended.  Activate SP-226 now."*).)*

*Last Updated: 2026-11-19 (🔵 **Created in Planning** (user, 2026-11-19: *"plan SP-226"*).)*
