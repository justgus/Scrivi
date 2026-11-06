---
sprint: SP-183
epic: EP-048
status: Closed
closed: 2026-11-05
activated: 2026-11-05
platform: Linux
created: 2026-10-15
---

# SP-183 — `[Linux]` [EP-048] **S5b**: line spacing and the first-line indent

**Status:** ✅ **CLOSED 2026-11-05** (user, 2026-11-05: *"yes, close SP-183 and plan SP-184"*) — activated 2026-11-05 (user, 2026-11-05: *"Activate SP-183 and run step 0."*) — created 2026-10-15 (user: *"plan EP-048 fully."*); ✅ **re-planned 2026-11-05** against today's code (user: *"plan SP-183 for EP-048"*), the last Sprint before [EP-048] can close besides [SP-184]. ✅ The S5 split was ruled 2026-10-15
([SP-182] Q2). Line spacing is in this Sprint ([SP-182] Q3, ruled). ⏳ Q1–Q2 are ruled AFTER plan step 0's measurement, not before.
**Carries:** [T-0610] ✅ → [`../../Tasks/Verified/Task-verified-0610.md`](../../Tasks/Verified/Task-verified-0610.md) · ➕ [I-0351] (found in step 0; ✅ kept by Q3) ✅ → [`../../Issues/Verified/Issue-verified-0351-0360.md`](../../Issues/Verified/Issue-verified-0351-0360.md).
**Epic:** [EP-048] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). **L10, its block half:** *"⚠️ Qt's first-line
indent is a BLOCK format (a document format, not a highlighter one — the SP-166 hanging-indent finding); measure it before
ruling the mechanism."*
**Builds on:** [SP-182] (S5a): the dialog, `ProjectPreferences` and the one typography source.
**Authority:** [EP-047] P3, P9 (line spacing fixed at 1.45 × size, the same for every face), P10 and AC6
(`../Epics/Closed/Epic-EP-047.md`) · `Scrivi/Views/ManuscriptTypography.swift` (`lineSpacing = 1.45`, the indent modes).
**Size:** M, ⚠️ with real risk that Qt cannot do it all (Q2). Needs a **live pass on scrivi-vm** (it counts for [EP-057]: no x86, GPU or network AC here).

---

## What reading the code found (2026-10-15)

- **What Apple draws** (P3, P10, AC6):
  - Three modes: **None · Every body paragraph · Book convention** (the default).
  - The amount is in ems: 1.5 by default, stepper 0.5–4, disabled at None.
  - Book convention indents no paragraph that follows anything but body text: not after a scene start, a heading, a list,
    a block quote or a chapter title. Headings and list items are never indented.
  - With an indent on, the stored blank line between paragraphs draws as a **small gap**. With None it is the full blank
    line.
  - **Zero characters in the file:** save bytes unchanged, and the indent survives typing, Return, undo and rebuild.
- **Apple's mechanism** is `firstLineHeadIndent` and a fixed line height on the paragraph style, both attributes TextKit
  draws with and never saves.
- ⚠️ **On Qt both are BLOCK formats** (`QTextBlockFormat::setTextIndent`, `setLineHeight`). [SP-166] found that setting one is
  a document edit, and [SP-174] Q3 accepted the consequence for list items: **no hanging indent on Linux**, recorded as a
  known difference. ⚠️ Taking the same way out here would leave L10 unmet, so this Sprint must find a way, or bring the
  difference back for a ruling.
- ⚠️ **Unmeasured, each one:**
  - (a) whether `QPlainTextDocumentLayout`, the Linux manuscript's layout since [EP-022], honours `textIndent` and
    `lineHeight` at all;
  - (b) what a `setBlockFormat` emits: `contentsChange` arguments, the modified flag, Qt's undo stack (disabled);
  - (c) whether `SceneDocument::applyContentsChange` would treat it as an edit, and so dirty and save a scene, or
    `HistoryCapture::noteEdit` would record a step;
  - (d) whether a blank block can draw shorter than a line, for the small gap;
  - (e) the cost over dumas: about 1,186 scenes, tens of thousands of blocks.
