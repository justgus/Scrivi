# Active Tasks

**Sprint SP-058** — `[Linux]` App Foundation: Docker/VNC harness, Qt6 skeleton, `ScriviBridge`,
hello-ScriviCore slice, CI. Epic: EP-020 `[Linux]`. Design: `platforms/linux/README.md`,
`ScriviCore/include/scrivi/scrivi.h`, `docs/Scrivi_Apple_Wrapper_Design_v0_1.md`.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0218 | `platforms/linux/` Qt6/QML CMake skeleton — app target links `libScriviCore.a` + `scrivi.h` | ✅ Verified |
| T-0219 | `ScriviBridge` (C++/Qt) — QML ↔ C ABI marshalling, JSON-envelope parse, `scrivi_free` discipline | ✅ Verified |
| T-0220 | Hello-ScriviCore QML window — call `scrivi_ensure_local_identity`, render decoded result | ✅ Verified |
| T-0221 | Docker image (Qt6 + CMake) + Xvfb + VNC — run the GUI headless, viewable from the Mac | ✅ Verified |
| T-0222 | CI job — build the Linux Qt/QML app (+ headless smoke) on relevant commits | 🟡 Implemented – Not Verified (CI-green pending push) |

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

## Build log — 2026-07-13 (Claude, Docker on the Mac)

Image `scrivi-linux` builds and links `scrivi_linux` clean; runs headless under Xvfb (loads the QML
window, stays up, no crash). ScriviCore round-trip proven in-container: a direct probe linking
`libScriviCore.a` and calling `scrivi_ensure_local_identity("Alpha Tester", …)` returns the expected
`{"ok":true,"result":{identityID, defaultPersonaID, displayName, createdNewIdentity:true}}` — exactly the
envelope `ScriviBridge` parses and `Main.qml` renders. Four real issues found and fixed against the
pinned **Qt 6.4** toolchain (each will recur on any Ubuntu 24.04 box, so they're recorded here):

1. **Host `build/` leaked into the image.** `COPY . /src` copied the Mac's `build/CMakeCache.txt` (absolute
   `/Users/...` paths) so in-container CMake refused to configure. **Fix:** added
   `platforms/linux/docker/Dockerfile.dockerignore` excluding `build/`, `**/build/`, `.git/`, Xcode cruft.
2. **`QQmlApplicationEngine::loadFromModule` is Qt 6.5+.** Not in 6.4. **Fix:** `main.cpp` uses
   `engine.load(QUrl("qrc:/Scrivi/qml/Main.qml"))`.
3. **qmltyperegistrar-generated TU couldn't find `ScriviBridge.hpp`** (bare `#include` from the build-tree
   root; 6.4 doesn't propagate per-file include dirs). **Fix:** `target_include_directories(scrivi_linux
   PRIVATE src)`.
4. **qrc prefix differs by Qt version.** 6.4 defaults to `:/<URI>`, later versions `:/qt/qml/<URI>`.
   **Fix:** pinned `RESOURCE_PREFIX /` in `qt_add_qml_module` so the load URL is deterministic.
5. **Missing runtime QML modules.** QtQuick.Controls needs `qtquick-templates`, `qtqml-workerscript`,
   `qtqml-models` at load time; without them the app fails with `module "QtQml.WorkerScript" is not
   installed`. **Fix:** added those three to both the Dockerfile and the CI workflow apt lists.

## GUI VERIFIED by user — 2026-07-13 (VNC)

The user connected a VNC client to the running container and **clicked "Ensure Local Identity"; the
identity text turned green** — the full QML → `ScriviBridge` → C ABI → ScriviCore → JSON → QML round-trip
confirmed visually in a real Qt window. **EP-020 AC1–AC4 verified.** T-0218–T-0221 → ✅ Verified per the
user's explicit approval. T-0222 (CI) stays 🟡 until the workflow runs green on a GitHub push.

**Two macOS/VNC gotchas hit during verification (harness hardened, so they won't recur):**
- **Port 5900 collides with macOS Screen Sharing** ("address already in use"; Finder `vnc://localhost:5900`
  hit the user's *own* Mac → "You cannot control your own screen"). → `build-and-run.sh` now defaults the
  **host port to 5901** (`SCRIVI_VNC_HOST_PORT` override); README documents it + the Docker Desktop port field.
- **Apple's Screen Sharing client refuses passwordless VNC** (won't enable "Sign In"). → `run-vnc.sh` now
  sets a **VNC password** (`scrivi`, `SCRIVI_VNC_PASSWORD` override) via `x11vnc -storepasswd`/`-rfbauth`.

**Latent harness bug found + fixed while hardening:** `run-vnc.sh`'s Xvfb-readiness loop called
`xdpyinfo`, which was **never installed** (exit 127) — the loop was a silent 5s no-op that sometimes let
the app race an unready display. → added **`x11-utils`** to the Dockerfile. Verified after the fix: Xvfb +
`scrivi_linux` + `x11vnc` all run and x11vnc listens on 5900 (→ host 5901).

---

*Last Updated: 2026-07-13 (User VNC-verified the GUI — green identity round-trip; EP-020 AC1–AC4 done,
T-0218–T-0221 → Verified. Hardened the VNC harness: host port 5901, VNC password, and fixed the
never-installed xdpyinfo (added x11-utils). T-0222/CI-green still pending a GitHub push.)*
