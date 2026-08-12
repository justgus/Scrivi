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

/* ---------------------------------------------------------------------------
 * Relationship graph (EP-031 SP-096) — scrivi.relationships.v1
 *
 * ONE canonical edge per relationship; the inverse direction is a read-time
 * label projection, never a second record. Creating the same relationship from
 * either endpoint is rejected as a duplicate (error.detail == "duplicateEdge").
 *
 * Endpoints are BARE IDs — no kind parameter. Kind is resolved through
 * objects/index.json, which is what makes item→artifact promotion cost zero
 * edge rewrites. Endpoints may be objects OR scenes.
 * ------------------------------------------------------------------------- */

const char* scrivi_create_edge(
    const char* projectRootPath,
    const char* fromID,
    const char* toID,
    const char* relationTypeCode,
    const char* note);

const char* scrivi_delete_edge(
    const char* projectRootPath,
    const char* edgeID);

/* Every edge touching endpointID in either direction, each carrying the label
 * that reads correctly from that endpoint. */
const char* scrivi_list_edges_for(
    const char* projectRootPath,
    const char* endpointID);

/* ---------------------------------------------------------------------------
 * Worlds (EP-031 SP-097) — scrivi.world.v1 / scrivi.world-binding.v1
 *
 * A world is a SEPARATE `.scrivworld` package referenced by the project; only
 * binding.json is project-local. `worldID` is the identity — every stored path
 * is a hint, verified against world.json before it is trusted.
 *
 * An unreachable world is a STATUS, never an error: `status` is one of
 * "available" | "missing" | "offline" | "unmounted" | "unavailable".
 * "missing" is reported only when positively established; otherwise the honest
 * fallback is "unavailable" (a wrong "missing" invites destructive remedies).
 * ------------------------------------------------------------------------- */

const char* scrivi_create_world(const char* projectRootPath,
                                const char* packagePath,
                                const char* displayName,
                                const char* epochLabel);

/* Binds an existing package. Read-only toward the world — takes no lock. */
const char* scrivi_add_world(const char* projectRootPath,
                             const char* packagePath);

const char* scrivi_list_worlds(const char* projectRootPath);
const char* scrivi_get_world_status(const char* projectRootPath, const char* worldID);
const char* scrivi_get_world_binding(const char* projectRootPath, const char* worldID);

/* Re-points a moved world. VERIFIES worldID before accepting the new path. */
const char* scrivi_relink_world(const char* projectRootPath,
                                const char* worldID,
                                const char* newPackagePath);

/* Removes only THIS project's reference; the world package is never touched. */
const char* scrivi_remove_world_reference(const char* projectRootPath,
                                          const char* worldID);

/* Epoch chain (Doc 1 §7.0). Layer 2→3: world epoch → project timeline. */
const char* scrivi_set_world_epoch_offset(const char* projectRootPath,
                                          const char* worldID,
                                          long long epochOffsetMs);

/* Layer 1→2: a historical timeline's offset against its WORLD's epoch. */
const char* scrivi_set_timeline_epoch_offset(const char* projectRootPath,
                                             const char* worldID,
                                             const char* timelineID,
                                             long long epochOffsetMs);

/* Resolves timeline→world→project; add an event's own offsetMs to the result. */
const char* scrivi_resolve_timeline_project_times(const char* projectRootPath,
                                                  const char* worldID,
                                                  const char* timelineID);

/* RelationType vocabulary — scrivi.relation-types.v1 */
const char* scrivi_list_relation_types(const char* projectRootPath);

const char* scrivi_upsert_relation_type(
    const char* projectRootPath,
    const char* relationTypeJson);

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

/* Creates a scene. `afterSceneID` inserts immediately AFTER the given scene ("" / NULL =
 * append to the end of the chapter). `beforeSceneID` inserts immediately BEFORE the given
 * scene and takes precedence when set ("" / NULL = not a before-insert) — this is the
 * Cmd-Enter-at-scene-start case, including a chapter's first scene. */
