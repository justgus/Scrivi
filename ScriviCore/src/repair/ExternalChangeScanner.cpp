#include "repair/ExternalChangeScanner.hpp"

#include "repair/RepairClassifier.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/ProjectJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <filesystem>

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
        relNewMeta.starts_with(projectRootPath))
    {
        relNewMeta = relNewMeta.substr(projectRootPath.size() + 1);
    }

    // EP-027 §8.1: scene refs are filename-only. The external rename left one ref whose file
    // is GONE (the stale ref) and the moved file as an orphan on disk. Rewrite the ref whose
    // file no longer exists to the orphan's bare filename. (The orphan's sidecar carries the
    // authoritative sceneID, passed in for the result message — identity is derived, never
    // duplicated in the ref.)
    (void)sceneID;
    const std::string chapterDir =
        std::filesystem::path(chapterMetaPath).parent_path().string();
    const std::string newFilename =
        std::filesystem::path(relNewMeta).filename().string();

    schemas::ChapterMetaData updated = chData;
    for (auto& ref : updated.scenes) {
        if (ref.metadataFilename == newFilename) { break; }   // already points at it
        const std::string refMetaAbs = util::join(chapterDir, ref.metadataFilename);
        auto existsR = fs.exists(refMetaAbs);
        if (existsR.ok() && !existsR.value()) {
            ref.metadataFilename = newFilename;   // stale ref → the moved file
            break;
        }
    }

    // Write .bak before rewriting
    auto bakPath = chapterMetaPath + ".bak";
    auto origR   = fs.readTextFile(chapterMetaPath);
    if (origR.ok()) { fs.atomicWriteTextFile(bakPath, origR.value());
}

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
        relNewChMeta.starts_with(projectRootPath))
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
    if (origR.ok()) { fs.atomicWriteTextFile(bakPath, origR.value());
}

    fs.atomicWriteTextFile(msMetaPath, schemas::serializeManuscriptMeta(updated));
    result.indexesDirty = true;
}

// ---------------------------------------------------------------------------
// scan
// ---------------------------------------------------------------------------

