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
