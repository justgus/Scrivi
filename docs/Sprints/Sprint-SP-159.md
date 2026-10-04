---
sprint: SP-159
epic: EP-046
also: EP-048
status: Planning
platform: Cross
created: 2026-10-04
---

# SP-159 — `[Cross]` [EP-046] S1: Planning — the manuscript DISPLAY design for both platforms (inline rendering + escape hiding)

**Status:** 🔵 **PLANNING** — ⚠️ **not active.** Activation needs the user's approval. ⚠️ Activating it also
ACTIVATES [EP-046], which moves from `Epic-backlog.md` to `Epic-active.md` at that time.
**Epic:** [EP-046] `[Apple]` The Manuscript Renderer — Inline Rendering (WYSIWYG) → [`../Epics/Epic-backlog.md`](../Epics/Epic-backlog.md)
**Tasks:** [T-0584] (Option-Return — a ruling) · [T-0585] (Find/Replace across hidden characters — a design question) →
[`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md). ⚠️ This Sprint RULES and DESIGNS them; it does not implement them.
**Authority:** [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §3A.0 (escape ruling), §4.3 (Model B), §4A (Q7 spike), §10 (sixteen rulings);
[`../Epics/Closed/Epic-EP-045.md`](../Epics/Closed/Epic-EP-045.md) (what E1 built).
**Size:** ⚠️ **MEDIUM — and it ships NO production code.** Its output is a design, rulings, ACs and a Sprint plan.

---

## ⚠️ WIDENED 2026-10-04 — the display half for BOTH platforms

✅ User: *"yes, draft the Epic and widen SP-159"*. ✅ The escape layer's WRITE half on Linux moved to **[EP-049]**
(now). ✅ **This Sprint now designs the DISPLAY half for Apple AND Linux:** hiding escape backslashes, rendering
formatting, revealing markers. [EP-048] (Linux) is then built from what this Sprint rules.

⚠️ **Why the display half is re-opened, not just extended:** the user's model of Apple is *"we present the
Markdown using a stock Markdown renderer and ensure (by escaping) that what the user types will not be rendered as
Markdown."* ⛔ **That is NOT what is built.** The `NSTextView` shows raw stored Markdown, and the hidden backslash
is CUSTOM code (near-zero font + clear colour in storage + caret snapping; R3 = (c), ruled 2026-10-03). Apple's
stock `AttributedString(markdown:)` is a one-way PARSER, used today only as a test oracle. ✅ The user asked for
that implementation to be looked at. ✅ The ESCAPING half of the user's model is exactly right.

### Added investigations

- **W1 — Review the current hiding (R3 = (c)) against a stock-renderer design.** For example: stock rendering for
  every paragraph except the one being edited, which shows its source. Compare the cost, the caret behaviour, the
  undo/save fidelity, and how much custom code each needs.
- **W2 — Verify Apple's APIs** (current macOS/iOS 26–27 docs, via the `.json` route): is there ANY editable
  Markdown or `AttributedString` round-trip in AppKit / TextKit 2 / SwiftUI `TextEditor` that keeps the file as
  written? ⚠️ The trade study found none; check before ruling on it.
- **W3 — Measure Qt's Markdown engine** on the rig's Qt 6.4: `QTextDocument::setMarkdown()` / `toMarkdown()` (md4c).
  Measure round-trip fidelity on the dumas fixture and on escaped text (⚠️ `toMarkdown()` rewrites the whole document
  — R2 and AC9 forbid that on save), `QPlainTextEdit` (today's editor, plain text only) vs `QTextEdit`, and whether Qt
  can hide a character without leaving a gap or a caret stop.

## Why a planning Sprint first

✅ EP-045 began with a design doc and a measured study, and that is why most of its surprises were found
BEFORE code. ⚠️ EP-046 is the Epic the study called "the expensive one". It has no ACs and no design yet. ⛔ Its
entry still describes a cost that E1 has partly paid (below).

---

## ✅ What is already settled (do not re-litigate)

| Settled | Where |
| ------- | ----- |
| **Model B (WYSIWYG):** formatting renders, markers hide unless the caret is inside them | Study §4.3 (user: *"my inclination is to wysiwyg"*) |
| **Typed markup is ALWAYS escaped.** Formatting is authored ONLY by command: bold, italic, heading, list (+ scene break) | Study §3A.0 |
| **Existing markup in files is INTENDED** and renders (e.g. the dumas fixture's `##` scene headings) | EP-045 R2 |
| **Whole-LINE markers (`#`, bullets) before INLINE ones (`**`)** | Study §4.3 |
| **Unexposed block intents (code block, quote, table) are DRAWN as prose** — carried as an AC of this Epic | EP-045 AC7, Q-AC7 = (a) |
| **The scene-break COMMAND needs a `scrivi_split_scene` endpoint that does not exist** — out of scope | EP-045 "verified gap" |

## ⚠️ What E1 changed about the cost (measured at planning, 2026-10-04)

⛔ EP-046's entry says Model B needs *"STORAGE attributes (which `ManuscriptTextView.swift:366-369` STRIPS on every
undo)"*. ✅ **E1 shipped exactly that route for the escape backslash** (EP-045 R3 = (c)): `EscapeHidingStyler` is the
text storage's delegate, it re-applies hiding after EVERY edit (typing, paste, undo/redo apply, rebuild), and
caret/selection snapping keeps the caret out of hidden characters. ✅ The study's *"storage-attribute route that
survives the undo path — READ, NOT RUN"* (§4.3) has therefore RUN, in production, for one character.
⚠️ **The open question is whether it GENERALISES** — to multi-character markers (`**`, `##`), to un-hiding when the
caret enters (Model B's re-entry, which the study lists as *"UNDESIGNED"*), and to the cost of parsing for intent
on every edit. → the spikes below.

---

## Plan

### Spikes (throwaway code in the scratchpad; numbers recorded here)

- **S1 — Hide multi-character markers with the E1 mechanism.** Generalise the styler's hidden attributes to `**`,
  `_`, `#`/`##` prefixes and list bullets. Do they survive undo/redo apply and rebuild? ⚠️ The caret snap handles ONE
  hidden character today (`MarkdownEscapes.isUnreachable`); a two-character `**` has two unreachable boundaries.
- **S2 — Re-entry (Model B).** Show a span's markers when the caret enters it and hide them on exit. Measure the
  restyle cost on a selection change, ⚠️ and whether un-hiding REFLOWS the line under the writer (text jumping
  sideways as `**` appears is the known risk of this model).
- **S3 — Drawing the formatting.** Bold/italic as font traits, headings as size + paragraph style: storage
  attributes vs rendering attributes, and how each survives the styler's re-apply and the undo path.
- **S4 — Parse cost.** `.full` parsing for intents per edited paragraph, and over the whole 1.85 MB manuscript at
  rebuild, on `dumas-prose-timelines` (1,185 scenes, each with a `##` heading: a ready-made test corpus). ⚠️ Against
  [I-0275]'s baseline (a 20–120 ms keystroke, AppKit's) — rendering must not add to it measurably.
