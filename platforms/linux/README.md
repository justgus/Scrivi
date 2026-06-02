# Scrivi — Linux Platform

**Status:** Placeholder — not yet started.

## Architecture

- **GUI:** Qt/QML
- **Backend:** ScriviCore (C++23 static library, built by CMake)
- **Integration:** Direct C++ calls — no adapter layer needed
- **Build system:** CMake + Qt6

## Build (future)

```bash
# Prerequisites: Qt 6.x installed, cmake 3.24+
cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt6_DIR>
cmake --build build --parallel
```

## Development begins when Apple platform is mature

See `platforms/android/README.md` for the maturity criteria that gate non-Apple platform development.
