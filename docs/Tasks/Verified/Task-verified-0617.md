# Verified Task — T-0617

**Sprint:** [SP-189] · **Epic:** [EP-053] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-04** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`)
**Archived:** 2026-11-04.

---

## T-0617 — `[Cross]` EP-053 S4: the background sync verification

**Epic:** [EP-053] (S4) · **Status:** ✅ **Verified 2026-11-04** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`) ·
**Sprint:** [SP-189] → [`../Sprints/Sprint-SP-189.md`](../Sprints/Sprint-SP-189.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** after every structural operation, a background task on both platforms re-reads only the files the delta names and
corrects any difference from the disk silently (design R3, P4, Q3), never overwriting a scene typed into meanwhile (Q2).

**Done (2026-11-04):** `scrivi_range_verify_read` (core); Linux `RangeVerify.hpp` + `EditorShell::verifyAgainstDisk`; Apple
`RangeVerify` + `ViewportSceneLoader.verifyAgainstDisk` + `applyCorrection`. A two-scene paste's verification = 1 listing +
2 reads. Tests on all three; mutations Linux 6/6, Apple 3/3. ✅ [SP-189] Q3 ruled 2026-11-04: a difference a delta cannot fix is logged only.

**Verified (2026-11-04):** ✅ the rig over the share — [T-0661] run 2 (build 1.0.348): 24 edits, no stall after any, every
verification 0 corrected. ✅ The Mac on a local disk (AC re-ruled 2026-11-04) — build 1.0.349: 22 edits, undos and redos, every
verification `0 corrected, 0 skipped, 0 not correctable`; `… landed after` 62–108 ms (scene edits), 180–253 ms (chapter edits);
the only stalls ~1 s at launch, before any edit.
