#pragma once

#include "scrivi/IDs.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace scrivi {

enum class RepairSeverity : std::uint8_t { info, warning, blocking };

enum class RepairCategory : std::uint8_t {
    none,
    safeExternalEdit,
    unregisteredManuscriptFile,
    missingContent,
    missingMetadata,
    possibleRename,
    orphanMetadata,
    corruptMetadata,
    unsupportedSchema,
    gitStateChanged,
    mergeConflict,
    unknownFile,
    unknownIssue
};

enum class RepairActionKind : std::uint8_t {
    none,
    reloadExternalVersion,
    keepCurrentVersion,
    saveCurrentVersionAsCopy,
    importAsNewScene,
    attachToExistingScene,
    regenerateMetadata,
    restoreFromSnapshot,
    createEmptyContentFile,
    relinkToFile,
    markMissing,
    removeFromProject,
    moveToInbox,
    ignore,
    deleteAfterConfirmation,
    openReadOnly,
    cancelOpen
};

struct RepairAction {
    RepairActionKind kind = RepairActionKind::none;
    std::string label;
    std::string detail;
};

struct RepairIssue {
    std::string issueID;

    RepairSeverity severity = RepairSeverity::info;
    RepairCategory category = RepairCategory::none;

    std::string title;
    std::string message;
    std::string path;
    std::string relatedPath;

    ProjectID projectID;
    ChapterID chapterID;
    SceneID   sceneID;

    std::vector<RepairAction> suggestedActions;
};

} // namespace scrivi
