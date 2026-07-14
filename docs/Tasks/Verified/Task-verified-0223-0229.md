# Verified Tasks — T-0223–T-0229 (SP-059, EP-021 [Linux] Project Lifecycle — create half)

_Archived 2026-07-14 on SP-059 close. All seven tasks ✅ Verified (T-0229 added mid-sprint, user-approved).
Original active-task detail preserved below verbatim._


**Sprint SP-059** — `[Linux]` Project Lifecycle (first of 2 EP-021 sprints): XDG `appSupportRoot`,
recent-projects store, landing view, and the New Project → `scrivi_create_project` path. Epic: EP-021
`[Linux]`. Design: `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md`,
`docs/Scrivi_Project_Package_Structure_v0_1.md`, `ScriviCore/include/scrivi/scrivi.h`. Apple behavior
analogues: `AppEnvironment.swift`, `NewProjectSheet.swift`, `ProjectBookmarkStore`.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0223 | XDG `appSupportRoot` resolver (`$XDG_DATA_HOME`/`~/.local/share/Scrivi`, created if absent) | ✅ Verified |
| T-0224 | Recent-projects store — JSON under appSupportRoot: add/update/remove/list, survives restart | ✅ Verified |
| T-0225 | `ScriviBridge` lifecycle methods — launch identity bootstrap + `createProject` | ✅ Verified |
| T-0226 | Landing view QML — recents list + empty state + New/Open actions (Open stubbed → SP-060) | ✅ Verified |
| T-0227 | New Project flow — title→slug→`<folder>/<slug>.scrivi`, create, add to recents, go to placeholder window | ✅ Verified |
| T-0228 | Docker/CI — apt lists (libssl-dev); headless persistence smoke | ✅ Verified |
| T-0229 | `EncryptedFileSecureStore` (Linux) — persistent identity so AC4 holds | ✅ Verified |

---

## SP-059 — [Linux] Project Lifecycle (create half)

**Epic:** EP-021 `[Linux]` · Delivered **AC1, AC2, AC4, AC5** (landing view; create project; identity
bootstrap; recent-projects persistence). Open/close + open-mode handling + full-loop verification are
SP-060.

**Step 0 — appSupportRoot convention (before T-0223):** recorded in `platforms/linux/README.md` — the
Linux `appSupportRoot` is **`$XDG_DATA_HOME/Scrivi`** (fallback **`~/.local/share/Scrivi`**), the Linux
counterpart to macOS Application Support. Critical-path: AC4 (identity persistence) and AC5 (recents) both
depend on it. EP-020's throwaway `/tmp/scrivi-linux-appsupport` retired here.

### T-0223 — XDG `appSupportRoot` resolver  *(AC4 foundation)*
A small C++ helper (callable from `ScriviBridge`) resolving the app-support directory: `$XDG_DATA_HOME/Scrivi`
if set and non-empty, else `~/.local/share/Scrivi`; expand `~` from `$HOME`; `mkdir -p`. Replaced the
hardcoded `/tmp` root from EP-020's hello-slice. Path computed app-side, passed into the existing
`appSupportRoot` C-ABI params.

### T-0224 — Recent-projects store  *(AC5)*
App-side store (Linux `ProjectBookmarkStore` analogue — no `scrivi_*` recents endpoint exists). A JSON file
under appSupportRoot (`recents.json`) holding an ordered list of `{path, title, lastOpened}`. Operations:
add-or-update (front + refresh lastOpened), remove (by path), list (newest-first). Round-trips across
process restarts. Plain absolute paths. Tolerates a missing/corrupt file (start empty, don't crash).

### T-0225 — `ScriviBridge` lifecycle methods  *(AC2, AC4)*
- **Launch identity bootstrap** — on app start, call `scrivi_ensure_local_identity(hostname, appSupportRoot)`
  once (mirrors Apple `AppEnvironment.bootstrap()`), stash `identityID`/`defaultPersonaID`/`displayName`,
  expose to QML (`ready` signal / properties).
- **`createProject(projectRootPath, title, slug)`** — fill remaining `scrivi_create_project` args
  (appSupportRoot + stashed identity) → call it → parse `{projectID, firstScene{...}}` → return to QML (or
  emit error). Same envelope-parse + `scrivi_free` (RAII `ScriviString`) + no backend logic discipline.

### T-0226 — Landing view QML  *(AC1)*
`Landing.qml` is the app's root screen (replaced the EP-020 hello window). A recent-projects `ListView`
(bound to T-0224), a first-launch empty state, a **New Project** button (opens T-0227), an **Open Project**
button present but **stubbed/disabled** ("Available in the next update" → SP-060), and (added later) a
**Quit** button (`Qt.quit()`).

### T-0227 — New Project flow  *(AC2)*
Mirrors Apple `NewProjectSheet`: a title + editable slug (auto-derived, validated non-empty), plus a
parent-folder choice. Composes `<folder>/<slug>.scrivi`. On Create → `bridge.createProject(...)` → on
success add to recents (T-0224) + navigate to a placeholder project window (real editor is EP-022); on
error surface it inline.
- **Fix 1 (QML wiring):** a **binding loop** (injected props `bridge`/`recents`/`appSupportRoot` shadowed
  the outer ids on their own RHS — renamed to `bridgeRef`/`recentsRef`/`appSupportRootPath`) and a
  **Connections warning** (typed the props `ScriviBridge`/`RecentsStore` instead of `var`).
- **Fix 2 (folder picker, user-reported):** the Qt Quick `FolderDialog` was the wrong tool — it can only
  accept an existing **child** folder, so "Choose…" couldn't select the folder being viewed (e.g. an empty
  `/projects`); OK stayed disabled. **Replaced with a Qt Widgets `QFileDialog` via
  `ScriviBridge::chooseFolder(startDir)`** (`getExistingDirectory`) — selects the current folder, offers
  "New Folder", needs no `Qt.labs.folderlistmodel`. `main.cpp` now uses `QApplication`; app links
  `Qt6::Widgets`; dropped the `qtquick-dialogs`/`qt-labs-folderlistmodel` apt packages.
