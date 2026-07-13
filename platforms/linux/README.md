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

## Build (native, on an Ubuntu box)

```bash
# Prerequisites: the exact apt packages listed in docker/Dockerfile — that list
# is canonical. Besides qt6-base-dev/qt6-declarative-dev + cmake/ninja, the
# QtQuick.Controls runtime needs several QML modules (qml6-module-qtquick*,
# plus qml6-module-qtquick-templates, qtqml-workerscript, qtqml-models);
# missing any of them fails at load with "module ... is not installed".
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
viewer). **Password: `scrivi`.** Close the app window to stop.

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
├── CMakeLists.txt          ← Qt6 app target, links the ScriviCore CMake target
├── src/
│   ├── main.cpp            ← QGuiApplication + QQmlApplicationEngine
│   ├── ScriviBridge.hpp    ← QML ↔ C ABI boundary (QML_ELEMENT)
│   └── ScriviBridge.cpp
├── qml/
│   └── Main.qml            ← hello-ScriviCore window (identity round-trip)
└── docker/
    ├── Dockerfile          ← Ubuntu 24.04 + Qt 6.4, builds the app
    ├── run-vnc.sh          ← Xvfb + app + x11vnc (image CMD)
    └── build-and-run.sh    ← one-command build + run from the Mac
```

## CI

`.github/workflows/scrivi-linux-ci.yml` builds the app on `ubuntu-24.04` (Qt 6.4 from apt) and runs a
headless Xvfb smoke test on every commit touching `platforms/linux/**`, `ScriviCore/**`, or the root
`CMakeLists.txt`. The ScriviCore `ctest` job is unchanged and stays Qt-free.
