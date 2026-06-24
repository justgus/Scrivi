# T-0180: Spotlight/Core Spotlight entitlements + project config

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi.entitlements`, `Scrivi.xcodeproj/project.pbxproj`, `Scrivi/App/Info.plist`
**Epic:** EP-017
**Sprint:** SP-045
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §6
**Related:** T-0182 (donations that actually exercise Core Spotlight), T-0184 (deep-link reopen — security-scoped bookmarks), T-0189 (EP-017 AC9 — original console errors)

**Goal:**
Add the macOS project configuration Core Spotlight requires, plus the `scrivi://` deep-link URL
scheme, without regressing project open/save/close.

**Key finding (verified against current docs/radar, not assumed):**
Core Spotlight needs no dedicated entitlement, but the **App Sandbox capability must be declared
in the project** — without it `corespotlightd` cannot resolve the bundle and donations fail
("Couldn't communicate with a helper application", -1003). The project previously had **no
`SystemCapabilities` entry**, a strong candidate for the EP-017 `CSInlineDonation …
SetStoreUpdateService was invalidated` errors.

**Implementation:**
- `Scrivi.entitlements`: `com.apple.security.app-sandbox` → **true** (kept
  `files.user-selected.read-write` so the `NSOpenPanel` open path still works).
- `project.pbxproj` `TargetAttributes` (macOS `C_TARGET`): added
  `SystemCapabilities → com.apple.Sandbox = { enabled = 1 }`.
- `Info.plist`: added `CFBundleURLTypes` for the `scrivi://` scheme (deep links the facade emits).

**Scope decisions (user-approved):**
- Full App Sandbox (not declare-disabled).
- Security-scoped bookmarks **deferred to T-0184** — Scrivi has no reopen-from-memory feature today
  (every open is a fresh `NSOpenPanel`), so nothing needs bookmarks yet; T-0184's deep-link reopen
  is the first flow that opens a project the user didn't pick this session.

**Build/Test status:** macOS build SUCCEEDED, codesign clean. Signed binary confirmed to carry
`app-sandbox=true` + `files.user-selected.read-write`; `scrivi` URL scheme present in built
Info.plist.

**Acceptance Criteria:**
- [x] Spotlight-required project config present (App Sandbox capability declared).
- [x] `scrivi://` deep-link URL scheme registered in Info.plist.
- [x] App builds and codesigns cleanly on macOS.
- [x] No regression to project open/save/close under the sandbox (AC10).

**Verification (user, 2026-06-23):** Ran a full open → write → save → close cycle under the
sandbox; **no sandbox `deny` / error messages in Console.** Confirms ScriviCore writes through the
sandbox cleanly with no regression to the core flow.

**Note — still open (not this task):** This verifies the sandbox did not break file I/O. It does
**not** confirm the original `CSInlineDonation` console errors are resolved — that requires live
Core Spotlight donations (T-0182) and is formally checked at EP-017 AC9 / T-0189.

*Verified by the user 2026-06-23.*
