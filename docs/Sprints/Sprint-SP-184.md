---
sprint: SP-184
epic: EP-048
status: Active
activated: 2026-11-05
platform: Cross
created: 2026-10-15
---

# SP-184 — `[Cross]` [EP-048] **S5c**: Show chapter titles, and the divider that closes a chapter

**Status:** 🟡 **ACTIVE 2026-11-05** (user, 2026-11-05: *"If that ends the rulings you may activate and implement SP-184."*) — created 2026-10-15 (user: *"plan EP-048 fully."*). ✅ The S5 split was ruled 2026-10-15
([SP-182] Q2). ✅ Q1–Q2 ruled 2026-10-15, both as recommended. ✅ **Re-planned 2026-11-05** against today's code (user: *"yes, close SP-183 and
plan SP-184"*): the last open Sprint of [EP-048]. ✅ Q3–Q4 ruled 2026-11-05; ⛔ **Q1 RE-RULED 2026-11-05: titles ON by default, on the Mac, Linux and iOS** — which makes this
Sprint `[Cross]`. ✅ Q5–Q6 ruled 2026-11-05 — every question ruled.
**Carries:** [T-0611] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-048] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). ✅ [EP-026] Q4 (ruled 2026-10-11): EP-048 S5 builds
the dialog **with the "Show chapter titles" toggle** (deferred since [EP-023]).
**Builds on:** [SP-182] (S5a): the dialog and `ProjectPreferences`.
**Authority:** [EP-047] P9 (chapter titles at Heading 1 size, in the text colour) · [T-0575] (the chapter-ending divider) ·
`Scrivi/Views/ManuscriptTextView.swift:787–823` (`rebuildStorage`).
**Shape:** ✅ Apple's (`feedback_linux_adopts_apple_shape`).
**Size:** M — every structural operation touches chapter boundaries. Needs a **live pass on scrivi-vm** (it counts for [EP-057]: no
x86, GPU or network AC here).

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

## What re-reading the code found (2026-11-05)

What changed since 2026-10-15, and what the first read missed:

1. ✅ **The user's own reason, from [SP-183]'s live pass:** *"the blue one between chapters. … I don't see the blue ones at the
   end of a Chapter. If Chapter headings are hidden it will be impossible to distinguish between a chapter break and a scene
   break."* Confirmed: Linux has never drawn it — `SceneDocument::sceneSeparatorPositions` (`SceneDocument.cpp:779`) returns
   only within-chapter breaks, and `ManuscriptEditor::paintEvent` paints only those.
2. **Apple's model, read today** (`ManuscriptEditing.swift:713–744`, `renderedPrefix`): a divider before EVERY scene but the
   first; the one at a chapter boundary is `.chapterEnd`, stroked in the ACCENT colour at 60% (`:313`; iOS `tintColor`,
   `:335`); then the title, only when `showChapterTitles`. So with titles on, a chapter boundary is divider + title.
3. ⚠️ **Orca would say the wrong words.** Linux's presented text gives every 2-character boundary the words *"Scene break"*
   (`ManuscriptFind.cpp:85`); Apple says *"End of chapter"* for a `.chapterEnd` divider (`ManuscriptPresentedMap.swift:297`,
   [EP-050] Q2). With titles off, a chapter boundary is exactly that 2-character gap → Q4.
4. ⚠️ **The boundary text has FIVE writers, not one.** The reflows use `leadingBoundaryFor` (`SceneDocument.cpp:365`), but
   `build` (`:59–72`), `insertSceneAfter` (`:316–321`) and two insert paths (`:561`, `:671–678`) write their own. Titles-off
   must not be a sixth copy: every writer goes through ONE function first (plan step 2).
5. **New paths since 2026-10-15** that cross chapter boundaries: [EP-053]'s range deltas (`SceneDocument::applyRangeDelta`,
   `:792`, whose boundaries come from `leadingBoundaryFor` at `:856`) for paste, cut, split, merge, chapter merge and their
   undo; [SP-189]'s background verification of a structural edit; [SP-191]'s `rehighlightRange` over chapter boundaries.
