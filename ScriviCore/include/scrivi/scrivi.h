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
 *
 * THREADS (SP-173, Scrivi_Core_Concurrency_Design_v0_1.md). Any function may be called from any thread. Every function that
 * takes a project root runs under that project's EXCLUSIVE lock, so no call sees another part-way through, and its envelope
 * (ok or not) also carries:
 *   "revision"   -- the project's change counter in this process: a call that may change the project bumps it, a read
 *                   does not. A result read in the background is current only if the revision has not moved since.
 *   "lockWaitMs" -- present only when the call waited 1 ms or more for the project's lock.
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

/* Releases the in-memory state a project accumulated while open -- today the
 * ProjectIndex (EP-039 AC1-AC3), which is built lazily on first use.
 *
 * WARNING: an app that opens projects and never calls this LEAKS ONE INDEX PER
 * PROJECT OPENED, for the life of the process. Call it when a project window
 * closes, alongside scrivi_history_close.
 *
 * Closing a project that was never opened -- or was already closed -- is a
 * NO-OP, not an error: `{"ok":true,"result":{"closed":false}}`. This mirrors
 * scrivi_history_close, so a caller can close unconditionally on a teardown
 * path without first asking whether there is anything to close.
 *
 * This does NOT write to the project. Everything it drops is derived and
 * rebuildable from disk (EP-027: the filesystem is authoritative). */
const char* scrivi_close_project(const char* projectRootPath);

/* Every scene whose story time is EXPLICITLY SET, in manuscript order.
 *
 * SPARSE: scenes sitting on the derived default chain are OMITTED -- their
 * offsets follow from FR-022m and cost nothing to leave out. "Set" means
 * offsetSource != "default", or durationSource != "default", or a non-empty
 * bandID.
 *
 * WARNING: AN EMPTY RESULT IS THE COMMON CASE, NOT AN ERROR. On a project where
 * nobody has pinned a scene in time, this returns zero records -- and the
 * envelope OMITS the "storyTimes" key entirely, because an empty array is not
 * serialised. Callers MUST decide success from the envelope's "ok" field and
 * NEVER from emptiness; "count" is always present for the same reason.
 * Treating empty as failure makes a real timeline draw as blank. */
const char* scrivi_list_story_times(const char* projectRootPath);

const char* scrivi_open_scene(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* sceneID);

/* SP-144 — `scrivi_open_scene` for a BULK VIEWPORT LOAD.
 *
 * Identical to scrivi_open_scene in every respect EXCEPT that it does NOT record
 * the scene as the project's "last writing surface".
 *
 * ⚠️ WHY IT EXISTS. Both platforms call open_scene ONCE PER SCENE to assemble the
 * continuous editor. Each such call read-modify-WROTE `workspace-state.json`, so
 * opening a 61-scene project performed 61 atomic writes to record a value only
 * the last of which survives. MEASURED through this ABI (61 scenes, Linux,
 * strace): the workspace file accounted for 62 opens + 61 `.tmp` opens + 61
 * renames of a 1,662-syscall load, while every manuscript sidecar was opened
 * just 3 times. On a network volume each write is a temp+write+rename
 * round-trip, which is why a SMALL project still loaded slowly.
 *
 * ⚠️ A SEPARATE ENDPOINT RATHER THAN A NEW PARAMETER: changing open_scene's
 * signature would break every existing caller on both platforms at once.
 *
 * ⛔ IT SUPPRESSES ONLY THE WRITE. The restore is unaffected — the result still
 * carries `restored` (selection + scroll) exactly as open_scene does, so a bulk
 * load can still place the writer's cursor. Use scrivi_open_scene for a scene the
 * writer NAVIGATED TO; that call is what records where they are. */
const char* scrivi_open_scene_for_bulk_load(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* sceneID);

/* SP-173 / I-0285 (fix 3) -- the TEXT of many scenes in one call, read in PARALLEL inside the core (the cost of reading a
 * manuscript over a network share is per-file latency; the project lock serialises the platforms' own calls).
 * sceneIDsJson: a JSON array of scene IDs, e.g. ["scene_a","scene_b"]. Read-only.
 * Result: {"scenes":[{"sceneID","markdown"}...], "failed":[{"sceneID","code","message"}...], "count", "failedCount"}, in the
 * order asked. WARNING: "scenes" / "failed" are OMITTED when empty; "count" and "failedCount" are always present.
 * A platform loading a manuscript asks in CHUNKS (e.g. 64) so its progress bar moves between calls. */
const char* scrivi_read_scene_texts(const char* projectRootPath, const char* sceneIDsJson);

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

