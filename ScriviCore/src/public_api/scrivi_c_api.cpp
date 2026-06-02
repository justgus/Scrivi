#include "scrivi/scrivi.h"

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"
#include "git/SystemGitProvider.hpp"
#include "platform/LocalFileSystem.hpp"
#include "platform/SystemUUIDProvider.hpp"
#include "schemas/RepairIssueJson.hpp"
#include "schemas/ObjectJson.hpp"
#include "util/Json.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Service implementations
// ---------------------------------------------------------------------------

namespace {

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
        if (it == store_.end())
            return scrivi::Result<scrivi::SecretBytes>::failure(
                {scrivi::ErrorCode::secureStoreError, "key not found"});
        return scrivi::Result<scrivi::SecretBytes>::success(it->second);
    }
};

// ---------------------------------------------------------------------------
// Singleton core — created once, lives for the process lifetime.
// ---------------------------------------------------------------------------

struct CoreSingleton {
    scrivi::platform::LocalFileSystem    fileSystem;
    scrivi::platform::SystemUUIDProvider uuidProvider;
    PrototypeClock                       clock;
    PrototypeSecureStore                 secureStore;
    scrivi::git::SystemGitProvider       gitProvider;
    std::unique_ptr<scrivi::ScriviCore>  core;

    CoreSingleton() {
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

scrivi::ScriviCore& core() {
    static CoreSingleton s;
    return *s.core;
}

// ---------------------------------------------------------------------------
// JSON envelope helpers
// ---------------------------------------------------------------------------

static std::string errorEnvelope(const scrivi::Error& e) {
    scrivi::util::JsonDoc err;
    err.setInt("code",    static_cast<int>(e.code));
    err.setString("message", e.message);
    scrivi::util::JsonDoc root;
    root.setBool("ok",    false);
    root.setSubDoc("error", std::move(err));
    return root.dump();
}

static std::string okEnvelope(scrivi::util::JsonDoc result) {
    scrivi::util::JsonDoc root;
    root.setBool("ok", true);
    root.setSubDoc("result", std::move(result));
    return root.dump();
}

static const char* heap(std::string s) {
    char* p = static_cast<char*>(std::malloc(s.size() + 1));
    std::memcpy(p, s.data(), s.size() + 1);
    return p;
}

static const char* S(const char* p) { return p ? p : ""; }

static scrivi::RepairActionKind repairKindFromStr(std::string_view s) {
    using K = scrivi::RepairActionKind;
    if (s == "relinkToFile")             return K::relinkToFile;
    if (s == "createEmptyContentFile")   return K::createEmptyContentFile;
    if (s == "markMissing")              return K::markMissing;
    if (s == "removeFromProject")        return K::removeFromProject;
    if (s == "moveToInbox")              return K::moveToInbox;
    if (s == "reloadExternalVersion")    return K::reloadExternalVersion;
    if (s == "regenerateMetadata")       return K::regenerateMetadata;
    if (s == "importAsNewScene")         return K::importAsNewScene;
    if (s == "attachToExistingScene")    return K::attachToExistingScene;
    if (s == "restoreFromSnapshot")      return K::restoreFromSnapshot;
    if (s == "keepCurrentVersion")       return K::keepCurrentVersion;
    if (s == "saveCurrentVersionAsCopy") return K::saveCurrentVersionAsCopy;
    if (s == "ignore")                   return K::ignore;
    if (s == "deleteAfterConfirmation")  return K::deleteAfterConfirmation;
    if (s == "openReadOnly")             return K::openReadOnly;
    if (s == "cancelOpen")              return K::cancelOpen;
    return K::none;
}

static std::string_view repairKindToStr(scrivi::RepairActionKind k) {
    using K = scrivi::RepairActionKind;
    switch (k) {
        case K::relinkToFile:             return "relinkToFile";
        case K::createEmptyContentFile:   return "createEmptyContentFile";
        case K::markMissing:              return "markMissing";
        case K::removeFromProject:        return "removeFromProject";
        case K::moveToInbox:              return "moveToInbox";
        case K::reloadExternalVersion:    return "reloadExternalVersion";
        case K::regenerateMetadata:       return "regenerateMetadata";
        case K::importAsNewScene:         return "importAsNewScene";
        case K::attachToExistingScene:    return "attachToExistingScene";
        case K::restoreFromSnapshot:      return "restoreFromSnapshot";
        case K::keepCurrentVersion:       return "keepCurrentVersion";
        case K::saveCurrentVersionAsCopy: return "saveCurrentVersionAsCopy";
        case K::ignore:                   return "ignore";
        case K::deleteAfterConfirmation:  return "deleteAfterConfirmation";
        case K::openReadOnly:             return "openReadOnly";
        case K::cancelOpen:               return "cancelOpen";
        case K::none:                     return "none";
    }
    return "none";
}

static scrivi::ObjectKind objectKindFromStr(std::string_view s) {
    if (s == "location") return scrivi::ObjectKind::location;
    if (s == "item")     return scrivi::ObjectKind::item;
    if (s == "rule")     return scrivi::ObjectKind::rule;
    if (s == "timeline") return scrivi::ObjectKind::timeline;
    return scrivi::ObjectKind::character;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// C API implementation
// ---------------------------------------------------------------------------

extern "C" {

void scrivi_free(const char* json) {
    std::free(const_cast<char*>(json));
}

const char* scrivi_ensure_local_identity(
    const char* displayName,
    const char* appSupportRoot)
{
    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = S(displayName);
    req.appSupportRoot       = S(appSupportRoot);

    auto r = core().ensureLocalIdentity(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("identityID",       v.identityID.value);
    doc.setString("defaultPersonaID", v.defaultPersonaID.value);
    doc.setString("displayName",      v.displayName);
    doc.setBool("createdNewIdentity", v.createdNewIdentity);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_create_project(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* title,
    const char* slug,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    scrivi::CreateProjectRequest req;
    req.projectRootPath = S(projectRootPath);
    req.appSupportRoot  = S(appSupportRoot);
    req.title           = S(title);
    req.slug            = S(slug);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().createProject(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc scene;
    scene.setString("sceneID",      v.firstSceneID.value);
    scene.setString("metadataPath", v.firstSceneMetadataPath);
    scene.setString("contentPath",  v.firstSceneContentPath);
    scrivi::util::JsonDoc doc;
    doc.setString("projectID", v.project.projectID.value);
    doc.setSubDoc("firstScene", std::move(scene));
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_open_project(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* identityID)
{
    scrivi::OpenProjectRequest req;
    req.projectRootPath = S(projectRootPath);
    req.appSupportRoot  = S(appSupportRoot);
    if (identityID && identityID[0] != '\0')
        req.currentIdentityID = scrivi::IdentityID{identityID};

    auto r = core().openProject(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();

    if (v.mode == scrivi::OpenMode::cannotOpen) {
        scrivi::Error err;
        err.code    = scrivi::ErrorCode::repairRequired;
        err.message = v.repairIssues.empty()
            ? "Project cannot be opened"
            : v.repairIssues.front().title;
        return heap(errorEnvelope(err));
    }

    if (v.mode == scrivi::OpenMode::repairRequired) {
        scrivi::util::JsonDoc doc;
        doc.setString("projectID", v.project.projectID.value);
        doc.setString("mode", "repairRequired");
        scrivi::schemas::appendRepairIssuesToDoc(doc, "repairIssues", v.repairIssues);
        return heap(okEnvelope(std::move(doc)));
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
    for (auto& s : v.scenes) {
        scrivi::util::JsonDoc entry;
        entry.setString("sceneID",      s.sceneID.value);
        entry.setString("chapterID",    s.chapterID.value);
        entry.setString("title",        s.title);
        entry.setString("slug",         s.slug);
        entry.setString("metadataPath", s.metadataPath);
        entry.setString("contentPath",  s.contentPath);
        doc.appendToArray("scenes", std::move(entry));
    }
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_open_scene(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* sceneID)
{
    scrivi::OpenSceneRequest req;
    req.projectRootPath = S(projectRootPath);
    req.appSupportRoot  = S(appSupportRoot);
    req.projectID       = scrivi::ProjectID{S(projectID)};
    req.sceneID         = scrivi::SceneID  {S(sceneID)};

    auto r = core().openScene(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc scene;
    scene.setString("sceneID",      v.scene.sceneID.value);
    scene.setString("chapterID",    v.scene.chapterID.value);
    scene.setString("title",        v.scene.title);
    scene.setString("slug",         v.scene.slug);
    scene.setString("metadataPath", v.scene.metadataPath);
    scene.setString("contentPath",  v.scene.contentPath);
    scrivi::util::JsonDoc doc;
    doc.setSubDoc("scene",    std::move(scene));
    doc.setString("markdown", v.markdown);
    return heap(okEnvelope(std::move(doc)));
}

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
    const char* displayName)
{
    scrivi::SaveSceneRequest req;
    req.projectID         = scrivi::ProjectID{S(projectID)};
    req.projectRootPath   = S(projectRootPath);
    req.appSupportRoot    = S(appSupportRoot);
    req.sceneID           = scrivi::SceneID{S(sceneID)};
    req.sceneMetadataPath = S(sceneMetadataPath);
    req.sceneContentPath  = S(sceneContentPath);
    req.markdown          = S(markdown);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().saveScene(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",  v.sceneID.value);
    doc.setBool("saved",      v.saved);
    doc.setInt("wordCount",   static_cast<int>(v.wordCount));
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_scan_for_external_changes(
    const char* projectRootPath,
    const char* appSupportRoot,
    int         includeGitStatus)
{
    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = S(projectRootPath);
    req.appSupportRoot   = S(appSupportRoot);
    req.includeGitStatus = includeGitStatus != 0;

    auto r = core().scanForExternalChanges(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("projectID",             v.projectID.value);
    doc.setBool("indexesDirty",            v.indexesDirty);
    doc.setBool("gitStatusChecked",        v.gitStatusChecked);
    doc.setBool("hasUnsnapshottedChanges", v.hasUnsnapshottedChanges);
    scrivi::schemas::appendRepairIssuesToDoc(doc, "repairIssues", v.repairIssues);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_apply_repair(
    const char* issueID,
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* actionKind,
    const char* targetPath,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    scrivi::ApplyRepairRequest req;
    req.issueID         = S(issueID);
    req.projectRootPath = S(projectRootPath);
    req.appSupportRoot  = S(appSupportRoot);
    req.actionKind      = repairKindFromStr(S(actionKind));
    req.targetPath      = S(targetPath);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().applyRepair(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("issueID",       v.issueID);
    doc.setString("actionApplied", std::string(repairKindToStr(v.actionApplied)));
    doc.setBool("resolved",        v.resolved);
    doc.setString("detail",        v.detail);
    scrivi::schemas::appendRepairIssuesToDoc(doc, "warnings", v.warnings);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_enable_git_snapshots(
    const char* projectRootPath,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* initialSnapshotLabel)
{
    scrivi::EnableGitRequest req;
    req.projectRootPath      = S(projectRootPath);
    req.initialSnapshotLabel = initialSnapshotLabel && initialSnapshotLabel[0]
                               ? initialSnapshotLabel : "Initial project";
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().enableGitSnapshots(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setBool("gitInitialized",    v.gitInitialized);
    doc.setBool("alreadyRepository", v.alreadyRepository);
    doc.setString("initialSnapshotID", v.initialSnapshotID.value);
    doc.setString("initialCommitID",   v.initialCommitID.value);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_create_snapshot(
    const char* projectRootPath,
    const char* identityID,
    const char* personaID,
    const char* displayName,
    const char* label,
    const char* note)
{
    scrivi::CreateSnapshotRequest req;
    req.projectRootPath = S(projectRootPath);
    req.label           = S(label);
    req.note            = S(note);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().createSnapshot(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("snapshotID", v.snapshotID.value);
    doc.setString("commitID",   v.commitID.value);
    doc.setString("createdAt",  v.createdAt);
    doc.setBool("created",      v.created);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_create_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* displayName,
    const char* slug,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName)
{
    scrivi::CreateObjectRequest req;
    req.projectRootPath = S(projectRootPath);
    req.objectKind      = objectKindFromStr(S(objectKind));
    req.displayName     = S(displayName);
    req.slug            = S(slug);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(authorDisplayName)
    };

    auto r = core().createObject(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("objectID", v.objectID.value);
    doc.setString("slug",     v.slug);
    doc.setString("path",     v.path);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_open_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectID)
{
    scrivi::OpenObjectRequest req;
    req.projectRootPath = S(projectRootPath);
    req.objectKind      = objectKindFromStr(S(objectKind));
    req.objectID        = scrivi::ObjectID{S(objectID)};

    auto r = core().openObject(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("objectJson", scrivi::schemas::serializeWorldObject(v.object));
    doc.setString("path",       v.path);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_save_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectJson,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName)
{
    auto kind   = objectKindFromStr(S(objectKind));
    auto parseR = scrivi::schemas::parseWorldObject(S(objectJson), kind);
    if (!parseR.ok()) return heap(errorEnvelope(parseR.error()));

    scrivi::SaveObjectRequest req;
    req.projectRootPath = S(projectRootPath);
    req.object          = std::move(parseR.value());
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(authorDisplayName)
    };

    auto r = core().saveObject(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("objectID", v.objectID.value);
    doc.setBool("saved",      v.saved);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_delete_object(
    const char* projectRootPath,
    const char* objectKind,
    const char* objectID)
{
    scrivi::DeleteObjectRequest req;
    req.projectRootPath = S(projectRootPath);
    req.objectKind      = objectKindFromStr(S(objectKind));
    req.objectID        = scrivi::ObjectID{S(objectID)};

    auto r = core().deleteObject(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("objectID", v.objectID.value);
    doc.setBool("deleted",    v.deleted);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_import_asset(
    const char* projectRootPath,
    const char* sourcePath,
    const char* category,
    const char* title,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName)
{
    scrivi::ImportAssetRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sourcePath      = S(sourcePath);
    req.category        = scrivi::assetCategoryFromString(S(category));
    req.title           = S(title);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(authorDisplayName)
    };

    auto r = core().importAsset(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("assetID",     v.assetID);
    doc.setString("assetPath",   v.assetPath);
    doc.setString("sidecarPath", v.sidecarPath);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_list_assets(
    const char* projectRootPath,
    const char* category)
{
    scrivi::ListAssetsRequest req;
    req.projectRootPath = S(projectRootPath);
    std::string catStr  = S(category);
    if (!catStr.empty())
        req.category = scrivi::assetCategoryFromString(catStr);

    auto r = core().listAssets(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    std::string arr = "[";
    bool first = true;
    for (const auto& a : v.assets) {
        if (!first) arr += ",";
        first = false;
        arr += "{\"assetID\":\"" + a.assetID + "\","
               "\"filename\":\"" + a.filename + "\","
               "\"category\":\"" + scrivi::assetCategoryString(a.category) + "\","
               "\"title\":\"" + a.title + "\"}";
    }
    arr += "]";
    scrivi::util::JsonDoc doc;
    doc.setString("assets", arr);
    doc.setInt("count", static_cast<int>(v.assets.size()));
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_remove_asset(
    const char* projectRootPath,
    const char* assetID)
{
    scrivi::RemoveAssetRequest req;
    req.projectRootPath = S(projectRootPath);
    req.assetID         = S(assetID);

    auto r = core().removeAsset(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("assetID", v.assetID);
    doc.setBool("deleted",   v.deleted);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_add_comment(
    const char* projectRootPath,
    const char* scopeKind,
    const char* targetID,
    const char* body,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName)
{
    scrivi::AddCommentRequest req;
    req.projectRootPath = S(projectRootPath);
    req.scopeKind       = S(scopeKind);
    req.targetID        = S(targetID);
    req.body            = S(body);
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(authorDisplayName)
    };

    auto r = core().addComment(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("commentID", v.commentID);
    doc.setBool("added",       v.added);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_list_comments(
    const char* projectRootPath,
    const char* scopeKind,
    const char* targetID)
{
    scrivi::ListCommentsRequest req;
    req.projectRootPath = S(projectRootPath);
    req.scopeKind       = S(scopeKind);
    req.targetID        = S(targetID);

    auto r = core().listComments(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("scopeKind", v.scopeKind);
    doc.setString("targetID",  v.targetID);
    doc.setInt("count",        static_cast<int>(v.comments.size()));
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_resolve_comment(
    const char* projectRootPath,
    const char* scopeKind,
    const char* targetID,
    const char* commentID,
    const char* identityID,
    const char* personaID,
    const char* resolverDisplayName)
{
    scrivi::ResolveCommentRequest req;
    req.projectRootPath = S(projectRootPath);
    req.scopeKind       = S(scopeKind);
    req.targetID        = S(targetID);
    req.commentID       = S(commentID);
    req.resolver = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(resolverDisplayName)
    };

    auto r = core().resolveComment(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("commentID", v.commentID);
    doc.setBool("resolved",    v.resolved);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_list_inbox(const char* projectRootPath)
{
    scrivi::ListInboxRequest req;
    req.projectRootPath = S(projectRootPath);

    auto r = core().listInbox(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    std::string arr = "[";
    bool first = true;
    for (const auto& e : v.entries) {
        if (!first) arr += ",";
        first = false;
        arr += "\"" + e.filename + "\"";
    }
    arr += "]";
    scrivi::util::JsonDoc doc;
    doc.setInt("count",        static_cast<int>(v.entries.size()));
    doc.setString("filenames", arr);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_import_from_inbox(
    const char* projectRootPath,
    const char* filename,
    const char* action,
    const char* category,
    const char* identityID,
    const char* personaID,
    const char* authorDisplayName)
{
    static const auto actionFromStr = [](std::string_view s) -> scrivi::InboxAction {
        if (s == "ignore")     return scrivi::InboxAction::ignore;
        if (s == "deleteFile") return scrivi::InboxAction::deleteFile;
        return scrivi::InboxAction::importAsAsset;
    };

    scrivi::ImportFromInboxRequest req;
    req.projectRootPath = S(projectRootPath);
    req.filename        = S(filename);
    req.action          = actionFromStr(S(action));
    req.assetCategory   = scrivi::assetCategoryFromString(S(category));
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(authorDisplayName)
    };

    auto r = core().importFromInbox(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("actionTaken", v.actionTaken);
    doc.setString("resultPath",  v.resultPath);
    doc.setString("assetID",     v.assetID);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_create_scene(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* chapterID,
    const char* afterSceneID,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    scrivi::CreateSceneRequest req;
    req.projectRootPath = S(projectRootPath);
    req.appSupportRoot  = S(appSupportRoot);
    req.projectID       = scrivi::ProjectID{S(projectID)};
    req.chapterID       = scrivi::ChapterID{S(chapterID)};
    req.afterSceneID    = scrivi::SceneID  {S(afterSceneID)};
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().createScene(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",      v.sceneID.value);
    doc.setString("chapterID",    v.chapterID.value);
    doc.setString("metadataPath", v.metadataPath);
    doc.setString("contentPath",  v.contentPath);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_create_chapter(
    const char* projectRootPath,
    const char* appSupportRoot,
    const char* projectID,
    const char* identityID,
    const char* personaID,
    const char* displayName)
{
    scrivi::CreateChapterRequest req;
    req.projectRootPath = S(projectRootPath);
    req.appSupportRoot  = S(appSupportRoot);
    req.projectID       = scrivi::ProjectID{S(projectID)};
    req.author = {
        scrivi::IdentityID{S(identityID)},
        scrivi::PersonaID {S(personaID)},
        S(displayName)
    };

    auto r = core().createChapter(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("chapterID",              v.chapterID.value);
    doc.setString("chapterMetadataPath",    v.chapterMetadataPath);
    doc.setString("firstSceneID",           v.firstSceneID.value);
    doc.setString("firstSceneMetadataPath", v.firstSceneMetadataPath);
    doc.setString("firstSceneContentPath",  v.firstSceneContentPath);
    return heap(okEnvelope(std::move(doc)));
}

} // extern "C"
