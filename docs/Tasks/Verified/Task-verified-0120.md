# T-0120: Remove toolbar strip and add Project menu

**Status:** ✅ Verified
**Epic:** EP-012
**Sprint:** SP-035

**Description:**
Removed the custom toolbar strip (`private var toolbar`) from `ManuscriptEditorView`. Added a `Project` `CommandMenu` in `ScriviApp.swift` with Open Project (`⌘O`), Project Settings (`⌘,`), and Close Project (`⌘W`). Project Settings and Close Project are disabled when no project is open. The `closeProject()` method was extracted from the inline toolbar action into `AppEnvironment`.

**Files Affected:**
- `Scrivi/Views/EditorView.swift` — toolbar removed
- `Scrivi/App/ScriviApp.swift` — Project CommandMenu added
- `Scrivi/App/AppEnvironment.swift` — `closeProject()` and `presentOpenProjectPanel()` extracted

**Verification:**
- ✅ No toolbar strip visible above the manuscript
- ✅ Project menu appears in menu bar with correct items and shortcuts
- ✅ Close Project and Project Settings disabled when no project is open