- **Harness:** `build-and-run.sh` bind-mounts `~/ScriviLinux/appsupport` → `/root/.local/share/Scrivi`
  (identity+recents persist across container restarts) and `~/ScriviLinux/projects` → `/projects`
  (macOS↔container shared drop-zone); the picker defaults to `/projects`.

### T-0228 — Docker/CI
- `libssl-dev` (OpenSSL 3, for the persistent secure store) in `docker/Dockerfile` **and** both CI
  workflows.
- `tests/persistence_smoke.{cpp,sh}` — a two-pass restart harness (Qt6::Core, no display) asserting
  **identity and recents persist**; wired into `scrivi-linux-ci.yml`.
- New `.cpp/.hpp/.qml/CMakeLists` are CMake-tracked only (not in `Scrivi.xcodeproj/project.pbxproj` —
  Apple-only rule).

### T-0229 — `EncryptedFileSecureStore` (Linux persistent identity)  *(AC4 — added mid-sprint, user-approved)*
**Why added:** AC4 was blocked by a discovered gap — ScriviCore's C ABI wired an **in-memory**
`PrototypeSecureStore` for every platform (`scrivi_c_api.cpp`), so the local identity was recreated on
every fresh process launch (a documented MVP placeholder, Architecture v0.3, that regressed persistence for
all platforms when the C++ adapter + Apple's `KeychainSecureStore` were retired). Proven via the Docker
persistence smoke: recents persisted, identity did not.
**Resolution (per T-0058 trade study, Option L-2):** added
`ScriviCore/src/platform/EncryptedFileSecureStore.{hpp,cpp}` — AES-256-GCM (OpenSSL 3) at
`<appSupportRoot>/secure/scrivi.identity.v1.enc`, HKDF-SHA256 key from `/etc/machine-id` + UID + username +
persisted salt. Wired into `scrivi_c_api.cpp`'s `CoreSingleton`, **gated to Linux**
(`CMAKE_SYSTEM_NAME STREQUAL "Linux"`) so Apple/Windows are untouched. Added 5 ctests
(`EncryptedSecureStoreTests.cpp`, Linux-only). Apple's identity persistence remains a separate latent gap
(not fixed here).

---

## Verification

**Verified in Docker (2026-07-14):** image builds; persistence smoke passes (identity + recents survive a
simulated restart); full Linux `ctest` = **271/271** (5 new secure-store tests); headless Xvfb GUI smoke
loads Landing.qml; macOS `ctest` = **264/264** (unchanged, secure store gated off).

**✅ User-verified over VNC (2026-07-14):** New Project creates a real `.scrivi` in `/projects` (**AC2**),
confirmed on disk **and accessible from macOS** via the shared bind mount; the Widgets `QFileDialog`
appears and allows selecting a folder incl. the default `/projects`; the created project **persists in
Recent Projects** after relaunch (**AC5**); after **Quit + relaunch** the app shows the same **"Signed in as
Scrivi"** identity with no credential/profile prompt (**AC4** — identity persisted via
`EncryptedFileSecureStore`); the landing view (**AC1**) and **Quit** button work. All of SP-059's
AC1/AC2/AC4/AC5 confirmed.

---

*Archived 2026-07-14 on SP-059 close (user-approved). All seven tasks Verified; EP-021 `[Linux]`
AC1/AC2/AC4/AC5 delivered & VNC-verified. EP-021 remains open — SP-060 is the open/close half.*
