## T-0136: iPhone Exclusion — Timeline Absent on Phone Idiom

**Status:** ✅ Implemented - Verified
**Component:** `EditorView.swift`
**Priority:** Medium
**Date Requested:** 2026-06-10
**Date Implemented:** 2026-06-10
**Date Verified:** 2026-06-10
**Sprint Assigned:** SP-038
**Epic:** EP-015

**Implementation Details:**
Inside the manuscript `VStack` in `ManuscriptEditorView`, `TimelineStripView()` is rendered via platform conditional: on iOS, shown only when `UIDevice.current.userInterfaceIdiom != .phone && env.timelineVisible`; on all other platforms when `env.timelineVisible`. Mirrors the T-0131 pattern for the inspector.
