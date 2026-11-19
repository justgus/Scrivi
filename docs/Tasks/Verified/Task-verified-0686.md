# Verified Task — T-0686

**Sprint:** [SP-223] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-19** (user, 2026-11-19: *"yes, close SP-223 and verify T-0686"*)
**Archived:** 2026-11-19.

---

## ✅ T-0686 — `[Apple]` EP-055 S3b: Find and Replace, Go To, branch marks, copy buffers and Format on iOS

**Epic:** [EP-055] (AC5, second half) · **Status:** ✅ **Verified 2026-11-19** (user) · **Sprint:** [SP-223] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-223.md`](../../Sprints/Closed/Sprint-SP-223.md) · **Date Requested:** 2026-11-18 (user: *"plan S3b for EP-055"*)

**What:** on iOS, Find and Replace over the presented text (`UIFindInteraction`), Go To on the iPad menu bar, the branch-point
marks, the nine copy buffers and the Format commands — moving the Mac host's `applyFormat`, buffer operations and Replace All,
and `ManuscriptPresentedMap`'s view-free part, into shared code that both hosts call. Rulings and ACs: the Sprint.

**Where:** `ManuscriptStructure.swift` (`replaceAll`, `copyIntoBuffer` / `cutIntoBuffer` / `pasteFromBuffer`), `ManuscriptCommands.swift` (`formatStep`),
`ManuscriptFind.swift` (`ManuscriptTextSearcher`), `ManuscriptPresentedMap.swift`, `ManuscriptUITextView.swift`, `ScriviApp.swift` (iPad menus).
