# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## T-0011: Swift Interop Prototype

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend) / Apple platform wrapper
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-003 (Planning)

**Rationale:**
The C++ core has no value if the Apple platform cannot call it. A minimal Swift wrapper that exercises `createProject()`, `openProject()`, and `saveScene()` proves the interop boundary works and establishes the pattern for the full Apple app shell.

**Current Behavior:**
No Swift/C++ interop wrapper exists.

**Desired Behavior:**
A minimal Apple-side wrapper can call `createProject()`, `openProject()`, and `saveScene()` through the C++ core. No backend behavior is reimplemented in Swift.

**Requirements:**
1. Swift/C++ direct interop (not Objective-C bridging) per approved approach in Repository Skeleton doc Section 2
2. Swift wrapper exposes `createProject()`, `openProject()`, `saveScene()` to Swift callers
3. Swift wrapper handles `Result<T>` conversion to Swift error/value pattern
4. No ScriviCore backend logic duplicated in Swift
5. Wrapper compiles and links against ScriviCore static library

**Design Approach:**
Use direct Swift/C++ interop. The wrapper lives outside `ScriviCore/` (it is a platform adapter, not part of the core). A minimal test target or playground can exercise the three entry points. The full Apple app shell is out of scope for this task.

**Components Affected:**
- New directory: `apple/` or `platforms/apple/` (wrapper target)
- ScriviCore/include/scrivi/: public headers consumed by Swift
- CMakeLists.txt: may need XCFramework or xcbuild integration for Xcode targets

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Swift code calls `createProject()` — project created on disk, no crash
2. Swift code calls `openProject()` on that project — returns active scene Markdown
3. Swift code calls `saveScene()` — Markdown persists and is readable
4. No backend logic (JSON parsing, path resolution, etc.) exists in Swift layer
5. Build succeeds on macOS 26+ with Xcode 26+

**Notes:**
Swift/C++ interop requires C++17 minimum; C++24 features in headers exposed to Swift must be compatible with the interop bridge. This may require careful header hygiene. Confirm interop toolchain behavior before implementation.

---

*Last Updated: 2026-05-20*
