---
sprint: SP-222
epic: EP-055
status: Active
platform: Apple
created: 2026-11-18
---

# SP-222 — `[Apple]` [EP-055] S3a: undo/redo, structural edits and cross-scene cut/copy/paste on iOS

**Status:** 🟡 **ACTIVE** 2026-11-18 (user, 2026-11-18: *"Q1-Q5 as recommended, activate and implement SP-222"*) · ✅ Q1–Q5 ruled as recommended.
**Carries:** [T-0685] (new) → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md); [T-0680] (EP-054 AC8, Q5).
**Epic:** [EP-055] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — AC5, its first half (Q1).
**Builds on:** [SP-198] (the `UITextView` host, the shared write path C1 and model sync C2 — `ManuscriptEditing.swift`), [SP-199]
(panels, gestures, toolbar); on the Mac, [SP-191] / [SP-196] (structural edits: one range call, the delta applied in memory) and
the history apply path (T-0205).
**Size:** L — three Mac host sections move to shared code, as C1 and C2 did in [SP-198].

---

## What reading the code found (2026-11-18)

- **The iOS host has no undo at all** (`ManuscriptUITextView.swift:288–289`: `undo()` / `redo()` overridden to do nothing, so the
  system's undo never acts behind the history's back — design §4). Copy, cut and paste work **within one scene** only
  (`:398–440`; `crossesScene` refuses the rest).
- **Everything AC5 needs lives in the Mac `Coordinator`** (`ManuscriptTextView.swift`), interleaved with AppKit:
  - the undo/redo apply path, `:356–549` (`performUndo` / `performRedo`, deferred steps behind an in-flight structural edit,
    [I-0319]'s earlier-session question, [I-0302]'s barrier notice);
  - the branch-point mark, `:549–955` (`HistoryForkAhead`, drawn marks, the fork popover);
  - structural edits, `:1145–1470` (`runStructuralEdit` — the ONE path for paste, split, chapter split, merge, chapter merge and
    their undo/redo; `handleCreateScene` / `Chapter`, `handleMergeScene` / `Chapter`);
  - cross-scene cut/copy/paste, `:1967–2396` (`selectionRun`, `copiedRange`, `cutAcrossScenes`, `pasteStructuredRange`);
  - the key routing that reaches them, `ManuscriptNSTextView` `:2743–2863`.
- **Go to Scene / Chapter / Manuscript Start and End already work in the iOS host** (`ManuscriptUITextView.swift:51–55`, `move`)
  — but nothing on the iPad calls them: the iPad's menu bar has File and Project only (`ScriviApp.swift:541–610`). → S3b.
- Find is `NSTextFinder` on the Mac (`ManuscriptNSTextView` `:2458`, `ManuscriptFind.swift`); iOS has `UIFindInteraction`. → S3b.

## Goal

On an iPad or iPhone the writer **undoes and redoes** exactly as on the Mac (Scrivi's history, not UIKit's), **splits and merges**
scenes and chapters and makes new ones, and **cuts, copies and pastes across scenes** — each a single undo step, writing the same
bytes as the Mac for the same action.

## Plan

1. **Shared code (design Q1 (c), extended):** move the undo/redo apply path and `runStructuralEdit` with its handlers and the
   cross-scene cut/copy/paste out of the Mac `Coordinator` into shared files, as functions of (storage, selection, loader, session)
   — the Mac host calls them exactly as today; the iOS host calls the same code. ⚠️ This edits the Mac's undo and structural paths;
   the interop suite guards them, plus a Mac live pass.
