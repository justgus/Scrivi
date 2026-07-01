# Active Epics

**Active: EP-017 (Spotlight Search Integration)** — the sole active Epic. (Its AC5 was unblocked by
the now-closed EP-018 / Per-Window Project Model, which delivered the verified deep link via T-0196 —
see `Closed/Epic-EP-018.md`.)

## EP-017: Spotlight Search Integration

**Status:** 🟡 Active
**Goal:** Make a writer's Scrivi project content discoverable from system Spotlight. Searching for a character, location, scene, or any written text should surface the relevant project (and the specific item within it), open Scrivi to that item, and work even when Scrivi is not running. Delivers both in-app Core Spotlight donations and an on-disk `.scrivi` importer extension.
**Date Created:** 2026-06-23
**Date Activated:** 2026-06-23
**Target Close Date:** TBD
**Actual Close Date:** —
**Design Reference:** `docs/Scrivi_Spotlight_Integration_Design_v0_1.md`
**Estimated Span:** 4 Sprints (SP-044 → SP-047)
**Origin:** Expanded from T-0175, per user direction.

### Strategic Rationale

Scrivi is an authoring/worldbuilding tool whose projects accumulate large, long-lived bodies of
text and structured objects. System-level search ("find the project and scene where I wrote
about the silver mines of Khaz'tul", with the app closed) is a first-class capability for a
writing app. Surfaced while investigating benign `CSInlineDonation … SetStoreUpdateService was
invalidated` console errors. Decision (per user): implement Spotlight support properly so those
donations succeed and serve a real feature.

### Acceptance Criteria

- [ ] **AC1** — A read-only ScriviCore facade returns the indexable content of a `.scrivi`
      project as JSON (one record per searchable item: project, scenes, characters, locations,
      items, rules, timelines), with unit tests against a fixture project.
- [ ] **AC2** — The architecture boundary is preserved: all project I/O for indexing lives in
      ScriviCore; neither the app nor the importer extension parses `.scrivi` JSON in Swift.
- [ ] **AC3** — While Scrivi runs, opening a project donates `CSSearchableItem`s for its
      project/scenes/objects to Core Spotlight; saving updates them; closing/removing a project
      deletes its items by domain identifier.
- [ ] **AC4** — Manuscript body text is indexed as plain text (Markdown markup stripped).
- [x] **AC5** — A Spotlight result deep-links back into Scrivi: selecting it opens the project
      and selects the specific scene/object. *(Delivered & user-verified via EP-018 / T-0196,
      2026-06-25 — `scrivi://open` URL path. Spotlight-result continuation path remains under
      T-0184 in SP-045.)*
- [x] ~~**AC6** — A Spotlight importer app-extension indexes `.scrivi` package contents on disk,
      so project content is findable even when Scrivi is not running.~~ **Descoped (I-0057,
      2026-07-01).** `CSImportExtension` is **non-functional on macOS** (Apple DTS; unchanged
      through macOS 26 Tahoe) — the OS spawns then invalidates the extension without ever driving
      it. Adopting **Option B**: the in-app `CSSearchableIndex` donor (AC1/AC5) is the sole
      Spotlight path; content is findable once the app has opened/edited a project, not while
      never-opened. (MDImporter `.mdimporter`/`CFPlugIn` — Option A — deferred; revisit if
      never-opened on-disk indexing becomes a hard requirement.)
- [x] ~~**AC7** — The importer extension reads project content via ScriviCore (links the core or
      its read-only facade), consistent with AC2.~~ **Descoped (I-0057)** — the extension honored
      the boundary but never runs on macOS.
- [x] ~~**AC8** — Required Spotlight/extension entitlements are present and the app/extension
      build and codesign cleanly on the macOS target.~~ **Descoped (I-0057)** — the appex built &
      codesigned cleanly (verified), but the API doesn't run; the target is being removed.
- [ ] **AC9** — With Spotlight support in place, the original `CSInlineDonation …
      SetStoreUpdateService was invalidated` console errors no longer appear (donations now
      succeed) — or are demonstrably benign and documented.
- [ ] **AC10** — Encrypted projects are not indexed in decrypted form (guard noted for the
      future encryption design); no regression to existing project open/save/close behavior.
- [ ] **AC11** — iOS/iPadOS/visionOS use of the shared Core Spotlight APIs is assessed and
      either implemented or explicitly deferred with rationale.

### Design Gates (resolved in SP-044 / T-0176)

- **Boundary (design doc §3):** see T-0176 resolution in the design doc.
- **Structural discrepancy (design doc §3a):** see T-0176 resolution in the design doc.

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-044 | Spotlight — design sign-off & ScriviCore indexing facade | ✅ Closed |
| SP-045 | Spotlight — Layer 1: in-app Core Spotlight donations & deep-link | ✅ Closed |
| SP-046 | Spotlight — Layer 2: on-disk `.scrivi` importer extension (+ I-0051 per-window frame restore) | ✅ Closed (2026-07-01) — Layer 2 descoped (I-0057); windowing items shipped |
| SP-047 | Spotlight — verification, cross-platform assessment, Epic close | 🟡 Active (2026-07-01) — final EP-017 sprint |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0176 | Resolve design gates — boundary (Option A) & adapter-location reconciliation | SP-044 | ✅ Verified |
| T-0177 | Define indexable record schema (fields/attributes per item type) | SP-044 | ✅ Verified |
| T-0178 | ScriviCore `extractSearchableText(projectPath) -> JSON` read-only facade | SP-044 | ✅ Verified |
| T-0179 | Unit + integration tests for the indexing facade against a fixture project | SP-044 | ✅ Verified |
| T-0180 | Spotlight/Core Spotlight entitlements + project config | SP-045 | ✅ Verified |
| T-0181 | `ScriviEngine` Swift API to fetch indexable records (calls the facade) | SP-045 | ✅ Verified |
| T-0182 | Donate `CSSearchableItem`s on project open/save; delete-by-domain on close | SP-045 | ✅ Verified |
| T-0183 | Markdown→plain-text extraction for body indexing | SP-045 | ✅ Verified |
| T-0184 | Deep-link: result continuation opens project & selects item | SP-045 | 🟢 Implemented - Not Verified — core verified via T-0196; Spotlight-continuation hardened (opens closed projects via donated URL); live tap verify → T-0189 |
| T-0185 | New Spotlight importer app-extension target + pbxproj wiring | SP-046 | ⚪ Descoped (I-0057) |
| T-0186 | Link ScriviCore (or facade) into the extension (Option A build graph) | SP-046 | ⚪ Descoped (I-0057) |
| T-0187 | Importer emits Spotlight attributes from facade JSON | SP-046 | ⚪ Descoped (I-0057) |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox/perf pass | SP-046 | ⚪ Descoped (I-0057) |
| T-0189 | End-to-end verification (donor search, deep-link tap, donations succeed) | SP-047 | 🟡 Active |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | SP-047 | 🟡 Active |

---

> **SP-045 closed (2026-06-25):** Spotlight Layer 1 task work complete. EP-018 (closed) delivered the per-window deep link, so **AC5 is met** — the `scrivi://` URL path is user-verified (via T-0196) and T-0184 was finished/hardened so a tapped `scene:` result opens even a closed project (via the donated deep-link URL). The only thing outstanding is the **live Spotlight-tap** end-to-end check, which depends on donations indexing and is formally tracked as **T-0189 (SP-047)**. Next sprint: **SP-046** (Layer 2 importer extension).

> **Layer 2 descoped (2026-07-01 — I-0057):** the on-disk importer was built as a `CSImportExtension`,
> which **does not function on macOS** (Apple DTS; unchanged through macOS 26 Tahoe). AC6–AC8 struck;
> T-0185–T-0188 marked Descoped. EP-017 now delivers Spotlight via the **in-app `CSSearchableIndex`
> donor only** (Layer 1) — content findable once the app has opened a project, not while never-opened.
> The MDImporter (`.mdimporter`) alternative is deferred. SP-046's windowing items (I-0051/I-0055) are
> done; the appex-removal cleanup + Verified sign-off for I-0057 remain.

*Last Updated: 2026-07-01 (Layer 2 on-disk importer descoped — I-0057: `CSImportExtension` non-functional on macOS per Apple DTS; EP-017 adopts the in-app donor (Option B), AC6–AC8 struck, T-0185–T-0188 Descoped. Earlier: 2026-06-25 SP-045 closed, AC5 met.)*
