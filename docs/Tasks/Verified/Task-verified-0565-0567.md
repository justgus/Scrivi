# Verified Tasks: T-0565 – T-0567

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0565** | ✅ **`SessionStore`** — `<appSupportRoot>/session.ini` via `QSettings`, keyed `[project/<projectID>]`, `path` an attribute | [SP-147] | [EP-043] | **2026-09-30** |
| **T-0566** | ✅ **Save on close/quit + the R6 guard** (`AppEnvironment::projectsToRestore`, `SCRIVI_NO_RESTORE` / `QT_QPA_PLATFORM=offscreen`) | [SP-147] | [EP-043] | **2026-09-30** |
| **T-0567** | ✅ **Restore at launch** (R4/R5) — `AppEnvironment::restoreSession`, per-project geometry on EVERY open, `clampedOnscreen`, in-flight R3 hold | [SP-147] | [EP-043] | **2026-09-30** |

✅ **VERIFIED by the user's live pass on the rig 2026-09-30** (GNOME, Wayland session over RDP):
- ✅ projects open at Quit reopen; ✅ a window CLOSED before quitting does not, ✅ and reopening it from
  Landing returns its old size ([R-Q2]);
- ✅ sizes, maximized state and splitter proportions are recorded per project;
- ✅ `SCRIVI_NO_RESTORE=1` opens Landing only, ✅ and the next normal launch restores the previous set —
  ⛔ the guard LOSES NOTHING (AC6);
- ✅ a project on an unplugged drive is skipped and returns, with its geometry, once the drive is back —
  *"through multiple restarts and existing window adjustments."*

✅ **T-0567 on the rig:** reopen, sizes, ✅ **maximized** ([I-0177]'s own symptom), splitters, and the
unplugged-drive skip all pass.
⚠️ **Window POSITION is NOT restored on Wayland — RULED ACCEPTED (user, 2026-09-30) → [I-0264]:** a
Wayland client can neither learn nor set its position (every saved frame read `0,0`); GNOME centres.
