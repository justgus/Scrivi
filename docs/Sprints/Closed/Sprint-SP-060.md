# SP-060: [Linux] Project Lifecycle — Open + Close + full-loop Verify (EP-021 close half)

**Status:** ✅ Closed
**Epic:** EP-021 `[Linux]` — Project Lifecycle & Landing (second of 2 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — no ScriviCore source change (open endpoint already
existed; close is app-side).
**Activated:** 2026-07-14
**Closed:** 2026-07-14 (user-approved)
**Goal:** Close the Linux project-lifecycle loop on top of SP-059's create half: add **Open Project**
(recents-row click **and** folder picker), handle the **three open modes** (`ready` / `repairRequired` /
`cannotOpen`), a **Close → return to landing** transition, and prove the full
**create → open → close → reopen-recent** loop over Docker/VNC + a headless CI smoke. Delivered **EP-021
AC3, AC6, AC7, AC8** — the sprint that closes the Epic.

**Design reference:** `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md`,
`docs/Scrivi_External_Change_Repair_Matrix_v0_2.md`, `ScriviCore/include/scrivi/scrivi.h`
(`scrivi_open_project`). Apple analogue: `AppEnvironment.swift` open-mode branching.

## Assigned Tasks — all ✅ Verified

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0230 | `ScriviBridge.openProject` — call `scrivi_open_project`, return mode-tagged result (`ready`/`repairRequired`; `cannotOpen`→error) | High | ✅ Verified |
| T-0231 | Open flow QML — recents-row click + Open Project (`QFileDialog` `.scrivi`); branch 3 modes; add-or-update recents | High | ✅ Verified |
| T-0232 | Close → landing — `ProjectWindow` Close returns cleanly (app-side; no `scrivi_close_project`), recents refreshed | Medium | ✅ Verified |
| T-0233 | Full-loop verify — headless create→open→close→reopen smoke + CI; EP-021 AC check + Epic-close prep | High | ✅ Verified |

## Exit criteria — met

Over VNC: from the landing view, **Open Project** (folder picker) **and** clicking a **recent** both opened
a `ready` project into the placeholder window — verified with two projects, one created in-app and one from
a **previous run** (recents + open together); a `repairRequired` project (broken `manuscript.meta.json`)
showed its issue dialog and stayed on landing; **Close** returned to landing with the project at the
**top** of recents. The `lifecycle_smoke` (create → open → close → reopen-recent + restart, stable
projectID + ≥1 scene) is wired into CI and green; persistence smoke still green; ScriviCore `ctest`
**271/271**; macOS untouched (`scrivi.h` unchanged, no ScriviCore/Apple source changed). User-verified over
VNC 2026-07-14.

## Retrospective

**What went well**
- **The design questions were already answered.** Most of SP-060's "open questions" were resolved by SP-059
  (appSupportRoot, recents, the QFileDialog picker, identity persistence) — so the sprint was a focused
  open/close/verify pass, not a fresh investigation. Confirming "no `scrivi_close_project` needed" up front
  (ScriviCore keeps no cross-call open handle) kept it app-side.
- **The headless lifecycle smoke earned its keep** — it proved the create→open→close→reopen loop (and its
  restart persistence) at the data level in CI, independent of the GUI.

**What didn't (and the fixes)**
- **A QML-scope bug silently broke Open.** `openPath()` began with `landingPage.errorText = ""`, but
  `landingPage` is a **`Component`**, not the instantiated Page — the write threw *"Cannot assign to
  non-existent property errorText"* and **aborted the whole function before `bridge.openProject` ran**, so
  both recents-click and Open Project did nothing. The headless smoke didn't catch it (it bypasses the QML
  path). **Fix:** hoisted the error text to a window-level `landingError` property; the Page's `errorText`
  binds to it read-only. **Lesson:** a QML `Component` id is not its instantiated item — don't write
  properties on it; and a smoke that exercises the bridge is not a substitute for driving the GUI.
- **Repair-test recipe surfaced for the tester.** To force `repairRequired`, delete a blocking metadata file
  (simplest: `manuscript/manuscript.meta.json`) — verified against `ProjectValidator`/`ProjectOpener`.

**Adjust going forward**
- GUI-affecting QML changes need a real click-through over VNC before "verified" — the developer caught
  the open bug that neither CI nor the headless smoke could.
- EP-022 (writing surface) inherits the whole lifecycle: landing, create, open (all modes), close, recents,
  persistent identity, the QFileDialog picker, and the Docker mounts.

---

*Closed 2026-07-14 — second and final sprint of EP-021 `[Linux]`. All four tasks Verified; EP-021
AC3/AC6/AC7/AC8 delivered & user-verified over VNC (with the `landingError` open bug found + fixed during
verification). Closes EP-021. User-approved close.*
