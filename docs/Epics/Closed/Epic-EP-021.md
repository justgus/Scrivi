# EP-021: [Linux] Project Lifecycle & Landing

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` via the plain-C ABI
through `ScriviBridge`. One ScriviCore change was made (additive, Linux-gated — see below).

**Status:** ✅ Closed (user-approved 2026-07-14)
**Goal:** The Linux app can **create, open, and close a real `.scrivi` project** and remember recent
projects — driven from a **landing view** — building the first real screens on the EP-020 spine. The Ubuntu
analogue of the Apple project-lifecycle work (EP-006 flows). Did **not** deliver the writing surface or
navigator (that's EP-022).

**Strategy:** Second of the `[Linux]` family (EP-020–EP-026). EP-020 proved QML → `ScriviBridge` → C ABI →
ScriviCore end-to-end; EP-021 extended `ScriviBridge` with the lifecycle calls, added multi-screen
navigation (landing → project window), and made per-user state persistent.

**Design references:** `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md`,
`docs/Scrivi_Project_Package_Structure_v0_1.md`, `docs/Scrivi_External_Change_Repair_Matrix_v0_2.md`,
`docs/Scrivi_SecureStore_Platform_Trade_Study_v0_1.md`, `ScriviCore/include/scrivi/scrivi.h`.

**Date Created:** 2026-07-13
**Actual Close Date:** 2026-07-14

## Acceptance Criteria — all met

- [x] AC1 — **Landing view** as the first screen (recents list + empty state + New/Open actions). ✅ SP-059,
  VNC-verified.
- [x] AC2 — **Create project** via `scrivi_create_project`, real `.scrivi` on disk, transition to the
  (placeholder) project window. ✅ SP-059, VNC-verified (package confirmed on disk + from macOS).
- [x] AC3 — **Open project** via recents/folder, handling all three modes (**ready** / **repairRequired** /
  **cannotOpen**). ✅ SP-060, VNC-verified: two `ready` projects opened via recents + Open Project button;
  `repairRequired` (broken `manuscript.meta.json`) showed the repair dialog and blocked; non-`.scrivi` path
  → inline error.
- [x] AC4 — **Local identity bootstrap** with no duplicate-identity churn across launches (identity
  persists). ✅ SP-059, VNC-verified ("Signed in as Scrivi" survives quit+relaunch); required the new
  `EncryptedFileSecureStore` (T-0229). Re-exercised on the open path in SP-060.
- [x] AC5 — **Recent-projects persistence** (path + title + last-opened), shown on landing, survives
  restart. ✅ SP-059 (`recents.json` under appSupportRoot), VNC-verified; opened project moves to the top
  (SP-060).
- [x] AC6 — **Close / return to landing** cleanly, recents updated. ✅ SP-060, VNC-verified. **No
  `scrivi_close_project` endpoint needed** — close is purely an app-side UI/state transition.
- [x] AC7 — **Verified:** full create→open→close→reopen-recent loop over Docker+VNC; CI green with a
  headless lifecycle smoke. ✅ SP-060 (`lifecycle_smoke` in CI + user VNC verification).
- [x] AC8 — **No regression:** `scrivi.h` unchanged; ScriviCore `ctest` green; macOS app + EP-020 harness
  unaffected. ✅ The one ScriviCore change (`EncryptedFileSecureStore`) is additive and **Linux-gated**
  (`CMAKE_SYSTEM_NAME STREQUAL "Linux"`); Linux `ctest` 271/271, macOS `ctest` 264/264, `scrivi.h`
  untouched.

## Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-059 | Foundation + Create — appSupportRoot, recents store, landing view, New Project → `scrivi_create_project` (AC1/AC2/AC4/AC5) | ✅ Closed | 2026-07-13 – 2026-07-14 |
| SP-060 | Open + Close + Verify — Open Project + picker, open modes, close→landing, full-loop + CI + Epic close (AC3/AC6/AC7/AC8) | ✅ Closed | 2026-07-14 – 2026-07-14 |

## Completion Summary

EP-021 delivered the full Linux project-lifecycle loop — **create → open → close → reopen** — driven from a
**landing view**, on the EP-020 spine, across two sprints. The app resolves a stable per-user
`appSupportRoot` (XDG: `$XDG_DATA_HOME/Scrivi` / `~/.local/share/Scrivi`), keeps a `recents.json`
recent-projects store, bootstraps a **persistent local identity**, creates real `.scrivi` packages via
`scrivi_create_project`, and opens them via `scrivi_open_project` with all three open modes handled
(`ready` opens the placeholder project window; `repairRequired` surfaces the issues and blocks; `cannotOpen`
shows an inline error). Close returns to landing and floats the project to the top of recents. Verified in
Docker+VNC by the developer and green in CI (build + persistence smoke + full-loop lifecycle smoke).

**Key decisions / artifacts later `[Linux]` Epics inherit:**
- **appSupportRoot convention:** `$XDG_DATA_HOME/Scrivi` else `~/.local/share/Scrivi` (`AppSupport` helper).
- **Recents store:** app-side `recents.json` under appSupportRoot (`RecentsStore`, add-or-update/remove/list,
  newest-first, atomic writes) — the Linux `ProjectBookmarkStore` analogue (no `scrivi_*` recents endpoint).
- **Persistent SecureStore (the notable core change):** the C ABI wired an **in-memory**
  `PrototypeSecureStore` for every platform, so identity was recreated each launch (a documented MVP
  placeholder; Apple's `KeychainSecureStore` had been deleted with the C++ adapter). SP-059/T-0229 replaced
  it on Linux with **`EncryptedFileSecureStore`** (AES-256-GCM/OpenSSL at `<appSupportRoot>/secure/`, per the
  T-0058 trade study), **gated to Linux**. Apple/Windows persistence remains an open follow-up.
- **Folder picker:** Qt **Widgets `QFileDialog`** via `ScriviBridge::chooseFolder` — the Qt Quick
  `FolderDialog` couldn't select the folder being viewed and needed extra QML modules. Reused for both
  create and open.
- **Close is app-side:** no `scrivi_close_project` — ScriviCore holds no cross-call open handle.
- **Dev harness extended:** `build-and-run.sh` bind-mounts a persistent app-support dir and a shared
  `~/ScriviLinux/projects` ↔ `/projects` folder (macOS ↔ container); a **Quit** button; the picker defaults
  to `/projects`.

**Lessons:** a QML `Component` id is not its instantiated item (writing `landingPage.errorText` silently
aborted Open until fixed with a window-level property); a headless smoke that exercises the bridge is **not**
a substitute for driving the GUI over VNC; runtime QML-module / provider gaps don't show at compile time.

**Next:** **EP-022 `[Linux]` — Writing Surface & Scene Navigator** (the core writing loop:
`scrivi_open_scene`/`scrivi_save_scene`, navigator, auto-save), building on this lifecycle.

---

*Closed 2026-07-14 — second `[Linux]` Epic; delivered in two sprints (SP-059, SP-060). All AC1–AC8 met and
user-verified over VNC. One additive, Linux-gated ScriviCore change (`EncryptedFileSecureStore`).
User-approved close.*
