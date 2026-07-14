# Active Tasks

**Sprint SP-060** — `[Linux]` Project Lifecycle (second of 2 EP-021 sprints): the **open + close** half —
Open Project (recents click + folder picker), the three open modes, Close → landing, and full-loop
verification. Epic: EP-021 `[Linux]`. Design: `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md`,
`ScriviCore/include/scrivi/scrivi.h` (`scrivi_open_project`). Apple analogues: `AppEnvironment.swift`
(`loadProject`/`closeProject`/open-mode branching).

> **Status: ✅ All Verified 2026-07-14** (user confirmed over VNC). SP-059 tasks T-0223–T-0229 are ✅
> Verified and archived to `Verified/Task-verified-0223-0229.md`.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0230 | `ScriviBridge.openProject` — call `scrivi_open_project`, return mode-tagged result (`ready`/`repairRequired`; `cannotOpen`→error) | ✅ Verified |
| T-0231 | Open flow QML — recents-row click + Open Project (`QFileDialog` `.scrivi`); branch 3 modes; add-or-update recents | ✅ Verified |
| T-0232 | Close → landing — `ProjectWindow` Close returns cleanly (app-side, no `scrivi_close_project`), recents refreshed | ✅ Verified |
| T-0233 | Full-loop verify — headless create→open→close→reopen smoke + CI; EP-021 AC check + Epic-close prep | ✅ Verified |

### Resolution notes (2026-07-14) — Implemented, Not Verified
- **T-0230** `ScriviBridge::openProject(projectRootPath, appSupportRoot)` — fills stashed `identityID`,
  calls `scrivi_open_project`, returns the ok `result` (`mode` + payload); `cannotOpen`/error → `errorOccurred` + `{}`.
- **T-0231** `Landing.qml` — added `openPath(path)` (branches `ready`/`repairRequired`/error), a
  **repairRequired** `Dialog` (lists `repairIssues[].title`, surface + block), enabled the **Open Project**
  button (`chooseFolder` → `openPath`) and **clickable recents rows** (`onClicked: openPath(modelData.path)`);
  `recents.addOrUpdate` on ready. `ProjectWindow.qml` generalized (created vs. opened, shows scene count).
- **T-0232** `ProjectWindow` Close → `stack.pop(null)` returns to landing; ready-open uses `stack.push`, so
  Close lands on the landing root; recents shows the project at front (add-or-update moved it there). No
  `scrivi_close_project` needed (recents app-side; core keeps no cross-call handle).
- **T-0233** `tests/lifecycle_smoke.{cpp,sh}` — two-pass create→open(ready)→close→reopen-recent harness
  (asserts stable projectID + ≥1 scene, survives restart); wired into `scrivi-linux-ci.yml`.

**Bug fixed (2026-07-14, user-reported — open did nothing):** recents-row click and the Open Project
button both failed to open. Cause: `openPath()` began with `landingPage.errorText = ""`, but `landingPage`
is a QML **`Component`**, not the instantiated Page — writing that property threw *"Cannot assign to
non-existent property errorText"* and **aborted the function before `bridge.openProject` was ever called**
(so the bug never showed in the headless smoke, which bypasses this QML path). **Fix:** hoisted the error
text to a window-level `landingError` property; the landing Page's `errorText` now binds to it read-only.
Verified in Docker: `openPath` runs through and pushes the project window (`stack.depth` 1→2), no warnings.

**Verified in Docker (2026-07-14):** lifecycle smoke **PASS** (create→open→close→reopen + restart, stable
projectID, 1 scene); persistence smoke still PASS; ScriviCore `ctest` **271/271**; app GUI loads warning-clean.
Working tree touches **only** `platforms/linux/` + Linux CI + docs — no ScriviCore/Apple source (AC8 holds;
`scrivi.h` unchanged; macOS build untouched).