2. **iOS host:** `undo()` / `redo()` (and the keyboard's shortcut bar, ⌘Z / ⇧⌘Z, three-finger swipe) run Scrivi's history; structural
   edits and cross-scene clipboard through the shared code; the fork chooser per Q4.
3. **Surfaces (Q3):** the iPad menu bar gets Edit's Undo / Redo and the Manuscript items (New Scene, New Chapter, Split, Merge) with
   the Mac's shortcuts; the text's edit menu (long-press) offers the structural edits at the caret.
4. **T-0680 (Q5):** Import ▸ Manuscript… with a project open on the iPad asks New Project… / Into This Project, as the Mac does.
5. **Checks:** iOS tests on the simulator (`ScriviTests-iOS`) through the shared functions — undo/redo of typing, each structural
   edit and its one-step undo, a cross-scene cut and paste; the Mac interop suite unchanged and green; macOS and iOS builds; CI.
6. **Live passes (user):** iPad and iPhone; the Mac re-checked for undo, a split, a merge and a cross-scene paste.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **One Sprint or two for AC5?** AC5 is undo/redo, structural edits, Find and Go To. | **Two.** S3a (this): undo/redo, structural edits, cross-scene clipboard — they share the history and the range call and must land together (a structural edit is one undo step). **S3b:** Find and Replace (`UIFindInteraction`), Go To on the iPad's menu bar, the branch-point marks, buffers and the Format commands. | ✅ As recommended (2026-11-18) |
| **Q2** | **Move to shared code, or write an iOS copy?** | **Move**, as [SP-198] Q1 ruled for typing: two undo paths would drift as two write paths did ([EP-049]). The cost is a change to the Mac's paths, held by the interop suite. | ✅ As recommended (2026-11-18) |
| **Q3** | **Where the writer finds them without a hardware keyboard.** | **The system's own places:** UIKit's undo/redo (the shortcut bar, three-finger swipe, shake if the writer has it on) routed to Scrivi's history; the structural edits in the text's edit menu at the caret, and in the iPad's menu bar with the Mac's shortcuts. No new toolbar buttons (Q8: defaults). | ✅ As recommended (2026-11-18) |
| **Q4** | **Redo at a branch** (the history forks). The Mac asks with a popover. | **Ask the same way, in this Sprint:** a popover (iPad) / sheet (iPhone) listing the branches. Without it a redo would pick a branch silently. The marks drawn in the text wait for S3b. | ✅ As recommended (2026-11-18) |
| **Q5** | **T-0680 (EP-054 AC8) in this Sprint?** It needs exactly the structured paste this Sprint brings. | **Yes:** a small step once the paste is shared, and it leaves EP-054 waiting only on the iPhone pass and AO3. | ✅ As recommended (2026-11-18) |

⛔ **NOT in this Sprint:** Find, Go To surfaces, branch-point marks, buffers, Format commands (S3b); the navigator, inspector and
timeline (AC6).

## Acceptance Criteria

- [x] AC1 — Undo and redo on iOS run Scrivi's history: typing, a structural edit, an earlier session's change (I-0319's question), a barrier's notice (I-0302); the system's own undo never acts
- [x] AC2 — New Scene, New Chapter, Split, Merge (scene and chapter) on iOS, each one range call and one undo step, the same bytes as the Mac
- [x] AC3 — Cut, copy and paste across scenes on iOS, as the Mac (the structured clipboard)
- [x] AC4 — Redo at a branch asks which (Q4)
- [x] AC5 — The surfaces per Q3 on the iPad and iPhone
- [x] AC6 — (Q5) Into This Project on the iPad ([T-0680])
- [ ] AC7 — iOS tests for AC1–AC4 on the simulator; Mac interop green; macOS and iOS builds; CI green
- [ ] AC8 — **Live passes (user):** iPad, iPhone, and the Mac re-checked

## Progress log

- **2026-11-18** — 🔵 **Created in Planning** (user, 2026-11-18: *"…proceed with EP-055."*), issued via `next-id.py`; [T-0685] filed.
  Q1–Q5 to rule.
- **2026-11-18** — 🟡 **Activated** (user, 2026-11-18: *"Q1-Q5 as recommended, activate and implement SP-222"*); [T-0685] and [T-0680] → active.
- **2026-11-18** — **Built.**
  - **Shared (Q2):** `Scrivi/Views/ManuscriptStructure.swift` (new, in all three targets' pbxproj) — the undo/redo apply path,
    `runStructuralEdit` and its edits, the cross-scene copy / cut / paste and the structured clipboard, MOVED out of the Mac
    `Coordinator`; the Mac `Coordinator` keeps forwarders and is the Mac `ManuscriptStructureHost`. The run render's storage
    half moved to `ManuscriptSceneSync.runFits` / `spliceRun` (`ManuscriptEditing.swift`). Design doc §4a; CLAUDE.md pointer.
  - **iOS host** (`ManuscriptUITextView.swift`): AC1 the view's `undoManager` runs the history (⌘Z, the Edit menu, the keyboard's
    key, the swipe, shake), [I-0319] and [I-0302] as alerts; AC2 the structural edits through the shared code, the cue as held
    input (⛔ not `isEditable`: it dismisses the keyboard); AC3 cut / copy / paste / ⌫ across scenes, and a single-scene cut or
    paste recorded as its own history event as on the Mac; AC4 redo at a branch as an action sheet (popover on the iPad).
  - **AC5 surfaces (Q3):** the Mac's four keys as key commands (priority over the system's ⌘⌫); the text's edit menu (New Scene,
    New Chapter, and the merges where the caret allows them); the iPad menu bar's Scene and Chapter menus (`ScriviApp.swift`).
  - **AC6 ([T-0680], Q5):** `presentImportManuscript` on the iPad no longer closes the project first; after the pick,
    `importManuscriptIntoOpenProject` asks New Project… / Into This Project / Cancel, and Into This Project is the editor's
    structured paste (the Mac's code, now one shared `importManuscript(_:into:paste:)` in `ProjectArchiveFlow.swift`).
  - **AC7 so far:** macOS and iOS builds ✅, no warnings in the touched files; Mac interop 350 passed, 0 failed (2 skipped) after
    the move, and again after the render share; iOS simulator 24 passed, 0 failed — 5 new (`IOSManuscriptStructureTests`:
    the system's undo/redo runs the history; split / merge / chapter split / chapter merge each undo and redo in one step with
    the view and the disk following, and redo at a branch asks; a cross-scene cut then paste, each undone in one step; ⌫
    across scenes leaves the clipboard; input held in flight). ⏳ CI after the push. ⚠️ No mutation run on the new iOS tests.