/* ---------------------------------------------------------------------------
 * Object CRUD (EP-031)
 *
 * `worldID` (SP-098 T-0405, I-0113) names the world a WORLD-SCOPED object lives
 * in. Pass "" (or NULL) for a project-scoped kind.
 *
 * ⚠️ WHICH KINDS ARE WHICH IS NOT WRITTEN HERE, DELIBERATELY (SP-116 T-0430,
 * I-0141). Ask scrivi_list_object_kinds — it derives the answer from
 * kAllStorableKinds + objectKindIsWorldScoped() and cannot go stale.
 *
 * This comment previously enumerated four kinds as world-scoped. SP-103 moved
 * six more to world scope on 2026-08-14 and the sentence rotted where it stood:
 * nobody edited it, the model changed underneath it, and a reader who trusted it
 * would pass "" for a character and get `worldRequired`. That is the failure
 * mode CLAUDE.md names — "a list rots without being edited" — so the list is
 * gone rather than corrected. A corrected list would rot again.
 *
 * ⚠️ Widened in SP-098: SP-097 added `worldID` to the requests and routed
 * storage through the world package, but these entry points were never widened,
 * so world-scoped objects were unreachable through the ABI entirely. The
 * facade-level tests passed throughout — they bypass this layer. Any test for a
 * boundary change belongs HERE, against scrivi_*, not against the C++ facade.
 *
 * `scrivi_save_object` takes no `worldID`: the object JSON it carries already
 * holds one.
 * ------------------------------------------------------------------------- */

const char* scrivi_create_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* displayName,
    const char* slug,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName,
    const char* worldID);

const char* scrivi_open_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectID,
    const char* worldID);

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
    const char* objectID,
    const char* worldID);

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
 * that reads correctly from that endpoint. An edge whose far endpoint lives in
 * an unavailable world still lists, flagged `otherPending` with its cached
 * `otherDisplayName` and `otherWorldStatus`. */
const char* scrivi_list_edges_for(
    const char* projectRootPath,
    const char* endpointID);

/* ---------------------------------------------------------------------------
 * Pending edges (EP-031 SP-098 T-0380) — World Data Separation v0.1 §4.6
 *
 * ⚠️ PENDING IS NOT DANGLING. An endpoint that fails to resolve because its
 * WORLD is unavailable is *pending*: held, never pruned, never modified. An
 * endpoint that fails to resolve while its world IS present is genuinely
 * dangling and is repaired away. Conflating the two destroys a writer's
 * relationships silently — the one failure in this Epic that is unrecoverable.
 *
 * Consequently the graph is FROZEN toward an unavailable world in both
 * directions: scrivi_create_edge and scrivi_delete_edge refuse explicitly with
 * error.detail == "worldPending:<status>", never a silent drop.
 *
 * This endpoint REPORTS; it repairs nothing. Each row carries the cached
 * displayName from the binding (§6.3) rather than a bare ID, so a writer being
 * asked whether to clear references is never deciding blind.
 * ------------------------------------------------------------------------- */

const char* scrivi_list_pending_edges(const char* projectRootPath);

/* ---------------------------------------------------------------------------
 * Object discovery (EP-031 SP-098 T-0378) — Worldbuilding Object Model v0.2 §5.5
 *
 * Both are pure reads over structures already resident — the object index and
 * the in-memory edge map — so they are O(n) with no extra I/O.
 *
 * `kindOrNull` filters scrivi_list_objects by kind ("" / NULL = every kind).
 * World objects appear when their world is available; when it is not, they are
 * simply absent from the listing rather than reported as deleted.
 *
 * ORPHAN = present in the index, absent from every endpoint in the edge map.
 * Orphans are DELIBERATELY RETAINED, never cleaned up: an object with no
 * relationships is a legitimate creative state — a character sketched before
 * they have a scene. Pruning the EDGE while keeping the now-unrelated OBJECT is
 * the correct behaviour; this endpoint exists to make that state findable, not
 * to offer a way to sweep it away.
 * ------------------------------------------------------------------------- */

const char* scrivi_list_objects(const char* projectRootPath, const char* kindOrNull);
const char* scrivi_list_orphaned_objects(const char* projectRootPath);

/* ---------------------------------------------------------------------------
 * Promotion / demotion (EP-031 SP-098 T-0379) — Object Model v0.2 §3.1
 *
 *   item → artifact  moves objects/items/<slug>.json into the world's artifacts/
 *                    and sets worldID (pass it in `worldIDOrNull`)
 *   artifact → item  the exact inverse through this same endpoint; worldID is
 *                    cleared and `worldIDOrNull` is ignored
 *
 * ⚠️ `objectID` IS PRESERVED AND ZERO EDGES ARE REWRITTEN. This is the safety
 * proof for storing bare `{id}` edge endpoints (§5.2): an endpoint that recorded
 * `{kind:"item", id:…}` would be stale on every edge the instant the object was
 * promoted. Here relationships.jsonl is not even opened.
 * ------------------------------------------------------------------------- */

const char* scrivi_promote_object(const char* projectRootPath,
                                  const char* objectID,
                                  const char* targetKind,
                                  const char* worldIDOrNull);

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

/* Both emit `packagePath` AND `lastKnownPackagePath`, and ⚠️ THEY MEAN
 * DIFFERENT THINGS (T-0419, I-0137):
 *
 *   packagePath           — a VERIFIED path. Non-empty ONLY when status ==
 *                           "available". Callers may trust it exists and holds
 *                           this world.
 *   lastKnownPackagePath  — where resolution LOOKED, whatever the outcome. It
 *                           may not exist, may be unreadable, or may hold a
 *                           DIFFERENT world (the identity-mismatch case).
 *                           ⚠️ NEVER treat it as proof of anything.
 *
 * Why the second exists: a host distinguishing "the volume is unmounted" from
 * "the world is merely offline" needs a path in exactly the case `packagePath`
 * is empty by design. Reusing one field for both would have widened a trusted
 * guarantee — the mistake I-0115 was written about. */
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

