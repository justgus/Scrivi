## T-0130: Inspector Panel Integration into ManuscriptEditorView — Width, Collapse, Expand

**Status:** ✅ Implemented - Verified
**Component:** `EditorView.swift`, `AppEnvironment.swift`, `ScriviApp.swift`
**Priority:** High
**Date Requested:** 2026-06-10
**Date Implemented:** 2026-06-10
**Date Verified:** 2026-06-10
**Sprint Assigned:** SP-037
**Epic:** EP-014

**Implementation Details:**
`AppEnvironment` gained `var inspectorVisible: Bool = true`. `ManuscriptEditorView` detail column is now an `HStack(spacing: 0)` containing the manuscript `VStack` on the left and `SceneInspectorView()` conditionally on the right when `env.inspectorVisible`. `ScriviApp.swift` gained `CommandMenu("View")` with `Toggle("Show Scene Inspector", isOn: Bindable(env).inspectorVisible).keyboardShortcut("i", modifiers: [.command, .option])`, disabled when `env.openProjectResult == nil`.
