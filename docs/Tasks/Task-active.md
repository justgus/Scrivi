# Active Tasks

Active Sprint: **SP-047** (EP-017 — Spotlight verification, cross-platform assessment, Epic close), activated 2026-07-01.

| ID | Title | Epic (Sprint) | Status |
| -- | ----- | ------------- | ------ |
| T-0189 | End-to-end verification: donated content is searchable; a Spotlight-result tap deep-links into Scrivi (opens project & selects item); donations succeed (no `SetStoreUpdateService` errors) | EP-017 (SP-047) | 🟡 Active |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | EP-017 (SP-047) | 🟡 Active |

> **Scope note (post-SP-046 / I-0057):** the Spotlight on-disk importer was descoped
> (`CSImportExtension` is non-functional on macOS). The **in-app `CSSearchableIndex` donor**
> (`Scrivi/App/SpotlightDonor.swift`) is now the sole Spotlight path, so T-0189 verifies donor-indexed
> search + deep-link tap (not app-closed on-disk indexing). Full reference notes for both tasks remain in
> `Task-backlog.md` (EP-017 section).

---

*Last Updated: 2026-07-01 (SP-047 activated; T-0189/T-0190 set Active. T-0189 rescoped from "app-closed search" to donor-indexed search per the I-0057 descope.)*
