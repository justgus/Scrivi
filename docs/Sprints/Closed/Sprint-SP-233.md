---
sprint: SP-233
epic: EP-059
status: Active
platform: Linux
created: 2026-11-21
---

# SP-233 — `[Linux]` [EP-059] A, first: the Scene Inspector's card stacks

**Status:** ✅ **CLOSED** 2026-11-22 (user, 2026-11-22: *"close SP-233.  Lets proceed to planning A3-A6."*). Was 🟡 **ACTIVE** 2026-11-21 (user, 2026-11-21: *"Q1-Q4 as recommended, implement SP-233"*); ✅ Q1–Q4 as recommended. Built 2026-11-21: AC1–AC4 met; AC5 green locally (CI on the push). AC6 ✅ (user, 2026-11-22: *"All 15 steps passed."*). ✅ AC5: CI on `c6cea5e` green (run 45060350237). Created 2026-11-21 (user, 2026-11-21: *"let's begin to plan the next Sprint in EP-059"*).
**Carries:** [T-0695] (B5, B6), [T-0491] (A7).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — the first Sprint of part **A**, with B5 and B6 brought
forward (user, 2026-11-21: *"Stack first"*; see below).
**Size:** M — a new surface for both stacks; every read and write it needs already exists.

---

## Why the stack comes first (2026-11-21)

Part A's object verbs (A5: Add Existing…, New, Remove from Scene) live **on a kind's card** on the Mac (`ObjectCard.swift`). The Mac's
Worldbuilding stack **ships empty** (`InspectorLayoutStore.makeDefault`): the writer adds the Characters card with **+**, then adds a
character in it. Linux has no cards, only one read-only tree grouped by kind, and a kind with no objects in the scene is not a row
(`SceneInspector::rebuildTree`). So a Linux writer would have **nowhere to add the first character**. The card stack (B5, B6) is
the floor A5 stands on. Ruled 2026-11-21: stack first (this Sprint), then the object verbs on a card (A3–A6, the next).

## What reading the code found (2026-11-21)

**The Mac** (`InspectorCardStackView.swift`, `InspectorLayoutStore.swift`, `InspectorCard.swift`):
- **Two stacks**, Writing and Worldbuilding, each a scrolling column of collapsible cards. A scene's cards are read from
  `inspector-layout.json` → `scenes[sceneID][stack]`, falling back to `defaultStacks[stack]`. Defaults: Writing = Tags, Outline, To-do;
  Worldbuilding = **empty** ("No cards yet. Use + to add one.").
- **Header:** **+ Add Card** (the cards this build has for the stack, by title; one already present is greyed) · **Stack Options ⋯**:
  Sort (Manual / Name / Recently Added, **per stack**, `stackSort`) and **Apply Layout to All Scenes…**.
- **A card:** a title, collapse (`collapsed`, saved), ✕ remove. A layout naming a card this build lacks is reported
  (*"This card isn't available in this version."*) with its own ✕ — never dropped silently (AC11).
- **Cards offered:** Writing: Tags, Outline, To-do, History, Sources. Worldbuilding: one per world-scoped kind, typeID
  `objects.<subdir>` (`objects.characters` …), ten today.
- **Sort** orders the CARDS (Manual = as stored, Name, Recently Added = reverse of insertion) and is handed to every card, so an object
  card orders its rows by it too (Manual = the graph's `sortIndex`).
- ⚠️ **No reorder on the Mac.** `moveCards(fromOffsets:toOffset:…)` exists in the store and **nothing calls it**; nothing sets an edge's
  `sortIndex` either. A7 says "card reorder"; the Mac has only the sort (Q2).

**Linux** (`SceneInspector.cpp`, `InspectorLayoutStore.hpp`):
- **Writing** is a fixed form: Tags, Outline, To-do, then History at the foot (`buildWritingTab`). **Worldbuilding** is one
  `QTreeWidget` of the scene's objects grouped by kind (`rebuildTree`), opened by double-click, Return or the context menu; four
  explained states (`setStatusLine`).
- `InspectorLayoutStore` reads and writes the document through the core and **patches, never reconstructs**, but
  ⚠️ *"does not model `stackSort`, `defaultStacks` or per-scene stacks"* — they round-trip untouched today.
- Kinds come from the core (`ObjectKindScope::kinds`: `kind`, `subdir`, `isWorldScoped`). So `objects.<subdir>` gives the Mac's
  typeIDs **derived, not restated** (the standing rule); `source` is project-scoped, so it is not a per-kind card (its card is C4's).

## Goal

The Scene Inspector's Writing and Worldbuilding tabs on Linux are the Mac's card stacks: the same cards per scene, read from and
written to the same `inspector-layout.json`, so a layout made on either platform shows the same on the other.

## Plan

1. **The store** (`InspectorLayoutStore`): read a scene's stack (falling back to `defaultStacks`, then Apple's defaults), add, remove,
   collapse, `stackSort`, apply to all scenes; still patching only these keys.
2. **A card stack widget** for each tab: a header (**+** menu, **⋯** menu) over a scroll area of cards; each card a header (▸/▾
   collapse, title, ✕) over its body. Every action is a button or a menu item (no gesture-only path).
3. **Writing cards** (Q1): Tags, Outline, To-do, History — today's widgets, each moved into a card unchanged.
4. **Worldbuilding cards:** one per world-scoped kind, from the core; the body is today's rows for that kind (label on every row,
   pending ⚠, the same three ways to open), sorted by the stack's sort. An empty card says the scene has none (Q3).
