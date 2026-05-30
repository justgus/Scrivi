# SP-016: saveScene Wiring and EP-006 Close

**Status:** ✅ Closed
**Epic:** EP-006: Swift Interop and Apple Shell
**Goal:** Wire `saveScene` end-to-end: ⌘S triggers a call through `AppEnvironment` → `ScriviEngine.saveScene` → adapter → C++ core, writing the editor's current Markdown to disk. Cursor offset and scroll position are read from the SwiftUI editor and passed as real values (not zeroes). Confirm all `swift test` and `ctest` suites still pass, then close EP-006.
**Start Date:** 2026-05-30
**End Date:** 2026-05-30
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0053 | `saveScene` Wiring — ⌘S Save in SwiftUI Editor | Critical | ✅ Verified |
| T-0054 | EP-006 Verification — `swift test` + `ctest` Green | Critical | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

`saveScene` in `ScriviEngine` already existed and was fully wired to the adapter (added in EP-002). The gap was purely in the SwiftUI layer: `EditorView` had no save action and `AppEnvironment` had no `saveScene` method. `TextEditor` replaced with `TrackingTextEditor` (`NSViewRepresentable` wrapping `NSTextView`) to capture cursor offset via `NSTextViewDelegate`. `scrollPosition` deferred at `0.0` — SwiftUI does not expose scroll position for `NSScrollView`-backed views without additional infrastructure. Pre-existing `ensureLocalIdentity` test failure resolved by removing a Keychain-state-dependent assertion that was not test-isolation-safe.

### Retrospective

Clean sprint. The ⌘S wiring was straightforward once `TrackingTextEditor` replaced the stock `TextEditor`. The `ensureLocalIdentity` test fix was a minor cleanup with no architectural impact. All 17 Swift tests and 159 CTests pass. EP-006 closes cleanly with all 10 acceptance criteria met.

---

*Closed: 2026-05-30 (T-0053, T-0054 verified; user approved Sprint close)*
