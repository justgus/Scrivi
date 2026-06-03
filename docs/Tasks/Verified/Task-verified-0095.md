## T-0095: EP-009 verification — all 18 ACs green; 171 ctests green; macOS smoke test

**Status:** ✅ Implemented - Verified
**Component:** EP-009 (all)
**Priority:** Critical
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-03
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-025

**Rationale:**
Final gate before EP-009 can close. All acceptance criteria must be verified by the user on the macOS app, and the full ctest suite must remain green.

**Verification Results:**

| AC | Description | Result |
| -- | ----------- | ------ |
| AC1 | Project opens to full-height editor, cursor ready | ✅ |
| AC2 | Scenes as continuous text, thin divider only | ✅ |
| AC3 | Viewport buffer loads neighbors of long scenes | ✅ |
| AC4 | `⌘↩` creates scene, inserts divider, moves cursor | ✅ |
| AC5 | `⌘⇧↩` creates chapter+scene, inserts divider, moves cursor | ✅ |
| AC6 | Navigator shows chapters as non-selectable headers | ✅ |
| AC7 | Auto-save fires ~1s after last keystroke | ✅ |
| AC8 | Scene saved immediately on scroll-exit | ✅ |
| AC9 | Scene saved on app quit; backup stub in place | ✅ |
| AC10 | Navigator lists all scenes in manuscript order | ✅ |
| AC11 | Navigator titles: first line or "Scene X" | ✅ |
| AC12 | Navigator titles update live ~300ms | ✅ |
| AC13 | Click Navigator row: loads scene, scrolls editor | ✅ |
| AC14 | Scroll up past divider promotes previous scene | ✅ |
| AC15 | Scroll down past divider promotes next scene | ✅ |
| AC16 | Editor is plain text / raw Markdown, no styling | ✅ |
| AC17 | 171/171 ctests green | ✅ |
| AC18 | macOS only; iOS/iPadOS deferred | ✅ |

---

*Closed: 2026-06-03*
