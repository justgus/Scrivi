---
sprint: SP-230
epic: EP-059
status: Closed
platform: Linux
created: 2026-11-20
---

# SP-230 — `[Linux]` [EP-059] E: untitled-scene titles, Go To, Navigator search

**Status:** ✅ **CLOSED** 2026-11-21 (user, 2026-11-21: *"Close SP-230"*). Was 🟡 **ACTIVE** (user, 2026-11-20: *"Q1-Q4 as recommended, activate SP-230"*); ✅ Q1–Q4 as recommended. Built 2026-11-20: AC1, AC3 met; AC2 met (keys re-ruled 2026-11-21: Ctrl+Home / Ctrl+End); AC4 ✅ (CI green on `98ee22f`); AC5 ✅ (user, 2026-11-21: *"All steps pass."*). **All ACs met — awaiting close approval.** Planned 2026-11-20 (user: *"We need to do the Linux gap epic first."*).
**Carries:** [T-0692] (E1, E3), [T-0249] (E2).
**Epic:** [EP-059] → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **E** (E1–E3), first by the Epic's order ("E and D
first; E1 is the user's example"). [SP-228] (TestFlight) waits until EP-059 is done (user, 2026-11-20).
**Size:** M — three surfaces, each with an Apple original to copy.

---

## What reading the code found (2026-11-20)

**E1 — untitled scenes.**
- **Mac:** a scene with no title shows its first non-blank line as the manuscript draws it — escapes, emphasis markers and
  heading prefixes hidden, a list prefix shown — or "Scene N" (its place in the manuscript) when the scene is blank.
  `ManuscriptPresenter.presentedFirstLine(of:)` (`ManuscriptPresenter.swift:198`, [I-0327]); used by the Navigator
  (`SceneNavigatorView.swift:776–784`), the detail sheet (`EditorView.swift:700`) and the timeline (`TimelineStripView.swift:513`).
  Kept current as the writer types (`ManuscriptEditing.swift:892`, `ManuscriptStructure.swift:960`).
- **Linux:** the Navigator says "Untitled Scene" (`EditorShell.cpp:2063`); the timeline already says "Scene N"
  (`EditorShell.cpp:4003` dots, `:4262` the Set Time Delta list), so Linux disagrees with itself; a delete confirmation says
  "this scene" (`:1083`).
- **Linux already has the rule's parts:** the presenter's per-block analysis (`ManuscriptPresenter::hiddenAtRest`) and, from it,
  the Find bar's `ManuscriptFind::PresentedText` — "the manuscript as the page shows it at rest", the Linux shape of Apple's.

**E2 — Go To.**
- **Mac:** Scene ▸ Go to Scene Start / End, Chapter ▸ Go to Chapter Start / End (no keys: every candidate is taken by an
  editing key, `ScriviApp.swift:409–413`), Project ▸ Go to Manuscript Start (⇧⌘0) / End (⇧⌘1); the iPad's are the same.
- **Linux:** none of the six. Linux has Scene, Chapter and Project menus to put them in (`ScriviWindow.cpp:788, 804, 818`).
- **[T-0249]** asked also for Page Forward / Backward; the Mac has neither (Q3).

**E3 — Navigator search.**
- **Mac** ([T-0569], `SceneNavigatorView.swift:597–760`): a Search field above the list. A chapter whose title matches keeps all
  its scenes; otherwise a scene stays when its shown title or its body matches, under its chapter. Bodies are matched as the
  writer sees them (`MarkdownEmphasis.searchable`: no escapes, emphasis markers or heading prefixes), case- and
  diacritic-insensitive, off the main thread; titles at once. While searching, every chapter is open ([SP-226] Q3).
- **Linux:** no search. `PresentedText` already holds each scene's body as the writer sees it, built after each load in idle
  slices (`FindBar.hpp:16`).

## Goal

On Linux an untitled scene is named as on the Mac everywhere a scene is named; the six Go To commands are in the menus; and the
Navigator filters by a search, with the Mac's rules.

## Plan

1. **E1:** a Linux `presentedFirstLine(text)` from the presenter's analysis of the scene's first block (Q1), used by the
   Navigator, the timeline's dots, the Set Time Delta list and the delete confirmation; refreshed when an edit changes a scene's first
   block. A smoke with the Mac's test cases (`ScriviInteropTests.swift:3827`).
2. **E2:** the six actions in Scene, Chapter and Project, on the editor's caret; keys as Q2. A smoke for each.
3. **E3:** a search field above the Navigator tree; the Mac's filter rules; bodies from `PresentedText` (Q4); every chapter open
   while searching. A smoke for the rules.
