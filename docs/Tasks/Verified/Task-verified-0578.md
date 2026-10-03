# Verified Task — T-0578

**Sprint:** [SP-154] · **Epic:** [EP-045] (AC3) · **Platform:** `[Apple]`
**Implemented:** 2026-10-03 · ✅ **USER-VERIFIED 2026-10-03 by live check:** *"passes"* (arrows, clicks and selection behave as before).
**Archived:** 2026-10-03.

---

## 🟠 T-0578 — `[Apple]` Source↔presented offsets + hidden-escape caret snap ([EP-045] AC3) — Implemented 2026-10-03 · ✅ **VERIFIED 2026-10-03 (user)**

✅ **Sprint:** [SP-154] · **Epic:** [EP-045] · ✅ **R3 = (c)** ruled 2026-10-03.
✅ `Scrivi/Views/ManuscriptEscapes.swift` (new, all three targets): `MarkdownEscapes.map` (UTF-16 boundary maps;
hard-line-break backslash handled), `escape` (write half, not yet wired — AC4), `hiddenKey`, `snapCaret`,
`snapSelection`. ✅ `ManuscriptTextView.setSelectedRanges` calls the snaps (inert until AC4 hides anything).
✅ Tests: 2,000-string corpus vs Apple's parser; snap rules. ✅ 145/145. ⚠️ **Nothing writer-visible changes in this
Task** — the live check is that caret, arrows, clicks and selection behave exactly as before.
