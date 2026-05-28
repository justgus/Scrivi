#pragma once

// ScriviCoreAdapter — thin C++ shim between ScriviCore's C++23 API and Swift/C++ direct interop.
//
// Design: all methods return std::string by value, containing a JSON envelope:
//   {"ok":true,"result":{...}} or {"ok":false,"error":{"code":N,"message":"..."}}
//
// Swift receives the std::string as a value type copy (String(cxxString)),
// then deserializes with Codable. No raw pointers cross the boundary as return values.
//
// See: docs/Scrivi_Apple_Wrapper_Design_v0_1.md

#include <atomic>
#include <memory>
#include <string>

// Retain/release hooks required by Swift's import_reference semantics.
namespace scrivi::apple {
    class ScriviAdapter;
}
void scriviAdapterRetain(scrivi::apple::ScriviAdapter* p) noexcept;
void scriviAdapterRelease(scrivi::apple::ScriviAdapter* p) noexcept;

namespace scrivi::apple {

class __attribute__((swift_attr("import_reference")))
      __attribute__((swift_attr("retain:scriviAdapterRetain")))
      __attribute__((swift_attr("release:scriviAdapterRelease")))
ScriviAdapter {
public:
    // Factory — creates with ref count 1. Swift will call Release when done.
    static ScriviAdapter* create();

    void retain()  noexcept;
    void release() noexcept;

    std::string ensureLocalIdentity(
        const char* requestedDisplayName,
        const char* appSupportRoot);

    std::string createProject(
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* title,
        const char* slug,
        const char* identityID,
        const char* personaID,
        const char* displayName);

    std::string openProject(
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* identityID);

    std::string saveScene(
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

    std::string scanForExternalChanges(
        const char* projectRootPath,
        const char* appSupportRoot,
        bool        includeGitStatus);

    std::string applyRepair(
        const char* issueID,
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* actionKind,
        const char* targetPath,
        const char* identityID,
        const char* personaID,
        const char* displayName);

    std::string enableGitSnapshots(
        const char* projectRootPath,
        const char* identityID,
        const char* personaID,
        const char* displayName,
        const char* initialSnapshotLabel);

    std::string createSnapshot(
        const char* projectRootPath,
        const char* identityID,
        const char* personaID,
        const char* displayName,
        const char* label,
        const char* note);

    // EP-005 — Object CRUD (all types)
    std::string createObject(
        const char* projectRootPath,
        const char* objectKind,
        const char* displayName,
        const char* slug,
        const char* identityID,
        const char* personaID,
        const char* authorDisplayName);

    std::string openObject(
        const char* projectRootPath,
        const char* objectKind,
        const char* objectID);

    std::string saveObject(
        const char* projectRootPath,
        const char* objectKind,
        const char* objectJson,  // full serialized WorldObject as JSON string
        const char* identityID,
        const char* personaID,
        const char* authorDisplayName);

    std::string deleteObject(
        const char* projectRootPath,
        const char* objectKind,
        const char* objectID);

    // EP-005 — Assets
    std::string importAsset(
        const char* projectRootPath,
        const char* sourcePath,
        const char* category,
        const char* title,
        const char* identityID,
        const char* personaID,
        const char* authorDisplayName);

    std::string listAssets(
        const char* projectRootPath,
        const char* category);  // empty string = all categories

    std::string removeAsset(
        const char* projectRootPath,
        const char* assetID);

    // EP-005 — Comments
    std::string addComment(
        const char* projectRootPath,
        const char* scopeKind,
        const char* targetID,
        const char* body,
        const char* identityID,
        const char* personaID,
        const char* authorDisplayName);

    std::string listComments(
        const char* projectRootPath,
        const char* scopeKind,
        const char* targetID);

    std::string resolveComment(
        const char* projectRootPath,
        const char* scopeKind,
        const char* targetID,
        const char* commentID,
        const char* identityID,
        const char* personaID,
        const char* resolverDisplayName);

    // EP-005 — Inbox
    std::string listInbox(
        const char* projectRootPath);

    std::string importFromInbox(
        const char* projectRootPath,
        const char* filename,
        const char* action,      // "importAsAsset" | "ignore" | "deleteFile"
        const char* category,    // asset category, used when action == "importAsAsset"
        const char* identityID,
        const char* personaID,
        const char* authorDisplayName);

private:
    ScriviAdapter();
    ~ScriviAdapter();

    ScriviAdapter(const ScriviAdapter&) = delete;
    ScriviAdapter& operator=(const ScriviAdapter&) = delete;

    std::atomic<int> refCount_{1};

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace scrivi::apple
