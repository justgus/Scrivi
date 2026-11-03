# Verified Task — T-0630

**Sprint:** [SP-197] · **Epic:** [EP-052] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0650] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0630 — `[Cross]` EP-052 S3: the Inspector and the timeline read off the main thread, revision-checked

**Epic:** [EP-052] (AC8) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0650] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-197] (🟡 Active) → [`../../Sprints/Closed/Sprint-SP-197.md`](../../Sprints/Closed/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** Mac and Linux in the same work (`feedback_linux_adopts_apple_shape`). The Inspector shows the new scene's cards
at once ([SP-197] Q2), runs [T-0629]'s read as ONE job off the main thread, and fills the cards when it lands if
`scrivi_get_project_revision` has not moved (D4); otherwise it reads again. The timeline's reloads do the same: the open's
main-thread fallback (`ProjectSession.swift:328`) and Linux's `reloadTimeline()` (`EditorShell.cpp:3351`). Linux's
object-card worker (`SceneInspector.cpp:399`) is the shape; its notes read (`:856`) joins it.
**Tests:** the inventory probes (`MainThreadInventoryTests`; `SCRIVI_INVENTORY=1 structural_baseline_smoke`) assert 0 file
operations on the main thread for click, select, scroll, navigator and the Inspector following; a result whose revision
moved is dropped.

**Done (2026-10-27):** Mac: `SceneInspectorReader` (`InspectorCard.swift:199`), owned by `SceneInspectorView`, makes ONE read
per scene off the main actor; every card (tags, outline, todo, the ten object cards, Sources) and the Properties tab are built
from it (`CardContext.scenePart`); a result whose revision moved is read again; while the read is out the cards keep their last
contents dimmed and take no edits, "Loading…" after 400 ms (Q2, `sceneReadGate`); a card's own write re-reads the scene. The
open's timeline fallback is re-read off the main actor (`ProjectSession.swift:560`). Linux: the Inspector's worker makes the one
call, the notes included (`SceneInspector.cpp:374`), revision-checked, the last contents dimmed, "Loading…" after 400 ms; the
load's stale timeline is re-read on a worker (`EditorShell.cpp:3355`). **Evidence:** interop 323/323, new
`MainThreadNoFileOpsTests` (0 main-thread file ops for click, select, scroll, navigator and the Inspector following; a card's
write re-reads); Linux smokes 38/38, `structural_baseline_smoke`'s new SP-197 gate (0 GUI-thread file ops for click, select,
scroll) — a mutation in a copy (the notes read back on the GUI thread) fails it on all four gestures.
