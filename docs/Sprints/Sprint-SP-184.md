---
sprint: SP-184
epic: EP-048
status: Planning
platform: Linux
created: 2026-10-15
---

# SP-184 — `[Linux]` [EP-048] **S5c**: Show chapter titles, and the divider that closes a chapter

**Status:** 🔵 **PLANNING** — created 2026-10-15 (user: *"plan EP-048 fully."*). ✅ The S5 split was ruled 2026-10-15
([SP-182] Q2). ✅ Q1–Q2 ruled 2026-10-15, both as recommended.
**Carries:** [T-0611] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-048] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). ✅ [EP-026] Q4 (ruled 2026-10-11): EP-048 S5 builds
the dialog **with the "Show chapter titles" toggle** (deferred since [EP-023]).
**Builds on:** [SP-182] (S5a): the dialog and `ProjectPreferences`.
**Authority:** [EP-047] P9 (chapter titles at Heading 1 size, in the text colour) · [T-0575] (the chapter-ending divider) ·
`Scrivi/Views/ManuscriptTextView.swift:787–823` (`rebuildStorage`).
**Shape:** ✅ Apple's (`feedback_linux_adopts_apple_shape`).
**Size:** M — every structural operation touches chapter boundaries. Needs a **rig live pass**.

---

## What reading the code found (2026-10-15)

- **Apple** (`rebuildStorage`, `ManuscriptTextView.swift:787–823`):
  - A divider sits between EVERY pair of adjacent scenes. The one that closes a chapter is drawn at full strength
    (`.chapterEnd`, [T-0575]).
  - The chapter title is drawn **only when `showChapterTitles` is on**, and its default is **off**
    (`ProjectPreferences.swift:123`, `?? false`).
  - Untitled chapters read *"Chapter N"*, by order.
- ⛔ **Linux differs twice:**
  1. It **always** draws a chapter heading at every chapter boundary, so a project with no setting shows titles on Linux and
     none on the Mac.
  2. It draws **no divider** at a chapter boundary. `SceneDocument.hpp:328–334` calls that *"macOS parity, where the
     DividerAttachmentCell sits only between same-chapter scenes"*. ⚠️ That comment predates [T-0575] (2026-10-02) and the
     toggle, and is now wrong.
- ✅ **The heading is presentation.** `SceneDocument`'s offset map is the authority for which scene owns a position, and the
  heading plus its blank lines lie outside every body (`isEditableRange` refuses them). The boundary text is written in a
  few places (`build`, `insertSceneAfter`, `reflowBoundaryAt`, `reflowAllChapterHeadings`, `setChapterTitle`, and the
  moves and removes that call them), so "no heading" is a change concentrated in `SceneDocument`.
- ⚠️ **These read the heading or rely on it** and must be checked with titles off:
  - Rename Chapter, which rewrites the live heading (`EditorShell.cpp:1302–1329, 1450`).
  - New chapter (Ctrl+Shift+Return) and its caret placement ([I-0300], fixed in [SP-180], ⏳ rig).
  - Split, merge and delete of scenes and chapters, and their undo.
  - Structured paste across chapters ([SP-180]; Q4 there: the flat text carries no heading).
  - Find, which never searches chapter titles ([SP-176]).
  - The separator rule painted from `sceneSeparatorPositions()`.
  - The navigator and the timeline read `chapterHeadingText`, which is unaffected.

## Goal

✅ **A project's chapter titles show on Linux exactly when they show on the Mac**, with the same full-strength divider closing
each chapter, and every structural operation behaves the same either way.

## Plan

1. **Inventory** every reader of the boundary text (`grep` across `platforms/linux/src`), recorded here before changing any.
2. **`SceneDocument` builds with or without headings.** A chapter boundary always holds the closing divider, painted at
   full strength in the accent at 60% ([T-0575]) beside the existing scene rule. The heading block is present only when
   titles are on (Heading 1 size, text colour, the project face, P9).
3. **The toggle:** the dialog gains Writing Surface ▸ *Show chapter titles in manuscript* (Apple's label and place). A
   toggle rebuilds the presentation only: caret and scroll kept, no disk write, no history event.
4. **Tests** (a new `chapter_titles_smoke`), with titles off AND on:
   - New chapter, split, merge, delete, rename and structured paste, and the undo of each, checked in the editor and on
     disk.
   - The caret after a new chapter ([I-0300]).
   - Find skips titles.
   - Save bytes unchanged by a toggle; the divider positions.
   - A Mac-written `showChapterTitles` is read.
   - **Mutations:** a heading drawn when off · no divider at a chapter end · a toggle that dirties a scene.
5. **Live pass** (user, rig), against the same project on the Mac.

## Questions — ✅ Q1–Q2 RULED 2026-10-15 (as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **The default for a project with no setting.** Apple's is OFF, so every existing Linux manuscript loses its chapter headings on first open. | ✅ **Apple's (off).** The setting travels with the project, so any other default makes one project look different on each machine; the writer turns titles on once, in Project Settings, and both platforms follow. Alternative: Linux keeps titles on until the project sets the key (the two platforms disagree until then) · ✅ **RULED** (user, 2026-10-15) |
| **Q2** | **The chapter-ending divider ([T-0575]) on Linux.** Not named in EP-048 or EP-026, but with titles off nothing else marks where a chapter ends. | ✅ **In this Sprint.** It is what Apple draws at that boundary, and without it Q1 leaves chapters invisible on Linux. Alternative: a separate Task · ✅ **RULED** (user, 2026-10-15) |

⛔ **NOT in this Sprint:** chapter titles editable in place (Apple's are drawn, not edited; Rename Chapter stays the path) ·
any change to how untitled chapters are numbered.

## Acceptance Criteria

- [ ] AC1 — Writing Surface ▸ Show chapter titles in manuscript, read from and written to `project-settings.json` (absent per
      Q1); a project shows titles on Linux exactly when it does on the Mac
- [ ] AC2 — Every chapter boundary has the full-strength closing divider ([T-0575], Q2); titles, when on, at Heading 1 size in
      the text colour and face
- [ ] AC3 — With titles off and on: new chapter, split, merge, delete, rename and structured paste, with their undo, behave
      as today, in the editor and on disk; Find never searches a title
- [ ] AC4 — A toggle changes no file and records no step; caret and scroll kept
- [ ] AC5 — `chapter_titles_smoke` in CI, mutation-checked; ctest green on macOS and Linux; all Linux smokes green
- [ ] AC6 — ✅ **Rig live pass (user)**

## Progress log

### 🔵 2026-10-15 — Sprint CREATED in Planning

User: *"plan EP-048 fully."* Issued with [SP-181]–[SP-183] via `next-id.py`. [T-0611] filed. Depends on [SP-182] Q2 (the
split). Q1–Q2 await rulings.

### 🔵 2026-10-15 — Q1–Q2 RULED (as recommended); ready to activate after [SP-182]

User: *"SP-182 Q5 and SP-184 Q1: agreed."* · *"SP-184 Q2: agreed."*
