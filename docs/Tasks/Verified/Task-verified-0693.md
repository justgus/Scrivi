# Verified Task — T-0693

**Sprint:** [SP-231] · **Epic:** [EP-059] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-21** (user, 2026-11-21: *"All the live tests pass."*)
**Archived:** 2026-11-21.

---

## ✅ T-0693 — `[Linux]` EP-059 D2 + D3: Reconnect Worlds, and the world-warning strip

**Epic:** [EP-059] (D2, D3) · **Status:** ✅ Verified (user, 2026-11-21: *"All the live tests pass."*) · **Sprint:** [SP-231] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-231.md`](../../Sprints/Closed/Sprint-SP-231.md) · **Date Requested:** 2026-11-20 (the Linux gap review, G5 and G6);
filed 2026-11-21 (user, 2026-11-21: *"Then you can begin planning SP-231."*)

**What:** a Reconnect Worlds button in the Worlds dialog that re-reads every open project's worlds and refreshes the Inspector, and
the Mac's world-warning strip (an unavailable world named, its pending links counted, Manage Worlds…, ×, View ▸ Show World
Warnings). Findings, questions and ACs: the Sprint.

**Built:** Reconnect Worlds (`WorldsDialog.cpp`, `AppEnvironment::reconnectWorlds`, and on coming back to the app, `main.cpp`);
`EditorShell::refreshWorlds` (off the main thread); `WorldWarningStrip` and View ▸ Show World Warnings; rejected imported
timelines in the Timeline panel's banner (`TimelinePanel::setRejectedImports`). Details: the Sprint.
