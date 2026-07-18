#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * scrivi.h — plain C public API for ScriviCore.
 *
 * All functions return a heap-allocated UTF-8 JSON string containing either:
 *   {"ok":true,  "result":{...}}
 *   {"ok":false, "error":{"code":N,"message":"..."}}
 *
 * The caller owns the returned pointer and MUST pass it to scrivi_free()
 * when done. Passing NULL to scrivi_free() is safe (no-op).
 *
 * All input const char* parameters must be valid UTF-8 C strings.
 * Passing NULL for any input is safe — it is treated as an empty string.
 */

void        scrivi_free(const char* json);

const char* scrivi_ensure_local_identity(
    const char* displayName,
    const char* appSupportRoot);

const char* scrivi_create_project(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* title,
    const char* slug,
    const char* identityID,
    const char* personaID,
    const char* displayName);

const char* scrivi_open_project(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* identityID);

const char* scrivi_open_scene(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* sceneID);

const char* scrivi_save_scene(
    const char* projectID,
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* sceneID,
    const char* sceneMetadataPath,
    const char* sceneContentPath,
    const char* markdown,
    long long   selectionAnchor,
    long long   selectionFocus,
    double      scroll,
    const char* identityID,
    const char* personaID,
    const char* displayName);

const char* scrivi_scan_for_external_changes(
    const char* projectRootPath,
    const char* appSupportRoot,
    int         includeGitStatus);

const char* scrivi_apply_repair(
    const char* issueID,
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* actionKind,
    const char* targetPath,
    const char* identityID,
    const char* personaID,
    const char* displayName);

const char* scrivi_enable_git_snapshots(
    const char* projectRootPath,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* initialSnapshotLabel);

const char* scrivi_create_snapshot(
    const char* projectRootPath,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* label,
    const char* note);

const char* scrivi_create_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* displayName,
    const char* slug,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName);

const char* scrivi_open_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectID);

const char* scrivi_save_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectJson,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName);

const char* scrivi_delete_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectID);

const char* scrivi_import_asset(
    const char* projectRootPath,
    const char* sourcePath,
    const char* category,
    const char* title,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName);

const char* scrivi_list_assets(
    const char* projectRootPath,
    const char* category);

const char* scrivi_remove_asset(
    const char* projectRootPath,
    const char* assetID);

const char* scrivi_add_comment(
    const char* projectRootPath,
    const char* scopeKind,
    const char* targetID,
    const char* body,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName);

const char* scrivi_list_comments(
    const char* projectRootPath,
    const char* scopeKind,
    const char* targetID);

const char* scrivi_resolve_comment(
    const char* projectRootPath,
    const char* scopeKind,
    const char* targetID,
    const char* commentID,
    const char* identityID,
    const char* personaID,
    const char* resolverDisplayName);

const char* scrivi_list_inbox(
    const char* projectRootPath);

const char* scrivi_import_from_inbox(
    const char* projectRootPath,
    const char* filename,
    const char* action,
    const char* category,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName);

const char* scrivi_create_scene(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* chapterID,
    const char* afterSceneID,
    const char* identityID,
    const char* personaID,
    const char* displayName);

/* Creates a chapter. `afterChapterID` positions the new chapter immediately AFTER the
 * given chapter (its folder is born in place with a between-neighbours order key, so no
 * reorder/rename is needed — EP-027 P6). Pass "" or NULL to append at the manuscript end. */
const char* scrivi_create_chapter(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* afterChapterID);

const char* scrivi_delete_scene(
    const char* projectRootPath,
    const char* sceneID);

const char* scrivi_delete_chapter(
    const char* projectRootPath,
    const char* chapterID);

const char* scrivi_reorder_scene(
    const char* projectRootPath,
    const char* sceneID,
    const char* sourceChapterID,
    const char* targetChapterID,
    const char* afterSceneID);

const char* scrivi_reorder_chapter(
    const char* projectRootPath,
    const char* chapterID,
    const char* afterChapterID);

const char* scrivi_rename_scene(
    const char* projectRootPath,
    const char* metadataPath,
    const char* newTitle);

const char* scrivi_rename_chapter(
    const char* projectRootPath,
    const char* metadataPath,
    const char* newTitle);

/* ---- Timeline (EP-016 SP-039) ------------------------------------------ */

