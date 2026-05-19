# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Overarching Principles
1.  Don't assume.  Don't hide confusion. Surface tradeoffs.
2.  Minimum code that solves the problem.  Do nothing speculative.
3.  Touch only what you must.  Clean up only your own mess.
4.  Define success criteria.  Loop until approved.
5.  I can make mistakes.  Do not assume I am always right.

## Project Overview

**Scrivi** is a multi-platform authoring, worldbuilding, and narrative design application. It helps writers, authors, dungeon masters, game designers, and scientists organize characters, scenes, locations, relationships, timelines, and maps for complex projects.

**Apple Platforms**: macOS 26.0+, iOS 26.0+, iPadOS 26.0+, visionOS 26.0+, Windows 11, Android, Linux

**Key Apple Technologies**: SwiftUI, SwiftData, CloudKit, PencilKit, MapKit, RealityKit (visionOS)

## Development Environment Requirements

**CRITICAL**: This project uses cutting-edge Apple technologies and deployment targets.

### Minimum Apple Versions - DO NOT USE OLDER VERSIONS

- **Xcode**: 26.2+ (build 17C52)
- **macOS**: 26.2+
- **iOS/iPadOS**: 26.2+
- **visionOS**: 26.2+
- **Swift**: 5.0+ with upcoming features enabled

**NEVER assume features, APIs, or behaviors from earlier versions of Xcode or any Apple platform.**

### Official Apple Documentation

When researching APIs, features, or behaviors, ONLY reference current documentation:

- **Xcode 26 Release Notes**: https://developer.apple.com/documentation/xcode-release-notes/xcode-26-release-notes
- **macOS 26 APIs**: https://developer.apple.com/documentation/macos-release-notes/
- **iOS 26 APIs**: https://developer.apple.com/documentation/ios-ipados-release-notes/
- **visionOS 26 APIs**: https://developer.apple.com/documentation/visionos-release-notes/
- **SwiftData Documentation**: https://developer.apple.com/documentation/swiftdata
- **Swift Testing (not XCTest)**: https://developer.apple.com/documentation/testing

### Key Apple Technology Features Used

This project uses **modern** Swift and SwiftUI features that may not exist in older SDKs:

- **Swift Testing** framework (NOT XCTest) - introduced in Xcode 26
- **SwiftData** with Schema versioning and migrations
- **@Observable** macro (Swift 5.9+)
- **\#Predicate** macro for SwiftData queries
- **Upcoming Swift features** enabled via compiler flags
- **Apple Intelligence APIs** (iOS 26.2+, macOS 26.2+, visionOS 26.2+)

### Apple Verification Steps

Before making assumptions about API availability or behavior:

1. **Check the deployment target**: All platforms target version 26.0 minimum
2. **Verify API availability**: Use `@available` checks or `#available` for runtime checks
3. **Consult current documentation**: Use the URLs above, not outdated Stack Overflow answers
4. **Test on the actual platform**: Code that works in older SDKs may not compile or behave correctly with newer deployment targets

### Common Pitfalls to Avoid (Apple)

- ❌ Assuming XCTest instead of Swift Testing
- ❌ Using deprecated SwiftData patterns from WWDC 2023
- ❌ Referencing earlier than iOS 26 API documentation
- ❌ Assuming CloudKit behaviors from older SDK versions
- ❌ Using pre-Swift 5.9 concurrency patterns
- ❌ Ignoring Swift 6 language mode compatibility

## Building and Testing on Apple Devices

### Build Commands

```bash
# Build for macOS
xcodebuild -scheme Cumberland-macOS -configuration Debug build

# Build for iOS
xcodebuild -scheme "Cumberland IOS" -configuration Debug -sdk iphonesimulator build

# Build for visionOS
xcodebuild -scheme Cumberland_visionOS -configuration Debug build

# Run tests
xcodebuild test -scheme Cumberland-macOS -configuration Debug
```

### Running in Xcode

Open `Cumberland.xcodeproj` and select the appropriate scheme:
- **Cumberland-macOS** - macOS app
- **Cumberland IOS** - iOS/iPadOS app
- **Cumberland\_visionOS** - visionOS app

## Core Xcode Architecture

### Data Model (SwiftData)

All models are in `Cumberland/Model/`:

