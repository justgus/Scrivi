# Scrivi — Windows Platform

**Status:** Placeholder — not yet started.

## Architecture

- **GUI:** Qt/QML
- **Backend:** ScriviCore (C++23 static library, built by CMake)
- **Integration:** Direct C++ calls — no adapter layer needed
- **Build system:** CMake + Qt6, targeting Windows 11

## Build (future)

```powershell
# Prerequisites: Qt 6.x installed, MSVC 2022, cmake 3.24+
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=<Qt6_DIR>
cmake --build build --config Release
```

## Development begins when Apple platform is mature

See `platforms/android/README.md` for the maturity criteria that gate non-Apple platform development.
