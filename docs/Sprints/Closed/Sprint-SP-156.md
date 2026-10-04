---
sprint: SP-156
epic: EP-045
status: Closed
closed: 2026-10-04
activated: 2026-10-04
platform: Apple
created: 2026-10-04
---

# SP-156 — `[Apple]` [EP-045] S4: Enter, Backspace and trailing whitespace (AC5 + AC6)

**Status:** ✅ **CLOSED 2026-10-04 (user-approved):** *"yes, close I-0274 as not a defect. Archive T-0580. Close SP-156."* Live check passed; [T-0580] verified and archived → [`../../Tasks/Verified/Task-verified-0580.md`](../../Tasks/Verified/Task-verified-0580.md).
**Task:** [T-0580] → [`../../Tasks/Verified/Task-verified-0580.md`](../../Tasks/Verified/Task-verified-0580.md)
**Epic:** [EP-045] → [`../../Epics/Closed/Epic-EP-045.md`](../../Epics/Closed/Epic-EP-045.md)
**Design:** [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md) §6
**Authority:** [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §3A.6 (Enter/Backspace ruling), §4B.5–§4B.6 (trailing-space ruling + amendment)
**Size:** ⚠️ **SMALL–MEDIUM.** One keystroke path, but it changes what the writer's Return key stores, and it touches undo.

---

## What AC5 and AC6 are

✅ **Return starts a new paragraph.** In Markdown a paragraph break is a blank line, so Return stores `\n\n`
(study §3A.6, user ruling). ✅ **Backspace at the start of a paragraph deletes ONE `\n`**, so the paragraph
merges upward as a soft break. A second Backspace closes the soft break up. The asymmetry is deliberate and was
ruled.

✅ **Before inserting the break, Return reduces the trailing spaces on the line to AT MOST ONE** (§4B.6, amended
from "delete one": CommonMark makes two OR MORE trailing spaces a hard break). ✅ **A trailing typed backslash
(stored `\\`) collapses to `\`**, so that a later merge leaves a deliberate hard break where the writer typed it.

⛔ **The whole Return gesture is ONE undo step** (design §6.3). It can be up to three changes (trim spaces,
collapse a backslash, insert the break). If they split, Undo after Return restores half of it.

---

## ⚠️ Measured at planning (2026-10-04)

### The parser: what each stored form presents (`AttributedString(markdown:)`, `.full`, macOS 27.2)

| Stored | Presented | Hard break? |
| ------ | --------- | ----------- |
| `end.\` + `\n\n` + `next.` (AC6's Return result) | `end.\` · `next.` (two paragraphs, **backslash LITERAL**) | no |
| `end.\\` + `\n\n` + `next.` (no collapse) | `end.\` · `next.` | no |
| `end.\` + `\n` + `next.` (after a Backspace merge) | `end.⏎next.` | ✅ **yes** |
| `end.\\` + `\n` + `next.` (merge, no collapse) | `end.\ next.` | no |
| `end.␣` + `\n` + `next.` | `end. next.` | no |
| `end.␣␣` + `\n` + `next.` | `end.⏎next.` | ✅ **yes** |

✅ The last three rows reconfirm §4B.5–§4B.6: one trailing space is safe, two is a hard break, and the collapsed
backslash produces the deliberate break after the merge.

⛔ **The first row conflicts with the escape map.** CommonMark keeps a backslash LITERAL at the end of a paragraph
(a hard break cannot end a block). `MarkdownEscapes.hidesBackslash` (`ManuscriptEscapes.swift:41-43`) hides ANY
backslash followed by `\n`, including one followed by a blank line. So after Return the map and the parser
disagree: under R3 = (c) the screen would HIDE the `\`, while the file, an export, and Linux all show it.
⚠️ AC4 never hit this because typing cannot produce a bare `\` before a newline. **AC6 is the first thing that
does.** → **Q1.**

### The edit path (read in the code, 2026-10-04)

- ⚠️ **Return is not overridden.** `ManuscriptNSTextView` overrides `insertText`, `shouldChangeText`,
  `deleteBackward` and `deleteForward` (`ManuscriptTextView.swift:2415`, `:2501`, `:2777`, `:2791`), but not
  `insertNewline(_:)`. Return reaches storage through AppKit's default path, as a single `\n`.
- ⚠️ **Return already commits a history event.** `isCommitBoundary` (`:337-344`) treats `\n` as a sentence
  terminator, and `textDidChange` flushes on it (`:954-958`). ⛔ **If Return is built as several storage edits,
  the FIRST `\n` commits an event and the rest land in the next one.** ✅ So the whole gesture must be ONE
  `replaceCharacters` call (trailing run + selection → normalised tail + `\n\n`), which gives one
  `textDidChange` and one flush.
- ✅ **Backspace already deletes one character**, so AC5's Backspace half needs no new behaviour. A test pins it.
  ⚠️ AC4's pair-widening (`MarkdownEscapes.snapSelection`, `ManuscriptEscapes.swift:131`) still applies. After a
  merge to `end.\` + `\n` + `next.`, a SECOND Backspace is widened to remove the `\` and the `\n` together. That
  is the expected result (the deliberate break and its marker go together), and the live pass confirms it.

---

## ✅ The rulings — Q1 RULED (a) · Q2 RULED YES · Q3 RULED (all 2026-10-04)

| # | ⚠️ Question | ⚠️ Recommendation |
| - | ----------- | ----------------- |
| **Q1** | ⛔ **Where does the collapsed `\` show?** §4B.6's ruling expected Return + `\` to *"remove it from the rendering"*. Measured: at the end of a paragraph the parser keeps it LITERAL. It only becomes a hidden hard break once a Backspace merges the paragraphs. **(a)** Follow the parser: fix `hidesBackslash` so a `\` before a blank line (or at the end of the fragment) is NOT hidden; the writer sees `end.\` until she merges. **(b)** Keep hiding it; the screen then shows something the file, an export and Linux do not. **(c)** Do not collapse on Return; collapse `\\` → `\` on the Backspace merge instead. | ✅ **RULED (a) 2026-10-04** (user: *"For Q1 I agree. A is correct."*). The map's only job is to agree with the parser (the AC3 oracle), and (a) keeps AC6's stored form exactly as ruled. (b) creates a screen/file disagreement. (c) moves the rule off the keystroke the user ruled on. |
| **Q2** | ⛔ **Normalise on the Backspace merge too?** §4B.6 left this open: a writer who presses Return FIRST and later adds two spaces at the end of the previous paragraph gets a hard break on merge, and nothing catches it. The merge is where the damage appears. | ✅ **Yes:** a Backspace that deletes a `\n` and leaves `≥2 spaces + \n` also reduces those spaces to one, in the SAME undo step. ⚠️ This goes beyond the literal *"Backspace deletes one `\n`"*, which is why it needs a ruling. ⚠️ **If unruled, the Sprint ships Return-only, exactly as AC6 is written.** ✅ **RULED YES 2026-10-04** (user: *"Fir Q2: I approve your recommendation."*) — ✅ implemented, then ⚠️ **SUBSUMED by Q3** (a join leaves no newline, and the space run becomes exactly one). |

---

## Acceptance Criteria

- [x] **AC5a — Return inserts `\n\n`.** `insertNewline(_:)` is overridden on `ManuscriptNSTextView`. Return with a
  selection replaces the selection. ✅ The existing guards still apply (no edit inside a heading, never across a
  divider), because the edit goes through `shouldChangeText`.
- [x] **AC5b — Backspace at a paragraph start — ⚠️ AMENDED BY Q3:** JOINS with ONE space (`a.⏎⏎b.` → `a. b.`), one edit;
  ⛔ one `\n` only after a bare trailing `\` (deliberate break) or when either paragraph is empty. ✅ Q2 subsumed.
  ✅ Tests `backspaceJoinsWithSpace`, `backspaceOneCharacterCases`, `backslashBreakAfterMerge`. ⚠️ Live re-check owed.
- [x] **AC6a — Trailing spaces reduced to AT MOST ONE on Return.** Design AC6's test: `"x␣␣␣"` + Return →
  `"x␣\n\n"`. Also `"x␣"` → `"x␣\n\n"` (unchanged) and `"x"` → `"x\n\n"`. Only the run BEFORE the caret/selection
  is touched.
- [x] **AC6b — A trailing escaped backslash collapses on Return.** `end.\\` + Return → `end.\` + `\n\n`. Only the
  FINAL pair collapses: `\\\\` + Return → `\\\` + `\n\n` (one literal `\`, then the break marker). A trailing
  `\*` (an escaped mark) is untouched.
- [x] **AC6c — The escape map agrees with the parser at a paragraph end (per Q1).** If (a): a `\` before
  `\n\n` or at the end of a fragment is NOT hidden; one before a single `\n` still is. Added to the AC3 corpus
  test against the `.full` oracle.
- [x] **AC-undo — The Return gesture is ONE undo step** (✅ mechanism proven by the suite; ✅ live ⌘Z PASSED 2026-10-04) ([EP-019], design §6.3). After `"x␣␣␣"` + Return, ONE ⌘Z
  restores `"x␣␣␣"` and the caret exactly, and ONE ⌘⇧Z re-applies `"x␣\n\n"`. Proven by the history event count
  through the real `textDidChange` → `HistoryCapture` path, not a mock.
  ⚠️ **AMENDED 2026-10-04, two corrections found in the code:** (1) ⛔ *"restores `x␣␣␣` exactly"* was WRONG — trailing
  whitespace is never its own step (`HistoryCapture.flush`, `:266-284`), so the typed spaces ride in the SAME event as
  the Return and one ⌘Z removes both. ✅ That is EP-019's existing behaviour, unchanged; the criterion is that no
  ⌘Z ever leaves HALF a Return. (2) ⚠️ No test can build the coordinator + `HistoryCapture` without a full
  `ProjectSession`, so the suite proves the MECHANISM — one Return = one `textDidChange` (the coordinator records
  one event per `textDidChange`, `:950-958`) — and ✅ **the live ⌘Z check is the end-to-end proof.**
- [x] **AC-measure — The other Return-family keys are MEASURED and RECORDED, not changed.** Shift-Return,
  Option-Return and keypad Enter: which action each sends and what each stores today. ⚠️ Any change to them needs
  its own ruling.
- [x] **AC-build — macOS + iOS + visionOS build; interop suite green** via `scripts/run-interop-tests.sh`. Tests
  drive the REAL `ManuscriptNSTextView` and AppKit's own key dispatch for Return
  (`feedback_test_through_the_real_dispatch`: T-0579's tests passed while AppKit took another route).
- [x] **AC-live — The user's live pass:** Return after a sentence ending in two spaces (one paragraph break on
  screen; the scene file holds one space + `\n\n`); Backspace at a paragraph start (merges, no hard break);
  ⌘Z after Return (one step); `\` + Return, then Backspace to merge (a hard break where the `\` was).

⛔ **NOT in this Sprint:** AC7, AC10. ⛔ Newlines arriving by PASTE are not converted (AC5 is the keystroke). ⛔ No
rewrite of existing scene files (R2): single-`\n` paragraphs already on disk stay as they are. ⛔ Linux (→ [EP-048]).

---

## ⚠️ Known consequences, recorded so they are not filed as defects

1. ⚠️ **Linux still inserts a single `\n` on Return** ([EP-048], same standing as Q-Linux in [SP-155]). On disk, a
   paragraph typed on Linux is a soft break. Under R3 = (c) the Mac screen shows storage, so nothing changes visibly
   today. ⚠️ **It WILL matter when [EP-046] renders paragraphs.** The real project (`the-stairs-of-tintagael`) is
   edited on both.
2. ⚠️ **Return on an already blank line stores `\n\n\n\n`.** CommonMark treats it as one paragraph break, while the
   screen (storage) shows the larger gap. Recorded, not ruled. Raise it if the live pass finds it jarring.
3. ⚠️ **Find/Replace across a hidden backslash** is still the [SP-155] follow-up. Not touched here.

---

## Progress log

### 🔵 2026-10-04 — Sprint CREATED in Planning

✅ Drafted at the user's request: *"Please draft the Sprint for AC5 and AC6."* ✅ The ID was issued by
`next-id.py`. ✅ The parser was measured first (table above, `scratchpad/sp156/probe.swift`, throwaway), and it
found the Q1 conflict between AC6's stored form and the AC4 escape map. ⚠️ The Task ID is allocated at
activation.

### 🟡 2026-10-04 — Q1 ruled (a); Sprint ACTIVATED; [T-0580] allocated

✅ User: *"For Q1 I agree. A is correct. the plan is approved. you may activate and implement SP-156"*. ⚠️ Q2 was not
ruled, so the Sprint ships Return-only, as the plan said it would.

### 🟢 2026-10-04 — AC5 + AC6 implemented ([T-0580]); ⚠️ AC-live needs the user

✅ **Measured first, with a second `.full` vs inline-only probe:** the inline-only oracle (used by the AC3 suite)
HIDES `end.\⏎⏎`, while `.full` keeps it literal — ✅ Q1 = (a) follows `.full`, the ruled mode (AC8). ⚠️ **And the
styler maps ONE LINE at a time** (`restyle`), so the map could not see whether the next line is blank, ⛔ and a ⌫
merge restyled only the caret's line, not the line holding the `\`. ✅ Both fixed: the map takes `continues:`; the
styler supplies it (blank line / divider / heading / end = not continuing) and restyles the line above.
✅ **AC-measure — what each Return-family key sends (AppKit's own dispatch):**

| Key | Action | Stored before | After SP-156 |
| --- | ------ | ------------- | ------------ |
| Return · keypad Enter | `insertNewline:` | `\n` | ✅ `\n\n` (AC5) |
| ⚠️ **Shift-Return** | ⚠️ **`insertNewline:` — the SAME action** | `\n` | ⚠️ `\n\n` — cannot differ without reading modifiers, which would itself be a ruling |
| Option-Return | `insertNewlineIgnoringFieldEditor:` | `\n` | unchanged `\n` (no ruling) |
| ⚠️ **Control-Return** | ⛔ ~~`insertLineBreak:` → U+2028~~ **MIS-MEASURED** — ✅ **in the app: the system context menu** (user, 2026-10-04); never reaches the text view | unchanged → [I-0274] filed on the wrong premise, ⚠️ close recommended |
| ⌘-Return | ⛔ ~~`noop:`~~ **MIS-MEASURED** — ✅ **in the app: Split Chapter** (menu key equivalent, user 2026-10-04) | unchanged |

✅ **Built:** `insertNewline(_:)` as ONE `insertVerbatim` replacement (trim + collapse + `\n\n`), reading backward from the
caret — ⛔ the first draft copied the document up to the caret (EP-045 trap #2) and was rewritten before testing.
✅ **Tests:** 7 through `keyDown` + one `.full` oracle test. ✅ **RED without the production change (6 tests, 28
issues); GREEN with it: 161/161** (`scripts/run-interop-tests.sh`). ✅ macOS / iOS / visionOS BUILD SUCCEEDED.
⚠️ **AC-live owed** — the four checks above, plus ⌘Z after Return (one step).

### ⚠️ 2026-10-04 — live check: 1 finding · 2 FAILED · 3 passed · 4 half-checked; Q2 ruled and implemented

✅ **The user's results:**
1. ⚠️ **Finding:** *"One end space was stripped and \n\n was entered and recorded. As we're not actually rendering
   the Markdown yet the Manuscript View shows both newlines."* ✅ Stored form correct. ⚠️ The blank line is VISIBLE
   because E1 shows storage (R3 = (c)) — ✅ recorded as a known E1 consequence until [EP-046] renders paragraphs.
2. ⛔ **FAILED:** *"Only one \n was removed."* ✅ That IS the ruled behaviour — ⛔ **but the ruling's premise does not
   hold in E1.** §3A.6 ruled one `\n` because *"the single \n is rendered as whitespace"*; ⛔ **E1 renders nothing**,
   so a soft break SHOWS as a line break and the paragraphs never visibly join. ⛔ **My live-check wording (*"merges,
   no line break"*) promised what E1 cannot show — that hid the gap.** → ⚠️ **Q3, owed to the user.**
3. ✅ **PASSED:** ⌘Z after Return is one step → **AC-undo MET.**
4. ⚠️ *"The end-of line character is a single backslash but it remains shown."* ✅ After Return that is Q1 = (a) as
   ruled (a `\` before a blank line is literal). ⚠️ **The second half — ⌫ to merge, then the `\` should HIDE — was
   not reported**; still owed.

✅ **Q2 RULED YES** (user: *"Fir Q2: I approve your recommendation."*) → `deleteBackward`: when a ⌫ deletes a `\n`
and leaves another `\n` preceded by ≥2 spaces, the surplus spaces go in the SAME single replacement. ✅ Test
`mergeNormalisesSpaces` (real key dispatch; includes §4B.6's inverse route — spaces added after the Return):
✅ **RED with the trim disabled (3 issues), GREEN with it — 162/162.**

### ✅ 2026-10-04 — Q3 RULED (join with a space) and implemented

⚠️ **Q3** — *until [EP-046] renders, what does ONE ⌫ at a paragraph start do?* ✅ **User: "Join with a space"**
(the recommended option). ✅ **`paragraphJoin(at:in:)` + `deleteBackward`:** the trailing spaces and the `\n\n` are
replaced by ONE space in one edit; ⛔ one `\n` only when the line above ends in a BARE `\` (odd backslash run — the
deliberate break survives and is hidden) or either paragraph is empty. ✅ **Q2 is subsumed** — the Q2-only code and
its test `mergeNormalisesSpaces` were REPLACED, not kept alongside. ✅ Recorded in design §6.1 and EP-045 AC5 as an
amendment of the §3A.6 ruling.
✅ **RED with the join disabled (5 issues), GREEN with it: 162/162.** ✅ macOS / iOS / visionOS BUILD SUCCEEDED.
⚠️ **Live re-check owed:** (2) ⌫ at a paragraph start → one paragraph, joined by a space; (4) `\` + Return, then ⌫ →
the `\` HIDES and the line break stays.

### ✅ 2026-10-04 — live re-check PASSES; [T-0580] VERIFIED; Sprint COMPLETE (awaiting close approval)

✅ User: *"live check passes."* → **AC-live MET**; ✅ [T-0580] marked Verified (archived with the close).
✅ **EP-045 AC5 and AC6 MET.**

⛔ **AC-measure WAS WRONG FOR TWO KEYS — the user found it.** ✅ *"Ctrl-Enter on Macos brings up a context menu (Ask
Siri, Cut, Copy Paste, etc). Cmd-Enter on Macos will Split the Chapter."* ⛔ **Cause:** the harness called
`NSTextView.keyDown(with:)` DIRECTLY, which skips `NSApplication`'s key-equivalent pass (menus — ⌘-Return is Split
Chapter) and macOS's system shortcuts (Control-Return opens the context menu). ⚠️ **The same class as [T-0579]'s
`.string` defect: the test chose the dispatch, AppKit did not.** ✅ The Return, keypad Enter, Shift-Return and
Option-Return rows reach the view as plain keys and the live check agrees with them; ✅ the user confirmed
Option-Return stores a single `0x0A`.
⚠️ **[I-0274] was filed on the Control-Return mis-measurement** → ✅ **recommended: CLOSE as not-a-defect** (no
default key reaches `insertLineBreak:`); awaiting the user.
⚠️ **Follow-up, not ruled:** Option-Return stores ONE `\n` — a soft break that shows as a line break today but will
render as a SPACE under [EP-046]. Options: treat as Return (`\n\n`), make it a deliberate hard break (`\` + `\n`),
or leave it.

### ✅ 2026-10-04 — Sprint CLOSED (user-approved); [T-0580] archived; [I-0274] closed

✅ User: *"yes, close I-0274 as not a defect. Archive T-0580. Close SP-156."* ✅ [T-0580] →
`../../Tasks/Verified/Task-verified-0580.md`. ⚪ [I-0274] → `../../Issues/Closed/Issue-closed-0274.md` (not a defect).

## Retrospective

**Completed:** ✅ AC5 + AC6 ([T-0580]): Return stores `\n\n` with trailing spaces trimmed and a trailing typed `\`
collapsed; ⌫ at a paragraph start joins with one space (Q3); the escape map agrees with `.full` at a paragraph end
(Q1 = (a)). ✅ Four rulings: Q1, Q2 (subsumed by Q3), Q3.
**Returned to Backlog:** none. ⚠️ **Unruled follow-up:** Option-Return stores ONE `\n` (renders as a space under [EP-046]).
**What went well:** ✅ the parser was measured BEFORE drafting, which found Q1; ✅ every behaviour test went red
without its code.
**What to improve:** ⛔ **my live-check wording promised what E1 cannot show** ("merges, no line break") — it hid the
Q3 gap until the user ran it. ⛔ **The Return-family measurement skipped menu and system shortcuts** (direct
`keyDown`) and produced a false Issue ([I-0274]). ✅ Write live checks as what the SCREEN will show, given R3 = (c).
**Carry-forward notes:** ⚠️ E1 shows STORAGE: a paragraph break is a visible blank line, and the Q1 `\` before a blank
line is visible, until [EP-046] renders.

