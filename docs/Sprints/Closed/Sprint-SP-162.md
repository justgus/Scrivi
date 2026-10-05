---
sprint: SP-162
epic: EP-046
status: Closed
closed: 2026-10-05
activated: 2026-10-05
platform: Apple
created: 2026-10-05
---

# SP-162 — `[Apple]` [EP-046] **E2-S2**: bold and italic + span reveal

**Status:** ✅ **CLOSED 2026-10-05 (user-approved):** *"Yes, verify T-0590 and close SP-162"* — activated 2026-10-05. ✅ Q1–Q3 ruled; all ACs met; [T-0590] VERIFIED and archived.
**Task:** ✅ [T-0590] → [`../../Tasks/Verified/Task-verified-0590.md`](../../Tasks/Verified/Task-verified-0590.md)
**Epic:** [EP-046] `[Apple]` The Manuscript Renderer — Inline Rendering → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md)
**Authority:** [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) — §0A
(rulings), §3.5 (as built in E2-S1), §4.3 (inline), §5 (span reveal), §12 (Sprint order). Previous Sprint:
[`Closed/Sprint-SP-161.md`](Closed/Sprint-SP-161.md).
**Size:** M. ✅ The mechanism exists (E2-S1's presenter); this Sprint extends the analyzer and the reveal to INLINE markers.

---

## Goal

✅ **`**bold**` and `*italic*` RENDER; their markers hide, and reappear (dimmed) for the span the caret touches**
(Model B, Q-E2-1 span half). Existing files' emphasis (R2) renders; typed `*` stays escaped and literal.

## EP-046 ACs this Sprint meets

| AC | Criterion (Epic wording) |
| -- | ------------------------ |
| **AC3** | Bold and italic render, nested included; markers hidden |
| **AC4** *(span half)* | Re-entry for inline markers: the span the caret touches shows its markers; attributes-only (no history event, no undo step) |
| **AC7** *(inline half)* | An indented paragraph renders its inline formatting in the body font; a quote keeps its `>` visible and renders inline; a table renders as stored |
| **AC5** *(re-checked)* | No invisible caret stop with inline markers present |
| **AC8** *(re-checked)* | Rebuild within 10 ms of today; < 1 ms added per keystroke (1.85 MB, with emphasis) |

## Plan

1. **Analyzer — inline** (`MarkdownBlocks.swift`): from source positions, `markers` (uncovered non-whitespace
   delimiters), `bold` / `italic` ranges, and `spans` (content + its delimiters, for the reveal), as spiked in SP-159
   (`scratchpad/sp159/h/MD.swift`). ✅ Each marker run is classified **OPENING** or **CLOSING** (needed by the span reveal
   and by the caret rules ruled for [T-0589]). AC7: an indented block is re-analysed with its indentation stripped and
   gets INLINE rendering only (no heading); a quote keeps `>` visible.
2. **Presenter — inline**: bold / italic / bold-italic by `NSFontManager` trait conversion of the line's font (body or
   heading); hidden markers at 0.01 pt + clear; a revealed span's markers in Q2's attributes (dimmed, body font).
3. **Span reveal**: the presenter's reveal unit grows from LINES to (lines for prefixes) + (the inline span the caret
   touches, design §5). The `hiddenTest(in:revealing:)` seam already takes the proposed selection.
