# Active Sprint

> Two sprints active: **SP-050** (EP-018, per-window model — deep-link rewrite & Epic verification, current focus) and **SP-045** (EP-017, Spotlight — its last task T-0184 is ⏸ paused pending EP-018). SP-048 and SP-049 closed 2026-06-24 — see `Closed/Sprint-SP-048.md`, `Closed/Sprint-SP-049.md`.

## SP-050: Per-Window Model — Deep-Link Rewrite & EP-018 Verification

**Status:** 🟡 Active
**Epic:** EP-018
**Start Date:** 2026-06-24
**End Date:** —
**Goal:** Finish EP-018 — confirm the deep-link handler is correct on the per-window model (much of
it already landed in T-0194), fix scene-ID matching, cross-reference the open-flow doc, and run
EP-018 R1–R5 verification. Unblocks EP-017 AC5.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0196 | Deep-link handler on the per-window model + scene-`ID` fix (R5); open-flow cross-ref; EP-018 verification | 🔵 Backlog |

### Acceptance Criteria

- [ ] A deep link opens/focuses the target project's window and selects the item by its `scene_…` ID (R5).
- [ ] Reuses `ScriviDeepLink` / `ProjectBookmarkStore` / `SpotlightDonor`; no single-project switch behavior remains.
- [ ] `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` cross-references the per-window model.
- [ ] EP-018 R1–R5 verified; EP-017 AC5 unblocked.

### Sprint Notes

- **Most of the deep-link rewrite already shipped in T-0194:** `handleDeepLink` now opens/focuses the
  target window via the registry + `ProjectWindowManager`, and loads-from-bookmark for closed
  projects (`ensureOpenAndShow`). Verified working in T-0194 testing. SP-050's remaining work is the
  **scene-`ID` selection fix** (deep link must match `scene_…` ID, not title), the open-flow doc
  cross-ref, and the formal EP-018 acceptance pass.

### Retrospective

*(to be completed at sprint close)*

---

## SP-045: Spotlight — Layer 1: In-App Core Spotlight Donations & Deep-Link

**Status:** 🟡 Active
**Epic:** EP-017
**Start Date:** 2026-06-23
**End Date:** —
**Goal:** While Scrivi runs, donate project/scene/object content to Core Spotlight on open/save,
delete it by domain on close, and make a Spotlight result open Scrivi to the specific item.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0180 | Spotlight/Core Spotlight entitlements + project config | ✅ Verified |
| T-0181 | `ScriviEngine` Swift API to fetch indexable records (calls the facade) | ✅ Verified |
| T-0182 | Donate `CSSearchableItem`s on project open/save; delete-by-domain on close | ✅ Verified |
| T-0183 | Markdown→plain-text extraction for body indexing | ✅ Verified |
| T-0184 | Deep-link: result continuation opens project & selects item | ⏸ Paused — depends on EP-018 |

### Acceptance Criteria

- [ ] Opening a project donates searchable items; saving updates them; closing deletes by domain.
- [ ] Body text indexed as plain text (Markdown stripped).
- [ ] Selecting a Spotlight result opens the project and selects the scene/object. *(⏸ blocked — T-0184 paused pending EP-018 per-window/per-project model; current single-shared-AppEnvironment cannot support correct per-project window behavior. See `docs/Scrivi_PerWindow_Project_Model_Design_v0_1.md`.)*
- [ ] Entitlements present; app builds and codesigns cleanly on macOS.

### Sprint Notes

- The indexing facade (`scrivi_extract_searchable_text`, SP-044) is the source of truth — Layer 1
  calls it via `ScriviEngine` (T-0181) and donates from the returned records.
- `domainIdentifier` = projectID; donations and delete-by-domain MUST key on that, not the
  per-machine identity.
- **T-0183 note:** the Markdown→plain-text core (`util/MarkdownStrip`) already landed in ScriviCore
  under T-0178 (the facade strips scene bodies). T-0183 here is the Swift-side confirmation /
  any remaining wiring, not a re-implementation.
- **T-0180 resolved:** Core Spotlight needs no dedicated entitlement, but the **App Sandbox
  capability must be declared** or `corespotlightd` can't resolve the bundle (donations fail).
  Sandbox was enabled (full, user-approved) + `scrivi://` URL scheme added. Security-scoped
  bookmarks deferred to T-0184 (no reopen-from-memory feature exists yet). Whether the original
  `CSInlineDonation` errors are gone is checked at AC9 / T-0189 once donations (T-0182) exist.

### Retrospective

*(to be completed at sprint close)*

---

*Last Updated: 2026-06-24 (T-0184 ⏸ PAUSED — verification surfaced that the app's single shared `AppEnvironment` is single-project across all windows, conflicting with the per-project/per-window + restore requirements. A correct deep-link handler depends on the per-window model now tracked under EP-018 (SP-048→SP-050). T-0184's reusable pieces (ScriviDeepLink, ProjectBookmarkStore, SpotlightDonor) are carried forward; its single-project control flow is replaced. T-0183 ✅ Verified.)*
