# Active Tasks

**Sprint SP-058** — `[Linux]` App Foundation: Docker/VNC harness, Qt6 skeleton, `ScriviBridge`,
hello-ScriviCore slice, CI. Epic: EP-020 `[Linux]`. Design: `platforms/linux/README.md`,
`ScriviCore/include/scrivi/scrivi.h`, `docs/Scrivi_Apple_Wrapper_Design_v0_1.md`.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0218 | `platforms/linux/` Qt6/QML CMake skeleton — app target links `libScriviCore.a` + `scrivi.h` | 🟢 Active |
| T-0219 | `ScriviBridge` (C++/Qt) — QML ↔ C ABI marshalling, JSON-envelope parse, `scrivi_free` discipline | 🟢 Active |
| T-0220 | Hello-ScriviCore QML window — call `scrivi_ensure_local_identity`, render decoded result | 🟢 Active |
| T-0221 | Docker image (Qt6 + CMake) + Xvfb + VNC — run the GUI headless, viewable from the Mac | 🟢 Active |
| T-0222 | CI job — build the Linux Qt/QML app (+ headless smoke) on relevant commits | 🟢 Active |

---

## SP-058 — [Linux] App Foundation — 🟢 Active (activated 2026-07-13)

**Epic:** EP-020 `[Linux]` · Delivers **AC1–AC5** (Qt6/QML app target linking ScriviCore; `ScriviBridge`
C-ABI marshalling; Docker+Xvfb/VNC harness; hello-ScriviCore round-trip; CI builds the Linux GUI).

**Step 0 — Qt version + Docker base decision (before T-0218):** pin the Qt6 minor version and Ubuntu base
image (match the alpha tester's Ubuntu) and record in `platforms/linux/README.md` — every later `[Linux]`
Epic inherits it.

### T-0218 — Qt6/QML CMake skeleton
`platforms/linux/CMakeLists.txt` adds a Qt6 QML app target depending on the ScriviCore CMake target
(reuse the root build) + includes `scrivi.h`; minimal `main.cpp` + `Main.qml` open a window.
**Note:** Linux source files are CMake-only — they do **not** go in `Scrivi.xcodeproj/project.pbxproj`
(that rule is scoped to Apple targets under `Scrivi/`+`ScriviCore/`).

### T-0219 — `ScriviBridge`
`QObject` exposed to QML; methods call `scrivi_*`, parse the `{"ok":...}`/`{"error":...}` envelope,
`scrivi_free` every returned pointer, return typed values / emit error signals. No backend logic — the
QML analogue of `ScriviEngine.swift`.

### T-0220 — Hello-ScriviCore slice
One QML control → `bridge.ensureLocalIdentity(...)` → `scrivi_ensure_local_identity` → display decoded
identity JSON. Proves QML → bridge → C ABI → ScriviCore → JSON → QML round-trip.

### T-0221 — Docker + Xvfb + VNC
`platforms/linux/docker/` image: Qt6 + build deps, builds the app, runs it under Xvfb + a VNC server so
the developer can open the running GUI from the Mac. README documents build + VNC-connect steps.

### T-0222 — CI
Extend `.github/workflows/` to build the Linux GUI on commits touching `platforms/linux/**` (+ ScriviCore,
since it links it). Headless smoke if feasible; at minimum a clean compile+link. ScriviCore `ctest` job
unchanged.

**Exit criteria:** From the Mac, `docker build` + VNC shows the Qt window; the hello control returns a
real identity from ScriviCore and displays it; CI builds the Linux app green; ScriviCore `ctest` + macOS
build untouched.

---

*Last Updated: 2026-07-13 (SP-058 activated — EP-020 `[Linux]` foundation. T-0218–T-0222 moved to active. First step is the Step-0 Qt/Docker-base decision before skeleton code.)*
