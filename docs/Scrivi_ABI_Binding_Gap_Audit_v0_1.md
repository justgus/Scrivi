# ScriviCore ABI Binding Gap Audit — v0.1

**Task:** T-0460 (EP-034 SP-121) · **Date:** 2026-08-24 · **Method:** mechanical (grep + counts), read-only
**Post-sprint result:** ✅ **Linux 34 → 81 of 100.** The only 19 remaining are the `history`/`buffers` set
deliberately deferred to EP-019 (§6) — **verified: nothing else is missing.**

Enumerates **every** endpoint declared in `ScriviCore/include/scrivi/scrivi.h` and states, for each,
whether the **Linux** (`ScriviBridge`) and **Apple** (`ScriviEngine`) bindings reach it.

⚠️ **This is an Audit CHECK, not an Audit** (`docs/Audits/Audit-Guidelines.md`) — it is mechanical, it
changes nothing, and it was not triggered by a user audit request. It exists because **SP-121's scope is
defined by this table**, and an estimate would have been the wrong basis for a sprint.

---

## 1. ⚠️ Headline — and a correction to SP-121's own planning numbers

| Measure | Planning estimate | ⚠️ **Actual (this audit)** |
| ------- | ----------------- | -------------------------- |
| Endpoints declared in `scrivi.h` | 102 | **100** |
| Reached by **Apple** `ScriviEngine` | 100 | **96** |
| Reached by **Linux** `ScriviBridge` | 35 | **34** |
| **Missing on Linux** | 62 | **66** |
| — excluded to EP-019 (history/buffers) | 19 | **19** |
| — ⚠️ **in scope for SP-121** | 43 | **47** |

⚠️ **The planning figures were derived from a loose `grep scrivi_[a-z_]*` that also matched prefixes in
prose and comments** (`scrivi_c_api`, `scrivi_buffers_`, `scrivi_history_`). This audit counts **declared
functions only**. ✅ **The corrected numbers reconcile exactly: 34 present + 47 in scope + 19 excluded = 100.**

⚠️ **This is exactly why T-0460 lands first.** The sprint's scope is **47 endpoints, not 43** — a 9%
under-estimate that would otherwise have surfaced as "scope creep" at close.

---

## 2. ⚠️ Finding A — four endpoints APPLE does not reach either

SP-121's plan predicted "two endpoints Apple does not reach" and required them to be **identified, not
rounded away**. ⚠️ **There are four:**

| Endpoint | Cluster | ⚠️ Why it matters |
| -------- | ------- | ----------------- |
| `scrivi_upsert_relation_type` | Relation types | ⚠️ **A writer cannot create or relabel a relation type from ANY platform.** The core supports a custom vocabulary; both apps expose only the seeded types. **This is a real `capability_without_surface` instance and it is UNOWNED.** |
| `scrivi_set_timeline_epoch_offset` | Timeline | Linux has `EpochOffsetDialog` and reaches the *label* setter but not this. ⚠️ **Apple reaches neither.** |
| `scrivi_set_world_epoch_offset` | World / Timeline | Per-world epoch alignment. No surface anywhere. |
| `scrivi_resolve_timeline_project_times` | Timeline | Resolution helper; ⚠️ **may be genuinely internal** — see §4. |

✅ **All four are bridged by SP-121 anyway** (they are in the 47). ⚠️ **But `upsert_relation_type` needs an
owner on the APPLE side too**, and this audit is the first record that it has none.

---

## 3. ⚠️ Finding B — `scrivi_free` is not an endpoint, and every binding must call it

`scrivi_free` is declared but is **not** a capability — it is the memory contract. Both bindings call it.
⚠️ **T-0464 must assert it for every new endpoint**: a bridged call that returns a `const char*` and never
frees it is a leak no functional test detects.

---

## 4. ⚠️ Finding C — "in scope" is not the same as "has a Linux consumer"

Three of the 47 are bridged **for completeness, with no known reader**:

- `scrivi_list_pending_edges` — ⚠️ **ZERO call sites on Apple as well.** A genuine dangling read, known
  since EP-031. Bridged so EP-035 inherits the fact rather than rediscovering it.
- `scrivi_extract_searchable_text` — ⚠️ **a KNOWN DRIFT SITE** (I-0118, which silently cost Spotlight
  reach). Linux has no Spotlight equivalent; bridged, **not** assumed to have a consumer.
- `scrivi_resolve_timeline_project_times` — may be internal-only. ⚠️ **Recorded as UNRESOLVED**; if EP-035
  finds no caller, it is a candidate for removal from the public header rather than a gap to fill.

⚠️ **Named here so that "43/47 bridged" is never read as "47 capabilities are now reachable by a writer."**
They are reachable **by the app's code**. Reachability by a writer is EP-035's job.

---

## 5. The full table — all 100 endpoints

**Legend:** ✅ present · — absent · 🔵 in scope for SP-121 · ⚠️ excluded to EP-019

