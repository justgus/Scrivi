# Scrivi — Linux Platform

**Status:** 🟢 In development — EP-020 `[Linux]` App Foundation (SP-058).

## Architecture

- **GUI:** Qt/QML
- **Backend:** ScriviCore (C++23 static library, built by the same CMake tree)
- **Integration:** Direct C++ calls through the plain-C ABI (`scrivi.h`) — no adapter. The QML
  `ScriviBridge` (`src/ScriviBridge.{hpp,cpp}`) is the Qt analogue of `ScriviEngine.swift`: it calls
  `scrivi_*`, parses the `{"ok":...}`/`{"error":...}` JSON envelope, and `scrivi_free`s every pointer.
  It holds **no** backend logic.
- **Build system:** CMake (opt-in `-DSCRIVI_BUILD_LINUX=ON`) + Qt6.

## Toolchain pin (EP-020 Step-0, 2026-07-13)

| Item | Pin |
| ---- | --- |
| OS base | **Ubuntu 24.04 LTS** |
| Qt | **Qt 6.4** (from Ubuntu 24.04 `apt`) |
| C++ | C++23 (matches ScriviCore) |
| Build | CMake ≥ 3.24 + Ninja |

Every later `[Linux]` Epic inherits this pin. Chosen to match the alpha tester's Ubuntu.

## App-support root (EP-021 / SP-059 Step-0, 2026-07-13)

The Linux app stores per-user state (local identity, recent-projects list, and anything ScriviCore keys
off `appSupportRoot`) under a single **XDG data directory** — the Linux counterpart to macOS Application
Support:

| Condition | `appSupportRoot` |
| --------- | ---------------- |
| `$XDG_DATA_HOME` set and non-empty | `$XDG_DATA_HOME/Scrivi` |
| otherwise | `$HOME/.local/share/Scrivi` |

The directory is created (`mkdir -p`) on first use. This value is passed into the existing `scrivi_*`
C-ABI `appSupportRoot` parameters (`scrivi_ensure_local_identity`, `scrivi_create_project`,
`scrivi_open_project`, …) — **no ScriviCore change**; the path is resolved app-side (`ScriviBridge`).

This retires EP-020's throwaway `/tmp/scrivi-linux-appsupport`, under which identity was recreated on
every run. Every later `[Linux]` Epic inherits this convention; identity and recents now persist across
app restarts. (In the Docker/VNC harness, `$HOME` is the container's root home, so state lives at
`/root/.local/share/Scrivi` unless `XDG_DATA_HOME` is overridden.)

## Persistent identity — EncryptedFileSecureStore (SP-059 / AC4)