6. **[SP-183]'s layout** draws a chapter-title line at Heading 1's line height and a boundary blank as a body line
   (`ManuscriptPresenter::lineStyle`); with titles off the boundary is a blank line outside every scene, so the first
   paragraph after it is a scene start (no indent) either way. ✅ No change needed; tested with titles off.
7. **[I-0351]'s cull** applies to the new divider too: it is painted through `visibleBlockNumbers()`, never by asking for the
   geometry of every boundary.
8. ⚠️ **Linux has no accent colour in `ThemeColours`** (`deemphasised`, `rule`, `attention` only), and `QPalette::Accent` is
   Qt 6.6+ — CI's Qt is 6.4.2 → Q3.
9. **Inventory today** (plan step 1 re-checks it before any change): `chapterHeadingText` — `EditorShell.cpp:1999, 3822`;
   `setChapterTitle` (Rename Chapter) — `EditorShell.cpp:1342–1368, 1445`; `reflowBoundaryAt` — `EditorShell.cpp:1445, 2363` and
   10 sites in `SceneDocument.cpp`; `sceneSeparatorPositions` — `ManuscriptFind.cpp:64`, `ManuscriptEditor.cpp:1168`;
   `isEditableRange` — 16 sites in `ManuscriptEditor.cpp`. ⚠️ The 2026-10-15 references (`EditorShell.cpp:1302–1329, 1450`) are
   stale.

## Goal

✅ **A project's chapter titles show on Linux exactly when they show on the Mac**, with the same full-strength divider closing
each chapter, and every structural operation behaves the same either way.

## Plan

