import Foundation
import ScriviCore

// ScriviEngine is the single Swift-side entry point to the ScriviCore C API.
//
// Each method calls a plain-C scrivi_* function, receives a heap-allocated
// UTF-8 JSON string, converts it to Swift String, calls scrivi_free(), then
// decodes the JSON envelope via Codable. No C++ types cross the boundary.

public final class ScriviEngine: @unchecked Sendable {

    public init() {}

    // MARK: — ensureLocalIdentity

    public func ensureLocalIdentity(
        displayName: String,
        appSupportRoot: String
    ) throws -> IdentityResult {
        let raw = displayName.withCString { dn in
            appSupportRoot.withCString { asr in
                scrivi_ensure_local_identity(dn, asr)
            }
        }
        return try decodeC(raw)
    }

    // MARK: — createProject

    public func createProject(
        projectRootPath: String,
        appSupportRoot: String,
        title: String,
        slug: String,
        authorshipRef: AuthorshipRef
    ) throws -> CreateProjectResult {
        let raw = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                title.withCString { t in
                    slug.withCString { s in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { dn in
                                    scrivi_create_project(prp, asr, t, s, iid, pid, dn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — openProject

    public func openProject(
        projectRootPath: String,
        appSupportRoot: String,
        identityID: String = ""
    ) throws -> OpenProjectResult {
        let raw = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                identityID.withCString { iid in
                    scrivi_open_project(prp, asr, iid)
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — openScene

    public func openScene(
        projectRootPath: String,
        appSupportRoot: String,
        projectID: String,
        sceneID: String
    ) throws -> OpenSceneResult {
        let raw = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                projectID.withCString { pid in
                    sceneID.withCString { sid in
                        scrivi_open_scene(prp, asr, pid, sid)
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — saveScene

    public func saveScene(
        projectID: String,
        projectRootPath: String,
        appSupportRoot: String,
        sceneID: String,
        sceneMetadataPath: String,
        sceneContentPath: String,
        markdown: String,
        authorshipRef: AuthorshipRef
    ) throws -> SaveSceneResult {
        let raw = projectID.withCString { pid in
            projectRootPath.withCString { prp in
                appSupportRoot.withCString { asr in
                    sceneID.withCString { sid in
                        sceneMetadataPath.withCString { smp in
                            sceneContentPath.withCString { scp in
                                markdown.withCString { md in
                                    authorshipRef.identityID.withCString { iid in
                                        authorshipRef.personaID.withCString { perid in
                                            authorshipRef.displayName.withCString { dn in
                                                scrivi_save_scene(
                                                    pid, prp, asr, sid, smp, scp,
                                                    md, iid, perid, dn
                                                )
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — scanForExternalChanges

    public func scanForExternalChanges(
        projectRootPath: String,
        appSupportRoot: String,
        includeGitStatus: Bool = true
    ) throws -> ScanResult {
        let raw = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                scrivi_scan_for_external_changes(prp, asr, includeGitStatus ? 1 : 0)
            }
        }
        return try decodeC(raw)
    }

    // MARK: — applyRepair

    public func applyRepair(
        issueID: String,
        projectRootPath: String,
        appSupportRoot: String,
        actionKind: String,
        targetPath: String = "",
        authorshipRef: AuthorshipRef
    ) throws -> ApplyRepairResult {
        let raw = issueID.withCString { iid in
            projectRootPath.withCString { prp in
                appSupportRoot.withCString { asr in
                    actionKind.withCString { ak in
                        targetPath.withCString { tp in
                            authorshipRef.identityID.withCString { identID in
                                authorshipRef.personaID.withCString { pid in
                                    authorshipRef.displayName.withCString { dn in
                                        scrivi_apply_repair(iid, prp, asr, ak, tp, identID, pid, dn)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — enableGitSnapshots

    public func enableGitSnapshots(
        projectRootPath: String,
        authorshipRef: AuthorshipRef,
        initialSnapshotLabel: String = "Initial project"
    ) throws -> EnableGitResult {
        let raw = projectRootPath.withCString { prp in
            authorshipRef.identityID.withCString { iid in
                authorshipRef.personaID.withCString { pid in
                    authorshipRef.displayName.withCString { dn in
                        initialSnapshotLabel.withCString { label in
                            scrivi_enable_git_snapshots(prp, iid, pid, dn, label)
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — createSnapshot

    public func createSnapshot(
        projectRootPath: String,
        authorshipRef: AuthorshipRef,
        label: String,
        note: String = ""
    ) throws -> CreateSnapshotResult {
        let raw = projectRootPath.withCString { prp in
            authorshipRef.identityID.withCString { iid in
                authorshipRef.personaID.withCString { pid in
                    authorshipRef.displayName.withCString { dn in
                        label.withCString { lbl in
                            note.withCString { n in
                                scrivi_create_snapshot(prp, iid, pid, dn, lbl, n)
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Object CRUD

    public func createObject(
        projectRootPath: String,
        objectKind: String,
        displayName: String,
        slug: String = "",
        authorshipRef: AuthorshipRef
    ) throws -> CreateObjectResult {
        let raw = projectRootPath.withCString { prp in
            objectKind.withCString { ok in
                displayName.withCString { dn in
                    slug.withCString { s in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { adn in
                                    scrivi_create_object(prp, ok, dn, s, iid, pid, adn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func openObject(
        projectRootPath: String,
        objectKind: String,
        objectID: String
    ) throws -> OpenObjectResult {
        let raw = projectRootPath.withCString { prp in
            objectKind.withCString { ok in
                objectID.withCString { oid in
                    scrivi_open_object(prp, ok, oid)
                }
            }
        }
        return try decodeC(raw)
    }

    public func saveObject(
        projectRootPath: String,
        objectKind: String,
        objectJson: String,
        authorshipRef: AuthorshipRef
    ) throws -> SaveObjectResult {
        let raw = projectRootPath.withCString { prp in
            objectKind.withCString { ok in
                objectJson.withCString { oj in
                    authorshipRef.identityID.withCString { iid in
                        authorshipRef.personaID.withCString { pid in
                            authorshipRef.displayName.withCString { adn in
                                scrivi_save_object(prp, ok, oj, iid, pid, adn)
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func deleteObject(
        projectRootPath: String,
        objectKind: String,
        objectID: String
    ) throws -> DeleteObjectResult {
        let raw = projectRootPath.withCString { prp in
            objectKind.withCString { ok in
                objectID.withCString { oid in
                    scrivi_delete_object(prp, ok, oid)
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Assets

    public func importAsset(
        projectRootPath: String,
        sourcePath: String,
        category: String,
        title: String,
        authorshipRef: AuthorshipRef
    ) throws -> ImportAssetResult {
        let raw = projectRootPath.withCString { prp in
            sourcePath.withCString { sp in
                category.withCString { cat in
                    title.withCString { t in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { adn in
                                    scrivi_import_asset(prp, sp, cat, t, iid, pid, adn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func listAssets(
        projectRootPath: String,
        category: String = ""
    ) throws -> ListAssetsResult {
        let raw = projectRootPath.withCString { prp in
            category.withCString { cat in
                scrivi_list_assets(prp, cat)
            }
        }
        return try decodeC(raw)
    }

    public func removeAsset(
        projectRootPath: String,
        assetID: String
    ) throws -> RemoveAssetResult {
        let raw = projectRootPath.withCString { prp in
            assetID.withCString { aid in
                scrivi_remove_asset(prp, aid)
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Comments

    public func addComment(
        projectRootPath: String,
        scopeKind: String,
        targetID: String,
        body: String,
        authorshipRef: AuthorshipRef
    ) throws -> AddCommentResult {
        let raw = projectRootPath.withCString { prp in
            scopeKind.withCString { sk in
                targetID.withCString { tid in
                    body.withCString { b in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { adn in
                                    scrivi_add_comment(prp, sk, tid, b, iid, pid, adn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func listComments(
        projectRootPath: String,
        scopeKind: String,
        targetID: String
    ) throws -> ListCommentsResult {
        let raw = projectRootPath.withCString { prp in
            scopeKind.withCString { sk in
                targetID.withCString { tid in
                    scrivi_list_comments(prp, sk, tid)
                }
            }
        }
        return try decodeC(raw)
    }

    public func resolveComment(
        projectRootPath: String,
        scopeKind: String,
        targetID: String,
        commentID: String,
        authorshipRef: AuthorshipRef
    ) throws -> ResolveCommentResult {
        let raw = projectRootPath.withCString { prp in
            scopeKind.withCString { sk in
                targetID.withCString { tid in
                    commentID.withCString { cid in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { adn in
                                    scrivi_resolve_comment(prp, sk, tid, cid, iid, pid, adn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Inbox

    public func listInbox(
        projectRootPath: String
    ) throws -> ListInboxResult {
        let raw = projectRootPath.withCString { prp in
            scrivi_list_inbox(prp)
        }
        return try decodeC(raw)
    }

    public func importFromInbox(
        projectRootPath: String,
        filename: String,
        action: String,
        category: String = "other",
        authorshipRef: AuthorshipRef
    ) throws -> ImportFromInboxResult {
        let raw = projectRootPath.withCString { prp in
            filename.withCString { fn in
                action.withCString { act in
                    category.withCString { cat in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { adn in
                                    scrivi_import_from_inbox(prp, fn, act, cat, iid, pid, adn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — reorderScene / reorderChapter

    public func reorderScene(
        projectRootPath: String,
        sceneID: String,
        sourceChapterID: String,
        targetChapterID: String,
        afterSceneID: String = ""
    ) throws -> ReorderSceneResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in
                sourceChapterID.withCString { src in
                    targetChapterID.withCString { tgt in
                        afterSceneID.withCString { after in
                            scrivi_reorder_scene(prp, sid, src, tgt, after)
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func reorderChapter(
        projectRootPath: String,
        chapterID: String,
        afterChapterID: String = ""
    ) throws -> ReorderChapterResult {
        let raw = projectRootPath.withCString { prp in
            chapterID.withCString { cid in
                afterChapterID.withCString { after in
                    scrivi_reorder_chapter(prp, cid, after)
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — deleteScene / deleteChapter

    public func deleteScene(
        projectRootPath: String,
        sceneID: String
    ) throws -> DeleteSceneResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in
                scrivi_delete_scene(prp, sid)
            }
        }
        return try decodeC(raw)
    }

    public func deleteChapter(
        projectRootPath: String,
        chapterID: String
    ) throws -> DeleteChapterResult {
        let raw = projectRootPath.withCString { prp in
            chapterID.withCString { cid in
                scrivi_delete_chapter(prp, cid)
            }
        }
        return try decodeC(raw)
    }

    // MARK: — renameScene / renameChapter

    public func renameScene(
        projectRootPath: String,
        metadataPath: String,
        newTitle: String
    ) throws -> RenameSceneResult {
        let raw = projectRootPath.withCString { prp in
            metadataPath.withCString { mp in
                newTitle.withCString { t in
                    scrivi_rename_scene(prp, mp, t)
                }
            }
        }
        return try decodeC(raw)
    }

    public func renameChapter(
        projectRootPath: String,
        metadataPath: String,
        newTitle: String
    ) throws -> RenameChapterResult {
        let raw = projectRootPath.withCString { prp in
            metadataPath.withCString { mp in
                newTitle.withCString { t in
                    scrivi_rename_chapter(prp, mp, t)
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — createScene / createChapter

    public func createScene(
        projectRootPath: String,
        appSupportRoot: String,
        projectID: String,
        chapterID: String,
        afterSceneID: String = "",
        authorshipRef: AuthorshipRef
    ) throws -> CreateSceneResult {
        let raw = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                projectID.withCString { pid in
                    chapterID.withCString { cid in
                        afterSceneID.withCString { asid in
                            authorshipRef.identityID.withCString { iid in
                                authorshipRef.personaID.withCString { perid in
                                    authorshipRef.displayName.withCString { dn in
                                        scrivi_create_scene(prp, asr, pid, cid, asid, iid, perid, dn)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func createChapter(
        projectRootPath: String,
        appSupportRoot: String,
        projectID: String,
        authorshipRef: AuthorshipRef
    ) throws -> CreateChapterResult {
        let raw = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                projectID.withCString { pid in
                    authorshipRef.identityID.withCString { iid in
                        authorshipRef.personaID.withCString { perid in
                            authorshipRef.displayName.withCString { dn in
                                scrivi_create_chapter(prp, asr, pid, iid, perid, dn)
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Timeline (EP-016 SP-039)

    public func getTimeline(projectRootPath: String) throws -> GetTimelineResult {
        let raw = projectRootPath.withCString { scrivi_get_timeline($0) }
        return try decodeC(raw)
    }

    public func setTimelineEpochLabel(projectRootPath: String, label: String) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            label.withCString { l in scrivi_set_timeline_epoch_label(prp, l) }
        }
        return try decodeC(raw)
    }

    public func setSceneStoryTime(projectRootPath: String, sceneID: String,
                                   offsetMs: Int64, source: String,
                                   gapMs: Int64 = 0,
                                   durationMs: Int64 = 3_600_000,
                                   durationSource: String = "default") throws -> SceneStoryTimeResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in
                source.withCString { src in
                    durationSource.withCString { ds in
                        scrivi_set_scene_story_time(prp, sid, offsetMs, src, gapMs, durationMs, ds)
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func getSceneStoryTime(projectRootPath: String, sceneID: String) throws -> SceneStoryTimeResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in scrivi_get_scene_story_time(prp, sid) }
        }
        return try decodeC(raw)
    }

    public func clearSceneStoryTime(projectRootPath: String, sceneID: String) throws -> SceneStoryTimeResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in scrivi_clear_scene_story_time(prp, sid) }
        }
        return try decodeC(raw)
    }

    public func assignSceneToBand(projectRootPath: String, sceneID: String, bandID: String) throws -> SceneStoryTimeResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in
                bandID.withCString { bid in scrivi_assign_scene_to_band(prp, sid, bid) }
            }
        }
        return try decodeC(raw)
    }

    public func unassignSceneFromBand(projectRootPath: String, sceneID: String) throws -> SceneStoryTimeResult {
        let raw = projectRootPath.withCString { prp in
            sceneID.withCString { sid in scrivi_unassign_scene_from_band(prp, sid) }
        }
        return try decodeC(raw)
    }

    public func getStoryStructure(projectRootPath: String) throws -> StoryStructureResult {
        let raw = projectRootPath.withCString { scrivi_get_story_structure($0) }
        return try decodeC(raw)
    }

    public func setStoryStructure(projectRootPath: String, structureID: String,
                                   bandLayoutJSON: String = "") throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            structureID.withCString { sid in
                bandLayoutJSON.withCString { bl in scrivi_set_story_structure(prp, sid, bl) }
            }
        }
        return try decodeC(raw)
    }

    public func updateBandLayout(projectRootPath: String, bandLayoutJSON: String) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            bandLayoutJSON.withCString { bl in scrivi_update_band_layout(prp, bl) }
        }
        return try decodeC(raw)
    }

    public func removeStoryStructure(projectRootPath: String) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { scrivi_remove_story_structure($0) }
        return try decodeC(raw)
    }

    public func createHistoricalEvent(projectRootPath: String, title: String,
                                       offsetMs: Int64, description: String = "",
                                       tagsJSON: String = "{}",
                                       authorshipRef: AuthorshipRef) throws -> HistoricalEventResult {
        let raw = projectRootPath.withCString { prp in
            title.withCString { t in
                description.withCString { desc in
                    tagsJSON.withCString { tj in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { dn in
                                    scrivi_create_historical_event(prp, t, offsetMs, desc, tj, iid, pid, dn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func updateHistoricalEvent(projectRootPath: String, eventID: String,
                                       title: String, offsetMs: Int64,
                                       description: String = "", tagsJSON: String = "{}") throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            eventID.withCString { eid in
                title.withCString { t in
                    description.withCString { desc in
                        tagsJSON.withCString { tj in
                            scrivi_update_historical_event(prp, eid, t, offsetMs, desc, tj)
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    public func deleteHistoricalEvent(projectRootPath: String, eventID: String) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            eventID.withCString { eid in scrivi_delete_historical_event(prp, eid) }
        }
        return try decodeC(raw)
    }

    public func listHistoricalEvents(projectRootPath: String) throws -> HistoricalEventsListResult {
        let raw = projectRootPath.withCString { scrivi_list_historical_events($0) }
        return try decodeC(raw)
    }

    public func importExternalTimeline(projectRootPath: String, timelineJSON: String,
                                        epochOffsetMs: Int64, assignedGreyShade: String = "") throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            timelineJSON.withCString { tj in
                assignedGreyShade.withCString { gs in
                    scrivi_import_external_timeline(prp, tj, epochOffsetMs, gs)
                }
            }
        }
        return try decodeC(raw)
    }

    public func updateImportedTimelineOffset(projectRootPath: String, timelineID: String,
                                              epochOffsetMs: Int64) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            timelineID.withCString { tid in
                scrivi_update_imported_timeline_offset(prp, tid, epochOffsetMs)
            }
        }
        return try decodeC(raw)
    }

    public func setImportedTimelineVisible(projectRootPath: String, timelineID: String,
                                            visible: Bool) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            timelineID.withCString { tid in
                scrivi_set_imported_timeline_visible(prp, tid, visible ? 1 : 0)
            }
        }
        return try decodeC(raw)
    }

    public func listImportedTimelines(projectRootPath: String) throws -> ImportedTimelinesListResult {
        let raw = projectRootPath.withCString { scrivi_list_imported_timelines($0) }
        return try decodeC(raw)
    }

    public func removeImportedTimeline(projectRootPath: String, timelineID: String) throws -> TimelineBoolResult {
        let raw = projectRootPath.withCString { prp in
            timelineID.withCString { tid in scrivi_remove_imported_timeline(prp, tid) }
        }
        return try decodeC(raw)
    }

    public func exportProjectTimeline(projectRootPath: String) throws -> ExportTimelineResult {
        let raw = projectRootPath.withCString { scrivi_export_project_timeline($0) }
        return try decodeC(raw)
    }
}

// MARK: — C boundary decode helper

// Converts a heap-allocated const char* JSON result from scrivi_* into a
// Swift Decodable value, calling scrivi_free() unconditionally before returning.
private func decodeC<T: Decodable>(_ ptr: UnsafePointer<CChar>?) throws -> T {
    guard let ptr else {
        throw ScriviError(code: -1, message: "scrivi returned null")
    }
    let json = String(cString: ptr)
    scrivi_free(ptr)
    let data = Data(json.utf8)
    let envelope = try JSONDecoder().decode(Envelope<T>.self, from: data)
    if !envelope.ok {
        let e = envelope.error ?? ErrorPayload(code: -1, message: "unknown error")
        throw ScriviError(code: e.code, message: e.message)
    }
    guard let result = envelope.result else {
        throw ScriviError(code: -1, message: "ok=true but result missing")
    }
    return result
}

// MARK: — Swift result types

public struct IdentityResult: Decodable, Sendable {
    public let identityID:         String
    public let defaultPersonaID:   String
    public let displayName:        String
    public let createdNewIdentity: Bool
}

public struct AuthorshipRef: Sendable {
    public let identityID:  String
    public let personaID:   String
    public let displayName: String

    public init(identityID: String, personaID: String, displayName: String) {
        self.identityID  = identityID
        self.personaID   = personaID
        self.displayName = displayName
    }
}

public struct SceneRef: Decodable, Sendable {
    public let sceneID:      String
    public let metadataPath: String
    public let contentPath:  String
}

public struct CreateProjectResult: Decodable, Sendable {
    public let projectID:  String
    public let firstScene: SceneRef
}

public struct ActiveSceneResult: Decodable, Sendable {
    public let sceneID:      String
    public let metadataPath: String
    public let contentPath:  String
    public let markdown:     String
}

public struct SceneInfo: Decodable, Sendable {
    public let sceneID:              String
    public let chapterID:            String
    public let title:                String
    public let chapterTitle:         String
    public let slug:                 String
    public let metadataPath:         String
    public let contentPath:          String
    public let chapterMetadataPath:  String

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        sceneID             = try c.decode(String.self, forKey: .sceneID)
        chapterID           = try c.decode(String.self, forKey: .chapterID)
        title               = try c.decodeIfPresent(String.self, forKey: .title)               ?? ""
        chapterTitle        = try c.decodeIfPresent(String.self, forKey: .chapterTitle)        ?? ""
        slug                = try c.decodeIfPresent(String.self, forKey: .slug)                ?? ""
        metadataPath        = try c.decode(String.self, forKey: .metadataPath)
        contentPath         = try c.decode(String.self, forKey: .contentPath)
        chapterMetadataPath = try c.decodeIfPresent(String.self, forKey: .chapterMetadataPath) ?? ""
    }

    private enum CodingKeys: String, CodingKey {
        case sceneID, chapterID, title, chapterTitle, slug,
             metadataPath, contentPath, chapterMetadataPath
    }

    public init(
        sceneID: String, chapterID: String, title: String = "", chapterTitle: String = "",
        slug: String = "", metadataPath: String, contentPath: String, chapterMetadataPath: String = ""
    ) {
        self.sceneID             = sceneID
        self.chapterID           = chapterID
        self.title               = title
        self.chapterTitle        = chapterTitle
        self.slug                = slug
        self.metadataPath        = metadataPath
        self.contentPath         = contentPath
        self.chapterMetadataPath = chapterMetadataPath
    }
}

public struct OpenProjectResult: Decodable, Sendable {
    public let projectID:    String
    public let mode:         String
    public let activeScene:  ActiveSceneResult?
    public let scenes:       [SceneInfo]
    public let repairIssues: [RepairIssueResult]

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        projectID    = try c.decode(String.self, forKey: .projectID)
        mode         = try c.decodeIfPresent(String.self, forKey: .mode) ?? "ready"
        activeScene  = try c.decodeIfPresent(ActiveSceneResult.self, forKey: .activeScene)
        scenes       = try c.decodeIfPresent([SceneInfo].self, forKey: .scenes) ?? []
        repairIssues = try c.decodeIfPresent([RepairIssueResult].self, forKey: .repairIssues) ?? []
    }

    private enum CodingKeys: String, CodingKey {
        case projectID, mode, activeScene, scenes, repairIssues
    }
}

public struct OpenSceneResult: Decodable, Sendable {
    public let scene:    SceneInfo
    public let markdown: String
}

public struct SaveSceneResult: Decodable, Sendable {
    public let sceneID:   String
    public let saved:     Bool
    public let wordCount: Int
}

public struct ScanResult: Decodable, Sendable {
    public let projectID:               String
    public let indexesDirty:            Bool
    public let gitStatusChecked:        Bool
    public let hasUnsnapshottedChanges: Bool
    public let repairIssues:            [RepairIssueResult]

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        projectID               = try c.decode(String.self, forKey: .projectID)
        indexesDirty            = try c.decode(Bool.self,   forKey: .indexesDirty)
        gitStatusChecked        = try c.decode(Bool.self,   forKey: .gitStatusChecked)
        hasUnsnapshottedChanges = try c.decode(Bool.self,   forKey: .hasUnsnapshottedChanges)
        repairIssues            = try c.decodeIfPresent([RepairIssueResult].self, forKey: .repairIssues) ?? []
    }

    private enum CodingKeys: String, CodingKey {
        case projectID, indexesDirty, gitStatusChecked, hasUnsnapshottedChanges, repairIssues
    }
}

public struct RepairActionResult: Decodable, Sendable {
    public let kind:   String
    public let label:  String
    public let detail: String
}

public struct RepairIssueResult: Decodable, Sendable {
    public let issueID:          String
    public let severity:         String
    public let category:         String
    public let title:            String
    public let message:          String
    public let path:             String
    public let relatedPath:      String
    public let projectID:        String
    public let chapterID:        String
    public let sceneID:          String
    public let suggestedActions: [RepairActionResult]

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        issueID          = try c.decode(String.self, forKey: .issueID)
        severity         = try c.decode(String.self, forKey: .severity)
        category         = try c.decode(String.self, forKey: .category)
        title            = try c.decode(String.self, forKey: .title)
        message          = try c.decode(String.self, forKey: .message)
        path             = try c.decodeIfPresent(String.self, forKey: .path)        ?? ""
        relatedPath      = try c.decodeIfPresent(String.self, forKey: .relatedPath) ?? ""
        projectID        = try c.decodeIfPresent(String.self, forKey: .projectID)   ?? ""
        chapterID        = try c.decodeIfPresent(String.self, forKey: .chapterID)   ?? ""
        sceneID          = try c.decodeIfPresent(String.self, forKey: .sceneID)     ?? ""
        suggestedActions = try c.decodeIfPresent([RepairActionResult].self, forKey: .suggestedActions) ?? []
    }

    private enum CodingKeys: String, CodingKey {
        case issueID, severity, category, title, message, path, relatedPath,
             projectID, chapterID, sceneID, suggestedActions
    }
}

public struct ApplyRepairResult: Decodable, Sendable {
    public let issueID:       String
    public let actionApplied: String
    public let resolved:      Bool
    public let detail:        String
    public let warnings:      [RepairIssueResult]

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        issueID       = try c.decode(String.self, forKey: .issueID)
        actionApplied = try c.decode(String.self, forKey: .actionApplied)
        resolved      = try c.decode(Bool.self,   forKey: .resolved)
        detail        = try c.decodeIfPresent(String.self, forKey: .detail) ?? ""
        warnings      = try c.decodeIfPresent([RepairIssueResult].self, forKey: .warnings) ?? []
    }

    private enum CodingKeys: String, CodingKey {
        case issueID, actionApplied, resolved, detail, warnings
    }
}

public struct EnableGitResult: Decodable, Sendable {
    public let gitInitialized:    Bool
    public let alreadyRepository: Bool
    public let initialSnapshotID: String
    public let initialCommitID:   String
}

public struct CreateSnapshotResult: Decodable, Sendable {
    public let snapshotID: String
    public let commitID:   String
    public let createdAt:  String
    public let created:    Bool
}

public struct CreateObjectResult: Decodable, Sendable {
    public let objectID: String
    public let slug:     String
    public let path:     String
}

public struct OpenObjectResult: Decodable, Sendable {
    public let objectJson: String
    public let path:       String
}

public struct SaveObjectResult: Decodable, Sendable {
    public let objectID: String
    public let saved:    Bool
}

public struct DeleteObjectResult: Decodable, Sendable {
    public let objectID: String
    public let deleted:  Bool
}

public struct ImportAssetResult: Decodable, Sendable {
    public let assetID:     String
    public let assetPath:   String
    public let sidecarPath: String
}

public struct ListAssetsResult: Decodable, Sendable {
    public let count:  Int
    public let assets: String

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        count  = try c.decode(Int.self, forKey: .count)
        assets = try c.decodeIfPresent(String.self, forKey: .assets) ?? "[]"
    }

    private enum CodingKeys: String, CodingKey { case count, assets }
}

public struct RemoveAssetResult: Decodable, Sendable {
    public let assetID: String
    public let deleted: Bool
}

public struct AddCommentResult: Decodable, Sendable {
    public let commentID: String
    public let added:     Bool
}

public struct ListCommentsResult: Decodable, Sendable {
    public let scopeKind: String
    public let targetID:  String
    public let count:     Int
}

public struct ResolveCommentResult: Decodable, Sendable {
    public let commentID: String
    public let resolved:  Bool
}

public struct ListInboxResult: Decodable, Sendable {
    public let count:     Int
    public let filenames: String

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        count     = try c.decode(Int.self, forKey: .count)
        filenames = try c.decodeIfPresent(String.self, forKey: .filenames) ?? "[]"
    }

    private enum CodingKeys: String, CodingKey { case count, filenames }
}

public struct ImportFromInboxResult: Decodable, Sendable {
    public let actionTaken: String
    public let resultPath:  String
    public let assetID:     String
}

public struct CreateSceneResult: Decodable, Sendable {
    public let sceneID:      String
    public let chapterID:    String
    public let metadataPath: String
    public let contentPath:  String
}

public struct CreateChapterResult: Decodable, Sendable {
    public let chapterID:              String
    public let chapterMetadataPath:    String
    public let firstSceneID:           String
    public let firstSceneMetadataPath: String
    public let firstSceneContentPath:  String
}

public struct DeleteSceneResult: Decodable, Sendable {
    public let sceneID:  String
    public let deleted:  Bool
}

public struct DeleteChapterResult: Decodable, Sendable {
    public let chapterID:     String
    public let scenesDeleted: Int
    public let deleted:       Bool
}

public struct RenameSceneResult: Decodable, Sendable {
    public let metadataPath: String
    public let newTitle:     String
    public let renamed:      Bool
}

public struct RenameChapterResult: Decodable, Sendable {
    public let metadataPath: String
    public let newTitle:     String
    public let renamed:      Bool
}

public struct ReorderSceneResult: Decodable, Sendable {
    public let sceneID:         String
    public let sourceChapterID: String
    public let targetChapterID: String
    public let reordered:       Bool
}

public struct ReorderChapterResult: Decodable, Sendable {
    public let chapterID: String
    public let reordered: Bool
}

// MARK: — Timeline Result Types (EP-016 SP-039)

public struct GetTimelineResult: Decodable, Sendable {
    public let timelineID: String
    public let epochLabel: String
    public let projectID:  String
    public let createdAt:  String
}

public struct SceneStoryTimeResult: Decodable, Sendable {
    public let sceneID:             String
    public let offsetMs:            Int64
    public let offsetSource:        String
    public let gapMs:               Int64
    public let durationMs:          Int64
    public let durationSource:      String
    public let inferenceHint:       String
    public let inferenceConfidence: Double
    public let bandID:              String
    public let bandAssignedAt:      String
}

public struct StoryStructureResult: Decodable, Sendable {
    public let hasStructure:   Bool
    public let structureID:    String
    public let bandLayoutJSON: String
}

public struct HistoricalEventResult: Decodable, Sendable {
    public let eventID: String
    public let slug:    String
}

public struct HistoricalEventsListResult: Decodable, Sendable {
    public let count:      Int
    public let eventsJSON: String
}

public struct ImportedTimelinesListResult: Decodable, Sendable {
    public let count:         Int
    public let timelinesJSON: String
}

public struct ExportTimelineResult: Decodable, Sendable {
    public let timelineJSON: String
}

public struct TimelineBoolResult: Decodable, Sendable {
    public let updated: Bool
}

// ScriviError, Envelope, ErrorPayload are in ScriviError.swift.