5. **Unknown cards** reported and removable, as the Mac.
6. **Tests:** `inspector_layout_smoke` and `scene_inspector_smoke` extended: defaults; add / remove / collapse persist and round-trip
   with keys Linux does not model left intact; a layout written as the Mac writes it shows the same cards; sort per stack; apply to all
   scenes; an unknown card; a world-scoped kind added to the core appears in + with no Linux edit.
7. **Live pass** on the Linux VM.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **The Writing tab as a card stack too?** | **Yes, both stacks.** B5 is "the writer's card stack honoured", and the Writing stack lives in the same file; a Mac writer who removed To-do should not see it on Linux. ⚠️ **One visible change:** History is not in the Mac's default Writing stack, so on Linux it appears only once added with + (it is always shown today). || ✅ As recommended (2026-11-21) |
| **Q2** | **A7's "card reorder".** | **Match the Mac: the sort only** (Manual / Name / Recently Added) and Apply Layout to All Scenes…; reword A7. The Mac has no way to reorder cards or objects (`moveCards` has no caller). Building one would be a Mac and iOS Sprint first, as you ruled for delete and promote. || ✅ As recommended (2026-11-21) |
| **Q3** | **A scene's existing objects with no card.** | **As the Mac:** Worldbuilding starts empty ("No cards yet. Use + to add one."), so a scene's characters show once the Characters card is added. Today's tree shows them with no step. No testers, so no migration (2026-11-20). || ✅ As recommended (2026-11-21) |
| **Q4** | **What a card looks like in Qt.** | **A framed section per card** (a `QFrame` with a header row and a body) in a `QScrollArea`, stacked top to bottom, as the Mac's column. A `QToolBox` (one open at a time) or tabs would not let several cards be open at once. || ✅ As recommended (2026-11-21) |

## What was built (2026-11-21)

- **The store** (`InspectorLayoutStore`): `cards`, `sort` / `setSort`, `addCard`, `removeCard`, `setCollapsed`,
  `applyStackToAllScenes` — Apple's operations, read as Apple decodes (a scene's layout without a stack's key reads empty; no layout
  → `defaultStacks`; no `defaultStacks` → Apple's defaults). Still patched: an edit rewrites only that scene's array, and each card
  entry keeps the keys Linux does not model (`kindFilter`, anything newer).
- **The card stacks** (`SceneInspector::buildStack` / `rebuildStack`): each tab is a header (**+** Add Card, **⋯** Sort ▸ Manual /
  Name / Recently Added, Apply Layout to All Scenes…) over a scrolling column of framed cards, each with ▾ collapse and ✕ (Q4). Text
  glyphs, not theme icons (SP-232's finding). A card this build lacks (the Mac's Sources, until C4) is reported — *"This card isn't
  available in this version."* — and removable.
- **Writing** (Q1): Tags, Outline, To-do and History are cards; the widgets are the same ones, moved into whichever card shows them.
  History's own header is hidden: the card's collapse opens and closes it (and its fetch).
- **Worldbuilding** (Q3): starts empty, as the Mac's. One card per world-scoped kind, from the core (`objects.<subdir>`, the Mac's
  typeIDs — no kind named in Qt). A card lists the scene's objects of its kind as SP-125's tree did (label on each row, pending ⚠,
  double-click / Return / Open Object), sorted by the stack's sort (A7, Q2); an empty card says *"No characters in this scene yet."*
  The scene-wide status line (pending worlds, unreadable index, kinds unread) is kept.
- **Shell:** `EditorShell` hands the panel every scene's ID for Apply Layout to All Scenes….