/* Object kinds and their scope (SP-116 T-0429, D5).
 *
 * Returns { kinds: [ { kind, subdir, isWorldScoped } ], count }.
 *
 * ⚠️ THIS EXISTS SO NO CALLER EVER RESTATES THE KIND PARTITION AGAIN. Before it,
 * nothing in the ABI exposed a kind's scope, so Swift restated it as
 * `kind != "source"` — correct only by accident, and the eighth occurrence of a
 * defect class that once blocked object creation in the app entirely.
 *
 * Derived core-side from kAllStorableKinds + objectKindIsWorldScoped(). Takes no
 * project: a kind's scope is a property of the MODEL, not of any one project.
 *
 * `world` is absent — it is a container created by scrivi_create_world, not a
 * storable object kind.
 */
const char* scrivi_list_object_kinds(void);

/* RelationType vocabulary — scrivi.relation-types.v1 */
const char* scrivi_list_relation_types(const char* projectRootPath);

const char* scrivi_upsert_relation_type(
    const char* projectRootPath,
    const char* relationTypeJson);

/* Assets (EP-005; world scope added SP-116 T-0426, D6)
 *
 * `worldID` names the world whose package the asset lives in, so a world-scoped
 * object's image TRAVELS WITH THE WORLD between projects. Pass "" (or NULL) for
 * the project's own assets — that is the pre-D6 behaviour and remains the
 * default, so existing callers and existing on-disk assets are unaffected.
 *
 * ⚠️ Which kinds are world-scoped is NOT restated here. Ask
 * scrivi_list_object_kinds (T-0429) — a comment enumerating kinds is exactly
 * what rotted in I-0141.
 *
 * A world write takes the package lock and fails with detail "worldLocked" when
 * another writer holds it; an unavailable world fails with
 * "worldUnavailable:<status>", byte-identical to the object endpoints.
 *
 * `projectID` identifies the lock holder; pass "" if unknown.
 *
 * scrivi_list_assets emits `assets` as a real JSON ARRAY of objects, each with
 * assetID / filename / category / title / assetPath (T-0427, T-0428).
 * ⚠️ It was a JSON-encoded STRING built without escaping until SP-116.
 */
const char* scrivi_import_asset(
    const char* projectRootPath,
    const char* sourcePath,
    const char* category,
    const char* title,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName,
    const char* worldID,
    const char* projectID);

const char* scrivi_list_assets(
    const char* projectRootPath,
    const char* category,
    const char* worldID);

const char* scrivi_remove_asset(
    const char* projectRootPath,
    const char* assetID,
    const char* worldID,
    const char* projectID);

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

/* ---- Merge — ⛔ RETIRED [SP-191] (Q3e) ----------------------------------- */
/* scrivi_merge_scene and scrivi_merge_chapter are gone: a scene merge is scrivi_range_cut of the zero-length range across
 * the break, a chapter merge scrivi_range_relabel (below, with the range operations). */

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

/* Inspector layout — `inspector-layout.json` at the project root (EP-041 SP-141,
   T-0507). Closes the [I-0197] Class B bypass: before these, BOTH the Swift and Qt
   apps read and wrote this file themselves, and they had already drifted apart
   ([I-0215] — Linux preserved unknown keys, Apple dropped them).

   ⚠️ THE DOCUMENT IS OPAQUE TO THE CORE (ruled 2026-09-18). The core owns
   atomicity, durability and repair; the APP owns meaning. The core does not
   validate the document's interior and does not know what a card kind or a tab is.
   Unknown keys therefore survive BY CONSTRUCTION — the core never interprets them.
   ⚠️ ACCEPTED COST, recorded so it is not later rediscovered as a defect: the core
   CANNOT validate what it stores here. A malformed layout is the app's to detect.

   ⚠️ ABSENCE SEMANTICS (ruled 2026-09-21) — "core reports, app decides".
   The GET always SUCCEEDS and reports what it found:
     {"ok":true,"result":{"status":"ok","document":{...}}}
     {"ok":true,"result":{"status":"absent"}}
     {"ok":true,"result":{"status":"unreadable","message":"..."}}
   A missing layout is NORMAL, not an error — every project created before this
   file existed has none — so it must not be reported as one. `absent` and
   `unreadable` stay DISTINCT so the app can warn about a recoverable corrupt file
   while silently defaulting for an absent one.
   ⛔ The core NEVER invents defaults (it cannot: it does not know what a tab is)
   and NEVER overwrites a corrupt file on read — the writer's layout may be
   recoverable by hand, and clobbering it on open would destroy that chance. */
const char* scrivi_get_inspector_layout(const char* projectRootPath);

/* Replaces the whole document. `documentJson` must be a JSON OBJECT; anything else
   is rejected as invalidArgument rather than written, since a non-object would make
   the file unreadable to every future GET.
   ⚠️ Writing over an `unreadable` file SUCCEEDS — that is the writer's explicit act,
   as distinct from the read path, which leaves damage alone. */
