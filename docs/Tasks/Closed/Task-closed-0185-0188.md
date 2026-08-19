# Task-closed-0185–0188: Spotlight on-disk importer (`CSImportExtension`) — Layer 2

**Status:** ⚪ **DESCOPED — Closed by decision (2026-07-01, user-approved)**
**Component:** `ScriviSpotlightImporter.appex` (removed), `Scrivi.xcodeproj`
**Priority:** High (at the time)
**Date Requested:** 2026-06-24
**Date Closed:** 2026-07-01
**Sprint Assigned:** SP-046
**Epic:** EP-017 — Spotlight Search Integration
**Filed here:** 2026-08-19 under audit ruling **R-06** —
[`../../Audits/Audit-Rulings-20260819.md`](../../Audits/Audit-Rulings-20260819.md)

**Reason for Closure:** ⚠️ **The platform API does not work.** `CSImportExtension` — the modern on-disk
Spotlight importer API these four Tasks were built on — is **non-functional on macOS**, confirmed by
**Apple DTS**. The extension target built and codesigned cleanly and **never ran**. Closed by the
**I-0057 Option-B decision**: adopt the in-app `CSSearchableIndex` donor instead.

⚠️ **"Closed by decision, not incomplete"** (`Sprint-SP-046.md`). The work was done; the platform refused
it.

---

## The four Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0185** | New Spotlight importer app-extension target (Xcode-authored) | ⚪ Descoped |
| **T-0186** | Link ScriviCore into the extension (Option A build graph) | ⚪ Descoped |
| **T-0187** | Importer emits Spotlight attributes from facade JSON | ⚪ Descoped |
| **T-0188** | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox pass | ⚪ Descoped |

All four acceptance criteria were **struck through and marked Descoped** in the sprint record:

- ~~Importer extension target builds and codesigns; pbxproj updated in the same step.~~ **Built and signed
  cleanly — but the API doesn't run on macOS; target removed.**
- ~~Extension reads project content via ScriviCore (no Swift JSON parsing of `.scrivi`).~~ **Boundary was
  honoured, but the extension never runs.**
- ~~OS indexes `.scrivi` contents with Scrivi not running; results appear in Spotlight.~~ **Impossible via
  `CSImportExtension` on macOS (Apple DTS).** Superseded by the in-app donor.
- ~~Extension respects the app-extension sandbox; indexing completes within reasonable time.~~ **Descoped.**

---

## What replaced it

The **in-app `CSSearchableIndex` donor** (Layer 1), which was already shipping. Content is findable once
the app has opened the project — a weaker guarantee than on-disk indexing, and the best macOS allows.

⚠️ **The descope cost no core logic.** The Option-A boundary discipline meant the surviving donor already
used the same `scrivi_extract_searchable_text` facade — so the ScriviCore side needed no change at all.

---

## ⚠️ Re-open condition

**Only if the `MDImporter` route is revisited** — the older, non-deprecated importer API that
`CSImportExtension` was meant to replace. `Sprint-SP-046.md` names this explicitly as the sole path back.

**Do not re-open these IDs against `CSImportExtension`.** The API's non-functionality on macOS is
confirmed by Apple DTS, not inferred.

---

## Retrospective note carried from SP-046

> *"The API viability (does `CSImportExtension` actually run on macOS?) should have been validated in
> SP-044's design sign-off before an extension target was built. A one-line DTS/forum check would have
> saved the T-0185–T-0188 build effort."*

⚠️ **The I-0057 diagnosis is worth remembering on its own merits:** the *absence* of a crash report plus
empty process-named logs, cross-checked against a healthy binary, signature and entitlements, correctly
pointed **away** from "our bug" and toward a non-functional platform API — **confirmed by Apple DTS rather
than guessed.**

---

## ⚠️ Filing correction (audit R-06)

These four Tasks were recorded **✅ Verified** in `Task-Documentation.md` while `Task-backlog.md` recorded
them **⚪ Descoped** — a flat contradiction (audit finding **F-06**), and they were being counted in the
index's headline Verified total.

`Sprint-SP-046.md` — the record of truth — states **Descoped** seven separate ways. **The backlog was
right; the index was wrong.** Verified count corrected **376 → 372**; these four moved here.

**Related:** I-0057 (✅ Resolved - Verified 2026-07-01), T-0189/T-0190 (the in-app donor that shipped),
T-0197 (iOS/iPadOS donation, still backlog).

---

*Closed 2026-07-01. Filed to `Closed/` 2026-08-19 under audit ruling R-06.*