- **Card** - Central entity for all content (characters, scenes, locations, maps, etc.)
  - Uses `kindRaw` string for CloudKit compatibility (use `kind` computed property in code)
  - Supports original image storage via `originalImageData` (external storage)
  - Draft map work saved in `draftMapWorkData` for cross-device editing
  - Many-to-many relationships with `StructureElement` (story structure assignments)
  - Edges represented via `CardEdge` (directed graph relationships)

- **StoryStructure** & **StructureElement** - Story structure templates (e.g., "Three-Act Structure")
  - Elements can be assigned to multiple cards (many-to-many)
  - Use `StructureAssignmentManager` for assignments

- **Board** & **BoardNode** - Visual canvas for relationship mapping (MurderBoard)

- **Source** & **Citation** - Research source tracking with automatic attribution

- **RelationType** - Custom relationship types between cards

**Schema Versioning**: Currently on `AppSchemaV5`. See `Migrations.swift` for migration history.

### CloudKit Sync

- Container: `iCloud.CumberlandCloud`
- All models use defaults for CloudKit compatibility (no non-optional properties without defaults)
- External storage (`@Attribute(.externalStorage)`) automatically uses CKAsset
- Schema migrations handled via `AppMigrations` plan

### State Management

- `@Observable` used for view models (e.g., `DrawingCanvasModel`, `LayerManager`)
- `@State` for local UI state
- `@AppStorage` for persisted user preferences
- SwiftData `@Query` for database queries

## Major Features

### TBD

## Agile Tracking Systems

This project uses an Agile workflow with four complementary tracking layers:

```
Epics → Sprints → Tasks / Issues
```

### Issues (I)
**For bugs, defects, and unintended behavior**

Location: `docs/Issues/`

- Active Issues in `Issue-active.md`
- Backlog Issues in `Issue-backlog.md`
- Verified Issues archived in `Issues/Verified/Issue-verified-XXXX-YYYY.md` batches
- Closed Issues (without verification) in `Issues/Closed/Issue-closed-XXX-XXX.md`
- Guidelines in `Issues/Issue-GUIDELINES.md`
- **Claude can mark as "Resolved - Not Verified"**
- **Claude can only mark as "Verified" after direct user approval**

### Tasks (T)
**For new features, improvements, and planned changes**

Location: `docs/Tasks/`

- Backlog Tasks in `Task-backlog.md`
- In-progress Tasks in `Task-active.md`
- Unverified Tasks in `Task-unverified.md`
- Verified Tasks archived in `Tasks/Verified/Task-verified-XXXX.md`
- Guidelines in `Task-Guidelines.md`
- Same verification workflow as Issues

### Sprints (SP)
**Fixed-duration iterations grouping Tasks and Issues into focused work batches**

Location: `docs/Sprints/`

- Active Sprint in `Sprint-active.md`
- All Sprints indexed in `Sprint-Documentation.md`
- Closed Sprints archived in `Sprints/Closed/Sprint-SP-XXX.md`
- Guidelines in `Sprint-GUIDELINES.md`
- **Claude can create and activate Sprints**
- **Claude can only close a Sprint after direct user approval**

### Epics (EP)
**Strategic milestones spanning multiple Sprints**

Location: `docs/Epics/`

- Active Epics in `Epic-active.md`
- All Epics indexed in `Epic-Documentation.md`
- Closed Epics archived in `Epics/Closed/Epic-EP-XXX.md`
- Guidelines in `Epic-GUIDELINES.md`
- **Claude can create Epics and mark them Complete**
- **Claude can only close an Epic after direct user approval**

**When implementing fixes or features**:
1. Document in appropriate file (Issue or Task)
2. Assign to current Sprint if active
3. Include file:line references
4. Mark as "Resolved/Implemented - Not Verified" when done
5. Leave verification status for user to confirm

## Documentation References

Comprehensive documentation in `Scrivi/Documentation/`:


## Apple Platform Conditionals

Use platform checks for platform-specific code:

```swift
#if os(macOS)
// macOS-only code
#elseif os(iOS)
// iOS-only code
#elseif os(visionOS)
// visionOS-only code
#endif

// Framework availability
#if canImport(PencilKit)
import PencilKit
#endif
```

