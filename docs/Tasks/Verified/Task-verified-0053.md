# T-0053: `saveScene` Wiring — ⌘S Save in SwiftUI Editor

**Status:** ✅ Verified
**Component:** platforms/apple/Sources/ScriviApp/ (EditorView, AppEnvironment)
**Priority:** Critical
**Date Requested:** 2026-05-30
**Date Implemented:** 2026-05-30
**Date Verified:** 2026-05-30
**Sprint Assigned:** SP-016

**Implementation Details:**
- `saveScene(markdown:cursorOffset:scrollPosition:)` added to `AppEnvironment` — reads `openProjectResult`, `projectRootPath`, and `authorshipRef`; delegates to `ScriviEngine.saveScene`
- `TextEditor` replaced with `TrackingTextEditor` (`NSViewRepresentable` wrapping `NSTextView`) — tracks cursor position via `NSTextViewDelegate`, reports as `cursorOffset: Double`
- ⌘S wired via `onKeyPress(.s, phases: .down)` with `.command` modifier guard
- Toolbar shows transient "Saved" label for 2 seconds on success; inline red error message on failure
- `scrollPosition` passed as `0.0` — SwiftUI does not expose scroll position for `NSScrollView`-backed views without additional infrastructure; deferred per acceptance criteria
- Pre-existing `ensureLocalIdentity` test failure fixed: removed `createdNewIdentity == true` assertion that was not test-isolation-safe (Keychain persists across runs)

---

*Verified: 2026-05-30 (user approved)*
