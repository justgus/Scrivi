# Verified Task — T-0618

**Sprint:** [SP-190] · **Epic:** [EP-053] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-04** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`)
**Archived:** 2026-11-04.

---

## T-0618 — `[Cross]` EP-053 S5: Mac and rig live passes over the share, timed

**Epic:** [EP-053] (S5) · **Status:** ✅ **Verified 2026-11-04** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`) · **Sprint:** [SP-190] →
[`../../Sprints/Closed/Sprint-SP-190.md`](../../Sprints/Closed/Sprint-SP-190.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** live passes on dumas-prose-timelines over the share on the Mac and the rig: every structural operation and its undo
and redo, timed by each platform's log, with no freeze; the Mac's timing source per [SP-190] Q1.

**Done (2026-11-04):** the Mac's diagnostic log ([SP-190] Q1: `DiagnosticLog`, Help ▸ Show Log Folder). The steps: the rig's
[T-0661] and the Mac's section, both in `../../Sprints/Rig-Live-Pass-Held.md`. ✅ [SP-190] Q2 ruled 2026-11-04: Samba on scrivi-vm (`ScriviOtherLoop`).

**Verified (2026-11-04):** ✅ the rig over the share — [T-0661] run 2 (build 1.0.348): 24 edits, no stall after any, every
verification 0 corrected. ✅ The Mac on a local disk (AC re-ruled 2026-11-04) — build 1.0.349: 22 edits, undos and redos, every
verification `0 corrected, 0 skipped, 0 not correctable`; `… landed after` 62–108 ms (scene edits), 180–253 ms (chapter edits);
the only stalls ~1 s at launch, before any edit.
