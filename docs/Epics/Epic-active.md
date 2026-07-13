# Active Epics

## EP-020: [Linux] App Foundation — Qt/QML Toolchain, Bridge & App Shell

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — touches `[ScriviCore]` only via the
existing C ABI (no core changes expected; any needed endpoint is a Task with a note).

**Status:** 🟡 Active (created & activated 2026-07-13)
**Goal:** A running, professional-feeling Qt6/QML Ubuntu app *shell* that links `libScriviCore.a`, calls
ScriviCore through the existing plain-C ABI (`scrivi.h`) via a thin C++ `ScriviBridge`, and is
build-and-run verifiable in a **Docker + Xvfb/VNC desktop** on the developer's Mac and green in CI. This
Epic delivers the toolchain, the bridge pattern, the app-window shell, and a "hello ScriviCore" vertical
slice — the working spine every subsequent `[Linux]` Epic builds screens onto. It does **not** deliver
any real editing screen (those are EP-021+).

**Strategy:** First of the `[Linux]` Epic family (EP-020–EP-026) that ports the macOS app's capabilities
to Ubuntu for an alpha tester who has no Mac. Ubuntu chosen first because it is CI-verifiable via Docker.
Qt/QML chosen per the approved architecture (`docs/Scrivi_Cpp24_Core_Repository_Skeleton_v0_2.md`,
`platforms/linux/README.md`): a real native desktop app (explicitly **not** a web app, **not** an
immediate-mode "toy" GUI), reusable later for the Windows `[Windows]` Epic family.

**Design references:** `platforms/linux/README.md` (Qt/QML + CMake + direct C++ calls),
`ScriviCore/include/scrivi/scrivi.h` (the 65-function C ABI the UI consumes),
`docs/Scrivi_Apple_Wrapper_Design_v0_1.md` (the Swift `ScriviEngine` the QML `ScriviBridge` mirrors —
same JSON-over-string boundary, no backend logic in the UI layer).

**Date Created:** 2026-07-13
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] AC1 — A Qt6/QML app target exists under `platforms/linux/`, built by CMake, linking
  `libScriviCore.a` and including `scrivi.h` directly (same-process C++ calls; no adapter).
- [ ] AC2 — A `ScriviBridge` C++ class marshals QML calls to `scrivi_*` functions, parses the
  `{"ok":...}` JSON envelope, surfaces results/errors to QML as typed values, and calls `scrivi_free`
  on every returned pointer (no leaks). This is the QML analogue of `ScriviEngine.swift` — it contains
  **no** backend logic.
- [ ] AC3 — A Docker image builds the Linux app (Qt6 + CMake) and runs it headless under Xvfb with a
  VNC server, so the developer can open the running GUI from the Mac over VNC.
- [ ] AC4 — A "hello ScriviCore" QML window calls at least one real `scrivi_*` function end-to-end
  (e.g. `scrivi_ensure_local_identity`) and displays the decoded result in a real Qt window — proving
  the full QML → bridge → C ABI → ScriviCore → JSON → QML round-trip.
- [ ] AC5 — CI builds the Linux Qt/QML app on every relevant commit (the ScriviCore `ctest` job is
  unchanged; a new job compiles the GUI and runs any headless smoke test).
- [ ] AC6 — No regression to ScriviCore or the Apple app: `scrivi.h` unchanged (or additive-only),
  backend `ctest` green, macOS build unaffected.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-058 | Docker/Xvfb/VNC harness + Qt6 skeleton + hello-ScriviCore slice + CI job | 🟢 Active | 2026-07-13 – (in progress) |

### Tasks

_(defined at sprint activation — see `docs/Tasks/Task-active.md` / `Task-backlog.md`)_

### Issues

_(none yet)_

### Scope Notes

- **Out of scope (later `[Linux]` Epics):** landing/project lifecycle UI (EP-021), writing surface +
  navigator (EP-022), structure editing (EP-023), inspector (EP-024), timeline (EP-025), undo/menus/
  settings + parity verification (EP-026). EP-020 delivers only the toolchain, bridge, shell, and
  hello-slice.
- **Bridge pattern:** `ScriviBridge` mirrors `ScriviEngine.swift` — same JSON-over-`std::string`
  boundary, same "UI layer holds no backend logic" rule. macOS App-layer *behavior* (sessions,
  bookmarks, window-frame persistence, history capture) is re-created in C++/QML screen-by-screen in the
  later Epics as each screen needs it, not ported wholesale here.
- **Verification model:** CI proves *it builds + core logic works headless*; the developer proves *the
  GUI works* visually via Docker+VNC; the alpha tester later proves it on real Ubuntu. CI alone cannot
  verify a GUI.

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-13 (EP-020 [Linux] created & activated — Qt/QML foundation for the Ubuntu app; SP-058 activated. First of the EP-020–EP-026 [Linux] family. See Epic-backlog.md for EP-021–EP-026.)*
