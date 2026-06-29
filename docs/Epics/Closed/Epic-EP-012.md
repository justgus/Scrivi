# Epic EP-012 — Closed

## EP-012: Menu Bar and Toolbar Replacement

**Status:** ✅ Closed
**Goal:** Replace the custom toolbar strip in `EditorView` with a proper menu bar (macOS/iPadOS) and
idiom-appropriate in-view affordances (iPhone), eliminating the toolbar strip and reclaiming that
vertical space for the manuscript surface.
**Date Created:** 2026-06-09
**Actual Close Date:** 2026-06-29
**Span:** SP-035 (initial macOS/iPadOS menu bar + About panel), with AC5 (iPhone) and the iPad
reachability gap (I-0054) closed later via the 2026-06-29 iOS Master/Detail rework.
**Origin:** The manuscript editor carried a custom in-view toolbar strip for Close Project / Project
Settings. Replacing it with the platform menu bar reclaims vertical space and aligns with platform
conventions.

### Acceptance Criteria — all met

- [x] A **Project** menu replaces the legacy "File" menu label and contains **Close Project** and **Project Settings…**. *(Verified, T-0120/T-0124.)*
- [x] An **Edit** menu provides standard system commands (Undo/Redo/Cut/Copy/Paste/Select All) via the responder chain. *(Verified.)*
- [x] A **Help/About** surface contains **About Scrivi** and a placeholder **User Manual** (disabled). *(Verified.)*
- [x] The custom toolbar strip is removed entirely; no vertical space reserved for it. *(Verified.)*
- [x] **AC5** — On **iPhone**, Close Project and Project Settings remain reachable in-view. *(Verified 2026-06-29 — delivered via the iOS Master/Detail nav-bar `•••` menu; see Outcome.)*
- [x] The About panel shows app name, version, and build number. *(Verified.)*
- [x] Menu items carry correct keyboard shortcuts (Close `⌘W`, Settings `⌘,` on macOS, standard Edit shortcuts). *(Verified.)*
- [x] No regression in Close Project / Project Settings behavior. *(Verified.)*

### Outcome — AC5 diverged from the original phoneToolbar design

The original AC5 plan (T-0123) was a `#if os(iOS)` `phoneToolbar` HStack in the editor's detail
column, gated to `userInterfaceIdiom == .phone`. When finally verified live on an iPhone 17 Pro /
iOS 27.0 (2026-06-28/29), that approach **did not work**: the iPhone landed in a chrome-less detail
with no navigation and no toolbar (a leaked macOS `minWidth: 700` frame pushed the
`NavigationSplitView` sidebar off the ~393pt screen), and the Project Settings button was never
wired to a sheet on iOS.

The shipped solution reworked iOS onto a single **Master/Detail `NavigationSplitView`** for all
idioms (side-by-side on iPad / iPhone landscape; navigator→manuscript push with a back chevron on
iPhone portrait), with a selection binding driving the detail, an iOS nav-bar **`•••` menu**
(Project Settings, Show Inspector, Show Timeline, Close Project) replacing the toolbar, the Project
Settings **sheet wired on iOS**, and an iPad **hardware-keyboard menu bar** (`ScriviApp.iosCommands`:
File New/Open/Close, Project Settings + view toggles) deconflicted against the iOS-synthesized menus.
On iOS, **New/Open close the open project first** so the single-scene Landing host can present the
sheet/importer. Full detail in **T-0123**'s 2026-06-29 Resolution addendum.

This same work closed **I-0054** (iPad: Project Settings / Close Project were unreachable — no phone
toolbar on iPad and no iOS menu bar), which had been surfaced while verifying T-0123.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0120 | Remove toolbar strip and add Project menu (Close Project, Project Settings) | ✅ Verified |
| T-0121 | Edit menu (standard responder-chain commands) | ✅ Verified |
| T-0122 | About panel (Scrivi/About menu) | ✅ Verified |
| T-0123 | iPhone in-view affordance for Close Project / Project Settings | ✅ Verified (2026-06-29, iOS Master/Detail rework) |
| T-0124 | EP-012 verification | ✅ Verified |

### Related Issues

- **I-0054** — iPad reachability of Project Settings / Close Project. ✅ Resolved - Verified (2026-06-29), in the same rework.

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-035 | Menu Bar, Project Commands, and About Panel | ✅ Closed |

### Components touched (AC5 / I-0054 rework, 2026-06-29)

- `Scrivi/Views/EditorView.swift` — single Master/Detail NavigationSplitView; iOS `•••` toolbar; Project Settings sheet; macOS-only `minWidth` constraint.
- `Scrivi/Views/SceneNavigatorView.swift` — optional `selection` binding; iOS selection-driven navigation; two-way highlight sync.
- `Scrivi/App/ScriviApp.swift` — `iosCommands` (iPad menu bar), `.commands` on the iOS WindowGroup.
- `Scrivi/App/AppEnvironment.swift` — `presentNewProject`/`presentOpenImporter` close-then-present on iOS; `requestOpenImporter` flag; `closeActiveProjectForLanding`.
- `Scrivi/Views/LandingView.swift` — observes `requestOpenImporter` to raise the document importer.

### Notes

- **No ScriviCore/C++ changes** — Swift/UI layer only.
- **"Save as you go"** — no explicit save UI by design (per user, 2026-06-29); save/flush incidents are tracked as Issues if they arise.
- Closed with explicit user approval (2026-06-29) alongside the T-0123 / I-0054 verification.
