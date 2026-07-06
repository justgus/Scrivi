# T-0184: Deep-Link — Spotlight Result Continuation (per-window model)

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/AppEnvironment.swift` (`handleSpotlightItem`), `Scrivi/App/ScriviApp.swift`
**Epic:** EP-017 (AC5)
**Sprint:** SP-045
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-25 (resumed after EP-018 unblocked it)
**Date Verified:** 2026-07-06 (via the T-0189 live Spotlight-tap run)

**What was done:**
The core deep link (open/focus the target window + select the scene by `scene_…` ID) was already
delivered and user-verified via EP-018 / T-0196. This task finished and hardened the Spotlight-result
*continuation* path:

1. **`handleSpotlightItem(uniqueIdentifier:relatedURL:)`** prefers the full donated deep-link URL.
   `SpotlightDonor` sets each item's `relatedUniqueIdentifier` to its `scrivi://open?project=…&item=…`
   URL (carrying the projectID), so a tapped `scene:` result opens even a **closed** project (resolved
   via its bookmark). Falls back to the uid-only path when no related URL is present.
   (`AppEnvironment.swift:344-385`)
2. **`ScriviApp`** recovers that URL from `activity.contentAttributeSet?.relatedUniqueIdentifier` on the
   `onContinueUserActivity(CSSearchableItemActionType)` handler and passes it through.
   (`ScriviApp.swift:189-199`)

No new files. macOS `ScriviApp` build clean; interop suite 26/26 (incl. `ScriviDeepLink` parse).

**Verification (user, 2026-07-06):**
Verified as part of the **T-0189** live run on a signed/installed `/Applications` build — a tapped
Spotlight result opened/focused the project and selected the target scene, including the closed-project
case (resolved via the donated `scrivi://open?project=…` deep link). The `scrivi://` URL-scheme route
was already the fully-verified path; this confirms the Spotlight continuation on top of it.

**Acceptance Criteria:**
- [x] A tapped Spotlight result opens/focuses the project and selects the scene by `scene_…` ID.
- [x] Works for both an already-open and a **closed** project (via donated deep-link URL + bookmark).
- [x] Falls back gracefully to the uid-only path when no related URL is present.

*Verified by the user 2026-07-06 via the T-0189 live Spotlight-tap run.*
