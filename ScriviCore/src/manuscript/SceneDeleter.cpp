#include "manuscript/SceneDeleter.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>

namespace scrivi::manuscript {

SceneDeleter::SceneDeleter(CoreServices& services)
    : services_(services) {}

Result<DeleteSceneResult> SceneDeleter::remove(const DeleteSceneRequest& request)
{
    if (request.sceneID.value.empty()) {
        return Result<DeleteSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sceneID must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string& root = request.projectRootPath;

    // EP-027 §8.1: identity + location are FILESYSTEM-AUTHORITATIVE. Scan the on-disk
    // chapters in order and, within each, the on-disk scene files (sceneID from each
    // sidecar) — not the cache arrays. Delete the matching scene's `.meta.json` + `.md`,
    // then rebuild that chapter's scenes[] cache from disk.
    auto chaptersR = listChaptersByOrder(fs, root);
    if (!chaptersR.ok()) { return Result<DeleteSceneResult>::failure(chaptersR.error()); }

    for (const auto& chapter : chaptersR.value()) {
        const std::string chMetaRel = chapter.chapterMetadataRelPath;
        const std::string chDir     = chapterDirOf(chMetaRel);

        auto scenesR = listScenesByOrder(fs, root, chMetaRel);
        if (!scenesR.ok()) { return Result<DeleteSceneResult>::failure(scenesR.error()); }

        auto it = std::find_if(scenesR.value().begin(), scenesR.value().end(),
            [&](const SceneEntry& e) { return e.sceneID.value == request.sceneID.value; });
        if (it == scenesR.value().end()) { continue; }

        // Delete the scene files from disk (best-effort — ignore missing).
        const std::string metaAbsPath =
            util::join(root, it->metadataRelPath);
        const std::string contentAbsPath =
            util::join(root, chDir + "/" + it->contentFilename);
        std::error_code ec;
        std::filesystem::remove(metaAbsPath, ec);
        std::filesystem::remove(contentAbsPath, ec);

        // Rebuild the chapter's scenes[] cache from disk (the scene is now gone).
        auto rb = rebuildChapterScenesIfInconsistent(fs, root, chMetaRel);
        if (!rb.ok()) { return Result<DeleteSceneResult>::failure(rb.error()); }

        DeleteSceneResult result;
        result.sceneID = request.sceneID;
        result.deleted = true;
        return Result<DeleteSceneResult>::success(std::move(result));
    }

    return Result<DeleteSceneResult>::failure(
        {.code = ErrorCode::invalidArgument,
         .message = "sceneID not found in any chapter: " + request.sceneID.value});
}

} // namespace scrivi::manuscript
