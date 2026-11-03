# Verified Task — T-0642

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Apple]`
**Implemented:** 2026-10-31 · ✅ **USER-VERIFIED 2026-11-01** (user, 2026-11-01: *"All these steps passed on both iPad and the Mac."*)
**Archived:** 2026-11-01.

---

## ✅ T-0642 — `[Apple]` The Timeline stays where the pinch leaves it

**Epic:** [EP-055] · **Status:** ✅ **Verified 2026-11-01** (user, 2026-11-01: *"All these steps passed on both iPad and the Mac."*) · **Sprint:** [SP-199] (🟡 Active) → [`Sprint-SP-199.md`](../../Sprints/Closed/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What (user, SP-199 live pass):** on release of a pinch the view jumped somewhere other than what was on screen.
**Cause:** `onEnded` recentred from the PRE-pinch window (the gesture state had already reset to 1).
**Done:** `onEnded` commits the zoom only (`TimelineStripView.swift:1393`); the window drawn during the pinch is kept.
⚠️ While pinching, the still point is the one at the same fraction across the screen as the view is through the timeline
(left edge at the start); it does not follow the fingers. Linux has no pinch (Ctrl+wheel and buttons). Checks: iOS 17/17 (`TimelineScaleTests` 3 new), Mac interop 327/327, Mac build, `check-layout-convergence.sh` clean.