4. **Caret snap with inline markers** (AC5): runs of hidden markers outside the revealed span. ⚠️ See Q1.
5. **Copy** (design §12, Q2): the external pasteboard gets the PRESENTED text, with hidden markers removed (extends E1's
   `writeSelection`, which today removes only escape backslashes); an in-app paste still restores the stored source
   (E1's `ownCopy`), so `**` survives copy → paste inside Scrivi.
5a. ✅ **A selection that PARTLY covers a formatted span is SPLIT, never left unbalanced** (user, 2026-10-05: *"If the
   user selects text in the middle of a bold section and past the end of the bold section, then a cut there cuts the
   text and splits the bold section. That is, we must remember to terminate the bold section at the cut point and
   insert a bold begin at the beginning of the cut text. Cutting and pasting bold text within the app should try to
   preserve its boldness. This is similarly true for a selected section that begins outside the bold section and ends
   inside it."*):
   - **Starts inside, ends outside** — `**bo|ld** an|d` → what stays: `**bo**d` (the span is CLOSED at the cut point);
     what is cut: `**ld** an` (an OPENER is prefixed, so it stays bold when pasted).
   - **Starts outside, ends inside** — `pr|e **bo|ld**` → what stays: `pr**ld**` (an OPENER at the cut point); what is
     cut: `e **bo**` (a CLOSER is appended).
   - **Inside on both ends** — `**a|bc|d**` → what stays: `**ad**` (already balanced); what is cut gets both
     markers: `**bc**`.
   - ✅ The in-app clipboard (`ownCopy`) holds the BALANCED source, so a paste inside Scrivi keeps the formatting;
     the external pasteboard gets the characters only (Q2). Copy (not cut) balances the clipboard the same way.
   - ✅ Every inserted marker obeys rule 4 of [T-0589]: a span begins and ends with a VISIBLE character — if the cut
     point leaves whitespace at a span's edge, the marker goes inside it, not after it.
   - ⚠️ Nested spans (`*it **both** it*`): balance each crossing span, innermost first.
   - ✅ **Q3 (ruled): the same for ⌫/⌦ over a selection, typing over it, and pasting over it** — every replacement of a
     selection that crosses a span boundary leaves the remaining text balanced.
6. **Tests**: analyzer against the parser over a corpus (SP-159 S5 method: escaped dumas + emphasis); presented
   paragraphs; LAID-OUT widths (E2-S1 method); span reveal posts 0 `textDidChange`; arrowing with inline markers has 0
   invisible stops; nested `*it **both** it*`; malformed `**unclosed` shows literally (Q-E2-7); AC7 indented/quoted.
   Mutation-check the new tests, as in E2-S1.
7. **AC8 measurement** on 1.85 MB dumas with synthetic emphasis (two spans per paragraph — S4's corpus), real files.
8. **Live pass** (user) — written from the app as it is: Scrivi autosaves; there is no Save.

## ✅ Questions — Q1, Q2, Q3 ALL RULED 2026-10-05 (before activation)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | Hints ON, a click at the visual start of a bold word — caret before or after the revealed `**`? | ✅ **RULED: AFTER it** (user: *"I agree. In addition, the hint should be shown to the left of the caret emphasising that the caret is inside the attributed section (headers too)."*). ✅ So the revealed OPENING marker sits to the LEFT of the caret, and ⚠️ **the same holds for HEADINGS**: on a heading line the caret never rests before or inside `## ` — it lands after it, with the dimmed `## ` to its left. ⚠️ **This changes E2-S1's heading behaviour** (today a caret can rest at the line start, before the revealed `## `). At the END of a span: before the closing marker, inside ([T-0589] rule 3) |
| **Q2** | What the external pasteboard gets for bold text copied out of Scrivi | ✅ **RULED: markers removed** (user: *"I agree markers removed."*). Plain characters; no RTF bold |
| **Q3** | ✅ **RULED: YES** (user: *"Q3: yes."*) — **the split rule below applies to EVERY edit that replaces a selection** (⌫/⌦ over a selection, typing over it, pasting over it) — not only cut? | ✅ Deleting `ld** and` from `**bold** and` leaves `**bo` — an unclosed `**` that then shows LITERALLY (Q-E2-7). Any replacement of a selection that crosses a span boundary breaks the balance the same way a cut does |

⛔ **NOT in this Sprint:** commands, lists, Option-Return [T-0584] (E2-S3) · Find/Replace [T-0585] (E2-S4) · the Markup Hints
toggle [T-0589] (EP-047) · Linux ([EP-048]).

## Acceptance Criteria

- [x] **AC3** bold/italic render, nested — analyzer + presented paragraph + LAID-OUT width (markers take no width) ✅
- [x] **AC4 (span half)** — `spanReveal`: the caret's span shows its markers, re-laid by TextKit, 0 `textDidChange` ✅
- [x] **AC7 (inline half)** — indented and quoted paragraphs render emphasis; a table does not ✅
- [x] **AC5 (re-checked)** — `caretHomes`: exact → / ← stop sequences; SP-161's walk still 0 stalls ✅
- [x] **AC8 (re-checked)** — measured below ✅
- [x] Copy: other apps get the presented text (Q2); Scrivi's paste restores the balanced source ✅
- [x] **AC12** cut/copy across a span edge SPLITS and balances, both directions + inside; bold pasted into bold MERGES ✅
- [x] **Q3** typing over, Return inside, ⌫ of a span's last character — all balanced ✅
- [x] **Q1** caret after an opener / `## ` (hint to its left), before a closer ✅
- [x] ✅ **Live pass (user) — PASSED 2026-10-05**; two amendments made after it, both re-checked ✅

---

## Progress log

### 🔵 2026-10-05 — Sprint CREATED in Planning

✅ User: *"create E2-S2"*. ✅ ID SP-162 issued by `next-id.py`. ✅ Scope from the approved design §12 (E2-S2) and the
ruled order Q-E2-3. ✅ The user's caret rulings for hidden markers (2026-10-05, recorded in [T-0589]) shape plan step 1
(opening/closing classification) and Q1.

### 🔵 2026-10-05 — Q1 + Q2 RULED; the split rule added (plan 5a); Q3 raised

✅ User: *"Q1: I agree. In addition, the hint should be shown to the left of the caret emphasising that the caret is
inside the atributed section (headers too). Q2: I agree markers removed."* ✅ Plus the cut/copy split rule (plan 5a,
quoted there). ⏳ Q3 (the same rule for every selection replacement) recommended, not yet ruled.

### 🔵 2026-10-05 — Q3 RULED

✅ User: *"Q3: yes."* — the split rule covers every replacement of a selection, not only cut. ✅ All three questions are
ruled; the Sprint is ready to activate on the user's approval.

### 🟡 2026-10-05 — Sprint ACTIVATED (user-approved); [T-0590] allocated

✅ User: *"activate SP-162"* ✅ [T-0590] issued by `next-id.py`. ✅ Left `Sprint-backlog.md` at activation.

### 🟠 2026-10-05 — IMPLEMENTED (T-0590 Implemented - Not Verified); ⏳ live pass owed

✅ User: *"implement SP-162"*.

**What was built:**
- `MarkdownBlocks.swift` — inline analysis: emphasis MARKERS (uncovered `*`/`_` runs where the style changes; each
  OPENING or closing), per-unit STYLES, maximal SPANS. A `* item` bullet, `**unclosed`, inline code and link syntax are
  not markers (they stay visible). AC7: an indented block is re-read inline-only; a quote keeps `>`; a table renders as stored.
- `MarkdownEmphasis.swift` (new; 3 targets) — the BALANCING model: a stretch becomes tokens (every unit except markers,
  with its style), the edit is applied to tokens, and the stretch is written back with the fewest markers. Rules:
  whitespace takes the style both neighbours share (a span begins/ends on a visible character — the user's rule);
  a newline carries no style (Return inside bold → two balanced spans); a span never OPENS on whitespace; the style that
  lasts longer opens outside; ⚠️ a span cannot open on punctuation right after a letter (or close on punctuation right
  before one) in CommonMark, so that one punctuation mark loses its style. Italic is written `*`.
- `ManuscriptPresenter.swift` — bold/italic/bold-italic fonts (on body or heading fonts); markers hidden, or dimmed in
  the span the caret is in; `stopTest` (escape → home before; opener and `## ` → home AFTER, Q1; closer → home before);
  `balancedEdit` (only the stretch the edit touches is rewritten; ✅ checked by the parser IN CONTEXT before it is
  applied — if it would not read back, the stretch is written without emphasis and `[SCRIVI-EDIT] emphasis could not be
  balanced` is logged); `balancedCopy`.
- `ManuscriptEscapes.swift` — the snap is role-aware: every spot in or beside a stop run goes HOME; one arrow step from
  home passes one visible character (→ past a closer, ← past the character before an opener/prefix).
- `ManuscriptTextView.swift` — `shouldChangeText` re-issues an unbalancing edit as the balanced one (ONE replacement →
  one history event); copy uses `balancedCopy`; ⌫ after an opener deletes the character before it; ⌫ at a heading's
  start removes the whole `## `; ⌦ before a closer deletes the character after it.
- `project.pbxproj` — `MarkdownEmphasis.swift` in ScriviApp, -iOS, -visionOS.

**Measured before wiring (harnesses `scratchpad/sp162/`):** realistic edits (escaped dumas paragraphs with 1–3 word
spans, random cut / type-over / paste / copy) read back exactly **5,999/6,000**; hostile random styles 4,871/5,000 —
the residue is what the in-context check catches.

**Tests:** ✅ **183/183 in 21 suites** (175 + 8 new in "Inline emphasis (EP-046 E2-S2)"; E1 snap unit tests moved to the
run-lookup API; the shared Linux byte corpus still green — stored bytes unchanged for its cases). ✅ **Mutations bite:**
balancing off → 4 tests red (9 issues); openers given a closer's home → 3 red; span reveal off → `spanReveal` red.
✅ `check-textkit2.sh` clean; ✅ iOS + visionOS BUILD SUCCEEDED.

**AC8 (real files, 1.85 MB dumas; E1 at HEAD vs E2-S2):**
| | as stored | + emphasis (2 spans/paragraph) |
| - | - | - |
| rebuild + viewport layout | 6.34 → **3.38 ms** | 6.55 → **4.40 ms** |
| keystroke (3 offsets, presentation) | 0.54–0.68 → 0.62–0.79 ms | 0.58–0.68 → 0.71–0.78 ms |
| balancing check per keystroke (plain typing, fast path) | — | **0.03 ms** (max 0.06) |
| a balanced cut across a span edge (+ layout) | — | 0.74 ms |

⚠️ **Decisions made in implementation — for the user to confirm in the live pass:**
1. **⌫ at the start of a heading removes the whole `## `** (the line becomes body text). Markers are atomic; deleting
   the prefix one character at a time would be editing markup by hand.
2. **Copy to other apps drops a heading's `## ` too**, not only inline markers (Q2 read as "what the writer sees").
3. **Punctuation at a span edge:** where CommonMark cannot open/close a span (punctuation glued to a letter), that one
   punctuation mark loses its style rather than leaving a visible marker.
⚠️ **Not covered:** a cut/copy that crosses a SCENE boundary goes through ScriviCore's structured fragments
(`structuredCopyIfCrossBoundary`), which are not balanced yet.

⏳ **Live pass (user) — Scrivi autosaves; there is no Save:**
1. ⚠️ **Scrivi cannot CREATE bold yet** — typed `*` is escaped, and ⌘B arrives in E2-S3. Use a TEST project (not real
   writing work) and add `**bold**` / `*italic*` to a scene file in a text editor before opening it. The bold and italic
   RENDER, with no markers visible.
2. Click at the start of a bold word: the dimmed `**` appears to the LEFT of the caret; type — the text joins the bold.
   Click at the end of it: type — the bold continues. Arrow across it: the caret never pauses without moving.
3. Select from the middle of a bold word past its end, cut (⌘X), paste it elsewhere: what is left stays bold and closed,
   and the pasted text is bold. Same for a selection that starts before a bold word and ends inside it.
4. Return in the middle of a bold word: both halves stay bold. ⌫ a one-letter bold word: no stray `****`.
5. ⌫ at the start of a heading (`##` dimmed to its left): the heading becomes body text (decision 1).
6. Undo each of the above: one ⌘Z undoes one gesture; the scene file keeps its format.

### ✅ 2026-10-05 — Live pass PASSED; decisions 1–3 CONFIRMED; two amendments implemented

✅ User: *"1: confirmed. 2: confirmed. 3: confirmed. The Live pass passes. All the test behavior works as expected. Actually I was expecging the Markup Hints to go away once the cursor was no longer at the first or last character of the section. They currently stay visible whenever the caret is inside the section no matter where the caret is in the section. Also, I find the BOLD text to be too subtle in both Light and Dark modes. Italics are fine. We just need a fine adjustment on how bold is bold."*
✅ **Decisions confirmed:** (1) ⌫ at a heading's start removes the whole `## `; (2) copy to other apps drops a heading's
`## ` too; (3) punctuation glued to a letter at a span edge loses its style.

**Amendment A — Markup Hints only at the edges of a section.** ⛔ The span reveal showed a span's markers whenever the
caret was anywhere inside it. ✅ Now only with the caret at the span's FIRST character (right after an opening marker)
or LAST character (right before a closing one) — `ManuscriptPresenter.revealSpans`. ⚠️ A nested span (`**a *b* c**`)
is one section: at any of its edges, all its markers show. ⚠️ **Headings unchanged** (the `## ` shows whenever the caret
is on the heading line) — ⏳ asked whether the same rule should apply there.

**Amendment B — bold weight.** ⛔ MEASURED: `NSFontManager`'s bold trait gives the monospaced system font **Semibold**
(weight 0.30) — the "too subtle" bold. ✅ Now one real weight step above the text: body → **Bold** (0.40); inside a heading
(already bold) → **Heavy** (0.56) — before, bold inside a heading did not show at all. Comparison sheets (Light/Dark,
Semibold / Bold / Heavy): `scratchpad/bold/bold-light.png`, `bold-dark.png`. ⚠️ "Black" does not exist for this face (it
falls back to Heavy).

✅ Tests: **184/184** (+1 `boldWeight`; `spanReveal` now asserts shown at the first character, hidden mid-span, shown at
the last). ⏳ **Re-check owed:** click into the middle of a bold word (no hints), then at its first and last character
(hints); compare bold against italic in Light and Dark.

### ✅ 2026-10-05 — Amendment B re-checked (bold weight good); heading reveal stays line-wide; nested reveal kept

✅ User: *"That's ok. The bold level is good. I notice that hitting a nested italic inside a bold section also shows all the hints. I think I like it. I think the writer is going to want to turn it on and off a lot though. It allows the writer to verify where her formatting start and end (something MS Word does not do), and Markup editors like Ulysses show the markup characters all the time only smaller and hidden. Our goal is to allow the writer to forget that she is editing markup. When we implement the \"Show/Hide Markup Hints\" view option we should also investigate toggling it with a keystroke."*
✅ **Bold weight: accepted.** ✅ **Headings: unchanged** — the `## ` shows whenever the caret is on the heading line
(answer to the open question: *"That's ok"*). ✅ **A nested span reveals all its markers** at any of its edges — kept
(*"I think I like it"*). ✅ The keystroke-toggle idea is recorded on [T-0589].

### ✅ 2026-10-05 — [T-0590] VERIFIED + archived; Sprint CLOSED (user-approved)

✅ User: *"Yes, verify T-0590 and close SP-162"* ✅ [T-0590] → [`../../Tasks/Verified/Task-verified-0590.md`](../../Tasks/Verified/Task-verified-0590.md).

---

## Retrospective

**Completed:** ✅ EP-046 AC3, AC4 (span half + the Q1 and live-pass amendments), AC7 (inline half), AC12 (incl. Q3); AC5 and
AC8 re-checked. Bold and italic render; markers are atomic stop runs with a home side; every edit that would unbalance
emphasis is re-issued as one balanced edit, checked by the parser before it is applied.
**Returned to Backlog:** none. **Carried:** cut/copy across a SCENE boundary is not balanced (ScriviCore structured
fragments) — ⚠️ not yet filed; the Markup Hints toggle + keystroke → [T-0589].
**What went well:** ✅ The balancing model was proven in a harness BEFORE it was wired in (5,999/6,000 realistic edits),
and each failure class found there became a rule (longer style opens outside; never open on whitespace; punctuation at an
edge). ✅ The user's caret rulings (Q1, rules 2–3) made one consistent "home side" model that also simplified the snap —
it no longer depends on the reveal. ✅ Measuring the font explained "too subtle": the bold trait gives Semibold.
**What to improve:** ⚠️ My first span reveal showed hints anywhere inside a section; the user expected them only at its
edges. The design said "the span the caret touches", which I read as "is inside". Ask what "touches" means before
building a visible behaviour. ⚠️ Two test assertions failed first because a caret placement one unit from home looks
exactly like an arrow step — the test, not the code, was wrong.
