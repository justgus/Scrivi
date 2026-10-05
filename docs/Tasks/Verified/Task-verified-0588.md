# Verified Task — T-0588

**Sprint:** [SP-161] · **Epic:** [EP-046] (E2-S1: AC1, AC2, AC4 line half, AC5, AC7, AC8, AC11) · **Platform:** `[Apple]`
**Implemented:** 2026-10-05 · ✅ **USER-VERIFIED 2026-10-05 by live pass:** *"All steps in the Live pass passed.  We can Verify T-0588.  If available, you are also authroized to close SP-161."*
**Archived:** 2026-10-05, with the Sprint close.

---

## ✅ T-0588 — `[Apple]` E2-S1: the presenter (route (a′)) + headings — ✅ **VERIFIED 2026-10-05 (user)**

**Created:** 2026-10-05 at [SP-161]'s activation (user: *"activate SP-161 and approve Q1 and Q2"*) · **Epic:** **[EP-046]** ·
**Sprint:** 🟡 **[SP-161]** → [`../Sprints/Sprint-SP-161.md`](../../Sprints/Closed/Sprint-SP-161.md)
✅ Carries SP-161's work: EP-046 **AC1, AC2, AC4 (line half), AC5, AC7, AC8, AC11**. That means an `NSTextContentStorageDelegate`
presenter (same characters, styled attributes; storage stays plain), the block analyzer with AC7 demotion, E1's escape
hiding moved onto it (`EscapeHidingStyler` retired), the hidden-run caret snap, headings with line reveal, and the AC8
measurement. First: measure the three carried gaps (shift-selection across hidden runs; VoiceOver/spell-check; the
unrendered dumas `##` line). ✅ Q1 (H1 22 / H2 18 / H3 16 pt, H4–H6 body bold) and Q2 (revealed prefix in
`tertiaryLabelColor`, body font) ruled. Design → [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md).
🟠 **2026-10-05 — IMPLEMENTED - NOT VERIFIED.** ✅ `Scrivi/Views/MarkdownBlocks.swift` + `Scrivi/Views/ManuscriptPresenter.swift`
(new, in `project.pbxproj` ×3 targets); `ManuscriptEscapes.swift` (hidden-run, direction-aware snap on an `isHidden` test);
`ManuscriptTextView.swift` (presenter installed; `EscapeHidingStyler` removed). ✅ Interop 175/175 (baseline 168/168);
mutations bite; iOS + visionOS build; AC8: rebuild 6.36 → 3.45 ms, keystroke +0.07–0.10 ms; dumas headings 1,172/1,172.
✅ **Live pass PASSED (user, 2026-10-05):** *"All steps in the Live pass passed.  We can Verify T-0588.  If available, you are also authroized to close SP-161."* → [`../Sprints/Sprint-SP-161.md`](../../Sprints/Closed/Sprint-SP-161.md) progress log.
