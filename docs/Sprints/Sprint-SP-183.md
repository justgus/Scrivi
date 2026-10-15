---
sprint: SP-183
epic: EP-048
status: Planning
platform: Linux
created: 2026-10-15
---

# SP-183 — `[Linux]` [EP-048] **S5b**: line spacing and the first-line indent

**Status:** 🔵 **PLANNING** — created 2026-10-15 (user: *"plan EP-048 fully."*). ✅ The S5 split was ruled 2026-10-15
([SP-182] Q2). Line spacing is in this Sprint ([SP-182] Q3, ruled). ⏳ Q1–Q2 are ruled AFTER plan step 0's measurement, not before.
**Carries:** [T-0610] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-048] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). **L10, its block half:** *"⚠️ Qt's first-line
indent is a BLOCK format (a document format, not a highlighter one — the SP-166 hanging-indent finding); measure it before
ruling the mechanism."*
**Builds on:** [SP-182] (S5a): the dialog, `ProjectPreferences` and the one typography source.
**Authority:** [EP-047] P3, P9 (line spacing fixed at 1.45 × size, the same for every face), P10 and AC6
(`../Epics/Closed/Epic-EP-047.md`) · `Scrivi/Views/ManuscriptTypography.swift` (`lineSpacing = 1.45`, the indent modes).
**Size:** M, ⚠️ with real risk that Qt cannot do it all (Q2). Needs a **rig live pass**.

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

## Goal

✅ **Linux draws the project's paragraph indent and line spacing as the Mac does**, without writing a character, dirtying a
scene or recording an undo step. Or, where Qt cannot, the difference is measured and brought back for a ruling.

## Plan

0. **Spike, measured** (Docker Qt 6.4.2 and the rig's Qt 6.10.2): (a)–(e) above, each with numbers or a screenshot, recorded
   here. ⛔ No mechanism is chosen before this.
1. **Mechanism, per Q1**, after step 0.
2. **The rules:** the three modes and Book convention's cases, the amount in ems, the small gap, and line spacing 1.45 × size.
3. **The dialog:** Typography gains Paragraph indent (Book convention · Every paragraph · None) and Indent (0.5–4 em, step
   0.5, disabled at None), Apple's rows and order.
4. **Tests** (a new `indent_smoke` or `settings_smoke` extended):
   - Save bytes unchanged; no dirty scene; no history step.
   - The indent survives typing, Return, undo, reload, paste, structured paste and Replace All.
   - Headings and list items are never indented.
   - Every Book convention case: scene start, after a heading, a list, a block quote, a chapter title.
   - **Mutations:** an indent on a heading · a format that dirties the scene · Book convention indenting a scene's first
     paragraph.
5. **Live pass** (user, rig), against the same project on the Mac.

## Questions — ruled after plan step 0

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **The mechanism.** Candidates: (A) block formats applied inside a guarded programmatic window, with their `contentsChange` filtered out of save and history · (B) a `QPlainTextDocumentLayout` subclass that offsets the first line and sets the line pitch when laying out, so nothing in the document changes · (C) moving the manuscript to `QTextEdit`'s layout. | ⏳ **After step 0.** (C) is not recommended: it reopens [EP-022]'s editor choice. Between (A) and (B), the numbers decide; (B) is preferred if Qt allows it, because the document stays untouched by construction |
| **Q2** | **If Qt cannot draw part of it** (most likely the small gap). | ⏳ **After step 0.** Bring each gap back with a screenshot from both platforms, as [SP-174] Q3 did: build it differently, or accept and record the difference. Nothing is accepted silently |

⛔ **NOT in this Sprint:** the hanging indent for list items (accepted as a difference by [SP-174] Q3; reopened only if this
Sprint's mechanism makes it cheap, and then only by ruling) · Show chapter titles (→ S5c).

## Acceptance Criteria

- [ ] AC0 — Plan step 0's (a)–(e) measured and recorded on both Qt versions; Q1 and Q2 ruled from them
- [ ] AC1 — **L10 (indent):** None · Every · Book convention, with the amount in ems, drawn as on the Mac; headings and list
      items never indented; Book convention's cases hold; the small gap per Q2
- [ ] AC2 — **L10 (line spacing):** 1.45 × size for every face
- [ ] AC3 — Zero characters in the file: save bytes unchanged, no dirty scene, no history step; survives typing, Return,
      undo, reload, paste and Replace All
- [ ] AC4 — The dialog's indent rows; smokes in CI, mutation-checked; ctest green on macOS and Linux
- [ ] AC5 — ✅ **Rig live pass (user)**

## Progress log

### 🔵 2026-10-15 — Sprint CREATED in Planning

User: *"plan EP-048 fully."* Issued with [SP-181], [SP-182] and [SP-184] via `next-id.py`. [T-0610] filed. Depends on [SP-182]
Q2 (the split). Q1–Q2 are ruled after the step 0 measurement.

### 🔵 2026-10-15 — The split ruled

User: *"SP-182 Q2 agreed."* and *"SP-182 Q3-Q4: agreed."* This Sprint stands, with line spacing in it. Q1–Q2 are still ruled
after step 0.
