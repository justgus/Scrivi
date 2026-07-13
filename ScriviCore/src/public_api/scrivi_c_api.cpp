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
#include "history/HistoryService.hpp"
#include "history/HistoryStore.hpp"
#include "util/PathUtils.hpp"
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

CoreSingleton& singleton() {
    static CoreSingleton s;
    return s;
}

scrivi::ScriviCore& core() {
    return *singleton().core;
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

// Error envelope from a raw code+message (for ABI-layer errors that don't
// originate from a scrivi::Result — e.g. "history not open").
static std::string errorEnvelope(scrivi::ErrorCode code, std::string_view message) {
    scrivi::util::JsonDoc err;
    err.setInt("code",    static_cast<int>(code));
    err.setString("message", message);
    scrivi::util::JsonDoc root;
    root.setBool("ok", false);
    root.setSubDoc("error", std::move(err));
    return root.dump();
}

// ---------------------------------------------------------------------------
// Undo/Redo history registry (EP-019 SP-052 — T-0202)
// ---------------------------------------------------------------------------
// One HistoryStore (owning a HistoryService + on-disk persistence, SP-054) per
// open project, keyed by projectRootPath. Opened by scrivi_history_open,
// discarded by scrivi_history_close. Guarded by a mutex because ScriviEngine may
// call from arbitrary Swift threads.

struct HistoryRegistry {
    std::mutex mutex;
    std::unordered_map<std::string, std::unique_ptr<scrivi::history::HistoryStore>> byRoot;
};

static HistoryRegistry& historyRegistry() {
    static HistoryRegistry r;
    return r;
}

// UTF-8-scalar-safe v7-style ID minter for history nodes/sessions. Kept local
// to the ABI layer so the shared UUIDProvider interface stays untouched; mirror
// of SystemUUIDProvider's format (<prefix>_<uuid-v7>).
static std::string mintHistoryID(std::string_view prefix) {
    // Reuse the process UUID provider via a throwaway SceneID mint, then swap
    // the prefix — cheaper than duplicating makeV7 and keeps one RNG path.
    static scrivi::platform::SystemUUIDProvider provider;
    const std::string raw = provider.newSceneID().value;      // "scene_<uuid>"
    const auto us = raw.find('_');
    const std::string uuid = (us == std::string::npos) ? raw : raw.substr(us + 1);
    return std::string(prefix) + "_" + uuid;
}

static std::string nowTimestamp() {
    static PrototypeClock clock;
    return clock.nowUTC();
}

static scrivi::history::EventKind eventKindFromStr(std::string_view s) {
    using K = scrivi::history::EventKind;
    if (s == "delete")  return K::Delete;
    if (s == "replace") return K::Replace;
    if (s == "paste")   return K::Paste;
    if (s == "cut")     return K::Cut;
    return K::Typing;   // default
}

// Appends the {sceneID,newText,cursorAfter} change (if any) to a "changes"
// array on `doc`, matching the multi-scene envelope shape (v1 emits at most one).
static void appendStepChanges(scrivi::util::JsonDoc& doc,
                              const scrivi::history::StepResult& step) {
    if (step.change.has_value()) {
        scrivi::util::JsonDoc c;
        c.setString("sceneID",    step.change->sceneID);
        c.setString("newText",    step.change->newText);
        c.setInt64("cursorAfter", step.change->cursorAfter);
        doc.appendToArray("changes", std::move(c));
    }
}

// Serializes step.forkAhead into the undo/redo envelope (§7/§10 T2) when the
// pointer landed on a fork. Absent otherwise, so the Swift popover shows/dismisses
// purely on the field's presence.
static void appendForkAhead(scrivi::util::JsonDoc& doc,
                            const scrivi::history::StepResult& step) {
    if (!step.forkAhead.has_value()) return;
    scrivi::util::JsonDoc fa;
    fa.setString("nodeID", step.forkAhead->nodeID);
    for (const auto& child : step.forkAhead->children) {
        scrivi::util::JsonDoc c;
        c.setString("eventID",   child.eventID);
        c.setString("preview",   child.preview);
        c.setString("timestamp", child.timestamp);
        c.setBool("isPrimary",   child.isPrimary);
        fa.appendToArray("children", std::move(c));
    }
    doc.setSubDoc("forkAhead", std::move(fa));
}

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

    // Restored cursor selection (scene-local offsets) and scroll fraction (I-0058).
    scrivi::util::JsonDoc restored;
    restored.setInt64("anchor", static_cast<int64_t>(v.restoredSelection.anchor));
    restored.setInt64("focus",  static_cast<int64_t>(v.restoredSelection.focus));
    restored.setDouble("scroll", v.restoredScroll.value);

    scrivi::util::JsonDoc doc;
    doc.setString("projectID", v.project.projectID.value);
    doc.setString("mode", "ready");
    doc.setSubDoc("activeScene", std::move(scene));
    doc.setSubDoc("restored", std::move(restored));
    for (auto& s : v.scenes) {
        scrivi::util::JsonDoc entry;
        entry.setString("sceneID",              s.sceneID.value);
        entry.setString("chapterID",            s.chapterID.value);
        entry.setString("title",                s.title);
        entry.setString("chapterTitle",         s.chapterTitle);
        entry.setString("slug",                 s.slug);
        entry.setString("metadataPath",         s.metadataPath);
        entry.setString("contentPath",          s.contentPath);
        entry.setString("chapterMetadataPath",  s.chapterMetadataPath);
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
    long long   selectionAnchor,
    long long   selectionFocus,
    double      scroll,
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
    req.selection = {
        selectionAnchor < 0 ? 0u : static_cast<std::size_t>(selectionAnchor),
        selectionFocus  < 0 ? 0u : static_cast<std::size_t>(selectionFocus)
    };
    req.scroll = { scroll };
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

const char* scrivi_delete_scene(
    const char* projectRootPath,
    const char* sceneID)
{
    scrivi::DeleteSceneRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sceneID         = scrivi::SceneID{S(sceneID)};

    auto r = core().deleteScene(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID", v.sceneID.value);
    doc.setBool("deleted",   v.deleted);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_delete_chapter(
    const char* projectRootPath,
    const char* chapterID)
{
    scrivi::DeleteChapterRequest req;
    req.projectRootPath = S(projectRootPath);
    req.chapterID       = scrivi::ChapterID{S(chapterID)};

    auto r = core().deleteChapter(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("chapterID",    v.chapterID.value);
    doc.setInt("scenesDeleted",   v.scenesDeleted);
    doc.setBool("deleted",        v.deleted);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_reorder_scene(
    const char* projectRootPath,
    const char* sceneID,
    const char* sourceChapterID,
    const char* targetChapterID,
    const char* afterSceneID)
{
    scrivi::ReorderSceneRequest req;
    req.projectRootPath  = S(projectRootPath);
    req.sceneID          = scrivi::SceneID  {S(sceneID)};
    req.sourceChapterID  = scrivi::ChapterID{S(sourceChapterID)};
    req.targetChapterID  = scrivi::ChapterID{S(targetChapterID)};
    req.afterSceneID     = scrivi::SceneID  {S(afterSceneID)};

    auto r = core().reorderScene(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",         v.sceneID.value);
    doc.setString("sourceChapterID", v.sourceChapterID.value);
    doc.setString("targetChapterID", v.targetChapterID.value);
    doc.setBool("reordered",         v.reordered);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_reorder_chapter(
    const char* projectRootPath,
    const char* chapterID,
    const char* afterChapterID)
{
    scrivi::ReorderChapterRequest req;
    req.projectRootPath = S(projectRootPath);
    req.chapterID       = scrivi::ChapterID{S(chapterID)};
    req.afterChapterID  = scrivi::ChapterID{S(afterChapterID)};

    auto r = core().reorderChapter(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("chapterID", v.chapterID.value);
    doc.setBool("reordered",   v.reordered);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_rename_scene(
    const char* projectRootPath,
    const char* metadataPath,
    const char* newTitle)
{
    scrivi::RenameSceneRequest req;
    req.projectRootPath = S(projectRootPath);
    req.metadataPath    = S(metadataPath);
    req.newTitle        = S(newTitle);

    auto r = core().renameScene(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("metadataPath", v.metadataPath);
    doc.setString("newTitle",     v.newTitle);
    doc.setBool("renamed",        v.renamed);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_rename_chapter(
    const char* projectRootPath,
    const char* metadataPath,
    const char* newTitle)
{
    scrivi::RenameChapterRequest req;
    req.projectRootPath = S(projectRootPath);
    req.metadataPath    = S(metadataPath);
    req.newTitle        = S(newTitle);

    auto r = core().renameChapter(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("metadataPath", v.metadataPath);
    doc.setString("newTitle",     v.newTitle);
    doc.setBool("renamed",        v.renamed);
    return heap(okEnvelope(std::move(doc)));
}

// ---- Timeline (EP-016 SP-039) -----------------------------------------------

const char* scrivi_get_timeline(const char* projectRootPath) {
    scrivi::GetTimelineRequest req;
    req.projectRootPath = S(projectRootPath);
    auto r = core().getTimeline(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("timelineID", v.timelineID);
    doc.setString("epochLabel", v.epochLabel);
    doc.setString("projectID",  v.projectID);
    doc.setString("createdAt",  v.createdAt);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_set_timeline_epoch_label(const char* projectRootPath, const char* label) {
    scrivi::SetTimelineEpochLabelRequest req;
    req.projectRootPath = S(projectRootPath);
    req.label           = S(label);
    auto r = core().setTimelineEpochLabel(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setBool("updated", r.value().updated);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_set_scene_story_time(const char* projectRootPath, const char* sceneID,
                                         int64_t offsetMs, const char* source,
                                         int64_t gapMs,
                                         int64_t durationMs, const char* durationSource) {
    scrivi::SetSceneStoryTimeRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sceneID.value   = S(sceneID);
    req.offsetMs        = offsetMs;
    req.source          = S(source);
    req.gapMs           = gapMs;
    req.durationMs      = durationMs;
    req.durationSource  = S(durationSource);
    auto r = core().setSceneStoryTime(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID", r.value().sceneID.value);
    doc.setBool("updated",   r.value().updated);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_get_scene_story_time(const char* projectRootPath, const char* sceneID) {
    scrivi::GetSceneStoryTimeRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sceneID.value   = S(sceneID);
    auto r = core().getSceneStoryTime(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",             v.sceneID.value);
    doc.setInt64("offsetMs",             v.offsetMs);
    doc.setString("offsetSource",        v.offsetSource);
    doc.setInt64("gapMs",                v.gapMs);
    doc.setInt64("durationMs",           v.durationMs);
    doc.setString("durationSource",      v.durationSource);
    doc.setString("inferenceHint",       v.inferenceHint);
    doc.setDouble("inferenceConfidence", v.inferenceConfidence);
    doc.setString("bandID",              v.bandID);
    doc.setString("bandAssignedAt",      v.bandAssignedAt);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_clear_scene_story_time(const char* projectRootPath, const char* sceneID) {
    scrivi::ClearSceneStoryTimeRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sceneID.value   = S(sceneID);
    auto r = core().clearSceneStoryTime(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID", r.value().sceneID.value);
    doc.setBool("cleared",   r.value().cleared);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_assign_scene_to_band(const char* projectRootPath, const char* sceneID,
                                         const char* bandID) {
    scrivi::AssignSceneToBandRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sceneID.value   = S(sceneID);
    req.bandID          = S(bandID);
    auto r = core().assignSceneToBand(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",  r.value().sceneID.value);
    doc.setBool("assigned",   r.value().assigned);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_unassign_scene_from_band(const char* projectRootPath, const char* sceneID) {
    scrivi::UnassignSceneFromBandRequest req;
    req.projectRootPath = S(projectRootPath);
    req.sceneID.value   = S(sceneID);
    auto r = core().unassignSceneFromBand(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("sceneID",   r.value().sceneID.value);
    doc.setBool("unassigned",  r.value().unassigned);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_get_story_structure(const char* projectRootPath) {
    scrivi::GetStoryStructureRequest req;
    req.projectRootPath = S(projectRootPath);
    auto r = core().getStoryStructure(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setBool("hasStructure",   v.hasStructure);
    doc.setString("structureID",  v.structureID);
    doc.setString("bandLayoutJSON", v.bandLayoutJSON);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_set_story_structure(const char* projectRootPath, const char* structureID,
                                        const char* bandLayoutJSON) {
    scrivi::SetStoryStructureRequest req;
    req.projectRootPath = S(projectRootPath);
    req.structureID     = S(structureID);
    req.bandLayoutJSON  = S(bandLayoutJSON);
    auto r = core().setStoryStructure(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setBool("set", r.value().set);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_update_band_layout(const char* projectRootPath, const char* bandLayoutJSON) {
    scrivi::UpdateBandLayoutRequest req;
    req.projectRootPath = S(projectRootPath);
    req.bandLayoutJSON  = S(bandLayoutJSON);
    auto r = core().updateBandLayout(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setBool("updated", r.value().updated);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_remove_story_structure(const char* projectRootPath) {
    scrivi::RemoveStoryStructureRequest req;
    req.projectRootPath = S(projectRootPath);
    auto r = core().removeStoryStructure(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setBool("removed", r.value().removed);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_create_historical_event(const char* projectRootPath,
                                             const char* title, int64_t offsetMs,
                                             const char* description, const char* tagsJSON,
                                             const char* identityID, const char* personaID,
                                             const char* displayName) {
    scrivi::CreateHistoricalEventRequest req;
    req.projectRootPath = S(projectRootPath);
    req.title           = S(title);
    req.offsetMs        = offsetMs;
    req.description     = S(description);
    req.author = {
        .identityID  = scrivi::IdentityID{S(identityID)},
        .personaID   = scrivi::PersonaID{S(personaID)},
        .displayName = S(displayName)
    };
    // Parse tagsJSON as a flat string array: ["tag1","tag2"]
    auto tagsR = scrivi::util::parseJson(S(tagsJSON));
    if (tagsR.ok()) {
        req.tags = tagsR.value().getStringArray("tags");
        // Also try root-level array
        if (req.tags.empty()) {
            const auto n = tagsR.value().arraySize("tags");
            (void)n; // getStringArray already handles this
        }
    }
    auto r = core().createHistoricalEvent(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("eventID", r.value().eventID);
    doc.setString("slug",    r.value().slug);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_update_historical_event(const char* projectRootPath, const char* eventID,
                                             const char* title, int64_t offsetMs,
                                             const char* description, const char* tagsJSON) {
    scrivi::UpdateHistoricalEventRequest req;
    req.projectRootPath = S(projectRootPath);
    req.eventID         = S(eventID);
    req.title           = S(title);
    req.offsetMs        = offsetMs;
    req.description     = S(description);
    auto tagsR = scrivi::util::parseJson(S(tagsJSON));
    if (tagsR.ok()) { req.tags = tagsR.value().getStringArray("tags"); }
    auto r = core().updateHistoricalEvent(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("eventID", r.value().eventID);
    doc.setBool("updated",   r.value().updated);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_delete_historical_event(const char* projectRootPath, const char* eventID) {
    scrivi::DeleteHistoricalEventRequest req;
    req.projectRootPath = S(projectRootPath);
    req.eventID         = S(eventID);
    auto r = core().deleteHistoricalEvent(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("eventID", r.value().eventID);
    doc.setBool("deleted",   r.value().deleted);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_list_historical_events(const char* projectRootPath) {
    scrivi::ListHistoricalEventsRequest req;
    req.projectRootPath = S(projectRootPath);
    auto r = core().listHistoricalEvents(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setInt("count",        r.value().count);
    doc.setString("eventsJSON", r.value().eventsJSON);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_import_external_timeline(const char* projectRootPath,
                                              const char* timelineJSON, int64_t epochOffsetMs,
                                              const char* assignedGreyShade) {
    scrivi::ImportExternalTimelineRequest req;
    req.projectRootPath    = S(projectRootPath);
    req.timelineJSON       = S(timelineJSON);
    req.epochOffsetMs      = epochOffsetMs;
    req.assignedGreyShade  = S(assignedGreyShade);
    auto r = core().importExternalTimeline(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("timelineID", r.value().timelineID);
    doc.setBool("imported",     r.value().imported);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_update_imported_timeline_offset(const char* projectRootPath,
                                                     const char* timelineID,
                                                     int64_t epochOffsetMs) {
    scrivi::UpdateImportedTimelineOffsetRequest req;
    req.projectRootPath = S(projectRootPath);
    req.timelineID      = S(timelineID);
    req.epochOffsetMs   = epochOffsetMs;
    auto r = core().updateImportedTimelineOffset(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("timelineID", r.value().timelineID);
    doc.setBool("updated",      r.value().updated);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_set_imported_timeline_visible(const char* projectRootPath,
                                                   const char* timelineID, int visible) {
    scrivi::SetImportedTimelineVisibleRequest req;
    req.projectRootPath = S(projectRootPath);
    req.timelineID      = S(timelineID);
    req.visible         = (visible != 0);
    auto r = core().setImportedTimelineVisible(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("timelineID", r.value().timelineID);
    doc.setBool("updated",      r.value().updated);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_list_imported_timelines(const char* projectRootPath) {
    scrivi::ListImportedTimelinesRequest req;
    req.projectRootPath = S(projectRootPath);
    auto r = core().listImportedTimelines(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setInt("count",           r.value().count);
    doc.setString("timelinesJSON", r.value().timelinesJSON);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_remove_imported_timeline(const char* projectRootPath, const char* timelineID) {
    scrivi::RemoveImportedTimelineRequest req;
    req.projectRootPath = S(projectRootPath);
    req.timelineID      = S(timelineID);
    auto r = core().removeImportedTimeline(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("timelineID", r.value().timelineID);
    doc.setBool("removed",      r.value().removed);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_export_project_timeline(const char* projectRootPath) {
    scrivi::ExportProjectTimelineRequest req;
    req.projectRootPath = S(projectRootPath);
    auto r = core().exportProjectTimeline(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));
    scrivi::util::JsonDoc doc;
    doc.setString("timelineJSON", r.value().timelineJSON);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_extract_searchable_text(const char* projectRootPath) {
    scrivi::ExtractSearchableTextRequest req;
    req.projectRootPath = S(projectRootPath);

    auto r = core().extractSearchableText(req);
    if (!r.ok()) return heap(errorEnvelope(r.error()));

    const auto& v = r.value();
    scrivi::util::JsonDoc doc;
    doc.setString("schema",           v.schema);
    doc.setString("domainIdentifier", v.domainIdentifier);
    doc.setString("projectRootPath",  v.projectRootPath);

    for (const auto& it : v.items) {
        scrivi::util::JsonDoc item;
        item.setString("uniqueIdentifier", it.uniqueIdentifier);
        item.setString("kind",             it.kind);
        item.setString("title",            it.title);
        item.setString("displayName",      it.displayName);
        item.setString("deepLink",         it.deepLink);
        // Optional fields: omitted (not null) when empty, per the schema.
        if (!it.containerTitle.empty())    { item.setString("containerTitle", it.containerTitle); }
        if (!it.contentDescription.empty()){ item.setString("contentDescription", it.contentDescription); }
        for (const auto& kw : it.keywords) { item.appendStringToArray("keywords", kw); }
        doc.appendToArray("items", std::move(item));
    }

    return heap(okEnvelope(std::move(doc)));
}

// ---- Undo/Redo history (EP-019 SP-052 — T-0202) -------------------------

// Helper: the history/ directory for a project root.
static scrivi::AbsolutePath historyDirFor(const std::string& projectRoot) {
    return scrivi::util::join(projectRoot, "history");
}

const char* scrivi_history_open(const char* projectRootPath) {
    const std::string root = S(projectRootPath);
    if (root.empty())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "projectRootPath is required"));

    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto& s = singleton();
    auto store = std::make_unique<scrivi::history::HistoryStore>(
        historyDirFor(root), &s.fileSystem, &s.clock);

    bool loaded = false;
    store->openOrCreate(mintHistoryID("ses"), nowTimestamp(), loaded);

    auto& svc = store->service();
    svc.setCapacity(store->settings().capacityEvents);
    scrivi::util::JsonDoc doc;
    doc.setString("sessionID",     svc.sessionID());
    doc.setString("currentNodeID", svc.currentNodeID());
    doc.setBool("canUndo",         svc.canUndo());
    doc.setBool("canRedo",         svc.canRedo());
    doc.setBool("loaded",          loaded);
    scrivi::util::JsonDoc st;
    st.setInt("capacityEvents",    store->settings().capacityEvents);
    st.setInt("staleBranchDays",   store->settings().staleBranchDays);
    st.setInt("idleRolloverHours", store->settings().idleRolloverHours);
    doc.setSubDoc("settings", std::move(st));

    reg.byRoot[root] = std::move(store);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_seed_scene(const char* projectRootPath,
                                      const char* sceneID,
                                      const char* sceneTextUtf8) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    auto& svc = it->second->service();
    const std::string sid = S(sceneID);
    const bool wasNew = svc.headTextForScene(sid).empty()
                        && svc.floorTexts().find(sid) == svc.floorTexts().end();
    svc.seedSceneBaseline(sid, S(sceneTextUtf8));
    // Persist the floor record once, the first time this scene enters history.
    if (wasNew) { it->second->persistFloor(sid, S(sceneTextUtf8), "seed"); }

    scrivi::util::JsonDoc doc;
    doc.setBool("seeded", true);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_record_event(const char* projectRootPath,
                                         const char* sceneID,
                                         const char* newSceneTextUtf8,
                                         const char* paramsJSON) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    scrivi::history::RecordParams p;
    p.sceneID      = S(sceneID);
    p.newSceneText = S(newSceneTextUtf8);
    p.timestamp    = nowTimestamp();

    auto paramsR = scrivi::util::parseJson(S(paramsJSON));
    if (paramsR.ok()) {
        const auto& pj = paramsR.value();
        p.kind         = eventKindFromStr(pj.getString("kind", "typing"));
        p.cursorBefore = pj.getInt64("cursorBefore", 0);
        p.cursorAfter  = pj.getInt64("cursorAfter", 0);
    }

    auto& svc = it->second->service();
    const auto r = svc.record(p, mintHistoryID("evt"));
    // Persist the new event node (if one was actually created).
    if (!r.noOp && !r.eventID.empty()) {
        auto nit = svc.nodes().find(r.eventID);
        if (nit != svc.nodes().end()) { it->second->persistEvent(nit->second); }
    }
    // Persist any branch-aware eviction the record triggered (§4.1) — ctl:purge /
    // ctl:evict records so evicted branches don't resurrect from the log on reload.
    if (r.evictedCount > 0) { it->second->persistEviction(r.eviction); }

    scrivi::util::JsonDoc doc;
    doc.setString("eventID",      r.eventID);
    doc.setBool("createdBranch",  r.createdBranch);
    doc.setInt("evictedCount",    r.evictedCount);
    doc.setBool("noOp",           r.noOp);
    doc.setBool("canUndo",        svc.canUndo());
    doc.setBool("canRedo",        svc.canRedo());
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_record_barrier(const char* projectRootPath,
                                           const char* paramsJSON) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    scrivi::history::BarrierParams p;
    p.timestamp = nowTimestamp();
    auto paramsR = scrivi::util::parseJson(S(paramsJSON));
    if (paramsR.ok()) {
        const auto& pj = paramsR.value();
        p.barrierKind = pj.getString("barrierKind", "");
        p.barrierNote = pj.getString("note", "");
    }

    auto& svc = it->second->service();
    const auto r = svc.recordBarrier(p, mintHistoryID("evt"));
    if (!r.eventID.empty()) {
        auto nit = svc.nodes().find(r.eventID);
        if (nit != svc.nodes().end()) { it->second->persistEvent(nit->second); }
    }

    scrivi::util::JsonDoc doc;
    doc.setString("eventID", r.eventID);
    doc.setBool("canUndo",   svc.canUndo());
    doc.setBool("canRedo",   svc.canRedo());
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_undo(const char* projectRootPath) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    auto& svc = it->second->service();
    const auto step = svc.undo();
    if (step.moved) { it->second->persistCtl("undo", step.nodeID); }

    scrivi::util::JsonDoc doc;
    doc.setBool("moved",  step.moved);
    doc.setString("nodeID", step.nodeID);
    doc.setBool("canUndo", step.canUndo);
    doc.setBool("canRedo", step.canRedo);
    doc.setBool("crossedSessionBoundary", step.crossedSessionBoundary);
    if (step.crossedSessionBoundary)
        doc.setString("boundaryTimestamp", step.boundaryTimestamp);
    if (step.stoppedAtBarrier) {
        scrivi::util::JsonDoc b;
        b.setString("kind", step.barrierKind);
        b.setString("note", step.barrierNote);
        doc.setSubDoc("stoppedAtBarrier", std::move(b));
    }
    appendStepChanges(doc, step);
    appendForkAhead(doc, step);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_redo(const char* projectRootPath) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    auto& svc = it->second->service();
    const auto step = svc.redo();
    if (step.moved) { it->second->persistCtl("redo", step.nodeID); }

    scrivi::util::JsonDoc doc;
    doc.setBool("moved",  step.moved);
    doc.setString("nodeID", step.nodeID);
    doc.setBool("canUndo", step.canUndo);
    doc.setBool("canRedo", step.canRedo);
    appendStepChanges(doc, step);
    appendForkAhead(doc, step);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_select_branch(const char* projectRootPath,
                                         const char* forkNodeID,
                                         const char* childEventID) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    auto& svc = it->second->service();
    const auto r = svc.selectBranch(S(forkNodeID), S(childEventID));
    if (r.ok) { it->second->persistSetPrimary(r.forkNodeID, r.childEventID); }

    scrivi::util::JsonDoc doc;
    doc.setBool("ok",            r.ok);
    doc.setString("forkNodeID",  r.forkNodeID);
    doc.setString("childEventID", r.childEventID);
    doc.setBool("canRedo",       r.canRedo);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_list_stale_branches(const char* projectRootPath) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    auto& svc = it->second->service();
    const auto stale = svc.listStaleBranches(nowTimestamp(),
                                             it->second->settings().staleBranchDays);

    scrivi::util::JsonDoc doc;
    doc.setInt("staleBranchDays", it->second->settings().staleBranchDays);
    for (const auto& sb : stale) {
        scrivi::util::JsonDoc b;
        b.setString("branchRootEventID", sb.branchRootEventID);
        b.setString("forkNodeID",        sb.forkNodeID);
        b.setString("preview",           sb.preview);
        b.setString("tipTimestamp",      sb.tipTimestamp);
        b.setInt("nodeCount",            sb.nodeCount);
        doc.appendToArray("branches", std::move(b));
    }
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_purge_branch(const char* projectRootPath,
                                        const char* branchRootEventID) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    auto& svc = it->second->service();
    const auto r = svc.purgeBranch(S(branchRootEventID));
    if (r.ok) { it->second->persistPurge(r.branchRootEventID); }

    scrivi::util::JsonDoc doc;
    doc.setBool("ok",                r.ok);
    doc.setString("branchRootEventID", r.branchRootEventID);
    doc.setInt("purgedCount",        r.purgedCount);
    doc.setBool("canUndo",           r.canUndo);
    doc.setBool("canRedo",           r.canRedo);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_validate_scene(const char* projectRootPath,
                                          const char* sceneID,
                                          const char* currentDiskTextUtf8) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);
    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));

    const bool mismatch = it->second->validateSceneHead(
        S(sceneID), S(currentDiskTextUtf8), nowTimestamp(), mintHistoryID("evt"));

    scrivi::util::JsonDoc doc;
    doc.setBool("externalChange", mismatch);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_get_settings(const char* projectRootPath) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);
    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));
    const auto& s = it->second->settings();
    scrivi::util::JsonDoc doc;
    doc.setInt("capacityEvents",    s.capacityEvents);
    doc.setInt("staleBranchDays",   s.staleBranchDays);
    doc.setInt("idleRolloverHours", s.idleRolloverHours);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_set_settings(const char* projectRootPath,
                                        const char* settingsJSON) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);
    auto it = reg.byRoot.find(root);
    if (it == reg.byRoot.end())
        return heap(errorEnvelope(scrivi::ErrorCode::invalidArgument,
                                  "history not open for this project"));
    scrivi::history::HistorySettings s = it->second->settings();
    auto pr = scrivi::util::parseJson(S(settingsJSON));
    if (pr.ok()) {
        const auto& pj = pr.value();
        s.capacityEvents    = pj.getInt("capacityEvents", s.capacityEvents);
        s.staleBranchDays   = pj.getInt("staleBranchDays", s.staleBranchDays);
        s.idleRolloverHours = pj.getInt("idleRolloverHours", s.idleRolloverHours);
    }
    it->second->setSettings(s);
    it->second->service().setCapacity(s.capacityEvents);
    it->second->checkpoint();
    scrivi::util::JsonDoc doc;
    doc.setBool("updated", true);
    return heap(okEnvelope(std::move(doc)));
}

const char* scrivi_history_close(const char* projectRootPath) {
    const std::string root = S(projectRootPath);
    auto& reg = historyRegistry();
    std::lock_guard<std::mutex> lock(reg.mutex);

    auto it = reg.byRoot.find(root);
    const bool wasOpen = it != reg.byRoot.end();
    if (wasOpen) {
        it->second->checkpoint();   // final checkpoint before discarding
        reg.byRoot.erase(it);
    }

    scrivi::util::JsonDoc doc;
    doc.setBool("closed", wasOpen);
    return heap(okEnvelope(std::move(doc)));
}

} // extern "C"
