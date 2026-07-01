# Active Sprint

> **Active: SP-047** (EP-017, Spotlight — verification, cross-platform assessment, Epic close), activated 2026-07-01. SP-046 closed 2026-07-01 (see `Closed/Sprint-SP-046.md`). SP-044/SP-045/SP-048/SP-049/SP-050 also closed. SP-047 is the final EP-017 sprint.

## SP-047: Spotlight — Verification, Cross-Platform Assessment, Epic Close

**Status:** 🟡 Active
**Epic:** EP-017
**Start Date:** 2026-07-01
**End Date:** —
**Goal:** Verify the surviving Spotlight path end-to-end (the in-app `CSSearchableIndex` donor — Layer 1),
confirm the original donation console errors are resolved, assess iOS/iPadOS/visionOS Spotlight, and
close EP-017. *(Layer 2 on-disk importer was descoped in SP-046 / I-0057 — `CSImportExtension` is
non-functional on macOS; the donor is now the sole Spotlight path.)*

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0189 | End-to-end verification: donated content is searchable; a Spotlight-result tap deep-links into Scrivi (opens project & selects item); donations succeed (no `SetStoreUpdateService` errors) | 🟡 Active |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | 🟡 Active |

### Acceptance Criteria

- [ ] All (non-descoped) EP-017 acceptance criteria verified by the user. AC6–AC8 (on-disk importer) are **descoped** (I-0057) and excluded.
- [ ] Content donated by the in-app `CSSearchableIndex` donor (`SpotlightDonor.swift`) is **findable in Spotlight** after a project is opened/edited; tapping a result opens the project and selects the specific scene/object (AC5 deep-link path, live-tap).
- [ ] Original `CSInlineDonation … SetStoreUpdateService was invalidated` errors no longer appear (donations now succeed) — or are demonstrably benign and documented (AC9).
- [ ] iOS/iPadOS/visionOS Spotlight either implemented or explicitly deferred with rationale (AC11).
- [ ] `ctest` green; macOS build + codesign clean; no regression to project open/save/close.

### Sprint Notes

- **Sole Spotlight path (post-SP-046 / I-0057):** the in-app `CSSearchableIndex` donor
  (`Scrivi/App/SpotlightDonor.swift`). There is **no on-disk importer** — content becomes findable once
  the app has opened/edited a project, not while the app has never been opened. Any "app-closed on-disk
  search" or "extension codesign" checks inherited from the old SP-047 plan are dropped.
- **AC5 deep-link** is already delivered/verified via EP-018 / T-0196 (the `scrivi://open` URL path) and
  T-0184 hardened the Spotlight-result continuation to open even a closed project via the donated URL.
  T-0189's remaining piece is the **live Spotlight-tap** end-to-end observation.
- **MDImporter (Option A) is deferred**, not in scope here. Revisit only if never-opened on-disk indexing
  becomes a hard requirement.
- **Epic close:** closing EP-017 needs user approval (per Epic guidelines); this sprint prepares that.

### Retrospective

*(to be completed at sprint close)*

---

*Last Updated: 2026-07-01 (SP-047 activated as the final EP-017 sprint; SP-046 closed. Scope adjusted for the I-0057 descope — donor is the sole Spotlight path; on-disk/extension acceptance criteria dropped. T-0189/T-0190 set Active.)*
