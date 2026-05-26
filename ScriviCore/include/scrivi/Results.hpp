#pragma once

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

} // namespace scrivi