- ✅ **A guarded window already exists.** `SceneDocument::reflowAllChapterHeadings` runs inside a caller-managed
  "programmatic" window with no dirty churn (`SceneDocument.hpp:325`). That is one candidate mechanism.

## What re-reading the code found (2026-11-05)

What has changed since 2026-10-15, and what the first read missed:

1. **The setting is shared with the Mac.** Apple stores `paragraphIndent` (`book` · `every` · `none`, absent = `book`) and
   `indentEm` in `project-settings.json` (`Scrivi/App/ProjectPreferences.swift:50–84`). Linux's `ProjectPreferences` reads the
   same file for the face and size, so a choice made on the Mac must draw on Linux and the other way round → AC1.
2. **Headings have their own line height:** 1.25 × their own size, chapter titles too (`ManuscriptTypography.swift:123`).
   The first plan named only the body's 1.45 → AC2. The small gap is ⅓ of a body line (`gapFraction`, `:117`).
3. **Book convention depends on the paragraph BEFORE** (`ManuscriptPresenter.swift:157–170`). An edit to one paragraph can
   change the next one's indent: typing `#` at a line's start, deleting the blank line between two paragraphs, a paste. ⚠️
   Linux's presenter is a `QSyntaxHighlighter`, which sets character formats one block at a time and cannot set a block
   format, so mechanism (A) needs its own pass that also re-checks the following block.
4. **The document now changes in more ways** than it did on 2026-10-15: [EP-053]'s range deltas
   (`SceneDocument::applyRangeDelta`, `SceneDocument.cpp:792`) for paste, cut, split and merge and their undo, and [SP-189]'s
   background verification. Each is a path the indent must survive → AC3. [SP-204]'s branch marks sit in the margin, outside
   the text, but are placed from line rectangles (`ManuscriptEditor::placeBranchMarks`), so they are re-checked with an indent on.
5. ⚠️ **[SP-204] found that Qt lays out every block before the one asked about** (`blockBoundingGeometry`). A mechanism that
   sets a format on every block at open could force the whole of dumas to be laid out → step 0 (e) measures the open, not
   just a keystroke.

## Goal

✅ **Linux draws the project's paragraph indent and line spacing as the Mac does**, without writing a character, dirtying a
scene or recording an undo step. Or, where Qt cannot, the difference is measured and brought back for a ruling.

## Plan

0. **Spike, measured** on Qt 6.4.2 (the CI's Docker image) and Qt 6.10 (the `scrivi-qt610` Docker image, and scrivi-vm's 6.10.2
   for a screenshot): (a)–(e) above, each with numbers or a screenshot, recorded here. (e) is dumas's open time and a
   keystroke's cost, with and without the indent. ⛔ No mechanism is chosen before this.
1. **Mechanism, per Q1**, after step 0.
2. **The rules:** the three modes and Book convention's cases (re-checked on the following block after an edit), the amount
   in ems, the small gap (⅓ line), body lines 1.45 × size and heading lines 1.25 × their own size; read from and written to
   `project-settings.json` under Apple's keys.
3. **The dialog:** Typography gains Paragraph indent (Book convention · Every paragraph · None) and Indent (0.5–4 em, step
   0.5, disabled at None), Apple's rows and order.
4. **Tests** (a new `indent_smoke` or `settings_smoke` extended):
   - Save bytes unchanged; no dirty scene; no history step.
   - The indent survives typing, Return, undo, reload, paste, structured paste and cut, split, merge and their undo, and
     Replace All.
   - An edit that changes the paragraph before (a `#` typed, a blank line deleted) updates the next paragraph's indent.
   - A `project-settings.json` written by the Mac draws the same on Linux.
   - Headings and list items are never indented.
   - Every Book convention case: scene start, after a heading, a list, a block quote, a chapter title.
   - **Mutations:** an indent on a heading · a format that dirties the scene · Book convention indenting a scene's first
     paragraph · the following block not re-checked after an edit.
5. **Live pass** (user, scrivi-vm), against the same project on the Mac.

