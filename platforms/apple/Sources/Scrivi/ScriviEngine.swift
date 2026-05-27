import Foundation
import ScriviCoreAdapter

// ScriviEngine is the single Swift-side entry point to the C++ ScriviCore backend.
//
// It owns the ScriviAdapter reference type (ARC-managed via import_reference retain/release).
// Each method converts Swift String parameters to const char* via withCString, receives a
// std::string JSON result by value, decodes the envelope, and returns a Swift result struct.
// No backend logic lives here — only type conversion and JSON decoding.

public final class ScriviEngine: @unchecked Sendable {

    private let adapter: scrivi.apple.ScriviAdapter

    public init() {
        adapter = scrivi.apple.ScriviAdapter.create()
    }

    // MARK: — ensureLocalIdentity

    public func ensureLocalIdentity(
        displayName: String,
        appSupportRoot: String
    ) throws -> IdentityResult {
        let json = displayName.withCString { dn in
            appSupportRoot.withCString { asr in
                adapter.ensureLocalIdentity(dn, asr)
            }
        }
        return try decode(json)
    }

    // MARK: — createProject

    public func createProject(
        projectRootPath: String,
        appSupportRoot: String,
        title: String,
        slug: String,
        authorshipRef: AuthorshipRef
    ) throws -> CreateProjectResult {
        let json = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                title.withCString { t in
                    slug.withCString { s in
                        authorshipRef.identityID.withCString { iid in
                            authorshipRef.personaID.withCString { pid in
                                authorshipRef.displayName.withCString { dn in
                                    adapter.createProject(prp, asr, t, s, iid, pid, dn)
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decode(json)
    }

    // MARK: — openProject

    public func openProject(
        projectRootPath: String,
        appSupportRoot: String,
        identityID: String = ""
    ) throws -> OpenProjectResult {
        let json = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                identityID.withCString { iid in
                    adapter.openProject(prp, asr, iid)
                }
            }
        }
        return try decode(json)
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
        let json = projectID.withCString { pid in
            projectRootPath.withCString { prp in
                appSupportRoot.withCString { asr in
                    sceneID.withCString { sid in
                        sceneMetadataPath.withCString { smp in
                            sceneContentPath.withCString { scp in
                                markdown.withCString { md in
                                    authorshipRef.identityID.withCString { iid in
                                        authorshipRef.personaID.withCString { perid in
                                            authorshipRef.displayName.withCString { dn in
                                                adapter.saveScene(
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
        return try decode(json)
    }

    // MARK: — scanForExternalChanges

    public func scanForExternalChanges(
        projectRootPath: String,
        appSupportRoot: String,
        includeGitStatus: Bool = true
    ) throws -> ScanResult {
        let json = projectRootPath.withCString { prp in
            appSupportRoot.withCString { asr in
                adapter.scanForExternalChanges(prp, asr, includeGitStatus)
            }
        }
        return try decode(json)
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
        let json = issueID.withCString { iid in
            projectRootPath.withCString { prp in
                appSupportRoot.withCString { asr in
                    actionKind.withCString { ak in
                        targetPath.withCString { tp in
                            authorshipRef.identityID.withCString { identID in
                                authorshipRef.personaID.withCString { pid in
                                    authorshipRef.displayName.withCString { dn in
                                        adapter.applyRepair(iid, prp, asr, ak, tp, identID, pid, dn)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return try decode(json)
    }

    // MARK: — enableGitSnapshots

    public func enableGitSnapshots(
        projectRootPath: String,
        authorshipRef: AuthorshipRef,
        initialSnapshotLabel: String = "Initial project"
    ) throws -> EnableGitResult {
        let json = projectRootPath.withCString { prp in
            authorshipRef.identityID.withCString { iid in
                authorshipRef.personaID.withCString { pid in
                    authorshipRef.displayName.withCString { dn in
                        initialSnapshotLabel.withCString { label in
                            adapter.enableGitSnapshots(prp, iid, pid, dn, label)
                        }
                    }
                }
            }
        }
        return try decode(json)
    }

    // MARK: — createSnapshot

    public func createSnapshot(
        projectRootPath: String,
        authorshipRef: AuthorshipRef,
        label: String,
        note: String = ""
    ) throws -> CreateSnapshotResult {
        let json = projectRootPath.withCString { prp in
            authorshipRef.identityID.withCString { iid in
                authorshipRef.personaID.withCString { pid in
                    authorshipRef.displayName.withCString { dn in
                        label.withCString { lbl in
                            note.withCString { n in
                                adapter.createSnapshot(prp, iid, pid, dn, lbl, n)
                            }
                        }
                    }
                }
            }
        }
        return try decode(json)
    }
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

public struct OpenProjectResult: Decodable, Sendable {
    public let projectID:    String
    public let mode:         String          // "ready" | "repairRequired"
    public let activeScene:  ActiveSceneResult?
    public let repairIssues: [RepairIssueResult]

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        projectID    = try c.decode(String.self, forKey: .projectID)
        mode         = try c.decodeIfPresent(String.self, forKey: .mode) ?? "ready"
        activeScene  = try c.decodeIfPresent(ActiveSceneResult.self, forKey: .activeScene)
        repairIssues = try c.decodeIfPresent([RepairIssueResult].self, forKey: .repairIssues) ?? []
    }

    private enum CodingKeys: String, CodingKey {
        case projectID, mode, activeScene, repairIssues
    }
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

// ScriviError, Envelope, ErrorPayload, and decode() are in ScriviError.swift.
