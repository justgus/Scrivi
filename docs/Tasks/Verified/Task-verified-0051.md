# T-0051: ScriviEngine Bootstrap — `AppEnvironment` Observable

**Status:** ✅ Verified
**Component:** platforms/apple/Sources/ScriviApp/AppEnvironment.swift
**Priority:** Critical
**Date Requested:** 2026-05-29
**Date Implemented:** 2026-05-29
**Date Verified:** 2026-05-29
**Sprint Assigned:** SP-015

**Implementation Details:**
- `@Observable @MainActor final class AppEnvironment` in `AppEnvironment.swift`
- Owns `ScriviEngine`; computes `appSupportRoot` from `FileManager`; stores `identityResult`, `authorshipRef`, `bootstrapError`
- `bootstrap()` is `@MainActor async`; calls `ensureLocalIdentity` using `Host.current().localizedName`; catches errors internally; prints `identity_`-prefixed ID to console
- Also owns project state: `openProjectResult`, `projectRootPath`, `projectError`; `openProject(at:)` and `createProject(at:title:slug:)` async methods
- `ScriviApp.swift` wires `.task { await env.bootstrap() }` on the root `ContentRootView` (not on `WindowGroup` — `.task` must be on a `View`, not a `Scene`, in Xcode 26)
- `ScriviError.init` made `public` in `ScriviError.swift` (was synthesized `internal`; inaccessible across module boundary)
- Identity persists across launches via `KeychainSecureStore` (T-0049); second launch returns `createdNewIdentity == false`

---

*Verified: 2026-05-29 (user approved)*
