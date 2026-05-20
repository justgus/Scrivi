#pragma once

#include "scrivi/RepairIssue.hpp"
#include "scrivi/Types.hpp"

#include <string>

namespace scrivi::repair {

// Builds RepairIssue structs for each detected condition.
struct RepairClassifier {
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
};

} // namespace scrivi::repair
