# Active Sprint

## SP-058: [Linux] Docker/VNC harness + Qt6 skeleton + hello-ScriviCore slice + CI

**Status:** 🟢 Active
**Epic:** EP-020 `[Linux]` — App Foundation
**Codebase:** `[Linux]` (`platforms/linux/`) + build tooling; no ScriviCore source changes expected.
**Activated:** 2026-07-13
**Goal:** Stand up the whole Linux toolchain and prove the spine end-to-end: a Qt6/QML app under
`platforms/linux/` that links `libScriviCore.a`, calls one real `scrivi_*` function through a thin C++
`ScriviBridge`, and shows the decoded result in a real window — buildable and runnable in a Docker +
Xvfb/VNC desktop the developer opens from the Mac, and green in CI. Delivers **EP-020 AC1–AC5**.

**Design reference:** `platforms/linux/README.md` (Qt/QML + CMake + direct C++ calls),
`ScriviCore/include/scrivi/scrivi.h` (C ABI), `docs/Scrivi_Apple_Wrapper_Design_v0_1.md` (the
`ScriviEngine.swift` pattern the QML `ScriviBridge` mirrors).

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0218 | `platforms/linux/` Qt6/QML CMake skeleton — app target links `libScriviCore.a` + `scrivi.h` | High | 🟢 Active |
| T-0219 | `ScriviBridge` (C++/Qt) — QML ↔ C ABI marshalling, JSON-envelope parse, `scrivi_free` discipline | High | 🟢 Active |
| T-0220 | Hello-ScriviCore QML window — call `scrivi_ensure_local_identity`, render decoded result | High | 🟢 Active |
| T-0221 | Docker image (Qt6 + CMake build) + Xvfb + VNC — run the GUI headless, viewable from the Mac | High | 🟢 Active |
| T-0222 | CI job — build the Linux Qt/QML app (+ any headless smoke) on relevant commits | Medium | 🟢 Active |

### Plan notes
- **Step 0 — Qt version + base image decision (before T-0218):** pin the Qt6 minor version and the
  Docker base (e.g. an Ubuntu LTS matching the tester's) and record it in `platforms/linux/README.md`,
  since every later `[Linux]` Epic inherits it. Confirm CMake can find both Qt6 and the ScriviCore
  static target from `platforms/linux/CMakeLists.txt`.
- **T-0218 — Skeleton.** `platforms/linux/CMakeLists.txt` adds a Qt6 QML app target that depends on the
  ScriviCore CMake target (reuse the root build, don't fork it) and includes `scrivi.h`. Minimal
  `main.cpp` + a `Main.qml` that just opens a window. **New `.cpp/.hpp/.qml/CMakeLists` files must be
  tracked** — note: `Scrivi.xcodeproj/project.pbxproj` is Apple-only, so Linux files do **not** go in it
  (CLAUDE.md's pbxproj rule is scoped to `Scrivi/`+`ScriviCore/` Apple targets); they live in the CMake
  build only. Confirm this expectation in the sprint's first commit.
- **T-0219 — Bridge.** `ScriviBridge` is a `QObject` exposed to QML; methods call `scrivi_*`, parse the
  `{"ok":...}`/`{"error":...}` envelope, `scrivi_free` every pointer, and return typed values / emit
  error signals. No backend logic. This is the reusable core every later screen calls.
- **T-0220 — Hello slice.** One button/field in QML → `bridge.ensureLocalIdentity(...)` →
  `scrivi_ensure_local_identity` → show the returned identity JSON decoded. Proves the full round-trip.
- **T-0221 — Docker/VNC.** A `platforms/linux/docker/` image: Qt6 + build deps, builds the app, runs it
  under `Xvfb` with a VNC server (e.g. x11vnc/TigerVNC). Document the one command to build + the VNC
  connect steps in the README so the developer can *see* the window from the Mac.
- **T-0222 — CI.** Extend `.github/workflows/` (new job or new workflow) to build the Linux GUI on
  commits touching `platforms/linux/**` (and ScriviCore, since it links it). Headless smoke if feasible;
  at minimum a clean compile+link.

**Exit criteria:** From the Mac, `docker build` + connect via VNC shows the Qt window; clicking the
hello control returns a real identity from ScriviCore and displays it. CI builds the Linux app green.
ScriviCore `ctest` and the macOS build are untouched.

---

*Last Updated: 2026-07-13 (SP-058 activated — first sprint of EP-020 [Linux]. Foundation: Docker/VNC harness, Qt6 skeleton, ScriviBridge, hello-ScriviCore slice, CI. Tasks T-0218–T-0222 created.)*
