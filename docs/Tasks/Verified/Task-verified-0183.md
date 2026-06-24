# T-0183: Markdown→plain-text extraction for body indexing

**Status:** ✅ Implemented - Verified
**Component:** `ScriviCore/src/util/MarkdownStrip.{hpp,cpp}` (core); consumed by the facade and Swift donor
**Epic:** EP-017
**Sprint:** SP-045
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23 (core landed under T-0178)
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §3b, §4b ("Markdown→plain-text belongs in the ScriviCore facade")
**Related:** T-0178 (MarkdownStrip + facade), T-0181 (Swift records), T-0182 (donation)

**Goal:**
Scene body Markdown is indexed as plain text (markup stripped) for `contentDescription`, with the
stripping shared by both Spotlight layers.

**Resolution — no new code required (satisfied by existing, verified work):**
Per design §4b, Markdown→plain-text **belongs in the ScriviCore facade**, not Swift, so both Layer 1
(in-app donation) and Layer 2 (importer) index identical text. That decision was implemented early
under T-0178:
- `util/MarkdownStrip.{hpp,cpp}` (ScriviCore) — strips ATX headings, emphasis/strong/strike, inline
  & fenced code, links/images, blockquote/list markers, horizontal rules; normalises blank lines;
  handles CRLF. 13 Catch2 unit cases.
- `ScriviCore::extractSearchableText` applies it to every scene body, so `contentDescription` in the
  `scrivi.searchableContent.v1` envelope is **already plain text**.
- Swift does **no** Markdown handling for indexing: `SpotlightDonor` assigns the facade's
  `record.contentDescription` directly to `CSSearchableItemAttributeSet.contentDescription`. (Verified
  by grep — the only Swift `markdown` references are the unrelated editing/save path.)

**Verification:**
- `MarkdownStripTests` (13 cases) — `ctest` green.
- Interop test #18 `extractSearchableTextDecodesRecords` asserts end-to-end:
  `# The Beginning\n\nThe **silver mines** of *Khaz'tul*.` → `contentDescription ==
  "The Beginning\nThe silver mines of Khaz'tul."` — passing (`xcodebuild test` 22/22).

**Acceptance Criteria:**
- [x] Scene body Markdown is stripped to plain text for `contentDescription`.
- [x] Stripping lives in the ScriviCore facade (shared by both layers); no Markdown logic in Swift.
- [x] Covered by passing unit + interop tests.

*Verified by the user 2026-06-23: the Markdown-strip work shipped under T-0178 and is exercised by
the facade and donor; no further implementation needed for T-0183.*
