# T-0177: Define indexable record schema (fields/attributes per item type)

**Status:** ✅ Implemented - Verified
**Component:** `docs/Scrivi_Spotlight_Integration_Design_v0_1.md` §4a
**Epic:** EP-017
**Sprint:** SP-044
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §4, §4a

**Goal:**
Define the JSON record schema the indexing facade emits — one record per searchable item
(project, scene, character, location, item, rule, timeline) — as the contract T-0178 implements
and T-0179 tests.

**Resolution (recorded in design doc §4a):**
Schema `scrivi.searchableContent.v1`. Each item carries `uniqueIdentifier` (`"<kind>:<id>"`,
stable), `kind`, `title`, `displayName`, optional `containerTitle` (scenes — chapter), optional
`contentDescription` (plain text), optional `keywords`, and a `deepLink`
(`scrivi://open?project=…&item=…`). Empty optional fields are omitted, not null. Degenerate
(no scenes/objects) projects return `ok:true` with only the project record; an invalid path
returns `ok:false`.

**Note:** Field mapping was reconciled to the real on-disk schemas during T-0178 (design §4c) —
`project.json` has only `title`; world objects expose `displayName`/`notes`/`tags`.

**Acceptance Criteria:**
- [x] Indexable record schema defined for project, scene, character, location, item, rule, timeline.

*Verified by the user 2026-06-23 (SP-044 close).*
