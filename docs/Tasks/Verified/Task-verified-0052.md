# T-0052: SwiftUI Shell — Landing View, Editor View, and Project Flows

**Status:** ✅ Verified
**Component:** platforms/apple/Sources/ScriviApp/ (LandingView, EditorView, NewProjectSheet)
**Priority:** Critical
**Date Requested:** 2026-05-29
**Date Implemented:** 2026-05-29
**Date Verified:** 2026-05-29
**Sprint Assigned:** SP-015

**Implementation Details:**
- `LandingView.swift`: "Open Project…" (NSOpenPanel, directories only) + "New Project…"; `repairRequired` mode shows inline error; sheet for `NewProjectSheet`
- `EditorView.swift`: `TextEditor` bound to `@State var markdown`; initialized `onAppear` from `openProjectResult?.activeScene?.markdown`; "Close Project" button; no ⌘S wiring (deferred to SP-016)
- `NewProjectSheet.swift`: Title + Slug fields; slug auto-populated via `onChange`; `NSSavePanel` for location; slug validated non-empty; inline error display; `dismiss()` on success
- `ContentRootView` in `ScriviApp.swift` dispatches `LandingView`/`EditorView` based on `env.openProjectResult`
- `.scrivi` project is a directory package on disk — Finder shows it as a folder during development (UTI registration requires full app install); correct behavior
- `createProject` followed immediately by `openProject` confirmed working end-to-end

---

*Verified: 2026-05-29 (user approved)*