4. **Live pass** on the Linux VM.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where the first-line rule lives.** The Mac's is in Swift, in the app. | **On Linux, in C++, from the presenter Linux already draws with** — only the scene's first block is analysed, so it costs nothing at load. The rule is presentation, which the architecture keeps out of the core, and Find already took this route (`ManuscriptFind.hpp`, ruled 2026-10-11). The alternative, one core endpoint for both platforms, would change the Mac too. | ✅ As recommended (2026-11-20) |
| **Q2** | **Keys for Go to Manuscript Start / End.** | **Ctrl+Shift+0 / Ctrl+Shift+1**, the Mac's keys with Ctrl for ⌘, and the scene and chapter ones without keys, as on the Mac. ⚠️ On some layouts Shift+0 arrives as `)`; a smoke checks the shortcut fires, and if it cannot be made to, I'll stop and ask. Ctrl+Home / Ctrl+End keep working as the editor's own. | ✅ As recommended (2026-11-20) |
| **Q3** | **[T-0249]'s Page Forward / Backward.** | **Drop them.** The Mac has none, and Page Up / Down already page the editor. T-0249 is closed by E2 alone. | ✅ As recommended (2026-11-20) |
| **Q4** | **Matching bodies before `PresentedText` is built** (it is built in idle slices after a load). | **Titles match at once; bodies join when it is ready**, as the Mac's off-main scan does, with "Searching…" under the field until then. Matching is case-insensitive and ignores diacritics. | ✅ As recommended (2026-11-20) |

## What was built (2026-11-20)

- **E1** — `ManuscriptPresenter::presentedFirstLine` (the Mac's rule, on the presenter's analysis of the scene's first block) and
  `EditorShell::sceneName(i)`: a title as it is; untitled, the first line as the page shows it; blank, "Scene N". Used by the
  Navigator, the timeline's dots, the Set Time Delta list and the delete confirmation; a scene's row follows her typing
  (`refreshSceneName`, from `onContentsChange`).
- **E2** — `EditorShell::goTo` and six menu items: Scene ▸ Go to Scene Start / End, Chapter ▸ Go to Chapter Start / End, Project ▸
  Go to Manuscript Start / End (Ctrl+Home / Ctrl+End, Q2 re-ruled). A heading prefix is a stop run, so Manuscript Start lands after `## ` when the first scene opens
  with one (as on the Mac). Keys: see the finding below.
- **E3** — a Search field (with a clear button) over the Navigator tree, in a column with it (`navigatorPane_`; View ▸ Navigator
  hides both). The Mac's rules (`applyNavigatorFilter`): a chapter-title match keeps its scenes, a name or body match keeps the
  scene, others are hidden; every chapter open while searching, the writer's chapters again when cleared (none recorded as
  hers). Bodies from the Find bar's presented text (`PresentedText::sceneTexts`), matched on a worker thread, folded for case and
  diacritics; "Searching…" until the text is built (`FindBar::presentedBuilt`, Q4).
