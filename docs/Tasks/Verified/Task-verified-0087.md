## T-0087: Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM package

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi.xcodeproj`, `Scrivi.xcworkspace`, `Scrivi/` source tree
**Priority:** Critical
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-024

**Rationale:**
The previous SPM-over-xcodeproj structure was broken: SPM could not resolve the C++ static library dependency, compiler errors cascaded across the Apple platform layer, and the user could not build or commit from Xcode.

**Desired Behavior:**
A clean Xcode-native structure with `Scrivi.xcworkspace` as the single entry point, all Swift and C++ adapter source in `Scrivi/` at the repo root, and CMake as the sole build authority for C++.

**Implementation Details:**
- All Apple-platform Swift and C++ source moved from `platforms/apple/Sources/` and `platforms/apple/Tests/` into `Scrivi/` (subdirs: `Adapter/`, `Engine/`, `App/`, `Views/`, `Tests/`)
- `Scrivi.xcodeproj/project.pbxproj` created: 6 targets (`ScriviCoreAdapter` static lib, `ScriviEngine` framework, `ScriviApp` macOS, `ScriviApp-iOS`, `ScriviApp-visionOS`, `ScriviInteropTests`); no SPM references; CMake shell script build phase on `ScriviCoreAdapter`
- `Scrivi.xcworkspace` created as single Xcode entry point
- `ScriviCore.xcodeproj` removed — CMake is sole C++ build authority
- `platforms/apple/` source tree deleted; `platforms/windows/`, `platforms/linux/`, `platforms/android/` placeholder READMEs created
- `CLAUDE.md` updated: repo structure diagram, Xcode project file rule, platform sections
- Compiler errors resolved by SP-026 (C API boundary migration)

**Files Affected:**
- `Scrivi/` — all Apple-platform source (new location)
- `Scrivi.xcodeproj/project.pbxproj` — created
- `Scrivi.xcworkspace/contents.xcworkspacedata` — created
- `ScriviCore.xcodeproj/` — deleted
- `platforms/apple/` — deleted
- `CLAUDE.md` — updated

**Verification:**
- ✅ Xcode builds clean from `Scrivi.xcworkspace`
- ✅ `ScriviInteropTests` green
- ✅ `ctest` count unchanged (165/165)

---

*Closed: 2026-06-03*