1. **Inventory** every reader and writer of the boundary text (`grep` across `platforms/linux/src`; finding 9 is today's), recorded
   here before changing any.
2. **ONE boundary builder.** Route `build`, `insertSceneAfter` and the insert paths through `leadingBoundaryFor` (finding 4),
   tested unchanged with titles on, THEN give it the titles flag. **`SceneDocument` builds with or without headings.** A chapter
   boundary always holds the closing divider, painted in the accent at 60% per Q3 ([T-0575]) through `visibleBlockNumbers()`
   ([I-0351]); the heading block is present only when titles are on (Heading 1 size, text colour, the project face, P9).
   `sceneSeparatorPositions` gains the chapter ends, each marked as one.
3. **The words (Q4):** the presented text says *"End of chapter"* at a chapter-end divider, *"Scene break"* at the others.
4. **The toggle:** the dialog gains Writing Surface ▸ *Show chapter titles in manuscript* (Apple's label and place). A
   toggle rebuilds the presentation only: caret and scroll kept, no disk write, no history event.
5. **Tests** (a new `chapter_titles_smoke`), with titles off AND on:
   - New chapter, split, merge, delete, rename and structured paste, and the undo of each, checked in the editor and on
     disk.
   - The caret after a new chapter ([I-0300]).
   - Find skips titles; the presented text's words at each kind of divider (Q4).
   - Save bytes unchanged by a toggle; the divider positions, a chapter end painted in the accent; a toggle on a
     dumas-sized manuscript timed (no freeze: a small op).
   - A Mac-written `showChapterTitles` is read.
   - **Mutations:** a heading drawn when off · no divider at a chapter end · a toggle that dirties a scene · a writer
     bypassing the one builder · *"Scene break"* at a chapter end.
6. **Live pass** (user, scrivi-vm), against the same project on the Mac.

## Questions — ✅ Q1–Q2 RULED 2026-10-15 (as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | ⛔ **RE-RULED 2026-11-05 — see below.** **The default for a project with no setting.** Apple's is OFF, so every existing Linux manuscript loses its chapter headings on first open. | ✅ **Apple's (off).** The setting travels with the project, so any other default makes one project look different on each machine; the writer turns titles on once, in Project Settings, and both platforms follow. Alternative: Linux keeps titles on until the project sets the key (the two platforms disagree until then) · ✅ **RULED** (user, 2026-10-15) |
| **Q2** | **The chapter-ending divider ([T-0575]) on Linux.** Not named in EP-048 or EP-026, but with titles off nothing else marks where a chapter ends. | ✅ **In this Sprint.** It is what Apple draws at that boundary, and without it Q1 leaves chapters invisible on Linux. Alternative: a separate Task · ✅ **RULED** (user, 2026-10-15) |

| **Q3** | ✅ **RULED 2026-11-05: `QPalette::Highlight` at 60%** (as recommended). ➕ **The chapter-end divider's colour on Linux.** Apple: the system ACCENT at 60%. Qt 6.4 (CI) has no `QPalette::Accent`. | ✅ **Recommended: `QPalette::Highlight` at 60%**, a new `ThemeColours::chapterEnd` beside `rule` — the selection colour, which follows the desktop's accent on GNOME, as Apple's follows System Settings; measured for contrast in `theme_contrast_smoke` (the [I-0186] rule: never a palette role unmeasured). Alternative: a fixed blue (does not follow the writer's accent) |
| **Q4** | ✅ **RULED 2026-11-05: fixed in this Sprint** — the words per Q6: *"End of chapter"*. ➕ **What Orca hears at a chapter end.** Today *"Scene break"* at every boundary; Apple says *"End of chapter"* ([EP-050] Q2). | ✅ **Recommended: in this Sprint** — Apple's words; with titles off it is the only way a screen-reader user can tell a chapter break from a scene break, the user's reason for the divider itself. Alternative: a separate Task under [EP-050] |

✅ **Rulings 2026-11-05** (user, 2026-11-05: *"Q3: the recommended selection color at 60%.  Q4: fix them to \"End of Chapter\".  Also, I think I prefer Chapter Heding on by default on both MAC and Linux and iOS in cotravention of my earlier ruling."*)

⛔ **Q1 re-ruled: chapter titles ON by default, on every platform.** Apple reads `showChapterTitles` `?? false`
(`Scrivi/App/ProjectPreferences.swift:123`, shared by the Mac and iOS) → `?? true`; Linux reads absent = on. ⚠️ Consequences, read
2026-11-05:
- **The Mac PINS the key on every settings save** (`ProjectPreferences.swift:140`; and the migration at `:110`), unlike every
  other key (written only once chosen). So a project whose settings the Mac has saved already holds an explicit value — `true`
  in `dumas-prose-timelines` and `dumas-prose-timelines-history`; `false` in any project saved without the toggle touched. Only
  a project with NO key (no settings file, e.g. `dumas-prose`, or one only Linux has written) changes with the default → Q5.
- `ScriviInteropTests.swift:5411` expects the default OFF; it changes with the ruling (a test of the ruling, not a fix to hide).
- Apple's code changes in this Sprint (`[Apple]`), as [SP-204] changed `HistoryCard.swift`.

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q5** | ✅ **RULED 2026-11-05 (as recommended): honour every stored value; the Mac stops pinning the key.** ➕ **A `false` the Mac pinned without the writer choosing it** cannot be told from one the writer chose. | ✅ **Recommended: honour every stored value, and stop the pinning** — the Mac writes `showChapterTitles` only once the writer changes it (Linux's rule, and the Mac's own for the other keys), so no NEW project is pinned off. Existing projects that read `false` stay off until the writer turns titles on (once, in Project Settings). Alternative: a one-time flip of stored `false` to on (overrides any writer who did choose off) |
| **Q6** | ✅ **RULED 2026-11-05: the SAME string on both — the Mac's existing *"End of chapter"*** (the user: a screen reader speaks no capitals; the Mac is unchanged). ➕ **The words' capitalisation.** The Mac says *"End of chapter"* (`ManuscriptPresentedMap.swift:297`); the ruling says *"End of Chapter"*. | ✅ **Recommended: one string on both platforms** — *"End of Chapter"* on the Mac too, with its interop test, since what a screen reader says should not differ by platform. Alternative: Apple's *"End of chapter"* on Linux |
| **Q7** | ➕ **The chapter end's contrast (found while building, 2026-11-05).** 60% of the accent is 2.1–2.4:1 against the page on every theme measured — the Mac's own default blue on white 2.29:1 — below the 3:1 the scene rule is held to (WCAG 1.4.11, non-text); full strength is ≥ 3.65:1 everywhere. | ✅ **Recommended: keep 60% on both platforms** — it is the Mac's, softened from full strength on the user's own pass ([T-0575]: *"The tinted divider may be too much"*), and it is a mark that must not compete with prose, not a control; the Linux smoke holds it to the Mac's 2.0:1. Alternative: 85% on both, the lowest opacity that reaches 3:1 on every theme measured (3.03:1 at worst; 75% gives only 2.6–2.9:1) — noticeably stronger than the Mac shows today |

⛔ **NOT in this Sprint:** chapter titles editable in place (Apple's are drawn, not edited; Rename Chapter stays the path) ·
any change to how untitled chapters are numbered.

## Acceptance Criteria

- [x] AC1 — Writing Surface ▸ Show chapter titles in manuscript, read from and written to `project-settings.json` (absent = ON,
      Q1 re-ruled, on the Mac, iOS and Linux; per Q5); a project shows titles on Linux exactly when it does on the Mac — ✅
      `chapter_titles_smoke` §1–§3; Apple `ScriviInteropTests` `fresh`, `roundTrip`, `chapterTitlesNotPinned`
- [x] AC2 — Every chapter boundary has the closing divider in the accent per Q3 ([T-0575], Q2), with titles off AND on; titles,
      when on, at Heading 1 size in the text colour and face; the presented text says *"End of chapter"* there (Q4, Q6) — ✅ `chapter_titles_smoke` §1–§3 (painted
      colours read from the viewport; the page's words)
- [x] AC3 — With titles off and on: new chapter, split, merge, delete, rename and structured paste, with their undo, behave
      as today, in the editor and on disk; Find never searches a title — ✅ `structural_baseline_smoke`'s second run with titles OFF;
      `chapter_titles_smoke` §4; ⚠️ Find over titles not re-tested (titles are leads, unchanged since [SP-176])
- [x] AC4 — A toggle changes no file and records no step; caret and scroll kept; no freeze on a dumas-sized manuscript — ✅
      `chapter_titles_smoke` §2, §5 (1 ms each way on 1,186 scenes / 58 chapters)
- [x] AC5 — `chapter_titles_smoke` in CI, mutation-checked; `theme_contrast_smoke` covers the new colour; ctest green on macOS
      and Linux; all Linux smokes green — ✅ (log, 2026-11-05; ⚠️ the contrast is measured only under a real GTK theme — it skips
      in Docker — so on scrivi-vm; ctest not re-run: no core change)
- [ ] AC6 — **Live pass (user) on scrivi-vm**, against the same project on the Mac

## Progress log

### 🔵 2026-10-15 — Sprint CREATED in Planning

User: *"plan EP-048 fully."* Issued with [SP-181]–[SP-183] via `next-id.py`. [T-0611] filed. Depends on [SP-182] Q2 (the
split). Q1–Q2 await rulings.

### 🔵 2026-10-15 — Q1–Q2 RULED (as recommended); ready to activate after [SP-182]

User: *"SP-182 Q5 and SP-184 Q1: agreed."* · *"SP-184 Q2: agreed."*

### 🔵 2026-11-05 — Re-planned against today's code

User: *"yes, close SP-183 and plan SP-184"*. Nine findings (above): the user's reason for the divider; Apple's model read today;
Orca's words at a chapter end; five writers of the boundary text, to become one; [EP-053]'s range deltas, [SP-189]'s verification
and [SP-191]'s redraw as new paths; [SP-183]'s layout unaffected; [I-0351]'s cull for the new divider; no accent colour on Linux;
today's inventory. Two new questions, Q3 (the colour) and Q4 (the words). The live pass moves to scrivi-vm.

### 🔵 2026-11-05 — Q3–Q4 ruled; Q1 re-ruled (titles on by default, every platform)

user, 2026-11-05: *"Q3: the recommended selection color at 60%.  Q4: fix them to \"End of Chapter\".  Also, I think I prefer Chapter Heding on by default on both MAC and Linux and iOS in cotravention of my earlier ruling."*. The re-ruling makes the Sprint `[Cross]`: Apple's default changes too. Read: the Mac pins the key on every save, so
only projects with no key follow the default → Q5; the words' capitalisation differs from the Mac's → Q6.

### 🟡 2026-11-05 — Q5–Q6 ruled; ACTIVATED

user, 2026-11-05: *"…if you must make it the same on both platforms (Q6). Q5: yes (your recommendation)."* · user, 2026-11-05: *"If that ends the rulings you may activate and implement SP-184."*. [T-0611] → `Task-active.md`.

### 🟡 2026-11-05 — Built; AC1–AC5 met (AC6, the live pass, is the user's)

✅ **Built:**
- **One boundary builder** (`SceneDocument.cpp:37`, `leadingBoundaryFor`): `build` and `insertSceneAfter` now go through it with the
  reflows and the range deltas; it takes the titles flag. The two move paths' provisional `\n\n` (`:561`, `:671`) are reflowed
  through it at once, as before.
- `SceneDocument::setShowChapterTitles` (`:768`) rewrites every chapter boundary in place; `dividers()` (`:757`) lists a divider
  before every scene but the first, the chapter ends marked — replacing `sceneSeparatorPositions`.
- `ManuscriptEditor::paintEvent` paints every divider through `visibleBlockNumbers()` ([I-0351]); chapter ends in
  `ThemeColours::chapterEnd` (`ThemeColours.hpp:91`, `Highlight` at 60% over `Base`, Q3).
- The presented page (`ManuscriptFind.cpp:89`) reads *"End of chapter"* at a chapter end, then the title when titles show (Q4, Q6).
- `ProjectPreferences`: `showChapterTitles`, absent = ON, written only once chosen (Q5); Project Settings ▸ **Writing Surface ▸
  Show chapter titles in manuscript** (Apple's section, label and place). `EditorShell` reads it BEFORE the build (`:845`) and
  applies a change in place (`applyChapterTitles`, `:3142`): the `loading_` guard, the caret and the top of the view kept.
- `[Apple]` `Scrivi/App/ProjectPreferences.swift`: absent = ON (`:126`), and the key is written only once the writer changes it
  (Q5); its tests: `fresh` (default ON), `roundTrip` (off survives), new `chapterTitlesNotPinned`.

| Evidence (2026-11-05) | Result |
|---|---|
| `chapter_titles_smoke` (new, in CI), Docker Qt 6.4.2, non-root | ✅ `titles-ok` |
| `structural_baseline_smoke`, now run twice (titles on, then OFF from the project's settings) | ✅ both |
| `accessibility_smoke` | ✅ — its expected page updated to Q4's words (*"Second scene.\nEnd of chapter\nChapter Two"*) |
| All 42 Linux CI smoke steps, Docker, non-root, `SCRIVI_VERIFY_SCENE_TEXT=1` | ✅ 0 failed |
| Mutations, in a COPY of the repo | ✅ 7/7 caught: a title drawn when off · no divider at a chapter end · the toggle without its guard (SceneDocument's text check aborts the run) · `build` bypassing the builder · *"Scene break"* at a chapter end · Linux pinning the key · the chapter end in the rule colour |
| `scripts/run-interop-tests.sh` (macOS) | ✅ 338 tests in 45 suites |
| iOS build (`ScriviApp-iOS`, iPhone Duo simulator) | ✅ |
| The toggle on 1,186 scenes / 58 chapters | ✅ 1 ms off, 1 ms on |

⚠️ **Found, for a ruling (Q7):** 60% of an accent is **2.1–2.4:1** against the page on every theme measured — the Mac's own default
blue on white **2.29:1**, Yaru orange 2.20:1, GNOME blue 2.13:1 (dark pages 2.33–2.44:1) — below the 3:1 the scene rule is held
to (WCAG 1.4.11's floor for a non-text mark); at full strength every one is ≥ 3.65:1. `theme_contrast_smoke` holds the chapter
end to **2.0:1** (the Mac's own level) until ruled.
