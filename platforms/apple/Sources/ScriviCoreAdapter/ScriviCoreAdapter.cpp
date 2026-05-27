#include "ScriviCoreAdapter.hpp"

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"
#include "git/SystemGitProvider.hpp"
#include "platform/LocalFileSystem.hpp"
#include "platform/SystemUUIDProvider.hpp"
#include "schemas/RepairIssueJson.hpp"
#include "util/Json.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Global retain/release hooks required by Swift's import_reference protocol.
// ---------------------------------------------------------------------------

void scriviAdapterRetain(scrivi::apple::ScriviAdapter* p) noexcept {
    if (p) p->retain();
}

void scriviAdapterRelease(scrivi::apple::ScriviAdapter* p) noexcept {
    if (p) p->release();
}

namespace scrivi::apple {

// ---------------------------------------------------------------------------
// Prototype service implementations — fully defined in .cpp, invisible to Swift.
// ---------------------------------------------------------------------------

struct PrototypeClock final : public scrivi::Clock {
    scrivi::ISO8601Timestamp nowUTC() override {
        auto now = std::chrono::system_clock::now();
        auto tt  = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        gmtime_s(&tm, &tt);
#else
        gmtime_r(&tt, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

struct PrototypeSecureStore final : public scrivi::SecureStore {
    std::unordered_map<std::string, scrivi::SecretBytes> store_;

    scrivi::Result<bool> containsSecret(std::string_view key) override {
        return scrivi::Result<bool>::success(store_.contains(std::string(key)));
    }

    scrivi::Result<void> putSecret(
            std::string_view key, const scrivi::SecretBytes& value) override {
        store_[std::string(key)] = value;
        return scrivi::Result<void>::success();
    }

    scrivi::Result<scrivi::SecretBytes> getSecret(std::string_view key) override {
        auto it = store_.find(std::string(key));
        if (it == store_.end()) {
            return scrivi::Result<scrivi::SecretBytes>::failure(
                {scrivi::ErrorCode::secureStoreError, "key not found"});
        }
        return scrivi::Result<scrivi::SecretBytes>::success(it->second);
    }
};

// ---------------------------------------------------------------------------
// Impl — owns all C++ service instances and the ScriviCore facade.
// ---------------------------------------------------------------------------

struct ScriviAdapter::Impl {
    scrivi::platform::LocalFileSystem    fileSystem;
    scrivi::platform::SystemUUIDProvider uuidProvider;
    PrototypeClock                       clock;
    PrototypeSecureStore                 secureStore;
    scrivi::git::SystemGitProvider       gitProvider;
    std::unique_ptr<scrivi::ScriviCore>  core;

    Impl() {
        scrivi::CoreServices svc;
        svc.fileSystem   = &fileSystem;
        svc.uuidProvider = &uuidProvider;
        svc.secureStore  = &secureStore;
        svc.clock        = &clock;
        svc.gitProvider  = &gitProvider;
        svc.logger       = nullptr;
        core = std::make_unique<scrivi::ScriviCore>(svc);
    }
};

// ---------------------------------------------------------------------------
// JSON envelope helpers
// ---------------------------------------------------------------------------

static std::string errorEnvelope(const scrivi::Error& e) {
    scrivi::util::JsonDoc err;
    err.setInt("code", static_cast<int>(e.code));
    err.setString("message", e.message);

    scrivi::util::JsonDoc root;
    root.setBool("ok", false);
    root.setSubDoc("error", std::move(err));
    return root.dump();
}

static std::string okEnvelope(scrivi::util::JsonDoc result) {
    scrivi::util::JsonDoc root;
    root.setBool("ok", true);
    root.setSubDoc("result", std::move(result));
    return root.dump();
}

// ---------------------------------------------------------------------------
// ScriviAdapter
// ---------------------------------------------------------------------------

ScriviAdapter* ScriviAdapter::create() {
    return new ScriviAdapter();
}

ScriviAdapter::ScriviAdapter() : impl_(std::make_unique<Impl>()) {}

ScriviAdapter::~ScriviAdapter() = default;

void ScriviAdapter::retain() noexcept {
    refCount_.fetch_add(1, std::memory_order_relaxed);
}

void ScriviAdapter::release() noexcept {
    if (refCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
    }
}

std::string ScriviAdapter::ensureLocalIdentity(
    const char* requestedDisplayName,
    const char* appSupportRoot)
{
    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = requestedDisplayName ? requestedDisplayName : "";
    req.appSupportRoot       = appSupportRoot       ? appSupportRoot       : "";

    auto result = impl_->core->ensureLocalIdentity(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();
    scrivi::util::JsonDoc doc;
    doc.setString("identityID",       v.identityID.value);
    doc.setString("defaultPersonaID", v.defaultPersonaID.value);
    doc.setString("displayName",      v.displayName);
    doc.setBool("createdNewIdentity", v.createdNewIdentity);
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::createProject(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* title,
    const char* slug,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectRootPath ? projectRootPath : "";
    req.appSupportRoot  = appSupportRoot  ? appSupportRoot  : "";
    req.title           = title           ? title           : "";
    req.slug            = slug            ? slug            : "";
    req.author = {
        scrivi::IdentityID{identityID  ? identityID  : ""},
        scrivi::PersonaID {personaID   ? personaID   : ""},
        displayName ? displayName : ""
    };

    auto result = impl_->core->createProject(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();
    scrivi::util::JsonDoc scene;
    scene.setString("sceneID",      v.firstSceneID.value);
    scene.setString("metadataPath", v.firstSceneMetadataPath);
    scene.setString("contentPath",  v.firstSceneContentPath);

    scrivi::util::JsonDoc doc;
    doc.setString("projectID", v.project.projectID.value);
    doc.setSubDoc("firstScene", std::move(scene));
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::openProject(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* identityID)
{
    scrivi::OpenProjectRequest req;
    req.projectRootPath = projectRootPath ? projectRootPath : "";
    req.appSupportRoot  = appSupportRoot  ? appSupportRoot  : "";
    if (identityID && identityID[0] != '\0')
        req.currentIdentityID = scrivi::IdentityID{identityID};

    auto result = impl_->core->openProject(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();

    // If the project cannot be opened at all, return an error envelope.
    if (v.mode == scrivi::OpenMode::cannotOpen) {
        scrivi::Error err;
        err.code    = scrivi::ErrorCode::repairRequired;
        err.message = v.repairIssues.empty()
            ? "Project cannot be opened"
            : v.repairIssues.front().title;
        return errorEnvelope(err);
    }

    // If repair is required, return an ok envelope that includes the full issues list.
    // Swift can inspect mode == "repairRequired" and present the repair UI.
    if (v.mode == scrivi::OpenMode::repairRequired) {
        scrivi::util::JsonDoc doc;
        doc.setString("projectID", v.project.projectID.value);
        doc.setString("mode", "repairRequired");
        scrivi::schemas::appendRepairIssuesToDoc(doc, "repairIssues", v.repairIssues);
        return okEnvelope(std::move(doc));
    }

    scrivi::util::JsonDoc scene;
    if (v.activeScene.has_value()) {
        scene.setString("sceneID",      v.activeScene->sceneID.value);
        scene.setString("metadataPath", v.activeScene->metadataPath);
        scene.setString("contentPath",  v.activeScene->contentPath);
    }
    scene.setString("markdown", v.activeSceneMarkdown);

    scrivi::util::JsonDoc doc;
    doc.setString("projectID", v.project.projectID.value);
    doc.setString("mode", "ready");
    doc.setSubDoc("activeScene", std::move(scene));
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::saveScene(
    const char* projectID,
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* sceneID,
    const char* sceneMetadataPath,
    const char* sceneContentPath,
    const char* markdown,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    scrivi::SaveSceneRequest req;
    req.projectID         = scrivi::ProjectID{projectID ? projectID : ""};
    req.projectRootPath   = projectRootPath   ? projectRootPath   : "";
    req.appSupportRoot    = appSupportRoot    ? appSupportRoot    : "";
    req.sceneID           = scrivi::SceneID{sceneID ? sceneID : ""};
    req.sceneMetadataPath = sceneMetadataPath ? sceneMetadataPath : "";
    req.sceneContentPath  = sceneContentPath  ? sceneContentPath  : "";
    req.markdown          = markdown          ? markdown          : "";
    req.author = {
        scrivi::IdentityID{identityID  ? identityID  : ""},
        scrivi::PersonaID {personaID   ? personaID   : ""},
        displayName ? displayName : ""
    };

    auto result = impl_->core->saveScene(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",   v.sceneID.value);
    doc.setBool("saved",       v.saved);
    doc.setInt("wordCount",    static_cast<int>(v.wordCount));
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::scanForExternalChanges(
    const char* projectRootPath,
    const char* appSupportRoot,
    bool        includeGitStatus)
{
    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectRootPath ? projectRootPath : "";
    req.appSupportRoot   = appSupportRoot  ? appSupportRoot  : "";
    req.includeGitStatus = includeGitStatus;

    auto result = impl_->core->scanForExternalChanges(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();
    scrivi::util::JsonDoc doc;
    doc.setString("projectID",               v.projectID.value);
    doc.setBool("indexesDirty",              v.indexesDirty);
    doc.setBool("gitStatusChecked",          v.gitStatusChecked);
    doc.setBool("hasUnsnapshottedChanges",   v.hasUnsnapshottedChanges);
    scrivi::schemas::appendRepairIssuesToDoc(doc, "repairIssues", v.repairIssues);
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::applyRepair(
    const char* issueID,
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* actionKind,
    const char* targetPath,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    // Decode the actionKind string via the schema module's converter.
    // We reproduce the lookup here to keep the adapter self-contained.
    static const auto kindFromStr = [](std::string_view s) -> scrivi::RepairActionKind {
        using K = scrivi::RepairActionKind;
        if (s == "relinkToFile")           return K::relinkToFile;
        if (s == "createEmptyContentFile") return K::createEmptyContentFile;
        if (s == "markMissing")            return K::markMissing;
        if (s == "removeFromProject")      return K::removeFromProject;
        if (s == "moveToInbox")            return K::moveToInbox;
        if (s == "reloadExternalVersion")  return K::reloadExternalVersion;
        if (s == "regenerateMetadata")     return K::regenerateMetadata;
        if (s == "importAsNewScene")       return K::importAsNewScene;
        if (s == "attachToExistingScene")  return K::attachToExistingScene;
        if (s == "restoreFromSnapshot")    return K::restoreFromSnapshot;
        if (s == "keepCurrentVersion")     return K::keepCurrentVersion;
        if (s == "saveCurrentVersionAsCopy") return K::saveCurrentVersionAsCopy;
        if (s == "ignore")                 return K::ignore;
        if (s == "deleteAfterConfirmation") return K::deleteAfterConfirmation;
        if (s == "openReadOnly")           return K::openReadOnly;
        if (s == "cancelOpen")             return K::cancelOpen;
        return K::none;
    };

    scrivi::ApplyRepairRequest req;
    req.issueID         = issueID        ? issueID        : "";
    req.projectRootPath = projectRootPath ? projectRootPath : "";
    req.appSupportRoot  = appSupportRoot  ? appSupportRoot  : "";
    req.actionKind      = kindFromStr(actionKind ? actionKind : "");
    req.targetPath      = targetPath     ? targetPath     : "";
    req.author = {
        scrivi::IdentityID{identityID  ? identityID  : ""},
        scrivi::PersonaID {personaID   ? personaID   : ""},
        displayName ? displayName : ""
    };

    auto result = impl_->core->applyRepair(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();

    // Encode RepairActionKind back to string.
    static const auto kindToStr = [](scrivi::RepairActionKind k) -> std::string_view {
        using K = scrivi::RepairActionKind;
        switch (k) {
            case K::relinkToFile:           return "relinkToFile";
            case K::createEmptyContentFile: return "createEmptyContentFile";
            case K::markMissing:            return "markMissing";
            case K::removeFromProject:      return "removeFromProject";
            case K::moveToInbox:            return "moveToInbox";
            case K::reloadExternalVersion:  return "reloadExternalVersion";
            case K::regenerateMetadata:     return "regenerateMetadata";
            case K::importAsNewScene:       return "importAsNewScene";
            case K::attachToExistingScene:  return "attachToExistingScene";
            case K::restoreFromSnapshot:    return "restoreFromSnapshot";
            case K::keepCurrentVersion:     return "keepCurrentVersion";
            case K::saveCurrentVersionAsCopy: return "saveCurrentVersionAsCopy";
            case K::ignore:                 return "ignore";
            case K::deleteAfterConfirmation: return "deleteAfterConfirmation";
            case K::openReadOnly:           return "openReadOnly";
            case K::cancelOpen:             return "cancelOpen";
            case K::none:                   return "none";
        }
        return "none";
    };

    scrivi::util::JsonDoc doc;
    doc.setString("issueID",       v.issueID);
    doc.setString("actionApplied", std::string(kindToStr(v.actionApplied)));
    doc.setBool("resolved",        v.resolved);
    doc.setString("detail",        v.detail);
    scrivi::schemas::appendRepairIssuesToDoc(doc, "warnings", v.warnings);
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::enableGitSnapshots(
    const char* projectRootPath,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* initialSnapshotLabel)
{
    scrivi::EnableGitRequest req;
    req.projectRootPath       = projectRootPath       ? projectRootPath       : "";
    req.initialSnapshotLabel  = initialSnapshotLabel  ? initialSnapshotLabel  : "Initial project";
    req.author = {
        scrivi::IdentityID{identityID  ? identityID  : ""},
        scrivi::PersonaID {personaID   ? personaID   : ""},
        displayName ? displayName : ""
    };

    auto result = impl_->core->enableGitSnapshots(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();
    scrivi::util::JsonDoc doc;
    doc.setBool("gitInitialized",    v.gitInitialized);
    doc.setBool("alreadyRepository", v.alreadyRepository);
    doc.setString("initialSnapshotID", v.initialSnapshotID.value);
    doc.setString("initialCommitID",   v.initialCommitID.value);
    return okEnvelope(std::move(doc));
}

std::string ScriviAdapter::createSnapshot(
    const char* projectRootPath,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* label,
    const char* note)
{
    scrivi::CreateSnapshotRequest req;
    req.projectRootPath = projectRootPath ? projectRootPath : "";
    req.label           = label           ? label           : "";
    req.note            = note            ? note            : "";
    req.author = {
        scrivi::IdentityID{identityID  ? identityID  : ""},
        scrivi::PersonaID {personaID   ? personaID   : ""},
        displayName ? displayName : ""
    };

    auto result = impl_->core->createSnapshot(req);
    if (!result.ok()) return errorEnvelope(result.error());

    const auto& v = result.value();
    scrivi::util::JsonDoc doc;
    doc.setString("snapshotID", v.snapshotID.value);
    doc.setString("commitID",   v.commitID.value);
    doc.setString("createdAt",  v.createdAt);
    doc.setBool("created",      v.created);
    return okEnvelope(std::move(doc));
}

} // namespace scrivi::apple
