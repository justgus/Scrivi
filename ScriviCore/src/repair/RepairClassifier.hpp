#pragma once

#include "scrivi/RepairIssue.hpp"
#include "scrivi/Types.hpp"

#include <string>

namespace scrivi::repair {

// Builds RepairIssue structs for each detected condition.
struct RepairClassifier {
    // --- Existing classifiers ---

    static RepairIssue missingContent(const ProjectID& projectID,
                                      const ChapterID& chapterID,
                                      const SceneID&   sceneID,
                                      const std::string& expectedPath);

    static RepairIssue missingMetadata(const ProjectID& projectID,
                                       const ChapterID& chapterID,
                                       const SceneID&   sceneID,
                                       const std::string& expectedPath);

    static RepairIssue corruptMetadata(const ProjectID& projectID,
                                       const ChapterID& chapterID,
                                       const SceneID&   sceneID,
                                       const std::string& path,
                                       const std::string& detail);

    static RepairIssue unregisteredFile(const ProjectID& projectID,
                                        const std::string& path);

    // --- T-0031: Rename detection classifiers ---

    // A .md file was renamed externally; metadata still references the old path.
    // currentContentPath: new (on-disk) absolute path of the .md file.
    // expectedContentPath: what the metadata currently points to (the old path).
    static RepairIssue possibleRename(const ProjectID&   projectID,
                                      const ChapterID&   chapterID,
                                      const SceneID&     sceneID,
                                      const std::string& currentContentPath,
                                      const std::string& expectedContentPath);

    // A .meta.json file was renamed externally; the chapter entry still
    // references the old metadata path but the sceneID inside the file matches.
    // currentMetaPath: new (on-disk) absolute path of the .meta.json file.
    // expectedMetaPath: what the chapter's scene list currently points to.
    static RepairIssue possibleMetadataRename(const ProjectID&   projectID,
                                              const ChapterID&   chapterID,
                                              const SceneID&     sceneID,
                                              const std::string& currentMetaPath,
                                              const std::string& expectedMetaPath);

    // Both .md and .meta.json were renamed together; sceneID inside metadata
    // matches, and the contentPath inside metadata matches the new .md filename.
    static RepairIssue possiblePairedRename(const ProjectID&   projectID,
                                            const ChapterID&   chapterID,
                                            const SceneID&     sceneID,
                                            const std::string& currentMetaPath,
                                            const std::string& currentContentPath);

    // A chapter folder was renamed externally; manuscript.meta.json still
    // references the old path, but the chapterID inside the folder matches.
    // currentFolderPath: new (on-disk) absolute path to the chapter directory.
    // expectedChapterPath: what manuscript.meta.json currently points to.
    static RepairIssue possibleChapterFolderRename(const ProjectID&   projectID,
                                                   const ChapterID&   chapterID,
                                                   const std::string& currentFolderPath,
                                                   const std::string& expectedChapterPath);
};

} // namespace scrivi::repair