## Questions — ruled after plan step 0

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **The mechanism.** Candidates: (A) block formats applied inside a guarded programmatic window, with their `contentsChange` filtered out of save and history · (B) a `QPlainTextDocumentLayout` subclass that offsets the first line and sets the line pitch when laying out, so nothing in the document changes · (C) moving the manuscript to `QTextEdit`'s layout. | ⏳ **After step 0.** (C) is not recommended: it reopens [EP-022]'s editor choice. Between (A) and (B), the numbers decide; (B) is preferred if Qt allows it, because the document stays untouched by construction |
| **Q2** | **If Qt cannot draw part of it** (most likely the small gap). | ⏳ **After step 0.** Bring each gap back with a screenshot from both platforms, as [SP-174] Q3 did: build it differently, or accept and record the difference. Nothing is accepted silently |

| **Q3** | ➕ **Add [I-0351] to this Sprint?** Found in step 0: every repaint walks the whole manuscript for the scene-break rules. | ✅ **Recommended: yes.** (B) re-lays out every block the walk touches, so with the walk in place a keystroke costs 18 ms instead of 1 ms; the fix is [SP-204]'s cull, a few lines in `ManuscriptEditor::paintEvent` |

✅ **RULED 2026-11-05** (user, 2026-11-05: *"Q1: (B).  Q2: stet.  Q3: keep, fix.  proceed with the implementation of SP-183."*): **Q1 = (B)** · **Q2 stands** as written (each gap brought back, nothing accepted silently; step 0 found none) · **Q3: [I-0351] kept and fixed here.**

**Step 0's recommendations (2026-11-05):** **Q1 = (B)**, the layout subclass: (A) draws nothing on either Qt
version and would dirty every scene it touched; (B) draws all of it and, with [I-0351] fixed, costs what today costs. **Q2:
nothing to accept** — the small gap draws (d). **Q3: yes.**

⛔ **NOT in this Sprint:** the hanging indent for list items (accepted as a difference by [SP-174] Q3; reopened only if this
Sprint's mechanism makes it cheap, and then only by ruling) · Show chapter titles (→ S5c).

## Acceptance Criteria

- [x] AC0 — Plan step 0's (a)–(e) measured and recorded on both Qt versions; Q1 and Q2 ruled from them — ✅ measured and
      recorded (2026-11-05, below) · ✅ Q1–Q3 ruled 2026-11-05
- [x] AC1 — **L10 (indent):** None · Every · Book convention, with the amount in ems, drawn as on the Mac; headings and list
      items never indented; Book convention's cases hold, including after an edit to the paragraph before; the small gap per
      Q2; the setting shared with the Mac through `project-settings.json` — ✅ `indent_smoke` §1, §2, §4
- [x] AC2 — **L10 (line spacing):** body lines 1.45 × size for every face; heading and chapter-title lines 1.25 × their own size
      — ✅ `indent_smoke` §1
- [x] AC3 — Zero characters in the file: save bytes unchanged, no dirty scene, no history step; survives typing, Return,
      undo, reload, paste, structured paste and cut, split and merge (and their undo), and Replace All; dumas opens no slower
      than step 0's baseline allows (Q1) — ✅ `indent_smoke` §1–§3 (no text change, no
      dirty scene, no scene file written, no history step); dumas-sized open 812 → 814 ms (log, 2026-11-05)
- [x] AC4 — The dialog's indent rows; smokes in CI, mutation-checked; ctest green on macOS and Linux — ✅ (log, 2026-11-05;
      ctest Linux not re-run: no core change)
- [x] AC5 — **Live pass (user) on scrivi-vm**, against the same project on the Mac — ✅ passed (user, 2026-11-05: *"all three re-test steps passed."* · *"C1, C2, C3, and C6 all passed."*); build 1.0.359 +
      the layout fix

## Progress log

### 🔵 2026-10-15 — Sprint CREATED in Planning

User: *"plan EP-048 fully."* Issued with [SP-181], [SP-182] and [SP-184] via `next-id.py`. [T-0610] filed. Depends on [SP-182]
Q2 (the split). Q1–Q2 are ruled after the step 0 measurement.

