# Verified Task — T-0631

**Sprint:** [SP-197] · **Epic:** [EP-052] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) — Apple only)
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0631 — `[Apple]` EP-052 S3: Spotlight on one serial background queue, per-scene, behind the saves

**Epic:** [EP-052] (AC9) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) — Apple only) · **Sprint:** [SP-197] (🟡 Active) → [`../../Sprints/Closed/Sprint-SP-197.md`](../../Sprints/Closed/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** v0.2 §5 S1–S4. Every `CSSearchableIndex` call (donate, re-index, delete on close) runs on one serial queue
(Apple: one thread at a time). The open's donation and the app-switch re-index (`ProjectSession.swift:541,550`) leave the
main thread; the extraction is a read behind the queue's saves; the app switch re-indexes only the scenes typed in
([T-0629] (b)); a close cancels a pending job. Only `spotlightDomainIdentifier` is set on the main actor, when the job lands.
**Tests:** the inventory's open and app-switch rows read 0 file operations on the main thread; a close during a donation.

**Done (2026-10-27):** every Spotlight call runs on one serial background queue (`SpotlightDonor.enqueue`,
`SpotlightDonor.swift:40`): the open's donation and the app-switch re-index extract there, behind the queue's saves; the app
switch re-indexes only the scenes typed in (`extractSearchableScenes`); a close cancels a pending job and deletes the domain
behind any donation already started (by the projectID, so it is known before the first donation lands). **Evidence:** interop
323/323; build clean. ⚠️ No test drives a close during a donation.
