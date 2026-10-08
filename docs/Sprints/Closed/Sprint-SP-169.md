---
sprint: SP-169
epic: EP-047
status: Closed
closed: 2026-10-08
activated: 2026-10-07
platform: Apple
created: 2026-10-07
---

# SP-169 — `[Apple]` [EP-047] **S3**: the first-line indent, and [I-0281]

**Status:** ✅ **CLOSED 2026-10-08 (user-approved):** *"close SP-169"* — activated 2026-10-07. All ACs met; T-0598 and [I-0281] VERIFIED and archived. (user: *"yes"* (to *"Shall I activate SP-169 and start?"*)). Created 2026-10-07; rulings P10 taken in planning.
**Tasks:** ✅ [T-0598] → [`../../Tasks/Verified/Task-verified-0598.md`](../../Tasks/Verified/Task-verified-0598.md) · **Issues:** ✅ [I-0281] → [`../../Issues/Verified/Issue-verified-0281-0290.md`](../../Issues/Verified/Issue-verified-0281-0290.md)
**Epic:** [EP-047] `[Apple]` Manuscript Typography & Preferences → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). Previous: [`Sprint-SP-168.md`](Sprint-SP-168.md).
**Authority:** EP-047 rulings **P3** (three modes) and **P10** (default Book convention · 1.5 em · the small gap · the book rule);
study [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §4D (a tab or 4 spaces
makes a `codeBlock`; `firstLineHeadIndent` costs ZERO characters). Issue: [I-0281] → [`../Issues/Issue-backlog.md`](../../Issues/Issue-backlog.md).
**Size:** M–L. ✅ Needs no rig: the live pass is on the Mac.

---

## ⚠️ What reading the code found (2026-10-07)

- ⛔ **The presenter does not see plain paragraphs.** `ManuscriptPresenter.textContentStorage(_:textParagraphWith:)` returns `nil` for a
  block with no markup (`:306`) — most of a novel — so TextKit draws it from STORAGE. ✅ **Decision (recommended, see Plan 2): the indent
  comes from the PRESENTER**, which then returns a paragraph for every body block. Storage stays plain, so the `.md` cannot change and an
  undo or rebuild cannot lose the indent — ✅ this RETIRES EP-047's TRAP (indent in storage, rebuilt at two sites) rather than managing
  it. ⚠️ Cost: the presenter now presents every paragraph → measured (Plan 7).
- ⚠️ **A Markdown paragraph is a BLOCK; TextKit's paragraph is a LINE.** A block with a soft break (`⏎` inside it) or an Option-Return
  hard break (`\` + `⏎`) is several TextKit paragraphs — only the block's FIRST line is indented.
- ⚠️ **Book convention needs the PREVIOUS block** (P10: indent only after body text). Changing a block (body ⇄ heading) changes the
  NEXT block's indent, so the presenter's edit widening (§3.5) must include the following block.
- ✅ **The paragraph-style builder exists** (`ManuscriptTypography.paragraphStyle`, SP-168) — the indent is one more input to it, so a
  list or heading line cannot drop it by building its own style.
- ✅ **A typography change already keeps the writer's place** (`applyTypography`, [I-0282]) — the indent settings join the
  typography value, so changing them re-presents through the same path.

## Goal

✅ **Scrivi sets prose like a book by default — first lines indented 1.5 em, paragraphs close together — with ZERO characters in the
file.** The writer can choose None or Every paragraph, and the amount, per project. ✅ And [I-0281]'s mis-placed emphasis is gone.

## EP-047 ACs this Sprint meets

| AC | Criterion (Epic wording, short) |
| -- | ------------------------------- |
| **AC6** | First-line indent: None · Every body paragraph · Book convention + amount; `firstLineHeadIndent`; zero characters in the file; headings and list items never indented; survives typing, Return, undo and rebuild |
| **AC8** | [I-0281] fixed: emphasis after an indented first line renders where its markers are; the L2 `knownDisagreements` entry flips and is removed |

## Plan

1. **Settings + the one source:** `paragraphIndent` (`none` · `every` · `book`; ABSENT = `book`, P10) and `indentEm` (ABSENT = 1.5) in
   `project-settings.json` via `ProjectPreferences` (written only once chosen, the SP-168 rule). Both join `ManuscriptTypography`, so a
   change goes through `applyTypography` (place kept). Project Settings ▸ Typography gains **Paragraph indent** (menu) and **Indent**
   (stepper, 0.5–4 em).
2. **The presenter indents** every BODY block's FIRST line: `firstLineHeadIndent = indentEm × size`, from the builder. ⛔ Never a
   heading, a list item, a chapter title; ⛔ never a continuation line. **Book:** only when the previous block in the SAME scene is
   body text — no indent at a scene's start or after a heading, list, quote or chapter title. **Every:** all body blocks.
3. **The small gap (P10):** with an indent on, the stored blank line between blocks draws at ~⅓ of a line (a paragraph style on the
   blank line, from the builder); the caret can still land there. None → the full blank line, as today.
4. **Edit widening:** an edit re-presents the FOLLOWING block too (its book-rule indent depends on this one).
5. **[I-0281]:** spike Apple's column convention for continuation lines after an indented first line (the CLI harness from SP-165),
   then correct `MarkdownBlocks`' `offset(line:column:)`; the L2 `knownDisagreements` entry must flip, and is removed.
6. **Tests** (interop, through the app's real paths — `feedback_test_through_the_real_dispatch`): each mode on a scene's first block,
   after a heading/list/quote, after body text, a soft-broken block (first line only); zero characters (`.md` bytes unchanged);
   survives typing, Return, undo, rebuild; the gap; settings round-trip and defaults; mutation-checked.
7. **Measure:** presenting every paragraph — open and keystroke on 1.8 MB (the SP-168 on-demand cost test, extended).
8. **Live pass** (user, Mac) — steps in the chat reply.

⛔ **NOT in this Sprint:** Markup Hints (S4) · Linux (EP-048 L10).

## ✅ Results (2026-10-07)

**Built:** `ManuscriptTypography` gains `indent` (`book` · `every` · `none`), `indentEm`, `firstLineIndent`, a `firstLineIndent:`
input to THE paragraph-style builder and `gapParagraphStyle` · the presenter draws the indent on a body block's FIRST line and the
small gap on stored blank lines (it now presents plain paragraphs whenever an indent is on); `isBody`, `previousBlock`,
`nextBlock` give the book rule (P10) · edit widening includes the following block · `ProjectPreferences.paragraphIndent` /
`indentEm` (written only once chosen) · Project Settings ▸ Typography: **Paragraph indent** + **Indent** (em) · **[I-0281]:**
`MarkdownBlocks` corrects continuation-line columns (`trueColumn`).

✅ **[I-0281] spike (Apple, 2026-10-07):** for a paragraph's continuation lines Apple reports the column AFTER stripping that line's
own leading whitespace, PLUS the first line's indent (`" She said⏎*no*"` → `no` at 3, true 2; `" x⏎  *c* d"` → 3, true 4).
True = reported − first-line indent + this line's indent. ✅ L2: AC3-raw 5/1,918 disagree (was 6 — I-0281's agrees now; the
five left are the accepted symbol / `~` / leftover-`*` classes); its `knownDisagreements` entry REMOVED; four I-0281 blocks
added to the structural set, which must agree.

| Check | Result |
| ----- | ------ |
| Interop (full) | ✅ 238/238 in 28 suites (+8 `ManuscriptIndentTests`, +1 rebuild test; L2 updated) |
| Mutations — indent (6) | ✅ 5/6 killed (book rule at a scene start, every line indented, no gap, a list item as body, default pinned on save). ⚠️ **M3 (no following-block widening) is EQUIVALENT in the harness:** TextKit 2 re-asks for paragraphs after an edit anyway (their ranges shift). Kept, commented — that is observed behaviour, not API |
| Mutation — [I-0281] | ✅ the correction undone → 3/50 structural blocks disagree (the fourth matched by coincidence) |
| Rebuild (`applyTypography`) | ✅ the presenter draws the indent after a rebuild; storage carries none |
| Cost (on-demand, 1.8 MB) | Literata 16 indent NONE 20.4 ms first layout · **Book (default) 22.9 ms** (+2.5 ms: every paragraph presented) · keystroke 0.8 ms and arrow 0.2 ms unchanged |
| iOS + visionOS builds · type-source guard · stub parity | ✅ · ✅ · ✅ (ctest: core unchanged since SP-168, 672/672) |

⚠️ **Found on the way:** TextKit 2 applies `firstLineHeadIndent` by moving the paragraph's FRAGMENT frame (x = 5 pt padding + 24 pt),
not the line's own bounds — the first test measurement read the line and saw no indent (probe, recorded in the test helper).

## Acceptance Criteria

- [x] AC6: the three modes + amount, from the presenter; book rule per P10; first line only; the small gap; zero characters; survives typing, Return, rebuild (undo: storage carries no indent, so it cannot lose one)
- [x] Settings: absent = Book / 1.5 em; written only once chosen; a change keeps the writer's place (through `applyTypography`)
- [x] AC8: [I-0281] fixed; L2 entry removed (and the L2 suite green)
- [x] Cost measured and recorded
- [x] Interop green (core unchanged); type-source guard clean
- [x] Live pass (user, Mac) — ✅ 2026-10-08, steps 1–6: *"1. passes. 2. passes. 3. passes. 4. passes. 5. passes. 6. passes."*