### 🔵 2026-10-15 — The split ruled

User: *"SP-182 Q2 agreed."* and *"SP-182 Q3-Q4: agreed."* This Sprint stands, with line spacing in it. Q1–Q2 are still ruled
after step 0.

### 🔵 2026-11-05 — Re-planned against today's code

User: *"plan SP-183 for EP-048"*. Five findings (above): the setting is shared with the Mac; heading lines are 1.25; Book
convention depends on the paragraph before; [EP-053]'s range deltas and [SP-189]'s verification are new paths to survive;
[SP-204]'s whole-document layout cost makes the open time part of step 0. The live pass moves to scrivi-vm. Q1–Q2 are still
ruled after step 0.

### 🟡 2026-11-05 — ACTIVATED; step 0 started

user, 2026-11-05: *"Activate SP-183 and run step 0."*. [T-0610] → `Task-active.md`.

### 🟡 2026-11-05 — Step 0 measured

`indent_probe`, a throwaway in a COPY of the repo (never in the working tree): the real `ManuscriptEditor`, `ManuscriptPresenter`
and `SceneDocument`, 16 px text, built Debug in Docker against **Qt 6.4.2** (`scrivi-linux`) and **Qt 6.10.2** (`scrivi-qt610`).
**Both versions gave the same results**; screenshots are byte-identical between them.

| | (A) block formats (`setTextIndent`, `setLineHeight`) | (B) a `QPlainTextDocumentLayout` subclass |
|---|---|---|
| (a) drawn? | ⛔ **No.** `QPlainTextDocumentLayout` ignores both: first-line x and block heights unchanged (x 4, 19 px lines), with `FixedHeight` and `ProportionalHeight` alike | ✅ **Yes.** First line at margin + 24 px (1.5 em), 23.2 px pitch (1.45 × 16), the caret and a click at the indent land at the paragraph's start |
| (b) what it emits | ⛔ `contentsChange(pos, n, n)` per block and sets the document modified (undo stays empty: it is disabled) | ✅ Nothing: the document is never changed; a keystroke emits only its own `contentsChange(pos, 0, 1)` |
| (c) in the app (read: `EditorShell::onContentsChange`, `EditorShell.cpp:1450`) | ⛔ outside `loading_`, equal non-zero counts reach `SceneDocument::applyContentsChange` (re-reads and re-verifies the scene's text) and then **dirty the scene and arm its save**; no history step (`reportEdit` finds the text unchanged) | ✅ not reached |
| (d) a blank block shorter than a line | ⛔ no (heights unchanged) | ✅ 7.7 px (⅓ line) |
| (e) dumas-sized (1,186 scenes, 17,487 blocks): first show · 20 keys | 620 ms · 87 ms (same as today: it draws nothing) | 634 ms · ⛔ **362 ms** with today's paint → ✅ **611 ms · 17 ms** with [I-0351] culled |

Today, for comparison: first show 621 ms, 20 keys 88 ms → 617 ms · 16 ms with [I-0351] culled. Scroll to the end: 2 ms today,
14 ms with (B) under today's paint, 1 ms with the cull.

**Found: [I-0351].** The cost of (B) per keystroke came from outside it: `ManuscriptEditor::paintEvent` asks for the geometry of
every scene separator in the manuscript before culling the off-screen ones (~95,000 `blockBoundingRect` calls per keystroke;
a backtrace taken inside the walk names `paintEvent` as the caller). (B) re-lays out each block that walk touches (~1,128
per keystroke); with the cull, 4–5. Filed and assigned here per CLAUDE.md; ✅ kept by Q3.

**What step 1 must settle (B's open risks, not blockers):**
- **Book convention needs the previous block's kind** inside the layout: the presenter's classification (heading, list,
  quote, fence) must be readable there without re-parsing.
- **When a paragraph's kind changes, the NEXT block must re-lay out** without a document edit. `markContentsDirty` emits
  `contentsChange(pos, n, n)`, which (c) shows would dirty the scene; candidates are `QTextBlock::clearLayout()` plus a
  viewport update, measured before use.
- **Scrolling counts lines, not pixels** (`QPlainTextEdit`): a ⅓-line gap is still one scroll step. Checked against the Mac
  in the live pass.
- **Where the glyphs sit in the 1.45 line:** the probe centres them; TextKit's fixed line height is matched to a Mac
  screenshot in step 2.

### 🟡 2026-11-05 — Built; AC0–AC4 met (AC5, the live pass, is the user's)

✅ **Built per Q1 = (B):**
- `ManuscriptLayout` (`platforms/linux/src/ManuscriptLayout.{hpp,cpp}`, new): `blockBoundingRect` re-lays each block Qt has laid
  out, the first line indented and every line on the block's pitch, glyphs at the bottom of the line (TextKit puts a fixed
  line's extra space above). Installed by `SceneDocument` in place of Qt's plain layout.
- The presenter decides each line's style while it highlights (`ManuscriptPresenter::lineStyle`, Apple's `isBody` and book
  rule) and leaves it on the block as `ManuscriptLineStyle` user data, which Qt never reports as an edit. After an edit,
  `rehighlightRange` also redraws the next paragraph's first line (step 0's open risk 2: no `markContentsDirty` needed).
  Body 1.45 × size · headings and chapter titles 1.25 × their own size · separators a body line · a blank line in a scene ⅓
  of a body line while an indent is on.
