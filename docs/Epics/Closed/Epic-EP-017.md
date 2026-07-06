# Epic EP-017 — Closed

## EP-017: Spotlight Search Integration

**Status:** ✅ Closed
**Goal:** Make a writer's Scrivi project content discoverable from system Spotlight. Searching for a character, location, scene, or any written text should surface the relevant project (and the specific item within it), open Scrivi to that item, and work even when Scrivi is not running. Delivers both in-app Core Spotlight donations and an on-disk `.scrivi` importer extension.
**Date Created:** 2026-06-23
**Date Activated:** 2026-06-23
**Actual Close Date:** 2026-07-06
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

- [x] **AC1** — A read-only ScriviCore facade returns the indexable content of a `.scrivi`
      project as JSON (one record per searchable item: project, scenes, characters, locations,
      items, rules, timelines), with unit tests against a fixture project. *(Met — T-0178
      `extractSearchableText` + C ABI; T-0179 = 7 integration + 13 unit cases, `ctest` green.
      Verified 2026-06-23.)*
- [x] **AC2** — The architecture boundary is preserved: all project I/O for indexing lives in
      ScriviCore; neither the app nor the importer extension parses `.scrivi` JSON in Swift. *(Met —
      T-0178/T-0176: no UI or third-party types in public headers; no backend logic in Swift; the
      donor consumes the facade envelope only. Verified 2026-06-23.)*
- [x] **AC3** — While Scrivi runs, opening a project donates `CSSearchableItem`s for its
      project/scenes/objects to Core Spotlight; saving updates them; closing/removing a project
      deletes its items by domain identifier. *(Met — code path per T-0182; the index-half was
      confirmed live on a signed/installed build in **T-0189** (Verified 2026-07-06). The dev-build
      `SetStoreUpdateService` rejection does not occur on the installed copy.)*
- [x] **AC4** — Manuscript body text is indexed as plain text (Markdown markup stripped). *(Met —
      T-0183/T-0178: `util/MarkdownStrip` in the ScriviCore facade (shared by both layers); interop
      test #18 proves `# The Beginning … **silver mines**` → plain text end-to-end. Verified
      2026-06-23.)*
- [x] **AC5** — A Spotlight result deep-links back into Scrivi: selecting it opens the project
      and selects the specific scene/object. *(Delivered & user-verified via EP-018 / T-0196,
      2026-06-25 — `scrivi://open` URL path. Spotlight-result continuation live-tap verified via
      T-0189, 2026-07-06.)*
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
- [x] **AC9** — With Spotlight support in place, the original `CSInlineDonation …
      SetStoreUpdateService was invalidated` console errors no longer appear (donations now
      succeed) — or are demonstrably benign and documented. *(Met — T-0189: on the signed/installed
      build donations are accepted and content indexes; the dev-build rejection is environmental.)*
- [x] **AC10** — Encrypted projects are not indexed in decrypted form (guard noted for the
      future encryption design); no regression to existing project open/save/close behavior. *(Met —
      T-0189: no open/save/close regression from the donation hooks. Encryption-at-rest does not
      exist yet, so the "no decrypted indexing" clause is a forward-guard note for the future
      encryption design.)*
- [x] **AC11** — iOS/iPadOS/visionOS use of the shared Core Spotlight APIs is assessed and
      either implemented or explicitly deferred with rationale. *(Met by explicit **deferral** —
      T-0190 (Verified 2026-07-06): donor is macOS-only by guard, not API; iOS enablement gated on
      bookmark/restore parity, visionOS on its backend linking. Follow-up: T-0197 (backlog).)*

### Design Gates (resolved in SP-044 / T-0176)

- **Boundary (design doc §3):** see T-0176 resolution in the design doc.
- **Structural discrepancy (design doc §3a):** see T-0176 resolution in the design doc.

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-044 | Spotlight — design sign-off & ScriviCore indexing facade | ✅ Closed |
| SP-045 | Spotlight — Layer 1: in-app Core Spotlight donations & deep-link | ✅ Closed |
| SP-046 | Spotlight — Layer 2: on-disk `.scrivi` importer extension (+ I-0051 per-window frame restore) | ✅ Closed (2026-07-01) — Layer 2 descoped (I-0057); windowing items shipped |
| SP-047 | Spotlight — verification, cross-platform assessment, Epic close | ✅ Closed (2026-07-06) — final EP-017 sprint |

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
| T-0184 | Deep-link: result continuation opens project & selects item | SP-045 | ✅ Verified (2026-07-06, via T-0189 live tap) |
| T-0185 | New Spotlight importer app-extension target + pbxproj wiring | SP-046 | ⚪ Descoped (I-0057) |
| T-0186 | Link ScriviCore (or facade) into the extension (Option A build graph) | SP-046 | ⚪ Descoped (I-0057) |
| T-0187 | Importer emits Spotlight attributes from facade JSON | SP-046 | ⚪ Descoped (I-0057) |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox/perf pass | SP-046 | ⚪ Descoped (I-0057) |
| T-0189 | End-to-end verification (donor search, deep-link tap, donations succeed) | SP-047 | ✅ Verified (2026-07-06) |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | SP-047 | ✅ Verified — DEFERRED (2026-07-06); follow-up T-0197 |

### Scope Notes

- **Layer 2 descoped (2026-07-01 — I-0057):** the on-disk importer was built as a `CSImportExtension`,
  which **does not function on macOS** (Apple DTS; unchanged through macOS 26 Tahoe). AC6–AC8 struck;
  T-0185–T-0188 marked Descoped. EP-017 delivers Spotlight via the **in-app `CSSearchableIndex`
  donor only** (Layer 1) — content findable once the app has opened a project, not while never-opened.
- **AC5 dependency:** unblocked by the now-closed EP-018 / Per-Window Project Model, which delivered
  the verified deep link via T-0196 (see `Closed/Epic-EP-018.md`).

### Completion Summary

EP-017 ships **system-Spotlight discoverability for Scrivi project content** on macOS via the in-app
`CSSearchableIndex` donor: opening/editing a project donates project/scene/object records (with
Markdown-stripped body text) to Core Spotlight; searching surfaces them, and tapping a result opens the
project and selects the specific item via the `scrivi://open` deep link. All indexing I/O lives in the
read-only ScriviCore facade (`extractSearchableText`), preserving the architecture boundary — no `.scrivi`
JSON parsing in Swift.

**Verified end-to-end on a signed/installed build (T-0189, 2026-07-06):** donations are accepted (the
long-standing `SetStoreUpdateService … invalidated` rejection proved to be a dev-build signing artifact,
not a code defect), content indexes, and the live Spotlight-result tap deep-links correctly. No
open/save/close regression.

**Deferred / descoped:**
- **AC6–AC8 (Layer 2 on-disk importer)** — descoped (I-0057): `CSImportExtension` is non-functional on
  macOS. Never-opened on-disk indexing via an MDImporter (Option A) is deferred; revisit only if it
  becomes a hard requirement.
- **AC11 (iOS/iPadOS/visionOS)** — met by explicit deferral (T-0190): donor is macOS-gated; iOS
  enablement is gated on bookmark/restore parity, visionOS on its backend linking. Follow-up **T-0197**
  carries this forward in the backlog.

All non-descoped acceptance criteria met; Epic closed with user approval 2026-07-06.

---

*Closed 2026-07-06 (user-approved). Final sprint SP-047 closed the same day.*