Identity persistence across restart requires a persistent `SecureStore`. ScriviCore's C ABI previously
wired an **in-memory** `PrototypeSecureStore` for every platform (a documented MVP placeholder in
Architecture v0.3), so the local identity was silently recreated on each launch — the regression that
followed retiring the C++ adapter (which had removed Apple's `KeychainSecureStore`).

Per the SecureStore Platform Trade Study (`docs/Scrivi_SecureStore_Platform_Trade_Study_v0_1.md`,
Option L-2), the Linux build now uses **`EncryptedFileSecureStore`** — the identity bundle is stored
AES-256-GCM-encrypted at `<appSupportRoot>/secure/scrivi.identity.v1.enc`, with the key derived
(HKDF-SHA256) from `/etc/machine-id` + UID + username + a persisted random salt. This works headless /
in Docker / CI (no D-Bus, no keyring daemon) and depends only on **OpenSSL 3 (`libssl-dev`)**. The
selection is gated to Linux in `ScriviCore/CMakeLists.txt` (`CMAKE_SYSTEM_NAME STREQUAL "Linux"`);
Apple/Windows keep their existing wiring. The libsecret hybrid remains deferred (trade study Phase 3).

## Build (native, on an Ubuntu box)

```bash
# Prerequisites: the exact apt packages listed in docker/Dockerfile — that list
# is canonical. Besides qt6-base-dev/qt6-declarative-dev + cmake/ninja + libssl-dev
# (OpenSSL 3, for the persistent secure store), the QtQuick.Controls runtime needs
# several QML modules (qml6-module-qtquick*, plus qml6-module-qtquick-templates,
# qtqml-workerscript, qtqml-models); missing any of them fails at load with
# "module ... is not installed". The New Project folder picker uses a Qt Widgets
# QFileDialog (via ScriviBridge), provided by qt6-base-dev — no extra QML dialog
# module needed. (The Qt Quick FolderDialog was dropped: it couldn't select the
# folder being viewed, only an existing child, which is wrong for choosing a
# create-destination, and it required the extra Qt.labs.folderlistmodel module.)
cmake -S . -B build -G Ninja -DSCRIVI_BUILD_LINUX=ON -DSCRIVI_BUILD_TESTS=OFF
cmake --build build --parallel
./build/platforms/linux/scrivi_linux
```

## Build + view from a Mac (Docker + VNC)

No Linux desktop needed — the image runs the GUI headless under Xvfb and serves it over VNC.

```bash
# From the repo root, with Docker running:
platforms/linux/docker/build-and-run.sh
```

Then connect a VNC client to **`vnc://localhost:5901`** (Finder → Go → Connect to Server, or any VNC
viewer). **Password: `scrivi`.** Use the in-app **Quit** button on the landing view to stop (the app is the
container's foreground process, so quitting also stops the container). Ctrl-C in the terminal also works.

### Persistent state + shared projects folder (macOS ↔ container)

`build-and-run.sh` bind-mounts two host directories so state and projects **survive the (`--rm`) container**
and are reachable from macOS in Finder:

| Host dir (macOS) | Container path | Purpose |
| ---------------- | -------------- | ------- |
| `~/ScriviLinux/appsupport` | `/root/.local/share/Scrivi` | App-support root — **identity + recents persist across container restarts** (quit, re-run the script, same identity + recents return). Override: `SCRIVI_APPSUPPORT_DIR`. |
| `~/ScriviLinux/projects` | `/projects` | **Shared drop-zone** visible on both sides. Copy existing `.scrivi` packages here from macOS to open them in the app; projects created under `/projects` appear on the Mac. Override: `SCRIVI_PROJECTS_DIR`. |

The New Project folder picker **defaults to `/projects`** when that mount is present (else `$HOME`), so you
land in the shared folder instead of the app's working directory. To verify restart-persistence visually:
create a project → **Quit** → re-run `build-and-run.sh` → the landing view shows the same identity
("Signed in as …") and the project in **Recent Projects**.

> **Why 5901 and a password (macOS gotchas):**
> - Host port is **5901, not 5900** — macOS's built-in Screen Sharing already binds 5900, so `-p
>   5900:5900` fails with `address already in use` (and Finder's `vnc://localhost:5900` would hit your
>   *own* Mac's screen, giving "You cannot control your own screen").
> - A **password is required** — Apple's Screen Sharing client refuses passwordless (`-nopw`) servers and
>   won't enable "Sign In" without one. Third-party viewers (TigerVNC, RealVNC) work regardless.
> - Override with `SCRIVI_VNC_HOST_PORT` / `SCRIVI_VNC_PASSWORD` env vars.

**Docker Desktop (GUI) equivalent:** Images tab → `scrivi-linux` → Run → Optional settings → set **Host
port = 5901** (container port 5900) → Run. Then connect VNC to `localhost:5901`, password `scrivi`.
(Docker Desktop can't *display* the GUI itself — you still need a separate VNC viewer.)

Manual CLI equivalent:

```bash
docker build -f platforms/linux/docker/Dockerfile -t scrivi-linux .
docker run --rm -p 5901:5900 scrivi-linux   # VNC → localhost:5901, password: scrivi
```

## Layout

```
platforms/linux/
├── CMakeLists.txt          ← Qt6 app + persistence-smoke targets, links ScriviCore
├── src/
│   ├── main.cpp            ← QGuiApplication + QQmlApplicationEngine (loads Landing.qml)
│   ├── AppSupport.{hpp,cpp}   ← XDG appSupportRoot resolver (T-0223)
│   ├── RecentsStore.{hpp,cpp} ← recent-projects JSON store, QML_ELEMENT (T-0224)
│   ├── ScriviBridge.hpp    ← QML ↔ C ABI boundary (QML_ELEMENT): bootstrap + createProject
│   └── ScriviBridge.cpp
├── qml/
│   ├── Landing.qml            ← root screen: recents + New/Open actions (T-0226)
│   ├── NewProjectDialog.qml   ← title→slug→folder→createProject flow (T-0227)
│   └── ProjectWindow.qml      ← placeholder project window (real editor is EP-022)
├── tests/
│   ├── persistence_smoke.cpp  ← headless identity+recents persistence harness (T-0228)
│   └── persistence_smoke.sh   ← two-pass restart driver
└── docker/
    ├── Dockerfile          ← Ubuntu 24.04 + Qt 6.4 + libssl-dev, builds the app
    ├── run-vnc.sh          ← Xvfb + app + x11vnc (image CMD)
    └── build-and-run.sh    ← one-command build + run from the Mac
```

## CI

`.github/workflows/scrivi-linux-ci.yml` builds the app on `ubuntu-24.04` (Qt 6.4 from apt) and runs a
headless Xvfb smoke test on every commit touching `platforms/linux/**`, `ScriviCore/**`, or the root
`CMakeLists.txt`. The ScriviCore `ctest` job is unchanged and stays Qt-free.
