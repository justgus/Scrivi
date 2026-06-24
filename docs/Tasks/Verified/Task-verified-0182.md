# T-0182: Donate `CSSearchableItem`s on project open/save; delete-by-domain on close

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/SpotlightDonor.swift`, `Scrivi/App/AppEnvironment.swift`
**Epic:** EP-017
**Sprint:** SP-045
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §5, §8
**Related:** T-0181 (record source), T-0189 (end-to-end Spotlight verification on a provisioned build), EP-017 AC9

**Goal:**
On project open/save, donate/update `CSSearchableItem`s via `CSSearchableIndex`; on close, delete by
`domainIdentifier`.

**Implementation:**
- `SpotlightDonor` (`#if canImport(CoreSpotlight) && os(macOS)`, no-op stub elsewhere): maps T-0181's
  `SearchableContentResult` records → `CSSearchableItem`s (title/displayName/contentDescription/
  keywords; `relatedUniqueIdentifier` = deep link; `domainIdentifier` = projectID), calls
  `indexSearchableItems` / `deleteSearchableItems(withDomainIdentifiers:)`. Best-effort, os_log
  instrumented. Content-type map: scene→.text, project→.folder, objects→.content.
- Wired into `AppEnvironment`: donate on open (`openProject`), re-donate on save (`onAppResign`),
  delete-by-domain on close (`closeProject`, via retained `spotlightDomainIdentifier`).
- Swift 6 concurrency: completion handler captures only the `Int` count, not the non-Sendable
  `[CSSearchableItem]`.
- Added `SpotlightDonor.swift` to `project.pbxproj` (all three app targets). Build clean; interop
  suite 22/22 green.

**Verification — debugger walkthrough (2026-06-23):**
Stepped the live app (user-driven, Xcode debugger + a Finder launch of the `/Applications` copy):
`openProject` → `donateSpotlight` → `extract OK: 24 records` → `donate: indexing 24 items,
domain=project_019e7537-…`. Counts correct on open (24) and save (add 1/delete 1). **The Swift
donation path is proven correct and complete.**

**Known environment limitation (not a code defect):**
Every donation is rejected by the OS with `CSIndexErrorDomain -1000 "Failed to request donation"` ←
`NSCocoaErrorDomain 4099 "connection to com.apple.SetStoreUpdateService was invalidated"`, alongside
`Unable to obtain a task name port right … (os/kern) failure (0x5)`; nothing lands in the index
(`mdfind kMDItemDomainIdentifier == 'project_…'` empty). Confirmed it is **not**: our logic, a
debugger artifact (fails on a plain Finder launch too), or a hard sandbox mach-lookup denial (no
`sandboxd` deny logged). Most likely a dev-signing/provisioning or local-daemon condition specific
to the development build on macOS 26. **Deferred to T-0189** to confirm Spotlight indexing on a
properly provisioned/installed (or App-Store-signed) build, where the donation XPC should be
accepted. This is also where EP-017 AC9 (original `CSInlineDonation` errors resolved) is judged.

Note: the `indexSearchableItems` completion handler reporting `error==nil` does **not** guarantee the
item reached the index (the underlying async `CSInlineDonation` can fail later); the log wording was
corrected from "indexed" to "submitted/accepted" to reflect this.

**Acceptance Criteria:**
- [x] Donate/update `CSSearchableItem`s on open and save (path proven in the debugger).
- [x] Delete by `domainIdentifier` on close.
- [x] No backend logic in Swift; best-effort (never blocks/crashes open/save).
- [~] Items actually queryable in Spotlight — blocked by the environment limitation above; deferred to T-0189.

*Verified by the user 2026-06-23: code path confirmed correct; donation rejection accepted as a
development-environment issue to be re-checked at T-0189.*
