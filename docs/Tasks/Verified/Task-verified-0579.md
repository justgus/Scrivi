# Verified Task — T-0579

**Sprint:** [SP-155] · **Epic:** [EP-045] (AC4) · **Platform:** `[Apple]`
**Implemented:** 2026-10-03 · ✅ **USER-VERIFIED 2026-10-03 by live check:** *"live check passes, close SP-155"* —
after the first live check found the copy/paste type defect and it was fixed.
**Archived:** 2026-10-03, with the Sprint close.

---

## 🟠 T-0579 — `[Apple]` The escape layer ([EP-045] AC4) — Implemented 2026-10-03 · ✅ **VERIFIED 2026-10-03 (user)**

✅ **Sprint:** [SP-155] · ✅ R1 (paste escaped), R2 (no escape pass), R3 = (c), Q-Linux = option 1.
✅ **Files:** `ManuscriptTextView.swift` (`EscapeHidingStyler`; `ManuscriptNSTextView` `insertText` /
`readSelection` / `writeSelection` / `insertVerbatim`; pair-widening in `shouldChangeText`; presented text in the
cross-scene copy), `ManuscriptEscapes.swift` (`hiddenBackslashes`), `BufferService.swift` (preview).
✅ **Tests:** suite "Escape layer (EP-045 AC4)" — typing escapes + hides; ⌫/⌦ remove the whole pair; own copy round
trip not double-escaped; existing `##` survives an in-app copy→paste; foreign paste escaped; a broken escape is
un-hidden; all 32 typed → stored → parsed (`.full`) → equal. ✅ 152/152.
⛔ **USER-FOUND DEFECT, 2026-10-03 (first live check) — FIXED:** *"So apparently the backslash strip only works for text
pasted and not typed. That makes no sense."* ✅ **The scene file showed it exactly:** the typed line was stored
correctly escaped; the line pasted back from a text editor was stored with NO escapes, and ⌘C had put the stored
backslashes on the clipboard. ⛔ **Cause, measured:** AppKit calls `writeSelection(to:type:)` /
`readSelection(from:type:)` with the LEGACY type `NSStringPboardType`, which is NOT equal to `.string` — so both
overrides fell through to AppKit. ⛔ **The tests passed because they named `.string` themselves.** ✅ **Fix:** accept
either spelling; reduce rich text / RTFD / HTML pastes to plain text and escape them too. ✅ Tests now go through
AppKit's own type dispatch (`readSelection(from:)`) and the type AppKit actually passes, plus a TextEdit-style
(RTF + plain) paste test. ✅ **RED with the old check (4 failures), GREEN with the fix — 153/153.**
⚠️ The test line pasted during the live check remains in the `dumas-prose-timelines` fixture UNESCAPED (R2: never
rewritten) — it is test data.

⚠️ **Live check owed:** type `*`, `_`, `#` and `\` — see them exactly as typed; ⌘C → paste into TextEdit (no
backslashes); paste from TextEdit; ⌫ over a mark; **⌘Z after typing a mark** (one step, no stray backslash);
the copy-buffer palette preview. ⚠️ **Linux will show the backslashes** (ruled, option 1).
