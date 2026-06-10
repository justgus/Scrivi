## T-0129: Scene Entities Stub Tab — Empty State, Placeholder Add Entity Button

**Status:** ✅ Implemented - Verified
**Component:** `SceneInspectorView.swift`
**Priority:** High
**Date Requested:** 2026-06-10
**Date Implemented:** 2026-06-10
**Date Verified:** 2026-06-10
**Sprint Assigned:** SP-037
**Epic:** EP-014

**Implementation Details:**
Private struct `SceneEntitiesTabView` inside `SceneInspectorView.swift`. `VStack(spacing: 12)` with: `Text("Scene Entities").font(.headline)` left-aligned with padding; `Spacer()`; `Text("No entities yet.").foregroundStyle(.secondary)` centered; `Button("Add Entity") { }.disabled(true)` centered; `Spacer()`.
