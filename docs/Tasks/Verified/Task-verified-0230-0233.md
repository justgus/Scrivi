# Verified Tasks — T-0230–T-0233 (SP-060, EP-021 [Linux] Project Lifecycle — open/close half)

_Archived 2026-07-14 on SP-060 / EP-021 close. All four tasks ✅ Verified. Original active-task detail
preserved below verbatim._


**Sprint SP-060** — `[Linux]` Project Lifecycle (second of 2 EP-021 sprints): the **open + close** half —
Open Project (recents click + folder picker), the three open modes, Close → landing, and full-loop
verification. Epic: EP-021 `[Linux]`. Design: `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md`,
`ScriviCore/include/scrivi/scrivi.h` (`scrivi_open_project`).

| ID | Title | Status |
| -- | ----- | ------ |
| T-0230 | `ScriviBridge.openProject` — call `scrivi_open_project`, return mode-tagged result (`ready`/`repairRequired`; `cannotOpen`→error) | ✅ Verified |
| T-0231 | Open flow QML — recents-row click + Open Project (`QFileDialog` `.scrivi`); branch 3 modes; add-or-update recents | ✅ Verified |
| T-0232 | Close → landing — `ProjectWindow` Close returns cleanly (app-side, no `scrivi_close_project`), recents refreshed | ✅ Verified |
| T-0233 | Full-loop verify — headless create→open→close→reopen smoke + CI; EP-021 AC check + Epic-close prep | ✅ Verified |

---

**Open-project envelope** (`scrivi_c_api.cpp`): `ready` → `{projectID, mode:"ready", activeScene, restored,
scenes:[...]}`; `repairRequired` → ok `{projectID, mode:"repairRequired", repairIssues:[...]}`;
`cannotOpen` → **error** envelope (surfaced by `parseEnvelope` as `errorOccurred`).

### T-0230 — `ScriviBridge.openProject(projectRootPath, appSupportRoot)`  *(AC3)*
Mirrors `createProject`: fills `appSupportRoot` + stashed `identityID`, calls `scrivi_open_project`, parses
the envelope. Returns the ok `result` `QVariantMap` (carries `mode`, and for `ready` the
`activeScene`/`scenes`); `cannotOpen` arrives as an error envelope → existing `parseEnvelope` emits
`errorOccurred` and returns `{}`. RAII `ScriviString` + `scrivi_free`, no backend logic. No ScriviCore change.

### T-0231 — Open flow (QML)  *(AC3)*
Two entry points call `bridge.openProject(path)`: a **recent row click** (rows enabled; entry's `path`) and
the **Open Project** button (reuses `bridge.chooseFolder(defaultProjectsFolder)` to pick an existing
`.scrivi`). `Landing.qml` gained a shared `openPath()` branching on `result.mode`: **ready** → push the
placeholder `ProjectWindow` + `recents.addOrUpdate`; **repairRequired** → a `Dialog` listing
`repairIssues[].title`, stay on landing, don't open; **cannotOpen / error** → inline error via
`errorOccurred`. `ProjectWindow.qml` generalized (created vs. opened, shows scene count).
- **Bug fixed (user-reported, open did nothing):** `openPath()` wrote `landingPage.errorText`, but
  `landingPage` is a QML **`Component`**, not the instantiated Page — the write threw and aborted the
  function before `bridge.openProject` ran. Fixed by hoisting the error text to a window-level
  `landingError` property (Page's `errorText` binds to it read-only). Never showed in the headless smoke
  (which bypasses the QML path).

### T-0232 — Close → landing  *(AC6)*
No core-side close required (recents app-side; ScriviCore keeps no cross-call open handle). `ProjectWindow`
Close → `stack.pop(null)` returns to the landing root; ready-open uses `stack.push`, so Close lands on
landing; the just-opened project sits at the **front** of recents (add-or-update moved it there). Purely a
UI/state transition.

### T-0233 — Full-loop verify + CI  *(AC7, AC8)*
`tests/lifecycle_smoke.{cpp,sh}` — a two-pass **create → open (ready) → close → reopen-from-recent** harness
(asserts stable projectID + ≥1 scene, survives a restart), wired into `scrivi-linux-ci.yml`. Walked all
EP-021 AC1–AC8 for the Epic-close check; `scrivi.h` unchanged, ScriviCore `ctest` green, macOS untouched.

**Repair-test recipe (for the tester):** to force `repairRequired`, delete a blocking metadata file — the
simplest is `manuscript/manuscript.meta.json` ("Missing manuscript.meta.json"); corrupting it, or deleting
`chapter.meta.json` / the scene `.md` / `project.json` also work (verified against `ProjectValidator` /
`ProjectOpener`). A non-`.scrivi` folder yields the `cannotOpen`/error path (inline error).

---

## Verification

**Verified in Docker (2026-07-14):** lifecycle smoke PASS (create→open→close→reopen + restart, stable
projectID, 1 scene); persistence smoke still PASS; ScriviCore `ctest` **271/271**; GUI loads warning-clean.
Working tree touched **only** `platforms/linux/` + Linux CI + docs (no ScriviCore/Apple source — AC8 holds).

**✅ User-verified over VNC (2026-07-14):** opened **two** `ready` projects — one created in-app, one from a
**previous run** (recents persistence + open together) — via both the recents list and the Open Project
button (**AC3**); a deliberately-broken copy (`manuscript.meta.json` deleted) showed the **"This project
needs repair"** dialog and stayed on landing (**AC3** repairRequired = surface + block); **Close** returned
to the landing view (**AC6**); the opened project appeared at the **top** of Recent Projects. CI green on
the committed tree. All SP-060 AC3/AC6/AC7/AC8 confirmed.

---

*Archived 2026-07-14 on SP-060 / EP-021 close (user-approved). All four tasks Verified; EP-021
AC3/AC6/AC7/AC8 delivered & VNC-verified. Closes EP-021.*
