#pragma once

#include "scrivi/IDs.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace scrivi {

using Utf8Text         = std::string;
using ISO8601Timestamp = std::string;
using Slug             = std::string;
using RelativePath     = std::string;
using AbsolutePath     = std::string;

enum class ManuscriptStatus : std::uint8_t { draft, revised, final, archived };
enum class PersonaKind      : std::uint8_t { individual, group };
enum class ProjectRole      : std::uint8_t { owner, editor, reader };
enum class MemberStatus     : std::uint8_t { active, removed };

enum class OpenMode : std::uint8_t {
    normalEdit,
    editWithWarnings,
    repairRequired,
    readOnly,
    cannotOpen
};

struct TextSelection {
    std::size_t anchor = 0;
    std::size_t focus  = 0;
};

struct ScrollPosition {
    double value = 0.0;
};

struct AuthorshipRef {
    IdentityID  identityID;
    PersonaID   personaID;
    std::string displayName;
};

struct LastWritingSurface {
    SceneID      sceneID;
    RelativePath contentPath;
    TextSelection selection;
    ScrollPosition scroll;
};

struct WorkspaceState {
    ProjectID          projectID;
    std::string        deviceID;
    IdentityID         identityID;
    PersonaID          activePersonaID;
    std::optional<LastWritingSurface> lastWritingSurface;
    ISO8601Timestamp   lastOpenedAt;
};

struct ProjectSummary {
    ProjectID    projectID;
    std::string  title;
    Slug         slug;
    AbsolutePath rootPath;
    bool         gitSnapshotsEnabled = false;
};

struct SceneSummary {
    SceneID          sceneID;
    ChapterID        chapterID;
    std::string      title;
    Slug             slug;
    ManuscriptStatus status = ManuscriptStatus::draft;
    RelativePath     metadataPath;
    RelativePath     contentPath;
};

} // namespace scrivi
