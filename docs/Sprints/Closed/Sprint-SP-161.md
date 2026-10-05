---
sprint: SP-161
epic: EP-046
status: Closed
closed: 2026-10-05
activated: 2026-10-05
platform: Apple
created: 2026-10-05
---

# SP-161 — `[Apple]` [EP-046] **E2-S1**: the presenter (route (a′)) + headings

**Status:** ✅ **CLOSED 2026-10-05 (user-approved):** *"All steps in the Live pass passed.  We can Verify T-0588.  If available, you are also authroized to close SP-161."* — activated 2026-10-05. ✅ Q1 + Q2 ruled. ✅ All ACs met; [T-0588] VERIFIED and archived.
**Task:** ✅ [T-0588] → [`../../Tasks/Verified/Task-verified-0588.md`](../../Tasks/Verified/Task-verified-0588.md)
**Epic:** [EP-046] `[Apple]` The Manuscript Renderer — Inline Rendering → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md)
**Authority:** [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) — ✅ approved 2026-10-05;
§0A (rulings), §2–§3 (mechanism), §4.1 (headings), §11 (ACs), §12 (Sprint order). Planning record:
[`Closed/Sprint-SP-159.md`](Closed/Sprint-SP-159.md).
**Size:** L. ⚠️ It replaces E1's storage styler under every existing escape test, so it is mostly a MIGRATION with
headings as its first visible result.

---

## Goal

✅ **Install route (a′) and prove it on the whole-line element first.** An `NSTextContentStorageDelegate` presents each
paragraph with the SAME characters and styled attributes; storage stays plain. E1's escape hiding moves onto it, and
`##` lines (every dumas scene opens with one) render as headings, revealing their prefix when the caret is on the line.

## EP-046 ACs this Sprint meets

| AC | Criterion (Epic wording) |
| -- | ------------------------ |
| **AC1** | Storage stays plain: rendering writes no storage attribute; save bytes unchanged |
| **AC2** | Headings render (`#`–`######`: heading font, prefix hidden), incl. every dumas `##` |
| **AC4** *(line half)* | Re-entry for whole-line prefixes: the prefix appears while the caret is on the line; attributes-only (no history event, no undo step) |
| **AC5** | No invisible caret stop: the hidden-run snap, both directions, clicks and shift-selection |
| **AC7** | Unexposed block intents draw as prose (indented, quote, table) |
| **AC8** | No added rebuild cost: 1.85 MB open/rebuild within 10 ms of today; < 1 ms added per keystroke |
| **AC11** | E1's escape hiding moved onto the presenter; `EscapeHidingStyler` retired; E1's suites stay green |

## Plan

1. **Measure the carried gaps first** (SP-159 retrospective; each one is a spike-sized check before code):
   shift-selection and drag-selection across a hidden run; VoiceOver and spell-check at a hidden character; the one
   dumas `##` line (of 1,172) that did not render as a heading.
2. **Block analyzer** (pure, new Swift file → `project.pbxproj` in the same step): block → kind, markers, prefixes,
   bold/italic ranges, spans, from `AttributedString(markdown:, .full, appliesSourcePositionAttributes)` (design §3.1).
   Includes the AC7 demotion (design §3.2). Tested against the parser over a corpus.