const char* scrivi_put_inspector_layout(const char* projectRootPath,
                                         const char* documentJson);

/* EP-047 AC1 (SP-167) — settings that TRAVEL with the project ([I-0278]), in
   `project-settings.json` at the package root. The SAME contract as the inspector layout:
   an opaque JSON object whose keys the apps define; GET result {status: "absent"|"unreadable"|"ok"
   [, message][, document]}; PUT replaces the whole document (read, merge your keys, write — so
   another platform's keys survive) and rejects a non-object. */
const char* scrivi_get_project_settings(const char* projectRootPath);
const char* scrivi_put_project_settings(const char* projectRootPath, const char* documentJson);

/* EP-047 AC2 (SP-167) — renames the project: writes `project.json`'s `title` in place (every
   other field kept). An empty or whitespace-only title is rejected. result: {title} */
const char* scrivi_set_project_title(const char* projectRootPath, const char* title);

const char* scrivi_assign_scene_to_band(const char* projectRootPath, const char* sceneID,
                                         const char* bandID);
const char* scrivi_unassign_scene_from_band(const char* projectRootPath, const char* sceneID);

const char* scrivi_get_story_structure(const char* projectRootPath);
const char* scrivi_set_story_structure(const char* projectRootPath, const char* structureID,
                                        const char* bandLayoutJSON);
const char* scrivi_update_band_layout(const char* projectRootPath, const char* bandLayoutJSON);
const char* scrivi_remove_story_structure(const char* projectRootPath);

/* Historical events. `tagsJSON` accepts EITHER `["a","b"]` OR `{"tags":["a","b"]}`
   — ⚠️ both are live: Linux sends the wrapped form, this header long documented the
   bare one, and T-0542 found that the bare form had NEVER worked. An absent or
   unparseable value means "no tags". `scrivi_list_historical_events` PROJECTS tags
   back (it did not before T-0542, which is why Linux read the package directly). */
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

/* SP-173 / I-0285 (Scrivi_Core_Concurrency_Design_v0_1.md D3) -- the timeline's five reads in ONE call, under ONE project
 * lock, so they describe one state of the project. Result keys, each exactly what its standalone endpoint returns:
 *   "timeline"          (scrivi_get_timeline)            "storyTimes"        (scrivi_list_story_times)
 *   "storyStructure"    (scrivi_get_story_structure)     "historicalEvents"  (scrivi_list_historical_events)
 *   "importedTimelines" (scrivi_list_imported_timelines)
 * A part that fails is replaced by "<part>Error" ({code, message}); the others are still returned.
 * WARNING: the standalone endpoints' empty-array traps apply inside each part (e.g. "storyTimes"."storyTimes" is OMITTED
 * when no scene has an explicit story time; use its "count").
 *
 * Every project endpoint's envelope carries "revision" (D2): this call's is the revision all five parts describe. */
const char* scrivi_load_timeline(const char* projectRootPath);

/* SP-173 / I-0285 (D4) -- the project's current "revision" (in the envelope) and nothing else; touches no file. Ask it on the
 * main thread just before applying a result read in the background: apply only if the two revisions match. */
const char* scrivi_get_project_revision(const char* projectRootPath);

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
 *   groupID (optional): events recorded with the SAME non-empty groupID, one after
 *   another, are ONE edit across several scenes (I-0270). scrivi_history_undo/redo
 *   step across the whole run in one call and return one entry per step in `changes`,
 *   in the order taken — the app applies ALL of them, not only the first.
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
 * barrier (undo stops with a notice).
 * OPTIONAL `sceneTexts` (an object, sceneID → text; classic barrier only) [I-0287]:
 * the text the structural op left in EACH scene it rewrote or created — a merge's
 * surviving scene, a split's head and tail. History reads those scenes as this text
 * from the barrier on. ⚠️ Omit it after an op that rewrote scene text and the next
 * edit's undo restores the PRE-op text. result: {eventID, canUndo, canRedo} */
const char* scrivi_history_record_barrier(const char* projectRootPath,
                                           const char* paramsJSON);

