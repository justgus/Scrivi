#pragma once

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

} // namespace scrivi
