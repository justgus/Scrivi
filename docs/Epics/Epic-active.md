# Active Epics

## EP-021: [Linux] Project Lifecycle & Landing

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` only via the
existing plain-C ABI through `ScriviBridge`. Any missing endpoint is a Task with a `[ScriviCore]` note.

**Status:** 🟡 Active (drafted & activated 2026-07-13)
**Goal:** The Linux app can **create, open, and close a real `.scrivi` project** and remember recent
projects — driven from a **landing view** — building the first real screens on the EP-020 spine. This is
the Ubuntu analogue of the Apple project-lifecycle work (EP-006 flows): a landing surface with recent
projects + new/open actions, project creation and opening through `scrivi_create_project` /
`scrivi_open_project`, local-identity bootstrap via `scrivi_ensure_local_identity`, and app-side recent-
project persistence (the Linux equivalent of `ProjectBookmarkStore`). It does **not** deliver the writing
surface or navigator (that's EP-022).

**Strategy:** Second of the `[Linux]` family (EP-020–EP-026). EP-020 proved QML → `ScriviBridge` → C ABI
→ ScriviCore end-to-end with the identity round-trip; EP-021 extends `ScriviBridge` with the lifecycle
calls and adds the first multi-screen navigation (landing → open project → placeholder project window).
Verified in Docker+VNC (developer) and, when a milestone lands, on real Ubuntu (alpha tester).

**Design references:**
- `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md` — the canonical create/open lifecycle the Linux
  app must mirror (first-launch, identity bootstrap, open modes, repair).
- `docs/Scrivi_Project_Package_Structure_v0_1.md` — on-disk `.scrivi` layout.
- `docs/Scrivi_External_Change_Repair_Matrix_v0_2.md` — the `repairRequired` / `cannotOpen` open modes.
- `ScriviCore/include/scrivi/scrivi.h` — `scrivi_ensure_local_identity`, `scrivi_create_project`,
  `scrivi_open_project` (signatures + envelope shapes confirmed 2026-07-13; see notes below).
- Apple analogues (behavior to re-create in C++/QML, **not** port wholesale): `ProjectBookmarkStore`,
  `OpenProjectRegistry`, `ProjectSession`, `LandingView.swift`, `NewProjectSheet.swift`.

**Date Created:** 2026-07-13
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [x] AC1 — A **landing view** is the app's first screen: shows a recent-projects list (empty-state on
  first launch) and **New Project** / **Open Project** actions. Real Qt window, keyboard + pointer usable.
  ✅ SP-059 (VNC-verified 2026-07-14; Open Project stubbed/disabled until SP-060).
- [x] AC2 — **Create project:** a "new project" flow collects a title (+ location) and calls
  `scrivi_create_project` through `ScriviBridge`, creating a real `.scrivi` package on disk; on success
  the app transitions out of the landing view to the opened project (placeholder project window is fine —
  the editor is EP-022). ✅ SP-059 (VNC-verified 2026-07-14; package confirmed on disk + from macOS).
- [ ] AC3 — **Open project:** selecting a recent project or choosing a folder calls `scrivi_open_project`
  and handles all three open modes — **ready**, **repairRequired**, **cannotOpen** — surfacing repair
  issues / errors to the user rather than crashing or silently failing. _(SP-060)_
- [x] AC4 — **Local identity bootstrap:** identity is ensured (`scrivi_ensure_local_identity`) before
  create/open as the flow requires, reusing the EP-020 bridge call; no duplicate-identity churn across
  app launches (identity persists — see the appSupportRoot note under Open Questions).
  ✅ SP-059 for the **create** path (VNC-verified: "Signed in as Scrivi" survives quit+relaunch, via the
  new `EncryptedFileSecureStore` / T-0229). Re-exercised on the **open** path in SP-060.
- [x] AC5 — **Recent-projects persistence (Linux `ProjectBookmarkStore` analogue):** opened/created
  projects are remembered across app restarts (path + title + last-opened), shown on the landing view,
  and removable. Stored app-side (no ScriviCore change) — location/format decided in the first sprint.
  ✅ SP-059 (`recents.json` under appSupportRoot; VNC-verified persistence across relaunch; remove wired,
  UI removal affordance lands with the open path in SP-060).
- [ ] AC6 — **Close / return to landing:** closing a project returns to the landing view cleanly
  (resources released, recent list updated). Whether a `scrivi_close_project` C-ABI endpoint is needed is
  resolved in planning (see Open Questions) — if so, it's a `[ScriviCore]` Task, additive-only. _(SP-060)_
- [ ] AC7 — **Verified:** developer confirms the full create→open→close→reopen-recent loop over
  Docker+VNC; CI stays green (build + headless smoke; a lifecycle smoke test if feasible headless).
  _(SP-060 — SP-059 verified the create+persistence portion.)_
- [~] AC8 — **No regression:** `scrivi.h` unchanged or additive-only; ScriviCore `ctest` green; the macOS
  app and EP-020's hello-slice/harness still build and run. **On track:** SP-059 added
  `EncryptedFileSecureStore` (additive, `scrivi.h` unchanged), Linux `ctest` 271/271, macOS `ctest`
  264/264 (secure store gated to Linux — macOS untouched). Final confirm with SP-060.

### Sprints

**Planned as 2 sprints** (decided 2026-07-13): foundation + create path first, then open/close +
verification & Epic close — mirroring how EP-020 ran (spine first, close-out second). Each sprint is a
shippable, VNC-verifiable milestone. Task IDs assigned at sprint activation.

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-059 | Foundation + Create — XDG `appSupportRoot`, recents store, landing view shell, New Project → `scrivi_create_project` (AC1, AC2, AC4, AC5) | ✅ Closed (VNC-verified) | 2026-07-13 – 2026-07-14 |
| SP-060 | Open + Close + Verify — Open Project + folder picker, open modes (ready/repairRequired/cannotOpen), close→landing, full create→open→close→reopen loop + CI + Epic close (AC3, AC6, AC7, AC8) | 🔵 Planning | — |

**Split rationale:** SP-059 lands the persistence foundation (Open Question #1, the critical path) plus the
first end-to-end path (create a real `.scrivi`, see it in recents, persist across restart). SP-060 adds the
open path — including the genuinely-branchy open-mode handling — closes the lifecycle loop, and verifies +
closes the Epic. AC4 (identity bootstrap) spans both since create and open each need it.

### Tasks

_(defined at sprint activation — see `docs/Tasks/Task-backlog.md` / `Task-active.md`)_

### Issues

_(none yet)_

### Open Questions (resolve during first-sprint planning)

1. **appSupportRoot persistence.** EP-020's hello-slice used a throwaway `/tmp/scrivi-linux-appsupport`,
   so identity was recreated each `docker run`. EP-021 must resolve a **stable per-user appSupportRoot**
   on Linux (XDG: `$XDG_DATA_HOME` / `~/.local/share/Scrivi`) so identity and recents persist. This is
   the app-side counterpart to macOS Application Support. **Decide the exact path convention first** —
   everything else (AC4, AC5) depends on it.
2. **Recent-projects store format.** Apple uses security-scoped bookmarks (`ProjectBookmarkStore`); Linux
   has no such thing — plain absolute paths in a JSON/QSettings file under appSupportRoot are the likely
   analogue. Decide format + location; confirm it survives app restart in the VNC harness.
3. **Is a `scrivi_close_project` endpoint needed?** The C ABI has create/open but **no close** and **no
   list-recents** (confirmed 2026-07-13). Recents are clearly app-side. Determine whether "close" needs
   any core-side flush/unlock (→ additive `[ScriviCore]` Task) or is purely a UI/state transition.
4. **Folder picker in Qt/QML on Linux.** `open_project` needs a directory path — use `FolderDialog`
   (Qt.labs.platform / QtQuick.Dialogs). Confirm the needed QML module is in the Dockerfile+CI apt lists
   (EP-020 lesson: runtime QML modules are separate packages).
5. **repairRequired UX scope.** `open_project` can return `repairRequired` with a list of issues. Decide
   how much repair UI EP-021 owns vs. defers — likely: **surface** the issues and block/allow open, full
   repair workflow deferred to a later Epic.

### Scope Notes

- **In scope:** landing view, create/open/close lifecycle, identity bootstrap, recent-projects
  persistence, open-mode handling (ready/repairRequired/cannotOpen surfaced), the navigation transition
  landing ↔ project window (placeholder project window).
- **Out of scope (later `[Linux]` Epics):** the manuscript editor + scene navigator (EP-022), structure
  editing (EP-023), inspector (EP-024), timeline (EP-025), undo/menus/settings + parity (EP-026), and the
  full external-change repair workflow.
- **Bridge discipline (inherited from EP-020):** every new lifecycle method on `ScriviBridge` parses the
  `{"ok":...}`/`{"error":...}` envelope, `scrivi_free`s every pointer (RAII `ScriviString`), and holds
  **no** backend logic. New `.cpp/.hpp/.qml` files are CMake-tracked (not in the Apple pbxproj) but stay
  browsable via the `platforms/linux` Xcode folder reference.
- **Verification model (inherited):** CI = build + headless smoke; developer = GUI over Docker+VNC (host
  port 5901, password `scrivi`); alpha tester = real Ubuntu.

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-14 (SP-059 ✅ closed & VNC-verified — EP-021 AC1/AC2/AC5 met and AC4 met for the
create path; AC8 on track. Notable: SP-059 could **not** stay "no ScriviCore change" — the C ABI's
in-memory `PrototypeSecureStore` had to be replaced (Linux-gated) with `EncryptedFileSecureStore` (T-0229,
user-approved) for identity to persist (AC4). EP-021 remains open; **SP-060** (open + close + full-loop
verify → AC3/AC6/AC7/AC8) is the second and final sprint. 5 open questions were resolved in SP-059
planning; appSupportRoot persistence — the critical-path one — is done.)*
