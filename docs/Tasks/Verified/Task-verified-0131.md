## T-0131: iPhone Exclusion — Inspector Absent on Phone Idiom

**Status:** ✅ Implemented - Verified
**Component:** `EditorView.swift`
**Priority:** Medium
**Date Requested:** 2026-06-10
**Date Implemented:** 2026-06-10
**Date Verified:** 2026-06-10
**Sprint Assigned:** SP-037
**Epic:** EP-014

**Implementation Details:**
Inside the `HStack` in `ManuscriptEditorView`, the `SceneInspectorView` is rendered via a platform conditional: on iOS, it is shown only when `UIDevice.current.userInterfaceIdiom != .phone && env.inspectorVisible`; on all other platforms (macOS, visionOS) it is shown when `env.inspectorVisible`. iPhone never sees the inspector; iPad and macOS do.
