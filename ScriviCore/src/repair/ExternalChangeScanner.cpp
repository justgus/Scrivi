#include "repair/ExternalChangeScanner.hpp"

#include "repair/RepairClassifier.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/ProjectJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <set>
#include <string>

namespace scrivi::repair {

ExternalChangeScanner::ExternalChangeScanner(CoreServices& services)
    : services_(services) {}

Result<ExternalChangeScanResult> ExternalChangeScanner::scan(
    const ExternalChangeScanRequest& request)
{
    auto& fs = *services_.fileSystem;

    // Read project.json for projectID
    auto projTextR = fs.readTextFile(util::join(request.projectRootPath, "project.json"));
    if (!projTextR.ok()) return Result<ExternalChangeScanResult>::failure(projTextR.error());

    auto projParsed = schemas::parseProject(projTextR.value());
    if (!projParsed.ok()) return Result<ExternalChangeScanResult>::failure(projParsed.error());

    const ProjectID projectID = projParsed.value().projectID;

    ExternalChangeScanResult result;
    result.projectID = projectID;

    // Read manuscript.meta.json
    auto msPath  = util::join(request.projectRootPath, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msPath);
    if (!msTextR.ok()) return Result<ExternalChangeScanResult>::failure(msTextR.error());

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) return Result<ExternalChangeScanResult>::failure(msParsed.error());

    // Track all registered content paths so we can detect unregistered files
    std::set<std::string> registeredContentPaths;

    for (auto& chapterRef : msParsed.value().chapters) {
        auto chPath   = util::join(request.projectRootPath, chapterRef.path);
        auto chTextR  = fs.readTextFile(chPath);

        if (!chTextR.ok()) {
            // chapter.meta.json missing
            result.repairIssues.push_back(
                RepairClassifier::missingMetadata(
                    projectID, chapterRef.chapterID, SceneID{""}, chPath));
            continue;
        }

        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) {
            result.repairIssues.push_back(
                RepairClassifier::corruptMetadata(
                    projectID, chapterRef.chapterID, SceneID{""}, chPath,
                    chParsed.error().message));
            continue;
        }

        for (auto& sceneRef : chParsed.value().scenes) {
            auto sMetaPath   = util::join(request.projectRootPath, sceneRef.metadataPath);
            auto sMetaTextR  = fs.readTextFile(sMetaPath);

            // Derive expected content path from metadata path (swap .meta.json → .md)
            const std::string expectedContentPath =
                util::replaceExtension(
                    util::replaceExtension(sceneRef.metadataPath, ""), ".md");

            if (!sMetaTextR.ok()) {
                result.repairIssues.push_back(
                    RepairClassifier::missingMetadata(
                        projectID, chapterRef.chapterID, sceneRef.sceneID, sMetaPath));
                // Register expected content path so filesystem walk doesn't flag it
                registeredContentPaths.insert(expectedContentPath);
                continue;
            }

            auto sParsed = schemas::parseSceneMeta(sMetaTextR.value());
            if (!sParsed.ok()) {
                result.repairIssues.push_back(
                    RepairClassifier::corruptMetadata(
                        projectID, chapterRef.chapterID, sceneRef.sceneID, sMetaPath,
                        sParsed.error().message));
                // Register expected content path so filesystem walk doesn't flag it
                registeredContentPaths.insert(expectedContentPath);
                continue;
            }

            const std::string contentPath = sParsed.value().contentPath;
            registeredContentPaths.insert(contentPath);

            auto absContentPath  = util::join(request.projectRootPath, contentPath);
            auto contentExistsR  = fs.exists(absContentPath);
            if (!contentExistsR.ok())
                return Result<ExternalChangeScanResult>::failure(contentExistsR.error());

            if (!contentExistsR.value()) {
                result.repairIssues.push_back(
                    RepairClassifier::missingContent(
                        projectID, chapterRef.chapterID, sceneRef.sceneID, absContentPath));
            }
        }
    }

    // Walk the manuscript directory looking for unregistered .md files
    auto msDir = util::join(request.projectRootPath, "manuscript");
    auto listR = fs.listDirectory(msDir);
    if (listR.ok()) {
        for (auto& chapterDirPath : listR.value()) {
            auto isDirR = fs.isDirectory(chapterDirPath);
            if (!isDirR.ok() || !isDirR.value()) continue;

            auto filesR = fs.listDirectory(chapterDirPath);
            if (!filesR.ok()) continue;

            for (auto& filePath : filesR.value()) {
                if (util::extension(filePath) != ".md") continue;

                // Build relative path from projectRoot
                // filePath is absolute; strip projectRootPath prefix + leading slash
                std::string rel = filePath.substr(request.projectRootPath.size() + 1);

                if (registeredContentPaths.find(rel) == registeredContentPaths.end()) {
                    result.repairIssues.push_back(
                        RepairClassifier::unregisteredFile(projectID, filePath));
                    result.indexesDirty = true;
                }
            }
        }
    }

    // Optional Git status check
    if (request.includeGitStatus && services_.gitProvider != nullptr) {
        auto isRepoR = services_.gitProvider->isRepository(request.projectRootPath);
        if (isRepoR.ok() && isRepoR.value()) {
            auto statusR = services_.gitProvider->status(request.projectRootPath);
            if (statusR.ok()) {
                result.gitStatusChecked = true;
                result.hasUnsnapshottedChanges =
                    statusR.value().hasUncommittedChanges ||
                    !statusR.value().untrackedFiles.empty();
            }
        }
    }

    return Result<ExternalChangeScanResult>::success(std::move(result));
}

} // namespace scrivi::repair
