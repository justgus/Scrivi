# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overarching Principles

1. Don't assume. Don't hide confusion. Surface tradeoffs.
2. Minimum code that solves the problem. Do nothing speculative.
3. Touch only what you must. Clean up only your own mess.
4. Define success criteria. Loop until approved.
5. Claude can make mistakes. Do not assume Claude is always right.
6. I (the Human) can make mistakes.  Do not assume that I (the Human) am always right.  

---

## Project Overview

**Scrivi** is a multi-platform authoring, worldbuilding, and narrative design application. It helps writers, authors, dungeon masters, game designers, and scientists organize characters, scenes, locations, relationships, timelines, and maps for complex projects.

**Target Platforms:** macOS 26.0+, iOS 26.0+, iPadOS 26.0+, visionOS 26.0+, Windows 11, Android, Linux

**Key Apple Technologies (when UI work begins):** SwiftUI, CloudKit, PencilKit, MapKit, RealityKit (visionOS)

---

## Architecture

### Source of Truth

The design documents in `docs/` are the authoritative source of truth for architecture, behavior, and implementation decisions. Any contradiction between this file and the design docs must be surfaced and reconciled before implementation begins.

Key documents:
- `docs/Scrivi_Architecture_v0_3.md` — **current approved architecture** (supersedes v0.2); pass-by-value principle, JSON-over-string boundary, confirmed decisions
- `docs/Scrivi_Backend_Behavior_Spec_v0_2.md` — operational behavior specification
- `docs/Scrivi_Backend_MVP_Slice_v0_1.md` — minimum viable backend slice definition
- `docs/Scrivi_Cpp24_Backend_Core_Plan_v0_1.md` — C++24 backend implementation plan (8 milestones)
- `docs/Scrivi_Cpp24_Core_API_Sketch_v0_3.md` — public API shape, types, boundary protocol, approved decisions
- `docs/Scrivi_Cpp24_Core_Repository_Skeleton_v0_2.md` — repository structure, CMake layout, adapter design, SPM package
- `docs/Scrivi_Apple_Wrapper_Design_v0_1.md` — ScriviCoreAdapter and ScriviEngine design (JSON-over-string boundary)
- `docs/Scrivi_Project_Package_Structure_v0_1.md` — on-disk `.scrivi` package layout
- `docs/Scrivi_Minimum_Schema_Set_v0_1.md` — required JSON schemas
- `docs/Scrivi_Project_Creation_and_Open_Flow_v0_2.md` — first-launch and project lifecycle flows
- `docs/Scrivi_External_Change_Repair_Matrix_v0_2.md` — 20 failure conditions and repair behavior
- `docs/Scrivi_Backend_Runtime_Trade_Study_v0_2.md` — rationale for C++24 backend selection
- `docs/Scrivi_Swift_Interop_Trade_Study_v0_1.md` — why JSON-over-string is the permanent boundary protocol
- `docs/Scrivi_Timeline_Panel_Design_v0_3.md` — Timeline Panel requirements, interaction model, persistence schema, Story Structure integration, historical events, multi-row imported timelines, clustering, Time Delta Picker (supersedes v0.1, v0.2)

### Backend: C++23 (ScriviCore)

The approved architecture uses a **shared C++23 static library (ScriviCore)** as the single backend core across all platforms. SwiftData is not the data layer — project data is stored as JSON files on disk inside a `.scrivi` project package, managed entirely by ScriviCore.

- All project I/O, schema read/write, identity management, Git snapshots, and external change detection live in C++.
- The Apple platform (Swift) calls into ScriviCore through `ScriviCoreAdapter` — a permanent C++ shim that serializes results to `std::string` JSON. No Objective-C bridging.
- The boundary protocol is **JSON-over-`std::string`**, permanent. Swift/C++ direct struct interop with ScriviCore public headers is not the boundary strategy.
- No backend logic is reimplemented in Swift. Swift is responsible for UI only.

### Apple Platform Layer

The Swift/Apple layer is a thin wrapper over ScriviCore via `ScriviCoreAdapter`. All Apple-platform source lives in `Scrivi/` at the repo root, owned directly by `Scrivi.xcodeproj`. There is no SPM package.

