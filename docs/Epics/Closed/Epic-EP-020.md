# EP-020: [Linux] App Foundation — Qt/QML Toolchain, Bridge & App Shell

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — touches `[ScriviCore]` only via the
existing C ABI (no core changes made).

**Status:** ✅ Closed (user-approved 2026-07-13)
**Goal:** A running, professional-feeling Qt6/QML Ubuntu app *shell* that links `libScriviCore.a`, calls
ScriviCore through the existing plain-C ABI (`scrivi.h`) via a thin C++ `ScriviBridge`, and is
build-and-run verifiable in a **Docker + Xvfb/VNC desktop** on the developer's Mac and green in CI. This
Epic delivered the toolchain, the bridge pattern, the app-window shell, and a "hello ScriviCore" vertical
slice — the working spine every subsequent `[Linux]` Epic builds screens onto. It did **not** deliver any
real editing screen (those are EP-021+).

**Strategy:** First of the `[Linux]` Epic family (EP-020–EP-026) porting the macOS app's capabilities to
Ubuntu for an alpha tester who has no Mac. Ubuntu first because it is CI-verifiable via Docker. Qt/QML per
the approved architecture: a real native desktop app (not a web app, not an immediate-mode GUI), reusable
later for the Windows Epic family.

**Design references:** `platforms/linux/README.md`, `ScriviCore/include/scrivi/scrivi.h`,
`docs/Scrivi_Apple_Wrapper_Design_v0_1.md`.

**Date Created:** 2026-07-13
**Actual Close Date:** 2026-07-13

## Acceptance Criteria — all met

- [x] AC1 — Qt6/QML app target under `platforms/linux/`, built by CMake, linking `libScriviCore.a` and
  including `scrivi.h` directly (same-process C++ calls; no adapter). *Verified: image builds + links
  `scrivi_linux`.*
- [x] AC2 — `ScriviBridge` C++ class marshals QML calls to `scrivi_*`, parses the `{"ok":...}` envelope,
  surfaces typed results/errors to QML, and `scrivi_free`s every returned pointer (RAII `ScriviString`
  guard). No backend logic. *Verified by inspection + round-trip.*
- [x] AC3 — Docker image builds the app and runs it headless under Xvfb + a VNC server, viewable from the
  Mac over VNC. *Verified: developer connected via VNC.*
- [x] AC4 — "Hello ScriviCore" QML window calls `scrivi_ensure_local_identity` end-to-end and displays
  the decoded result. *Verified: user clicked the control and saw the green identity result over VNC —
  full QML → bridge → C ABI → ScriviCore → JSON → QML round-trip.*
- [x] AC5 — CI builds the Linux Qt/QML app on relevant commits with a headless smoke test; ScriviCore
  `ctest` job unchanged. *Verified: `Scrivi Linux App CI` green (run 29283026516, 58s).*
- [x] AC6 — No regression to ScriviCore or the Apple app: `scrivi.h` unchanged, macOS build unaffected.
  *Verified: no `ScriviCore/` or `Scrivi/` (Apple) source changed; root `CMakeLists.txt` change is
  additive (`SCRIVI_BUILD_LINUX`, off by default). `platforms/linux` added to Xcode as a browse-only
  folder reference (not compiled into any Apple target).*

## Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-058 | Docker/Xvfb/VNC harness + Qt6 skeleton + hello-ScriviCore slice + CI job | ✅ Closed | 2026-07-13 – 2026-07-13 |

## Completion Summary

EP-020 stood up the entire Linux toolchain and proved the spine end-to-end in a single sprint (SP-058).
The Qt6/QML app (`platforms/linux/`, Ubuntu 24.04 + Qt 6.4) links `libScriviCore.a`, calls the plain-C
ABI through a thin `ScriviBridge` (the QML analogue of `ScriviEngine.swift`, no backend logic), and shows
a decoded `scrivi_ensure_local_identity` result in a real Qt window — user-verified over VNC. A
Docker + Xvfb + x11vnc harness lets the Mac developer *see* the running GUI (host port 5901, password
`scrivi`), and CI builds + headless-smoke-tests the app green on every relevant commit. ScriviCore and the
Apple app are untouched.

**Key decisions / artifacts that later `[Linux]` Epics inherit:**
- **Toolchain pin:** Ubuntu 24.04 LTS + Qt 6.4 (apt) + C++23 + CMake/Ninja. Target Qt 6.4's actual API
  surface, not a newer one (several 6.5+ assumptions in the first draft had to be corrected).
- **Bridge pattern:** `ScriviBridge` = `ScriviEngine.swift` — JSON-over-string boundary, `scrivi_free`
  discipline via RAII, no backend logic. Every later screen calls this.
- **Build wiring:** `-DSCRIVI_BUILD_LINUX=ON` gates the Qt app; it reuses the root ScriviCore CMake
  target. Linux source is CMake-only (not in the Apple pbxproj, per CLAUDE.md) — but is browsable in
  Xcode via a folder reference.
- **Developer VNC harness:** `platforms/linux/docker/` (Dockerfile + run-vnc.sh + build-and-run.sh), host
  port 5901 + VNC password (macOS Screen Sharing constraints), `x11-utils` for the Xvfb readiness check.
- **Runtime QML modules** are separate apt packages (`qtquick-templates`, `qtqml-workerscript`,
  `qtqml-models`) — a clean compile does not catch a missing one; both Dockerfile and CI list them.

**Verification model proven:** CI covers build + headless smoke; the developer covers the GUI visually
over VNC; the alpha tester later covers real Ubuntu. The split caught issues (missing runtime modules, the
never-installed `xdpyinfo`) that neither path alone would have.

**Next:** EP-021 `[Linux]` — Project Lifecycle & Landing (create/open/close `.scrivi`, landing view),
building the first real screens on this spine.

---

*Closed 2026-07-13 — first `[Linux]` Epic delivered in one sprint (SP-058); all AC1–AC6 met and verified.
User-approved close.*
