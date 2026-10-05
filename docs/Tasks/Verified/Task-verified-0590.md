# Verified Task — T-0590

**Sprint:** [SP-162] · **Epic:** [EP-046] (E2-S2: AC3, AC4 span half + Q1 amendment, AC7 inline half, AC12; AC5 + AC8 re-checked) · **Platform:** `[Apple]`
**Implemented:** 2026-10-05 · ✅ **USER-VERIFIED 2026-10-05 by live pass** (decisions 1–3 confirmed; two amendments re-checked): *"Yes, verify T-0590 and close SP-162"*
**Archived:** 2026-10-05, with the Sprint close.

---

## ✅ T-0590 — `[Apple]` E2-S2: bold and italic + span reveal + the split rule — ✅ **VERIFIED 2026-10-05 (user)**

**Created:** 2026-10-05 at [SP-162]'s activation (user: *"activate SP-162"*) · **Epic:** **[EP-046]** ·
**Sprint:** 🟡 **[SP-162]** → [`../Sprints/Sprint-SP-162.md`](../../Sprints/Closed/Sprint-SP-162.md)
✅ Carries SP-162's work: EP-046 **AC3** (bold/italic render), **AC4** span half + the Q1 amendment (a caret arriving at the
start of a span or a heading lands AFTER the opening marker / `## `, hint to its left), **AC7** inline half, **AC12** (a
selection that partly covers a span is SPLIT and balanced — cut, copy, and per Q3 every selection replacement), presented
copy with markers removed (Q2); AC5 + AC8 re-checked. Design → [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md).
🟠 **2026-10-05 — IMPLEMENTED - NOT VERIFIED.** ✅ `MarkdownBlocks.swift` (inline analysis), `MarkdownEmphasis.swift` (new — the
balancing model), `ManuscriptPresenter.swift`, `ManuscriptEscapes.swift` (role-aware snap), `ManuscriptTextView.swift`;
`project.pbxproj` ×3. ✅ Interop 183/183; mutations bite; iOS + visionOS build; AC8 met (rebuild 6.55 → 4.40 ms with
emphasis; balancing check 0.03 ms/keystroke). ⚠️ Three implementation decisions to confirm, and the live pass →
[`../Sprints/Sprint-SP-162.md`](../../Sprints/Closed/Sprint-SP-162.md) progress log.
✅ **2026-10-05 — Live pass PASSED; decisions 1–3 CONFIRMED (user).** ✅ Two amendments implemented after it (Markup Hints only at a
section's first/last character; bold = a real Bold weight, Heavy inside headings) — 184/184. ✅ Amendments re-checked (bold weight accepted; nested reveal kept). ✅ **VERIFIED:** *"Yes, verify T-0590 and close SP-162"*