Result<ExternalChangeScanResult> ExternalChangeScanner::scan(
    const ExternalChangeScanRequest& request) const
{
    auto& fs = *services_.fileSystem;

    // Read project.json for projectID
    auto projTextR = fs.readTextFile(util::join(request.projectRootPath, "project.json"));
    if (!projTextR.ok()) { return Result<ExternalChangeScanResult>::failure(projTextR.error());
}

    auto projParsed = schemas::parseProject(projTextR.value());
    if (!projParsed.ok()) { return Result<ExternalChangeScanResult>::failure(projParsed.error());
}

    const ProjectID projectID = projParsed.value().projectID;

    ExternalChangeScanResult result;
    result.projectID = projectID;

    // Read manuscript.meta.json
    const std::string msMetaPath = util::join(request.projectRootPath,
                                              "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<ExternalChangeScanResult>::failure(msTextR.error());
}

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<ExternalChangeScanResult>::failure(msParsed.error());
}

    const auto& msData = msParsed.value();

    // Track all registered content paths (to detect unregistered .md files)
    std::set<std::string> registeredContentPaths;

    // Track registered chapter meta paths (to detect chapter folder renames)
    std::set<std::string> registeredChapterMetaPaths;

    // Track registered scene meta ABS paths (those a chapter's scenes[] points at and that
    // exist on disk). A scene .meta.json on disk NOT in this set is an orphan — a candidate
    // for an external rename (EP-027 §8.1: refs are filename-only, so a rename is detected by
    // an orphan appearing in the same chapter dir where a ref's file went missing; the
    // orphan's own sidecar still carries the authoritative sceneID).
    std::set<std::string> registeredSceneMetaPaths;

    // Gather info about scenes whose metadata file is missing (for rename detection)
    std::vector<MissingMetaInfo> missingMetas;

    // Chapters whose folder is missing entirely (for chapter folder rename detection)
    // Maps chapterID → expected chapter meta path
    std::map<std::string, std::string> missingChapterFolders;

    // ---------------------------------------------------------------------------
    // Main scan loop
    // ---------------------------------------------------------------------------
    for (const auto& chapterRef : msData.chapters) {
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

        // EP-027 §8.1: scene refs are BARE FILENAMES resolved against the chapter's own
        // folder; scene identity + contentPath come from the sidecar.
        const std::string chapterDir =
            std::filesystem::path(chapterRef.path).parent_path().string();

        for (const auto& sceneRef : chData.scenes) {
            const std::string sMetaRel = chapterDir + "/" + sceneRef.metadataFilename;
            auto sMetaPath  = util::join(request.projectRootPath, sMetaRel);
            auto sMetaTextR = fs.readTextFile(sMetaPath);

            // Derive expected content path from metadata filename (swap .meta.json → .md)
            const std::string expectedContentPath =
                util::replaceExtension(
                    util::replaceExtension(sMetaRel, ""), ".md");

            if (!sMetaTextR.ok()) {
                // sceneID is unknown here (the ref no longer carries it and the sidecar is
                // unreadable); rename detection matches on the meta path instead.
                result.repairIssues.push_back(
                    RepairClassifier::missingMetadata(
                        projectID, chapterRef.chapterID, SceneID{""}, sMetaPath));
                registeredContentPaths.insert(expectedContentPath);
                // Record for rename detection
                missingMetas.push_back({
                    .chapterID=chapterRef.chapterID,
                    .sceneID=SceneID{""},
                    .expectedMetaPath=sMetaPath,
                    .chapterMetaPath=chPath
                });
                continue;
            }

            auto sParsed = schemas::parseSceneMeta(sMetaTextR.value());
            if (!sParsed.ok()) {
                result.repairIssues.push_back(
                    RepairClassifier::corruptMetadata(
                        projectID, chapterRef.chapterID, SceneID{""}, sMetaPath,
                        sParsed.error().message));
                registeredContentPaths.insert(expectedContentPath);
                continue;
            }

            const SceneID sceneID = sParsed.value().sceneID;
            registeredSceneMetaPaths.insert(sMetaPath);   // this ref resolves — not an orphan
            // contentPath is a bare filename (§8.1) — resolve against the chapter dir.
            const std::string contentRel =
                chapterDir + "/" + sParsed.value().contentPath;
            registeredContentPaths.insert(contentRel);

            auto absContentPath = util::join(request.projectRootPath, contentRel);
            auto contentExistsR = fs.exists(absContentPath);
            if (!contentExistsR.ok()) {
                return Result<ExternalChangeScanResult>::failure(contentExistsR.error());
}

            if (!contentExistsR.value()) {
                result.repairIssues.push_back(
                    RepairClassifier::missingContent(
                        projectID, chapterRef.chapterID, sceneID, absContentPath));
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
            if (!isDirR.ok() || !isDirR.value()) { continue;
}

            auto filesR = fs.listDirectory(chapterDirPath);
            if (!filesR.ok()) { continue;
}

            for (auto& filePath : filesR.value()) {
                const auto ext = util::extension(filePath);

                if (ext == ".md") {
                    std::string rel = filePath.substr(request.projectRootPath.size() + 1);
                    if (!registeredContentPaths.contains(rel)) {
                        result.repairIssues.push_back(
                            RepairClassifier::unregisteredFile(projectID, filePath));
                        result.indexesDirty = true;
                    }
                } else if (ext == ".json") {
                    // Could be an orphan .meta.json — check if it's registered
                    if (registeredChapterMetaPaths.contains(filePath)) {
                        continue; // it's a registered chapter meta
}
                    if (util::filename(filePath) == "chapter.meta.json") {
                        continue; // handled separately below
}

                    // Try to parse as scene metadata to get sceneID
                    auto textR = fs.readTextFile(filePath);
                    if (!textR.ok()) { continue;
}
                    auto parsed = schemas::parseSceneMeta(textR.value());
                    if (!parsed.ok()) { continue;
}

                    // A true orphan: a scene .meta.json on disk that no chapter's scenes[]
                    // resolves to. (A ref that DID resolve registered its abs path above.)
                    if (registeredSceneMetaPaths.contains(filePath)) { continue; }
                    orphanMetasBySceneID[parsed.value().sceneID.value].push_back(filePath);
                }
            }
        }
    }

    // ---------------------------------------------------------------------------
    // T-0031: Metadata rename detection (EP-027 §8.1 basis)
    // A scene .meta.json renamed externally leaves (a) a missing ref in its chapter and
    // (b) an orphan .meta.json in that same chapter folder. Refs are filename-only, so we
    // pair them by CHAPTER DIRECTORY, not by a sceneID in the ref. The orphan's OWN sidecar
    // still carries the authoritative sceneID — we read it there and thread it through the
    // issue/auto-apply, so identity is never lost (it's derived, not duplicated in the ref).
    // ---------------------------------------------------------------------------

    // Flatten orphans into a per-chapter-directory list (sceneID read from the sidecar).
    std::map<std::string, std::vector<std::string>> orphansByChapterDir;
    for (auto& [sceneIDVal, paths] : orphanMetasBySceneID) {
        (void)sceneIDVal;
        for (auto& p : paths) { orphansByChapterDir[util::parent(p)].push_back(p); }
    }

    for (auto& missing : missingMetas) {
        const auto expectedChDir = util::parent(missing.expectedMetaPath);
        auto dirIt = orphansByChapterDir.find(expectedChDir);

        if (dirIt == orphansByChapterDir.end() || dirIt->second.empty()) {
            continue;   // no orphan in this chapter dir — a genuine missing metadata
        }
        const auto& sameDirCandidates = dirIt->second;

        // Derive the sceneID from the (single) orphan's sidecar — this is the moved scene's
        // authoritative identity.
        auto orphanSceneID = [&](const std::string& p) -> SceneID {
            auto t = fs.readTextFile(p);
            if (!t.ok()) { return SceneID{""}; }
            auto s = schemas::parseSceneMeta(t.value());
            return s.ok() ? s.value().sceneID : SceneID{""};
        };

        if (sameDirCandidates.size() == 1) {
            // Unambiguous single candidate in the same chapter dir — auto-apply.
            const std::string& orphanPath = sameDirCandidates.front();
            const SceneID sceneID = orphanSceneID(orphanPath);
            auto chTextR = fs.readTextFile(missing.chapterMetaPath);
            if (chTextR.ok()) {
                auto chParsed = schemas::parseChapterMeta(chTextR.value());
                if (chParsed.ok()) {
                    autoApplyMetadataRename(
                        fs,
                        missing.chapterMetaPath,
                        chParsed.value(),
                        sceneID,
                        orphanPath,
                        request.projectRootPath,
                        result);
                    // Remove any missingMetadata issue we staged for this chapter's missing
                    // scene (matched by chapter + the now-resolved sceneID).
                    auto toErase1 = std::ranges::remove_if(result.repairIssues,
                        [&](const RepairIssue& i) {
                            return i.category == RepairCategory::missingMetadata &&
                                   i.chapterID.value == missing.chapterID.value;
                        });
                    result.repairIssues.erase(toErase1.begin(), result.repairIssues.end());
                }
            }
        } else {
            // Multiple candidates in same dir — stage for user review (sceneID from the
            // first candidate's sidecar).
            result.repairIssues.push_back(
                RepairClassifier::possibleMetadataRename(
                    projectID, missing.chapterID, orphanSceneID(sameDirCandidates.front()),
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
            if (!isDirR.ok() || !isDirR.value()) { continue;
}

            // Skip registered chapter directories
            // A registered chapter dir is the parent of a registered chapter meta path
            bool alreadyRegistered = false;
            for (const auto& regPath : registeredChapterMetaPaths) {
                if (util::parent(regPath) == chapterDirPath) {
                    alreadyRegistered = true;
                    break;
                }
            }
            if (alreadyRegistered) { continue;
}

            // Look for chapter.meta.json in this unregistered directory
            auto candidateChMeta = util::join(chapterDirPath, "chapter.meta.json");
            auto chTextR         = fs.readTextFile(candidateChMeta);
            if (!chTextR.ok()) { continue;
}

            auto chParsed = schemas::parseChapterMeta(chTextR.value());
            if (!chParsed.ok()) { continue;
}

            const auto& foundChapterID = chParsed.value().chapterID.value;
            auto mIt = missingChapterFolders.find(foundChapterID);
            if (mIt == missingChapterFolders.end()) { continue;
}

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
                    auto toErase2 = std::ranges::remove_if(result.repairIssues,
                        [&](const RepairIssue& i) {
                            return i.category == RepairCategory::missingMetadata &&
                                   i.chapterID.value == foundChapterID &&
                                   i.sceneID.value.empty();
                        });
                    result.repairIssues.erase(toErase2.begin(), result.repairIssues.end());
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
