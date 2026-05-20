#include "repair/RepairClassifier.hpp"

namespace scrivi::repair {

RepairIssue RepairClassifier::missingContent(
    const ProjectID& projectID,
    const ChapterID& chapterID,
    const SceneID&   sceneID,
    const std::string& expectedPath)
{
    RepairIssue issue;
    issue.severity  = RepairSeverity::blocking;
    issue.category  = RepairCategory::missingContent;
    issue.title     = "Missing scene content file";
    issue.message   = "The Markdown file for this scene could not be found.";
    issue.path      = expectedPath;
    issue.projectID = projectID;
    issue.chapterID = chapterID;
    issue.sceneID   = sceneID;
    issue.suggestedActions.push_back({
        RepairActionKind::createEmptyContentFile,
        "Create empty file",
        "Create an empty .md file at the expected path."
    });
    issue.suggestedActions.push_back({
        RepairActionKind::markMissing,
        "Mark as missing",
        "Keep the scene in the index but flag it as missing."
    });
    return issue;
}

RepairIssue RepairClassifier::missingMetadata(
    const ProjectID& projectID,
    const ChapterID& chapterID,
    const SceneID&   sceneID,
    const std::string& expectedPath)
{
    RepairIssue issue;
    issue.severity  = RepairSeverity::blocking;
    issue.category  = RepairCategory::missingMetadata;
    issue.title     = "Missing scene metadata";
    issue.message   = "The .meta.json file for this scene could not be found.";
    issue.path      = expectedPath;
    issue.projectID = projectID;
    issue.chapterID = chapterID;
    issue.sceneID   = sceneID;
    issue.suggestedActions.push_back({
        RepairActionKind::regenerateMetadata,
        "Regenerate metadata",
        "Create a new .meta.json from the existing content file."
    });
    return issue;
}

RepairIssue RepairClassifier::corruptMetadata(
    const ProjectID& projectID,
    const ChapterID& chapterID,
    const SceneID&   sceneID,
    const std::string& path,
    const std::string& detail)
{
    RepairIssue issue;
    issue.severity  = RepairSeverity::blocking;
    issue.category  = RepairCategory::corruptMetadata;
    issue.title     = "Corrupt scene metadata";
    issue.message   = detail;
    issue.path      = path;
    issue.projectID = projectID;
    issue.chapterID = chapterID;
    issue.sceneID   = sceneID;
    issue.suggestedActions.push_back({
        RepairActionKind::regenerateMetadata,
        "Regenerate metadata",
        "Replace the corrupt .meta.json with a freshly generated one."
    });
    issue.suggestedActions.push_back({
        RepairActionKind::restoreFromSnapshot,
        "Restore from snapshot",
        "Restore the last known good version from a Git snapshot."
    });
    return issue;
}

RepairIssue RepairClassifier::unregisteredFile(
    const ProjectID& projectID,
    const std::string& path)
{
    RepairIssue issue;
    issue.severity  = RepairSeverity::warning;
    issue.category  = RepairCategory::unregisteredManuscriptFile;
    issue.title     = "Unregistered Markdown file";
    issue.message   = "A .md file was found with no matching metadata entry.";
    issue.path      = path;
    issue.projectID = projectID;
    issue.suggestedActions.push_back({
        RepairActionKind::importAsNewScene,
        "Import as new scene",
        "Create metadata for this file and add it to the manuscript."
    });
    issue.suggestedActions.push_back({
        RepairActionKind::moveToInbox,
        "Move to inbox",
        "Move to the project inbox for later review."
    });
    issue.suggestedActions.push_back({
        RepairActionKind::ignore,
        "Ignore",
        "Leave the file in place without registering it."
    });
    return issue;
}

} // namespace scrivi::repair