- **S5 — Commands.** Bold on a selection writes `**…**` UNESCAPED (`insertVerbatim`), as ONE undo step; toggling it off;
  a selection that contains escape pairs; one that spans paragraphs or scenes.

### Deliverables

1. **Design doc** `docs/Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`, as E1's was: the mechanism, the element order,
   the re-entry model, the commands, the Find/Replace model, and the spike numbers.
2. **EP-046's acceptance criteria**, written into the Epic, including the carried AC7 criterion.
3. **The rulings below**, taken.
4. **An implementation Sprint plan**: Sprints sized and ordered (whole-line markers first, per the study).

## ⚠️ Rulings owed (taken during the Sprint, after the spikes inform them)

| # | Question |
| - | -------- |
| **Q-E2-1** | **Re-entry granularity:** when the caret enters, which markers appear — the enclosing span's only, the whole paragraph's, or the line's? |
| **Q-E2-2** | **The command surface:** menu items + `⌘B`/`⌘I`? Which heading levels? Which list kinds (bulleted, numbered)? |
| **Q-E2-3** | **Sprint split:** whole-line elements (headings, lists) first, then inline (bold, italic) — confirm, or merge |
| **Q-E2-4** = [T-0584] | **Option-Return:** paragraph · deliberate hard break · leave it (recommendation at filing: hard break) |
| **Q-E2-5** = [T-0585] | **Find/Replace:** match the PRESENTED text (what the writer sees) and write replacements through the escape layer? |
| **Q-E2-6** | ⚠️ **REPLACED 2026-10-04 — the rendering ARCHITECTURE, for both platforms:** (i) custom hiding over the source (today's R3 = (c), generalised); (ii) stock rendering for non-active paragraphs + source for the active one; (iii) other — after W1–W3. ⚠️ It may revise R3 = (c) on Apple |
| **Q-E2-8** | **Linux display method where Qt cannot do it Apple's way:** match the RESULT by Qt means and record the difference (the T-0576 precedent), per element |
| **Q-E2-7** | **Malformed existing markup** (an unclosed `**` in an old file): render as the parser reads it, or show it literally? |

## Acceptance Criteria (planning)

- [ ] **AC-P1** — Spikes S1–S5 run, with their numbers recorded in this file and in the design doc.
- [ ] **AC-P2** — The E2 design doc exists, and the user has reviewed it.
- [ ] **AC-P3** — Q-E2-1 … Q-E2-7 ruled by the user.
- [ ] **AC-P4** — EP-046 has written ACs (incl. the carried block-intents criterion), and the Epic record moves to `Epic-active.md`.
- [ ] **AC-P5** — Implementation Sprints proposed, ordered and sized; [T-0584] and [T-0585] assigned to one of them.
- [ ] **AC-P6** — W1–W3 run and recorded; Q-E2-6 and Q-E2-8 ruled; ✅ **[EP-048] SCOPED** (its ACs written) from the ruled design.

⛔ **NOT in this Sprint:** any production code; the scene-split endpoint; `paragraphIndent` and fonts ([EP-047]); Linux's escape WRITE half ([EP-049]); BUILDING Linux's display ([EP-048] — designed here, built there).

---

## Progress log

### 🔵 2026-10-04 — Sprint CREATED in Planning

✅ Created at the user's request: *"file the two tasks and create a planning sprint for EP-046"*. ✅ ID issued by
`next-id.py`. ✅ [T-0584] and [T-0585] filed under EP-046. ✅ Checked first: the study's settled rulings and open
items (§4.3 lists re-entry as *"UNDESIGNED"* and the undo-surviving storage route as *"READ, NOT RUN"*), and that
E1's `EscapeHidingStyler` has since run that route in production.

### 🔵 2026-10-04 — WIDENED to `[Cross]`: the display design for both platforms

✅ User: *"yes, draft the Epic and widen SP-159"*. ✅ Added W1–W3, replaced Q-E2-6 with the rendering-architecture
question, added Q-E2-8 and AC-P6. ✅ The Linux write half → [EP-049]. ⚠️ Recorded: the hidden backslash is Claude's
custom implementation (R3 = (c)), not TextKit 2 behaviour. The user asked for it to be reviewed (W1).