**✅ User-verified over VNC (2026-07-14):** opened **two** `ready` projects — one created in-app, one from a
**previous run** (recents persistence + open together) — via both the recents list and the Open Project
button (**AC3**); a deliberately-broken copy (`the-legend-of-myton-bad`, `manuscript.meta.json` deleted)
showed the **"This project needs repair"** dialog and stayed on landing (**AC3** repairRequired = surface +
block); **Close** returned to the landing view (**AC6**); the opened project appeared at the **top** of
Recent Projects. All SP-060 AC3/AC6/AC7/AC8 confirmed.

---

## SP-060 — [Linux] Project Lifecycle (open/close half) — 🟢 Active (activated 2026-07-14)

**Epic:** EP-021 `[Linux]` · Delivers **AC3, AC6, AC7, AC8** (open project + open-mode handling; close →
landing; full-loop verification; no-regression). Closes EP-021 pending user approval.

**Inherited from SP-059 (do not re-decide):** appSupportRoot (`AppSupport`), recents (`RecentsStore`),
picker = Widgets `QFileDialog` via `ScriviBridge::chooseFolder`, identity persistence
(`EncryptedFileSecureStore`). **repairRequired UX = surface + block** (list issues, don't enter the
project; full repair deferred to a later Epic).

**Open-project envelope** (`scrivi_c_api.cpp`): `ready` → `{projectID, mode:"ready", activeScene, restored,
scenes:[...]}`; `repairRequired` → ok `{projectID, mode:"repairRequired", repairIssues:[...]}`;
`cannotOpen` → **error** envelope (surfaced by `parseEnvelope` as `errorOccurred`).

### T-0230 — `ScriviBridge.openProject(projectRootPath)`  *(AC3)*
Mirror `createProject`: fill `appSupportRoot` + stashed `identityID`, call
`scrivi_open_project(projectRootPath, appSupportRoot, identityID)`, parse the envelope. Return the ok
`result` `QVariantMap` (carries `mode`, and for `ready` the `activeScene`/`scenes`); `cannotOpen` arrives
as an error envelope → existing `parseEnvelope` emits `errorOccurred` and returns `{}`. RAII `ScriviString`
+ `scrivi_free`, no backend logic. **No ScriviCore change.**

### T-0231 — Open flow (QML)  *(AC3)*
Two entry points call `bridge.openProject(path)`:
- **Recent row click** — enable the landing recents rows; use the entry's `path`.
- **Open Project button** — reuse `bridge.chooseFolder(defaultProjectsFolder)` to pick an existing
  `.scrivi` directory.
Branch on `result.mode`: **ready** → push placeholder `ProjectWindow` (projectID/title/scene count) and
`recents.addOrUpdate(path, title)`; **repairRequired** → dialog listing `repairIssues[].title`, stay on
landing, do **not** open; **cannotOpen / error** → inline error on landing. A recent whose path no longer
resolves surfaces the bridge error (offer removal from recents).

### T-0232 — Close → landing  *(AC6)*
Confirm no core-side close is required (recents app-side; ScriviCore keeps no cross-call open handle).
`ProjectWindow`'s Close returns to the landing `StackView` root; the just-closed project appears at the
**front** of recents (newest-first); error state cleared. Purely a UI/state transition.

### T-0233 — Full-loop verify + CI  *(AC7, AC8)*
Headless smoke driving **create → open (ready) → close (state reset) → reopen-from-recent** against a temp
appSupportRoot: assert the reopened project matches (stable projectID, scene present) and recents survives
an app restart. Wire into `scrivi-linux-ci.yml`. Then walk **all EP-021 AC1–AC8** for the Epic-close check;
`scrivi.h` unchanged/additive, ScriviCore `ctest` green, macOS build untouched.

**Exit criteria:** Over VNC, Open Project (picker) and clicking a recent both open a `ready` project into
the placeholder window; `repairRequired` shows its issues and stays on landing; an unreadable path shows an
inline error; Close returns to landing with the project at the front of recents; the full
create→open→close→reopen-recent loop works and persists across restart. CI green (build + headless
full-loop smoke). ScriviCore `ctest` + macOS build untouched. EP-021 ready to close pending user approval.

---

*Last Updated: 2026-07-14 (SP-060 🟢 activated — EP-021 close half; tasks T-0230–T-0233. SP-059 tasks
archived to Verified/Task-verified-0223-0229.md.)*
