# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0612 — `[Linux]` A diagnostic log: what the app was doing, and when the window stopped answering

**Epic:** [EP-026] · **Status:** 🟠 **Implemented - Not Verified** (2026-10-17) — ⏳ rig live pass (SP-185 AC6) · **Sprint:** [SP-185] (🟡 Active) →
[`../Sprints/Sprint-SP-185.md`](../Sprints/Sprint-SP-185.md) · **Date Requested:** 2026-10-17 (user: *"We may need to create a
log file so that we can record what the app is doing. A simple code inspection may not be adequate."*)

**What:** a log file the rig keeps between runs, so a freeze or a disappearance can be attributed afterwards. Its shape is
[SP-185] Q1 (✅ ruled 2026-10-17: *"build the log as proposed."*): one file under `~/.local/state/scrivi/`, timed spans of the app's operations, a
main-thread stall watchdog, and Qt's own warnings and fatal messages.

**Done (2026-10-17):** `DiagnosticLog` (`platforms/linux/src/DiagnosticLog.hpp/.cpp`) with the watchdog, Qt's messages, a
crash report with a backtrace, timed spans across the editor, find bar and workers, and Help ▸ Show Log Folder. Proven by a
new `diagnostic_log_smoke` (CI); 3 mutations caught.

---

*Last Updated: 2026-10-22 (**T-0619 → ✅ Verified** (user) and archived → `Verified/Task-verified-0619.md`, with [SP-191]'s close.)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** ([SP-191]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-21 (**T-0605, T-0607, T-0608, T-0609, T-0613, T-0615 → ✅ Verified** (user: Mac + rig live passes, build 75) and archived → `Verified/Task-verified-XXXX.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** ([SP-187]); ⏳ live passes.)*

*Last Updated: 2026-10-20 (**T-0614 → ✅ Verified** (user) and archived → `Verified/Task-verified-0614.md`, with SP-186's close.)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** ([SP-182]); ⏳ rig live pass.)*

