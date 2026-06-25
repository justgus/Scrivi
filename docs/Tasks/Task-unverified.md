# Unverified Tasks

Tasks listed here are implemented and awaiting user verification before being archived.

---

## T-0184: Deep-Link — Spotlight Result Continuation (per-window model)

**Status:** 🟢 Implemented - Not Verified
**Component:** `AppEnvironment.swift` (`handleSpotlightItem`), `ScriviApp.swift`
**Priority:** Medium
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-25 (resumed after EP-018 unblocked it)
**Date Verified:** —
**Sprint Assigned:** SP-045
**Epic:** EP-017 (AC5)

### What was done

The core deep link (open/focus the target window + select the scene by `scene_…` ID) was already
delivered and **user-verified via EP-018 / T-0196**. This task finished the remaining piece — the
Spotlight-result *continuation* path — and hardened it:

1. **`handleSpotlightItem(uniqueIdentifier:relatedURL:)`** now prefers the full donated deep-link URL.
   `SpotlightDonor` sets each item's `relatedUniqueIdentifier` to its `scrivi://open?project=…&item=…`
   URL, which carries the **projectID**. So a tapped `scene:` result can now open even a **closed**
   project (resolved via its bookmark) — removing the prior limit where scene taps only worked when the
   project was already open. Falls back to the uid-only path when no related URL is present.
   (`AppEnvironment.swift:344-385`)
2. **`ScriviApp`** recovers that URL from `activity.contentAttributeSet?.relatedUniqueIdentifier` on the
   `onContinueUserActivity(CSSearchableItemActionType)` handler and passes it through.
   (`ScriviApp.swift:189-199`)

No new files. macOS `ScriviApp` build clean; interop suite 26/26 (incl. `ScriviDeepLink` parse).

### Verification needed

- **Live Spotlight tap** — search a scene in Spotlight, tap the result; confirm Scrivi opens/focuses
  the project and selects the scene. Works for both an open and a **closed** project.
- **Caveat:** depends on donations actually indexing (the T-0182 `SetStoreUpdateService` env issue on
  dev-signed sandboxed builds) and the continuation API is known unreliable on SwiftUI macOS. The
  `scrivi://` URL-scheme route is the fully-verified path. Full end-to-end Spotlight-tap verification is
  formally **T-0189 (SP-047)**.

---

*Last Updated: 2026-06-25 (T-0196 verified & archived; T-0184 implemented on the per-window model — awaiting live Spotlight-tap verification, formally T-0189).*
