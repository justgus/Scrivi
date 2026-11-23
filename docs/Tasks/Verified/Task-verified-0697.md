# Verified Task — T-0697

**Sprint:** [SP-235] · **Epic:** [EP-059] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-22** (user, 2026-11-22: *"Steps 11-18 pass.  I saw all the banners and they appeared and disappeared at the proper times."*)
**Archived:** 2026-11-22.

---

## ✅ T-0697 — `[Linux]` EP-059 B1–B4: the object detail view and its image, as the Mac's

**Epic:** [EP-059] (B1, B2, B3, B4) · **Status:** ✅ Verified 2026-11-22 (user, 2026-11-22: *"Steps 11-18 pass.  I saw all the banners and they appeared and disappeared at the proper times."*) · **Sprint:** [SP-235] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-235.md`](../../Sprints/Closed/Sprint-SP-235.md) · **Date Requested:** 2026-11-22 (user, 2026-11-22: *"close SP-234, then plan B1-B4"*)

**What:** the Mac's object detail sheet (`ObjectDetailSheet.swift`, `ObjectImageSection.swift`) on Linux, in place of the "not built
yet" message (`EditorShell.cpp:4503`): Name, Subtitle, Notes, Tags, edited and saved by patch; the image added, replaced, chosen from
the world and removed; read-only and explained when the object's world is away; nothing lost when the world goes away while it is open.
Sources, Relationships and in-sheet navigation are C1–C4. Findings, questions and ACs: the Sprint.

**Built:** `ObjectDetailDialog` (new) — the fields, Save / Undo Changes / the guarded close, the image (Add / Replace / Choose
Existing / Remove; a shared asset is never deleted), read-only and the world-away view, the re-read on `refreshWorlds`; opened from
`EditorShell::onOpenObjectRequested`; the row's menu says View Detail. Details and the live pass: the Sprint.