- **Tests** — `navigator_search_smoke` (new, in CI): the Mac's five first-line cases, the Navigator's names, a name following
  typing, the six Go To commands, seven searches. `chapter_titles_smoke` now finds the Navigator by its class (it is no longer
  the shell's first tree).

⚠️ **Finding, Q2 (2026-11-20):** on Linux **Ctrl+Shift+1–9 already paste copy buffers 1–9** (`BuffersPalette.cpp:59`, the buffers'
keys), so Ctrl+Shift+1 cannot be Go to Manuscript End. (The Mac's ⇧⌘1 is free because its buffers use ⌘ / ⌃⌘ / ⌥ without ⇧.)
As planned, I stopped and asked; the items ship without keys until it is ruled.
Re-ruled 2026-11-21 (user: *"Ctrl+Alt+0 / Ctrl+Alt+1 as recommended"*), then ⚠️ found taken too: Ctrl+Alt+0 / 1 are Format ▸ Body / Heading 1
(`FormatMenu.cpp:16,19`, [SP-175]). Taken on Linux: Ctrl+1–9, Ctrl+Shift+1–9, Alt+1–9, Ctrl+Alt+0–3. Asked again.
✅ **Ruled 2026-11-21** (user: *"Ctrl+Home / Ctrl+End as recommended"*): Go to Manuscript Start / End show **Ctrl+Home / Ctrl+End**.
In the editor its own Ctrl+Home / End answer first, and in the Navigator tree its own (measured: the shortcut does not reach the
action from the tree); the menu action takes them elsewhere.

**Tests (AC4, 2026-11-20):** Linux (Docker, Qt 6.4) ctest **884/884** as non-root and **44/44** CI smokes (43 + the new one). No
core or Apple code changed. ✅ CI green on `98ee22f` (Linux App CI run 44941236517, `navigator-search-ok`). macOS core ctest not rerun: no core code changed (879/879 at SP-229).

## Live pass (AC5) — the Linux VM

**A. Scene names**
1. A scene with no title whose text starts with a heading or with bold/italic: the Navigator shows its first line as the page
   shows it, without `##` or `**`.
2. A blank untitled scene is "Scene N" (its place in the manuscript).
3. Type at the start of that blank scene: its Navigator name becomes what you typed, as you type.
4. Hover a timeline dot, and open Set Time Delta…: the same names.

**B. Go To** (caret in the middle of a scene)
5. Scene ▸ Go to Scene Start, then Go to Scene End.
6. Chapter ▸ Go to Chapter Start, then Go to Chapter End.
7. Project ▸ Go to Manuscript Start, then Go to Manuscript End; the menu shows Ctrl+Home / Ctrl+End beside them.

**C. Navigator search**
8. Type part of a chapter's title: that chapter, with all its scenes.
9. Type a word from a scene's text that is not in its name: that scene, under its chapter. (Right after opening a large project
   "Searching…" may show for a moment.)
10. Type a word with an accent without it (`cafe` for café) or in capitals: still found.
11. Clear with the × in the field: every row back, and your chapters open or closed as before.

## Acceptance Criteria

- [x] AC1 — (E1) An untitled scene is named by its first line as the manuscript shows it, or "Scene N" when blank: in the
      Navigator, the timeline, the Set Time Delta list and the delete confirmation; it follows the writer's typing. The Mac's test cases pass.
- [x] AC2 — (E2) Go to Scene Start / End, Chapter Start / End and Manuscript Start / End, in the Scene, Chapter and Project menus,
      move the caret there and scroll to it; Manuscript Start / End on Q2's keys (re-ruled: Ctrl+Home / Ctrl+End).
- [x] AC3 — (E3) The Navigator's search, with the Mac's rules: chapter-title match keeps its scenes, scene title or body match
      keeps the scene, bodies as the writer sees them, every chapter open while searching, cleared by its clear button.
- [x] AC4 — Linux ctest (non-root, tests on), the smokes (new ones added to CI), macOS core ctest, CI green.
- [x] AC5 — Live pass on the Linux VM (user, 2026-11-21: *"All steps pass."*).

## Progress log

- **2026-11-20** — 🔵 **Created in Planning** (user, 2026-11-20: *"Nope.  Can't archive yet.  We need to do the Linux gap epic
  first."*), issued via `next-id.py`; carries [T-0692] and [T-0249]. Q1–Q4 to rule.
- **2026-11-20** — 🟡 **ACTIVE** (user, 2026-11-20: *"Q1-Q4 as recommended, activate SP-230"*); T-0692 and T-0249 → active.
- **2026-11-21** — ✅ **CLOSED** (user, 2026-11-21: *"Close SP-230"*); T-0692, T-0249 archived.
- **2026-11-21** — ✅ AC5 live pass (user, 2026-11-21: *"All steps pass."*); CI green; T-0692, T-0249 → ✅ Verified. Awaiting close approval.
- **2026-11-21** — Q2 re-ruled: Ctrl+Home / Ctrl+End (Ctrl+Alt+0 / 1 found taken by Format first); AC2 met.
- **2026-11-20** — Built: AC1, AC3 met; AC2 built without keys (⚠️ Ctrl+Shift+1–9 are the copy buffers on Linux: Q2 to re-rule); AC4 green locally; T-0692, T-0249 → 🟠.

*Last Updated: 2026-11-21 (✅ **CLOSED**.)*

*Last Updated: 2026-11-21 (✅ All ACs met — live pass, CI green; awaiting close approval.)*

*Last Updated: 2026-11-21 (Q2 re-ruled: Ctrl+Home / Ctrl+End; ⏳ AC5 live pass, CI.)*

*Last Updated: 2026-11-20 (Built; ⏳ Q2 keys re-rule, AC5 live pass.)*

*Last Updated: 2026-11-20 (🟡 **ACTIVE** (user, 2026-11-20: *"Q1-Q4 as recommended, activate SP-230"*).)*

*Last Updated: 2026-11-20 (🔵 **Created in Planning**.)*
