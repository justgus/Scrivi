---
id: SP-026
title: "Apple Platform C API Boundary — Retire C++ Adapter, Wire scrivi.h"
epic: EP-009
status: 🟡 Active
start_date: 2026-06-02
end_date: TBD
---

## Goal

Resolve the root cause of Xcode compiler errors introduced by the rearchitecture. The `ScriviCoreAdapter` C++23 target forces Swift/C++ interop mode across all app targets, triggering the exact constraints documented in the Swift Interop Trade Study. The fix is to move the C++/adapter layer entirely out of Xcode and wire Swift directly to the plain-C API (`scrivi.h`) that already exists in ScriviCore.

**Root cause:** `ScriviCoreAdapter` is a C++23 static library compiled inside Xcode. App targets that link it activate Swift/C++ interop mode. That mode blocks on `std::string` return types, interior-pointer constraints, and the C++ reference-type import machinery — none of which apply to a plain-C boundary.

**Solution:** ScriviCore already exposes a complete plain-C API (`ScriviCore/include/scrivi/scrivi.h`, `scrivi_c_api.cpp`). This API returns heap-allocated `const char*` JSON strings; callers call `scrivi_free()` when done. Swift can call plain-C functions with zero interop friction. The adapter layer (`ScriviCoreAdapter.cpp/.hpp`, `KeychainSecureStore.cpp/.hpp`) is removed from the Xcode project entirely — CMake builds the C API into `libScriviCore.a` and Xcode links that.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0090 | Remove `ScriviCoreAdapter` target from Xcode — update `project.pbxproj` | 🔵 Planned |
| T-0091 | Replace adapter module map with `ScriviCore` plain-C module map (`scrivi.h`) | 🔵 Planned |
| T-0092 | Rewrite `ScriviEngine.swift` — call `scrivi_*` C functions; `scrivi_free()` after each call | 🔵 Planned |
| T-0093 | Update `ScriviInteropTests.swift` — fix `@testable import` to correct module name | 🔵 Planned |
| T-0094 | Verify: Xcode build clean, `ScriviInteropTests` green, `ctest` count unchanged | 🔵 Planned |

## Acceptance Criteria

- [ ] `Scrivi.xcodeproj` contains no `ScriviCoreAdapter` target, no `ScriviCoreAdapter.cpp`, no `KeychainSecureStore.cpp` in any build phase
- [ ] A single module map in `Scrivi/Adapter/module.modulemap` (or new location) exposes `scrivi.h` as module `ScriviCore`; no C++ types visible to Swift
- [ ] `ScriviEngine.swift` calls `scrivi_ensure_local_identity`, `scrivi_create_project`, `scrivi_open_project`, `scrivi_open_scene`, `scrivi_save_scene`, `scrivi_create_scene`, `scrivi_create_chapter` (and all other methods) via plain-C function calls; calls `scrivi_free()` on each returned `const char*`; no `import ScriviCoreAdapter`; no `scrivi::apple::ScriviAdapter` reference
- [ ] `ScriviInteropTests.swift` imports the correct test target module (`@testable import ScriviApp` or the actual target name); all 17 interop tests compile and pass
- [ ] `cmake --build build --parallel && ctest --test-dir build --output-on-failure` remains fully green
- [ ] macOS app builds and launches without crash

## Notes

- The `ScriviCoreAdapter.cpp/.hpp` and `KeychainSecureStore.cpp/.hpp` files remain on disk — they are not deleted. They are removed from Xcode targets only. The files may be useful for reference or future non-Xcode platforms.
- `scrivi_c_api.cpp` is already in `ScriviCore/CMakeLists.txt` (added in the uncommitted diff). The CMake build already compiles the C API into `libScriviCore.a`.
- The Xcode build links `libScriviCore.a` (built by the CMake shell script phase). After this sprint, all of ScriviCore — including the C API — comes in via that `.a`. No separate adapter `.a` is needed.
- `KeychainSecureStore` was wired inside `ScriviAdapter::Impl`. The C API (`scrivi_c_api.cpp`) uses `PrototypeSecureStore` (in-memory). Keychain persistence on the C API path is a future task — not in scope here.
- The `scrivi_c_api.cpp` singleton uses `PrototypeSecureStore`; identity will not persist across process restarts until a future `KeychainSecureStore` integration task.

---

*Last Updated: 2026-06-02 (SP-026 created and activated)*