3. **The presenter**: the `NSTextContentStorageDelegate`, same-length paragraphs (Apple's contract), with a block cache
   keyed on range + text (TextKit 2 asks per LINE; design §13).
4. **Migrate E1** (AC11): the presenter hides escape backslashes with the unchanged `MarkdownEscapes.map`.
   `EscapeHidingStyler` (`ManuscriptTextView.swift:3070`) and its install (`:321`) are removed. The storage delegate
   slot frees up. ⚠️ `SceneBoundaryTable`'s comment (`:2992`) explains it observes a notification BECAUSE the slot
   was taken; it keeps the notification (it needs `willProcessEditing`).
5. **Hidden-run query + snap** (AC5): `MarkdownEscapes.isUnreachable` reads `hiddenKey` from STORAGE today. Under (a′)
   storage carries nothing, so the query reads the presenter's per-block result. Three call sites:
   `shouldChangeText` pair delete (`:2488`), `setSelectedRanges` caret + selection snap (`:2644`, `:2646`). Generalise
   from one backslash to runs (design §3.4; S1 measured 4 → 0 invisible stops).
6. **Headings + line reveal** (AC2, AC4 line half): heading font for the line, prefix hidden; the reveal driver issues
   an attributes-only storage edit over the old and new caret lines (S4b: that is what re-asks the delegate;
   `invalidateLayout(for:)` does not).
7. **Tests**: existing suites "Markdown escapes (AC3)", "Escape layer (AC4)", "Block intents as prose (AC7)",
   "Return and Backspace (AC5/AC6)" stay green (`ScriviInteropTests.swift:3736`, `:3956`, `:4163`, `:4248`). Some of
   them read `hiddenKey` from storage (15 references) and must move to the new query; that is an expected change,
   not a weakened test. New: AC1 storage-attribute scan + save round-trip; AC2 analyzer + dumas heading count; AC5
   x-duplicate-step corpus test (S1 method); reveal posts 0 `textDidChange`.
8. **AC8 measurement** on the 1.85 MB fixture (S4/S4b method), recorded here.
9. **Live pass** (user): headings visible on the dumas project; prefix appears on the caret's line; typing, undo,
   save unchanged. iOS / visionOS must still BUILD.

## ✅ Questions — RULED at activation 2026-10-05 (as recommended)

| # | Question | Ruling |
| - | -------- | -------------- |
| **Q1** | Heading sizes over the monospaced body font (EP-047 owns the typeface) | ✅ **RULED:** H1 22 pt, H2 18 pt, H3 16 pt, H4–H6 body size bold |
| **Q2** | How a revealed prefix looks | ✅ **RULED:** `tertiaryLabelColor`, body font (as spiked) |

⛔ **NOT in this Sprint:** bold/italic rendering and span reveal (E2-S2) · commands, lists, [T-0584] (E2-S3) ·
Find/Replace, [T-0585] (E2-S4) · Linux ([EP-048]).

## Acceptance Criteria

- [x] **AC1** storage stays plain — test `headingPresented` (storage font scan) ✅
- [x] **AC2** headings render — analyzer + presented-paragraph + LAID-OUT width tests ✅; dumas 1,172/1,172 ✅; live pass ✅
- [x] **AC4 (line half)** — `lineReveal`: prefix shows on the caret's line (Q2 attributes), re-laid by TextKit, 0 `textDidChange` ✅
- [x] **AC5** — `noInvisibleStops`: → and ← through escapes + a heading, 0 invisible stops; shift-→/← never stall ✅
- [x] **AC7** — analyzer: indented / quoted / listed `##` are not headings ✅; E1's indented-escape suite green ✅
- [x] **AC8** — measured on 1.85 MB with the REAL source files (below) ✅
- [x] **AC11** — `EscapeHidingStyler` removed; every E1 suite green under the presenter ✅
- [x] The three carried gaps (plan step 1) measured and recorded ✅
- [x] ✅ **Live pass (plan step 9) — PASSED (user, 2026-10-05)**

---

## Progress log

### 🔵 2026-10-05 — Sprint CREATED in Planning

✅ User: *"yes, mark T-0587 as verified.  Close SP-159 and create E2-S1"*. ✅ ID SP-161 issued by `next-id.py`. ✅ Scope
from the approved design §12 (E2-S1) and the ruled order Q-E2-3. ✅ Call sites read for plan steps 4–5 (`hiddenKey`,
`snapCaret`, `snapSelection`, `EscapeHidingStyler`).

### 🟡 2026-10-05 — Sprint ACTIVATED (user-approved); Q1 + Q2 RULED; [T-0588] allocated

✅ User: *"activate SP-161 and approve Q1 and Q2"* ✅ [T-0588] issued by `next-id.py`. ✅ Left `Sprint-backlog.md` at activation.

### 🟠 2026-10-05 — IMPLEMENTED (T-0588 Implemented - Not Verified); ⏳ live pass owed

✅ User: *"implement SP-161"*.

**Step 1 — the three carried gaps, measured (spike harness, route (a′)):**
| Gap | Result |
| --- | ------ |
| The one dumas `##` line of 1,172 not rendered (SP-159 S4) | ✅ **Real, not noise:** 4 dumas scene files have NO final newline, and the app's divider (`SceneDivider.string`: `￼` + `\n`) then sits on the scene's LAST line (`testr￼⏎## …`). A block that ends only at blank/divider LINES swallows the next scene's heading. ✅ Fixed in design: **a block ends at the divider CHARACTER** (`blockEndsAtDivider` test; dumas assembled 1,172/1,172) |
| Shift-selection / drag across hidden characters | ⚠️ 3 dead steps per 3 hidden backslashes with no snap. ⛔ **E1's `snapSelection` always moved a selection's end FORWARD**, so shift-← stalled at a hidden escape. ✅ Direction-aware now (`previousEnd`). Drag: hit-testing lands before/after a run consistently (SP-159 S4c); the reveal is not applied mid-drag |
| VoiceOver / spell-check | ⚠️ **VoiceOver gets the STORED text** (`accessibilityValue` = `Mr\. Smith… ## Heading`), escapes and prefixes included — true since E1. ✅ Spell-check: no false flags from escapes (`don\'t`, `well\-known` clean; a name is flagged with or without its backslash). → ⚠️ **Accessibility recommended as its own Issue** (not in EP-046's ACs) |

**What was built:**
- `Scrivi/Views/MarkdownBlocks.swift` (new; all 3 targets) — the block analyzer: ATX heading lines the parser reports, not inside code/quote/list/table (AC7); setext drawn as stored.
- `Scrivi/Views/ManuscriptPresenter.swift` (new; all 3 targets, macOS code) — `NSTextContentStorageDelegate` (same-length paragraphs: Q1 heading fonts, hidden prefix, E1's hidden escapes) + `NSTextStorageDelegate` (a character edit re-presents its whole block and the line above, by an `.editedAttributes` edit) + `reveal(for:in:)` (lines holding the selection's ends; Q2 attributes) + `hiddenTest(in:revealing:)` (what the snap reads).
- `ManuscriptEscapes.swift` — `hiddenKey` and the storage-reading snap removed; `isUnreachable` / `snapCaret` / `snapSelection` take an `isHidden` test, handle hidden RUNS, and `snapSelection` is direction-aware.
- `ManuscriptTextView.swift` — presenter installed (`makeNSView`); pair-delete and `setSelectedRanges` read the presenter; reveal after `super.setSelectedRanges` (not mid-drag); `EscapeHidingStyler` deleted.
- `project.pbxproj` — both files in ScriviApp, ScriviApp-iOS, ScriviApp-visionOS.

**Tests:** baseline (pre-change) **168/168 in 19 suites**. After: ✅ **175/175 in 20 suites** — new suite "Manuscript presenter (EP-046 E2-S1)" (6 tests), E1 snap tests moved to the `isHidden` API + 1 new (`hiddenRuns`). ✅ **Mutations bite:** block widening off → `editRepresentsBlock` red (⚠️ E1's query-only `backslashBreakAfterMerge` stayed GREEN — the reason the new tests read LAID-OUT widths); E1's forward-only selection snap restored → shift-← assertion red. ✅ `check-textkit2.sh` clean. ✅ iOS + visionOS BUILD SUCCEEDED.

**AC8 (REAL source files, 1.85 MB dumas, harness `scratchpad/ac8/`):**
| | Today (E1 at HEAD) | E2-S1 |
| - | - | - |
| rebuild + viewport layout | 6.36 ms | **3.45 ms** (faster: no eager whole-document scan) |
| keystroke @123,465 / 513,559 / 1,816,059 | 0.687 / 0.551 / 0.594 ms | 0.792 / 0.617 / 0.689 ms (**+0.07–0.10 ms**) |

⏳ **Live pass owed (plan step 9, the user's):** open dumas — every scene's `##` line is a heading (no `##` visible); put the caret on a heading line — `## ` appears dimmed in the body font, and hides again when the caret leaves; arrow through a heading and an escaped character — no stop where the caret does not move; type, undo, save — files unchanged in format.

### ✅ 2026-10-05 — Live pass PASSED; [T-0588] VERIFIED + archived; [I-0277] filed; Sprint CLOSED (user-approved)

✅ User: *"Yes, file the voice over issue as an issue. You instructed me to "type, undo, and save" but there is no
save. All steps in the Live pass passed. We can Verify T-0588. If available, you are also authroized to close SP-161."*
⚠️ **Correction (the user's):** the live-pass script said *"type, undo, save"*. ⛔ **Scrivi has no Save** — scenes
autosave (1 s debounce) and save immediately on a scene exit. The step that passed is "type and undo; the scene files
keep their format". ✅ [T-0588] → [`../../Tasks/Verified/Task-verified-0588.md`](../../Tasks/Verified/Task-verified-0588.md).
✅ [I-0277] (VoiceOver reads the stored text) → `Issue-backlog.md`, Not Assigned.

---

## Retrospective

**Completed:** ✅ EP-046 AC1, AC2, AC4 (line half), AC5, AC7, AC8, AC11, and the three carried gaps. Route (a′) is
in production. Storage is plain, headings render, the caret's line reveals its prefix, and E1's styler is retired.
The rebuild got FASTER (6.36 → 3.45 ms on 1.85 MB).
**Returned to Backlog:** none. **Filed:** [I-0277] (VoiceOver).
**What went well:** ✅ Measuring the carried gaps FIRST turned "one heading in 1,172 didn't render" into a real design
rule (a block ends at the divider CHARACTER) before any code assumed otherwise, and it found E1's shift-← stall.
✅ The new tests read what TextKit LAID OUT, and a mutation showed why: E1's query-only test of the same case stayed
green with the block widening removed.
**What to improve:** ⚠️ My live-pass script told the user to "save" in an app that has no Save (it autosaves). Write
live-pass steps from the app as it is, not from a generic editor. ⚠️ Two of the 7 new tests failed on their first run,
one from a presenter gap (block at the divider character) and one from the test's own wrong assumption (fixture text
carries no font). Both are fixed, but the second cost a run.
