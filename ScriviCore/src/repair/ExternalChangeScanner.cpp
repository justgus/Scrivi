#include "repair/ExternalChangeScanner.hpp"

#include "repair/RepairClassifier.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/ProjectJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace scrivi::repair {

ExternalChangeScanner::ExternalChangeScanner(CoreServices& services)
    : services_(services) {}

// ---------------------------------------------------------------------------
// Internal rename-detection helpers
// ---------------------------------------------------------------------------

// Holds info about a scene whose registered .meta.json is missing (needed for
// rename detection cross-reference).
struct MissingMetaInfo {
    ChapterID   chapterID;
    SceneID     sceneID;
    std::string expectedMetaPath;   // absolute path that was expected
    std::string chapterMetaPath;    // absolute path to the chapter.meta.json
};

// Try to apply an unambiguous metadata rename in-place: update the chapter's
// scene list to point at the new metadata path, then rewrite chapter.meta.json.
static void autoApplyMetadataRename(
    FileSystem&                    fs,
    const std::string&             chapterMetaPath,
    const schemas::ChapterMetaData& chData,
    const SceneID&                 sceneID,
    const std::string&             newMetaAbsPath,
    const std::string&             projectRootPath,
    ExternalChangeScanResult&      result)
{
    // Build relative new meta path
    std::string relNewMeta = newMetaAbsPath;
    if (relNewMeta.size() > projectRootPath.size() &&
        relNewMeta.substr(0, projectRootPath.size()) == projectRootPath)
    {
        relNewMeta = relNewMeta.substr(projectRootPath.size() + 1);
    }

    // Copy chapter data and update the matching scene entry
    schemas::ChapterMetaData updated = chData;
    for (auto& ref : updated.scenes) {
        if (ref.sceneID.value == sceneID.value) {
            ref.metadataPath = relNewMeta;
            break;
        }
    }

    // Write .bak before rewriting
    auto bakPath = chapterMetaPath + ".bak";
    auto origR   = fs.readTextFile(chapterMetaPath);
    if (origR.ok()) fs.atomicWriteTextFile(bakPath, origR.value());

    fs.atomicWriteTextFile(chapterMetaPath, schemas::serializeChapterMeta(updated));
    result.indexesDirty = true;
}

// Try to apply an unambiguous chapter folder rename in-place: update the
// manuscript.meta.json chapter entry to point at the new folder's meta path.
static void autoApplyChapterFolderRename(
    FileSystem&                       fs,
    const std::string&                msMetaPath,
    const schemas::ManuscriptMetaData& msData,
    const ChapterID&                  chapterID,
    const std::string&                newChMetaAbsPath,
    const std::string&                projectRootPath,
    ExternalChangeScanResult&         result)
{
    // Build relative new chapter meta path
    std::string relNewChMeta = newChMetaAbsPath;
    if (relNewChMeta.size() > projectRootPath.size() &&
        relNewChMeta.substr(0, projectRootPath.size()) == projectRootPath)
    {
        relNewChMeta = relNewChMeta.substr(projectRootPath.size() + 1);
    }

    schemas::ManuscriptMetaData updated = msData;
    for (auto& ref : updated.chapters) {
        if (ref.chapterID.value == chapterID.value) {
            ref.path = relNewChMeta;
            break;
        }
    }

    auto bakPath = msMetaPath + ".bak";
    auto origR   = fs.readTextFile(msMetaPath);
    if (origR.ok()) fs.atomicWriteTextFile(bakPath, origR.value());

    fs.atomicWriteTextFile(msMetaPath, schemas::serializeManuscriptMeta(updated));
    result.indexesDirty = true;
}