const char* scrivi_get_timeline(const char* projectRootPath);
const char* scrivi_set_timeline_epoch_label(const char* projectRootPath, const char* label);

const char* scrivi_set_scene_story_time(const char* projectRootPath, const char* sceneID,
                                         int64_t offsetMs, const char* source,
                                         int64_t gapMs,
                                         int64_t durationMs, const char* durationSource);
const char* scrivi_get_scene_story_time(const char* projectRootPath, const char* sceneID);
const char* scrivi_clear_scene_story_time(const char* projectRootPath, const char* sceneID);

const char* scrivi_assign_scene_to_band(const char* projectRootPath, const char* sceneID,
                                         const char* bandID);
const char* scrivi_unassign_scene_from_band(const char* projectRootPath, const char* sceneID);

const char* scrivi_get_story_structure(const char* projectRootPath);
const char* scrivi_set_story_structure(const char* projectRootPath, const char* structureID,
                                        const char* bandLayoutJSON);
const char* scrivi_update_band_layout(const char* projectRootPath, const char* bandLayoutJSON);
const char* scrivi_remove_story_structure(const char* projectRootPath);

const char* scrivi_create_historical_event(const char* projectRootPath,
                                             const char* title, int64_t offsetMs,
                                             const char* description, const char* tagsJSON,
                                             const char* identityID, const char* personaID,
                                             const char* displayName);
const char* scrivi_update_historical_event(const char* projectRootPath, const char* eventID,
                                             const char* title, int64_t offsetMs,
                                             const char* description, const char* tagsJSON);
const char* scrivi_delete_historical_event(const char* projectRootPath, const char* eventID);
const char* scrivi_list_historical_events(const char* projectRootPath);

const char* scrivi_import_external_timeline(const char* projectRootPath,
                                              const char* timelineJSON, int64_t epochOffsetMs,
                                              const char* assignedGreyShade);
const char* scrivi_update_imported_timeline_offset(const char* projectRootPath,
                                                     const char* timelineID,
                                                     int64_t epochOffsetMs);
const char* scrivi_set_imported_timeline_visible(const char* projectRootPath,
                                                   const char* timelineID, int visible);
const char* scrivi_list_imported_timelines(const char* projectRootPath);
const char* scrivi_remove_imported_timeline(const char* projectRootPath, const char* timelineID);
const char* scrivi_export_project_timeline(const char* projectRootPath);

/* ---- Searchable content (EP-017 SP-044 — Spotlight indexing facade) ----- */

/*
 * Returns the indexable content of a .scrivi project as the
 * scrivi.searchableContent.v1 envelope:
 *   {"ok":true,"result":{
 *     "schema":"scrivi.searchableContent.v1",
 *     "domainIdentifier":"project_...",      // delete-by-domain key (projectID)
 *     "projectRootPath":"/abs/path.scrivi",
 *     "items":[ {"uniqueIdentifier":"...","kind":"...", ...}, ... ]
 *   }}
 * One record per project / scene / world object. Read-only. Both the in-app
 * donor and the importer extension call this for a single source of indexing
 * truth. Caller owns the result; free with scrivi_free.
 */
const char* scrivi_extract_searchable_text(const char* projectRootPath);

/* ---- Undo/Redo history (EP-019 SP-052 — T-0202) ------------------------- */

/*
 * Persistent (per-project, on-disk) undo/redo history engine, one instance per
 * open project keyed by projectRootPath. Branching, disk persistence, capacity,
 * sessions, stale-branch detection/purge are all in place (EP-019 SP-052..055).
 * The remaining deferred functions are scrivi_history_get_tree (history panel,
 * SP-057) and scrivi_buffers_* (copy buffers, SP-056) — they land with their
 * sprints.
 *
 * Standard envelope conventions apply: each returns a heap JSON string freed
 * with scrivi_free. Offsets/cursors are scene-local UTF-8 byte offsets that
 * originate and are consumed entirely inside the engine.
 */

/* Opens (creates) a fresh history for the project and mints a session.
 * result: {sessionID, currentNodeID, canUndo, canRedo} */
const char* scrivi_history_open(const char* projectRootPath);

/* Seeds a scene's baseline (floor) text without recording an event. Call once,
 * with the scene's current on-disk text, before that scene is first edited this
 * session, so the first event diffs against real text and undo stops at the
 * baseline instead of emptying the scene. No-op if the scene already has a head.
 * result: {seeded} */
