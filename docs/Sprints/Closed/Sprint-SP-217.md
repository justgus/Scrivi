---
sprint: SP-217
epic: none
status: Closed
closed: 2026-11-13
platform: Linux
created: 2026-11-13
---

# SP-217 — `[Linux]` [I-0349] Hidden markup without the fallback-font search

**Status:** ✅ **CLOSED** 2026-11-13 (user, 2026-11-13: *"Linux steps 1-5 and Mac step 6 all pass, close SP-217"*). Activated 2026-11-13 (user, 2026-11-13: *"Q1–Q3 as recommended, activate and implement SP-217"*). Planned 2026-11-13 (user: *"close SP-216 and plan the I-0349 font fix"*). ✅ Q1–Q3 ruled as recommended.
**Fixes:** [I-0349] Part 2 → [`../../Issues/Issue-backlog.md`](../../Issues/Issue-backlog.md).
**Epic:** none (Q1).
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
  **Spike (AC1, 2026-11-13)** — harness `spike.cpp` (in a scratch folder, not the repo): the river-of-time and dumas texts in a
  `QPlainTextEdit`, Literata 16 pt, a highlighter hiding `\`+punctuation, `**` and `# `; a fresh process per candidate; first
  layout timed from `setPlainText` through the first paint; the face read from the hidden `\`'s glyph run; residue = a probe
  line's width with the marks hidden minus the same line with them deleted.

  | Candidate | scrivi-vm (Qt 6.10.2, 2,278 fonts): river · dumas first layout | Hidden `\` drawn by | Residue | Fallback engines (gdb) |
  | --- | --- | --- | --- | --- |
  | today: 0.01 pt | **9,505 ms · 9,679 ms** | Noto Color Emoji @ 0 px | 1.05 px | up to index 255 |
  | (a) 0.01 pt + the face named | 9,428 ms · 9,681 ms | Noto Color Emoji @ 0 px | 1.05 px | — |
  | 0 % letter spacing, body size | 0 ms (river) | Literata @ 21 px | ⛔ 110.97 px (spacing 0 % is ignored) | — |
  | 1 px (0.75 pt) alone | 7 ms | Literata @ 1 px | ⛔ 6.19 px | — |
  | **(b) 1 px + `AbsoluteSpacing` minus the mark's own advance** | **7 ms · 336 ms** | **Literata @ 1 px** | **1.05 px — the same as today** | **index 0 only** |

  Qt 6.4.2 (Docker, 8 fonts): (b) Literata @ 1 px, residue 1.05 px; today's format crashed the harness's glyph-run probe (a 0 px
  glyph run). The 1.05 px is the probe line's own, the same under both formats — the hidden marks add none (no-hinting changed
  nothing). ✅ **(b) chosen**: it alone resolves in the face with no fallback and leaves today's residue.
1. **Build** the ruled format in `hiddenFormat()`; nothing else in the presenter changes.
2. **Checks:** a smoke lays out hidden `\`, `**` and `#` and checks (i) the hidden runs are drawn by the manuscript's face (the
   `QRawFont` of the run's glyphs, or `QFontInfo` — the spike picks the reliable one), (ii) zero residue as W3, (iii) caret stops
   unchanged; mutation-checked by restoring 0.01 pt. ctest and every smoke green.
3. **Measure on scrivi-vm:** the river-of-time and dumas, the watchdog's "not answering" after `load: finished`, before and after.
4. **Live pass (user)** on scrivi-vm.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Epic.** I-0349 was backlogged with no Sprint (2026-11-04). The active Epics are EP-054 (Import & Export), EP-055 (iPad and iPhone) and EP-057 (holding Linux rig passes). | **None** — a standalone Sprint for a Linux defect, as [I-0349] has no Epic. | ✅ **RULED as recommended** (user, 2026-11-13: *"Q1–Q3 as recommended, activate and implement SP-217"*) |
| **Q2** | **The two UI glyphs** (◎ `U+25CE`, − `U+2212`): in or out? | **In, if the spike finds them costly** (> 100 ms at launch on scrivi-vm): name where they are drawn and draw them from a face that has them. Out (a new Issue) otherwise. | ✅ **RULED as recommended** (user, 2026-11-13: *"Q1–Q3 as recommended, activate and implement SP-217"*) |
| **Q3** | **Measure the Mac too?** | **Yes, once:** open the river-of-time on the Mac with the diagnostics on and confirm no stall after the load — so "Apple is unaffected" is measured, not assumed. No Apple code changes. | ✅ **RULED as recommended** (user, 2026-11-13: *"Q1–Q3 as recommended, activate and implement SP-217"*) |

