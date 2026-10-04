# Active Epics

## EP-049: `[Linux]` ⚠️ **The Manuscript Storage Format on Linux — the escape layer's WRITE half**

**Status:** 🟡 **ACTIVE 2026-10-04** (user: *"activate SP-160 and implement it"*) — created 2026-10-04 (user: *"yes, draft the Epic and widen SP-159"*).
⚠️ **Priority: NOW** (user, 2026-10-04: *"1. now."*). **Sprint:** 🟡 **[SP-160]** (ACTIVE) → [`../Sprints/Sprint-SP-160.md`](../Sprints/Sprint-SP-160.md) · **Task:** [T-0586]. Every Linux writing session until it lands stores text in the
pre-escaping format.
**Reference:** Apple's escape layer as SHIPPED by [EP-045] → [`Closed/Epic-EP-045.md`](Closed/Epic-EP-045.md), [SP-155]
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

---

## ✅ **[EP-045]** — `[Apple]` **The Manuscript Renderer — Foundations** — **CLOSED 2026-10-04 (user-approved)**

→ [`Closed/Epic-EP-045.md`](Closed/Epic-EP-045.md). ✅ **Six Sprints: [SP-153] · [SP-154] · [SP-155] · [SP-156] ·
[SP-157] · [SP-158]**, all closed. ✅ **All eleven ACs met.** ✅ Audit Check → [`../Audits/Audit-Check-20261004.md`](../Audits/Audit-Check-20261004.md).
⚠️ **Carried forward:** block-intent drawing → [EP-046]; Linux parity → [EP-048]; [I-0275] (Issue backlog).
⛔ **No Epic is active.**
---

## ✅ **[EP-043]** — `[Linux]` **The Session** — **CLOSED 2026-09-30 (user-approved)**

→ [`Closed/Epic-EP-043.md`](Closed/Epic-EP-043.md). ✅ **Four Sprints: [SP-145] · [SP-146] · [SP-147] ·
[SP-148]**, all closed. ✅ **All ten ACs met** (R1–R8, AC-build, AC-live) — ✅ **[I-0176], [I-0177], [I-0178]
VERIFIED**: Linux opens many projects, one window each, reopening where the writer left them (size,
maximized, splitters). ⚠️ **Window POSITION is ruled out on Wayland** ([I-0264]).
⚠️ **Carried forward, NOT closed by this Epic:** [I-0255] (Timeline visibility persistence, `[Cross]`);
[I-0244] (the Linux shell gap — its SIBLING Epic, sequenced after this one per [R-Q5]); the untested
desktop-logout path; Landing staying up beside restored windows (a parity question — Apple dismisses its
Welcome).

---

