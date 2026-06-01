#include "repair/RepairClassifier.hpp"
#include "util/Hash.hpp"

#include <string>

namespace scrivi::repair {

// Generate a stable issueID from category + primary IDs + path.
// Using a short prefix + truncated SHA-256 keeps it readable and unique.
static std::string makeIssueID(RepairCategory cat,
                                const SceneID&   sceneID,
                                const ChapterID& chapterID,
                                const std::string& path)
{
    std::string input = std::to_string(static_cast<int>(cat))
                      + "|" + sceneID.value
                      + "|" + chapterID.value
                      + "|" + path;
    return "issue-" + util::sha256Hex(input).substr(0, 16);
}

RepairIssue RepairClassifier::missingContent(
    const ProjectID& projectID,
    const ChapterID& chapterID,
    const SceneID&   sceneID,
    const std::string& expectedPath)
{
    RepairIssue issue;
    issue.issueID   = makeIssueID(RepairCategory::missingContent, sceneID, chapterID, expectedPath);
    issue.severity  = RepairSeverity::blocking;
    issue.category  = RepairCategory::missingContent;
    issue.title     = "Missing scene content file";
    issue.message   = "The Markdown file for this scene could not be found.";
    issue.path      = expectedPath;
    issue.projectID = projectID;
    issue.chapterID = chapterID;
    issue.sceneID   = sceneID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::relinkToFile,
        .label="Relink to another file",
        .detail="Point the scene metadata to a different .md file."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::createEmptyContentFile,
        .label="Create empty file",
        .detail="Create an empty .md file at the expected path."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::markMissing,
        .label="Mark as missing",
        .detail="Keep the scene in the index but flag it as missing."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::removeFromProject,
        .label="Remove from project",
        .detail="Remove the scene entry from the chapter; files are preserved on disk."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::restoreFromSnapshot,
        .label="Restore from snapshot",
        .detail="Restore the last known good version from a Git snapshot."
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
    issue.issueID   = makeIssueID(RepairCategory::missingMetadata, sceneID, chapterID, expectedPath);
    issue.severity  = RepairSeverity::blocking;
    issue.category  = RepairCategory::missingMetadata;
    issue.title     = "Missing scene metadata";
    issue.message   = "The .meta.json file for this scene could not be found.";
    issue.path      = expectedPath;
    issue.projectID = projectID;
    issue.chapterID = chapterID;
    issue.sceneID   = sceneID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::regenerateMetadata,
        .label="Regenerate metadata",
        .detail="Create a new .meta.json from the existing content file."
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
    issue.issueID   = makeIssueID(RepairCategory::corruptMetadata, sceneID, chapterID, path);
    issue.severity  = RepairSeverity::blocking;
    issue.category  = RepairCategory::corruptMetadata;
    issue.title     = "Corrupt scene metadata";
    issue.message   = detail;
    issue.path      = path;
    issue.projectID = projectID;
    issue.chapterID = chapterID;
    issue.sceneID   = sceneID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::regenerateMetadata,
        .label="Regenerate metadata",
        .detail="Replace the corrupt .meta.json with a freshly generated one."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::restoreFromSnapshot,
        .label="Restore from snapshot",
        .detail="Restore the last known good version from a Git snapshot."
    });
    return issue;
}

RepairIssue RepairClassifier::unregisteredFile(
    const ProjectID& projectID,
    const std::string& path)
{
    RepairIssue issue;
    issue.issueID   = makeIssueID(RepairCategory::unregisteredManuscriptFile,
                                  SceneID{""}, ChapterID{""}, path);
    issue.severity  = RepairSeverity::warning;
    issue.category  = RepairCategory::unregisteredManuscriptFile;
    issue.title     = "Unregistered Markdown file";
    issue.message   = "A .md file was found with no matching metadata entry.";
    issue.path      = path;
    issue.projectID = projectID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::importAsNewScene,
        .label="Import as new scene",
        .detail="Create metadata for this file and add it to the manuscript."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::moveToInbox,
        .label="Move to inbox",
        .detail="Move to the project inbox for later review."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::ignore,
        .label="Ignore",
        .detail="Leave the file in place without registering it."
    });
    return issue;
}

// ---------------------------------------------------------------------------
// T-0031: Rename detection classifiers
// ---------------------------------------------------------------------------

