#include "manuscript/SceneMerger.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

namespace scrivi::manuscript {

namespace {

// Append `addition` onto `base`, separated by a blank line, matching the scene-body
// concatenation the app performs. If either side is empty the separator is elided so the
// survivor never gains stray leading/trailing blank lines.
std::string joinBodies(const std::string& base, const std::string& addition) {
    if (base.empty())     { return addition; }
    if (addition.empty()) { return base; }
    return base + "\n\n" + addition;
}

} // namespace

SceneMerger::SceneMerger(CoreServices& services)
    : services_(services) {}

Result<MergeSceneResult> SceneMerger::merge(const MergeSceneRequest& request)
{
    if (request.sceneID.value.empty()) {
        return Result<MergeSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sceneID must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string& root = request.projectRootPath;

    // EP-027 §8.1: identity + location are FILESYSTEM-AUTHORITATIVE. Scan on-disk chapters
    // in order and, within each, the on-disk scenes (sceneID from each sidecar) to find the
    // scene to merge — not the cache arrays.
    auto chaptersR = listChaptersByOrder(fs, root);
    if (!chaptersR.ok()) { return Result<MergeSceneResult>::failure(chaptersR.error()); }

    for (const auto& chapter : chaptersR.value()) {
        const std::string chMetaRel = chapter.chapterMetadataRelPath;
        const std::string chDir     = chapterDirOf(chMetaRel);

        auto scenesR = listScenesByOrder(fs, root, chMetaRel);
        if (!scenesR.ok()) { return Result<MergeSceneResult>::failure(scenesR.error()); }
        const auto& scenes = scenesR.value();

        auto it = std::find_if(scenes.begin(), scenes.end(),
            [&](const SceneEntry& e) { return e.sceneID.value == request.sceneID.value; });
        if (it == scenes.end()) { continue; }

        // The predecessor is the survivor. If the scene is first in its chapter there is no
        // predecessor to merge into (that is the whole-chapter merge case, T-0300).
        if (it == scenes.begin()) {
            return Result<MergeSceneResult>::failure(
                {.code = ErrorCode::invalidArgument,
                 .message = "scene is first in its chapter; no predecessor to merge into: "
                            + request.sceneID.value});
        }
        const SceneEntry merged   = *it;
        const SceneEntry survivor = *std::prev(it);

        const std::string survivorContentRel = chDir + "/" + survivor.contentFilename;
        const std::string survivorMetaRel    = survivor.metadataRelPath;
        const std::string mergedContentRel    = chDir + "/" + merged.contentFilename;
        const std::string mergedMetaRel        = merged.metadataRelPath;

        // A scene body may not exist yet; a missing `.md` reads as empty.
        auto contentExists = [&](const std::string& rel) -> Result<bool> {
            return fs.exists(util::join(root, rel));
        };

        // Read both bodies.
        std::string survivorBody;
        {
            auto ex = contentExists(survivorContentRel);
            if (!ex.ok()) { return Result<MergeSceneResult>::failure(ex.error()); }
            if (ex.value()) {
                auto r = fs.readTextFile(util::join(root, survivorContentRel));
                if (!r.ok()) { return Result<MergeSceneResult>::failure(r.error()); }
                survivorBody = r.value();
            }
        }
        std::string mergedBody;
        bool mergedContentPresent = false;
        {
            auto ex = contentExists(mergedContentRel);
            if (!ex.ok()) { return Result<MergeSceneResult>::failure(ex.error()); }
            mergedContentPresent = ex.value();
            if (mergedContentPresent) {
                auto r = fs.readTextFile(util::join(root, mergedContentRel));
                if (!r.ok()) { return Result<MergeSceneResult>::failure(r.error()); }
                mergedBody = r.value();
            }
        }

        // Append the merged body onto the survivor and write it back atomically.
        auto w = fs.atomicWriteTextFile(util::join(root, survivorContentRel),
                                        joinBodies(survivorBody, mergedBody));
        if (!w.ok()) { return Result<MergeSceneResult>::failure(w.error()); }

        // Remove the merged scene's files (the content may be absent).
        auto rmMeta = fs.removeFile(util::join(root, mergedMetaRel));
        if (!rmMeta.ok()) { return Result<MergeSceneResult>::failure(rmMeta.error()); }
        if (mergedContentPresent) {
            auto rmContent = fs.removeFile(util::join(root, mergedContentRel));
            if (!rmContent.ok()) { return Result<MergeSceneResult>::failure(rmContent.error()); }
        }

        // Rebuild the chapter's scenes[] cache from disk (the merged scene is now gone).
        auto rb = rebuildChapterScenesIfInconsistent(fs, root, chMetaRel);
        if (!rb.ok()) { return Result<MergeSceneResult>::failure(rb.error()); }

        MergeSceneResult result;
        result.survivorSceneID      = survivor.sceneID;
        result.mergedSceneID        = request.sceneID;
        result.chapterID            = chapter.chapterID;
        result.survivorMetadataPath = survivorMetaRel;
        result.survivorContentPath  = survivorContentRel;
        result.chapterMetadataPath  = chMetaRel;
        result.merged               = true;
        return Result<MergeSceneResult>::success(std::move(result));
    }

    return Result<MergeSceneResult>::failure(
        {.code = ErrorCode::invalidArgument,
         .message = "sceneID not found in any chapter: " + request.sceneID.value});
}

} // namespace scrivi::manuscript
