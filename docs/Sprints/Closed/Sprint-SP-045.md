# Sprint SP-045: Spotlight — Layer 1: In-App Core Spotlight Donations & Deep-Link

**Status:** ✅ Closed
**Epic:** EP-017
**Start Date:** 2026-06-23
**End Date:** 2026-06-25
**Goal:** While Scrivi runs, donate project/scene/object content to Core Spotlight on open/save,
delete it by domain on close, and make a Spotlight result open Scrivi to the specific item.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0180 | Spotlight/Core Spotlight entitlements + project config | ✅ Verified |
| T-0181 | `ScriviEngine` Swift API to fetch indexable records (calls the facade) | ✅ Verified |
| T-0182 | Donate `CSSearchableItem`s on project open/save; delete-by-domain on close | ✅ Verified |
| T-0183 | Markdown→plain-text extraction for body indexing | ✅ Verified |
| T-0184 | Deep-link: result continuation opens project & selects item | 🟢 Implemented - Not Verified (core verified via T-0196; live Spotlight-tap → T-0189) |

### Acceptance Criteria

- [x] Opening a project donates searchable items; saving updates them; closing deletes by domain. *(T-0182 — `SpotlightDonor.donate` on open/save, `deleteProject(domainIdentifier:)` on close, keyed on projectID.)*
- [x] Body text indexed as plain text (Markdown stripped). *(T-0183 — facade strips scene bodies via `util/MarkdownStrip`.)*
- [x] Selecting a Spotlight result opens the project and selects the scene/object. *(T-0184 on the per-window model; core deep link user-verified via EP-018 / T-0196. Live Spotlight-tap continuation deferred to T-0189 / SP-047.)*
- [x] Entitlements present; app builds and codesigns cleanly on macOS. *(T-0180 — App Sandbox capability declared + `scrivi://` scheme; macOS build/codesign clean.)*

### Outcome

Layer 1 of Spotlight is in place: while Scrivi runs, opening a project donates its
project/scene/object records to Core Spotlight (re-donated on save), and closing deletes them by
domain identifier (the projectID). Body text is indexed as Markdown-stripped plain text. Tapping a
result opens Scrivi to the specific item.

- **Indexing source of truth:** the SP-044 facade (`scrivi_extract_searchable_text`) via
  `ScriviEngine` (T-0181). No `.scrivi` JSON is parsed in Swift.
- **Deep link:** delivered on the EP-018 per-window model. The `scrivi://open?project=…&item=…`
  URL-scheme route is fully user-verified (via EP-018 / T-0196 — a scene link lands exactly on its
  target). The Spotlight-result *continuation* path (T-0184) was hardened to prefer the donated full
  deep-link URL (carries the projectID), so a tapped `scene:` result can open even a **closed**
  project.

### Carried forward

- **T-0184 live Spotlight-tap verification → T-0189 (SP-047).** It depends on donations actually
  *indexing*, which is gated by the T-0182 `SetStoreUpdateService -4099` env issue on dev-signed
  sandboxed builds, and the continuation API is unreliable on SwiftUI macOS. The implementation is
  complete and the URL-scheme route is the fully-verified path.
- **AC9 (original `CSInlineDonation` errors gone / documented benign)** is checked at T-0189 once
  donations reliably index.
- **Next:** SP-046 (Layer 2 — on-disk `.scrivi` importer extension).

### Retrospective

**What went well:**
- The facade-as-single-source-of-truth boundary held cleanly — Layer 1 is a thin donor over
  `ScriviEngine`, with zero `.scrivi` parsing in Swift.
- The deep link came out stronger than scoped: EP-018 (which this sprint surfaced) gave a correct
  per-window foundation, and T-0184's continuation now opens closed projects via the donated URL —
  better than the original "project must already be open" design.

**What was harder than expected:**
- Donations *accepting* (completion handler `error == nil`) but not necessarily *indexing* on
  dev-signed sandboxed builds (`SetStoreUpdateService -4099`) blurs the line between "implemented"
  and "verifiable." Live Spotlight-tap verification had to be deferred to T-0189 rather than done
  in-sprint.
- T-0184 forced an architectural detour: verifying it surfaced the single-project window model,
  spawning EP-018 (SP-048→050). T-0184 paused there and resumed only after EP-018 closed.

---

*Closed 2026-06-25 with user approval. Task work complete; T-0184 live Spotlight-tap verification carried to T-0189 (SP-047).*
