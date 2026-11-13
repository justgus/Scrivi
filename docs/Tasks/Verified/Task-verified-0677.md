# Verified Task — T-0677

**Sprint:** [SP-216] · **Epic:** [EP-054] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-13** (user, 2026-11-13: *"the live pass checks 1-6 all passed."*)
**Archived:** 2026-11-13.

---

## ✅ T-0677 — `[Linux]` EP-054: the start window gets the project window's menu bar

**Epic:** [EP-054] · **Status:** ✅ **Verified 2026-11-13** (user) · **Sprint:** [SP-216] (✅ Closed 2026-11-13, AC4) →
[`../../Sprints/Closed/Sprint-SP-216.md`](../../Sprints/Closed/Sprint-SP-216.md) · **Date Requested:** 2026-11-13 (user: *"Linux's start window
should eventually have a menu bar, so we should create one that mirrors the Project menu bar, only with all the project-only
items greyed out."*)

**What:** Linux's start window (`LandingWindow`) carries the project window's menu bar — the same menus, items and order, built
by one builder — with the project-only items greyed out, as the Mac's menu bar is with no project open.
**Built (2026-11-13):** `ScriviWindow::buildMenuBar(host, w, env)` (static, `platforms/linux/src/ScriviWindow.cpp:233`), called by
each project window and by `LandingWindow` (`LandingWindow.cpp:26`); the hand-wired `landingSettingsAction` retired;
`AppEnvironment::shellController()`. `settings_smoke` §7, mutation-checked. Evidence: Docker Qt 6.4.2, ctest 847/847.
**Verified (2026-11-13):** the user's live pass on scrivi-vm, steps 1–6 (SP-216 AC4).