- `ManuscriptTypography` gains Apple's modes and constants; `ProjectPreferences` reads and writes Apple's `paragraphIndent`
  (an unknown mode KEPT, drawn as Book) and `indentEm`, only once chosen; `EditorShell::applyTypography` passes them on.
- Project Settings ▸ Typography: **Paragraph indent** (Book convention · Every paragraph · None) and **Indent** (0.5–4 em, step
  0.5, disabled at None), Apple's rows and order.
- [I-0351]: `ManuscriptEditor::paintEvent` culls separators by block number first, through `visibleBlockNumbers()` (shared with
  [SP-204]'s branch marks).

**Found while testing (the tests were wrong, not the code):** a typed `#` is stored escaped (`\#`, EP-049), so the smoke makes
a heading with Format ▸ Heading 1 (Ctrl+Alt+1); Return starts a new PARAGRAPH (it stores a blank line), so the new paragraph
is indented under Book convention, as on the Mac.

| Evidence (2026-11-05) | Result |
|---|---|
| `indent_smoke` (new, in CI), Docker Qt 6.4.2, non-root | ✅ `indent-ok` |
| All 41 Linux CI smoke steps, Docker, non-root, `SCRIVI_VERIFY_SCENE_TEXT=1` | ✅ 0 failed |
| Mutations, in a COPY of the repo | ✅ 5/5 caught: an indent on a heading · Book indenting a scene's first paragraph · the next paragraph not redrawn · a block format that dirties the scene (caught by `dirtySceneCount`, added when the first run missed it: an unchanged scene's save writes no file) · [I-0351]'s cull removed (3,772 rectangles) |
| ScriviCore ctest, macOS | ✅ 806/806 on two runs; ⚠️ one earlier run failed one test → [I-0352] (backlog; not reproduced in 300 repeats; this Sprint changes no core code) |
| Dumas-sized (1,186 scenes) open + first show · 20 keys · 20 Page Down, Qt 6.4.2 | Qt's plain layout 812 ms · 21 ms · 40 ms → ManuscriptLayout 814 ms · 23 ms · 52 ms |
| The same on Qt 6.10.2 | 869 ms · 20 ms · 39 ms → 857 ms · 24 ms · 49 ms |

