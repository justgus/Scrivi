# Verified Issues: I-0351 – I-0360

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0341–I-0350 are in [`Issue-verified-0341-0350.md`](Issue-verified-0341-0350.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0351** | `[Linux]` **Every repaint of the manuscript walks the whole document: the scene-break rules ask Qt for each separator's geometry, then drop the off-screen ones** — ~95,000 `blockBoundingRect` calls per keystroke on 1,186 scenes. Found in [SP-183] step 0. → [details](#i-0351) | Medium | [SP-183] | ✅ **Verified 2026-11-05** (the user's live pass on scrivi-vm: no lag) |
| **I-0357** | `[Linux]` **Toggling "Show chapter titles in manuscript" styles the wrong lines until the project is reopened** | Medium | SP-207 | ✅ **Verified 2026-11-09** |

## I-0351: `[Linux]` Every repaint of the manuscript walks the whole document for the scene-break rules

**Found:** 2026-11-05, [SP-183] step 0 (`indent_probe`, a throwaway in a copy of the repo: the real `ManuscriptEditor` and
`ManuscriptPresenter` over 1,186 scenes, ~17,500 blocks, Docker, Qt 6.4.2 and 6.10.2, Debug).
**What:** `ManuscriptEditor::paintEvent` (`platforms/linux/src/ManuscriptEditor.cpp:1188–1205`) draws each scene-break rule by
calling `blockBoundingGeometry` for **every** separator in the manuscript, and only then skips the off-screen ones
(`:1204`). [SP-204] found that `blockBoundingGeometry` walks from the first visible block to the one asked for, laying out
every block between, so every paint walks the whole document.
**Measured** (a backtrace from inside the walk confirms `ManuscriptEditor::paintEvent` is the caller):

| | Today | Culled by block number first (probe only) |
|---|---|---|
| `blockBoundingRect` calls per keystroke | ~95,000 | ~180 |
| 20 keystrokes | 88 ms | 16 ms |
| Blocks laid out after the first show | 1,243 | 39 |

**Why it matters:** about 3.6 ms per keystroke on synthetic text, more on a real book. [SP-183]'s indent layout runs per laid-out
block, so with this walk in place a keystroke cost 18 ms instead of 1 ms; [SP-184] adds chapter dividers to the same loop.
**Fix (proposed):** the same as [SP-204]'s branch marks: cull each separator by block number against the visible range before
asking for its geometry. Tested by counting layout calls per keystroke, with a mutation that removes the cull.
**Fix (2026-11-05):** `ManuscriptEditor::paintEvent` culls each separator by block number before its geometry
(`platforms/linux/src/ManuscriptEditor.cpp:1207`), through `visibleBlockNumbers()`, now shared with [SP-204]'s branch marks.
**Evidence:** `indent_smoke` §5 (80 scenes): one keystroke asks for a bounded number of block rectangles; the mutation that
removes the cull is caught (3,772). On a dumas-sized manuscript (1,186 scenes), 20 keystrokes 88 → 21 ms (Qt 6.4.2, Docker).
**Status:** ✅ **Verified 2026-11-05** (user, 2026-11-05: *"all three re-test steps passed."* · *"C1, C2, C3, and C6 all passed."*; pass 1: *"But there was no lag"*).

---

## I-0357: `[Linux]` Toggling "Show chapter titles" styles the wrong lines until the project is reopened

**Reported:** 2026-11-09 by the user on scrivi-vm (build 1.0.367, `87fea0f`), during [SP-207]'s Linux live pass (unrelated to
it): *"when I change the project setting "Show Chapter Titles in Manuscript" the result is never correct. If I change the option
from checked to unchecked, the first sentence in many chapters (but not all) becomes the Chapter Title typeface. If the option
goes from unchecked to checked, the Chapter titles appear in body text format. This is only a display issue because if I
immeditely quit the app and reload, the chapter heading and first chapters are all properly displayed. In either direction."*
**Cause (read from the code, not yet proved by a test):** `EditorShell::applyChapterTitles` →
`SceneDocument::setShowChapterTitles` → `reflowAllChapterHeadings` → `reflowBoundaryAt(i)` for each chapter. Each call edits the
document (`cursor.insertText(desired)`) and only THEN shifts `segments_[i..].bodyStart` by the length change
(`SceneDocument.cpp`, `reflowBoundaryAt`). `QSyntaxHighlighter` restyles the edited blocks synchronously, inside `insertText`, and
`ManuscriptPresenter::highlightBlock` decides "chapter title" by `sceneOf(line) < 0`, which reads those not-yet-shifted
offsets (`sceneIndexForEditablePosition`). So the edited lines are classified against stale positions: a body's first line can
read as "outside any scene" (title face), and a new title can read as body text. Nothing restyles those blocks afterwards; a
reopen builds everything from correct offsets. "Many but not all" fits: whether a stale offset lands on the wrong side
depends on each boundary's length change.
**Fix direction:** restyle the boundary blocks once the offsets are current (after the reflow) — each chapter boundary's lines,
not the whole manuscript — and a test that toggles both ways and checks each line's format.
**Apple:** not reported; Apple rebuilds its storage on the toggle.
**Fixed 2026-11-09 (SP-207, user: *"fold I-0357 into SP-207 and fix it"*):** `SceneDocument::reflowBoundaryAt`
(`platforms/linux/src/SceneDocument.cpp`) now shifts the scene offsets BEFORE `insertText`, so the presenter restyles the edited
lines against current offsets. One function, so every title rewrite is covered: the toggle, a chapter rename, a new chapter,
a merge or paste that moves a boundary.
**Test:** `chapter_titles_smoke` gained `titleFaceRight` — the lines drawn in the title face must be exactly the titles shown —
at load, after titles off and after titles on. ✅ Red without the fix, on scrivi-vm: off → *"drawn: Alpha one. / Beta one. /
Gamma one. | expected: "*; on → *"drawn: Gamma one. | expected: The Abbé / Chapter 2 / Chapter 3"* (the user's two symptoms).
✅ Green with it.
**Status:** ✅ **Verified 2026-11-09** (user, 2026-11-09: *"I-0357 verified, close SP-207"*). Archived when [SP-207] closed.
