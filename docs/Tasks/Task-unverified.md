# Unverified Tasks

Tasks listed here are implemented and awaiting user verification before being archived.

---

## T-0083: `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified
**Note:** SP-024 paused pending SP-026 (C API boundary fix). Verification deferred until build is clean.

Created `Scrivi/Views/SceneNavigatorView.swift`. SwiftUI `List` with `.sidebar` style inside a `NavigationSplitView` (added to `EditorView.swift`). Groups scenes by chapter into `Section` headers ("Chapter 1", "Chapter 2"…). Each row is a `NavigatorRow` showing the scene title; no IDs, slugs, or UUIDs visible to the author. Section headers are non-selectable. Active scene highlighted with accent color tint.

---

## T-0084: Navigator title derivation — first line of author text or "Scene X"; chapter section headers

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified
**Note:** SP-024 paused pending SP-026.

Title resolution in `SceneNavigatorView.chapterGroups`: `loader.liveTitles[sceneID]` first (trimmed non-empty first line), falling back to "Scene X" (X = 1-based ordinal in full `allScenes` array). Chapter headers derived from unique `chapterID` transitions in `allScenes`, labelled "Chapter N".

---

## T-0085: Live title updates — ~300ms debounce on keystroke for currently active scene segment

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified
**Note:** SP-024 paused pending SP-026.

Added `titleTask` to `ManuscriptTextView.Coordinator`. In `textDidChange`, after extracting the current segment's text, a 300ms debounced `Task` fires `loader.updateLiveTitle(firstLine, forSceneID:)`. `ViewportSceneLoader` gained `liveTitles: [String: String]` (via `@Observable`) and `updateLiveTitle(_:forSceneID:)`.

---

## T-0086: Click-to-navigate — load selected scene, scroll editor to segment start

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified
**Note:** SP-024 paused pending SP-026.

`SceneNavigatorView` row `.onTapGesture` calls `navigate(to:sceneID)`, which calls `loader.navigateTo(sceneID:engine:ref:)` (saves current scene, reloads viewport centered on target). `EditorView` stores `navigateToSceneID: String?` state; `ManuscriptTextView.updateNSView` scrolls to that range and clears the binding.

**Modified files:**
- `Scrivi/Views/SceneNavigatorView.swift` — new file
- `Scrivi/Views/ViewportSceneLoader.swift` — added `allScenes`, `liveTitles`, `updateLiveTitle`, `navigateTo`
- `Scrivi/Views/ManuscriptTextView.swift` — added `navigateToSceneID` binding, 300ms title debounce, scroll-to-scene
- `Scrivi/Views/EditorView.swift` — replaced VStack with `NavigationSplitView`; added `navigateToSceneID` state

---

## T-0087: Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM package

**Sprint:** SP-024 | **Epic:** EP-009
**Status:** 🟠 Implemented — Not Verified
**Note:** Structural rearchitecture complete. Compiler errors remain; resolved by SP-026.

Replaced the broken SPM-over-xcodeproj double-indirection with a clean Xcode-native structure:

- All Apple-platform Swift and C++ source moved from `platforms/apple/Sources/` and `platforms/apple/Tests/` into `Scrivi/` at the repo root (subdirs: `Adapter/`, `Engine/`, `App/`, `Views/`, `Tests/`)
- `Scrivi.xcodeproj/project.pbxproj` created: 6 targets (`ScriviCoreAdapter` static lib, `ScriviEngine` framework, `ScriviApp` macOS, `ScriviApp-iOS`, `ScriviApp-visionOS`, `ScriviInteropTests`); no SPM references; CMake shell script phase on `ScriviCoreAdapter`
- `Scrivi.xcworkspace` created as the single Xcode entry point
- `ScriviCore.xcodeproj` removed — CMake is the sole build authority for C++
- `platforms/apple/Package.swift`, `Sources/`, `Tests/`, `.swiftpm/` removed
- `platforms/windows/`, `platforms/linux/`, `platforms/android/` placeholder READMEs created
- `CLAUDE.md` updated: repo structure diagram, Xcode project file rule, platform sections

**Modified files:**
- `Scrivi/Adapter/`, `Scrivi/Engine/`, `Scrivi/App/`, `Scrivi/Views/`, `Scrivi/Tests/` — new locations
- `Scrivi.xcodeproj/project.pbxproj` — created
- `Scrivi.xcworkspace/contents.xcworkspacedata` — new
- `ScriviCore.xcodeproj/` — deleted
- `platforms/apple/` source tree — deleted
- `platforms/windows/README.md`, `platforms/linux/README.md`, `platforms/android/README.md` — new
- `CLAUDE.md` — updated

---

*Last Updated: 2026-06-02 (SP-026 closed; T-0090–T-0094 verified and archived; SP-024 resumes)*
