#pragma once

#include "scrivi/AssetTypes.hpp"
#include "scrivi/CommentTypes.hpp"
#include "scrivi/ObjectTypes.hpp"
#include "scrivi/RepairIssue.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
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

    std::vector<SceneSummary> scenes;   // all scenes in manuscript order

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

// ---------------------------------------------------------------------------
// openScene (EP-008 T-0060)
// ---------------------------------------------------------------------------

struct OpenSceneResult {
    SceneSummary scene;
    Utf8Text     markdown;

    TextSelection  restoredSelection;
    ScrollPosition restoredScroll;
};

// ---------------------------------------------------------------------------
// createScene / createChapter (EP-009 SP-021 T-0067)
// ---------------------------------------------------------------------------

struct CreateSceneResult {
    SceneID      sceneID;
    ChapterID    chapterID;
    RelativePath metadataPath;
    RelativePath contentPath;
};

struct CreateChapterResult {
    ChapterID    chapterID;
    RelativePath chapterMetadataPath;

    SceneID      firstSceneID;
    RelativePath firstSceneMetadataPath;
    RelativePath firstSceneContentPath;
};

// ---------------------------------------------------------------------------
// deleteScene / deleteChapter (EP-010 SP-027)
// ---------------------------------------------------------------------------

struct DeleteSceneResult {
    SceneID sceneID;
    bool    deleted = false;
};

struct DeleteChapterResult {
    ChapterID chapterID;
    int       scenesDeleted = 0;
    bool      deleted       = false;
};

// ---------------------------------------------------------------------------
// renameScene / renameChapter (EP-010 SP-028)
// ---------------------------------------------------------------------------

struct RenameSceneResult {
    RelativePath metadataPath;
    std::string  newTitle;   // the value actually written (trimmed to "" if whitespace)
    bool         renamed = false;
};

// ---------------------------------------------------------------------------
// reorderScene / reorderChapter (EP-010 SP-029)
// ---------------------------------------------------------------------------

struct ReorderSceneResult {
    SceneID   sceneID;
    ChapterID sourceChapterID;
    ChapterID targetChapterID;
    bool      reordered = false;
};

struct ReorderChapterResult {
    ChapterID chapterID;
    bool      reordered = false;
};

struct RenameChapterResult {
    RelativePath metadataPath;
    std::string  newTitle;
    bool         renamed = false;
};

// ---------------------------------------------------------------------------
// Timeline results (EP-016 SP-039)
// ---------------------------------------------------------------------------

struct GetTimelineResult {
    std::string  timelineID;
    std::string  epochLabel;
    std::string  projectID;
    std::string  createdAt;
};
struct SetTimelineEpochLabelResult { bool updated = false; };

struct SetSceneStoryTimeResult     { SceneID sceneID; bool updated = false; };
struct GetSceneStoryTimeResult {
    SceneID     sceneID;
    int64_t     offsetMs            = 0;
    std::string offsetSource;
    int64_t     gapMs               = 0;
    int64_t     durationMs          = 3'600'000;
    std::string durationSource;
    std::string inferenceHint;
    double      inferenceConfidence = -1.0;
    std::string bandID;
    std::string bandAssignedAt;
};
struct ClearSceneStoryTimeResult    { SceneID sceneID; bool cleared = false; };
struct AssignSceneToBandResult      { SceneID sceneID; bool assigned = false; };
struct UnassignSceneFromBandResult  { SceneID sceneID; bool unassigned = false; };

struct GetStoryStructureResult {
    bool        hasStructure   = false;
    std::string structureID;
    std::string bandLayoutJSON;
};
struct SetStoryStructureResult    { bool set     = false; };
struct UpdateBandLayoutResult     { bool updated = false; };
struct RemoveStoryStructureResult { bool removed = false; };

struct CreateHistoricalEventResult { std::string eventID; std::string slug; };
struct UpdateHistoricalEventResult { std::string eventID; bool updated = false; };
struct DeleteHistoricalEventResult { std::string eventID; bool deleted = false; };
struct ListHistoricalEventsResult  { std::string eventsJSON; int count = 0; };

struct ImportExternalTimelineResult       { std::string timelineID; bool imported = false; };
struct UpdateImportedTimelineOffsetResult { std::string timelineID; bool updated  = false; };
struct SetImportedTimelineVisibleResult   { std::string timelineID; bool updated  = false; };
struct ListImportedTimelinesResult        { std::string timelinesJSON; int count = 0; };
struct RemoveImportedTimelineResult       { std::string timelineID; bool removed = false; };
struct ExportProjectTimelineResult        { std::string timelineJSON; };

// ---------------------------------------------------------------------------
// Searchable content (EP-017 SP-044 — Spotlight indexing facade)
// ---------------------------------------------------------------------------

// One indexable record. Mirrors the scrivi.searchableContent.v1 item schema.
// containerTitle/keywords/contentDescription are optional (empty = omit on emit).
struct SearchableItem {
    std::string uniqueIdentifier;   // "<kind>:<id>"
    std::string kind;               // project|scene|character|location|item|rule|timeline
    std::string title;
    std::string displayName;
    std::string containerTitle;     // scenes only (chapter title)
    std::string contentDescription; // plain text (Markdown stripped for scenes)
    std::vector<std::string> keywords;
    std::string deepLink;           // scrivi://open?project=<projectID>&item=<uid>
};

struct ExtractSearchableTextResult {
    std::string schema = "scrivi.searchableContent.v1";
    std::string domainIdentifier;   // projectID — the Spotlight delete-by-domain key
    AbsolutePath projectRootPath;
    std::vector<SearchableItem> items;
};

} // namespace scrivi
