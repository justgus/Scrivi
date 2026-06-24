# T-0193: Introduce `OpenProjectRegistry` in `AppEnvironment`

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/OpenProjectRegistry.swift` (new); `Scrivi/App/AppEnvironment.swift`
**Epic:** EP-018
**Sprint:** SP-048
**Date Requested:** 2026-06-24
**Date Implemented:** 2026-06-24
**Date Verified:** 2026-06-24
**Design Reference:** `Scrivi_PerWindow_Project_Model_Design_v0_1.md` §4 (registry), §3.2 (V1 spike — registry is authoritative R3 guard)
**Related:** T-0191 (spike that made the registry the authoritative R3 guard), T-0192 (ProjectSession)

**Goal:**
Add the authoritative record of which projects are open (`projectID → ProjectSession`) and use it as
the R3 non-reentrancy guard: re-opening an already-open project reuses the existing session rather
than duplicating it. (Per the T-0191 spike, native `WindowGroup(for:)` de-dup is not race-safe, so
the registry — not native behavior — is the source of truth.)

**Resolution:**
- New `@Observable @MainActor OpenProjectRegistry`: `sessions: [String: ProjectSession]`, with
  `session(for:)`, `isOpen(projectID:)`, `register(_:)`, `deregister(projectID:)` / `deregister(_:)`,
  `openProjectIDs`, `isEmpty`.
- `AppEnvironment.openProjects` registry wired:
  - `openProject(at:)` — after load, checks the registry; if the project is already open it discards
    the just-loaded session and reuses the registered one (R3); otherwise registers the new session.
  - `closeProject()` — deregisters before teardown (while projectID is still valid).
  - `handleDeepLink` — "already open?" now consults the registry (authoritative) instead of only
    `activeSession`.
- Scope note: the registry is complete and correct now; its visible "focus existing **window**"
  payoff lands in T-0194 (real per-window). The R3 "don't duplicate an open project" logic is live
  and exercised today.
- `OpenProjectRegistry.swift` added to `project.pbxproj` (fileRef C051 + 3 target build files +
  group + 3 sources phases).

**Verification:**
- macOS build (`xcodebuild -scheme ScriviApp`) — BUILD SUCCEEDED.
- User manual deep-link regression 2026-06-24 (single running instance):
  - Project already open + `open scrivi://open?project=<id>` (×2) → app focuses, **stays one
    window**, no duplicate, no reload. (R3 reuse path.)
  - Close project (⌘W) then deep link to same id → reopens in the one window.
  - Deep link to unknown projectID → friendly "Open this project in Scrivi once…" error, no crash.
  - Note: a "second window" seen earlier was a separate **process** (Xcode debugger build vs.
    installed build), which the in-process registry cannot and should not de-dup — not a regression.

**Acceptance Criteria:**
- [x] `OpenProjectRegistry` exists and is wired into `AppEnvironment`.
- [x] Re-opening an already-open project reuses the existing session (R3), verified via deep link.
- [x] macOS build clean; no regression to open/close/deep-link.

*Verified by the user 2026-06-24 via manual deep-link regression.*
