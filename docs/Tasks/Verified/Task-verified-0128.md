## T-0128: SceneInspectorView Skeleton — Panel, Tab Bar Structure

**Status:** ✅ Implemented - Verified
**Component:** `SceneInspectorView.swift` (new file)
**Priority:** High
**Date Requested:** 2026-06-10
**Date Implemented:** 2026-06-10
**Date Verified:** 2026-06-10
**Sprint Assigned:** SP-037
**Epic:** EP-014

**Implementation Details:**
New file `Scrivi/Views/SceneInspectorView.swift`. `enum InspectorTab: String, CaseIterable, Identifiable` with `.entities` case. `SceneInspectorView` has `@State private var selectedTab: InspectorTab = .entities`, a `Picker` in `.segmented` style at the top, a `Divider`, and a `switch selectedTab` content block. Frame: `.frame(minWidth: 240, idealWidth: 280, maxWidth: 280)`. Left `Divider()` provided by the view itself. `project.pbxproj` updated with `C034`/`C034_BF`, `D034_BF`, `E034_BF` entries for all three app targets.
