# Scrivi — Manuscript Renderer **E2: Inline Rendering (WYSIWYG)** — Design v0.1

**Status:** ✅ **APPROVED 2026-10-05** (user: *"I have reviewed the design document and approve it. I also approve your recommendations for the 8 rulings."*).
✅ **All eight rulings Q-E2-1 … Q-E2-8 TAKEN as recommended** — see §0A.
**Date:** 2026-10-05
**Sprint / Task:** [SP-159] / [T-0587] · **Epics:** [EP-046] `[Apple]` (designs) · [EP-048] `[Linux]` (scoped from this)
**Platform:** `[Cross]`: the display design for Apple **and** Linux.
**Authority:** [`Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md)
(§3A.0, §4.3 Model B, §4A, §10) · [`Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](Scrivi_Manuscript_Renderer_E1_Design_v0_1.md)
(what E1 built) · [`Epics/Closed/Epic-EP-045.md`](Epics/Closed/Epic-EP-045.md).

⚠️ **Every number here was measured in SP-159** (§10). The spike code is throwaway and NOT committed (EP-045 precedent):
`scratchpad/sp159/h/*.swift` (Apple, macOS 27.2, Swift 6.4, TextKit 2) and `scratchpad/sp159/qt/w3.cpp` (Ubuntu 24.04,
Qt 6.4.2, in Docker — the Linux image's own base and Qt).

---

## 0. The recommendation in one paragraph

✅ **Render through a PRESENTATION layer, not through storage.** On Apple, an `NSTextContentStorageDelegate` hands
TextKit 2 each paragraph with **the same characters and styled attributes** (bold, italic, heading size, markers and
escape backslashes at 0.01 pt + clear). On Linux, a `QSyntaxHighlighter` does exactly the same thing to
`QPlainTextEdit`. **Storage stays plain Markdown on both platforms**, so the save path, the undo path, EP-019 history and
ScriviCore never see a styling attribute. The **stock parser decides** what is styled (`AttributedString(markdown:)`
on Apple, md4c on Linux). It is never the editing surface, because **no API on either platform round-trips Markdown**
(W2, W3). ⚠️ This **revises how R3 = (c) is implemented, not what it shows**. E1's escape hiding moves onto the same
layer, and its storage delegate and undo-path re-application are retired.

⚠️ **Why this is new:** E1's route (a) used the same delegate and was rejected, because it **removed** the backslash.
That changed the paragraph's length and broke the caret (`moveRight` 0 → 8). ✅ **Apple's contract for that delegate
is explicit:** *"The attributed string for a custom text paragraph must have `range.length`"*
([`textContentStorage(_:textParagraphWith:)`](https://developer.apple.com/documentation/appkit/nstextcontentstoragedelegate/textcontentstorage(_:textparagraphwith:)),
fetched via the `.json` route; the same sentence is in `NSTextContentManager.h:278-284`). Route (a) broke that rule.
✅ **Route (a′) keeps every character and changes only attributes, and it measured clean** (§2.3).

---

## 0A. ✅ Rulings — taken 2026-10-05 (user approved every recommendation)

| # | Ruling |
| - | ------ |
| **Q-E2-1** | Re-entry: **span** for inline markers; **line** for whole-line prefixes (§5) |
| **Q-E2-2** | Commands: **⌘B** (`**`), **⌘I** (`*`, never `_`), **Heading 1–3** (⌥⌘1–3), **Body** (⌥⌘0), **bulleted** and **numbered** lists; **Return continues a list**, Return on an empty item ends it; the flank rule of §6.1 (§6.2) |
| **Q-E2-3** | Sprint order: **E2-S1 presenter + headings → E2-S2 bold/italic → E2-S3 commands + [T-0584] → E2-S4 Find/Replace + [T-0585]** (§12) |
| **Q-E2-4** = [T-0584] | Option-Return: **(b) a hard break**, `\` + `\n` (§6.3) |
| **Q-E2-5** = [T-0585] | Find/Replace: **match the PRESENTED text**; replacements written through the escape layer (§7) |
| **Q-E2-6** | Architecture: **(iii) route (a′)**, a presentation layer. ⚠️ **Revises R3 = (c)'s MECHANISM** (attributes move from storage to the presented paragraph); what the writer sees is unchanged (§2) |
| **Q-E2-7** | Malformed markup: **render as the parser reads it** (an unclosed `**` shows literally) (§8) |
| **Q-E2-8** | Linux: **`QSyntaxHighlighter`** presenter; parser **(L-b)**: md4c inside ScriviCore, tied to Apple's parser by an agreement test (§9) |

---

## 1. What is settled (not re-opened)

| Settled | Where |
| ------- | ----- |
| **Model B:** formatting renders; markers hide unless the caret is in them | Study §4.3 |
| **Typed markup is ALWAYS escaped**; formatting is authored only by command | Study §3A.0, E1 AC4 |
| **Existing markup in files is intended** and renders (dumas `##` headings) | E1 R2 |
| **Whole-line markers before inline ones** | Study §4.3 |
| **Unexposed block intents (code block, quote, table) draw as prose** | EP-045 AC7 → EP-046 |
| **The scene-break command needs `scrivi_split_scene`** — out of scope | E1 §10 |
| **Parse with `.full`** (AC8) — the parser decides attributes; its string is never displayed | E1 §4.5 |

---

## 2. ⚠️ Q-E2-6 — the rendering ARCHITECTURE (W1, W2, W3)

### 2.1 W2 — is there an editable Markdown round-trip in Apple's frameworks? ⛔ **No.**

Checked 2026-10-05 against the installed **macOS 27.2 SDK** (`Xcode-beta`) and Apple's docs (`.json` route):

| API | What it does | Round-trip? |
| --- | ------------ | ----------- |
| `AttributedString(markdown:options:)` (macOS 12+) | **Parses.** Options: `interpretedSyntax`, `appliesSourcePositionAttributes` (macOS 13+) | ⛔ one-way |
| `Foundation.swiftinterface` (27.2 SDK) | every `markdown` symbol is `MarkdownParsingOptions`, `MarkdownSourcePosition`, `MarkdownDecodableAttributedStringKey`, `decodeMarkdown` | ⛔ **no encoder exists** |
| SwiftUI `TextEditor(text: Binding<AttributedString>, selection:)` (26.0+) | rich-text editing of an **`AttributedString`**, with `AttributedTextFormattingDefinition` constraints | ⛔ edits attributes, not Markdown, and has no Markdown export |
| AppKit `NSAttributedString` document types | no Markdown type in `AppKit.framework/Headers` | ⛔ |
| `NSTextList.includesTextListMarkers` (**26.0+**, default `false`) | TextKit 2 draws list markers that are **not** in storage | — (used in §4.2) |

✅ **So the user's model — *"a stock Markdown renderer, with escaping so what is typed is not rendered as Markdown"* —
is right about the escaping, and right that a stock component should decide the rendering.** ⛔ But no stock component
can be the **editor**. Any of them would rewrite the file on save, which R2/AC9 forbid. Qt's `toMarkdown()` shows how
badly that goes (§2.4).

### 2.2 The three options

| | **(i) Storage-attribute styler** (R3 = (c), generalised) | **(ii) Stock render for inactive paragraphs, source for the active one** | **(iii) ✅ Presentation layer — route (a′)** |
| - | - | - | - |
| How | the text storage's delegate writes fonts + 0.01 pt markers **into storage** after every edit | non-active paragraphs show the parser's OUTPUT (markers gone) | the content-storage delegate returns each paragraph with the **same characters**, styled |
| Storage = file? | ✅ characters yes; ⚠️ storage carries styling attributes | ⛔ **no** — the rendered string is shorter than the source | ✅ yes, plain |
| Undo path (`applySceneChange` sets uniform attributes) | ⚠️ must re-style after (works — S1: identical after replace) | ⛔ | ✅ **nothing to re-apply** (S4c) |
| Rebuild cost (1.85 MB dumas) | ⛔ **+201 ms as stored, +682 ms with emphasis** (E1 today: 5 ms) | — | ✅ **0.45 ms + 6.8 ms viewport layout; 143 paragraphs styled** |
| Per keystroke | ✅ ≤ 0.32 ms | — | ✅ +0.2–0.3 ms over no-delegate (0.86 vs 0.53–0.66 ms incl. viewport relayout) |
| Caret | snap over hidden runs (S1) | ⛔ offsets disagree with storage (route (a)'s 0 → 8 jump) | ✅ steps one source offset (S4b); same snap as (i) |
| Custom code | styler + snap | ⛔ a custom `NSTextContentManager` that maps locations itself | delegate + analyzer + snap |
| Apple contract | — | ⛔ violates *"must have `range.length`"* | ✅ honours it |

⚠️ **(ii)'s VISIBLE behaviour is still on offer.** "Styled everywhere except where I am editing" is (iii) with
**block-granularity re-entry** (§5). The two differ only in mechanism, and (ii)'s mechanism cannot be built on
TextKit 2 without replacing the content manager.

✅ **RECOMMENDATION: (iii).** ⚠️ It **revises R3 = (c)'s mechanism** (R3 was ruled for "storage attributes on the
backslash"). The backslash still renders at 0.01 pt + clear, and the snap still keeps the caret out of it. What
changes is that the attributes now live in the presented paragraph, not in storage. ⛔ **That is a change to a
ruled mechanism, so it is the user's to rule (Q-E2-6).**

### 2.3 Route (a′) — what was measured (S4b, S4c)

| Probe | Result |
| ----- | ------ |
| TextKit 2 kept (`textLayoutManager != nil`) | ✅ yes |
| `a **bold** b \*x` width | ✅ **88.42 pt**, identical to the storage route and to the marker-free target |
| `moveRight` from 0 | ✅ `[0,1,2,…,17]`, one source offset per step (route (a) jumped 0 → 8) |
| Click hit-testing at the hidden-run boundary | ✅ left of it → offset 2 (before the hidden `**`); right of it → 4 (before `b`) |
| Typing inside a styled span | ✅ the paragraph is re-requested (2 delegate calls), the width updates, storage font stays plain |
| Undo-path whole replace with plain attributes | ✅ width back to 64.31 pt; nothing to re-apply |
| Reveal: `invalidateLayout(for:)` | ⛔ does **not** re-ask the delegate |
| Reveal: an **attributes-only** storage edit (`edited(.editedAttributes, …)`) | ✅ re-asks the delegate (width 88.42 → 120.55: markers shown) |
| Attributes-only edits: `textDidChange` posts / undo registered / `SceneBoundaryTable` | ✅ 0 / no / ignored (both observers guard `.editedCharacters`) |

### 2.4 W3 — Qt 6.4.2's Markdown engine (Linux)

| Probe | Result |
| ----- | ------ |
| `QTextDocument::setMarkdown()` → `toMarkdown()` on 1,185 dumas files | ⛔ **0 byte-identical**; 5 equal after trimming; ⛔ **1,179 re-wrapped at 80 columns**; `##` kept 1,168/1,168 |
| Escaped text (`Mr\. Smith\, … \#5`) → `toMarkdown()` | ⛔ **writes the escapes out UNESCAPED** (`1 * 2 * 3 and _x_ #5`). The next open would turn the writer's literals into live markup. **Data corruption, not just reformatting.** |
| md4c **reading** escapes (`toPlainText()`) | ✅ presents exactly what was typed: 1,199/1,200 escaped dumas paragraphs (the 1 is a soft line break shown as a space — correct CommonMark, same as Apple) |
| `QSyntaxHighlighter` on `QPlainTextEdit` (today's editor): hide `**` with 0.01 pt + transparent | ✅ **194.45 pt = target 194.45 pt** (zero residue) |
| Heading line with a 20 pt highlighter format | ✅ line height 32 vs 20: `QPlainTextEdit` honours per-block size, so **no switch to `QTextEdit` is needed** |
| The document's **stored** format at a hidden marker | ✅ untouched (highlighter formats are presentation-only — the same property as route (a′)) |
| Insert + undo | ✅ hiding intact |
| Caret x per position across a hidden `**` | ⚠️ 8, 9, 10 all at x = 85.88: **the same invisible stops as Apple**, so Linux needs the same snap |
| 1.85 MB `setPlainText` + highlighter attach + show / keystroke near the end | 29 ms / 0.06 ms (regex highlighter — a real parse will cost more; not measured) |

✅ **So `setMarkdown`/`toMarkdown` are ruled out as the editing model** (W3's warning confirmed). ✅ The Qt shape that
matches (iii) **already exists in Qt** as `QSyntaxHighlighter`. That honours `feedback_linux_adopts_apple_shape`
by the same mechanism class rather than by imitation.

⚠️ **Linux still needs a parser that reports SOURCE positions.** Qt's md4c is private (`QTextMarkdownImporter`), and
its public API returns a document without positions. → Q-E2-8.

---

## 3. The mechanism (Apple): `ManuscriptPresenter`

### 3.1 Parts

1. **Block analyzer** (pure, unit-testable): one **block** (a maximal run of non-blank lines, never crossing a
   divider line) → `kind` (paragraph / heading(n) / list item / demoted code / quote / table), `markers`, `bold`,
   `italic`, `prefixes`, `spans`. Built from `AttributedString(markdown:, .full, appliesSourcePositionAttributes)`:
   - Source positions cover **content only, never delimiters** (measured: `**emphasized**` → `1:11-1:20`). ✅ So
     **markers = source characters no run covers**, minus whitespace and newlines (soft breaks have no position).
   - Columns are **UTF-8 bytes** (`NSAttributedString.h:405`); convert per line, as Q7(b) did. `endColumn` is inclusive.
   - Whole-line prefixes (`## `, `- `, `1. `) are taken from the **intent the parser reported**, then matched on the line.
     A line that merely starts with `**` is not a prefix.
2. **Paragraph presenter**: the `NSTextContentStorageDelegate`. For a requested paragraph range, it styles the
   enclosing block (cached by block range + text) and returns the paragraph's slice, **same length**.
3. **Hidden-run query** for the caret: replaces `MarkdownEscapes.isUnreachable`'s storage-attribute read
   (`hiddenKey`). It reads the presenter's per-block result instead, because storage no longer carries the attribute.
4. **Reveal driver**: on a caret move, it issues an attributes-only edit over the old and new reveal regions (§5).

### 3.2 What the presenter applies

| Element | Attributes (presented paragraph only) |
| ------- | ------------------------------------- |
| Marker / escape backslash / hidden prefix | `.font` 0.01 pt + `.foregroundColor` clear (measured residue ≤ 0.02 pt) |
| Revealed marker | `.foregroundColor` `tertiaryLabelColor`, body font |
| Bold / italic / both | `NSFontManager` trait conversion of the body (or heading) font |
| Heading `n` | the heading font for the whole line (spike: 22/18/16 pt bold for 1/2/3); its prefix hidden |
| List item | hanging indent (`headIndent` = prefix width); prefix shown dimmed (see §4.2) |
| Code block / quote / table (AC7) | **prose**: a code block is re-analysed with its indentation stripped (inline still renders); a quote keeps its `>` **visible** and its inline renders; a table renders exactly as stored |

### 3.3 Why it beats E1's storage styler at rebuild

`rebuildStorage` costs **~0.0 s** on TextKit 2 today (SP-133 T-0529) because layout is lazy. Eager styling of
1.85 MB costs **201–682 ms** (S4). That lands on **every open and every structural op** (scene create, merge, chapter
create). The presenter is asked only for paragraphs that are **laid out** (143 at open).

### 3.4 E1 migration (part of the first implementation Sprint)

- `EscapeHidingStyler` (storage delegate) and its re-application on the undo path are **retired**. The presenter hides
  escape backslashes with E1's **unchanged** `MarkdownEscapes.map` / `hiddenBackslashes`.
- `MarkdownEscapes.snapCaret` / `snapSelection` generalise from "one hidden backslash" to **hidden runs** (S1):
  a forward step from just before a run lands after the first visible character past it; anything else lands before
  the run. **Measured:** `a **bold** b \*x` goes from 4 invisible extra stops (E1's snap) to **0**, in both directions.
- ⚠️ The cross-pair delete in `shouldChangeText` (`ManuscriptTextView.swift:2488`) reads the same query.

### 3.5 ✅ AS BUILT — E2-S1 ([SP-161], 2026-10-05); supersedes §3.1–§3.4's detail where they differ

- ⚠️ **A block ends at the divider CHARACTER, not at a divider line.** A scene file with no final newline (4 in dumas)
  puts the divider (`￼` + `\n`) on its last line; ending blocks only at lines merged the next scene's `##` into it.
- ✅ **The presenter is ALSO the text storage's delegate.** TextKit 2 re-asks only for the EDITED paragraphs, so a
  character edit is widened to its whole block (+ the line above, as E1's styler did) with an `.editedAttributes`
  edit. Without it, a ⌫ merge left a hard-break backslash visible on the line above (a mutation proved it).
- ✅ **The line reveal covers the lines of BOTH selection ends**, so starting a selection on a heading does not
  re-hide (and reflow) its prefix under the pointer; ⛔ never applied mid-drag (`stillSelecting`).
- ✅ **`snapSelection` is direction-aware**: a shrinking selection drops a hidden run. E1 always pushed the end
  forward, so shift-← stalled at a hidden escape (measured, SP-161 step 1).
- ✅ The analyzer cache is keyed by block TEXT (what it depends on) — no invalidation needed.
- ⚠️ **Found, not fixed: VoiceOver reads the STORED text** (backslashes, `##`) — true since E1; not an EP-046 AC.

### 3.6 ✅ AS BUILT — E2-S2 ([SP-162], 2026-10-05)

- ✅ **Markers are STOP RUNS with a home side**, independent of the reveal: escape → before; opener and heading prefix →
  AFTER (Q1: the hint shows to the caret's left); closer → before. One arrow step from home passes one visible
  character. ⚠️ This replaced E2-S1's "a revealed prefix is a caret stop".
- ✅ **Markers are atomic.** ⌫ after an opener deletes the character before it; ⌫ at a heading's start removes the whole
  prefix; ⌦ before a closer deletes the character after it.
- ✅ **Balanced edits (AC12, Q3)** — `MarkdownEmphasis`: tokens (units minus markers, with style) → edit → fewest
  markers. Whitespace takes the style both neighbours share; a newline carries none; a span never opens on whitespace;
  the longer-lasting style opens outside. ⚠️ CommonMark cannot open a span on punctuation glued to a preceding letter
  (or close one before a following letter): that punctuation mark loses its style. ✅ Checked by the parser in context
  before it is applied; on failure the stretch is written without emphasis (logged). Measured 5,999/6,000 realistic edits.
- ⚠️ **Not balanced yet:** cut/copy across a SCENE boundary (ScriviCore structured fragments).
- ✅ **Live-pass amendments (user, 2026-10-05):** (A) a span's markers show only with the caret at its FIRST or LAST
  character (it was: anywhere inside) — refines §5's span reveal; (B) bold is a real weight step — `NSFontManager`'s bold
  trait gives this face SEMIBOLD (0.30, measured); body bold is now Bold (0.40), bold in a heading Heavy (0.56).
  ✅ Decisions confirmed: ⌫ at a heading's start removes the `## `; copy to other apps drops `## `; edge punctuation glued
  to a letter loses its style.

### 3.7 ✅ AS BUILT — E2-S3 ([SP-163], 2026-10-05)

- ✅ **Commands are STYLE edits on the tokens** — the same rewrite core as E2-S2's balanced edits, with one difference: an edit
  that cannot be balanced is written without emphasis; a COMMAND that cannot be is refused (it must never drop formatting).
- ✅ **The normaliser needs CONTEXT** (one token each side of the rewritten stretch) and its passes repeat until stable.
- ✅ **Lists:** prefix visible + dimmed, hanging indent (§4.2 v1); a stop run like a heading prefix; numbering kept sequential.
  ⚠️ An EMPTY item gives the parser no run — recognised by the analyzer directly.
- ✅ **Q1's pending pair** lives in the text view's storage only (no `didChangeText`): autosave would otherwise write `****`.
- ✅ **Option-Return** stores `\` + `\n` on Apple AND Linux; a single newline may sit inside a span.
- ✅ **[T-0591] Apple-side:** each scene part of a cross-scene edit / copy / paste goes through the same balancing.

### 3.8 ✅ AS BUILT — E2-S4 ([SP-164], 2026-10-06) — supersedes §7's "not spiked"

- ⛔ There was NO in-manuscript Find before; ✅ AppKit's find bar (`NSTextFinder`) over a presented-text client — spike-proven on
  1.85 MB (escape, hidden marker and heading-prefix matches all map back to storage). Presented text: 166–178 ms to build, rebuilt
  only after an edit.
- ⚠️ Incremental search reads the client on a BACKGROUND queue: the client reads an immutable snapshot.
- ✅ Replace = typed text (escaped, balanced, the match's first character's style); Replace All = one grouped history step.
- ✅ The Navigator's search jump uses the same presented matching.
- ✅ **Live-pass fixes ([SP-164], 2026-10-06):** Replace All is applied in ONE editing pass (`applyReplacements`: each
  replacement escaped, escape-snapped and balanced as a single Replace) — 1,173 replacements on dumas: apply 83 ms, history
  220 ms (was ~1 minute through the typing path). The client returns **true** from `shouldReplaceCharacters` and applies the
  batch once, so the find bar reports its count. A multi-scene undo/redo places the caret ONCE. The Navigator's FILTER matches
  presented text too (`MarkdownEmphasis.searchable`), and the jump lands the caret ON the match (approved by the user).

---

## 4. Element order (Q-E2-3) and per-element notes

### 4.1 Headings (whole-line) — first

✅ Measured: `## The claim stated plainly` hidden prefix + 18 pt bold = **267.06 pt vs target 267.05**, line height 21 = target.
✅ On dumas: **1,171 of 1,172** `## ` lines render as headings (one heading line is not parsed as a heading; not traced).

### 4.2 Lists (whole-line) — a measured complication

⛔ **TextKit 2 draws an `NSTextList` bullet only from the paragraph's FIRST character's attributes.** With no prefix in
storage the bullet draws. With the stored `- ` hidden (0.01 pt, clear **or** text colour) the bullet is **invisible**,
measured both ways (`lp-*.png`). So hiding `- ` and letting TextKit draw `•` does not work.

✅ **Recommendation for v1:** lists keep their prefix **visible but dimmed** (`- `, `1. `), with a hanging indent so
wrapped lines align under the text. That is honest Markdown and adds no mechanism. A drawn `•` would need an
`NSTextLayoutFragment` subclass; that is deferred until the user asks for it.

### 4.3 Bold and italic (inline) — second

✅ Measured: `This is **emphasized**.` hidden = **152.71 pt vs target 152.69** (raw 184.83).
⚠️ Rendering attributes cannot carry this, and **they cannot even carry the font**: a bold `.font` set with
`setRenderingAttributes` **neither re-lays out nor draws bold** (S3, mono and proportional; `s3-render-*.png`). A
heading-size rendering font left the line at 16 pt. ✅ Fonts must be in the presented paragraph (iii) or in storage (i).

---

## 5. ⚠️ Q-E2-1 — re-entry granularity (S2)

The reveal is an attributes-only edit, so it is cheap. **The reflow is the cost the writer feels.**
Paragraph from the spike (`s2-*.png`), caret entering `**already decided**`:

| Policy | Restyle | Line fragments | Caret x | A word later on the line |
| ------ | ------- | -------------- | ------- | ------------------------ |
| **block** (whole paragraph's markers appear) | 0.17 ms (2 KB block with 40 spans: 1.32 ms) | ⚠️ **11 → 12** (the paragraph re-wraps) | 128.59 → 144.65 | 241.10 → 273.23 |
| **line** | 0.19 ms | 11 → 12 | same | same |
| **span** (only the span the caret touches) | 0.20 ms; span → span 0.29 ms | ✅ **11 → 11** | 128.59 → 144.65 | 241.10 → 273.23 |

⚠️ **Every policy moves the caret** (by the revealed markers' width: 16 pt for `**`). Only span reveal leaves the
rest of the paragraph where it was. **Block reveal re-wraps it:** words visibly change lines whenever the caret
crosses into a paragraph with formatting.

⚠️ **Why re-entry is not optional:** with no reveal, the boundary after a hidden closing `**` is unreachable. A writer
can never place the caret **between `bold` and the `.` after it** to type un-bold text there (S1 §5: the snap sends
it inside the span). Re-entry is what makes both sides of a marker addressable.

✅ **RECOMMENDATION: span for inline markers; line for whole-line prefixes** (`## ` appears when the caret is on the
heading line). This is Obsidian's Live Preview model. ⛔ Rejected: block, because of the re-wrap.

---

## 6. ⚠️ Q-E2-2 — the commands (S5)

### 6.1 Measured rules

Corpus: dumas prose paragraphs **escaped as if typed** (the case commands meet in practice), 2,000 random selections
per row, success = Apple's parser reports emphasis on **exactly** the selected presented text.

| Delimiter | Word-aligned | Arbitrary | Arbitrary, snapped to word edges |
| --------- | ------------ | --------- | -------------------------------- |
| `**` (bold) | ✅ 2000/2000 | ⚠️ 1967/2000 | ✅ 2000/2000 |
| `*` (italic) | ✅ 2000/2000 | ⚠️ 1970/2000 | ✅ 2000/2000 |
| `_` (italic) | ✅ 2000/2000 | ⛔ **286/2000** | ✅ 2000/2000 |

- ⛔ **Italic writes `*`, never `_`.** `_` does not open or close inside a word (`wheth_er Dumas_ inten` → nothing).
- ⚠️ **Every arbitrary-selection failure is CommonMark flanking.** For example, `doing**\. The sword**`: an opener
  preceded by a letter and followed by punctuation (the escaped `.`) does not open. ✅ **Rule:** trim whitespace from
  both ends, then extend an edge outward to the word boundary **only if** it would not flank (measured equivalent:
  2000/2000).
- ✅ **A selection across paragraphs** wraps **each paragraph** separately, in **one** replacement
  (`First **paragraph here.**⏎⏎**Second o**ne there.`). ⛔ Never across a scene divider — a cross-scene selection is
  refused or split by scene, as `deleteAcrossScenes` already does (not spiked).
- ✅ **One command = one edit:** `shouldChangeText` → one `replaceCharacters` → `didChangeText` posted
  **1** `textDidChange`. That is one EP-019 history event. Written with `insertVerbatim` (unescaped), per E1 §5.2.
- ✅ **Toggle-off** re-writes the enclosing node. Un-bolding `beta` in `**alpha beta gamma**` gives
  `**alpha** beta **gamma**` (parser: strong = `alpha`, `gamma`). Toggling a whole node removes its delimiters.
  Nesting survives (`*it **both** it*` → un-bold → `*it both it*`, emphasis kept).

### 6.2 Recommended surface (for ruling)

| Command | Writes | Key |
| ------- | ------ | --- |
| Bold | `**…**` | ⌘B |
| Italic | `*…*` | ⌘I |
| Heading 1 / 2 / 3 | `# ` / `## ` / `### ` line prefix (replaces an existing one) | ⌥⌘1 / ⌥⌘2 / ⌥⌘3 |
| Body (remove heading or list) | removes the prefix | ⌥⌘0 |
| Bulleted list | `- ` prefix on each selected paragraph | ⌥⌘L (proposed) |
| Numbered list | `1. `, `2. `, … | ⌥⌘N (proposed) |

⚠️ **Open within Q-E2-2:** (a) whether Return in a list item continues the list (writes `\n\n- `) and Return on an empty
item ends it; (b) whether `#` (level 1) is offered, given Q8 (the chapter title is metadata, rendered by the view).
Recommendation: offer levels 1–3 (`#` in a scene is an ordinary heading per Q8), and yes to (a).

### 6.3 Q-E2-4 = [T-0584] — Option-Return

Recommendation (as filed): **(b) a deliberate hard break**, storing `\` + `\n`. E1's map already hides that backslash
(when a non-blank line follows), and AC6 already writes the same form. ⚠️ Under rendering, a bare single `\n` would
display as a **space**, so leaving it (c) loses the writer's line break.

---

## 7. Q-E2-5 = [T-0585] — Find / Replace

✅ **Recommendation: match the PRESENTED text, and write replacements through the escape layer.**
- Under (iii) the gap widens: `**bold**` presents `bold`. A search for `bold here` must match across a hidden `**`.
- ✅ The seam exists: `MarkdownEscapes.map` gives presented ↔ source offsets per fragment; the analyzer adds hidden markers.
  Search runs on the presented text of each block. A match maps back to a source range, snapped so it never splits
  an escape pair or a marker run.
- Replace: the replacement is **escaped** (it is typed text). ⚠️ A replacement that would cut through a marker
  (a match spanning `bo**ld` boundaries) replaces the presented characters and keeps the markers in place.
- ⚠️ **NOT spiked:** whether AppKit's find bar (`NSTextFinder` with a custom `NSTextFinderClient`) can be pointed at
  presented text, or whether Scrivi needs its own find UI. That is the first task of the Sprint that carries T-0585.

---

## 8. Q-E2-7 — malformed existing markup

✅ **Recommendation: render exactly as the parser reads it.** Measured: `**unclosed bold` parses as **literal text**.
The `**` is covered content, not a marker, so it shows as typed and nothing is hidden. That is the same rule as
everywhere else (the parser decides), with no second interpretation to drift from it.

---

## 9. ⚠️ Q-E2-8 — Linux: same result by Qt means

| Apple | Linux (recommended) | Measured? |
| ----- | ------------------- | --------- |
| `NSTextContentStorageDelegate` presenter | `QSyntaxHighlighter` on `ManuscriptEditor` (`QPlainTextEdit`) | ✅ W3 (hiding, heading height, undo) |
| `AttributedString(markdown:)` with source positions | ⚠️ **a parser with source offsets** — options below | ⛔ not chosen |
| Reveal = attributes-only storage edit | `rehighlightBlock()` on the old and new caret blocks | ⚠️ API exists; not timed |
| Hidden-run caret snap | the same rule, applied in a `cursorPositionChanged` handler | ⚠️ needed (W3: x-duplicate stops); not built |
| `NSTextList` bullet: invisible (§4.2) | prefix visible + dimmed, hanging indent | ⚠️ `QPlainTextDocumentLayout` indent not measured |

**Parser options for Linux (the decision inside Q-E2-8):**
- **(L-a) md4c, used directly by the Linux app** (Ubuntu 24.04 ships `libmd4c-dev` 0.4.8 — checked; MIT). Its text callbacks point into the
  input buffer, so covered/uncovered works the same way as on Apple. ⚠️ Escaped-character callbacks not yet checked.
- **(L-b) md4c inside ScriviCore** behind an internal C++ API (Linux calls C++ directly). Apple keeps
  `AttributedString` per Q7(b). An **interop agreement test** (core analyzer vs Apple's parser over the AC3 corpus)
  guards against the two parsers drifting.
- (L-c) one core analyzer for **both** platforms. ⛔ It would re-open Q7(b) (Apple's parser won on correctness), so it
  is not recommended.

✅ **Recommendation: (L-b).** One Linux parser in the shared core, with an agreement test tying it to Apple's. That
matches the standing rule that parallel lists must not be allowed to drift silently.

---

## 10. Spike numbers (AC-P1)

| Spike | Measurement | Result |
| ----- | ----------- | ------ |
| **S1** | `This is **emphasized**.`: raw / hidden / target | 184.83 / **152.71** / 152.69 pt |
| S1 | `## The claim stated plainly`: hidden / target | **267.06** / 267.05 pt; h 21 / 21 |
| S1 | invisible extra caret stops in `a **bold** b \*x`: unsnapped / E1 snap / generalised | 5 / 4 / **0** |
| S1 | storage route survives undo-path replace and rebuild | ✅ attributes identical |
| S1 | typing after a hidden closing `**` | typing attributes 12 pt (inherited); restyle corrects to 13 pt plain |
| S1 | `NSTextList` bullet with hidden `- ` | ⛔ invisible (draws from the first character's attributes) |
| **S2** | reveal restyle: block / line / span | 0.17 / 0.19 / 0.20 ms (2 KB, 40 spans: 1.32 ms) |
| S2 | reflow: block/line re-wrap 11 → 12 lines; span 11 → 11; caret shift 16.06 pt (all) | see §5 |
| **S3** | bold via rendering attribute: mono / proportional | no re-layout, ⛔ **not drawn bold** (284.04 pt in storage vs 278.99) |
| S3 | heading size via rendering attribute | line stays 16 pt (storage: 26 pt) |
| S3 | attributes-only edits: textDidChange / undo / char-edit notifications | 0 / none / 0 (10 attr-only) |
| **S4** | rebuild 1.85 MB: E1 styler / (i) prefiltered / (i) all blocks | **5.2** / 200.9 / 411.5 ms (as stored); 5.1 / 682.3 / 683.2 ms (+ emphasis) |
| S4 | per keystroke, (i), incl. delegate | 0.03–0.13 ms (as stored); 0.17–0.26 ms (+ emphasis); largest block 0.19 ms |
| S4 | baseline to compare against | [I-0275]: 20–120 ms per keystroke (AppKit's) → **< 0.3 % added** |
| **S4b** | (iii) rebuild 1.85 MB | **0.45 ms** + 6.76 ms viewport layout; **143** paragraphs styled |
| S4c | (iii) keystroke incl. viewport relayout vs no delegate | 0.86 vs 0.53–0.66 ms |
| **S5** | command success (see §6.1) | `**`/`*` 2000/2000 word-aligned; `_` 286/2000 arbitrary |
| S5 | one command | 1 `textDidChange` |
| **W2** | Markdown round-trip in macOS 27.2 SDK | ⛔ none (parser only) |
| **W3** | Qt 6.4.2 `toMarkdown()` | ⛔ 0/1,185 identical; 1,179 re-wrapped; escapes dropped |
| W3 | md4c presents escaped text as typed | ✅ 1,199/1,200 |
| W3 | `QSyntaxHighlighter` hiding residue / heading height | **0.00 pt** / 32 vs 20 |

---

## 11. EP-046 acceptance criteria (✅ written into the Epic 2026-10-05, from the rulings)

| # | Criterion | Verified by |
| - | --------- | ----------- |
| **AC1** | **Storage stays plain**: no styling attribute is written to the text storage by rendering; the save path writes bytes unchanged (E1 AC9 holds) | integration test: edit + save + reload byte-equal; a storage-attribute scan finds no render keys |
| **AC2** | **Headings render** (`#`–`######` lines: heading font, prefix hidden), incl. every dumas `##` | unit test on the analyzer + live pass with a screenshot |
| **AC3** | **Bold and italic render**, nested included; markers hidden | analyzer test against the parser over a corpus + live pass |
| **AC4** | **Re-entry**: span for inline markers, line for prefixes (Q-E2-1); reveal is an attributes-only change (no history event, no undo step) | unit test (notification counts) + live pass |
| **AC5** | **The caret never rests on an invisible stop** — generalised hidden-run snap, both directions, clicks and shift-selection | unit test: x-duplicate steps = 0 over a corpus (S1 method) |
| **AC6** | **Commands** (Q-E2-2): ⌘B, ⌘I, Heading 1–3, Body, bulleted + numbered lists, Return continues a list; one edit each; exact-coverage rate 100% on the S5 corpus with the flank rule | corpus test (S5 method) |
| **AC7** *(carried from EP-045)* | **Unexposed block intents draw as prose**: an indented paragraph renders its inline formatting in the body font; a quote keeps its `>` and renders inline; a table renders as stored | unit test + live pass |
| **AC8** | **No added rebuild cost**: open/rebuild of the 1.85 MB fixture within 10 ms of today; keystroke cost added < 1 ms | measurement recorded (S4/S4b method) |
| **AC9** | **Option-Return stores a hard break** `\` + `\n` (Q-E2-4, [T-0584]) | unit test on the edit path |
| **AC10** | **Find/Replace matches the presented text** and writes replacements escaped (Q-E2-5, [T-0585]) | corpus test + live pass |
| **AC11** | **E1's escape hiding moved onto the presenter**; `EscapeHidingStyler` retired; every E1 escape test still passes | the existing E1 suite, green |
| **AC12** *(added 2026-10-05, user — [SP-162])* | **A cut, copy or any selection replacement that partly covers a formatted span SPLITS it, never unbalances it** — closed/re-opened at the cut point, the cut text carrying its own markers (both directions, nested); across scenes by [T-0591] ([SP-163]) | balanced-edit tests + live pass |

---

## 12. Implementation Sprints (AC-P5 — ✅ order ruled, Q-E2-3)

| Sprint | Platform | Content | Size |
| ------ | -------- | ------- | ---- |
| **E2-S1** | `[Apple]` | the presenter (route (a′)); the block analyzer + AC7 demotion; move E1's escape hiding onto it (AC11); generalised snap (AC5); **headings** + line reveal (AC2, part of AC4); AC1, AC8 | L |
| **E2-S2** | `[Apple]` | **bold / italic** + span reveal (AC3, AC4); copy puts presented text on the pasteboard with markers removed (extends E1's `writeSelection`) | M |
| **E2-S3** | `[Apple]` | **commands** (AC6) incl. lists (prefix visible, hanging indent) and Return-continues-list; **[T-0584]** Option-Return (AC9) | M |
| **E2-S4** | `[Apple]` | **Find/Replace** over presented text, **[T-0585]** (AC10); first task: the `NSTextFinderClient` question (§7) | M |
| **EP-048 S1–S4** | `[Linux]` | scoped in [`Epics/Epic-backlog.md`](Epics/Epic-backlog.md) EP-048 (ACs L1–L8), from Q-E2-8 | — |

---

## 13. Not measured / risks

- ⚠️ **Multi-line blocks:** TextKit 2 requests paragraphs per **line**, so a soft-wrapped Markdown paragraph (`\n`
  inside a block) is parsed once per line. A block cache keyed on range + text bounds this; not measured.
- ⚠️ **Shift-selection and drag-selection across hidden runs** were not measured (E1 R3 left the same gap).
- ⚠️ **VoiceOver / accessibility** reads storage. Hidden markers may be spoken; not checked.
- ⚠️ **Spell-check underlines** come from storage; markers are not words, but behaviour at a hidden run was not checked.
- ⚠️ **iOS/visionOS:** `NSTextContentStorageDelegate` is the same API in UIKit (iOS 15+). Not built there.
- ⚠️ **The one dumas `##` line not rendered** as a heading (1,171/1,172) was not traced.
- ⚠️ **Linux parser escapes (md4c callbacks for `\*`)** were not checked; that is the first measurement of EP-048.
