# T-0189: End-to-end Spotlight verification (donor search, deep-link tap, donations succeed)

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/SpotlightDonor.swift`, `Scrivi/App/AppEnvironment.swift`, `Scrivi/App/ScriviApp.swift`
**Epic:** EP-017 (AC3 index-half, AC5 live-tap, AC9, AC10)
**Sprint:** SP-047
**Date Requested:** 2026-06-23
**Date Verified:** 2026-07-06

**Goal:**
Prove the in-app `CSSearchableIndex` donor path works end-to-end on a properly signed/installed build:
donated project/scene/object content is findable in Spotlight, tapping a result deep-links into Scrivi
and selects the item, donations are accepted by the OS (no `SetStoreUpdateService` rejection), and the
donation hooks introduce no project-lifecycle regression.

**Verification (user, 2026-07-06 — live run on a self-installed `/Applications` build):**
The user produced a signed copy in `/Applications` (via Archive, done themselves), ran Scrivi, and
**verified T-0189**. On the properly-signed/installed build (not the dev-run), the donation path works
end-to-end: content is indexed, a Spotlight-result tap opens/focuses the project and selects the target
scene/object, and no open/save/close regression was observed.

**What this closes:**
- **AC3 (index-half)** — donate on open / re-donate on save / delete-by-domain on close now actually
  reach the index on a signed build (the T-0182 dev-build `SetStoreUpdateService … invalidated`
  rejection does not occur on the installed copy).
- **AC5 (live-tap)** — a tapped Spotlight result opens the project and selects the item (mechanism was
  already verified via T-0196; this is the live observation through Spotlight).
- **AC9** — the original `CSInlineDonation … SetStoreUpdateService was invalidated` errors no longer
  block indexing on the signed build.
- **AC10** — no regression to project open/save/close from the donation hooks. (Encryption-at-rest does
  not exist yet, so the "no decrypted indexing" clause is a forward-guard note only.)

**Acceptance Criteria:**
- [x] Donated content is findable in Spotlight after a project is opened/edited (signed build).
- [x] Tapping a Spotlight result opens/focuses the project and selects the exact scene/object.
- [x] Donations are accepted by the OS on the installed build (AC9).
- [x] No project open/save/close regression from the Spotlight hooks (AC10).

*Verified by the user 2026-07-06 via a live run of a self-installed signed `/Applications` build.*
