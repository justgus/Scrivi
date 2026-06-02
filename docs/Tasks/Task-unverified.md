# Unverified Tasks

Tasks listed here are implemented and awaiting user verification before being archived.

---

## T-0083: `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified

Created `SceneNavigatorView.swift` in `Sources/ScriviApp/`. SwiftUI `List` with `.sidebar` style inside a `NavigationSplitView` (added to `EditorView.swift`). Groups scenes by chapter into `Section` headers ("Chapter 1", "Chapter 2"…). Each row is a `NavigatorRow` showing the scene title; no IDs, slugs, or UUIDs visible to the author. Section headers are non-selectable (SwiftUI `Section` header is not tappable). Active scene highlighted with accent color tint.

---

## T-0084: Navigator title derivation — first line of author text or "Scene X"; chapter section headers

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified

Title resolution in `SceneNavigatorView.chapterGroups`: `loader.liveTitles[sceneID]` first (trimmed non-empty first line), falling back to "Scene X" (X = 1-based ordinal in full `allScenes` array). Chapter headers derived from unique `chapterID` transitions in `allScenes`, labelled "Chapter N" (N = ordinal appearance order).

---

## T-0085: Live title updates — ~300ms debounce on keystroke for currently active scene segment

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified

Added `titleTask` to `ManuscriptTextView.Coordinator`. In `textDidChange`, after extracting the current segment's text, a 300ms debounced `Task` fires `loader.updateLiveTitle(firstLine, forSceneID:)`. `ViewportSceneLoader` gained `liveTitles: [String: String]` (published via `@Observable`) and `updateLiveTitle(_:forSceneID:)`. Navigator re-renders automatically when `liveTitles` changes since it reads from `@Observable loader`.

---

## T-0086: Click-to-navigate — load selected scene, scroll editor to segment start

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified

`SceneNavigatorView` row `.onTapGesture` calls `navigate(to:sceneID)`, which calls `loader.navigateTo(sceneID:engine:ref:)` (saves current scene, reloads viewport centered on target), then calls `onNavigate(sceneID)` callback. `EditorView` stores `navigateToSceneID: String?` state and passes it as `@Binding` to `ManuscriptTextView`. In `ManuscriptTextView.updateNSView`, if `navigateToSceneID` is set and the segment exists in current boundaries, scrolls to that range and clears the binding.

**Modified files:**
- `platforms/apple/Sources/ScriviApp/SceneNavigatorView.swift` — new file
- `platforms/apple/Sources/ScriviApp/ViewportSceneLoader.swift` — added `allScenes` (public), `liveTitles`, `updateLiveTitle`, `navigateTo`
- `platforms/apple/Sources/ScriviApp/ManuscriptTextView.swift` — added `navigateToSceneID` binding, 300ms title debounce, scroll-to-scene in `updateNSView`
- `platforms/apple/Sources/ScriviApp/EditorView.swift` — replaced VStack with `NavigationSplitView`; added `navigateToSceneID` state
- `ScriviCore.xcodeproj/project.pbxproj` — PBXFileReference, PBXBuildFile, PBXGroup, PBXSourcesBuildPhase entries for `SceneNavigatorView.swift`

---

---

## T-0087: Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM package

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified

Replaced the broken SPM-over-xcodeproj double-indirection with a clean Xcode-native structure:

- All Apple-platform Swift and C++ source moved from `platforms/apple/Sources/` and `platforms/apple/Tests/` into `Scrivi/` at the repo root (subdirs: `Adapter/`, `Engine/`, `App/`, `Views/`, `Tests/`)
- `Scrivi.xcodeproj/project.pbxproj` replaced wholesale: 6 targets (`ScriviCoreAdapter` static lib, `ScriviEngine` framework, `ScriviApp` macOS, `ScriviApp-iOS`, `ScriviApp-visionOS`, `ScriviInteropTests`); no SPM references; CMake shell script phase on `ScriviCoreAdapter`
- `Scrivi.xcworkspace` created as the single Xcode entry point
- `ScriviCore.xcodeproj` removed — CMake is the sole build authority for C++
- `platforms/apple/Package.swift`, `Sources/`, `Tests/`, `.swiftpm/` removed
- `platforms/windows/`, `platforms/linux/`, `platforms/android/` placeholder READMEs created
- `CLAUDE.md` updated: repo structure diagram, Xcode project file rule, Apple/Windows/Android platform sections

**Modified files:**
- `Scrivi/Adapter/` — new location (from `platforms/apple/Sources/ScriviCoreAdapter/`)
- `Scrivi/Engine/` — new location (from `platforms/apple/Sources/Scrivi/`)
- `Scrivi/App/` — new location (from `platforms/apple/Sources/ScriviApp/`)
- `Scrivi/Views/` — new location (from `platforms/apple/Sources/ScriviApp/`)
- `Scrivi/Tests/` — new location (from `platforms/apple/Tests/`)
- `Scrivi.xcodeproj/project.pbxproj` — replaced
- `Scrivi.xcworkspace/contents.xcworkspacedata` — new
- `ScriviCore.xcodeproj/` — deleted
- `platforms/apple/Package.swift` + source trees — deleted
- `platforms/windows/README.md`, `platforms/linux/README.md`, `platforms/android/README.md` — new
- `CLAUDE.md` — updated

---

*Last Updated: 2026-06-02 (T-0087 implemented; awaiting verification)*
