# T-0179: Unit + integration tests for the indexing facade against a fixture project

**Status:** ✅ Implemented - Verified
**Component:** `ScriviCore/tests/` (Catch2)
**Epic:** EP-017
**Sprint:** SP-044
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §4b
**Related:** T-0178 (facade under test)

**Goal:**
Unit + integration tests proving the indexing facade returns correct, well-formed output against
a real on-disk project, covering all item types and the degenerate/error cases.

**Implementation:**
- **`tests/integration/SearchableContentTests.cpp`** — 7 Catch2 cases (builds a real project in a
  temp dir via the core API, then asserts the typed `ExtractSearchableTextResult`):
  1. schema string, `domainIdentifier` = projectID, projectRootPath, project record + deep link;
  2. scene record: `containerTitle` = "Chapter 1" + Markdown body stripped to plain text + deep link;
  3. all five world-object kinds, name/notes/tags → title/contentDescription/keywords;
  4. identifier stability across repeated runs;
  5. degenerate project → project record only;
  6. malformed object file skipped (non-fatal);
  7. invalid path → `ok:false`.
- **`tests/unit/MarkdownStripTests.cpp`** — 13 cases for the Markdown stripper (added under T-0178,
  registered here): headings, emphasis/strong/strike, inline code, links/images, blockquote/list
  markers, horizontal rules, fenced code, CRLF, blank-line collapsing, edge cases.

**Build/Test status:** `ctest` **224/224 green** (217 prior + 7 new integration cases; MarkdownStrip
unit cases counted within). Test files live under the `ScriviCore` folder reference in Xcode — no
`project.pbxproj` change needed.

**Acceptance Criteria:**
- [x] `ctest` green, including new facade tests against a fixture project.
- [x] Tests cover all item types, empty project, malformed object, and invalid path.

*Verified by the user 2026-06-23 (SP-044 close).*
