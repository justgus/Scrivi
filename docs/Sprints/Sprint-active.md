# Active Sprint

> Two sprints active: **SP-048** (EP-018, per-window model — current focus) and **SP-045** (EP-017, Spotlight — its last task T-0184 is ⏸ paused pending EP-018).

## SP-048: Per-Window Model — Foundation (Spike, ProjectSession, Registry)

**Status:** 🟡 Active
**Epic:** EP-018
**Start Date:** 2026-06-24
**End Date:** —
**Goal:** De-risk and lay the foundation for the per-window/per-project model: prove the
`WindowGroup(for:)` focus-by-value behavior (V1 spike), then extract a per-window
`ProjectSession` off the shared `AppEnvironment` and add an `OpenProjectRegistry` — all
behavior-preserving (app stays single-window until SP-049). No ScriviCore/C++ changes.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3 mechanism) | 🔵 Backlog |
| T-0192 | Extract `ProjectSession`; move per-project state + methods off `AppEnvironment` (behavior-preserving) | 🔵 Backlog |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` (projectID → session) | 🔵 Backlog |

### Acceptance Criteria

- [ ] T-0191 produces a clear yes/no on `WindowGroup(for:)` focus-by-value on macOS 26, and the R3 mechanism (native de-dup vs. registry + `NSApp` activation) is chosen and recorded before any production refactor lands.
- [ ] `ProjectSession` owns all former per-project state; `AppEnvironment` keeps only engine + identity + registry; app behavior is unchanged (still single window this sprint).
- [ ] `OpenProjectRegistry` exists and is wired into `AppEnvironment`.
- [ ] macOS build clean; `ctest` green; no regression to project open/save/close.

### Sprint Notes

- **Spike-gates-refactor:** T-0191 is throwaway and must report before T-0194 (SP-049) commits the `WindowGroup(for:)` conversion. Per the approved design doc §8 step 0.
- Reuses `ProjectBookmarkStore` (built under T-0184) for the later restore manifest — that work is not wasted.

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
