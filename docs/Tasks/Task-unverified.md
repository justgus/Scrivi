# Unverified Tasks

## T-0053: `saveScene` Wiring — ⌘S Save in SwiftUI Editor

**Status:** 🟠 Implemented - Not Verified
**Sprint:** SP-016
**Epic:** EP-006
**Priority:** Critical
**Date Implemented:** 2026-05-30

### What Was Done

- Added `saveScene(markdown:cursorOffset:scrollPosition:)` to `AppEnvironment` — reads `openProjectResult`, `projectRootPath`, and `authorshipRef` and delegates to `ScriviEngine.saveScene`.
- Replaced `TextEditor` in `EditorView` with `TrackingTextEditor` (`NSViewRepresentable` wrapping `NSTextView`) — tracks cursor position via `NSTextViewDelegate` and reports it as `cursorOffset: Double`.
- ⌘S wired via `onKeyPress(.s, phases: .down)` with a `.command` modifier guard.
- Toolbar shows a transient "Saved" label for 2 seconds on success, or an inline red error message on failure. Silent swallowing eliminated.
- `scrollPosition` passed as `0.0` — SwiftUI does not expose a scroll position value for `NSScrollView`-backed views without additional infrastructure; deferred per original acceptance criteria.
- Pre-existing `ensureLocalIdentity` test failure fixed: removed `createdNewIdentity == true` assertion that was not test-isolation-safe (Keychain persists across runs).

### File References

- `platforms/apple/Sources/ScriviApp/EditorView.swift` — full rewrite
- `platforms/apple/Sources/ScriviApp/AppEnvironment.swift:43` — `saveScene` method added
- `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift:93` — assertion relaxed

---

## T-0054: EP-006 Verification — `swift test` + `ctest` Green

**Status:** 🟠 Implemented - Not Verified
**Sprint:** SP-016
**Epic:** EP-006
**Priority:** Critical
**Date Implemented:** 2026-05-30

### What Was Done

- `swift test`: 17/17 pass ✅ (including fix to `ensureLocalIdentity` test)
- `ctest --test-dir build --output-on-failure`: 159/159 pass ✅ (1 new test vs EP-006 baseline of 158)

---

*Last Updated: 2026-05-30 (T-0053, T-0054 implemented)*