const char* scrivi_history_seed_scene(const char* projectRootPath,
                                      const char* sceneID,
                                      const char* sceneTextUtf8);

/* Records a text event by diffing newSceneTextUtf8 against the engine's cached
 * head text for sceneID. paramsJSON: {kind, cursorBefore, cursorAfter}
 *   kind is one of "typing|delete|replace|paste|cut" (default "typing").
 * result: {eventID, createdBranch, evictedCount, noOp, canUndo, canRedo} */
const char* scrivi_history_record_event(const char* projectRootPath,
                                         const char* sceneID,
                                         const char* newSceneTextUtf8,
                                         const char* paramsJSON);

/* Records a structural barrier node (section 4.5). paramsJSON: {barrierKind, note}.
 * result: {eventID, canUndo, canRedo} */
const char* scrivi_history_record_barrier(const char* projectRootPath,
                                           const char* paramsJSON);

/* Moves the current pointer back one node.
 * result: {moved, changes:[{sceneID,newText,cursorAfter}], nodeID,
 *          canUndo, canRedo, crossedSessionBoundary, boundaryTimestamp?,
 *          stoppedAtBarrier:{kind,note}?,
 *          forkAhead:{nodeID, children:[{eventID,preview,timestamp,isPrimary}]}?}
 * forkAhead is present only when the step lands on a fork (>= 2 children); it
 * drives the inline fork popover (SP-055 / §10 T2). */
const char* scrivi_history_undo(const char* projectRootPath);

/* Moves the current pointer forward to the primary child.
 * result: {moved, changes:[...], nodeID, canUndo, canRedo, forkAhead?} */
const char* scrivi_history_redo(const char* projectRootPath);

/* Re-primaries a fork: sets forkNodeID's primaryChildID to childEventID (SP-055 /
 * §5, §7). Does NOT move the current pointer — the caller walks the now-primary
 * branch via scrivi_history_redo. Fails if childEventID is not a child of the fork.
 * result: {ok, forkNodeID, childEventID, canRedo} */
const char* scrivi_history_select_branch(const char* projectRootPath,
                                         const char* forkNodeID,
                                         const char* childEventID);

/* Lists stale branches (SP-055 / §5, T-0212): every non-primary subtree whose
 * newest node (tip) is older than the project's staleBranchDays setting. A branch
 * holding the live pointer (on the root->current path) is never reported.
 * result: {staleBranchDays, branches:[{branchRootEventID, forkNodeID, preview,
 *          tipTimestamp, nodeCount}]}
 * branches is empty when staleBranchDays <= 0 or nothing is stale. */
const char* scrivi_history_list_stale_branches(const char* projectRootPath);

/* Purges a branch subtree with user confirmation (SP-055 / §5, T-0212): erases
 * branchRootEventID and all descendants and writes a ctl:purge record so the
 * branch does not resurrect on reload. Rejects (ok=false) an unknown node, the
 * root, or a node on the root->current path. Does NOT move the current pointer.
 * result: {ok, branchRootEventID, purgedCount, canUndo, canRedo} */
const char* scrivi_history_purge_branch(const char* projectRootPath,
                                        const char* branchRootEventID);

/* Head-hash validation (§6.b). Compares currentDiskTextUtf8 for sceneID against
 * the head hash persisted at last close; on mismatch (edited outside Scrivi)
 * records an externalChange barrier and re-seeds the scene's floor from disk —
 * never modifying the manuscript. Call once per loaded scene right after open.
 * result: {externalChange} (true if a mismatch/barrier occurred). */
const char* scrivi_history_validate_scene(const char* projectRootPath,
                                          const char* sceneID,
                                          const char* currentDiskTextUtf8);

/* History capacity/session settings (Trade T1). result / settingsJSON:
 * {capacityEvents, staleBranchDays, idleRolloverHours}. project.json is
 * canonical; set writes the checkpoint mirror. */
const char* scrivi_history_get_settings(const char* projectRootPath);
const char* scrivi_history_set_settings(const char* projectRootPath,
                                        const char* settingsJSON);

/* Closes the history: writes a final state.json checkpoint and discards the
 * in-memory tree. result: {closed} */
const char* scrivi_history_close(const char* projectRootPath);

#ifdef __cplusplus
}
#endif
