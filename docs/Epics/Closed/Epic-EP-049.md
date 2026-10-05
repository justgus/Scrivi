# EP-049: `[Linux]` ⚠️ **The Manuscript Storage Format on Linux — the escape layer's WRITE half**

**Status:** ✅ **CLOSED 2026-10-05 (user-approved):** *"close SP-160, run the Audit Check, and close EP-049"* — activated 2026-10-04, created 2026-10-04. ✅ **All ACs met** ([SP-160]). ✅ Audit Check → [`../../Audits/Audit-Check-20261004-EP049.md`](../../Audits/Audit-Check-20261004-EP049.md).
⚠️ **Priority: NOW** (user, 2026-10-04: *"1. now."*). **Sprint:** ✅ **[SP-160]** (CLOSED) → [`../../Sprints/Closed/Sprint-SP-160.md`](../../Sprints/Closed/Sprint-SP-160.md) · **Task:** [T-0586]. Every Linux writing session until it lands stores text in the
pre-escaping format.
**Reference:** Apple's escape layer as SHIPPED by [EP-045] → [`Closed/Epic-EP-045.md`](Epic-EP-045.md), [SP-155]
(AC4), [SP-156] (AC5/AC6, Q1–Q3). ✅ **The maxim (user, 2026-10-04): *"do what Apple does, the way Apple does it."***
**Platform shape (user ruling 2026-10-04):** ✅ implemented in Linux's OWN editor layer (`ManuscriptEditor` /
`SceneDocument`) over ScriviCore, as Apple's lives in its own (`ManuscriptEscapes.swift`, `ManuscriptTextView.swift`).
⛔ Not moved into ScriviCore. ⛔ No third implementation.

**Goal:** ✅ **Linux writes the SAME `.md` that Apple writes for the same keystrokes.** A `*` typed on Linux is
stored as `\*`; a Return stores `\n\n`. ⚠️ **Why it cannot wait:** today Linux stores keystrokes as typed. Once
[EP-046] renders, a Linux-typed `2 * 3 * 4` becomes italics on Apple, and Linux-typed paragraphs (single `\n`)
merge into one.

⛔ **WRITE half only.** ✅ What lands in the file. ⛔ **NOT** how it looks: hiding the backslashes and rendering are
the DISPLAY half, designed for BOTH platforms in [SP-159] (widened 2026-10-04), then built in [EP-048].
⚠️ **Interim, accepted:** Linux shows the escape backslashes (`\*`) until the display half lands (the SP-155
Q-Linux standing, now for Linux-typed text too).

### Acceptance Criteria (each "as Apple does")

| # | Criterion | Apple reference |
| - | --------- | --------------- |
| **AC1** | **Typed escaping:** each of the 32 ASCII punctuation marks typed is stored with a backslash | `MarkdownEscapes.escape`, `insertText` (SP-155 AC4b) |
| **AC2** | **Paste escapes** foreign text (R1); ⚠️ **Scrivi's OWN copy pasted back is restored EXACTLY** (not double-escaped; intended `##` stays markup) | `readSelection` + `ownCopy` (SP-155 AC4c) |
| **AC3** | **Copy / cut put the writer's text on the clipboard** — un-escaped | `writeSelection` (SP-155 AC4e) |
| **AC4** | **An escape pair is deleted as ONE unit** by Backspace, Delete, word/line deletes and cut — never an orphaned `\` or a bare live mark | `shouldChangeText` pair-widening (SP-155 AC4d) |
| **AC5** | **Return stores `\n\n`**, first trimming trailing spaces to at most one and collapsing a typed trailing `\\` to `\`. ⚠️ **Shift-Return and keypad Enter the same** (Qt's default Shift-Return inserts U+2028 — MEASURE). Option/Alt-Return: as [T-0584] rules | `insertNewline` (SP-156 AC5a/AC6) |
| **AC6** | **Backspace at a paragraph start JOINS with one space** (Q3), except after a bare trailing `\` or beside an empty paragraph | `paragraphJoin` (SP-156 Q3) |
| **AC4b** | **An insertion never lands between `\` and its mark** — ⚠️ found at SP-160 planning: Linux SHOWS the backslash, so the caret can rest inside a pair | Apple's caret snap (`presentedToSource`) |
| **AC7** | **Existing text is NEVER rewritten** (R2); untouched scene bytes round-trip unchanged | EP-045 R2, AC9 |
| **AC8** | ✅ **Byte-identical to Apple:** the same scripted keystrokes produce the same `.md` on both platforms, checked against a SHARED corpus of input → stored cases that both platforms' tests read | the AC3/AC4 test corpora |
| **AC-live** | The user types and pastes punctuation, Return and Backspace on the rig; the scene file holds Apple's format | — |

⚠️ **Undo:** Linux has NO undo yet (`ManuscriptEditor.cpp:25`, deferred to EP-026), so Apple's "one gesture, one undo
step" criteria have no Linux target. ✅ Each gesture is still ONE document edit (one `QTextCursor` edit block), so
that EP-026 inherits the right shape.
⚠️ **AC8's shared corpus is TEST DATA, not a third implementation** — offered as the drift guard. Remove it if the
user rules it unwanted.

⛔ **OUT:** hiding backslashes / any rendering (→ [SP-159] design → [EP-048]) · undo (→ EP-026) · the
scene-boundary table (Linux already has it) · Find/Replace ([T-0585]).

### ✅ AC status at close (2026-10-05) — Audit Check F-2

- ✅ **AC1–AC8 + AC4b — MET 2026-10-04** ([T-0586], user-verified on the rig: *"All five checks pass."*). ✅ AC8's shared
  corpus (`ScriviCore/tests/fixtures/manuscript_format_corpus.json`, 18 cases) passes on BOTH platforms; a mutation
  fails both.
- ✅ **AC-live — MET 2026-10-04** (rig, build 55).
- ✅ **[I-0276] — added in scope** (found in SP-160's live pass; linked by the user's standing rule): the Linux launch
  window opened a project on a single click → click selects, double-click / Return opens. ✅ Verified on the rig,
  build 57.
- ✅ **Fixed in scope:** Shift-Return wrote U+2028 into scene files; Ctrl+Delete bypassed the boundary guard.

## ✅ Close — 2026-10-05 (user-approved)

✅ User: *"close SP-160, run the Audit Check, and close EP-049"*
✅ **Delivered:** Linux writes the same `.md` that Apple writes for the same gestures — escaping, paste/copy, pair
deletion, Return/⌫ — checked by one corpus on both platforms.
⚠️ **Carried forward, NOT closed by this Epic:** Linux still SHOWS the escape backslashes (the display half →
[SP-159] design → [EP-048]); Alt-Return stores Apple's single `\n` until [T-0584] rules; Linux undo (EP-026).

