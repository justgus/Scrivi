## T-0135: Timeline Integration into ManuscriptEditorView — Docking, Height, Collapse

**Status:** ✅ Implemented - Verified
**Component:** `EditorView.swift`, `AppEnvironment.swift`, `ScriviApp.swift`, `project.pbxproj`
**Priority:** High
**Date Requested:** 2026-06-10
**Date Implemented:** 2026-06-10
**Date Verified:** 2026-06-10
**Sprint Assigned:** SP-038
**Epic:** EP-015

**Implementation Details:**
`AppEnvironment` gained `var timelineVisible: Bool = true`. `ManuscriptEditorView` manuscript `VStack` has `TimelineStripView()` at the bottom conditional on `env.timelineVisible`. `ScriviApp.swift` `CommandMenu("View")` gained `Toggle("Show Timeline", isOn: Bindable(env).timelineVisible).keyboardShortcut("t", modifiers: [.command, .option])` disabled when no project open. `project.pbxproj` updated with `C035`/`C035_BF`, `D035_BF`, `E035_BF` for all three app target Sources phases.
