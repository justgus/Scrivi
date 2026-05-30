# Active Sprint

## SP-016: saveScene Wiring and EP-006 Close

**Status:** 🟡 Active
**Epic:** EP-006: Swift Interop and Apple Shell
**Goal:** Wire `saveScene` end-to-end: ⌘S triggers a call through `AppEnvironment` → `ScriviEngine.saveScene` → adapter → C++ core, writing the editor's current Markdown to disk. Cursor offset and scroll position are read from the SwiftUI editor and passed as real values (not zeroes). Confirm all `swift test` and `ctest` suites still pass, then close EP-006.
**Start Date:** 2026-05-30
**End Date:** —
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0053 | `saveScene` Wiring — ⌘S Save in SwiftUI Editor | Critical | 🟡 Active |
| T-0054 | EP-006 Verification — `swift test` + `ctest` Green | Critical | 🔵 Backlog |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

`saveScene` in `ScriviEngine` already exists and is fully wired to the adapter (added in EP-002). The gap is purely in the SwiftUI layer: `EditorView` has no save action, and `AppEnvironment` has no `saveScene` method. Cursor offset capture via `TextEditor` uses `@FocusState` + `NSTextView` introspection or a simple deferred approach; scroll position is similarly approximated. The adapter already accepts `cursorOffset` and `scrollPosition` as `double` (T-0048).

---

*Last Updated: 2026-05-30 (SP-016 activated)*
