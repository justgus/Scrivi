#pragma once

#include "scrivi/AssetTypes.hpp"
#include "scrivi/CommentTypes.hpp"
#include "scrivi/ObjectTypes.hpp"
#include "scrivi/RepairIssue.hpp"
#include "scrivi/Types.hpp"

#include <optional>
#include <vector>

namespace scrivi {

struct EnsureIdentityResult {
    IdentityID  identityID;
    PersonaID   defaultPersonaID;
    std::string displayName;
    bool        createdNewIdentity = false;
};

struct CreateProjectResult {
    ProjectSummary project;

    ManuscriptID manuscriptID;
    ChapterID    firstChapterID;
    SceneID      firstSceneID;

    RelativePath firstSceneMetadataPath;
    RelativePath firstSceneContentPath;

    WorkspaceState workspaceState;

    bool                   gitInitialized    = false;
    std::optional<SnapshotID> initialSnapshotID;

    std::vector<RepairIssue> warnings;
};

struct OpenProjectResult {
    OpenMode mode = OpenMode::cannotOpen;

    ProjectSummary project;

    std::optional<WorkspaceState> workspaceState;
    std::optional<SceneSummary>   activeScene;
    Utf8Text                      activeSceneMarkdown;

    TextSelection  restoredSelection;
    ScrollPosition restoredScroll;

    std::vector<RepairIssue> repairIssues;
};

struct SaveSceneResult {
    SceneID sceneID;

    bool saved                  = false;
    bool metadataUpdated        = false;
    bool workspaceStateUpdated  = false;

    std::size_t wordCount      = 0;
    std::size_t characterCount = 0;

    bool hasUnsnapshottedChanges = false;

    std::vector<RepairIssue> repairIssues;
};

struct ExternalChangeScanResult {
    ProjectID projectID;

    std::vector<RepairIssue> repairIssues;

    bool indexesDirty             = false;
    bool gitStatusChecked         = false;
    bool hasUnsnapshottedChanges  = false;
};

struct EnableGitResult {
    bool       gitInitialized    = false;
    bool       alreadyRepository = false;
    SnapshotID initialSnapshotID;
    CommitID   initialCommitID;
    std::vector<RepairIssue> warnings;
};

struct CreateSnapshotResult {
    SnapshotID       snapshotID;
    CommitID         commitID;
    ISO8601Timestamp createdAt;
    bool             created = false;
};

struct ApplyRepairResult {
    std::string       issueID;
    RepairActionKind  actionApplied = RepairActionKind::none;
    bool              resolved = false;
    std::string       detail;                      // human-readable outcome
    std::vector<RepairIssue> warnings;             // any new issues discovered during repair
};

// ---------------------------------------------------------------------------
// Object CRUD results (EP-005)
// ---------------------------------------------------------------------------

struct CreateObjectResult {
    ObjectID     objectID;
    Slug         slug;
    AbsolutePath path;             // absolute path to the created .json file
};

struct OpenObjectResult {
    WorldObject  object;
    AbsolutePath path;
};

struct SaveObjectResult {
    ObjectID objectID;
    bool     saved = false;
};

struct DeleteObjectResult {
    ObjectID objectID;
    bool     deleted = false;
};

// ---------------------------------------------------------------------------
// Asset results (EP-005 T-0041)
// ---------------------------------------------------------------------------

struct ImportAssetResult {
    std::string  assetID;
    AbsolutePath assetPath;    // absolute path to the copied binary file
    AbsolutePath sidecarPath;  // absolute path to the .meta.json file
};

struct ListAssetsResult {
    std::vector<AssetMeta> assets;
};

struct RemoveAssetResult {
    std::string assetID;
    bool        deleted = false;
};

// ---------------------------------------------------------------------------
// Comment results (EP-005 T-0044)
// ---------------------------------------------------------------------------

struct AddCommentResult {
    std::string commentID;
    bool        added = false;
};

struct ListCommentsResult {
    std::vector<Comment> comments;
    std::string          scopeKind;
    std::string          targetID;
};

struct ResolveCommentResult {
    std::string commentID;
    bool        resolved = false;
};

// ---------------------------------------------------------------------------
// Inbox results (EP-005 T-0046)
// ---------------------------------------------------------------------------

struct InboxEntry {
    std::string  filename;      // basename, e.g. "photo.jpg"
    AbsolutePath absolutePath;
};

struct ListInboxResult {
    std::vector<InboxEntry> entries;
};

struct ImportFromInboxResult {
    std::string actionTaken;   // "importAsAsset" | "ignored" | "deleted"
    std::string resultPath;    // absolute path of resulting asset (empty for ignore/delete)
    std::string assetID;       // set when actionTaken == "importAsAsset"
};

} // namespace scrivi
