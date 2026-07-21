#include "manuscript/ChapterMerger.hpp"

#include "manuscript/ChapterDeleter.hpp"
#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace scrivi::manuscript {

namespace {

// The slug portion after the `<key>-` prefix of a scene meta filename, sans ".meta.json"
// (mirrors SceneReorderer's file-local helper — the base slug is preserved across a move).
std::string baseSlugOf(const std::string& metaFilename) {
    constexpr std::string_view kMetaSuffix = ".meta.json";
    std::string stem = metaFilename;
    if (stem.size() > kMetaSuffix.size() &&
        stem.compare(stem.size() - kMetaSuffix.size(),
                     kMetaSuffix.size(), kMetaSuffix) == 0) {
        stem = stem.substr(0, stem.size() - kMetaSuffix.size());
    }
    const auto dash = stem.find('-');
    return (dash == std::string::npos) ? stem : stem.substr(dash + 1);
}

} // namespace

ChapterMerger::ChapterMerger(CoreServices& services)
    : services_(services) {}

Result<MergeChapterResult> ChapterMerger::merge(const MergeChapterRequest& request)
{
    if (request.chapterID.value.empty()) {
        return Result<MergeChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string& root = request.projectRootPath;

    // 1. Disk order is authoritative (EP-027 B3). Locate the merged chapter and its
    //    predecessor (the chapter immediately before it in reading order = the survivor).
    auto chaptersR = listChaptersByOrder(fs, root);
    if (!chaptersR.ok()) { return Result<MergeChapterResult>::failure(chaptersR.error()); }
    const auto& chapters = chaptersR.value();

    auto it = std::find_if(chapters.begin(), chapters.end(),
        [&](const ChapterEntry& e) { return e.chapterID.value == request.chapterID.value; });
    if (it == chapters.end()) {
        return Result<MergeChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID not found on disk: " + request.chapterID.value});
    }
    if (it == chapters.begin()) {
        return Result<MergeChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapter is first in the manuscript; no predecessor to merge into: "
                        + request.chapterID.value});
    }
    const ChapterEntry merged    = *it;
    const ChapterEntry survivor  = *std::prev(it);

    const std::string survivorChMetaRel = survivor.chapterMetadataRelPath;
    const std::string survivorDir       = chapterDirOf(survivorChMetaRel);
    const std::string mergedChMetaRel   = merged.chapterMetadataRelPath;
    const std::string mergedDir         = chapterDirOf(mergedChMetaRel);

    // 2. The mint anchor: the predecessor's LAST scene order key. New keys go after it so
    //    the relocated scenes append to the end of the survivor in their existing order.
    auto survScenesR = listScenesByOrder(fs, root, survivorChMetaRel);
    if (!survScenesR.ok()) { return Result<MergeChapterResult>::failure(survScenesR.error()); }
    std::string lo = survScenesR.value().empty() ? std::string()
                                                 : survScenesR.value().back().orderKey;

    // 3. Relocate every scene of the merged chapter into the survivor's folder, in order.
    auto mergedScenesR = listScenesByOrder(fs, root, mergedChMetaRel);
    if (!mergedScenesR.ok()) { return Result<MergeChapterResult>::failure(mergedScenesR.error()); }

    int relocated = 0;
    for (const auto& scene : mergedScenesR.value()) {
        const std::string newKey = util::keyAfter(lo);
        if (newKey.empty()) {
            return Result<MergeChapterResult>::failure(
                {.code = ErrorCode::internalError,
                 .message = "could not compute a scene order key after " + lo});
        }

        const std::string baseSlug       = baseSlugOf(scene.metadataFilename);
        const std::string srcMetaRel     = mergedDir + "/" + scene.metadataFilename;
        const std::string srcContentRel  = mergedDir + "/" + scene.contentFilename;
        const std::string dstMetaFile    = newKey + "-" + baseSlug + ".meta.json";
        const std::string dstContentFile = newKey + "-" + baseSlug + ".md";
        const std::string dstMetaRel     = survivorDir + "/" + dstMetaFile;
        const std::string dstContentRel  = survivorDir + "/" + dstContentFile;

        // Move the content file first, then the sidecar (a scene may have no `.md` yet).
        auto srcContentExists = fs.exists(util::join(root, srcContentRel));
        if (!srcContentExists.ok()) {
            return Result<MergeChapterResult>::failure(srcContentExists.error());
        }
        if (srcContentExists.value()) {
            auto mv = fs.renamePath(util::join(root, srcContentRel),
                                    util::join(root, dstContentRel));
            if (!mv.ok()) { return Result<MergeChapterResult>::failure(mv.error()); }
        }
        {
            auto mv = fs.renamePath(util::join(root, srcMetaRel),
                                    util::join(root, dstMetaRel));
            if (!mv.ok()) { return Result<MergeChapterResult>::failure(mv.error()); }
        }

        // Rewrite the moved sidecar's slug + contentPath (bare filename) to the new stem.
        {
            auto sTextR = fs.readTextFile(util::join(root, dstMetaRel));
            if (!sTextR.ok()) { return Result<MergeChapterResult>::failure(sTextR.error()); }
            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) { return Result<MergeChapterResult>::failure(sParsed.error()); }
            sParsed.value().slug        = newKey + "-" + baseSlug;
            sParsed.value().contentPath = dstContentFile;
            auto sw = fs.atomicWriteTextFile(util::join(root, dstMetaRel),
                                             schemas::serializeSceneMeta(sParsed.value()));
            if (!sw.ok()) { return Result<MergeChapterResult>::failure(sw.error()); }
        }

        lo = newKey;   // next scene appends after this one
        ++relocated;
    }

    // 4. Rebuild the survivor's scenes[] cache from disk (it gained scenes).
    auto rb = rebuildChapterScenesIfInconsistent(fs, root, survivorChMetaRel);
    if (!rb.ok()) { return Result<MergeChapterResult>::failure(rb.error()); }

    // 5. Remove the now-empty merged chapter: its manuscript.meta.json entry + folder. The
    //    folder no longer holds any scene files (only chapter.meta.json), so a wholesale
    //    remove is safe — this is where I-0083 differed (it removed the folder BEFORE moving
    //    the scenes out). Reuse ChapterDeleter for the index-erase + folder-remove.
    ChapterDeleter deleter{services_};
    DeleteChapterRequest delReq;
    delReq.projectRootPath = root;
    delReq.chapterID       = request.chapterID;
    auto delR = deleter.remove(delReq);
    if (!delR.ok()) { return Result<MergeChapterResult>::failure(delR.error()); }

    // 6. Rebuild the chapter index cache so its array order matches disk (best effort).
    rebuildIndexIfInconsistent(fs, root);

    MergeChapterResult result;
    result.survivorChapterID           = survivor.chapterID;
    result.mergedChapterID             = request.chapterID;
    result.survivorChapterMetadataPath = survivorChMetaRel;
    result.scenesRelocated             = relocated;
    result.merged                      = true;
    return Result<MergeChapterResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
