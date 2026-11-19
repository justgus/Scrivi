# Verified Task — T-0688

**Sprint:** [SP-225] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-19** (user, 2026-11-19: *"all steps pass, close SP-225.  In addition the hover worked for both the pointer and the pencil."*)
**Archived:** 2026-11-19.

---

## ✅ T-0688 — `[Apple]` EP-055 S5: worlds and the timeline's dots on iPad and iPhone

**Epic:** [EP-055] (AC6, part) · **Status:** ✅ **Verified 2026-11-19** (user) · **Sprint:** [SP-225] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-225.md`](../../Sprints/Closed/Sprint-SP-225.md) · **Date Requested:** 2026-11-19 (user: *"plan SP-225"*)

**What:** world Create (in a folder picked in Files, Q1), Add Existing and Relink on iOS through `.fileImporter`; world activation at
open now runs on iOS; the timeline's scene dot is a 44 pt touch target on iOS; an aggregate opens by tap on every platform (Q2) and
lists its members by name on iOS; hover is the system's (Q3). Rulings and ACs: the Sprint.

**Where:** `WorldsView.swift` (`handlePicked`, the split actions), `AppEnvironment.swift` (`activateWorlds`, `activateWorldsOffMain`),
`TimelineStripView.swift` (`kMinTouchTarget`, `AggregateMembersList`, the two aggregate views, `SceneDotView`),
`ScriviManuscriptTests-iOS.swift` (`worldCreatedInFolder`).

**Tested:** iOS 35/35 (1 new), Mac interop 352/352, macOS and iOS builds, the six CI guard scripts. ✅ the iPad, iPhone and Mac live passes, pointer and Pencil hover included (user, 2026-11-19).