const char* scrivi_create_scene(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* chapterID,
    const char* afterSceneID,
    const char* beforeSceneID,
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

/* ---- Merge (EP-028 SP-074) --------------------------------------------- */

/* Join a scene into the scene immediately BEFORE it in the same chapter. The predecessor
 * (the survivor) keeps its own order-key files and gains the merged scene's body appended;
 * the merged scene's files are removed. Fails if `sceneID` is first in its chapter (no
 * predecessor — that is the whole-chapter merge, scrivi_merge_chapter). Result JSON carries
 * survivorSceneID, mergedSceneID, chapterID, survivorMetadataPath, survivorContentPath,
 * chapterMetadataPath, merged. */
const char* scrivi_merge_scene(
    const char* projectRootPath,
    const char* sceneID);

/* Merge a whole chapter into the chapter immediately BEFORE it in manuscript order. Every
 * scene file in `chapterID` is RELOCATED into the predecessor chapter's folder (appended
 * after its last scene, order-key files renamed, sidecar slug/contentPath rewritten), then
 * the emptied chapter folder + its manuscript.meta.json entry are removed. Atomic fix for
 * I-0083 (no scene is deleted). Fails if `chapterID` is first in the manuscript (no
 * predecessor). Result JSON carries survivorChapterID, mergedChapterID,
 * survivorChapterMetadataPath, scenesRelocated, merged. */
const char* scrivi_merge_chapter(
    const char* projectRootPath,
    const char* chapterID);

/* ---- Timeline (EP-016 SP-039) ------------------------------------------ */

const char* scrivi_get_timeline(const char* projectRootPath);
const char* scrivi_set_timeline_epoch_label(const char* projectRootPath, const char* label);

const char* scrivi_set_scene_story_time(const char* projectRootPath, const char* sceneID,
                                         int64_t offsetMs, const char* source,
                                         int64_t gapMs,
                                         int64_t durationMs, const char* durationSource);
const char* scrivi_get_scene_story_time(const char* projectRootPath, const char* sceneID);
const char* scrivi_clear_scene_story_time(const char* projectRootPath, const char* sceneID);

/* Scene writing-tool card content (EP-030 SP-091) — tags / outline / todo.
   Setters replace their field wholesale; the card UI always sends the complete list.
   `tagsJson` is a JSON string array: ["battle","ada-pov"]
   `todoJson` is a JSON array of objects: [{"text":"…","done":false}] */
const char* scrivi_set_scene_tags(const char* projectRootPath, const char* sceneID,
                                   const char* tagsJson);
const char* scrivi_set_scene_outline(const char* projectRootPath, const char* sceneID,
                                      const char* outline);
const char* scrivi_set_scene_todo(const char* projectRootPath, const char* sceneID,
                                   const char* todoJson);
/* Returns all three in one envelope so a card stack costs one call, not three. */
const char* scrivi_get_scene_notes(const char* projectRootPath, const char* sceneID);

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
 * scrivi_history_get_tree landed in EP-030 SP-092 (T-0394) for the history CARD
 * (the panel moved to the Scene Inspector); scrivi_buffers_* (copy buffers)
 * landed in SP-056 — see the Copy buffers section below.
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
 * head text for sceneID. paramsJSON: {kind, cursorBefore, cursorAfter, bufferID?}
 *   kind is one of "typing|delete|replace|paste|cut" (default "typing").
 *   bufferID (optional, "1".."9") tags a cut-into-buffer event with the copy-buffer
 *   slot it fed (EP-019 SP-056, Trade T3); omitted/empty for ordinary events.
 *   It is metadata only — preserved across reload, not consumed by undo/redo.
 * result: {eventID, createdBranch, evictedCount, noOp, canUndo, canRedo} */
const char* scrivi_history_record_event(const char* projectRootPath,
                                         const char* sceneID,
                                         const char* newSceneTextUtf8,
                                         const char* paramsJSON);

/* Records a structural barrier node (section 4.5). paramsJSON: {barrierKind, note}.
 * OPTIONAL `structuralPayload` (an object): when present, records a REVERSIBLE
 * structural node instead of a hard barrier (T-0356 / AC6) — undo/redo step ACROSS it
 * and return the payload as `structuralInverse` for the app to replay (the inverse
 * fragment op + reload). The engine treats the payload as opaque. Absent ⇒ classic
 * barrier (undo stops with a notice). result: {eventID, canUndo, canRedo} */
const char* scrivi_history_record_barrier(const char* projectRootPath,
                                           const char* paramsJSON);

/* Moves the current pointer back one node.
 * result: {moved, changes:[{sceneID,newText,cursorAfter}], nodeID,
 *          canUndo, canRedo, crossedSessionBoundary, boundaryTimestamp?,
 *          stoppedAtBarrier:{kind,note}?,
 *          structuralInverse:{direction:"undo"|"redo", payload:{…}}?,
 *          forkAhead:{nodeID, children:[{eventID,preview,timestamp,isPrimary}]}?}
 * structuralInverse is present only when the step crossed a reversible structural node
 * (T-0356 / AC6): `moved` is true, there are no `changes`, and the app runs the inverse
 * fragment op from `payload` then reloads the manuscript. forkAhead is present only when
 * the step lands on a fork (>= 2 children); it drives the inline fork popover
 * (SP-055 / §10 T2). */
const char* scrivi_history_undo(const char* projectRootPath);

/* Moves the current pointer forward to the primary child.
 * result: {moved, changes:[...], nodeID, canUndo, canRedo,
 *          structuralInverse:{direction:"undo"|"redo", payload:{…}}?, forkAhead?}
 * structuralInverse (T-0356) carries direction "redo" here — the app re-runs the
 * forward structural op. */
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
/* A windowed read-only projection of the node graph for the history card
 * (EP-030 SP-092 / T-0394; design §10 Trade T2 option B).
 *
 * paramsJSON: {"aroundNodeID"?: string, "maxNodes"?: int} — both optional. The
 * window walks outward from aroundNodeID (default: the current node): ancestors
 * first, then descendants breadth-first, capped at maxNodes (default 200). An
 * unknown aroundNodeID falls back to the current node rather than erroring.
 *
 * Returns {rootID, currentNodeID, totalNodeCount, truncated, nodes:[{eventID,
 * parentID, primaryChildID, childIDs[], kind, sceneID, preview, timestamp,
 * sessionID, bufferID, barrierKind, barrierNote, onPrimarySpine, isCurrent,
 * changeOffsetUtf8, changeLength, removedLength, whitespaceKind}]}.
 *
 * whitespaceKind is set only when the event's change is ENTIRELY whitespace, as
 * "<kind>:<count>" — "newline:2" | "tab:1" | "space:3" — so a UI can NAME such an
 * event instead of rendering it as empty. `preview` rewrites \n/\r/\t to spaces, so
 * it cannot carry this: a newline-only event is indistinguishable from an empty one
 * once trimmed (T-0397). OMITTED (empty) for every event containing real text.
 *
 * changeOffsetUtf8/changeLength/removedLength locate the event's edit in its
 * scene: the scene-local UTF-8 byte offset, the INSERTED byte count, and the
 * REMOVED byte count. A pure deletion therefore has changeLength == 0 and
 * removedLength > 0 — consumers must use removedLength to give it a real span,
 * or a deletion collapses to a single offset and ties with the neighbouring
 * insertion (I-0106). All three are 0 for the root and for barriers.
 *
 * NOTE: childIDs is OMITTED for a leaf (empty arrays are not emitted) — decoders
 * must tolerate its absence (see I-0094). */
const char* scrivi_history_get_tree(const char* projectRootPath,
                                    const char* paramsJSON);

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

/* Record the exact bytes just written to a scene file (I-0104). Call this from
 * the manuscript save path immediately AFTER the scene file is persisted,
 * passing the same text that was written. History then carries a disk-derived
 * head hash, so the next open compares disk-to-disk in
 * scrivi_history_validate_scene and only a genuine third-party edit raises an
 * externalChange barrier. Omitting this call does not corrupt anything — the
 * scene simply keeps its previous baseline. result: {recorded:true}. */
const char* scrivi_history_note_scene_persisted(const char* projectRootPath,
                                                const char* sceneID,
                                                const char* diskTextUtf8);

/* History capacity/session settings (Trade T1). result / settingsJSON:
 * {capacityEvents, staleBranchDays, idleRolloverHours}. project.json is
 * canonical; set writes the checkpoint mirror. */
const char* scrivi_history_get_settings(const char* projectRootPath);
const char* scrivi_history_set_settings(const char* projectRootPath,
                                        const char* settingsJSON);

/* Closes the history: writes a final state.json checkpoint and discards the
 * in-memory tree. result: {closed} */
const char* scrivi_history_close(const char* projectRootPath);

/* ---- Structured Cut/Copy/Paste (EP-029 SP-086 — T-0351) ----------------- */

/*
 * extract-fragment — turn a manuscript range that may cross scene/chapter
 * boundaries into a scrivi.fragment.v1 structured fragment. READ-ONLY: never
 * mutates the project (⌘C is non-destructive; cut and paste-splice are separate
 * endpoints, SP-088 / SP-087).
 *
 * `spansJson` is { "spans": [ { "sceneID", "startByte", "endByte" }, ... ] } — one
 * span per scene the selection touches, IN MANUSCRIPT READING ORDER. startByte/
 * endByte are scene-local UTF-8 byte offsets into the scene body (same convention
 * as the history engine); endByte is exclusive and >= startByte. The editor derives
 * these from its storage-range -> sceneID map; dividers/headings (UI-only) are never
 * part of a span.
 *
 * result: {
 *   schema: "scrivi.fragment.v1",
 *   pieces: [ { opensWith: "none"|"scene"|"chapter",
 *               chapterTitle?,           // present only when opensWith == "chapter"
 *               text,                     // this scene's selected body
 *               partial?: "head"|"tail"   // omitted (null) for whole scenes
 *             }, ... ],
 *   plainText                             // pieces joined by a blank-line seam (for the
 *                                         // system pasteboard; T2=A)
 * }
 *
 * invalidArgument if spans is empty, out of manuscript order, has startByte>endByte,
 * references an unknown scene, or a span exceeds its scene body. Standard envelope
 * conventions apply (heap JSON, free with scrivi_free). Design §3, §4.1. */
const char* scrivi_fragment_extract(const char* projectRootPath,
                                    const char* spansJson);

/*
 * cut-with-merge — extract the fragment (as scrivi_fragment_extract), then delete the
 * spanned text and collapse the spanned scenes/chapters into ONE continuous scene, exactly
 * as ⌘X behaves in a flat document (design §4.3, delete-and-fold). The head scene keeps its
 * prefix, the tail scene's suffix is folded onto it (blank-line seam), every other scene in
 * the span is deleted, and any chapter left empty is removed. Scenes AFTER the span in a
 * partially-covered chapter are left untouched. `spansJson` has the same shape + ordering as
 * scrivi_fragment_extract.
 *
 * result: { fragment: <scrivi.fragment.v1>, survivingSceneID, removedSceneIDs:[...],
 * removedChapterIDs:[...] } — the fragment is the removed content (for the buffer / undo);
 * the removed IDs are in reading order (for undo, §5). invalidArgument on the same
 * span-validation failures as extract. Standard envelope conventions (heap JSON, free with
 * scrivi_free). Design §4.3. */
const char* scrivi_fragment_cut(const char* projectRootPath,
                                const char* spansJson);

/*
 * paste-splice — insert a scrivi.fragment.v1 (as produced by scrivi_fragment_extract
 * or carried in a copy buffer) at a caret, reconstructing every carried scene/chapter
 * boundary as if inserting into one continuous flat document (design §4.2). The target
 * scene is split at the caret; the leading (head) piece extends the head; each subsequent
 * piece creates a scene (opensWith "scene") or a chapter + first scene (opensWith
 * "chapter") after the running insertion point via the EP-027 create primitives; the
 * target scene's tail-suffix follows the whole pasted run onto the last created scene.
 * Fresh scene/chapter IDs are minted; order-keys are assigned by the EP-027 model.
 *
 * `fragmentJson` is a scrivi.fragment.v1 object { pieces:[...], plainText? }. The caret is
 * (`caretSceneID`, `caretByteOffset`) — a scene-local UTF-8 byte offset into that scene's
 * body; it must be inside a real scene (the editor's caret-in-heading refusal + divider-span
 * normalisation are SP-089 preconditions). identity/persona/displayName authorise the create
 * primitives.
 *
 * result: { targetSceneID, createdSceneIDs:[...], createdChapterIDs:[...] } — the created IDs
 * are in reading order (for history/undo, §5). invalidArgument if the fragment is empty, the
 * caret scene is unknown, or the caret offset exceeds the scene body. Standard envelope
 * conventions (heap JSON, free with scrivi_free). Design §4.2. */
const char* scrivi_fragment_paste(const char* projectRootPath,
                                  const char* appSupportRoot,
                                  const char* projectID,
                                  const char* fragmentJson,
                                  const char* caretSceneID,
                                  long long   caretByteOffset,
                                  const char* identityID,
                                  const char* personaID,
                                  const char* displayName);

/* uncut-paste — the exact inverse of scrivi_fragment_paste (EP-029 AC6 / T-0356): undo of a
 * structured paste, and redo of a structured cut. Folds the created scenes back into the target
 * scene and STRIPS the pasted fragment's piece texts (restoring the target's original pre-paste
 * body = head + tail), then deletes the created scenes/chapters. Keyed by IDs + the fragment,
 * never byte spans — the app does no byte-offset math for the inverse.
 *
 * `fragmentJson` is the same scrivi.fragment.v1 that was pasted (its piece lengths drive the
 * strip). `targetSceneID` is the scene the paste split. `createdIDsJson` is an object
 * { "sceneIDs":[...], "chapterIDs":[...] } listing exactly what the paste created (as reported by
 * scrivi_fragment_paste); absent keys ⇒ empty. result: { survivingSceneID }. invalidArgument if
 * the fragment is empty, the target/created scenes are unknown, or a scene body does not match the
 * pasted piece text it should bound (a state this fragment's paste could not have produced). */
const char* scrivi_fragment_uncut_paste(const char* projectRootPath,
                                        const char* fragmentJson,
                                        const char* targetSceneID,
                                        const char* createdIDsJson);

/* ---- Copy buffers (EP-019 SP-056 — T-0213) ------------------------------ */

/*
 * Multiple copy buffers — vim/emacs-register-style numbered slots (v1: IDs
 * "1"-"9") that the writer loads text into and pastes from WITHOUT touching the
 * system pasteboard (design §9). Per-project + persistent, stored by ScriviCore
 * in history/buffers.json (scrivi.buffers.v1). Independent of the undo/redo
 * history engine: each call is a stateless read-modify-write of buffers.json, so
 * there is no open/close pairing and buffers survive quit/relaunch trivially.
 * Whether a copy/cut/paste becomes a HISTORY event is decided by the app (Trade
 * T3: copy≠event, cut=event, paste=ordinary paste event) — this API only stores
 * the slots. Standard envelope conventions apply (heap JSON, free with
 * scrivi_free). A bufferID outside "1".."9" returns an invalidArgument error.
 */

/* Loads `textUtf8` into slot `bufferID` (create-or-replace), stamping updatedAt.
 * `fragmentJson` is an OPTIONAL serialized scrivi.fragment.v1 object ("" / NULL =
 * plain-text slot, the SP-056 behaviour); when present the slot holds a structured
 * fragment for cross-boundary paste (T-0355 / AC4). A load always replaces both text
 * and fragment. result: {bufferID, updatedAt} */
const char* scrivi_buffers_load(const char* projectRootPath,
                                const char* bufferID,
                                const char* textUtf8,
                                const char* fragmentJson);

/* Reads one slot. result: {bufferID, text, updatedAt, present[, fragment]} —
 * `present` is false and text/updatedAt are empty when the slot is unset; `fragment`
 * (a nested scrivi.fragment.v1 object) appears only for structured slots. */
const char* scrivi_buffers_get(const char* projectRootPath,
                               const char* bufferID);

/* Lists all non-empty slots (ascending by bufferID).
 * result: {count, buffers:[{bufferID, text, updatedAt[, fragment]}, ...]} */
const char* scrivi_buffers_list(const char* projectRootPath);

/* Clears one slot (removes its entry). result: {bufferID, cleared} — `cleared`
 * is false when the slot was already empty. */
const char* scrivi_buffers_clear(const char* projectRootPath,
                                 const char* bufferID);

#ifdef __cplusplus
}
#endif