/* ---- Undo capture rules (SP-177 / T-0604, EP-026 S1) -------------------- *
 * WHEN an undo step ends — the rules every platform used to carry itself (Apple: HistoryCapture.swift) — decided by the
 * core, one session per open history (opened by scrivi_history_open; scrivi_history_close commits what is pending).
 * Carets are UTF-16 units of the scene text (SP-177 Q2). `nowMs` is any monotonic millisecond clock the platform keeps
 * (SP-177 Q1): the core never reads a clock or owns a timer — it returns `idleDeadlineMs`, and the platform calls
 * scrivi_capture_idle at that time. Every envelope's result carries
 *   {committed, canUndo, canRedo, hasPending, idleDeadlineMs, atBarrier}   (idleDeadlineMs -1 = nothing pending)
 * atBarrier [I-0302]: the newest step is a hard barrier — canUndo is false, but an undo there reports the barrier
 * (stoppedAtBarrier), so a platform keeps Undo available to show its notice.
 * undoCrossesSessionAt [I-0319] (present only then): the next undo crosses a session BORDER — it undoes a change, made
 * at this timestamp, from an earlier session than the undos are in (this session after a new change or an open; else the
 * session of the last change undone or redone) — the platform asks the writer (Undo / Cancel) BEFORE calling undo.
 * redoFork [I-0320] (present only then): the redo from here has >= 2 branches — {nodeID, children:[{eventID, preview,
 * timestamp, isPrimary, sceneID, changeOffsetUtf8}]} as forkAhead ([SP-192]: sceneID + scene-local UTF-8 byte where the
 * branch parts from the current text, as the tree's). The
 * platform shows the branch chooser on REDO (choosing = scrivi_history_select_branch, then redo), never after an undo.
 * Rules (design §4.a / §4.a.1): an edit LATCHES; it commits on `.` `!` `?` or a newline typed, a caret move (another
 * scene: always; the same scene: unless only whitespace changed), paste/cut, 45 s idle, a barrier, undo, close. An
 * autosave never commits unless already idle. A scene's floor is seeded on its first edit of the session. */

/* The writer changed sceneID to sceneTextUtf8 (its whole text); caretUtf16 = the caret after the edit; textBeforeUtf8 =
 * the scene before the edit. Do NOT call for text an undo/redo applied. */
const char* scrivi_capture_note_edit(const char* projectRootPath, const char* sceneID, const char* sceneTextUtf8,
                                     int caretUtf16, const char* textBeforeUtf8, long long nowMs);
/* The caret moved into sceneID. editInFlight: the move came from an edit this turn. applying: an undo/redo is being
 * applied (position only). */
const char* scrivi_capture_caret_moved(const char* projectRootPath, const char* sceneID, int editInFlight, int applying);
/* A paste or cut is about to change the text: commits what was pending and labels the next event kind ("paste"|"cut");
 * bufferID ("1".."9", or "") tags a cut into a copy buffer. Follow the change with scrivi_capture_flush(trigger=kind). */
const char* scrivi_capture_begin(const char* projectRootPath, const char* kind, const char* bufferID);
/* Commits the pending edit now (trigger is a label; kind defaults to "typing"). */
const char* scrivi_capture_flush(const char* projectRootPath, const char* trigger, const char* kind);
/* An autosave is about to write: commits only if the session is already idle (T-0396). */
const char* scrivi_capture_before_save(const char* projectRootPath, long long nowMs);
/* The platform's idle timer fired: commits if idle at nowMs (a stale timer does nothing). */
const char* scrivi_capture_idle(const char* projectRootPath, long long nowMs);
/* One edit across several scenes (I-0270): editsJSON = [{sceneID, textBefore, textAfter, caret}] (caret: UTF-16 in
 * textBefore, where the edit began); one undo step. */
const char* scrivi_capture_grouped_edit(const char* projectRootPath, const char* editsJSON, const char* kind);
/* Commits what was pending, then records a barrier — params as scrivi_history_record_barrier (including sceneTexts,
 * [I-0287]); with no sceneID the pending scene is the attribution. result adds eventID. */
const char* scrivi_capture_record_barrier(const char* projectRootPath, const char* paramsJSON);
/* Commits what was pending, then scrivi_history_undo; result as scrivi_history_undo plus hasPending. Each change also
 * carries cursorAfterUtf16. */
const char* scrivi_capture_undo(const char* projectRootPath);
/* scrivi_history_redo (nothing is committed first — as Apple); result as scrivi_history_redo plus hasPending. */
const char* scrivi_capture_redo(const char* projectRootPath);
/* After applying an undo/redo: the caret rests in sceneID, which now reads sceneTextUtf8. */
const char* scrivi_capture_note_applied(const char* projectRootPath, const char* sceneID, const char* sceneTextUtf8);
/* The current {canUndo, canRedo, hasPending, idleDeadlineMs}. */
const char* scrivi_capture_state(const char* projectRootPath);

/* Moves the current pointer back one node.
 * result: {moved, changes:[{sceneID,newText,cursorAfter}], nodeID,
 *          canUndo, canRedo, crossedSessionBoundary, boundaryTimestamp?,
 *          (crossedSessionBoundary [I-0319]: this undo crossed a session border (see undoCrossesSessionAt);
 *           boundaryTimestamp = that change's time — never set for undoing this session's change),
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
 * records an externalChange barrier carrying the disk text (barrier.sceneTexts,
 * I-0288 — the scene reads it from that node on; its floor is untouched) —
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
 * result: { targetSceneID, createdSceneIDs:[...], createdChapterIDs:[...], sceneTexts:{sceneID: body} } — the created
 * IDs are in reading order (for history/undo, §5); sceneTexts [I-0297] is the body it WROTE to the target and every
 * created scene, exactly as on disk (no need to read them back). While the project's history is open, those writes are
 * recorded as Scrivi's own [I-0296]. invalidArgument if the fragment is empty, the
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
 * scrivi_fragment_paste); absent keys ⇒ empty. result: { survivingSceneID, sceneTexts:{survivingSceneID: body} } —
 * sceneTexts [I-0297] is the restored body as written. invalidArgument if
 * the fragment is empty, the target/created scenes are unknown, or a scene body does not match the
 * pasted piece text it should bound (a state this fragment's paste could not have produced). */
