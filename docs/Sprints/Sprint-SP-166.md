---
sprint: SP-166
epic: EP-048
status: Active
activated: 2026-10-07
platform: Linux
created: 2026-10-07
---

# SP-166 — `[Linux]` [EP-048] **S2**: the presenter: escapes hidden, headings and emphasis rendered, caret snap

**Status:** 🟡 **ACTIVE 2026-10-07** — ✅ **code + offscreen evidence done 2026-10-07; ⏳ BLOCKED on the rig for the live pass** (→ work switches to [EP-047]). — created 2026-10-07 (user: *"Then begin the S2 planning."*). ✅ Q1–Q2 ruled (*"Q1: Add the new criterion.  Q2: one Sprint."*).
**Tasks:** 🟡 [T-0595] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md)
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). Previous: [`Closed/Sprint-SP-165.md`](Closed/Sprint-SP-165.md).
**Authority:** [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §2.4 (W3), §3.5–§3.7 (Apple as built, the
shape to adopt), §5 (re-entry), §9 (Qt means). Apple reference: `ManuscriptPresenter.swift`, `MarkdownBlocks.swift`, `ManuscriptEscapes.swift`.
**Size:** L. ⚠️ **The live pass needs the Linux rig, which is unavailable (2026-10-07).** Code + offscreen smokes can be done without it;
at the live pass, work switches to [EP-047] (user, 2026-10-07).

---

## ⚠️ What reading the code found (2026-10-07)

- ✅ **One `QTextDocument` holds the whole manuscript** (`SceneDocument`), with protected chapter-heading and separator text between
  scenes. ⚠️ So a Markdown block must end at a scene boundary, the Qt form of Apple's §3.5 rule ("a block ends at the divider
  character"). Otherwise a scene's last line and the next scene's `##` merge into one block.
- ⚠️ **A Qt block is a LINE; a Markdown block is a RUN of lines.** `QSyntaxHighlighter::highlightBlock` sees one line, but its
  rendering depends on the other lines of its Markdown block, and an edit on one line can change another (`*a⏎b*`). The presenter
  must analyse the whole Markdown block and `rehighlightBlock` its sibling lines when the analysis changes. Apple solved the same
  shape by widening an edit to its block (§3.5, "the presenter is ALSO the storage delegate").
- ✅ **`ManuscriptEscapes::hiddenBackslashes` already exists on Linux** (EP-049 ported the read map along with the write half). L4
  is wiring, not porting.
- ✅ **`normalizeCaret` already snaps the caret out of protected boundary text** (T-0246). L5 extends it with stop runs that have
  a home side (§3.6).
- ⚠️ **Linux has no undo** (`setUndoRedoEnabled(false)`, the custom history is not on Linux). "Survives undo" (W3) is moot until it is.
- ⛔ **SCOPE GAP — EP-048's ACs do not include Apple's atomic markers or balanced edits** (EP-046 AC12, §3.6), nor copy without
  emphasis markers. ⚠️ Once markers are HIDDEN, a ⌫ next to one would delete an invisible `*` and silently change the formatting.
  → **Q1**.
- ✅ `escape_smoke` already builds `ManuscriptEditor` + `SceneDocument` offscreen (`QT_QPA_PLATFORM=offscreen`): the harness exists.

## Goal

✅ **A Linux writer sees the manuscript Apple shows:** no escape backslashes, headings at heading size with their `#` hidden,
bold and italic rendered with their markers hidden, markers revealed when the caret enters (line / span, Q-E2-1), and a caret
that never stops inside something it cannot see.

## EP-048 ACs this Sprint meets

| AC | Criterion (Epic wording) |
| -- | ------------------------ |
| **L3** | The Linux presenter: a `QSyntaxHighlighter` on `ManuscriptEditor`; the document's stored text and formats untouched; save bytes unchanged |
| **L4** | Escape backslashes hidden (Linux shows them today) with E1's rule; hard-break backslash per E1 AC6 |
| **L5** | Hidden-run caret snap (W3 measured the same invisible stops as Apple) |
| **L6** | Headings + bold/italic render; re-entry span/line (Q-E2-1) via `rehighlightBlock`; AC7 prose demotion |
| **L9** (part) | Markers are atomic: ⌫/⌦ never delete a hidden marker alone (balanced edits + copy → S3) |

## Plan

1. **`ScriviBridge::analyzeMarkdown(block)`**: the `scrivi_analyze_markdown` binding, with UTF-8 bytes converted to QString (UTF-16)
   offsets, the same conversion the L2 test uses (every unit of a scalar). Gap audit: Linux ✅.
2. **`ManuscriptPresenter` (Linux)**, a `QSyntaxHighlighter` on the editor's document. It finds the line's Markdown block (non-blank
   lines, bounded by scene edges from `SceneDocument`), gets the analysis (cached by block TEXT, Apple's §3.5 key), and applies to
   this line: hidden escapes + hidden markers + hidden heading prefixes (W3: 0.01 pt + transparent), heading size/weight, bold and
   italic. Edits re-highlight every line of the edited block.
3. **Reveal (Q-E2-1, as amended in §3.6):** the heading prefix shows with the caret on its line (either selection end); a span's
   markers show with the caret at its first or last character. `rehighlightBlock` on the old and new caret lines only; never
   mid-drag.
4. **Caret snap (L5):** stop runs with a home side (escape → before; opener and heading prefix → after; closer → before),
   direction-aware for selections (§3.5), folded into `normalizeCaret`.
5. **AC7:** whatever the analyzer demotes (code blocks, tables, nested headings, and now fences, [I-0280]) is drawn as stored.
6. **Lists** (whole-line, the analyzer already reports them): prefix visible and dimmed. ⚠️ The hanging indent in
   `QPlainTextDocumentLayout` was never measured (design §9). Spike it first; if it cannot, record the difference (T-0576 precedent).
7. **Fonts:** Apple's weights (§3.6 B: body bold = Bold, bold in a heading = Heavy) and heading sizes, matched by RESULT in Qt.
8. **Smokes (offscreen, Docker):** stored text and save bytes unchanged after highlighting (L3); hidden widths ≈ 0 (W3's 0.00 pt
   residue); caret stops across escapes, markers, prefixes; reveal on enter and leave; blocks bounded at scene edges; cost on dumas
   (open + a keystroke near the end; W3's regex baseline was 29 ms / 0.06 ms).
9. ⏳ **Live pass on the rig (user)**. ⛔ Blocked until the rig returns. Steps will go in the chat reply.

## ✅ Questions — Q1–Q2 RULED 2026-10-07

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | The scope gap: atomic markers, balanced edits and copy-without-markers are not EP-048 ACs | ✅ **RULED: add L9** — atomic markers in THIS Sprint; balanced edits + copy in **S3** |
| **Q2** | One Sprint for L3–L6, or split as Apple did (E2-S1 headings / E2-S2 emphasis)? | ✅ **RULED: one Sprint** |

## ✅ Results (2026-10-07) — code + offscreen evidence; ⏳ live pass blocked on the rig

**Built:** `MarkdownAnalysis.hpp` (Apple's `MarkdownBlocks.Analysis`, UTF-16) · `ScriviBridge::analyzeMarkdown` (static; UTF-8 → UTF-16,
every unit of a scalar) · `ManuscriptPresenter.{hpp,cpp}` (`QSyntaxHighlighter`; blocks bounded by scene text; analysis cached by
block text; hidden / dimmed / heading / bold / italic formats; reveal; stop runs; every line of an edited block re-highlighted) ·
`ManuscriptEditor`: caret + selection snap (Apple's `snapCaret` / `snapSelection`, direction-aware), reveal on cursor moves, never
mid-drag, atomic markers in `handleDeletion` · `EditorShell`: one presenter, DETACHED during each load and re-attached after, so a
load highlights once against a current scene map · `presenter_smoke` (+ `.sh`), all four targets that compile the editor updated.

| Check | Result |
| ----- | ------ |
| `presenter_smoke` (offscreen, real analyzer, uid 1000) | ✅ `presenter-ok` |
| Mutations (11) | ✅ **11 / 11 killed**. ⚠️ The first pass let 2 through, both TEST defects: "drawn hidden" passed when NOTHING was drawn (`drawnSize` returned 0 for no format); and scene edges are always `\n\n`, so the rule only bites at a CHAPTER HEADING — a title `*Draft*` would have rendered as Markdown. Both tests fixed |
| All Linux smokes (the rig loop, in Docker) | ✅ **27 / 27**. (`dumas_world_fixture` is a fixture generator — "NOT a test", its header says — counted by the loop because it lives in `tests/`; it runs clean with its arguments. Pre-existing.) |
| Full Linux app build (Release) | ✅ no errors |
| Cost — 1.8 MB, 10,471 DISTINCT blocks (Release, Docker on the Mac) | ✅ attach + first highlight **181–188 ms**; a keystroke near the end **1 ms**. ⚠️ The first measurement was INVALID (identical paragraphs → 3 analyzer calls, the cache measured) — fixed |

⛔ **Found on the way:** `~QSyntaxHighlighter` clears formats inside an edit block, which emits `contentsChange` into a still-connected
slot of the half-destroyed presenter → *"pure virtual method called"*. ✅ The presenter detaches in its own destructor.

### ⚠️ Differences from Apple, recorded (T-0576 precedent: match the RESULT, record the difference)
- ⛔ **No hanging indent for list items.** A `QSyntaxHighlighter` can set only CHARACTER formats; a hanging indent needs a
  `QTextBlockFormat`, which is a DOCUMENT edit (`contentsChange`, the scene map, dirty flags) — ⛔ against **L3** ("the document's
  stored formats untouched"). Prefixes ARE visible and dimmed. ⏳ For the live pass: is the difference acceptable?
- Heading sizes are Apple's RATIOS over the body size (22/18/16 over 13), not Apple's absolute points; bold-in-heading is
  `QFont::ExtraBold` (Apple: Heavy).
- No undo on Linux, so W3's "hiding survives undo" has nothing to survive yet.

### ⏳ Live pass (user, on the rig) — when it returns
1. Open dumas (or tintagael) — the text shows NO backslashes; `## ` lines are large headings with no `#`.
2. Put the caret on a heading line — the `## ` appears, dimmed; move off — it hides again.
3. Arrow through `Mr. Smith` (stored `Mr\. Smith`) — no invisible stop at the period.
4. Find bold/italic text — rendered, no `**`; caret at its first or last letter shows the markers, dimmed; in the middle hides them.
5. Caret right after an opening `**` (start of a bold word), ⌫ — deletes the character before it, the bold stays; at a heading's
   start, ⌫ — the `## ` goes, the line becomes body text.
6. Type, click, drag-select across formatting — nothing jumps or flickers mid-drag; Scrivi autosaves; the scene file on disk has
   the same Markdown as before plus your edit.

## Acceptance Criteria

- [ ] L3: presenter attached; stored text, formats and save bytes unchanged (smoke)
- [ ] L4: escape and hard-break backslashes hidden per E1's rule (smoke over the shared corpus)
- [ ] L5: no caret stop inside a hidden run; selection snapping direction-aware (smoke)
- [ ] L6: headings and bold/italic render; line and span reveal; AC7 demotion; blocks end at scene edges (smoke)
- [ ] L9 (part): atomic markers — ⌫ / ⌦ never delete a hidden marker alone
- [ ] Lists: dimmed prefix; hanging indent measured, or the difference recorded
- [ ] Cost on dumas recorded; ctest + Linux smokes green in Docker
- [ ] ⏳ Live pass on the rig (user)
