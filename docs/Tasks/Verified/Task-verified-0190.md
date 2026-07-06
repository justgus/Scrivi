# T-0190: iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification

**Status:** ✅ Assessment complete - Verified (disposition: **DEFERRED**, user-approved 2026-07-06)
**Component:** `Scrivi/App/SpotlightDonor.swift`, `Scrivi/App/AppEnvironment.swift` (assessment only — no code change)
**Epic:** EP-017 (AC11)
**Sprint:** SP-047
**Date Requested:** 2026-06-23
**Date Assessed:** 2026-07-01
**Date Verified:** 2026-07-06

**Goal:**
Assess iOS/iPadOS/visionOS use of the shared Core Spotlight APIs and either implement it or explicitly
defer with rationale (AC11).

**Assessment (from code):**
- The Spotlight donor is **macOS-only by guard, not by API limitation** — `SpotlightDonor` is wrapped in
  `#if canImport(CoreSpotlight) && os(macOS)`; iOS/iPadOS/visionOS compile the no-op stub
  (`SpotlightDonor.swift:4`, `:99-100`). Its `AppEnvironment` invocation is likewise `#if os(macOS)`-gated.
- `CSSearchableIndex` / `CSSearchableItem` are cross-platform, and the record source (facade via
  `ScriviEngine.searchableContent`) already links/runs on iOS (I-0053). Enabling iOS/iPadOS is
  mechanically small (broaden two guards + entitlements).

**Disposition: DEFERRED (user-approved 2026-07-06).**
Rationale:
1. Verify the macOS path first (T-0189, now ✅) before multiplying platform surface — port a known-good path.
2. iOS reopen/restore is incomplete: `ProjectBookmarkStore` is macOS-only, so a tapped iOS Spotlight
   result couldn't yet resolve a persisted project across launches (deep-link consumer side needs iOS work).
3. visionOS is still on the `ScriviEngine` stub (backend not linked) — it cannot produce searchable content
   yet; defer with the rest of visionOS enablement.
4. Cost of deferring is low and reversible (a two-guard change + entitlements when we do it).

**AC11 outcome:** satisfied by **explicit deferral with rationale** (the AC's own "assessed and either
implemented or explicitly deferred" wording). Future enablement tracked as **T-0197** (backlog), gated on
(a) T-0189 done ✅ and (b) iOS bookmark/restore parity.

**Acceptance Criteria (AC11):**
- [x] iOS/iPadOS/visionOS Core Spotlight use assessed.
- [x] Explicitly deferred with documented rationale; follow-up tracked (T-0197).

*Verified by the user 2026-07-06 — defer decision confirmed.*
