# SP-015: macOS App Target and SwiftUI Shell

**Status:** ✅ Closed
**Epic:** EP-006: Swift Interop and Apple Shell
**Goal:** Add a native macOS app target to `ScriviCore.xcodeproj`, bootstrap identity at launch, and deliver a minimal SwiftUI shell proving the end-to-end path: `openProject` → `TextEditor` display and a reachable `createProject` flow. `saveScene` (⌘S) and cursor/scroll wiring deferred to SP-016.
**Start Date:** 2026-05-29
**End Date:** 2026-05-29
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0050 | macOS App Target — Xcode Project Setup | Critical | ✅ Verified |
| T-0051 | ScriviEngine Bootstrap — `AppEnvironment` Observable | Critical | ✅ Verified |
| T-0052 | SwiftUI Shell — Landing View, Editor View, and Project Flows | Critical | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

App sandbox disabled for this sprint — `NSOpenPanel`/`NSSavePanel` work without security-scoped bookmark machinery. The SPM package at `platforms/apple/` is linked into the Xcode app target via `XCLocalSwiftPackageReference`. `.task` bootstrap placed on root `View` (not `WindowGroup`) due to Xcode 26 resolution behavior. `ScriviError.init` made `public` to allow cross-module construction. Ad-hoc signing (`CODE_SIGN_IDENTITY = "-"`) used for local development.

### Retrospective

More build system friction than expected from the Xcode native target + SPM local package combination: duplicate `.swiftmodule` names, Info.plist in Resources phase, C++ interop mode missing, ad-hoc signing required, and `keychain-access-groups` entitlement triggering a login keychain password prompt. All resolved. The `.scrivi` directory package shows as a plain folder in Finder during development — correct and expected until UTI is system-registered via a full app install. End-to-end flow (createProject → openProject → TextEditor) confirmed working.

---

*Closed: 2026-05-29 (T-0050, T-0051, T-0052 verified; user approved Sprint close)*
