# Verified Task — T-0639

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Apple]`
**Implemented:** 2026-10-31 · ✅ **USER-VERIFIED 2026-11-01** (user, 2026-11-01: *"All these steps passed on both iPad and the Mac."*)
**Archived:** 2026-11-01.

---

## ✅ T-0639 — `[Apple]` The Timeline's zoom and position come back next time

**Epic:** [EP-055] · **Status:** ✅ **Verified 2026-11-01** (user, 2026-11-01: *"All these steps passed on both iPad and the Mac."*) · **Sprint:** [SP-199] (🟡 Active) → [`../../Sprints/Sprint-SP-199.md`](../../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What (user):** a zoom and scroll position should be restored when the app next opens the project. Apple never saved it;
Linux has since T-0338 (SP-083). **Ruling (2026-10-31):** per project, per device, outside the package.
**Done:** `TimelineViewStateStore` (`TimelineStripView.swift:3692`) keeps `[zoom, scroll]` in UserDefaults under
`scrivi.timeline.<projectID>.view`; restored on `projectID` (`:1373`), saved on every change. `EditorView` passes the projectID.
⚠️ The position is a fraction of the span: if the span changes (an imported timeline shown or hidden), the same fraction is a
different time. Linux behaves the same. Checks: iOS 17/17 (`TimelineScaleTests` 3 new), Mac interop 327/327, Mac build, `check-layout-convergence.sh` clean.
