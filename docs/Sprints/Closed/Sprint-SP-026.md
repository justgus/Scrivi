---
id: SP-026
title: "Apple Platform C API Boundary — Retire C++ Adapter, Wire scrivi.h"
epic: EP-009
status: ✅ Closed
start_date: 2026-06-02
end_date: 2026-06-02
---

## Goal

Resolve the root cause of Xcode compiler errors introduced by the rearchitecture. The `ScriviCoreAdapter` C++23 target forces Swift/C++ interop mode across all app targets, triggering the exact constraints documented in the Swift Interop Trade Study. The fix is to move the C++/adapter layer entirely out of Xcode and wire Swift directly to the plain-C API (`scrivi.h`) that already exists in ScriviCore.

**Root cause:** `ScriviCoreAdapter` is a C++23 static library compiled inside Xcode. App targets that link it activate Swift/C++ interop mode. That mode blocks on `std::string` return types, interior-pointer constraints, and the C++ reference-type import machinery — none of which apply to a plain-C boundary.

**Solution:** ScriviCore already exposes a complete plain-C API (`ScriviCore/include/scrivi/scrivi.h`, `scrivi_c_api.cpp`). This API returns heap-allocated `const char*` JSON strings; callers call `scrivi_free()` when done. Swift can call plain-C functions with zero interop friction. The adapter layer (`ScriviCoreAdapter.cpp/.hpp`, `KeychainSecureStore.cpp/.hpp`) is removed from the Xcode project entirely — CMake builds the C API into `libScriviCore.a` and Xcode links that.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0090 | Remove `ScriviCoreAdapter` target from Xcode — update `project.pbxproj` | ✅ Verified |
| T-0091 | Replace adapter module map with `ScriviCore` plain-C module map (`scrivi.h`) | ✅ Verified |
| T-0092 | Rewrite `ScriviEngine.swift` — call `scrivi_*` C functions; `scrivi_free()` after each call | ✅ Verified |
| T-0093 | Update `ScriviInteropTests.swift` — fix `@testable import` to correct module name | ✅ Verified |
| T-0094 | Verify: Xcode build clean, `ScriviInteropTests` green, `ctest` count unchanged | ✅ Verified |

## Acceptance Criteria

- [x] `Scrivi.xcodeproj` contains no `ScriviCoreAdapter` target, no `ScriviCoreAdapter.cpp`, no `KeychainSecureStore.cpp` in any build phase
- [x] Module map in `ScriviCore/include/scrivi/module.modulemap` exposes `scrivi.h` as module `ScriviCore`; no C++ types visible to Swift
- [x] `ScriviEngine.swift` calls all `scrivi_*` C functions and `scrivi_free()` on each returned `const char*`; no `import ScriviCoreAdapter`; no C++ adapter reference
- [x] `ScriviInteropTests.swift` uses `@testable import ScriviApp`
- [x] `cmake --build build --parallel` green; macOS app builds and launches without crash

## Completion Notes

- `LIBRARY_SEARCH_PATHS = "$(SRCROOT)/build/ScriviCore"` and `-lc++` added to all build configurations — required for the linker to locate `libScriviCore.a` and resolve C++ stdlib symbols from the static library
- `ScriviError.swift`: removed dead `decode(_ cxxString: std.string)` function that referenced the now-removed C++ module
- `DividerAttachmentCell.height` moved to a file-scope `private let dividerCellHeight` to resolve a Swift 6 `@MainActor` inference error on a `static let` inside an `NSTextAttachmentCell` subclass
- `ScriviCoreAdapter.cpp/.hpp` and `KeychainSecureStore.cpp/.hpp` remain on disk in `Scrivi/Adapter/` for reference; not compiled by any target
- Identity does not persist across process restarts (C API uses `PrototypeSecureStore`); Keychain integration is a future task

---

*Closed: 2026-06-02*
