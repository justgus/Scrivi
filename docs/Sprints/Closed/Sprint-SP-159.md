---
sprint: SP-159
epic: EP-046
also: EP-048
status: Closed
closed: 2026-10-05
activated: 2026-10-05
platform: Cross
created: 2026-10-04
---

# SP-159 — `[Cross]` [EP-046] S1: Planning — the manuscript DISPLAY design for both platforms (inline rendering + escape hiding)

**Status:** ✅ **CLOSED 2026-10-05 (user-approved):** *"yes, mark T-0587 as verified.  Close SP-159 and create E2-S1"* — activated 2026-10-05. ✅ All six ACs met; [T-0587] VERIFIED and archived. ✅ [EP-046] ACTIVATED with it. ⛔ Ships NO production code.
**Epic:** [EP-046] `[Apple]` The Manuscript Renderer — Inline Rendering (WYSIWYG) → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md)
**Tasks:** [T-0587] (this Sprint's design work) · [T-0584] (Option-Return — a ruling) · [T-0585] (Find/Replace across hidden characters — a design question) →
[`../Tasks/Task-active.md`](../../Tasks/Task-active.md). ⚠️ This Sprint RULES and DESIGNS them; it does not implement them.
**Authority:** [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §3A.0 (escape ruling), §4.3 (Model B), §4A (Q7 spike), §10 (sixteen rulings);
[`../Epics/Closed/Epic-EP-045.md`](../../Epics/Closed/Epic-EP-045.md) (what E1 built).
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

## ✅ Rulings — ALL TAKEN 2026-10-05 (as recommended; see the progress log and design §0A)

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

- [x] **AC-P1** — Spikes S1–S5 run, with their numbers recorded in this file and in the design doc. ✅ 2026-10-05 (below; design doc §10).
- [x] **AC-P2** — The E2 design doc exists, and the user has reviewed it. ✅ APPROVED 2026-10-05 → [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md).
- [x] **AC-P3** — Q-E2-1 … Q-E2-7 ruled by the user. ✅ 2026-10-05 (with Q-E2-8).
- [x] **AC-P4** — EP-046 has written ACs (incl. the carried block-intents criterion), and the Epic record moves to `Epic-active.md`. ✅ AC1–AC11 WRITTEN into EP-046 (`Epic-active.md`) 2026-10-05; record moved at activation.
- [x] **AC-P5** — Implementation Sprints proposed, ordered and sized; [T-0584] and [T-0585] assigned to one of them. ✅ E2-S1…E2-S4 (design §12, order ruled Q-E2-3); [T-0584] → E2-S3, [T-0585] → E2-S4.
- [x] **AC-P6** — W1–W3 run and recorded; Q-E2-6 and Q-E2-8 ruled; ✅ **[EP-048] SCOPED** (its ACs written) from the ruled design. ✅ W1–W3 recorded; Q-E2-6 + Q-E2-8 ruled; EP-048 SCOPED 2026-10-05 (ACs L1–L8, `Epic-backlog.md`).

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

### 🟡 2026-10-05 — Sprint ACTIVATED (user-approved); [EP-046] ACTIVATED; [T-0587] allocated; [T-0584] + [T-0585] → `Task-active.md`

✅ User: *"activate SP-159"*



### 🟡 2026-10-05 — Spikes S1–S5 and W1–W3 RUN; design doc DRAFTED; rulings owed

✅ User: *"The active Sprint is SP-159. Please implement it."* ✅ Design doc →
[`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md). ⛔ No production code
changed. Spike code is throwaway, not committed: `scratchpad/sp159/h/*.swift` (macOS 27.2, TextKit 2) and
`scratchpad/sp159/qt/w3.cpp` (Ubuntu 24.04 + Qt 6.4.2 in Docker, the Linux image's base).

**⚠️ The headline: a route nobody had measured — (a′).** E1's rejected route (a) (`NSTextContentStorageDelegate`)
failed because it REMOVED the backslash and so changed the paragraph's length. Apple's documented contract is
*"must have `range.length`"*. ✅ Returning the SAME characters with styled attributes keeps that contract and
measured clean: caret steps one source offset at a time; hidden widths identical to the storage route; reveal by an
attributes-only edit; storage stays plain, so the undo path needs nothing re-applied; ✅ **rebuild of 1.85 MB costs
0.45 ms + 6.8 ms viewport layout, vs 201–682 ms for E1's storage route generalised.** Qt has the same shape:
`QSyntaxHighlighter` (presentation-only, survives undo, zero residue).

| Spike | Result |
| ----- | ------ |
| **S1** hide `**`/`##` with E1's mechanism | ✅ `**` hidden 152.71 pt vs target 152.69; `## ` 267.06 vs 267.05; survives undo-path replace + rebuild (attributes identical). Invisible caret stops in `a **bold** b \*x`: E1 snap 4 → generalised hidden-run snap **0**. ⛔ `NSTextList` bullet draws from the FIRST character → invisible when `- ` is hidden |
| **S2** re-entry | restyle 0.17–0.20 ms (2 KB/40 spans: 1.32 ms). ⚠️ block/line reveal re-wraps the paragraph (11 → 12 lines); span reveal does not (11 → 11). Every policy shifts the caret by the revealed markers (16 pt) |
| **S3** drawing | ⛔ a `.font` RENDERING attribute neither re-lays out nor even draws bold; heading size stays 16 pt. Fonts must be in the presented paragraph or storage. Attribute-only edits: 0 `textDidChange`, no undo, 0 character edits |
| **S4** parse cost (dumas, 1.85 MB) | per keystroke ≤ 0.32 ms (vs [I-0275] 20–120 ms). ⛔ Eager rebuild: E1 5.2 ms → **201 ms** (as stored) / **682 ms** (+ emphasis). Route (a′): **0.45 ms + 6.8 ms**, 143 paragraphs styled; keystroke 0.86 vs 0.53–0.66 ms. Headings rendered 1,171/1,172 |
| **S5** commands (2,000 selections each, escaped dumas) | `**`/`*`: 2000/2000 word-aligned, 1967–1970 arbitrary, **2000/2000** with the flank rule. ⛔ `_`: 286/2000 arbitrary → italic writes `*`. One command = 1 `textDidChange`. Cross-paragraph wraps each paragraph in one replacement; toggle-off rewrites the node |
| **W1** | (ii) "stock render for inactive paragraphs" cannot be built on TextKit 2 (violates the `range.length` contract; route (a)'s 0 → 8 caret jump); its VISIBLE behaviour = (iii) with block reveal |
| **W2** | ⛔ macOS 27.2 SDK has NO Markdown encoder: only `MarkdownParsingOptions`/`SourcePosition`/decoding. SwiftUI `TextEditor(AttributedString)` (26+) edits attributes, not Markdown. `NSTextList.includesTextListMarkers` (26+) found |
| **W3** | ⛔ `toMarkdown()`: 0/1,185 identical, 1,179 re-wrapped at 80 cols, **escapes written unescaped** (literal → live markup). ✅ md4c reads escapes as typed 1,199/1,200. ✅ `QSyntaxHighlighter` on `QPlainTextEdit`: residue 0.00 pt, heading h 32 vs 20, stored format untouched, survives undo; ⚠️ same invisible caret stops. `libmd4c-dev` 0.4.8 in Ubuntu 24.04 |

**Recommendations (⏳ for the user to rule — AC-P3):** Q-E2-6 = **(iii) route (a′)** (⚠️ revises R3 = (c)'s
*mechanism*, not what it shows) · Q-E2-1 = **span** for inline, **line** for prefixes · Q-E2-2 = ⌘B / ⌘I, headings 1–3
(⌥⌘1–3, ⌥⌘0 body), bulleted + numbered lists, Return continues a list · Q-E2-3 = whole-line first (E2-S1 headings,
E2-S2 inline, E2-S3 commands + T-0584, E2-S4 Find + T-0585) · Q-E2-4 = **(b)** hard break · Q-E2-5 = **match the
presented text**, replace through the escape layer · Q-E2-7 = **as the parser reads it** (unclosed `**` shows
literally) · Q-E2-8 = `QSyntaxHighlighter` + parser option **(L-b)** md4c in ScriviCore, tied to Apple's parser by an
agreement test.

### ✅ 2026-10-05 — Design APPROVED; all eight rulings TAKEN; AC-P2…AC-P6 met

✅ User: *"I have reviewed the design document and approve it. I also approve your recommendations for the 8 rulings."*
✅ Rulings as recommended (design §0A): Q-E2-1 span/line · Q-E2-2 ⌘B ⌘I, H1–3, Body, bulleted + numbered, Return
continues a list · Q-E2-3 E2-S1 → S4 · Q-E2-4 (b) hard break · Q-E2-5 presented text · Q-E2-6 (iii) route (a′)
(⚠️ revises R3 = (c)'s mechanism; noted in the E1 design §4.4) · Q-E2-7 as the parser reads it · Q-E2-8
`QSyntaxHighlighter` + md4c in ScriviCore (L-b). ✅ EP-046 ACs AC1–AC11 written into the Epic. ✅ [T-0584] → E2-S3,
[T-0585] → E2-S4. ✅ EP-048 scoped (ACs L1–L8). ⏳ **Sprint close needs the user's approval.**

### ✅ 2026-10-05 — Sprint CLOSED (user-approved); [T-0587] VERIFIED + archived; E2-S1 created as [SP-161]

✅ User: *"yes, mark T-0587 as verified.  Close SP-159 and create E2-S1"*
✅ [T-0587] → [`../../Tasks/Verified/Task-verified-0587.md`](../../Tasks/Verified/Task-verified-0587.md). ✅ [T-0584] and [T-0585] are RULED
here; their implementation stays in `Task-active.md`, assigned to E2-S3 / E2-S4. ✅ E2-S1 → **[SP-161]** (🔵 Planning) →
[`Sprint-SP-161.md`](Sprint-SP-161.md).

---

## Retrospective

**Completed:** ✅ AC-P1…AC-P6. Spikes S1–S5 and investigations W1–W3 were measured, and the E2 design doc was approved. All
eight rulings Q-E2-1…Q-E2-8 were taken. EP-046's ACs AC1–AC11 are written, EP-048 is scoped (L1–L8), and the
implementation Sprints E2-S1…E2-S4 are ordered.
**Returned to Backlog:** none. [T-0584] and [T-0585] were RULED here as planned; their implementation was always for later
EP-046 Sprints (E2-S3, E2-S4), and both stay in `Task-active.md`.
**What went well:** ✅ re-reading a REJECTED route against Apple's documented contract found route (a′). Route (a) failed
because it changed the paragraph's length, not because the delegate was the wrong door. The result is 0.45 ms instead of
201–682 ms per rebuild, and no undo-path re-application. ✅ Measuring Qt in the Linux image's own base (Docker) settled
W3 without touching the rig. ✅ A spike result that looked wrong (5 headings of 1,172) was debugged before it was
recorded (a divider line was being merged into the next block).
**What to improve:** ⚠️ The first S4 timings (328/850 ms) were reported in chat before the heading-count probe was checked; the same
bug had inflated them (corrected: 201/682 ms). Check the correctness probe BEFORE reading the timing.
⚠️ Not measured and carried into E2-S1 as its first tasks: shift-selection across hidden runs, VoiceOver and spell-check
at hidden markers, the one dumas `##` line that does not render. → owned by **[SP-161]**.
