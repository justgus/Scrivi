# SP-047: Spotlight — Verification, Cross-Platform Assessment, Epic Close

**Status:** ✅ Closed (2026-07-06, user-approved)
**Epic:** EP-017
**Start Date:** 2026-07-01
**End Date:** 2026-07-06
**Goal:** Verify the surviving Spotlight path end-to-end (the in-app `CSSearchableIndex` donor — Layer 1),
confirm the original donation console errors are resolved, assess iOS/iPadOS/visionOS Spotlight, and
close EP-017. *(Layer 2 on-disk importer was descoped in SP-046 / I-0057 — `CSImportExtension` is
non-functional on macOS; the donor is now the sole Spotlight path.)*

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0189 | End-to-end verification: donated content is searchable; a Spotlight-result tap deep-links into Scrivi (opens project & selects item); donations succeed (no `SetStoreUpdateService` errors) | ✅ Verified (2026-07-06) → `Verified/Task-verified-0189.md` |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | ✅ Verified — DEFERRED (2026-07-06); follow-up T-0197 → `Verified/Task-verified-0190.md` |

### Acceptance Criteria — all met

- [x] All (non-descoped) EP-017 acceptance criteria verified by the user. AC6–AC8 (on-disk importer) are **descoped** (I-0057) and excluded. *(2026-07-06)*
- [x] Content donated by the in-app `CSSearchableIndex` donor (`SpotlightDonor.swift`) is **findable in Spotlight** after a project is opened/edited; tapping a result opens the project and selects the specific scene/object (AC5 deep-link path, live-tap). *(T-0189, verified on a signed/installed build 2026-07-06.)*
- [x] Original `CSInlineDonation … SetStoreUpdateService was invalidated` errors no longer appear (donations now succeed) — or are demonstrably benign and documented (AC9). *(Accepted on the signed build — T-0189.)*
- [x] iOS/iPadOS/visionOS Spotlight either implemented or explicitly deferred with rationale (AC11). *(Deferred — T-0190; follow-up T-0197.)*
- [x] `ctest` green; macOS build + codesign clean; no regression to project open/save/close. *(No lifecycle regression observed in the T-0189 live run.)*

### Sprint Notes

- **Sole Spotlight path (post-SP-046 / I-0057):** the in-app `CSSearchableIndex` donor
  (`Scrivi/App/SpotlightDonor.swift`). There is **no on-disk importer** — content becomes findable once
  the app has opened/edited a project, not while the app has never been opened.
- **AC5 deep-link** is delivered/verified via EP-018 / T-0196 (the `scrivi://open` URL path); T-0184
  hardened the Spotlight-result continuation to open even a closed project via the donated URL. T-0189's
  remaining piece was the **live Spotlight-tap** end-to-end observation.
- **MDImporter (Option A) is deferred**, not in scope here. Revisit only if never-opened on-disk indexing
  becomes a hard requirement.

### Retrospective

**Completed & verified:**
- T-0189 — end-to-end Spotlight verification (donor search + result tap + donations accepted) on a
  self-installed signed `/Applications` build. Closed AC3-index, AC5-tap, AC9, AC10.
- T-0190 — iOS/iPadOS/visionOS assessment; disposition **DEFERRED** with rationale. Closed AC11;
  spawned follow-up T-0197.

**Key learning:** the long-standing `SetStoreUpdateService … invalidated` donation rejection was a
**dev-build signing/environment artifact**, not a code defect — it disappears on a properly signed/
installed copy. Verifying Spotlight features must be done on an installed build, not the Xcode dev-run.

**Carry-forward:** T-0197 (enable iOS/iPadOS donation + iOS bookmark/restore consumer; visionOS when its
backend links) sits in the backlog for a future sprint.

**Epic close:** SP-047 was the final EP-017 sprint. With all non-descoped acceptance criteria met,
**EP-017 was closed alongside this sprint** (user-approved 2026-07-06).

---

*Closed 2026-07-06 (user-approved). Final EP-017 sprint — closed together with EP-017.*