// ---------------------------------------------------------------------------
// scan
// ---------------------------------------------------------------------------

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
    const std::string msMetaPath = util::join(request.projectRootPath,
                                              "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) return Result<ExternalChangeScanResult>::failure(msTextR.error());

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) return Result<ExternalChangeScanResult>::failure(msParsed.error());

    const auto& msData = msParsed.value();

    // Track all registered content paths (to detect unregistered .md files)
    std::set<std::string> registeredContentPaths;

    // Track registered chapter meta paths (to detect chapter folder renames)
    std::set<std::string> registeredChapterMetaPaths;

    // Gather info about scenes whose metadata file is missing (for rename detection)
    std::vector<MissingMetaInfo> missingMetas;

    // Chapters whose folder is missing entirely (for chapter folder rename detection)
    // Maps chapterID → expected chapter meta path
    std::map<std::string, std::string> missingChapterFolders;

    // ---------------------------------------------------------------------------
    // Main scan loop
    // ---------------------------------------------------------------------------
    for (auto& chapterRef : msData.chapters) {
        auto chPath = util::join(request.projectRootPath, chapterRef.path);
        registeredChapterMetaPaths.insert(chPath);

        auto chTextR = fs.readTextFile(chPath);

        if (!chTextR.ok()) {
            // chapter.meta.json missing — may be a folder rename
            result.repairIssues.push_back(
                RepairClassifier::missingMetadata(
                    projectID, chapterRef.chapterID, SceneID{""}, chPath));
            missingChapterFolders[chapterRef.chapterID.value] = chPath;
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

        const auto& chData = chParsed.value();

        for (auto& sceneRef : chData.scenes) {
            auto sMetaPath  = util::join(request.projectRootPath, sceneRef.metadataPath);
            auto sMetaTextR = fs.readTextFile(sMetaPath);

            // Derive expected content path from metadata path (swap .meta.json → .md)
            const std::string expectedContentPath =
                util::replaceExtension(
                    util::replaceExtension(sceneRef.metadataPath, ""), ".md");

            if (!sMetaTextR.ok()) {
                result.repairIssues.push_back(
                    RepairClassifier::missingMetadata(
                        projectID, chapterRef.chapterID, sceneRef.sceneID, sMetaPath));
                registeredContentPaths.insert(expectedContentPath);
                // Record for rename detection
                missingMetas.push_back({
                    chapterRef.chapterID,
                    sceneRef.sceneID,
                    sMetaPath,
                    chPath
                });
                continue;
            }

            auto sParsed = schemas::parseSceneMeta(sMetaTextR.value());
            if (!sParsed.ok()) {
                result.repairIssues.push_back(
                    RepairClassifier::corruptMetadata(
                        projectID, chapterRef.chapterID, sceneRef.sceneID, sMetaPath,
                        sParsed.error().message));
                registeredContentPaths.insert(expectedContentPath);
                continue;
            }

            const std::string contentPath = sParsed.value().contentPath;
            registeredContentPaths.insert(contentPath);

            auto absContentPath = util::join(request.projectRootPath, contentPath);
            auto contentExistsR = fs.exists(absContentPath);
            if (!contentExistsR.ok())
                return Result<ExternalChangeScanResult>::failure(contentExistsR.error());

            if (!contentExistsR.value()) {
                result.repairIssues.push_back(
                    RepairClassifier::missingContent(
                        projectID, chapterRef.chapterID, sceneRef.sceneID, absContentPath));
            }
        }
    }

    // ---------------------------------------------------------------------------
    // Filesystem walk: unregistered .md files + collect orphan .meta.json files
    // ---------------------------------------------------------------------------

    // Maps sceneID.value → list of orphan meta absolute paths (for rename detection)
    std::map<std::string, std::vector<std::string>> orphanMetasBySceneID;

    const auto msDir = util::join(request.projectRootPath, "manuscript");
    auto listR = fs.listDirectory(msDir);
    if (listR.ok()) {
        for (auto& chapterDirPath : listR.value()) {
            auto isDirR = fs.isDirectory(chapterDirPath);
            if (!isDirR.ok() || !isDirR.value()) continue;

            auto filesR = fs.listDirectory(chapterDirPath);
            if (!filesR.ok()) continue;

            for (auto& filePath : filesR.value()) {
                const auto ext = util::extension(filePath);

                if (ext == ".md") {
                    std::string rel = filePath.substr(request.projectRootPath.size() + 1);
                    if (registeredContentPaths.find(rel) == registeredContentPaths.end()) {
                        result.repairIssues.push_back(
                            RepairClassifier::unregisteredFile(projectID, filePath));
                        result.indexesDirty = true;
                    }
                } else if (ext == ".json") {
                    // Could be an orphan .meta.json — check if it's registered
                    if (registeredChapterMetaPaths.find(filePath) !=
                        registeredChapterMetaPaths.end())
                        continue; // it's a registered chapter meta
                    if (util::filename(filePath) == "chapter.meta.json")
                        continue; // handled separately below

                    // Try to parse as scene metadata to get sceneID
                    auto textR = fs.readTextFile(filePath);
                    if (!textR.ok()) continue;
                    auto parsed = schemas::parseSceneMeta(textR.value());
                    if (!parsed.ok()) continue;

                    // Only treat as orphan if its path is not a registered meta path
                    // (registered meta paths are those in chapter scene lists —
                    //  we didn't track them above, so check by absence from
                    //  registeredContentPaths is wrong; we use sceneID uniqueness instead)
                    orphanMetasBySceneID[parsed.value().sceneID.value].push_back(filePath);
                }
            }
        }
    }

    // ---------------------------------------------------------------------------
    // T-0031: Metadata rename detection
    // For each missingMetadata issue, look for an orphan .meta.json with
    // a matching sceneID in the same chapter directory.
    // ---------------------------------------------------------------------------
    for (auto& missing : missingMetas) {
        const auto& sceneIDVal = missing.sceneID.value;
        auto it = orphanMetasBySceneID.find(sceneIDVal);
        if (it == orphanMetasBySceneID.end()) continue; // no candidate found

        const auto& candidates = it->second;

        // Determine expected chapter directory
        const auto expectedChDir = util::parent(missing.expectedMetaPath);

        // Filter candidates to those in the same chapter directory
        std::vector<std::string> sameDirCandidates;
        for (const auto& cPath : candidates) {
            if (util::parent(cPath) == expectedChDir)
                sameDirCandidates.push_back(cPath);
        }

        if (sameDirCandidates.empty()) {
            // Candidates exist but in different directories — stage ambiguous issue
            if (!candidates.empty()) {
                result.repairIssues.push_back(
                    RepairClassifier::possibleMetadataRename(
                        projectID, missing.chapterID, missing.sceneID,
                        candidates.front(), missing.expectedMetaPath));
            }
            continue;
        }

        if (sameDirCandidates.size() == 1) {
            // Unambiguous single candidate in the same chapter dir — auto-apply.
            // Read the chapter meta to perform the update.
            auto chTextR = fs.readTextFile(missing.chapterMetaPath);
            if (chTextR.ok()) {
                auto chParsed = schemas::parseChapterMeta(chTextR.value());
                if (chParsed.ok()) {
                    autoApplyMetadataRename(
                        fs,
                        missing.chapterMetaPath,
                        chParsed.value(),
                        missing.sceneID,
                        sameDirCandidates.front(),
                        request.projectRootPath,
                        result);
                    // Remove the corresponding missingMetadata issue we already staged
                    result.repairIssues.erase(
                        std::remove_if(result.repairIssues.begin(), result.repairIssues.end(),
                            [&](const RepairIssue& i) {
                                return i.category == RepairCategory::missingMetadata &&
                                       i.sceneID.value == sceneIDVal;
                            }),
                        result.repairIssues.end());
                }
            }
        } else {
            // Multiple candidates in same dir — stage for user review
            result.repairIssues.push_back(
                RepairClassifier::possibleMetadataRename(
                    projectID, missing.chapterID, missing.sceneID,
                    sameDirCandidates.front(), missing.expectedMetaPath));
        }
    }

    // ---------------------------------------------------------------------------
    // T-0031: Chapter folder rename detection
    // For each chapter whose folder was missing, look for an unregistered
    // subdirectory of manuscript/ that contains a chapter.meta.json with a
    // matching chapterID.
    // ---------------------------------------------------------------------------
    if (!missingChapterFolders.empty() && listR.ok()) {
        for (auto& chapterDirPath : listR.value()) {
            auto isDirR = fs.isDirectory(chapterDirPath);
            if (!isDirR.ok() || !isDirR.value()) continue;

            // Skip registered chapter directories
            // A registered chapter dir is the parent of a registered chapter meta path
            bool alreadyRegistered = false;
            for (const auto& regPath : registeredChapterMetaPaths) {
                if (util::parent(regPath) == chapterDirPath) {
                    alreadyRegistered = true;
                    break;
                }
            }
            if (alreadyRegistered) continue;

            // Look for chapter.meta.json in this unregistered directory
            auto candidateChMeta = util::join(chapterDirPath, "chapter.meta.json");
            auto chTextR         = fs.readTextFile(candidateChMeta);
            if (!chTextR.ok()) continue;

            auto chParsed = schemas::parseChapterMeta(chTextR.value());
            if (!chParsed.ok()) continue;

            const auto& foundChapterID = chParsed.value().chapterID.value;
            auto mIt = missingChapterFolders.find(foundChapterID);
            if (mIt == missingChapterFolders.end()) continue;

            // Found a match — auto-apply (single candidate guaranteed since
            // chapterIDs are unique and we've found the first match)
            auto msTextR2 = fs.readTextFile(msMetaPath);
            if (msTextR2.ok()) {
                auto msParsed2 = schemas::parseManuscriptMeta(msTextR2.value());
                if (msParsed2.ok()) {
                    autoApplyChapterFolderRename(
                        fs,
                        msMetaPath,
                        msParsed2.value(),
                        ChapterID{foundChapterID},
                        candidateChMeta,
                        request.projectRootPath,
                        result);
                    // Remove the corresponding missingMetadata issue for this chapter
                    result.repairIssues.erase(
                        std::remove_if(result.repairIssues.begin(), result.repairIssues.end(),
                            [&](const RepairIssue& i) {
                                return i.category == RepairCategory::missingMetadata &&
                                       i.chapterID.value == foundChapterID &&
                                       i.sceneID.value.empty();
                            }),
                        result.repairIssues.end());
                }
            }
            // Remove from missing set so we don't process it again
            missingChapterFolders.erase(mIt);
        }
    }

    // ---------------------------------------------------------------------------
    // Optional Git status check
    // ---------------------------------------------------------------------------
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
