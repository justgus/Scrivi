# Verified Task — T-0580

**Sprint:** [SP-156] · **Epic:** [EP-045] (AC5 + AC6) · **Platform:** `[Apple]`
**Implemented:** 2026-10-04 · ✅ **USER-VERIFIED 2026-10-04 by live check:** *"live check passes."* — after the first
live check found that a single `\n` shows as a line break in E1, which led to ruling Q3 (⌫ joins with a space).
**Archived:** 2026-10-04, with the Sprint close.

---

## ✅ T-0580 — `[Apple]` Return, Backspace and trailing whitespace ([EP-045] AC5 + AC6) — Implemented 2026-10-04 · ✅ **VERIFIED 2026-10-04 (user):** *"live check passes."*

**Sprint:** [SP-156] → [`../../Sprints/Closed/Sprint-SP-156.md`](../../Sprints/Closed/Sprint-SP-156.md) · **Epic:** [EP-045] · ✅ Q1 = (a) (user, 2026-10-04); ✅ Q2 ruled YES, then SUBSUMED by ✅ **Q3 (ruled 2026-10-04): ⌫ at a paragraph start JOINS with one space** (`paragraphJoin(at:in:)`; one `\n` only after a bare `\` or beside an empty paragraph). Red→green, 162/162.
✅ **Files:** `ManuscriptTextView.swift` — `ManuscriptNSTextView.insertNewline(_:)` (one replacement: trim trailing
spaces to ≤1, collapse a final `\\` pair to `\`, insert `\n\n`; reads backward from the caret, never copies the
document); `EscapeHidingStyler.restyle` restyles the line ABOVE the edit too, and `lineContinuesParagraph` tells
the map whether the next line continues the paragraph (a blank line, divider or heading does not).
`ManuscriptEscapes.swift` — `hidesBackslash(_:at:continues:)`: a `\` before a blank line or the end stays literal
(Q1 = (a), measured under `.full`); `isBlankLine`; `map` / `hiddenBackslashes` take `continues:`.
✅ **Tests:** new suite "Return and Backspace (EP-045 AC5/AC6)" — 7 tests through AppKit's own `keyDown` dispatch;
`paragraphEndBackslash` in the AC3 suite against the `.full` oracle (the `"a\\\n"` edge case moved there from the
inline-only list). ✅ **RED without the change** (6 tests, 28 issues), ✅ **GREEN with it: 161/161.** ✅ macOS / iOS /
visionOS BUILD SUCCEEDED.
⚠️ **Live check 2026-10-04:** ✅ undo passed; ⛔ ⌫ at a paragraph start FAILED visually (E1 showed the soft break as a line break) → ✅ Q3 ruled + implemented; ⚠️ re-check owed, plus the merge half of the `\` check.
