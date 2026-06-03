## T-0083: `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/SceneNavigatorView.swift`, `Scrivi/Views/EditorView.swift`
**Priority:** High
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-024

**Rationale:**
Authors need a persistent, scannable list of all scenes to orient themselves in the manuscript and jump between scenes without scrolling.

**Desired Behavior:**
A sidebar list displays all scenes in manuscript order, grouped under non-selectable chapter headers. Each row shows the scene title. The active scene is highlighted.

**Implementation Details:**
Created `Scrivi/Views/SceneNavigatorView.swift`. SwiftUI `List` with `.sidebar` style inside a `NavigationSplitView` (added to `EditorView.swift`). Groups scenes by chapter into `Section` headers ("Chapter 1", "Chapter 2"…). Each row is a `NavigatorRow` showing the scene title; no IDs, slugs, or UUIDs visible to the author. Section headers are non-selectable. Active scene highlighted with accent color tint.

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift` — new file
- `Scrivi/Views/EditorView.swift` — replaced VStack with `NavigationSplitView`

**Verification:**
- ✅ Sidebar renders all scenes in manuscript order
- ✅ Chapter section headers are non-selectable
- ✅ Active scene highlighted with accent tint
- ✅ No IDs, slugs, or UUIDs visible

---

*Closed: 2026-06-03*