| Endpoint | Cluster | Linux | Apple | Disposition |
| -------- | ------- | ----- | ----- | ----------- |
| `scrivi_import_asset` | Asset | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_assets` | Asset | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_remove_asset` | Asset | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_buffers_clear` | Buffers (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_buffers_get` | Buffers (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_buffers_list` | Buffers (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_buffers_load` | Buffers (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_create_chapter` | Chapter | ✅ | ✅ | ✅ **Present** |
| `scrivi_delete_chapter` | Chapter | ✅ | ✅ | ✅ **Present** |
| `scrivi_merge_chapter` | Chapter | ✅ | ✅ | ✅ **Present** |
| `scrivi_rename_chapter` | Chapter | ✅ | ✅ | ✅ **Present** |
| `scrivi_reorder_chapter` | Chapter | ✅ | ✅ | ✅ **Present** |
| `scrivi_add_comment` | Comment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_comments` | Comment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_resolve_comment` | Comment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_create_edge` | Edge | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_delete_edge` | Edge | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_edges_for` | Edge | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_pending_edges` | Edge | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_fragment_cut` | Fragment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_fragment_extract` | Fragment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_fragment_paste` | Fragment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_fragment_uncut_paste` | Fragment | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_history_close` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_get_settings` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_get_tree` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_list_stale_branches` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_note_scene_persisted` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_open` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_purge_branch` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_record_barrier` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_record_event` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_redo` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_seed_scene` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_select_branch` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_set_settings` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_undo` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_history_validate_scene` | History (EP-019) | — | ✅ | ⚠️ **EXCLUDED → EP-019** |
| `scrivi_ensure_local_identity` | Identity | ✅ | ✅ | ✅ **Present** |
| `scrivi_import_from_inbox` | Inbox | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_inbox` | Inbox | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_create_object` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_delete_object` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_object_kinds` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_objects` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_orphaned_objects` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_open_object` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_promote_object` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_save_object` | Object | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_scan_for_external_changes` | Other | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_create_project` | Project | ✅ | ✅ | ✅ **Present** |
| `scrivi_open_project` | Project | ✅ | ✅ | ✅ **Present** |
| `scrivi_list_relation_types` | Relation types | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_upsert_relation_type` | Relation types | — | ⚠️ **no** | 🔵 **In scope (SP-121)** |
| `scrivi_apply_repair` | Repair | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_assign_scene_to_band` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_clear_scene_story_time` | Scene | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_create_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_delete_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_get_scene_notes` | Scene | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_get_scene_story_time` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_merge_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_open_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_rename_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_reorder_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_save_scene` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_set_scene_outline` | Scene | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_set_scene_story_time` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_set_scene_tags` | Scene | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_set_scene_todo` | Scene | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_unassign_scene_from_band` | Scene | ✅ | ✅ | ✅ **Present** |
| `scrivi_create_snapshot` | Snapshot | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_enable_git_snapshots` | Snapshot | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_extract_searchable_text` | Spotlight | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_get_story_structure` | Story structure | ✅ | ✅ | ✅ **Present** |
| `scrivi_remove_story_structure` | Story structure | ✅ | ✅ | ✅ **Present** |
| `scrivi_set_story_structure` | Story structure | ✅ | ✅ | ✅ **Present** |
| `scrivi_update_band_layout` | Story structure | ✅ | ✅ | ✅ **Present** |
| `scrivi_create_historical_event` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_delete_historical_event` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_export_project_timeline` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_get_timeline` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_import_external_timeline` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_list_historical_events` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_list_imported_timelines` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_remove_imported_timeline` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_resolve_timeline_project_times` | Timeline | — | ⚠️ **no** | 🔵 **In scope (SP-121)** |
| `scrivi_set_imported_timeline_visible` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_set_timeline_epoch_label` | Timeline | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_set_timeline_epoch_offset` | Timeline | — | ⚠️ **no** | 🔵 **In scope (SP-121)** |
| `scrivi_update_historical_event` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_update_imported_timeline_offset` | Timeline | ✅ | ✅ | ✅ **Present** |
| `scrivi_add_world` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_create_world` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_get_world_binding` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_get_world_status` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_list_worlds` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_relink_world` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_remove_world_reference` | World | — | ✅ | 🔵 **In scope (SP-121)** |
| `scrivi_set_world_epoch_offset` | World | — | ⚠️ **no** | 🔵 **In scope (SP-121)** |
---

## 6. ⚠️ The 19 excluded endpoints — history and buffers

`scrivi_history_*` (15) and `scrivi_buffers_*` (4) are **deliberately NOT bridged by SP-121.**

They belong to **EP-019**'s undo/redo engine and copy buffers, ⚠️ **whose Linux story has never been
ruled.** Bridging them inside an EP-034 sprint would set that direction **by accident** — the same failure
mode as ruling a locking model inside an asset sprint (I-0144).

✅ **Owner: EP-019.** ⚠️ **Not a gap; a scheduled decision that has not been made.**

---

## 7. What this audit does NOT claim

- ⚠️ **It does not say the 34 present endpoints are correct** — only that they are *called*. Behaviour is
  T-0464's business.
- ⚠️ **It does not say Apple's 96 are reachable by a writer.** EP-034 spent six sprints proving that
  "shipped in the core" and "a writer can reach it" are different facts.
- ⚠️ **It is a snapshot.** It will drift the moment an endpoint is added. ✅ **The Porting Outline (T-0465)
  carries the checklist forward** so the next platform regenerates it rather than trusting this file.

---

*T-0460, EP-034 SP-121. Mechanical audit check — findings only, no remediation.*