const char* scrivi_fragment_uncut_paste(const char* projectRootPath,
                                        const char* fragmentJson,
                                        const char* targetSceneID,
                                        const char* createdIDsJson);

/* ---- Range operations (EP-053 S1, SP-186 — T-0614) ---------------------- */

/*
 * The Range (design v0.2 §4.2): ONE contiguous run through the manuscript, as an ordered list of scene entries —
 * scrivi.range.v1:
 *   { "schema": "scrivi.range.v1",
 *     "scenes": [ { sceneID, chapterID, chapterTitle, sceneTitle, start, length, sceneLength, text }, ... ] }
 * The array's order IS the links. A scene break is the step from one entry to the next; a CHAPTER break is a step between
 * entries whose chapterID differs. `start`, `length` and `sceneLength` are UTF-8 BYTES; only the first entry may start
 * mid-scene and only the last may end mid-scene; `length` is the byte length of `text`.
 *
 * The write operations return the DELTA, the whole change in reading order, for the platform to apply to its own scene
 * list instead of re-opening the project:
 *   { scenes: [ { change: "created"|"rewritten"|"moved", sceneID, chapterID, title, chapterTitle, metadataPath,
 *                 contentPath, chapterMetadataPath, afterSceneID, text? } ],      (text absent for "moved")
 *     removedSceneIDs: [...], createdChapters: [ { chapterID, title, metadataPath, afterChapterID } ],
 *     removedChapterIDs: [...] }
 * ⚠️ An empty list is an ABSENT key. `afterSceneID` is the scene before it in reading order now ("" = the first).
 * While the project's history is open, every scene text written is recorded as Scrivi's own [I-0296].
 * Standard envelope conventions (heap JSON, free with scrivi_free).
 * ✅ [SP-191] Q3b: a Range entry may carry `sceneMeta` / `chapterMeta` — the removed scene's / chapter's sidecar, serialized
 * as a STRING. A cut and a relabel that empties a chapter fill them; a paste that keeps an entry's ID (the manuscript no
 * longer has it) writes them back, so an undo restores the sidecar exactly. An empty `sceneID` / `chapterID` mints one.
 * ✅ [SP-188] Q3: a Range may carry a top-level `"join": { sceneID, byte, balanced, plain }` — set by scrivi_range_cut on
 * the Range it returns when it BALANCED the join (head and tail joined directly would leave an emphasis span open): at
 * `byte` of the survivor it wrote `balanced` where the plain join had `plain`. An UNDO RECORD: scrivi_range_paste puts
 * `plain` back before pasting, and refuses the Range anywhere but at that survivor as the cut left it. A platform drops it
 * before the Range goes on the clipboard or into a buffer. A cut ignores a join it is given.
 */

/* Reads the Range from (startSceneID, startByte) to (endSceneID, endByte), walking the manuscript's order — reads only
 * those scenes' texts. result: { range }. invalidArgument if a scene is unknown, the end precedes the start, or an
 * offset is outside its scene or inside a UTF-8 sequence. */
const char* scrivi_range_read(const char* projectRootPath,
                              const char* startSceneID, long long startByte,
                              const char* endSceneID, long long endByte);

/* Pastes a Range at a caret (design §4.4). The caret scene is split at `caretByteOffset`; its head takes the first
 * entry's text; each later entry creates a scene, or a chapter and its first scene where the chapter changes; the tail
 * goes onto the last created scene. When the Range opens a chapter, the caret chapter's scenes after the caret move into
 * the last created chapter [I-0311]. A scene or chapter is re-created under the Range's own ID when the manuscript does
 * not have it (it was cut), else under a fresh one (it was copied). ✅ [SP-191] Q7: at a scene's very start (offset 0,
 * more than one entry) the new scenes go BEFORE the caret scene, which keeps its identity and takes the LAST entry's text
 * in front of its own. `caretSceneText` is the caret scene's text as the
 * editor holds it: ⛔ refused (invalidArgument) when it is not what is on disk. result: { delta, range } — `range` is what
 * the paste INSERTED, under the real IDs (its undo is scrivi_range_cut of exactly that). */
const char* scrivi_range_paste(const char* projectRootPath,
                               const char* rangeJson,
                               const char* caretSceneID,
                               long long   caretByteOffset,
                               const char* caretSceneText,
                               const char* identityID,
                               const char* personaID,
                               const char* displayName);

/* Cuts a Range out of the manuscript (design §4.3): the start scene keeps the text before it, joined directly to the
 * text after it in the end scene; every other scene in it is deleted; every chapter left empty is deleted; the end
 * chapter's scenes after it join the start scene's chapter. Relationship edges are NOT pruned (a paste of the same Range
 * brings the IDs back). ⛔ Refused (invalidArgument) when the Range is not what the manuscript holds — its scenes
 * consecutive, in the chapters it names, its first and last texts as on disk. ✅ [SP-191] Q7: when the Range begins at
 * the start scene's very first character the END scene survives instead (with its tail): the scene whose text survives
 * keeps its identity. ✅ [SP-188] Q3: the join is BALANCED (emphasis never left open); `range` then carries the `join` its
 * undo needs. result: { delta, range, undoCaretSceneID, undoCaretByte } — `range` is the Range removed (the clipboard's
 * content, its join dropped); its undo is scrivi_range_paste of it at (undoCaretSceneID, undoCaretByte), a byte of the
 * PLAIN join. */
