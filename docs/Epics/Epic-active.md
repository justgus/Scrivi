# Active Epics

## EP-006: Swift Interop and Apple Shell

**Status:** 🟠 Complete - Pending Close
**Goal:** Deliver a working macOS app that opens and writes a `.scrivi` project through the full stack — SwiftUI shell → ScriviEngine → ScriviCoreAdapter → ScriviCore — with production-quality service wiring (Keychain identity, real file I/O) and enough UI to prove the end-to-end vertical slice.
**Date Created:** 2026-05-28
**Start Date:** 2026-05-28
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [x] `KeychainSecureStore` implemented; identity (identityID + private key material) survives process restart on macOS
- [x] macOS app target added to `platforms/apple/`; builds and runs on macOS 26.0+
- [x] `SwiftUI` document-based app shell presents a single-window editor for a `.scrivi` project
- [x] `openProject` loads the active scene and displays its Markdown content in a `TextEditor`
- [x] `saveScene` writes the editor's current text back to disk on explicit save (⌘S)
- [x] `createProject` flow reachable from the app (new document or first-launch path)
- [x] `JsonDoc` gains `setDouble`/`getDouble`; `ScrollPosition.value` stored as a true `double` on disk
- [x] `saveScene` in Swift passes live cursor offset and scroll position from the SwiftUI editor
- [x] All existing `swift test` interop tests still pass (17/17)
- [x] `ctest --test-dir build --output-on-failure` still passes (159/159)

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-014 | Infrastructure — Keychain Identity and JsonDoc Double | ✅ Closed | 2026-05-28 – 2026-05-29 |
| SP-015 | macOS App Target and SwiftUI Shell | ✅ Closed | 2026-05-29 – 2026-05-29 |
| SP-016 | saveScene Wiring and EP-006 Close | 🟡 Active | 2026-05-30 – — |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0048 | `JsonDoc` Double Support — `setDouble` / `getDouble` | ✅ Verified |
| T-0049 | `KeychainSecureStore` — macOS Keychain Implementation | ✅ Verified |
| T-0050 | macOS App Target — Xcode Project Setup | ✅ Verified |
| T-0051 | ScriviEngine Bootstrap — `AppEnvironment` Observable | ✅ Verified |
| T-0052 | SwiftUI Shell — Landing View, Editor View, and Project Flows | ✅ Verified |
| T-0053 | `saveScene` Wiring — ⌘S Save in SwiftUI Editor | 🟠 Not Verified |
| T-0054 | EP-006 Verification — `swift test` + `ctest` Green | 🟠 Not Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

**Baseline entering EP-006:**
- ScriviEngine exposes all 25 facade methods (T-0047, EP-005 complete).
- `PrototypeSecureStore` is in-memory only — identity is lost on every process restart.
- No macOS app target exists; the Apple layer is a library + test harness only.
- `JsonDoc` has no `setDouble`/`getDouble`; scroll position is stored as `int * 1000` workaround.
- `saveScene` in Swift passes zeroes for cursor and scroll (adapter accepts them; wiring deferred).
- `openProject` returns a single `activeScene`; multi-scene manifest design is deferred.

**Open questions from Architecture v0.3 addressed in this Epic:**
- Open Question 2 (`JsonDoc` double) → T-0048 or equivalent
- Open Question 3 (`KeychainSecureStore`) → first sprint
- Open Question 4 (cursor/scroll wiring from SwiftUI) → UI sprint

**Deferred out of EP-006:**
- Multi-scene project manifest (`openProject` richer result) — deferred to EP-007 or later
- Device identity (`IOPlatformExpertDevice` or Keychain UUID as `deviceID`) — deferred
- `appSupportRoot` bootstrap on non-Apple platforms — not in scope
- Repair UI — deferred
- CloudKit sync — future Epic
- iOS / iPadOS / visionOS UI — future Epics
- Git snapshot UI — future Epic

### Completion Summary

*(Filled in when EP-006 reaches 🟠 Complete)*

---

*Last Updated: 2026-05-30 (T-0053, T-0054 implemented; all acceptance criteria met; EP-006 complete pending user close)*
