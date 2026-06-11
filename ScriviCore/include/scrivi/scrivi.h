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

const char* scrivi_create_chapter(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* identityID,
    const char* personaID,
    const char* displayName);

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

#ifdef __cplusplus
}
#endif