const char* scrivi_range_cut(const char* projectRootPath,
                             const char* rangeJson,
                             const char* identityID,
                             const char* personaID,
                             const char* displayName);

/* Relabels a run of whole scenes into another chapter ([SP-191] Q3e — a chapter merge, and its undo). `rangeJson` names the
 * run: one chapter's consecutive scenes (their text is not read; entries may carry empty text). `targetChapterID` is:
 *   • the chapter BEFORE the run's — the run must open its chapter; it is appended there (a chapter merge);
 *   • the chapter AFTER it — the run must close its chapter; it goes before that chapter's first scene;
 *   • a chapter the manuscript does not have ("" = a fresh ID) — the run must close its chapter; the chapter is created
 *     right after it, from the first entry's `chapterMeta` when it has one (an undo), else new.
 * The reading order never changes. On disk the scene files MOVE to the target chapter's folder (their sidecars' slug and
 * contentPath rewritten only when their names would collide there); no text is rewritten. A chapter left empty is deleted
 * after its scenes are out, and its sidecar travels as the result range's first entry's `chapterMeta`.
 * result: { delta, range, fromChapterID } — the inverse is scrivi_range_relabel(range, fromChapterID). */
const char* scrivi_range_relabel(const char* projectRootPath,
                                 const char* rangeJson,
                                 const char* targetChapterID,
                                 const char* identityID,
                                 const char* personaID,
                                 const char* displayName);

/* ---- The write queue (EP-052 SP-194 — T-0624) --------------------------- */

/*
 * One write queue per project, in the core (docs/Scrivi_Background_Writes_Design_v0_1.md W1–W7, v0.2 §3): ONE writer
 * thread takes the project's writes first-in-first-out, so they land in the order they were made. An enqueue returns at
 * once with a TICKET (increasing per project). Every synchronous endpoint that may touch the project's files first waits
 * for the jobs queued before it, so it sees every earlier write (W3). The history log's appends and checkpoints go through
 * the same queue. scrivi_history_close returns once its final records have landed.
 */

/* Queues scene saves — each one, when it runs, is scrivi_save_scene, and on success tells history the bytes it wrote
 * (scrivi_history_note_scene_persisted is then not the platform's to call). A scene queued again before its write starts
 * keeps one save, the newer, at the end of the queue — never moved past a queued structural operation.
 * scenesJson: { projectID, appSupportRoot, identityID, personaID, displayName,
 *               scenes: [ { sceneID, metadataPath, contentPath, markdown, selectionAnchor, selectionFocus, scroll } ] }
 * result: { tickets: [ { sceneID, ticket } ], ticket }   (ticket = the last; absent for an empty list) */
const char* scrivi_enqueue_save_scenes(const char* projectRootPath, const char* scenesJson);

/* Queues one structural operation — when it runs, scrivi_range_paste, scrivi_range_cut or scrivi_range_relabel with these
 * arguments, checked against the manuscript as it is THEN (W7c). Its outcome carries that endpoint's result, the delta
 * included (W7-Q1 (a)).
 * opJson: { op: "paste" | "cut" | "relabel", range, identityID, personaID, displayName,
 *           caretSceneID, caretByteOffset, caretSceneText   (paste)
 *           targetChapterID                                 (relabel) }
 * result: { ticket } */
const char* scrivi_enqueue_range_op(const char* projectRootPath, const char* opJson);

/* Waits until every job up to `ticket` has landed (ticket <= 0: every job queued so far), at most `timeoutMs` (negative:
 * as long as it takes; 0: just look). Holds no lock while it waits. Returns every outcome since the last call:
 * result: { landed, held, heldReason?, pending,
 *           outcomes: [ { ticket, kind: "save" | "structural" | "history", sceneID?, ok, result?, error?,
 *                         message?, supersededBy?, file?, held? } ] }
 * HELD: a save or structural write failed with an I/O or permission error (the drive is pulled, the share drops). It
 * stays first, every later job waits behind it, and the core retries it every 5 s; its first failure is an outcome (with
 * held: true), and so is its success when it lands. While held, this call returns at once (landed false), and every synchronous endpoint
 * that would wait behind the queue fails at once with error.detail "writeQueueHeld". scrivi_close_project does not wait
 * for a held queue: its jobs stay queued and land when the project is back. Any other failed write is reported and the
 * queue goes on.
 * A save's outcome with supersededBy was replaced by that later ticket's save. A history outcome is reported only when
 * it FAILED (one per file); its records are written ahead of the next ([I-0268]).
 * WARNING: "outcomes" is OMITTED when there are none. */
