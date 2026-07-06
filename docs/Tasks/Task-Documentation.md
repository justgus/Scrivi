# Task - Index

This is the main index for all Scrivi Tasks. Tasks track planned improvements, new features, and requirement changes to the system.

> **Note:** For bugs and unintended behavior, see [Issues (I)](../Issues/Issue-Documentation.md)
> **Related:** [Epics](../Epics/Epic-Documentation.md) | [Sprints](../Sprints/Sprint-Documentation.md)

## Organization

- **Task-backlog.md** — Proposed Tasks not assigned to a Sprint
- **Task-active.md** — In-Progress Tasks assigned to a Sprint
- **Task-unverified.md** — Implemented Tasks awaiting user verification
- **Verified/Task-verified-XXXX.md** — Verified Tasks (one file per Task)

## Backlog Tasks

Currently: **21 backlog Tasks**

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0139 | `TimelineMetaJson` schema — read/write `timeline.meta.json` | EP-016 | 🔵 Backlog |
| T-0140 | `SceneMetaJson` extension — `storyTime` block read/write | EP-016 | 🔵 Backlog |
| T-0141 | `StoryStructureJson` schema — read/write `story-structure.json` | EP-016 | 🔵 Backlog |
| T-0142 | `HistoricalEventJson` schema — read/write `historical-events/<id>.json` | EP-016 | 🔵 Backlog |
| T-0143 | `ExternalTimelineJson` schema — read/write imported timeline files | EP-016 | 🔵 Backlog |
| T-0144 | ScriviCore facade — timeline, story-time, and band-assignment operations | EP-016 | 🔵 Backlog |
| T-0145 | ScriviCore facade — story structure, historical events, imported timelines, export | EP-016 | 🔵 Backlog |
| T-0146 | C API + `ScriviEngine.swift` — expose all timeline operations | EP-016 | 🔵 Backlog |
| T-0147 | Integration tests — timeline persistence round-trip | EP-016 | 🔵 Backlog |
| T-0148 | `createProject` update — write `timeline.meta.json` on new project | EP-016 | 🔵 Backlog |
| T-0156 | Story Structure band overlay — colored bands, label row, Structure selector menu | EP-016 | 🔵 Backlog |
| T-0157 | Band border drag — proportional resize, persistence | EP-016 | 🔵 Backlog |
| T-0158 | Band assignment by drag-up to label — ring color on dot | EP-016 | 🔵 Backlog |
| T-0159 | Band assignment via context menu ("Assign to Act…" / "Unassign") | EP-016 | 🔵 Backlog |
| T-0160 | Scene dot context menu — full menu | EP-016 | 🔵 Backlog |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | EP-016 | 🔵 Backlog |
| T-0162 | Imported timeline row — render events as grey row below project row | EP-016 | 🔵 Backlog |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | EP-016 | 🔵 Backlog |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | EP-016 | 🔵 Backlog |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | EP-016 | 🔵 Backlog |
| T-0167 | EP-016 verification | EP-016 | ✅ Verified (2026-06-23) |

See: [Task-backlog.md](Task-backlog.md)

## Active Tasks

Currently: **2 active Tasks** (SP-047)

| Task | Title | Sprint | Status |
| ---- | ----- | ------ | ------ |
| T-0189 | End-to-end verification (donor search, deep-link tap, donations succeed) | SP-047 | 🟡 Active |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | SP-047 | 🟡 Active |

> **Cleanup (2026-07-01):** The former stale Active-Tasks list carried three SP-042 (closed) items —
> **T-0166, T-0169, T-0170**. All are now user-verified and archived to `Verified/`
> (`Task-verified-0166.md`, `Task-verified-0169.md`, `Task-verified-0170.md`) and appear in no active
> or backlog list.

See: [Task-active.md](Task-active.md)

## Unverified Tasks (Implemented, Awaiting Verification)

Currently: **0 unverified Tasks**

| Task | Title | Status |
| ---- | ----- | ------ |
| —    | None  | —      |

See: [Task-unverified.md](Task-unverified.md)


## Verified Tasks

Currently: **76 verified Tasks**