- `ScriviCoreAdapter` (C++, `Scrivi/Adapter/`) accepts `const char*` inputs, returns `std::string` JSON results by value. Built as a static library target inside Xcode.
- `ScriviEngine.swift` (`Scrivi/Engine/`) holds the adapter reference, converts Swift types, decodes JSON envelopes, throws `ScriviError`. Built as a framework target inside Xcode.
- App source in `Scrivi/App/` and `Scrivi/Views/`. Interop tests in `Scrivi/Tests/`.
- `Scrivi.xcworkspace` is the entry point — open this, not the xcodeproj directly.
- SwiftUI for all UI on Apple platforms (macOS active; iOS/visionOS targets stubbed, in progress)
- CloudKit for sync (future — not in current Epics)
- PencilKit, MapKit, RealityKit as appropriate per platform

### Windows / Linux Platform Layer

Qt/QML GUI calling ScriviCore directly (no adapter needed — same C++ process). Projects live in `platforms/windows/` and `platforms/linux/`. Not yet started — see Android maturity criteria.

### Android Platform Layer

Deferred. See `platforms/android/README.md` for the maturity criteria that gate Android development.

---

## Development Environment

### C++ Backend

- **CMake:** 3.24+
- **C++ Standard:** C++23 (targeting C++24 features as compiler support matures)
- **Compiler:** Apple Clang (macOS), GCC or Clang (Linux), MSVC (Windows)
- **Test Framework:** Catch2 v3 (via CMake FetchContent)
- **JSON:** nlohmann/json v3.11.3 (via CMake FetchContent, hidden behind `Json` wrapper — never exposed in public headers)

Build commands:
```bash
# Configure
cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON

# Build
cmake --build build --parallel

# Test
ctest --test-dir build --output-on-failure
```

**Xcode project file rule:** The user commits and creates PRs from Xcode. Every time a new `.cpp`, `.hpp`, or `.swift` file is added anywhere under `Scrivi/` or `ScriviCore/`, `Scrivi.xcodeproj/project.pbxproj` MUST be updated in the same step — before the build, not after. Failing to do this means the user cannot see new files in Xcode and cannot stage them for commit. This is a non-negotiable requirement. `ScriviCore.xcodeproj` no longer exists — do not reference it.

### Apple Platform (when UI work begins)

- **Xcode:** 26.2+ (build 17C52)
- **macOS:** 26.2+
- **iOS/iPadOS:** 26.2+
- **visionOS:** 26.2+
- **Swift:** 6.0+ (Swift 6 language mode)

**NEVER assume features, APIs, or behaviors from earlier versions of Xcode or any Apple platform.**

Official documentation references:
- Xcode 26 Release Notes: https://developer.apple.com/documentation/xcode-release-notes/xcode-26-release-notes
- macOS 26 APIs: https://developer.apple.com/documentation/macos-release-notes/
- iOS 26 APIs: https://developer.apple.com/documentation/ios-ipados-release-notes/
- visionOS 26 APIs: https://developer.apple.com/documentation/visionos-release-notes/
- SwiftData: https://developer.apple.com/documentation/swiftdata
- Swift Testing: https://developer.apple.com/documentation/testing

Common pitfalls to avoid (Apple):
- ❌ Using Objective-C bridging instead of Swift/C++ direct interop
- ❌ Reimplementing backend logic in Swift
- ❌ Using SwiftData as the source of truth for project data
- ❌ Referencing pre-iOS 26 API documentation
- ❌ Ignoring Swift 6 language mode compatibility
- ❌ Using pre-Swift 5.9 concurrency patterns

---

## Repository Structure

