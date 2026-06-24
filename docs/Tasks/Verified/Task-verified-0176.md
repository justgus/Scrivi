# T-0176: Resolve design gates — boundary (Option A) & adapter-location reconciliation

**Status:** ✅ Implemented - Verified
**Component:** `docs/Scrivi_Spotlight_Integration_Design_v0_1.md` §3b
**Epic:** EP-017
**Sprint:** SP-044
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §3, §3a, §3b

**Goal:**
Resolve the two EP-017 design gates blocking implementation: the architecture boundary (how the
importer extension reads `.scrivi` content without violating "no backend logic in Swift") and the
structural discrepancy over the adapter's real location/build graph.

**Resolution (recorded in design doc §3b):**
- **Build-graph reality:** `platforms/apple/` is a stale `.build` cache, not a live SPM package —
  CLAUDE.md's "no SPM package" is correct. The C++ adapter was retired in SP-026; the live boundary
  is a pure C ABI (`scrivi.h`, `scrivi_*` → `const char*` JSON, freed via `scrivi_free`), published
  as the `ScriviCore` Clang module. The app links the CMake-built `libScriviCore.a`.
- **Boundary decision — Option A, refined:** the importer extension calls a new C ABI function
  `scrivi_extract_searchable_text(projectRootPath)` in ScriviCore's `public_api` facade. Both Layer 1
  (app) and Layer 2 (extension) call the same function — single source of indexing truth. Option B
  (Swift JSON parsing) remains rejected. T-0186 replicates the app's link recipe under the extension
  sandbox; Markdown→plain-text belongs in the ScriviCore facade.

**Acceptance Criteria:**
- [x] Boundary decision (Option A) recorded in the design doc.
- [x] Adapter-location discrepancy investigated and settled against the real build graph.

*Verified by the user 2026-06-23 (SP-044 close).*
