# Verified Task — T-0640

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (iPad and Mac live passes (user, 2026-11-01) and [T-0652] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0640 — `[Cross]` Zoom to the current scene

**Epic:** [EP-055] · **Status:** ✅ **Verified 2026-11-02** (iPad and Mac live passes (user, 2026-11-01) and [T-0652] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-199] (🟡 Active) → [`Sprint-SP-199.md`](../../Sprints/Closed/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What (user):** a small Timeline button that zooms in until single dots show and centres on the scene the Manuscript and
Navigator have selected. **Ruling (2026-10-31):** zoom until nearby dots separate.
**Done (Apple):** a `scope` button in the Timeline header (disabled with no current scene). `zoomToCurrentScene`
(`TimelineStripView.swift:1827`) finds the nearest other main-row item and zooms so it sits a twelfth of the window away
(capped by the existing `maxZoom`), then centres on `loader.viewportSceneID`. Checks: iOS 17/17 (`TimelineScaleTests` 3 new), Mac interop 327/327, Mac build, `check-layout-convergence.sh` clean.
**Linux (2026-11-01):** a `◎` button left of the +/− control (`platforms/linux/src/TimelinePanel.cpp:51`), disabled with no
active scene; `TimelinePanel::zoomToActiveScene` (`:695`) is Apple's rule (nearest other project-row dot a twelfth of the
window away, zoom capped at 500), centres the active scene and saves the view (T-0338). Checks: `timeline_cluster_smoke`
(a clustered scene separates and is centred; the view is saved), Linux smokes 38/38. ⏳ Rig live pass.
