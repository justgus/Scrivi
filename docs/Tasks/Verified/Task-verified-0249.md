# Verified Task — T-0249

**Sprint:** [SP-230] · **Epic:** [EP-059] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-21** (user, 2026-11-21: *"All steps pass."*)
**Archived:** 2026-11-21.

---

## ✅ T-0249 — `[Linux]` Manuscript navigation gestures

**Epic:** **EP-059** E2 (was EP-022; moved 2026-11-20) · **Status:** ✅ Verified (user, 2026-11-21: *"All steps pass."*) · **Sprint:** [SP-230] (✅ Closed) → [`../../Sprints/Closed/Sprint-SP-230.md`](../../Sprints/Closed/Sprint-SP-230.md); ✅ Page Forward / Backward dropped (SP-230 Q3: the Mac has neither); ✅ Manuscript Start / End on Ctrl+Home / Ctrl+End (Q2 re-ruled 2026-11-21), scene and chapter without keys as on the Mac

Page Forward / Page Backward + jump to absolute manuscript start / end. **Gestures and keystrokes are
undecided** — laptops and tablets lack extended keyboards, so no PageUp/PageDown/Home/End may be
assumed; pick touch-friendly bindings. Raised during SP-063 VNC verification.

⚠️ **Constrained by the VNC input path** (see `project_linux_vnc_input_constraints`): the
macOS→VNC→x11vnc→Xvfb→Qt chain drops Ctrl+Shift+key combos and pinch/pixelDelta gestures, so **a
gesture must never be the only path** to any of these actions.

*EP-022 itself is ✅ Closed (2026-07-15); this is an unscheduled follow-up, likely landing with EP-026
menus/parity.*
