# Sprint SP-044: Spotlight — Design Sign-off & ScriviCore Indexing Facade

**Status:** ✅ Closed
**Epic:** EP-017
**Start Date:** 2026-06-23
**End Date:** 2026-06-23
**Goal:** Resolve the two EP-017 design gates, then build the read-only ScriviCore facade that
returns a `.scrivi` project's indexable content as JSON — the single source of indexing truth for
both Layer 1 and Layer 2.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0176 | Resolve design gates — boundary (Option A) & adapter-location reconciliation | ✅ Verified |
| T-0177 | Define indexable record schema (fields/attributes per item type) | ✅ Verified |
| T-0178 | ScriviCore `extractSearchableText(projectPath) -> JSON` read-only facade | ✅ Verified |
| T-0179 | Unit + integration tests for the indexing facade against a fixture project | ✅ Verified |

### Acceptance Criteria

- [x] Boundary decision (Option A) and adapter-location discrepancy resolved and recorded in the design doc.
- [x] Indexable record schema defined for project, scene, character, location, item, rule, timeline.
- [x] `extractSearchableText(projectPath) -> JSON` implemented in ScriviCore and exposed through the C API.
- [x] `ctest` green, including new facade tests against a fixture project. (224/224, 2026-06-23.)

### Retrospective

**Completed:**
- Both EP-017 design gates resolved and recorded in the design doc (§3b boundary = Option A on the
  pure C ABI; §3a build-graph discrepancy settled — `platforms/apple/` is a stale cache).
- `scrivi.searchableContent.v1` schema defined (§4a).
- Read-only facade `scrivi_extract_searchable_text` → `ScriviCore::extractSearchableText`
  implemented, composing existing readers; `util/MarkdownStrip` added (T-0183 core, early).
- 7 facade integration tests + 13 MarkdownStrip unit tests; `ctest` 224/224 green.

**Returned to Backlog:**
- None — all four tasks completed and verified within the sprint.

**What went well:**
- The facade composed existing schema readers cleanly; no new file I/O invented.
- Three schema discrepancies between the design's wishlist and the real on-disk schemas were
  caught during implementation and reconciled in writing (design §4c) rather than silently coded:
  `domainIdentifier` = projectID (not the per-machine identity), real field mapping, and the
  `result`-nested envelope.
- The Xcode folder-reference for `ScriviCore` means new core/test files need no per-file pbxproj
  entry — confirmed before assuming the CLAUDE.md rule applied.

**What to improve:**
- One T-0179 assertion initially baked in the real UUID provider's `project_` prefix, which the
  deterministic test mock doesn't produce — caught on first run. Prefer asserting invariants
  (domainIdentifier == projectID) over provider-specific surface in tests using mocks.
- Status drift recurred briefly: T-0176/T-0177 lagged in Task-backlog after the Sprint/Epic tables
  moved on. Keep the four tracking layers in lockstep on every status change.

**Carry-forward notes:**
- SP-045 (Layer 1) consumes the facade via `ScriviEngine` (T-0181). The facade emits
  `domainIdentifier` = projectID — donations and delete-by-domain must key on that.
- Core Spotlight (`CSSearchableIndex`) needs no dedicated entitlement with the sandbox off (current
  state); Layer 1's main project-config need is a `CFBundleURLTypes` `scrivi://` scheme for the
  deep links the facade already emits. **Verify the Core Spotlight entitlement assumption against
  the Xcode 26 / macOS 26 release notes at T-0180 start** before committing.
- The `com.caposoft.scrivi.project` UTI is already exported in the app Info.plist — a prerequisite
  for the Layer 2 importer (SP-046) is therefore already in place.

---

*Closed 2026-06-23 with user approval.*