⚠️ Not run: `indent_smoke` on Qt 6.10 (the live pass on scrivi-vm runs 6.10.2); ctest on Linux (no core change).
⚠️ **To check in the live pass (Q2: brought back, not accepted silently):** whether a selection over several lines shows thin
gaps between lines (Qt paints a selection to each line's natural height, not the pitch), and where the glyphs sit in a line
beside the Mac.

### 🟡 2026-11-05 — Live pass 1 (user, scrivi-vm, build 1.0.359 `8bf0518`): one defect, fixed; two questions

The user: *"4. no gaps in selection between lines within a paragraph. Slight gaps between paragraphs. 5. As I type at the end of
the manuscript, the text above the caret seemed to get reformatted avery couple of keystrokes. As I typed, the top paragraph
got narrower and narrower. When I selected text there the selection and the displayed text were not synchronized. But there
was no lag."* Log kept at `~/evidence-2026-11-05/sp183-scrivi.log` on the VM.

⛔ **Defect (step 5), in `ManuscriptLayout`, fixed:** it recognised its own layout by comparing the first line's position with
values it COMPUTED, to 0.01 px — but Qt stores positions in 1/64 px, so the compare failed on layouts that were already ours,
and every failure re-laid the block, with its width read back from our own (rounded) lines. **Measured** (Docker, Qt 6.4.2,
typing at the end of a 200-scene manuscript): **96 re-lays per keystroke at 16 pt, 59 at 17 pt** (every visible paragraph, on
every paint) against 1 at 13.5 pt. Docker's fonts give whole-pixel widths, so nothing narrowed there; on the VM's metrics each
re-lay shaved the width, and painting and hit-testing ran on different layouts (the unsynchronised selection).
**Fix:** the first line's position as Qt REPORTS IT BACK after our layout is kept on the line style and compared EXACTLY; the
width is Qt's own (remembered whenever Qt lays a block, its lines at exactly (margin, 0)), never read from our lines. After it:
**4 re-lays per keystroke** at every size. **Tests:** `indent_smoke` §1 (5 passes over every block: no re-lay; the first line
spans the text width less the indent) and §5 (re-lays per keystroke bounded) — ✅ the old compare put back in a COPY fails §1
(10 re-lays in 5 passes); the fixed code passes.
⚠️ Not reproduced through the real window on dumas-prose in Docker (stable widths, 16 re-lays per keystroke) — the VM's
re-test confirms the fix.

**Separators (the user: *"On the VM the paragraph separators are not displaying"*):** ✅ clarified (user, 2026-11-05): *"the blue
one between chapters. I see the grey ones between scenes no problem. … If Chapter headings are hidden it will be impossible to
distinguish between a chapter break and a scene break."* — ✅ **Not this Sprint's, and not a regression:** Linux has never drawn
the chapter-ending divider ([T-0575]); it is [SP-184]'s (S5c, [T-0611]), planned with Show chapter titles for exactly the
user's reason.

**The Mac's Heading 1 (the user: *"when I select "Format->Heading 1" with nothing selected, the screen flashes at me"*):** not
this Sprint's — the core REFUSES a heading on an empty line (measured; with the caret in text and nothing selected it works).
Filed as [I-0353] (backlog), a question for the user. ⚠️ The live-pass steps given in chat said to choose Heading 1 THEN type,
which is the refused case — my error.

**Evidence after the fix (2026-11-05):** `indent_smoke` ✅; all 41 Linux smoke steps ✅ 0 failed (⚠️ one batch run just before
reported 13 failed with no assertion printed — the output was not kept; a re-run with full output: 0 failed).

### 🟡 2026-11-05 — Live pass 2 (user, scrivi-vm): the re-test passed

The user: *"all three re-test steps passed."* — typing at the end of dumas, the paragraphs above keep their width; a selection
near the top sits on its text; the grey scene rules appear. ✅ Step 5's defect is fixed on the VM. ✅ [I-0351]: no lag (pass 1,
*"But there was no lag"*) and the re-test.
✅ **Then reported (user, 2026-11-05):** *"C1, C2, C3, and C6 all passed."* — Book convention as drawn; the dialog's three modes
and 2.0 em; Ctrl+Alt+1 on a line with text, then Ctrl+Z; the Mac shows the setting. **All six ACs met; T-0610 and [I-0351]
verified.** ✅ **Closed 2026-11-05** (user, 2026-11-05: *"yes, close SP-183 and plan SP-184"*).