| Task   | Title | File | Status |
| ------ | ----- | ---- | ------ |
| T-0001 | Repository Skeleton | [Task-verified-0001.md](Verified/Task-verified-0001.md) | ✅ Verified |
| T-0002 | Core Value Types | [Task-verified-0002.md](Verified/Task-verified-0002.md) | ✅ Verified |
| T-0003 | Utility Foundation | [Task-verified-0003.md](Verified/Task-verified-0003.md) | ✅ Verified |
| T-0004 | Mock Services | [Task-verified-0004.md](Verified/Task-verified-0004.md) | ✅ Verified |
| T-0005 | Schema Read/Write | [Task-verified-0005.md](Verified/Task-verified-0005.md) | ✅ Verified |
| T-0006 | Project Creation | [Task-verified-0006.md](Verified/Task-verified-0006.md) | ✅ Verified |
| T-0007 | Open/Resume | [Task-verified-0007.md](Verified/Task-verified-0007.md) | ✅ Verified |
| T-0008 | Save Scene | [Task-verified-0008.md](Verified/Task-verified-0008.md) | ✅ Verified |
| T-0009 | External Change Scan | [Task-verified-0009.md](Verified/Task-verified-0009.md) | ✅ Verified |
| T-0010 | Git Snapshots | [Task-verified-0010.md](Verified/Task-verified-0010.md) | ✅ Verified |
| T-0011 | Swift Interop Prototype | [Task-verified-0011.md](Verified/Task-verified-0011.md) | ✅ Verified |
| T-0012 | Identity Service and UUID Provider | [Task-verified-0012.md](Verified/Task-verified-0012.md) | ✅ Verified |
| T-0013 | appSupportRoot Directory Bootstrap | [Task-verified-0013.md](Verified/Task-verified-0013.md) | ✅ Verified |
| T-0014 | Introduce `src/domain/` and Relocate Slug and TextStats | [Task-verified-0014.md](Verified/Task-verified-0014.md) | ✅ Verified |
| T-0015 | Move `AppSupportLayout` from `src/util/` to `src/platform/` | [Task-verified-0015.md](Verified/Task-verified-0015.md) | ✅ Verified |
| T-0016 | Merge `MockServicesTests.cpp` into `ResultTests.cpp` | [Task-verified-0016.md](Verified/Task-verified-0016.md) | ✅ Verified |
| T-0017 | Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp` | [Task-verified-0017.md](Verified/Task-verified-0017.md) | ✅ Verified |
| T-0018 | First Green Build and Full Test Suite Verification | [Task-verified-0018.md](Verified/Task-verified-0018.md) | ✅ Verified |
| T-0019 | Add `SnapshotMetadataJson` Schema | [Task-verified-0019.md](Verified/Task-verified-0019.md) | ✅ Verified |
| T-0020 | Add `SnapshotMetadataJson` Test Coverage | [Task-verified-0020.md](Verified/Task-verified-0020.md) | ✅ Verified |
| T-0021 | Milestone 10 Verification — External Change Scan | [Task-verified-0021.md](Verified/Task-verified-0021.md) | ✅ Verified |
| T-0022 | Milestone 11 Verification — Git Snapshots | [Task-verified-0022.md](Verified/Task-verified-0022.md) | ✅ Verified |
| T-0023 | Extract `ScriviError.swift` as a Separate File | [Task-verified-0023.md](Verified/Task-verified-0023.md) | ✅ Verified |
| T-0024 | Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | [Task-verified-0024.md](Verified/Task-verified-0024.md) | ✅ Verified |
| T-0025 | Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | [Task-verified-0025.md](Verified/Task-verified-0025.md) | ✅ Verified |
| T-0026 | Add Swift Interop Tests for New Adapter Methods | [Task-verified-0026.md](Verified/Task-verified-0026.md) | ✅ Verified |
| T-0027 | Full MVP Loop Integration Test | [Task-verified-0027.md](Verified/Task-verified-0027.md) | ✅ Verified |
| T-0028 | `RepairIssueJson` Schema Module | [Task-verified-0028.md](Verified/Task-verified-0028.md) | ✅ Verified |
| T-0029 | `applyRepair` Facade Method — Request, Result, and Dispatch | [Task-verified-0029.md](Verified/Task-verified-0029.md) | ✅ Verified |
| T-0030 | Repair Handlers — Manuscript File Operations | [Task-verified-0030.md](Verified/Task-verified-0030.md) | ✅ Verified |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | [Task-verified-0031.md](Verified/Task-verified-0031.md) | ✅ Verified |
| T-0032 | Integration Tests for `applyRepair` | [Task-verified-0032.md](Verified/Task-verified-0032.md) | ✅ Verified |
| T-0033 | Adapter — `applyRepair` + Full `RepairIssue` Serialization | [Task-verified-0033.md](Verified/Task-verified-0033.md) | ✅ Verified |
| T-0034 | Character Object Schema (`ObjectJson`) | [Task-verified-0034.md](Verified/Task-verified-0034.md) | ✅ Verified |
| T-0035 | `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters | [Task-verified-0035.md](Verified/Task-verified-0035.md) | ✅ Verified |
| T-0036 | Integration Tests for Character Object CRUD | [Task-verified-0036.md](Verified/Task-verified-0036.md) | ✅ Verified |
| T-0037 | Remaining Object Types — Location, Item, Rule, Timeline Schemas | [Task-verified-0037.md](Verified/Task-verified-0037.md) | ✅ Verified |
| T-0038 | Generalize Object CRUD Facade to All Object Types | [Task-verified-0038.md](Verified/Task-verified-0038.md) | ✅ Verified |
| T-0039 | Integration Tests for All Object Types CRUD | [Task-verified-0039.md](Verified/Task-verified-0039.md) | ✅ Verified |
| T-0040 | Asset Metadata Sidecar Schema (`AssetMetaJson`) | [Task-verified-0040.md](Verified/Task-verified-0040.md) | ✅ Verified |
| T-0041 | `importAsset` / `listAssets` / `removeAsset` Facade Methods | [Task-verified-0041.md](Verified/Task-verified-0041.md) | ✅ Verified |
| T-0042 | Integration Tests for Asset Operations | [Task-verified-0042.md](Verified/Task-verified-0042.md) | ✅ Verified |
| T-0043 | Comments Schema (`CommentJson`) and Comment Types | [Task-verified-0043.md](Verified/Task-verified-0043.md) | ✅ Verified |
| T-0044 | `addComment` / `listComments` / `resolveComment` Facade Methods | [Task-verified-0044.md](Verified/Task-verified-0044.md) | ✅ Verified |
| T-0045 | Integration Tests for Comment Operations | [Task-verified-0045.md](Verified/Task-verified-0045.md) | ✅ Verified |
| T-0046 | Inbox — `listInbox` / `importFromInbox` Facade Methods | [Task-verified-0046.md](Verified/Task-verified-0046.md) | ✅ Verified |
| T-0047 | `ScriviCoreAdapter` — Expose All EP-005 Facade Methods | [Task-verified-0047.md](Verified/Task-verified-0047.md) | ✅ Verified |
| T-0048 | `JsonDoc` Double Support — `setDouble` / `getDouble` | [Task-verified-0048.md](Verified/Task-verified-0048.md) | ✅ Verified |
| T-0049 | `KeychainSecureStore` — macOS Keychain Implementation | [Task-verified-0049.md](Verified/Task-verified-0049.md) | ✅ Verified |
| T-0050 | macOS App Target — Xcode Project Setup | [Task-verified-0050.md](Verified/Task-verified-0050.md) | ✅ Verified |
| T-0051 | ScriviEngine Bootstrap — `AppEnvironment` Observable | [Task-verified-0051.md](Verified/Task-verified-0051.md) | ✅ Verified |
| T-0052 | SwiftUI Shell — Landing View, Editor View, and Project Flows | [Task-verified-0052.md](Verified/Task-verified-0052.md) | ✅ Verified |
| T-0053 | `saveScene` Wiring — ⌘S Save in SwiftUI Editor | [Task-verified-0053.md](Verified/Task-verified-0053.md) | ✅ Verified |
| T-0054 | EP-006 Verification — `swift test` + `ctest` Green | [Task-verified-0054.md](Verified/Task-verified-0054.md) | ✅ Verified |
| T-0055 | Ubuntu CMake Build — GCC/Clang Green + Gap Document | [Task-verified-0055.md](Verified/Task-verified-0055.md) | ✅ Verified |
| T-0056 | Windows CMake Build — MSVC Green + Gap Document | [Task-verified-0056.md](Verified/Task-verified-0056.md) | ✅ Verified |
| T-0057 | `AppSupportLayout` — Linux and Windows Platform Paths | [Task-verified-0057.md](Verified/Task-verified-0057.md) | ✅ Verified |
| T-0058 | SecureStore Trade Study — Linux and Windows | [Task-verified-0058.md](Verified/Task-verified-0058.md) | ✅ Verified |
| T-0059 | `OpenProjectResult` — Add Scene List | [Task-verified-0059.md](Verified/Task-verified-0059.md) | ✅ Verified |
| T-0060 | `openScene` Facade Method — Switch Active Scene | [Task-verified-0060.md](Verified/Task-verified-0060.md) | ✅ Verified |
| T-0061 | Adapter + Swift Engine — Expose `openScene` and Scene List | [Task-verified-0061.md](Verified/Task-verified-0061.md) | ✅ Verified |
| T-0062 | Integration Tests — Multi-Scene `openProject` and `openScene` | [Task-verified-0062.md](Verified/Task-verified-0062.md) | ✅ Verified |
| T-0063 | clang-tidy — `readability-braces-around-statements` (142 warnings) | [Task-verified-0063.md](Verified/Task-verified-0063.md) | ✅ Verified |
| T-0064 | clang-tidy — `modernize-use-designated-initializers` + `readability-qualified-auto` (51 warnings) | [Task-verified-0064.md](Verified/Task-verified-0064.md) | ✅ Verified |
| T-0065 | clang-tidy — `[[nodiscard]]`, `const`, and `static` member functions (21 warnings) | [Task-verified-0065.md](Verified/Task-verified-0065.md) | ✅ Verified |
| T-0066 | clang-tidy — Remaining checks (20 warnings) | [Task-verified-0066.md](Verified/Task-verified-0066.md) | ✅ Verified |
| T-0067 | `CreateSceneRequest` / `CreateSceneResult` types + `ScriviCore::createScene()` facade | [Task-verified-0067.md](Verified/Task-verified-0067.md) | ✅ Verified |
| T-0068 | `SceneCreator` implementation — slug generation, file writes, chapter index update | [Task-verified-0068.md](Verified/Task-verified-0068.md) | ✅ Verified |
| T-0069 | `CreateChapterRequest` / `CreateChapterResult` types + `ScriviCore::createChapter()` facade | [Task-verified-0069.md](Verified/Task-verified-0069.md) | ✅ Verified |
| T-0070 | `ChapterCreator` implementation — slug generation, file writes, manuscript index update, auto first scene | [Task-verified-0070.md](Verified/Task-verified-0070.md) | ✅ Verified |
| T-0071 | Integration tests — `createScene` ordering, insert-after-current, `createChapter` with auto-scene | [Task-verified-0071.md](Verified/Task-verified-0071.md) | ✅ Verified |
| T-0083 | `SceneNavigatorView` — sidebar List in manuscript order | [Task-verified-0083.md](Verified/Task-verified-0083.md) | ✅ Verified |
| T-0084 | Navigator title derivation — first line or "Scene X"; chapter headers | [Task-verified-0084.md](Verified/Task-verified-0084.md) | ✅ Verified |
| T-0085 | Live title updates — ~300ms debounce | [Task-verified-0085.md](Verified/Task-verified-0085.md) | ✅ Verified |
| T-0086 | Click-to-navigate — load scene, scroll editor | [Task-verified-0086.md](Verified/Task-verified-0086.md) | ✅ Verified |
| T-0087 | Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM | [Task-verified-0087.md](Verified/Task-verified-0087.md) | ✅ Verified |
| T-0090 | Remove `ScriviCoreAdapter` target from Xcode — update `project.pbxproj` | SP-026 (no separate file) | ✅ Verified |
| T-0091 | Replace adapter module map with `ScriviCore` plain-C module map (`scrivi.h`) | SP-026 (no separate file) | ✅ Verified |
| T-0092 | Rewrite `ScriviEngine.swift` — call `scrivi_*` C functions; `scrivi_free()` after each call | SP-026 (no separate file) | ✅ Verified |
| T-0093 | Update `ScriviInteropTests.swift` — fix `@testable import` to correct module name | SP-026 (no separate file) | ✅ Verified |
| T-0094 | Verify: Xcode build clean, `ScriviInteropTests` green, `ctest` count unchanged | SP-026 (no separate file) | ✅ Verified |
| T-0106 | Wire `reorderScene`/`reorderChapter` through C API and `ScriviEngine.swift` | [Task-verified-0106.md](Verified/Task-verified-0106.md) | ✅ Verified |
| T-0107 | SwiftUI drag-and-drop for scene rows (within-chapter + cross-chapter) | [Task-verified-0107.md](Verified/Task-verified-0107.md) | ✅ Verified |
| T-0108 | SwiftUI drag-and-drop for chapter rows + insertion-line highlight | [Task-verified-0108.md](Verified/Task-verified-0108.md) | ✅ Verified |
| T-0109 | Global chapter title toggle + headings in writing surface | SP-032 inline | ✅ Verified |
| T-0110 | Navigator title fallback chain + delete-of-open-scene edge case + cursor placement | SP-032 inline | ✅ Verified |
| T-0113 | EP-011 Behavior Spec — scroll, cursor, separator, focus, delete rules | [Task-verified-0113.md](Verified/Task-verified-0113.md) | ✅ Verified |
| T-0114 | All-scenes-in-memory viewport strategy — replace load/release cycle | [Task-verified-0114.md](Verified/Task-verified-0114.md) | ✅ Verified |
| T-0115 | Global cursor position tracking + scene start position map | [Task-verified-0115.md](Verified/Task-verified-0115.md) | ✅ Verified |
| T-0116 | Correct cursor placement after navigate and delete | [Task-verified-0116.md](Verified/Task-verified-0116.md) | ✅ Verified |
| T-0117 | Reliable first-responder transfer — replace `focusManuscriptView` workaround | [Task-verified-0117.md](Verified/Task-verified-0117.md) | ✅ Verified |
| T-0119 | EP-011 verification — macOS smoke test + deferred EP-010 AC | [Task-verified-0119.md](Verified/Task-verified-0119.md) | ✅ Verified |
| T-0120 | Remove toolbar strip and add Project menu (Close Project, Project Settings) | [Task-verified-0120.md](Verified/Task-verified-0120.md) | ✅ Verified |
| T-0121 | Add Edit menu with standard responder-chain commands and keyboard shortcuts | [Task-verified-0121.md](Verified/Task-verified-0121.md) | ✅ Verified |
| T-0122 | Add About menu with About panel (name, version, build) and User Manual placeholder | [Task-verified-0122.md](Verified/Task-verified-0122.md) | ✅ Verified |
| T-0123 | iPhone affordance for Close Project / Project Settings (iOS Master/Detail rework; AC5) | [Task-verified-0123.md](Verified/Task-verified-0123.md) | ✅ Verified |
| T-0124 | EP-012 verification | [Task-verified-0124.md](Verified/Task-verified-0124.md) | ✅ Verified |
| T-0128 | SceneInspectorView skeleton — panel, tab bar structure | [Task-verified-0128.md](Verified/Task-verified-0128.md) | ✅ Verified |
| T-0129 | Scene Entities stub tab — empty state, placeholder Add Entity button | [Task-verified-0129.md](Verified/Task-verified-0129.md) | ✅ Verified |
| T-0130 | Inspector panel integration into ManuscriptEditorView — width, collapse, expand | [Task-verified-0130.md](Verified/Task-verified-0130.md) | ✅ Verified |
| T-0131 | iPhone exclusion — Inspector absent on phone idiom | [Task-verified-0131.md](Verified/Task-verified-0131.md) | ✅ Verified |
| T-0132 | EP-014 verification | [Task-verified-0132.md](Verified/Task-verified-0132.md) | ✅ Verified |
| T-0133 | TimelineStripView skeleton — panel chrome, horizontal scroll container | [Task-verified-0133.md](Verified/Task-verified-0133.md) | ✅ Verified |
| T-0134 | Placeholder graphical event markers — static stub layout | [Task-verified-0134.md](Verified/Task-verified-0134.md) | ✅ Verified |
| T-0135 | Timeline integration into ManuscriptEditorView — docking, height, collapse | [Task-verified-0135.md](Verified/Task-verified-0135.md) | ✅ Verified |
| T-0136 | iPhone exclusion — Timeline absent on phone idiom | [Task-verified-0136.md](Verified/Task-verified-0136.md) | ✅ Verified |
| T-0137 | EP-015 verification | [Task-verified-0137.md](Verified/Task-verified-0137.md) | ✅ Verified |
| T-0149 | `TimelineStripView` rebuild — horizontal line, scene dots, manuscript-order layout | [Task-verified-0149.md](Verified/Task-verified-0149.md) | ✅ Verified |
| T-0150 | Scene dot drag — horizontal gesture, story-time update | [Task-verified-0150.md](Verified/Task-verified-0150.md) | ✅ Verified |
| T-0151 | Time Delta Picker — spinner, named anchors, dismiss behavior | [Task-verified-0151.md](Verified/Task-verified-0151.md) | ✅ Verified |
| T-0152 | Time Delta Picker — pre-population and anchor base | [Task-verified-0152.md](Verified/Task-verified-0152.md) | ✅ Verified |
| T-0153 | Scene dot tooltip / popover — title, chapter, human-readable story-time | [Task-verified-0153.md](Verified/Task-verified-0153.md) | ✅ Verified |
| T-0154 | Timeline panel resize — drag top edge, dynamic minimum height | [Task-verified-0154.md](Verified/Task-verified-0154.md) | ✅ Verified |
| T-0155 | Expand Timeline Forward/Backward popover | [Task-verified-0155.md](Verified/Task-verified-0155.md) | ✅ Verified |
| T-0168 | Scene duration, chain propagation, project timeline defaults, "Immediately after" | [Task-verified-0168.md](Verified/Task-verified-0168.md) | ✅ Verified |
| T-0176 | Resolve design gates — boundary (Option A) & adapter-location reconciliation | [Task-verified-0176.md](Verified/Task-verified-0176.md) | ✅ Verified |
| T-0177 | Define indexable record schema (fields/attributes per item type) | [Task-verified-0177.md](Verified/Task-verified-0177.md) | ✅ Verified |
| T-0178 | ScriviCore `extractSearchableText(projectPath) -> JSON` read-only facade | [Task-verified-0178.md](Verified/Task-verified-0178.md) | ✅ Verified |
| T-0179 | Unit + integration tests for the indexing facade against a fixture project | [Task-verified-0179.md](Verified/Task-verified-0179.md) | ✅ Verified |
| T-0180 | Spotlight/Core Spotlight entitlements + project config | [Task-verified-0180.md](Verified/Task-verified-0180.md) | ✅ Verified |
| T-0181 | `ScriviEngine` Swift API to fetch indexable records (calls the facade) | [Task-verified-0181.md](Verified/Task-verified-0181.md) | ✅ Verified |
| T-0182 | Donate `CSSearchableItem`s on project open/save; delete-by-domain on close | [Task-verified-0182.md](Verified/Task-verified-0182.md) | ✅ Verified |
| T-0183 | Markdown→plain-text extraction for body indexing | [Task-verified-0183.md](Verified/Task-verified-0183.md) | ✅ Verified |

