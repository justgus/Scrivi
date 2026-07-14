# Active Sprint

## SP-060: [Linux] Project Lifecycle — Open + Close + full-loop Verify (EP-021 close half)

**Status:** 🟢 Active
**Activated:** 2026-07-14
**Epic:** **EP-021** `[Linux]` — Project Lifecycle & Landing (second of 2 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) — calls `[ScriviCore]` only via the existing C ABI through
`ScriviBridge`. **No ScriviCore source change expected:** `scrivi_open_project` already exists and close is
app-side (no `scrivi_close_project` endpoint — confirmed 2026-07-13). Additive-only if anything.
**Goal:** Close the Linux project-lifecycle loop on top of SP-059's create half. Add **Open Project**
(recents row click **and** folder picker), handle the **three open modes** (`ready` / `repairRequired` /
`cannotOpen`), a **Close → return to landing** transition, and prove the full
**create → open → close → reopen-recent** loop over Docker/VNC + a headless CI smoke. Delivers **EP-021
AC3, AC6, AC7, AC8** and closes the Epic (pending user approval).

**Design reference:** `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md` (open flow + modes),
`docs/Scrivi_External_Change_Repair_Matrix_v0_2.md` (repair issue shape), `ScriviCore/include/scrivi/scrivi.h`
(`scrivi_open_project`). Apple analogues (behavior, not code): `AppEnvironment.swift`
(`loadProject`/`openProjectFromPickedURL`/`closeProject`, open-mode branching), `ProjectSession.load`.

### Inherited decisions (resolved in SP-059 — do not re-litigate)
- **appSupportRoot** = `$XDG_DATA_HOME/Scrivi` else `~/.local/share/Scrivi` (`AppSupport` helper). ✅
- **Recents** = `recents.json` under appSupportRoot (`RecentsStore`, add-or-update/remove/list). ✅
- **Folder/file picker** = Qt **Widgets `QFileDialog`** via `ScriviBridge` (not Qt Quick FolderDialog — it
  couldn't select the viewed folder and needed extra QML modules). ✅ Reuse for the Open picker.
- **Identity** persists via `EncryptedFileSecureStore` (Linux-gated). ✅
- **repairRequired UX (decided 2026-07-14):** **surface + block** — list the issue titles in a dialog and
  do **not** enter the project; full repair workflow (`scrivi_apply_repair`) is deferred to a later Epic.

### Open-project envelope (from `scrivi_c_api.cpp`, for the bridge/QML)
- **`ready`** → `{"ok":true,"result":{projectID, mode:"ready", activeScene{sceneID,metadataPath,contentPath,markdown}, restored{anchor,focus,scroll}, scenes:[...]}}`
- **`repairRequired`** → `{"ok":true,"result":{projectID, mode:"repairRequired", repairIssues:[{...}]}}`
- **`cannotOpen`** → **error** envelope (`{"ok":false,"error":{code,message}}`) — surfaced by the bridge's
  existing `parseEnvelope` as `errorOccurred`.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0230 | `ScriviBridge.openProject(projectRootPath)` — fill appSupportRoot + identity, call `scrivi_open_project`, return mode-tagged result (`ready`/`repairRequired`; `cannotOpen`→error) | High | ✅ Verified |
| T-0231 | Open flow in QML — live recents-row click **and** Open Project → `QFileDialog` pick `.scrivi`; branch the three modes (ready→project window; repairRequired→issues dialog, stay on landing; cannotOpen→inline error); add-or-update recents on open | High | ✅ Verified |
| T-0232 | Close → landing — `ProjectWindow` Close returns to the landing view cleanly (app-side; no `scrivi_close_project`), recents refreshed/newest-first | Medium | ✅ Verified |
| T-0233 | Full-loop verify — headless smoke (create→open→close→reopen-recent) + CI; EP-021 acceptance-criteria verification & Epic-close prep | High | ✅ Verified |

> **✅ User-verified over VNC (2026-07-14):** opened two `ready` projects (one in-app-created, one from a
> **previous run**) via both the recents list and the Open Project button (**AC3**); a broken copy
> (`manuscript.meta.json` deleted) showed **"This project needs repair"** and stayed on landing (AC3
> repairRequired = surface + block); **Close** returned to landing (**AC6**); opened project moved to the
> **top** of recents. Backed by: lifecycle smoke PASS, persistence smoke PASS, ScriviCore `ctest` 271/271,
> no ScriviCore/Apple change (AC8). **A GUI open bug found + fixed during verification:** `openPath` wrote
> `landingPage.errorText` (a `Component`, not the Page) → exception aborted the open before it ran; fixed by
> hoisting error text to a window-level `landingError`. **All AC3/AC6/AC7/AC8 met — EP-021 ready to close
> pending user approval.**

### Plan notes
- **T-0230 — Bridge.openProject.** Mirror `createProject`: fill appSupportRoot + stashed identityID, call
  `scrivi_open_project(projectRootPath, appSupportRoot, identityID)`, parse the envelope. `ready` and
  `repairRequired` come back on the ok path (return the `QVariantMap` incl. `mode`); `cannotOpen` arrives
  as an error envelope → existing `parseEnvelope` emits `errorOccurred` and returns an empty map. Same
  RAII `ScriviString` / no-backend-logic discipline. **No ScriviCore change.**
- **T-0231 — Open flow (QML).** Two entry points: (a) clicking a **recent row** (rows become enabled;
  `path` from the recents entry), (b) **Open Project** button → `bridge.chooseFolder(defaultProjectsFolder)`
  reused to pick an existing `.scrivi` directory. Both call `bridge.openProject(path)` then branch on
  `result.mode`: **ready** → push the placeholder `ProjectWindow` (show projectID/title/scene count) and
  `recents.addOrUpdate(path, title)`; **repairRequired** → a dialog listing `repairIssues[].title`, stay on
  landing, do not open; **cannotOpen / error** → inline error label on landing (via `errorOccurred`). A
  recent whose path no longer resolves surfaces the bridge error (and can be removed from recents).
- **T-0232 — Close → landing.** Confirm no core-side close is needed (recents are app-side; ScriviCore
  holds no open handle across C-ABI calls). `ProjectWindow`'s existing Close returns to the landing
  `StackView` root; ensure recents shows the just-closed project at the front (newest-first) and any
  error state is cleared. Purely a UI/state transition.
- **T-0233 — Full-loop verify + CI.** Extend the headless harness (or add a second one) to drive
  create → open (ready) → "close" (state reset) → reopen-from-recent against a temp appSupportRoot,
  asserting the reopened project matches (projectID stable, scene present) and recents is intact across a
  restart. Wire into `scrivi-linux-ci.yml`. Then walk every EP-021 AC (AC1–AC8) for the Epic-close check.

**Exit criteria:** Over VNC: from the landing view, **Open Project** (folder picker) **and** clicking a
**recent** both open a `ready` project into the placeholder window; a `repairRequired` project shows its
issue list and stays on landing; an unreadable path shows an inline error; **Close** returns to landing
with the project at the front of recents; the full **create → open → close → reopen-recent** loop works and
**persists across app restart**. CI green (build + headless full-loop smoke). ScriviCore `ctest` + macOS
build untouched. On success, **EP-021 is ready to close** (all AC1–AC8 met) pending user approval.

---

*Last Updated: 2026-07-14 (SP-060 🟢 activated — EP-021 `[Linux]` close half: open + close + full-loop
verify (AC3/AC6/AC7/AC8). Tasks T-0230–T-0233. Inherits SP-059's appSupportRoot/recents/QFileDialog/
identity decisions; repairRequired = surface+block. First: T-0230 bridge.openProject.)*
