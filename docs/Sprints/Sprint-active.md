# Active Sprints

⏸️ **SP-124 is PAUSED** (EP-038, sprint 2 of 2) — ⚠️ **the DRIVE PULL.** ✅ **Paused 2026-08-31 by user
ruling, NOT blocked** — ⚠️ **its S1 baseline is captured and the rig's `cifs` mount is left INTACT**, so
it resumes without redoing setup.

✅ **SP-127 CLOSED 2026-09-02** (user-approved) → [`Closed/Sprint-SP-127.md`](Closed/Sprint-SP-127.md) — **EP-035 AC3** delivered. ⚠️ **Its live pass found FOUR defects, all Verified.**

⚠️ **NO SPRINT IS CURRENTLY ACTIVE.** ⏸️ **SP-124 is PAUSED and resumable** — ✅ **its gate is now satisfied: SP-127 delivered the Worlds surface it was waiting for.**

⚠️ **WHY THE SWAP** (user, 2026-08-31): ✅ ***"the App won't incorrectly represent the mount point until
it can correctly represent the mount point."*** ⚠️ **Linux has NO world surface at all** —
`addWorld`/`relinkWorld`/`getWorldStatus`/`getWorldBinding` are bridged with ⚠️ **ZERO callers**.
⚠️ **So a drive-loss sprint could measure OS signals but could never SHOW them to a writer**, and
⚠️ **I-0181 was re-scoped to a LATENT CORE defect once that was seen.** ✅ **Building the surface first
means SP-124's scenarios get verified against something a writer can actually read.**

✅ **THE SWAP WAS VINDICATED.** ⚠️ **SP-127's live pass found a 🔴 DATA-LOSS defect (I-0183) that only
a real degraded mount could trigger** — ⚠️ **10 of 12 relationships destroyed in the user's real
project** — ⚠️ **and it was found THROUGH the new surface, by a writer using it.** ⚠️ **A drive-loss
sprint run first would have measured the OS correctly and never seen it.**

⚠️ **I-0181 is now worth revisiting**: the Worlds dialog is exactly where a false `missing` would
become visible to a writer, which is the condition the Issue was parked on.

✅ **SP-126 CLOSED 2026-08-30** → [`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) — the three-tab
Scene Inspector shell, ⚠️ **closing EP-035 AC10** and retiring the "Scene Entities" placeholder.

✅ **SP-125 CLOSED 2026-08-29** → [`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) — the first
Linux object surface.

✅ **SP-123 CLOSED 2026-08-29** → [`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) — the rig is
BUILT, GREEN, and runs the app on real hardware.

### ⚠️ What is available to activate next

| Candidate | Epic | State |
| --------- | ---- | ----- |
| **AC3** — world binding + ⚠️ **the RELINK UI that does not exist** | EP-035 | ⚪ Not planned — ⚠️ **a moved world currently cannot be repaired from the app at all** |
| **AC4** — object CRUD | EP-035 | ⚪ Not planned |
| ⚠️ **Session persistence** (I-0176/0177/0178) | ⚠️ **NO EPIC** | ⚠️ **Apple's EP-018 equivalent — wants its own Epic** |
| **I-0180** — `[Apple]` object-card label | ⚠️ **NO HOME** | ⚠️ **EP-034 is closed** |

---
---