```
Scrivi/
├── Scrivi.xcworkspace           ← Open this in Xcode (entry point)
├── Scrivi.xcodeproj             ← Xcode project (all Apple targets)
├── Scrivi/                      ← Apple platform Swift + Adapter source
│   ├── Adapter/                 ← ScriviCoreAdapter.cpp/.hpp, KeychainSecureStore, module.modulemap
│   ├── Engine/                  ← ScriviEngine.swift, ScriviError.swift
│   ├── App/                     ← ScriviApp.swift, AppEnvironment.swift, Info.plist, entitlements
│   ├── Views/                   ← All SwiftUI views
│   └── Tests/                   ← ScriviInteropTests.swift
├── ScriviCore/                  ← C++23 backend static library (CMake builds this)
│   ├── include/scrivi/          ← Public headers (no UI types, no third-party types)
│   ├── src/
│   │   ├── public_api/          ← ScriviCore.cpp facade implementation
│   │   ├── schemas/             ← JSON schema read/write modules
│   │   ├── util/                ← PathUtils, Slug, TextStats, Json, AtomicWrite, Hash
│   │   └── platform/            ← NullLogger, LocalFileSystem, mock services
│   └── tests/
│       ├── unit/                ← Unit tests (Catch2)
│       ├── integration/         ← Integration tests against real temp directories
│       ├── mocks/               ← Test-only mock implementations
│       └── fixtures/            ← Canonical test project fixtures
├── platforms/
│   ├── windows/                 ← Qt/QML Windows project (placeholder)
│   ├── linux/                   ← Qt/QML Linux project (placeholder)
│   └── android/                 ← Deferred (see README for maturity criteria)
├── docs/                        ← All design and architecture documents
│   ├── Epics/                   ← Epic tracking
│   ├── Sprints/                 ← Sprint tracking
│   ├── Tasks/                   ← Task tracking
│   └── Issues/                  ← Issue tracking
└── CMakeLists.txt               ← Root CMake configuration (ScriviCore only)
```

---

## Agile Tracking

This project uses an Agile workflow with four complementary tracking layers:

```
Epics → Sprints → Tasks / Issues
```

### Issues (I) — bugs, defects, unintended behavior
- Active: `docs/Issues/Issue-active.md`
- Backlog: `docs/Issues/Issue-backlog.md`
- Archived (verified): `docs/Issues/Verified/Issue-verified-XXXX-YYYY.md`
- Archived (closed): `docs/Issues/Closed/Issue-closed-XXX-XXX.md`
- Guidelines: `docs/Issues/Issue-GUIDELINES.md`
- Claude can mark as "Resolved - Not Verified"
- Claude can only mark as "Verified" after direct user approval

### Tasks (T) — new features, improvements, planned changes
- Backlog: `docs/Tasks/Task-backlog.md`
- Active: `docs/Tasks/Task-active.md`
- Unverified: `docs/Tasks/Task-unverified.md`
- Archived (verified): `docs/Tasks/Verified/Task-verified-XXXX.md`
- Guidelines: `docs/Tasks/Task-Guidelines.md`
- Same verification workflow as Issues

### Sprints (SP) — fixed-duration iterations
- Active: `docs/Sprints/Sprint-active.md`
- Index: `docs/Sprints/Sprint-Documentation.md`
- Archived: `docs/Sprints/Closed/Sprint-SP-XXX.md`
- Guidelines: `docs/Sprints/Sprint-GUIDELINES.md`
- Claude can create and activate Sprints
- Claude can only close a Sprint after direct user approval

### Epics (EP) — strategic milestones spanning multiple Sprints
- Active: `docs/Epics/Epic-active.md`
- Index: `docs/Epics/Epic-Documentation.md`
- Archived: `docs/Epics/Closed/Epic-EP-XXX.md`
- Guidelines: `docs/Epics/Epic-GUIDELINES.md`
- Claude can create Epics and mark them Complete
- Claude can only close an Epic after direct user approval

**When implementing fixes or features:**
1. Document in the appropriate file (Issue or Task)
2. Assign to the current Sprint if active
3. Include file:line references
4. Mark as "Resolved/Implemented - Not Verified" when done
5. Leave verification status for the user to confirm

---

## Apple Platform Conditionals (for future UI work)

```swift
#if os(macOS)
// macOS-only code
#elseif os(iOS)
// iOS-only code
#elseif os(visionOS)
// visionOS-only code
#endif

#if canImport(PencilKit)
import PencilKit
#endif
```