**Tests (AC5):** `inspector_layout_smoke` §5 (store: Apple's reads, each edit, keys kept, Apply to All, defaults);
`inspector_stack_smoke` (new, in CI) through a real project window: the layout's cards, an unknown card, + derived from the core,
rows and labels, sort of cards and rows, collapse and ✕ written, the History card, a second scene's own cards, Apply to All.
⚠️ `structural_baseline_smoke`'s rig loaded a HIDDEN, unsized window, and its reopen checks (I-0347, I-0348) held only by the accident
of the panels' size hints (a 26 px-wide editor on `main`). The card stacks changed those hints and both failed; ✅ the rig now sizes
and shows the window before the load, as the app does (`AppEnvironment.cpp`), and passes. No app code changed for it. `branch_smoke` §4 opened History by clicking its header, which is now hidden (the card drives it); it now opens
it the writer's way, + ▸ History then Apply Layout to All Scenes….

## Live pass (AC6) — the Linux VM

Build on the VM (`git pull`, then build as §9 of the rig doc). For a project with objects:
`./build/platforms/linux/scrivi_linux_inspector_fixture ~/StackPass.inkwyn`, then open `~/StackPass.inkwyn`. Show the Inspector.

1. Put the caret in the first scene. Click the Inspector's Writing tab: three cards, Tags, Outline and To-do, each with ▾ at the left and ✕ at the right; no History card. A + at the top left, a ⋯ at the top right.
2. Click the Worldbuilding tab: "No cards yet. Use + to add one."
3. Click +: a menu of the world's kinds, alphabetical (Artifacts, Buildings, Characters, …).
4. Choose Characters: a Characters card lists the scene's characters, each with its relationship after a dash.
5. Click + and choose Locations, then + and a kind the scene has none of: that last card says "No … in this scene yet."
6. Click +: the three kinds you added are greyed.
7. Double-click a character: the same message as before (the detail view is not built yet). Right-click a character ▸ Open Object: the same. Select one and press Return: the same.
8. Click ⋯ ▸ Name: the cards in alphabetical order. ⋯ ▸ Recently Added: the newest card first. ⋯ ▸ Manual: the order you added them.
9. Click ▾ on Characters: it collapses to its title. Click ✕ on the empty card: it goes.
10. Move the caret into another scene: Worldbuilding says "No cards yet. Use + to add one." Move back to the first scene: its cards as you left them, Characters collapsed.
11. Click ⋯ ▸ Apply Layout to All Scenes…, then move to another scene: the same cards there.
12. Click the Writing tab, then + ▸ History: a History card, open, listing the scene's history. Click its ▾: it closes.
13. Click ✕ on To-do. Move to another scene: To-do is still there. Move back: gone.
14. Quit (Ctrl+Q) and reopen the project: the cards, the collapse and the sort as you left them.
15. (If the project is reachable from the Mac) open it on the Mac: the first scene shows the same cards in both tabs.

## Acceptance Criteria

- [x] AC1 — (B5) Each scene's Writing and Worldbuilding cards are read from `inspector-layout.json`, falling back to the defaults; a
      layout written on the Mac shows the same cards on Linux; keys Linux does not model round-trip untouched.
- [x] AC2 — (B6) **+** adds any card this build has for the stack (one present is greyed); ✕ removes; collapse is saved; an unknown
      card is reported and removable. Worldbuilding's cards are derived from the core's kinds.
- [x] AC3 — (A7, as Q2) The stack's sort (Manual / Name / Recently Added) orders the cards and each object card's rows; Apply Layout to
      All Scenes…; each reachable by menu.
- [x] AC4 — A kind's card lists the scene's objects of that kind as today's tree did (labels, pending, open three ways), or says there
      are none.
- [x] AC5 — ✅ Docker (Qt 6.4, non-root): ctest **884/884**, smokes **47/47** (the new one in CI); ✅ CI on `c6cea5e` green (run 45060350237).
- [x] AC6 — Live pass on the Linux VM: all 15 steps (user, 2026-11-22: *"All 15 steps passed."*).

## Progress log

- **2026-11-22** — ✅ **CLOSED** (user, 2026-11-22: *"close SP-233.  Lets proceed to planning A3-A6."*); T-0695, T-0491 archived.
- **2026-11-22** — ✅ AC5: CI on `c6cea5e` green (run 45060350237). Every AC met.
- **2026-11-22** — ✅ AC6: all 15 steps pass on the VM (user, 2026-11-22: *"All 15 steps passed."*); T-0695, T-0491 → ✅ Verified. ⏳ CI on `c6cea5e` (run 45060350237).
- **2026-11-21** — Built: AC1–AC4 met; AC5 green locally (884/884, 47/47); two smokes' rigs corrected (see Tests); T-0695, T-0491 → 🟠. ⏳ AC6 live pass, CI.
- **2026-11-21** — 🟡 **ACTIVE** (user, 2026-11-21: *"Q1-Q4 as recommended, implement SP-233"*); Q1–Q4 as recommended; T-0695, T-0491 → active.
- **2026-11-21** — 🔵 **Created in Planning**, issued via `next-id.py`; carries [T-0695] (new) and [T-0491]. Stack first and delete /
  promote on every platform ruled (user, 2026-11-21); Q1–Q4 to rule.

*Last Updated: 2026-11-22 (✅ CLOSED.)*

*Last Updated: 2026-11-21 (🟡 **ACTIVE**; Q1–Q4 as recommended.)*

*Last Updated: 2026-11-21 (🔵 **Created in Planning**; Q1–Q4 to rule.)*