## Closed Tasks

Currently: **2 closed Tasks**

| Task | Title | File | Status |
| ---- | ----- | ---- | ------ |
| T-0125 | Two-finger swipe gesture on ManuscriptTextView — macOS (trackpad) | [Task-closed-0125.md](Closed/Task-closed-0125.md) | 🔴 Closed |
| T-0127 | EP-013 verification | [Task-closed-0127.md](Closed/Task-closed-0127.md) | 🔴 Closed |

## Statistics

- **Total Tasks:** 183
- **Backlog:** 1 🔵 (T-0118). *(EP-017: T-0189/T-0190 now Active in SP-047; T-0185–T-0188 ⚪ Descoped per I-0057 — `CSImportExtension` non-functional on macOS.)*
- **Superseded:** 1 ⚪ (T-0175 → expanded into EP-017)
- **Active:** 2 🟡 (SP-047 tasks T-0189, T-0190)
- **Unverified:** 0
- **Next available:** T-0191

> Note: EP-016 tasks (T-0167, T-0170–T-0174) verified and archived to `Verified/` on 2026-06-23;
> SP-044 tasks (T-0176–T-0179) verified and archived 2026-06-23. Some historical aggregate counts
> above predate a full re-audit and may lag.

---

*Last Updated: 2026-07-01 (SP-047 activated — T-0189/T-0190 set Active; T-0185–T-0188 Descoped per I-0057. Stale SP-042 items T-0166/T-0169/T-0170 user-verified and archived to Verified/ — removed from all active/backlog lists. Active-count stat corrected to 2; backlog 22→21.)*
