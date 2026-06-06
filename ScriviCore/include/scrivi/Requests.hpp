#pragma once

#include "scrivi/AssetTypes.hpp"
#include "scrivi/CommentTypes.hpp"
#include "scrivi/ObjectTypes.hpp"
#include "scrivi/RepairIssue.hpp"
#include "scrivi/Types.hpp"

#include <optional>
#include <string>

namespace scrivi {

struct EnsureIdentityRequest {
    std::string  requestedDisplayName;
    AbsolutePath appSupportRoot;
};

struct CreateProjectRequest {
    AbsolutePath projectRootPath;
    std::string  title;
    Slug         slug;
    AuthorshipRef author;

    std::string initialChapterTitle = "Chapter 1";
    std::string initialChapterSlug  = "chapter-001";
    std::string initialSceneTitle   = "Opening Scene";
    std::string initialSceneSlug    = "001-opening-scene";

    bool enableGitSnapshots = false;

    AbsolutePath appSupportRoot;
};

struct OpenProjectRequest {
    AbsolutePath              projectRootPath;
    AbsolutePath              appSupportRoot;
    std::optional<IdentityID> currentIdentityID;
};

struct SaveSceneRequest {
    ProjectID    projectID;
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    SceneID      sceneID;
    RelativePath sceneMetadataPath;
    RelativePath sceneContentPath;

    Utf8Text      markdown;
    TextSelection selection;
    ScrollPosition scroll;

    AuthorshipRef author;

    std::optional<std::string> previouslyLoadedContentHash;
};

struct ExternalChangeScanRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;
    bool         includeGitStatus = true;
};

struct EnableGitRequest {
    AbsolutePath  projectRootPath;
    AuthorshipRef author;
    std::string   initialSnapshotLabel = "Initial project";
};

struct CreateSnapshotRequest {
    AbsolutePath  projectRootPath;
    AuthorshipRef author;
    std::string   label;
    std::string   note;
};

struct ApplyRepairRequest {
    std::string       issueID;
    AbsolutePath      projectRootPath;
    AbsolutePath      appSupportRoot;
    RepairActionKind  actionKind = RepairActionKind::none;
    std::string       targetPath;   // optional — used for relink/move operations
    AuthorshipRef     author;
};

// ---------------------------------------------------------------------------
// Object CRUD requests (EP-005)
// ---------------------------------------------------------------------------

struct CreateObjectRequest {
    AbsolutePath  projectRootPath;
    ObjectKind    objectKind = ObjectKind::character;
    std::string   displayName;     // required — slug is derived from this
    Slug          slug;            // optional override; if empty, derived from displayName
    AuthorshipRef author;
};

struct OpenObjectRequest {
    AbsolutePath  projectRootPath;
    ObjectKind    objectKind = ObjectKind::character;
    ObjectID      objectID;
};

struct SaveObjectRequest {
    AbsolutePath  projectRootPath;
    WorldObject   object;          // full updated object; objectID must match an existing file
    AuthorshipRef author;
};

struct DeleteObjectRequest {
    AbsolutePath  projectRootPath;
    ObjectKind    objectKind = ObjectKind::character;
    ObjectID      objectID;
};

// ---------------------------------------------------------------------------
// Asset requests (EP-005 T-0041)
// ---------------------------------------------------------------------------

struct ImportAssetRequest {
    AbsolutePath  projectRootPath;
    AbsolutePath  sourcePath;         // absolute path to the file to import
    AssetCategory category;
    std::string   title;
    AuthorshipRef author;
};

struct ListAssetsRequest {
    AbsolutePath             projectRootPath;
    std::optional<AssetCategory> category;   // if set, filter by this category
};

struct RemoveAssetRequest {
    AbsolutePath projectRootPath;
    std::string  assetID;
};

// ---------------------------------------------------------------------------
// Comment requests (EP-005 T-0044)
// ---------------------------------------------------------------------------

struct AddCommentRequest {
    AbsolutePath  projectRootPath;
    std::string   scopeKind;   // "scene" | "object"
    std::string   targetID;    // sceneID or objectID value
    std::string   body;
    AuthorshipRef author;
};

struct ListCommentsRequest {
    AbsolutePath projectRootPath;
    std::string  scopeKind;
    std::string  targetID;
};

struct ResolveCommentRequest {
    AbsolutePath  projectRootPath;
    std::string   scopeKind;
    std::string   targetID;
    std::string   commentID;
    AuthorshipRef resolver;
};

// ---------------------------------------------------------------------------
// Inbox requests (EP-005 T-0046)
// ---------------------------------------------------------------------------

enum class InboxAction : std::uint8_t { importAsAsset, ignore, deleteFile };

struct ListInboxRequest {
    AbsolutePath projectRootPath;
};

struct ImportFromInboxRequest {
    AbsolutePath  projectRootPath;
    std::string   filename;       // basename of the file in inbox/dropped-files/
    InboxAction   action = InboxAction::importAsAsset;
    AssetCategory assetCategory = AssetCategory::other;  // used when action == importAsAsset
    AuthorshipRef author;
};

// ---------------------------------------------------------------------------
// openScene (EP-008 T-0060)
// ---------------------------------------------------------------------------

struct OpenSceneRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;
    ProjectID    projectID;
    SceneID      sceneID;
};

// ---------------------------------------------------------------------------
// createScene / createChapter (EP-009 SP-021 T-0067)
// ---------------------------------------------------------------------------

struct CreateSceneRequest {
    AbsolutePath  projectRootPath;
    AbsolutePath  appSupportRoot;
    ProjectID     projectID;
    ChapterID     chapterID;
    SceneID       afterSceneID;   // insert after this scene; empty = append to end of chapter
    AuthorshipRef author;
};

struct CreateChapterRequest {
    AbsolutePath  projectRootPath;
    AbsolutePath  appSupportRoot;
    ProjectID     projectID;
    AuthorshipRef author;
};

// ---------------------------------------------------------------------------
// deleteScene / deleteChapter (EP-010 SP-027)
// ---------------------------------------------------------------------------

struct DeleteSceneRequest {
    AbsolutePath projectRootPath;
    SceneID      sceneID;
};

struct DeleteChapterRequest {
    AbsolutePath projectRootPath;
    ChapterID    chapterID;
};

// ---------------------------------------------------------------------------
// renameScene / renameChapter (EP-010 SP-028)
// ---------------------------------------------------------------------------

struct RenameSceneRequest {
    AbsolutePath projectRootPath;
    RelativePath metadataPath;  // relative path to scene.meta.json
    std::string  newTitle;      // blank/whitespace saved as ""
};

// ---------------------------------------------------------------------------
// reorderScene / reorderChapter (EP-010 SP-029)
// ---------------------------------------------------------------------------

struct ReorderSceneRequest {
    AbsolutePath projectRootPath;
    SceneID      sceneID;
    ChapterID    sourceChapterID;   // chapter currently containing the scene
    ChapterID    targetChapterID;   // chapter to move the scene into (same = reorder within)
    SceneID      afterSceneID;      // insert after this scene; empty = insert at beginning
};

struct ReorderChapterRequest {
    AbsolutePath projectRootPath;
    ChapterID    chapterID;
    ChapterID    afterChapterID;    // insert after this chapter; empty = insert at beginning
};

struct RenameChapterRequest {
    AbsolutePath projectRootPath;
    RelativePath metadataPath;  // relative path to chapter.meta.json
    std::string  newTitle;      // blank/whitespace saved as ""
};

} // namespace scrivi
