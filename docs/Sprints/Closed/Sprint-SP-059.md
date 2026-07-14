# SP-059: [Linux] Project Lifecycle — appSupportRoot + recents store + landing view + create-project

**Status:** ✅ Closed
**Epic:** EP-021 `[Linux]` — Project Lifecycle & Landing (first of 2 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) + `[ScriviCore]` (secure store — added mid-sprint, see below).
**Activated:** 2026-07-13
**Closed:** 2026-07-14 (user-approved)
**Goal:** Give the Linux app a persistent home and its first real end-to-end path: a stable per-user
`appSupportRoot`, a recent-projects store, a landing view, and a New Project flow that calls
`scrivi_create_project` to write a real `.scrivi` on disk and transitions to a placeholder project window.
Delivered **EP-021 AC1, AC2, AC4, AC5** (the create half; open/close/verify is SP-060).

**Design reference:** `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md`,
`docs/Scrivi_Project_Package_Structure_v0_1.md`, `ScriviCore/include/scrivi/scrivi.h`,
`docs/Scrivi_SecureStore_Platform_Trade_Study_v0_1.md` (T-0058, Option L-2). Apple behavior analogues:
`AppEnvironment.swift`, `NewProjectSheet.swift`, `ProjectBookmarkStore`.

## Assigned Tasks — all ✅ Verified

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0223 | XDG `appSupportRoot` resolver (`$XDG_DATA_HOME`/`~/.local/share/Scrivi`, created if absent) | High | ✅ Verified |
| T-0224 | Recent-projects store — JSON under appSupportRoot: add/update/remove/list, survives restart | High | ✅ Verified |
| T-0225 | `ScriviBridge` lifecycle methods — launch identity bootstrap + `createProject` | High | ✅ Verified |
| T-0226 | Landing view QML — recents list + empty state + New/Open actions (Open stubbed → SP-060) | High | ✅ Verified |
| T-0227 | New Project flow — title→slug→`<folder>/<slug>.scrivi`, create, add to recents, placeholder window | High | ✅ Verified |
| T-0228 | Docker/CI — apt lists (libssl-dev); headless persistence smoke | Medium | ✅ Verified |
| T-0229 | `EncryptedFileSecureStore` (Linux, ScriviCore) — persistent identity so AC4 holds (added mid-sprint) | High | ✅ Verified |

## Exit criteria — met

Over VNC the app opened to a **landing view** (AC1); **New Project** created a real `.scrivi` in `/projects`
(AC2) — confirmed on disk **and** accessible from macOS via the shared bind mount; the project **persisted
in Recent Projects** after relaunch (AC5); **Quit + relaunch** showed the same **"Signed in as Scrivi"**
identity with no credential/profile prompt (AC4 — persistence via `EncryptedFileSecureStore`). CI green
(build + headless persistence smoke). ScriviCore `ctest` = **271/271** (5 new secure-store tests); macOS
`ctest` = **264/264** (unchanged — secure store gated to Linux). User-verified over VNC 2026-07-14.

## Scope amendment (mid-sprint, user-approved)

The sprint's original premise — "**No ScriviCore source changes expected**" — did not hold. AC4 (identity
persists across restart) was blocked: ScriviCore's C ABI wired an **in-memory** `PrototypeSecureStore` (a
documented MVP placeholder, Architecture v0.3) for every platform, so the local identity was recreated on
each fresh launch. This was proven by the Docker persistence smoke (recents persisted; identity did not).
**T-0229** was added and user-approved to implement the already-designed `EncryptedFileSecureStore` (T-0058
trade study, Option L-2: AES-256-GCM/OpenSSL at `<appSupportRoot>/secure/`), **gated to Linux** so
Apple/Windows wiring is untouched. This also restores an identity-persistence guarantee Apple lost when its
`KeychainSecureStore` was deleted alongside the retired C++ adapter (a latent gap on Apple, not fixed here).

## Retrospective

**What went well**
- The `.scrivi` create result and the bridge/store contracts matched the C ABI on first read; the create
  path worked end-to-end once the secure store persisted.
- **Investigate-before-descope paid off.** When AC4 failed, digging into the trade study revealed the fix
  was already designed and scoped as "sprint-level work" — so the right move was to implement it, not defer.
- Bind-mounting the app-support dir + a shared `/projects` folder let the developer *visually* verify
  restart-persistence and inspect the real package from macOS — the split (CI + automated smoke + human VNC)
  caught what neither alone would.

**What didn't (and the fixes)**
- **The Qt Quick `FolderDialog` was the wrong tool.** It can only accept an existing *child* folder, so
  "Choose…" couldn't select the folder being viewed (e.g. an empty `/projects`) — OK stayed disabled.
  Replaced with a Qt Widgets `QFileDialog` via `ScriviBridge::chooseFolder` (`getExistingDirectory`);
  app now links `Qt6::Widgets` and uses `QApplication`. Dropped the `QtQuick.Dialogs` /
  `Qt.labs.folderlistmodel` dependency entirely.
- **Runtime QML module trap again (EP-020 lesson repeated).** The first FolderDialog attempt silently
  failed because the non-native dialog pulls in `Qt.labs.folderlistmodel`, a separate apt package — a clean
  compile didn't catch it. (Now moot after the QFileDialog swap.)
- **QML binding hygiene:** injected props named the same as outer ids (`bridge`/`recents`) caused a binding
  loop; renamed to `…Ref`/`…Path` and typed them concretely (`ScriviBridge`/`RecentsStore`) to also clear a
  `Connections` warning.
- **No clean Quit / ephemeral container.** The app had no quit action, and killing the process took the
  `--rm` container (and the test project) down. Added a **Quit** button (`Qt.quit()`) and bind mounts so
  state survives container restarts.

**Adjust going forward**
- For "choose a destination folder" flows on Qt, prefer the Widgets `QFileDialog` over the Quick
  `FolderDialog` — the latter's child-selection model is wrong for create/save-as and drags in an extra
  runtime module.
- SP-060 inherits the persistent `appSupportRoot`, the recents store, `EncryptedFileSecureStore`, the
  landing view, and the Docker mounts; it adds open/close + open-mode handling + full-loop verification.

---

*Closed 2026-07-14 — first sprint of EP-021 `[Linux]`. All seven tasks Verified (T-0229 added mid-sprint,
user-approved); EP-021 AC1/AC2/AC4/AC5 delivered & user-verified over VNC. EP-021 remains open (SP-060 is
the open/close half). User-approved close.*