const char* scrivi_wait_for_writes(const char* projectRootPath, long long ticket, long long timeoutMs);

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
 * `rangeJson` is an OPTIONAL serialized scrivi.range.v1 object ("" / NULL = plain-text
 * slot, the SP-056 behaviour); when present the slot holds a structured Range for
 * cross-boundary paste. ⛔ Anything else (the old scrivi.fragment.v1 included) is refused
 * (invalidArgument). A load always replaces both text and range. result: {bufferID, updatedAt}
 * ✅ [SP-191] Q4a: a slot written before the buffers held Ranges (it holds a `fragment`)
 * reads as EMPTY, and the next write drops it. */
const char* scrivi_buffers_load(const char* projectRootPath,
                                const char* bufferID,
                                const char* textUtf8,
                                const char* rangeJson);

/* Reads one slot. result: {bufferID, text, updatedAt, present[, range]} —
 * `present` is false and text/updatedAt are empty when the slot is unset; `range`
 * (a nested scrivi.range.v1 object) appears only for structured slots. */
const char* scrivi_buffers_get(const char* projectRootPath,
                               const char* bufferID);

/* Lists all non-empty slots (ascending by bufferID).
 * result: {count, buffers:[{bufferID, text, updatedAt[, range]}, ...]} */
const char* scrivi_buffers_list(const char* projectRootPath);

/* Clears one slot (removes its entry). result: {bufferID, cleared} — `cleared`
 * is false when the slot was already empty. */
const char* scrivi_buffers_clear(const char* projectRootPath,
                                 const char* bufferID);

/* EP-048 L1 (SP-165) — what one BLOCK of manuscript Markdown is (a block = a maximal run of
 * non-blank lines). Stateless: no project. For a platform with no Markdown parser of its own
 * (Linux); it reports what Apple's MarkdownBlocks.analyze reports, and an interop test holds
 * the two in agreement.
 * Every range is {start, end}: half-open UTF-8 BYTE offsets relative to `blockUtf8`.
 * result: {length,
 *          headings:  [{line, prefix, level}],
 *          listItems: [{line, prefix, ordered, number}],
 *          markers:   [{range, opens}],          emphasis delimiters the parser used
 *          styleRuns: [{range, bits}],           bits: 1 italic, 2 bold; unstyled bytes omitted
 *          spans:     [{start, end}]}            emphasis + its markers (the reveal unit)
 * ⚠️ An EMPTY array is OMITTED (the envelope convention) — read a missing key as empty. */
const char* scrivi_analyze_markdown(const char* blockUtf8);

/* SP-174 / T-0601 (EP-048 S3a) -- THE FORMAT COMMANDS AND BALANCED EDITS, for both platforms (a port of Apple's Swift).
 * Stateless: each takes ONE SCENE's text (UTF-8; a formatting block never crosses a scene) and returns ONE edit to apply as ONE
 * history event. ALL OFFSETS ARE UTF-16 UNITS of that text (both platforms index text in UTF-16).
 * An edit result: {refused, range:{location,length}, replacement, selection:{location,length}} -- `refused` true means "do
 * nothing" (a command that could only be done by destroying formatting is refused, never half-done).
 * Bits: 1 italic, 2 bold.
 *
 * word_range:      the word the caret is IN -> {found, range}; between words found is false (the platform's pending pair).
 * emphasis_edit:   toggle bold/italic on a selection; caret >= 0: a WORD formatted for that caret, which is kept.
 * paragraph_edit:  format = body | heading1 | heading2 | heading3 | bulletList | numberedList, per paragraph touched;
 *                  numbered items stay sequential.
 * list_return_edit: Return in a list item (continue / end the list); refused when the caret is not in one.
 * balanced_edit:   replace a range with `replacementUtf8` (stored form) keeping emphasis balanced -> {plain} when the edit as
 *                  typed is already balanced (the common case), else {range, replacement, caret}.
 *                  keepsOwnFormatting != 0: Scrivi's own copy being pasted keeps ITS styles.
 * balanced_copy:   {source, presented} -- what Scrivi's own paste gets, and what other apps get.
 * style_at_caret:  {style} -- what typing at the caret would take.
 * balance_paste_pieces: a cross-scene paste's pieces (JSON array of strings) inside the caret's span -> {pieces, count}.
 *                  WARNING: "pieces" is OMITTED when empty; use "count". */
const char* scrivi_markdown_word_range(const char* sceneUtf8, int caret);
const char* scrivi_markdown_emphasis_edit(const char* sceneUtf8, int location, int length, int bit, int caret);
const char* scrivi_markdown_paragraph_edit(const char* sceneUtf8, int location, int length, const char* format);
const char* scrivi_markdown_list_return_edit(const char* sceneUtf8, int caret);
const char* scrivi_markdown_balanced_edit(const char* sceneUtf8, int location, int length, const char* replacementUtf8,
                                          int keepsOwnFormatting);
const char* scrivi_markdown_balanced_copy(const char* sceneUtf8, int location, int length);
const char* scrivi_markdown_style_at_caret(const char* sceneUtf8, int caret);
const char* scrivi_markdown_balance_paste_pieces(const char* piecesJson, int caretStyle);

#ifdef __cplusplus
}
#endif
