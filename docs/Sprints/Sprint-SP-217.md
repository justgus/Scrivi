---
sprint: SP-217
epic: none (Q1)
status: Planning
platform: Linux
created: 2026-11-13
---

# SP-217 — `[Linux]` [I-0349] Hidden markup without the fallback-font search

**Status:** 🔵 **Planning** 2026-11-13 (user, 2026-11-13: *"close SP-216 and plan the I-0349 font fix"*). Q1–Q3 to rule.
**Fixes:** [I-0349] Part 2 → [`../Issues/Issue-backlog.md`](../Issues/Issue-backlog.md).
**Epic:** Q1.
**Size:** S — one format, measured before and after; no new command.

---

## What is known (2026-11-13, I-0349 Part 2)

- **The stall is not the load.** `the-river-of-time` (3 scenes): `load: finished in 202 ms`, then the window does not answer for
  **9.9 s** (9.8 s on relaunch; 12.5 s on a `/tmp` copy, so not SMB). dumas: ~5.5 s, the same stack (Part 1's samples).
- **Where:** the main thread's first layout of the manuscript — `EditorShell::refreshBranchMarks` (`EditorShell.cpp:2783`) →
  `ManuscriptEditor::visibleBlockNumbers` (`ManuscriptEditor.cpp:1094`) → `QPlainTextDocumentLayout::layoutBlock` →
  `QTextEngine::shapeText` → `QFontEngineMulti::ensureEngineAt` → `FT_New_Face`, fallback engine after fallback engine.
- **Why:** hidden markup is drawn in `ManuscriptPresenter::hiddenFormat()` (`ManuscriptPresenter.cpp:281`) at **0.01 pt** — 0 px.
  At that size the manuscript's face yields no glyph for the hidden `\` (the character gdb caught), so Qt opens the system's
  fallback fonts in turn: **2,278** on scrivi-vm. Every hidden mark (`\`, `**`, `#`) takes this path; a text whose punctuation is
  escaped has one on nearly every sentence.
- **What must not change** (design `Scrivi_Manuscript_Renderer_E2_Design_v0_1.md` §2.4, W3): a hidden run leaves **0.00 pt**
  residue (194.45 pt = target), stays out of the stored text (highlighter formats are presentation only), and keeps the caret snap.
- **Apple is not affected** as far as is known: it uses the same 0.01 pt (W1/W2), and Core Text resolves it without a fallback
  search. Not measured this Sprint (Q3).
- **Two UI glyphs** also went to the fallback fonts at launch: `U+25CE` ◎ (122 engines tried) and `U+2212` − (10). Where they are
  drawn is not yet named (Q2).

## Goal

A manuscript with hidden markup is answering as soon as its load finishes: hidden marks resolve in the manuscript's own face, with
no fallback-font search, while staying invisible and taking no width.

## Plan

0. **Spike (no app change):** in a harness on scrivi-vm (its 2,278 fonts, Qt 6.10.2) and in Docker (Qt 6.4.2), lay out the
   river-of-time and dumas texts under each candidate hidden format; for each, time the first layout, count the font engines
   loaded (gdb on `ensureEngineAt`, as in I-0349) and measure the residue as W3 did. Candidates:
   - **(a)** 0.01 pt with the manuscript's family set on the format, so the face is asked first;
   - **(b)** a 1 px font (the smallest a face will rasterise) with letter spacing that cancels the advance
     (`QFont::AbsoluteSpacing`, minus the glyph's width), transparent;
   - **(c)** another the spike turns up.
   The format whose residue is 0.00 pt and whose first layout loads no fallback engine is Q-ruled into step 1.
1. **Build** the ruled format in `hiddenFormat()`; nothing else in the presenter changes.
2. **Checks:** a smoke lays out hidden `\`, `**` and `#` and checks (i) the hidden runs are drawn by the manuscript's face (the
   `QRawFont` of the run's glyphs, or `QFontInfo` — the spike picks the reliable one), (ii) zero residue as W3, (iii) caret stops
   unchanged; mutation-checked by restoring 0.01 pt. ctest and every smoke green.
3. **Measure on scrivi-vm:** the river-of-time and dumas, the watchdog's "not answering" after `load: finished`, before and after.
4. **Live pass (user)** on scrivi-vm.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Epic.** I-0349 was backlogged with no Sprint (2026-11-04). The active Epics are EP-054 (Import & Export), EP-055 (iPad and iPhone) and EP-057 (holding Linux rig passes). | **None** — a standalone Sprint for a Linux defect, as [I-0349] has no Epic. | — |
| **Q2** | **The two UI glyphs** (◎ `U+25CE`, − `U+2212`): in or out? | **In, if the spike finds them costly** (> 100 ms at launch on scrivi-vm): name where they are drawn and draw them from a face that has them. Out (a new Issue) otherwise. | — |
| **Q3** | **Measure the Mac too?** | **Yes, once:** open the river-of-time on the Mac with the diagnostics on and confirm no stall after the load — so "Apple is unaffected" is measured, not assumed. No Apple code changes. | — |

⛔ **NOT in this Sprint:** the escape layer itself (why every `.` and `,` in these scenes is stored escaped — a separate question
if the user wants it asked), moving the first layout off the main thread, [I-0360].

## Acceptance Criteria

- [ ] AC1 — The spike's table (both Qt versions): each candidate's first-layout time, fallback engines loaded and residue; the
      chosen format ruled
- [ ] AC2 — Hidden markup (`\`, `**`, `#`) is drawn by the manuscript's face with no fallback-font search, and still leaves
      0.00 pt residue and the same caret stops; a smoke checks all three (mutation-checked); ctest and the smokes green
- [ ] AC3 — On scrivi-vm, the river-of-time and dumas answer within 500 ms of `load: finished` (today 9.9 s and ~5.5 s),
      measured from the log
- [ ] AC4 — Q2 and Q3 as ruled
- [ ] AC5 — **Live pass (user)** on scrivi-vm: both projects open with no freeze; hidden markup still hidden, revealed markup still
      dimmed, the caret still steps over hidden marks

## Progress log

- **2026-11-13** — 🔵 **Created in Planning** (user, 2026-11-13: *"close SP-216 and plan the I-0349 font fix"*), issued via
  `next-id.py`, from I-0349 Part 2's measurements the same day. Q1–Q3 to rule.
