# SP-058: [Linux] Docker/VNC harness + Qt6 skeleton + hello-ScriviCore slice + CI

**Status:** ✅ Closed
**Epic:** EP-020 `[Linux]` — App Foundation
**Codebase:** `[Linux]` (`platforms/linux/`) + build tooling; no ScriviCore source changes.
**Activated:** 2026-07-13
**Closed:** 2026-07-13 (user-approved)
**Goal:** Stand up the whole Linux toolchain and prove the spine end-to-end: a Qt6/QML app under
`platforms/linux/` that links `libScriviCore.a`, calls one real `scrivi_*` function through a thin C++
`ScriviBridge`, and shows the decoded result in a real window — buildable and runnable in a Docker +
Xvfb/VNC desktop the developer opens from the Mac, and green in CI. Delivered **EP-020 AC1–AC5**.

**Design reference:** `platforms/linux/README.md` (Qt/QML + CMake + direct C++ calls),
`ScriviCore/include/scrivi/scrivi.h` (C ABI), `docs/Scrivi_Apple_Wrapper_Design_v0_1.md` (the
`ScriviEngine.swift` pattern the QML `ScriviBridge` mirrors).

## Assigned Tasks — all ✅ Verified

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0218 | `platforms/linux/` Qt6/QML CMake skeleton — app target links `libScriviCore.a` + `scrivi.h` | High | ✅ Verified |
| T-0219 | `ScriviBridge` (C++/Qt) — QML ↔ C ABI marshalling, JSON-envelope parse, `scrivi_free` discipline | High | ✅ Verified |
| T-0220 | Hello-ScriviCore QML window — call `scrivi_ensure_local_identity`, render decoded result | High | ✅ Verified |
| T-0221 | Docker image (Qt6 + CMake build) + Xvfb + VNC — run the GUI headless, viewable from the Mac | High | ✅ Verified |
| T-0222 | CI job — build the Linux Qt/QML app (+ headless smoke) on relevant commits | Medium | ✅ Verified |

## Exit criteria — met

From the Mac, `docker build` + VNC showed the Qt window; clicking the hello control returned a real
identity from ScriviCore and displayed it (green text, user-verified over VNC). CI built the Linux app
green (`Scrivi Linux App CI`, run 29283026516, success in 58s). ScriviCore `ctest` and the macOS build
were untouched (no `ScriviCore/` or Apple source changed).

## Retrospective

**What went well**
- The scaffolding matched the real C ABI on the first read: the `{"ok":...}`/`{"error":...}` envelope and
  the `ensureLocalIdentity` result fields (`identityID`, `defaultPersonaID`, `displayName`,
  `createdNewIdentity`) lined up exactly with `ScriviBridge` and `Main.qml` — no core changes needed.
- The `ScriviBridge` = `ScriviEngine.swift` analogy held: a thin QObject with `scrivi_free` RAII and no
  backend logic. The pattern is reusable for every later screen.
- Verification model worked as designed: CI proved build + headless smoke; the developer proved the GUI
  visually over VNC; the split caught things neither alone would have.

**What didn't (and the fixes, for later `[Linux]` Epics)**
- **Qt version drift bit repeatedly.** The scaffolding assumed a newer Qt than the pinned 6.4:
  `loadFromModule` (6.5+), the qmltyperegistrar include path, and the qrc `RESOURCE_PREFIX` all differed.
  Fixes: `engine.load(QUrl(...))`, `target_include_directories(... PRIVATE src)`, and a pinned
  `RESOURCE_PREFIX /`. **Lesson: build against the pinned Qt early — assumptions about "current Qt" are
  wrong on Ubuntu 24.04 apt.**
- **Docker context hygiene.** `COPY . /src` dragged the host `build/` (absolute-path `CMakeCache.txt`)
  into the image; added `Dockerfile.dockerignore`.
- **Runtime QML modules are separate apt packages.** QtQuick.Controls needs `qtquick-templates`,
  `qtqml-workerscript`, `qtqml-models` at *load* time — a clean compile doesn't catch a missing runtime
  module. Added them to the Dockerfile **and** CI.
- **macOS/VNC friction for the developer harness.** Port 5900 collides with macOS Screen Sharing, and
  Apple's client refuses passwordless VNC. Hardened the scripts: default host port **5901**, VNC
  **password** (`scrivi`), both overridable via env vars, documented in the README (+ Docker Desktop
  port field).
- **Latent harness bug:** the Xvfb-readiness loop called `xdpyinfo`, which was never installed (exit
  127) — a silent 5s no-op that let the app race an unready display. Added `x11-utils`.

**Adjust going forward**
- Every later `[Linux]` Epic inherits the **Ubuntu 24.04 + Qt 6.4** pin — target that Qt's actual API
  surface, not a newer one.
- The developer-facing VNC harness (port 5901, password) is now the baseline; the alpha tester inherits
  it.

---

*Closed 2026-07-13 — first sprint of EP-020 `[Linux]`. All five tasks Verified; all EP-020 AC1–AC5
delivered (AC6 no-regression held). User-approved close.*
