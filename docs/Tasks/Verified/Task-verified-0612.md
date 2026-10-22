# Verified Task — T-0612

**Sprint:** [SP-185] · **Epic:** [EP-026] · **Platform:** `[Linux]`
**Implemented:** 2026-10-17 · ✅ **USER-VERIFIED 2026-10-22** (user, 2026-10-22: *"T-0612 and I-0310 are verified"* — the rig's real `scrivi.log` on build 79, read 2026-10-22) — SP-185 AC2 and AC6: build 79's log on the rig holds the build line, timed load spans, each structural worker's phases, the watchdog's stall lines with what was running, and a clean `session ended` after each interrupted load.
**Archived:** 2026-10-22, on the user's instruction; the Sprint closed the same day (user-approved).

---

## T-0612 — `[Linux]` A diagnostic log: what the app was doing, and when the window stopped answering

**Epic:** [EP-026] · **Status:** ✅ **Verified 2026-10-22** (user, 2026-10-22: *"T-0612 and I-0310 are verified"* — the rig's real `scrivi.log` on build 79, read 2026-10-22) · **Sprint:** [SP-185] →
[`../../Sprints/Closed/Sprint-SP-185.md`](../../Sprints/Closed/Sprint-SP-185.md) · **Date Requested:** 2026-10-17 (user: *"We may need to create a
log file so that we can record what the app is doing. A simple code inspection may not be adequate."*)

**What:** a log file the rig keeps between runs, so a freeze or a disappearance can be attributed afterwards. Its shape is
[SP-185] Q1 (✅ ruled 2026-10-17: *"build the log as proposed."*): one file under `~/.local/state/scrivi/`, timed spans of the app's operations, a
main-thread stall watchdog, and Qt's own warnings and fatal messages.

**Done (2026-10-17):** `DiagnosticLog` (`platforms/linux/src/DiagnosticLog.hpp/.cpp`) with the watchdog, Qt's messages, a
crash report with a backtrace, timed spans across the editor, find bar and workers, and Help ▸ Show Log Folder. Proven by a
new `diagnostic_log_smoke` (CI); 3 mutations caught.