⛔ **NOT in this Sprint:** the escape layer itself (why every `.` and `,` in these scenes is stored escaped — a separate question
if the user wants it asked), moving the first layout off the main thread, [I-0360].

## Acceptance Criteria

- [x] AC1 — The spike's table (both Qt versions): each candidate's first-layout time, fallback engines loaded and residue; the
      chosen format ruled
- [x] AC2 — Hidden markup (`\`, `**`, `#`) is drawn by the manuscript's face with no fallback-font search, and still leaves
      0.00 pt residue and the same caret stops; a smoke checks all three (mutation-checked); ctest and the smokes green
- [x] AC3 — On scrivi-vm, the river-of-time and dumas answer within 500 ms of `load: finished` (today 9.9 s and ~5.5 s),
      measured from the log
- [x] AC4 — Q2 and Q3 as ruled
- [x] AC5 — **Live pass (user)** on scrivi-vm: both projects open with no freeze; hidden markup still hidden, revealed markup still
      dimmed, the caret still steps over hidden marks

## Progress log

- **2026-11-13** — 🔵 **Created in Planning** (user, 2026-11-13: *"close SP-216 and plan the I-0349 font fix"*), issued via
  `next-id.py`, from I-0349 Part 2's measurements the same day. Q1–Q3 to rule.
- **2026-11-13** — 🟡 **Activated**; ✅ Q1–Q3 ruled as recommended (user, 2026-11-13: *"Q1–Q3 as recommended, activate and implement SP-217"*).
- **2026-11-13** — ✅ **AC1 met** (the spike, above). ✅ **Built (AC2):** `ManuscriptPresenter::hide(start, count, text)`
  (`ManuscriptPresenter.cpp:282`) replaces `hiddenFormat()`: per hidden character, 0.75 pt, transparent, `AbsoluteSpacing` of
  minus its advance in the document's face at that size; the three call sites (heading prefix, inline markers, escapes) use it.
  `presenter_smoke`: a hidden `\`, `#` and `*` each drawn by the document's own face at ≥ 1 px (new); `drawnHidden` now reads
  "< 1 pt"; the revealed-prefix check tightened to "> 1 pt" (0.75 passed the old "> 0.5"). Mutation-checked: 0.01 pt back →
  the three face checks fail (the probe returns no face for a 0 px mark, as Qt 6.4 segfaults reading its glyph run); no
  spacing → the two no-width checks fail. Docker Qt 6.4.2: ctest 847/847, all 43 smokes. ✅ **AC2 met.**
- **2026-11-13** — **AC3 (scrivi-vm, the user's build 1.0.391 vs this change, the same copies of each project + app-support in
  `/tmp`, Xvfb):** river-of-time **11,581 ms → no stall** (the watchdog never fires) ✅; dumas **10,973 ms → 751 ms** ⛔ over the
  500 ms written into AC3. Sampled (gdb, main thread): the 751 ms is no longer fonts — it is `ManuscriptAccessibility` →
  `ManuscriptFind::PresentedText::refresh` → `buildScene` for all 1,174 scenes at once (`hiddenAtRest`, `analyzeMarkdown`,
  `sceneOf`), the accessibility layer of [I-0349] Part 1. ⏳ **The user's ruling: amend AC3 for dumas, or widen the Sprint.**
- **2026-11-13** — **Q2 measured:** first layout in a fresh process, the app font: `A` 14 ms, ◎ 37 ms (DejaVu Sans), − 18 ms
  (Noto Sans Bengali) — both under 100 ms, so **out** as ruled → [I-0361] (the timeline's zoom buttons, `TimelinePanel.cpp:57`,
  `:74`). Q3 (the Mac) is a step in the live pass.
- **2026-11-13** — ✅ **AC3 met** (user, 2026-11-13: *"AC3 is met.  Your 500ms limit was arbitrary.  751ms is acceptable."*) dumas' 751 ms is accepted; its cause stays recorded as [I-0349] Part 3.
- **2026-11-13** — ✅ **AC4, AC5 met** (user, 2026-11-13: *"Linux steps 1-5 and Mac step 6 all pass, close SP-217"*): Q2 → [I-0361]; Q3 measured on the Mac (step 6: no stall after the load); the
  Linux live pass (steps 1–5) on scrivi-vm. [I-0349] Part 2 ✅ verified; Part 3 stays open in the backlog.
- **2026-11-13** — ✅ **CLOSED** (user, 2026-11-13: *"Linux steps 1-5 and Mac step 6 all pass, close SP-217"*).
