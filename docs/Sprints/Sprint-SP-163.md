---
sprint: SP-163
epic: EP-046
status: Active
activated: 2026-10-05
platform: Apple
created: 2026-10-05
---

# SP-163 — `[Apple]` [EP-046] **E2-S3**: the formatting commands + lists + Option-Return

**Status:** 🟡 **ACTIVE 2026-10-05** (user: *"Lets activate SP-163."*) — created 2026-10-05. ✅ Q1–Q7 ruled.
**Tasks:** [T-0592] (this Sprint's work) · [T-0584] Option-Return · [T-0591] cross-scene balancing (added at activation) → [`../Tasks/Task-active.md`](../Tasks/Task-active.md)
**Epic:** [EP-046] `[Apple]` The Manuscript Renderer — Inline Rendering → [`../Epics/Epic-active.md`](../Epics/Epic-active.md)
**Carries:** [T-0584] Option-Return (ruled Q-E2-4 = (b), a hard break) · ✅ [T-0591] balance emphasis across SCENE boundaries (added by the user at activation).
**Authority:** [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) — §0A
(Q-E2-2 command surface, Q-E2-4), §4.2 (lists), §6 (commands: the S5 measurements and the flank rule), §3.6 (E2-S2 as built).
Previous Sprint: [`Closed/Sprint-SP-162.md`](Closed/Sprint-SP-162.md).
**Size:** M. ✅ The machinery exists: a command is a STYLE edit on E2-S2's tokens (`MarkdownEmphasis`), written back balanced.

---

## Goal

✅ **The writer can CREATE formatting.** Under the escape ruling (study §3A.0) the commands are the feature's ENTIRE input
surface — until now, bold could only come from a file edited elsewhere.

## EP-046 ACs this Sprint meets

| AC | Criterion (Epic wording) |
| -- | ------------------------ |
| **AC6** | Commands (Q-E2-2): ⌘B `**`, ⌘I `*` (never `_`), Heading 1–3 ⌥⌘1–3, Body ⌥⌘0, bulleted + numbered lists, Return continues a list; one edit each; 100% exact coverage on the S5 corpus with the flank rule |
| **AC9** | Option-Return stores a hard break `\` + `\n` (Q-E2-4, [T-0584]) |
| *(design §4.2)* | Lists RENDER: prefix visible but dimmed, hanging indent |

## Plan

1. **A Format menu** (macOS: none exists today — verified 2026-10-05): Bold ⌘B · Italic ⌘I · Heading 1/2/3 ⌥⌘1/2/3 ·
   Body ⌥⌘0 · Bulleted List ⌥⌘L · Numbered List ⌥⌘N. ✅ **Checked 2026-10-05: no conflict** — the copy buffers take
   ⌘/⌃/⌥ + 1–9 with exactly ONE modifier; the View toggles take ⌥⌘I/T/B; ⇧⌘0/1 go to the manuscript start/end.
   Enabled only with the manuscript focused. Routed like the Scene/Chapter menus (session action closures).
2. **Bold / Italic as a STYLE EDIT** (`ManuscriptPresenter`): tokens of the selection (+ the spans it touches) → set the
   bit on every visible selected token, or CLEAR it when all of them already have it (toggle) → `MarkdownEmphasis`
   normalise + serialise → parser check in context → ONE replacement through the `applyingBalancedEdit` path.
   ✅ That gives the design §6.1 rules for free: whitespace trimmed at the edges (the user's rule 4), `*` not `_`,
   a selection across paragraphs formatted per paragraph, toggle-off of part of a span splits it, nesting kept.
   ⚠️ The flank rule (§6.1: extend an edge to the word boundary only if it would not flank) is E2-S2's punctuation rule
   in reverse — measure against the S5 corpus (2,000 selections × `**`/`*`, escaped dumas) and pick (Q3).
2a. **The PENDING pair (Q1, between words):** ⌘B / ⌘I with the caret between words inserts the marker pair (`****` / `**`)
   with the caret between, presented as REVEALED hints. The presenter tracks it as pending (the parser sees no span in an
   empty pair). Typing inside fills it — `**x**` is a real span from the first character. ⛔ If the caret leaves while it
   is still empty, the pair is REMOVED. ⚠️ To settle in the work: the removal must not leave an extra undo step (an
   insert-then-remove pair should vanish from history, or be one step), and ⌘B again while pending removes it (toggle).
3. **Headings / Body** per selected paragraph: write, replace or remove the ATX prefix (`# `/`## `/`### `); a paragraph is
   exactly one of body / heading / list item, so a heading command replaces a list prefix and vice versa (Q5).
4. **Lists** per selected paragraph: `- ` or `1. `, `2. `… (Q4); the command again on a list of that kind removes it.
   **Rendering** (design §4.2): prefix VISIBLE, dimmed, with a hanging indent (`headIndent` = prefix width) so wrapped
   lines align under the text. The analyzer reports list prefixes (by the parser's `listItem` intent, like headings).
   The prefix is a stop run with its home AFTER it, and ⌫ at an item's start removes it (as for headings — confirmed).
   **Return** in a list item writes `\n\n` + the next prefix; Return on an EMPTY item removes its prefix (ends the list).
5. **Option-Return** ([T-0584], AC9): `insertNewlineIgnoringFieldEditor:` stores `\` + `\n` (one edit). ⚠️ See Q6.
6. **Tests**: every command through the REAL menu/key dispatch (`feedback_test_through_the_real_dispatch`); exact stored
   bytes; one `textDidChange` per command; the S5 corpus through the real command; toggle on/off round trip; lists:
   rendering (hanging indent laid out), Return continue/end, ⌫ at an item start; Option-Return bytes + hidden backslash.
   Mutation-check the new tests.
7. **Live pass** (user) — written from the app as it is (it autosaves; there is no Save).

## ✅ Questions — ALL RULED 2026-10-05 at activation

| # | Question | Ruling |
| - | -------- | -------------- |
| **Q1** | ⌘B / ⌘I with NO selection (just a caret) | ✅ **RULED:** in a word → **format the word**. ✅ **Between words → a PENDING empty pair** (user: *"add the start and end hints, so that when the writer start typing, the text is bold or italic. Visibly … the start and end hints smashed together with the caret between them"*). ⚠️ Design (plan 2a): Markdown cannot STORE an empty `****` (the parser reads it as four literal asterisks), so the pair is PENDING — shown as revealed hints with the caret between them, kept once typing fills it, and ⛔ removed if the caret leaves it empty, so no stray `****` ever reaches the file |
| **Q2** | The shortcut set above (⌘B, ⌘I, ⌥⌘1–3, ⌥⌘0, ⌥⌘L, ⌥⌘N) | ✅ **RULED: keep it** |
| **Q3** | An arbitrary selection whose edge cannot open/close a span (punctuation glued to a letter, S5's 1.6%) | ✅ **RULED: shrink to the visible text that CAN carry it** — one rule for edits and commands |
| **Q4** | Numbered lists: what numbers are stored | ✅ **RULED: sequential and kept sequential** — a command or Return renumbers the following items in the same edit |
| **Q5** | A heading command on a list item (or a list command on a heading) | ✅ **RULED: replace** — a paragraph is one of body / heading / list item |
| **Q6** | Option-Return at the END of a paragraph: CommonMark reads `\` before a blank line as a LITERAL backslash (EP-045 AC6, Q1 = (a)), so it shows until the writer types the next line | ✅ **RULED: accept** — the backslash shows until typing begins on the next line |
| **Q7** | List prefix: visible + dimmed (design §4.2, v1) vs hidden with a drawn `•` (an `NSTextLayoutFragment` subclass; TextKit's own bullet draws invisible — SP-159 S1) | ✅ **RULED: visible and dimmed** |

8. ✅ **[T-0591] — balance emphasis across SCENE boundaries** (added at activation): cross-scene cut/⌫/⌦
   (`deleteAcrossScenes`), the structured copy (`structuredCopyIfCrossBoundary`: balanced per-scene slices; other apps get
   the presented text — Q2), and the structured paste (merge bold into bold). Each gesture stays ONE grouped history event.
   ⚠️ Rule early in the Sprint: per-scene balancing Apple-side (as E2-S2) or in ScriviCore's fragment ops (Linux would
   get it — [EP-048] L7).

⛔ **NOT in this Sprint:** Find/Replace [T-0585] (E2-S4) · the Markup Hints toggle [T-0589] (EP-047) · Linux ([EP-048]).

## Acceptance Criteria

- [x] **AC6** — every command, one edit each (`textDidChange` = 1), exact bytes; corpus test (300 random selections over escaped prose): 0 refused, 0 leaked, 0 missed ✅
- [x] **AC9** — Option-Return stores `\` + `\n` ([T-0584]) — Apple AND Linux (shared corpus) ✅
- [x] Lists render (dimmed prefix, hanging indent); Return continues / ends a list (renumbering, Q4); ⌫ at an item's start removes its prefix ✅
- [x] Q1: in a word → the word; between words → a PENDING pair: kept when typed into, removed if left empty, nothing recorded ✅
- [x] [T-0591]: cross-scene cut/⌫/⌦, copy and paste keep emphasis balanced in every scene; other apps get the presented text ✅ (unit-level; ⏳ live)
- [x] ✅ **Live pass (user) — PASSED 2026-10-06** (18 steps; steps 12 and 17 fixed and re-checked; [I-0279] and the span-edge fix re-checked)

---

## Progress log

### 🔵 2026-10-05 — Sprint CREATED in Planning

✅ User: *"Lets file the balancing task for the cut copy across scene boundaries … Then we can begin E2-S3. I pushed the
working tree."* ✅ ID SP-163 issued by `next-id.py`. ✅ Scope from the approved design §12 (E2-S3) and Q-E2-2 / Q-E2-4.
✅ Shortcut conflicts checked in `ScriviApp.swift` and `ManuscriptNSTextView.keyDown` before proposing (none).

### 🟡 2026-10-05 — Q1–Q7 RULED; [T-0591] ADDED; Sprint ACTIVATED (user-approved); [T-0592] allocated

✅ User: *"Q1: in a word, format the word. between words, I know it will be harder but, add the start and end hints, so that when the writer start typing, the text is bold or italic. Visibly which is surface as the start and end hints smashed together with the caret between them. Q2: keep it. Q3: your recommendation confirmed. Q4: your recommendation confirmed. Q5: Replace confirmed. Q6: confirmed show until typing begins. Q7: confirmed visible and dimmed. Add T-0591 to SP-163 as well. Lets activate SP-163."*
✅ [T-0592] issued by `next-id.py`. ✅ [T-0591] moved from `Task-backlog.md` to `Task-active.md`. ✅ Left `Sprint-backlog.md`.

### 🟠 2026-10-05 — IMPLEMENTED (T-0592, T-0584, T-0591 Implemented - Not Verified); ⏳ live pass owed

✅ User: *"Undo an abandoned pair: agreed. it should be removed. Please give me the recommendation for T-0591 when the work starts, although from your comments I think I already know what the recommendation will be. Go ahead and Implement SP-163."*
✅ **[T-0591] RULED at the start of the work: Apple-side** (AskUserQuestion → *"Apple-side now (Recommended)"*). ScriviCore
has no Markdown parser (`MarkdownStrip` only strips), so core-side balancing would first need md4c — [EP-048] L1's work.
Revisit when EP-048 brings md4c into the core (the core could then own ALL balancing, for both platforms).
✅ **Pending-pair undo ruling** (user: *"agreed. it should be removed"*): ⚠️ READ first — autosave runs `flushThenSave` 1 s
after an edit, so a pair in the SCENE text would be committed and written. ✅ So the pair lives in the text view's storage
ONLY (inserted / removed without `didChangeText`): history, autosave and the scene text never see it; typing into it is an
ordinary recorded edit (`**x**`); an abandoned pair leaves nothing.

**What was built:**
- `ManuscriptCommands.swift` (new; 3 targets) — `wordRange` (Q1: a caret touching a letter is IN the word; an escaped `'`/`-`
  inside a word keeps it one word); `emphasisEdit` (toggle a style on the selection's tokens → E2-S2's rewrite core →
  parser check; a COMMAND that cannot be balanced is REFUSED with a beep, never written without formatting);
  `paragraphEdit` (heading / body / bullet / numbered per paragraph, Q5 replace, same command again → body, numbered items
  kept sequential incl. the ones that follow, Q4); `listReturnEdit` (next item + renumber; an EMPTY item ends the list and
  the items after close the gap); `balancePastePieces` ([T-0591]).
- `ManuscriptPresenter.swift` — list rendering (prefix visible + dimmed, hanging indent, Q7); list prefix = a stop run
  (home after it; ⌫ at an item's start removes it); the PENDING pair (shown as revealed hints, its caret home between);
  `rewrite` factored out of `balancedEdit` (shared by edits and commands) and ⚠️ now normalises WITH one token of
  context each side.
- `MarkdownEmphasis.swift` — ⚠️ `normalize` repeats its whitespace and punctuation passes until stable (one could undo the
  other: `p**ublication **\(`); a single newline may sit INSIDE a span (only a paragraph break ends one — Option-Return's
  `\`+`\n` inside bold must not get a closer after its backslash); `serialize(open:leaveOpen:)`; `ManuscriptFormat`.
- `MarkdownBlocks.swift` — list items from the parser's `listItem` intent (prefix, ordered, number); ⚠️ the fast path now
  also looks for list lines (a plain `- item` has no `#*_`); ⚠️ an EMPTY item (`2. ` alone) gives the parser no run, so it
  is recognised directly.
- `ManuscriptTextView.swift` — `applyFormat` (what the menu calls), `applyCommandEdit` (one replacement), pending pair
  insert/remove (+ ⌫/⌦ in it, ⌘B again, whitespace typed into it goes before it), Return in a list, Option-Return
  (`insertNewlineIgnoringFieldEditor:` → `\`+`\n`), [T-0591] in `deleteAcrossScenes` (each scene part through
  `balancedEdit`), `structuredCopyIfCrossBoundary` (balanced pieces + presented text) and `pasteStructuredFragment`.
- `ScriviApp.swift` — the **Format** menu (⌘B, ⌘I, ⌥⌘1–3, ⌥⌘0, ⌥⌘L, ⌥⌘N); `ProjectSession.formatAction`.
- ✅ **Linux, in the same work** (`feedback_linux_adopts_apple_shape`): `ManuscriptEditor::handleReturn` Alt-Return now stores
  `\`+`\n`; the SHARED corpus case updated. ✅ Built in the canonical Linux image (Ubuntu 24.04 / Qt 6.4): `escape_smoke`
  **PASS**; ✅ the same binary against the OLD corpus **FAILS** on that case (the check bites). Image and build cache removed.
- `project.pbxproj` — `ManuscriptCommands.swift` in ScriviApp, -iOS, -visionOS.

**Measured on the way (harness `scratchpad/sp163/`):** random ⌘B selections over escaped prose — refused 6.9% → 2.0% (context)
→ 0% of selections with a letter in them (stable passes). E2-S2's corpora re-run: realistic edits **6,000/6,000** (was
5,999); hostile 4,892/5,000 (was 4,871).

**Tests:** ✅ **193/193 in 22 suites** (184 + 9 new in "Format commands (EP-046 E2-S3)"; `AC5a` updated — it pinned the
pre-ruling Option-Return). ✅ **Mutations bite (5/5):** word formatting off; pending pair not removed; renumbering off;
normalisation context off; list fast path off. ✅ `check-textkit2.sh` clean; ✅ iOS + visionOS BUILD SUCCEEDED.

⚠️ **Decisions made in implementation — for the user to confirm:**
1. **A caret just BEFORE or AFTER a word counts as IN it** (Q1: it touches a letter) — ⌘B there bolds the word.
2. **The same heading level / list kind again returns the paragraph to body** (Body ⌥⌘0 does it too).
3. **Return on an EMPTY list item renumbers the items after it** (they close the gap — Markdown still reads them as one list).
4. **Whitespace typed into an empty pending pair goes before it**; the pair stays pending.
5. **A Format command that cannot be written balanced is refused with a beep** — it never drops existing formatting.
6. After ⌘B on a selection, the selection takes the opening `**` with it (E2-S2's selection snap) — the same text stays selected.
⚠️ **Not done:** Return in a list item does not trim trailing spaces before the caret (EP-045 AC6 does for body text) — minor.

⏳ **Live pass (user) — Scrivi autosaves; there is no Save:**
1. Select words, ⌘B: bold; ⌘B again: plain. ⌘I inside bold: bold-italic. Select the middle of a bold phrase, ⌘B: it splits.
2. Caret inside a word, ⌘B: the word goes bold, the caret stays put. Caret between two words (after a space), ⌘B: `****`
   appears dimmed with the caret between — type: the text is bold. Try again and click elsewhere instead: it vanishes, and
   ⌘Z does not bring it back.
3. ⌥⌘2 on a paragraph: heading; ⌥⌘1: level 1; ⌥⌘0: body. ⌥⌘L on two paragraphs: a bulleted list (dimmed `- `, wrapped
   lines indented). ⌥⌘N: numbered 1., 2. Return at the end of an item: the next number, later items renumbered. Return on
   the empty item: the list ends.
4. Option-Return mid-paragraph: a line break, no backslash visible once the next line has text.
5. Across a scene divider: select from inside a bold word into the next scene, cut — both scenes keep balanced bold; paste
   it inside a bold word elsewhere — it stays bold, no stray `**`.
6. ⌘Z after each command undoes exactly that command; the scene files keep their format.

### ✅ 2026-10-05 — Decisions 1–6 CONFIRMED (user)

✅ User: *"1. confirmed. 2. confirmed. 3. I believe so. confirmed. 4. confirmed. 5. confirmed. 6. confirmed."*
⚠️ User: live-pass steps belong in the REPLY, not only in a document (*"My markdown reader is horrible and formats data
poorly. Just give me the steps here."*) — given in chat; ⏳ live pass owed.

### 🟠 2026-10-05 — Live pass: 16/18 PASS; steps 12 and 17 FIXED; Format menu moved — ⏳ re-check

✅ User: steps 1–11, 13–16 pass (*"12. passes when the instructions are followed"*; 17 *"technically passes"*). ⚠️ Noted:
*"Cmd-Opt-3 on my computer is currently mapped to Dropbox Screenshot"* — a clash on the user's Mac, shortcut kept (Q2).

1. ✅ **Format menu between Edit and View** (user: *"First put the Format Menu in the list between the Edit and View Menu"*):
   `CommandGroup(replacing: .textFormatting)` — the system Format menu's place. ⛔ A `CommandMenu` lands after View.
   ✅ Test `formatMenuPlacement` reads the RUNNING app's `NSApp.mainMenu` (the tests are hosted in Scrivi).
2. ⛔→✅ **Step 12 — Return at the START of item 1** (caret home after `1. `): ⛔ the prefix was lost (`\n\n2. alpha…`), the caret
   jumped to the end, and the old first item became body text with a literal `2.`. ✅ **Cause (reproduced):** the command's
   edit (correct: an empty item above) went through `shouldChangeText`, where E1's escape-PAIR snap saw an edit starting
   beside a stop run (`1. `) and widened it back over the prefix. ✅ Fix: that snap never touches an edit the presenter
   COMPUTED (`applyingBalancedEdit`) — it also protected every balanced edit. Now `1. ⏎⏎2. alpha⏎⏎3. beta…`, caret at `alpha`.
3. ⛔→✅ **Step 17 — a cross-scene paste made ALL the pasted text bold** (user: *"Only the first word of the pasted section
   should have been bold"*). ✅ Cause: I had pasted text take the caret's style (like typing). ✅ **Fix: Scrivi's OWN paste
   keeps ITS formatting** — in-scene (`ownCopy` → `balancedEdit(keepsOwnFormatting:)`, which rewrites even a marker-free
   paste that lands inside a span) and cross-scene (`balancePastePieces`: the first piece CLOSES the caret's span, the last
   re-opens it). Bold pasted into bold still merges. ⚠️ Typing, and text pasted from OTHER apps (it has no formatting), still
   take the surrounding style — ⏳ to confirm.

✅ Tests **196/196** (+3); mutations bite (snap guard removed → step-12 test red; own-formatting off → step-17 test red).

### 🟠 2026-10-05 — Re-check: 1–4 and 6 PASS; step 5 failed → [I-0279] FOUND, FILED and FIXED

✅ User: *"1. passes. 2. passes. 3. passes. 4. passes. 5. fails. The plain word is bold. Also after I copied the cross scene
data into the clipboard I couldn't copy anything else into the clipboard. All copies after that failed and every paste
pasted the cross scene test from step 4 above."* (+ console log — it did not show the cause; the code did.)
✅ **One defect, both symptoms:** [I-0279] — the cross-scene copy's fragment was held FOREVER (since SP-089), so every ⌘V
pasted it. Step 5's "plain word" paste was really that stale fragment (it began with the bold word). ✅ Fixed
(`StructuredClipboard`, tied to the pasteboard's change count); added to this Sprint. Interop **197/197**.
⏳ Re-check: step 5 again, and the clipboard after a cross-scene copy.

### 🟠 2026-10-06 — [I-0279] re-check PASSES (steps 1–5); new defect at span edges FIXED — ⏳ re-check

✅ User: *"1. passes. 2. passes. 3. passes. … 4. passes. 5. passes."* — the clipboard ([I-0279]) and the own-formatting paste
both pass.
⛔ **Found in the same pass** (user): *"Position the caret at the beginning or ending of the section [and type a space] and the
section disappears and the * or **'s become part of the manuscript … I thought we were going to push or pull the spaces to
outside the attributed sections."* ✅ **Cause:** a typed space is plain typing; the balancing's fast path let it straight in,
leaving `** bold**` / `**bold **` — not emphasis, so the markers showed. (⌫ already went through the balancing.) ✅ **Fix:**
`balancedEdit` also rewrites any edit AT a span's edge (just after an opener, just before a closer); the normaliser moves edge
whitespace outside: a space at the start → ` **bold**`; at the end → `**bold** ` with the caret after the space (outside —
[T-0589] rule 3's "the space after the bold word is plain"). A LETTER at the edge still joins the span (rules 2–3).
✅ Test `edgeWhitespace`; interop **198/198**; mutation (edge rule off) → red.

### ✅ 2026-10-06 — Span-edge re-check PASSES; [T-0592], [T-0584], [T-0591], [I-0279] VERIFIED

✅ User: *"1 through 6 above all pass."* ✅ Every AC met; the live pass is complete. ⏳ Archive with the Sprint close — awaiting the user's approval.
