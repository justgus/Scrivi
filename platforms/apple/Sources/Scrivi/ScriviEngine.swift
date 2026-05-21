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
    public let projectID:   String
    public let activeScene: ActiveSceneResult
}

public struct SaveSceneResult: Decodable, Sendable {
    public let sceneID:   String
    public let saved:     Bool
    public let wordCount: Int
}

// MARK: — Error type

public struct ScriviError: Error, Sendable {
    public let code:    Int
    public let message: String
}

// MARK: — JSON envelope decoding

private struct Envelope<T: Decodable>: Decodable {
    let ok:     Bool
    let result: T?
    let error:  ErrorPayload?
}

private struct ErrorPayload: Decodable {
    let code:    Int
    let message: String
}

private func decode<T: Decodable>(_ cxxString: std.string) throws -> T {
    let json = String(cxxString)
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