RepairIssue RepairClassifier::possibleRename(
    const ProjectID&   projectID,
    const ChapterID&   chapterID,
    const SceneID&     sceneID,
    const std::string& currentContentPath,
    const std::string& expectedContentPath)
{
    RepairIssue issue;
    issue.issueID     = makeIssueID(RepairCategory::possibleRename,
                                    sceneID, chapterID, currentContentPath);
    issue.severity    = RepairSeverity::warning;
    issue.category    = RepairCategory::possibleRename;
    issue.title       = "Possible scene file rename";
    issue.message     = "A manuscript file may have been renamed externally. "
                        "The metadata still references the old filename.";
    issue.path        = currentContentPath;   // new (actual) location
    issue.relatedPath = expectedContentPath;  // old (referenced) location
    issue.projectID   = projectID;
    issue.chapterID   = chapterID;
    issue.sceneID     = sceneID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::relinkToFile,
        .label="Accept rename",
        .detail="Update scene metadata to reference the new filename."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::ignore,
        .label="Ignore",
        .detail="Leave metadata unchanged for now."
    });
    return issue;
}

RepairIssue RepairClassifier::possibleMetadataRename(
    const ProjectID&   projectID,
    const ChapterID&   chapterID,
    const SceneID&     sceneID,
    const std::string& currentMetaPath,
    const std::string& expectedMetaPath)
{
    RepairIssue issue;
    issue.issueID     = makeIssueID(RepairCategory::possibleRename,
                                    sceneID, chapterID, currentMetaPath);
    issue.severity    = RepairSeverity::warning;
    issue.category    = RepairCategory::possibleRename;
    issue.title       = "Possible scene metadata rename";
    issue.message     = "A metadata file may have been renamed externally. "
                        "The chapter entry still references the old path.";
    issue.path        = currentMetaPath;   // new (actual) location
    issue.relatedPath = expectedMetaPath;  // old (registered) path
    issue.projectID   = projectID;
    issue.chapterID   = chapterID;
    issue.sceneID     = sceneID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::relinkToFile,
        .label="Accept rename",
        .detail="Update the chapter's scene list to reference the new metadata path."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::ignore,
        .label="Ignore",
        .detail="Leave the chapter entry unchanged for now."
    });
    return issue;
}

RepairIssue RepairClassifier::possiblePairedRename(
    const ProjectID&   projectID,
    const ChapterID&   chapterID,
    const SceneID&     sceneID,
    const std::string& currentMetaPath,
    const std::string& currentContentPath)
{
    RepairIssue issue;
    issue.issueID     = makeIssueID(RepairCategory::possibleRename,
                                    sceneID, chapterID, currentMetaPath + "|" + currentContentPath);
    issue.severity    = RepairSeverity::warning;
    issue.category    = RepairCategory::possibleRename;
    issue.title       = "Possible scene file pair rename";
    issue.message     = "Both a scene's manuscript file and its metadata appear "
                        "to have been renamed together externally.";
    issue.path        = currentMetaPath;
    issue.relatedPath = currentContentPath;
    issue.projectID   = projectID;
    issue.chapterID   = chapterID;
    issue.sceneID     = sceneID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::relinkToFile,
        .label="Accept paired rename",
        .detail="Update the chapter's scene list and metadata contentPath to the new names."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::ignore,
        .label="Ignore",
        .detail="Leave both entries unchanged for now."
    });
    return issue;
}

RepairIssue RepairClassifier::possibleChapterFolderRename(
    const ProjectID&   projectID,
    const ChapterID&   chapterID,
    const std::string& currentFolderPath,
    const std::string& expectedChapterPath)
{
    RepairIssue issue;
    issue.issueID     = makeIssueID(RepairCategory::possibleRename,
                                    SceneID{""}, chapterID, currentFolderPath);
    issue.severity    = RepairSeverity::warning;
    issue.category    = RepairCategory::possibleRename;
    issue.title       = "Possible chapter folder rename";
    issue.message     = "A chapter folder may have been renamed externally. "
                        "The manuscript still references the old folder path.";
    issue.path        = currentFolderPath;    // new (actual) folder
    issue.relatedPath = expectedChapterPath;  // old (registered) path
    issue.projectID   = projectID;
    issue.chapterID   = chapterID;
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::relinkToFile,
        .label="Accept folder rename",
        .detail="Update manuscript metadata to reference the new chapter folder path."
    });
    issue.suggestedActions.push_back({
        .kind=RepairActionKind::ignore,
        .label="Ignore",
        .detail="Leave the manuscript entry unchanged for now."
    });
    return issue;
}

} // namespace scrivi::repair
