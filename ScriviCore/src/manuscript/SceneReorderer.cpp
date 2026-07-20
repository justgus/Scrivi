#include "manuscript/SceneReorderer.hpp"

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

// The slug portion after the `<key>-` prefix of a scene meta filename, sans ".meta.json".
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

SceneReorderer::SceneReorderer(CoreServices& services)
    : services_(services) {}

Result<ReorderSceneResult> SceneReorderer::reorder(const ReorderSceneRequest& request)
{
    if (request.sceneID.value.empty()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sceneID must not be empty"});
    }
    if (request.sourceChapterID.value.empty() || request.targetChapterID.value.empty()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sourceChapterID and targetChapterID must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string& root = request.projectRootPath;
    const bool sameChapter =
        (request.sourceChapterID.value == request.targetChapterID.value);

    // EP-027 §8.1: a scene reorder is FILESYSTEM-AUTHORITATIVE — it MOVES the scene's
    // `<key>-<slug>.{meta.json,md}` files into the destination chapter folder and gives
    // them a NEW order-key strictly between their new neighbours. No SceneRef is shuffled
    // across sidecars (the old bug); the chapter caches are rebuilt from disk afterwards.

    // 1. Resolve source + target chapter folders from disk (identity from the sidecar).
    auto srcChR = findChapterByID(fs, root, request.sourceChapterID);
    if (!srcChR.ok()) { return Result<ReorderSceneResult>::failure(srcChR.error()); }
    auto dstChR = sameChapter ? srcChR
                              : findChapterByID(fs, root, request.targetChapterID);
    if (!dstChR.ok()) { return Result<ReorderSceneResult>::failure(dstChR.error()); }

    const std::string srcChMetaRel = srcChR.value().chapterMetadataRelPath;
    const std::string dstChMetaRel = dstChR.value().chapterMetadataRelPath;
    const std::string dstDir = chapterDirOf(dstChMetaRel);

    // 2. Locate the moving scene on disk in the source chapter.
    auto srcScenesR = listScenesByOrder(fs, root, srcChMetaRel);
    if (!srcScenesR.ok()) { return Result<ReorderSceneResult>::failure(srcScenesR.error()); }

    auto movingIt = std::find_if(srcScenesR.value().begin(), srcScenesR.value().end(),
        [&](const SceneEntry& e) { return e.sceneID.value == request.sceneID.value; });
    if (movingIt == srcScenesR.value().end()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sceneID not found in sourceChapter: " + request.sceneID.value});
    }
    const SceneEntry moving = *movingIt;

    // 3. Destination scene list, EXCLUDING the mover (it may already live there). Compute
    //    the (lo, hi) key window from afterSceneID.
    auto dstScenesR = sameChapter ? srcScenesR
                                  : listScenesByOrder(fs, root, dstChMetaRel);
    if (!dstScenesR.ok()) { return Result<ReorderSceneResult>::failure(dstScenesR.error()); }

    std::vector<SceneEntry> others;
    others.reserve(dstScenesR.value().size());
    for (const auto& e : dstScenesR.value()) {
        if (e.sceneID.value != moving.sceneID.value) { others.push_back(e); }
    }

    std::string lo, hi;
    if (request.afterSceneID.value.empty()) {
        lo = std::string();
        hi = others.empty() ? std::string() : others.front().orderKey;
    } else {
        auto afterIt = std::find_if(others.begin(), others.end(),
            [&](const SceneEntry& e) { return e.sceneID.value == request.afterSceneID.value; });
        if (afterIt == others.end()) {
            lo = others.empty() ? std::string() : others.back().orderKey;
            hi = std::string();
        } else {
            lo = afterIt->orderKey;
            auto nextIt = std::next(afterIt);
            hi = (nextIt == others.end()) ? std::string() : nextIt->orderKey;
        }
    }

    // 4. No-op fast path: same chapter and the mover's key is already in (lo, hi).
    const bool alreadyBetween = sameChapter &&
        (lo.empty() || lo < moving.orderKey) && (hi.empty() || moving.orderKey < hi);
    if (alreadyBetween) {
        ReorderSceneResult result;
        result.sceneID             = request.sceneID;
        result.sourceChapterID     = request.sourceChapterID;
        result.targetChapterID     = request.targetChapterID;
        // No files moved — report the CURRENT paths so callers can always refresh
        // from the result (I-0081).
        result.metadataPath        = chapterDirOf(srcChMetaRel) + "/" + moving.metadataFilename;
        result.contentPath         = chapterDirOf(srcChMetaRel) + "/" + moving.contentFilename;
        result.chapterMetadataPath = dstChMetaRel;
        result.reordered           = true;
        return Result<ReorderSceneResult>::success(std::move(result));
    }

    const std::string newKey = util::keyBetween(lo, hi);
    if (newKey.empty()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::internalError,
             .message = "could not compute a scene order key between neighbours"});
    }

    // 5. Move the files into the destination folder with the new `<newKey>-<slug>` stem,
    //    rewriting the sidecar slug + contentPath (bare filename).
    const std::string baseSlug     = baseSlugOf(moving.metadataFilename);
    const std::string srcDir       = chapterDirOf(srcChMetaRel);
    const std::string srcMetaRel   = srcDir + "/" + moving.metadataFilename;
    const std::string srcContentRel= srcDir + "/" + moving.contentFilename;
    const std::string dstMetaFile  = newKey + "-" + baseSlug + ".meta.json";
    const std::string dstContentFile = newKey + "-" + baseSlug + ".md";
    const std::string dstMetaRel   = dstDir + "/" + dstMetaFile;
    const std::string dstContentRel= dstDir + "/" + dstContentFile;

    if (srcContentRel != dstContentRel) {
        auto mv = fs.renamePath(util::join(root, srcContentRel),
                                util::join(root, dstContentRel));
        if (!mv.ok()) { return Result<ReorderSceneResult>::failure(mv.error()); }
    }
    if (srcMetaRel != dstMetaRel) {
        auto mv = fs.renamePath(util::join(root, srcMetaRel),
                                util::join(root, dstMetaRel));
        if (!mv.ok()) { return Result<ReorderSceneResult>::failure(mv.error()); }
    }

    {
        auto sTextR = fs.readTextFile(util::join(root, dstMetaRel));
        if (!sTextR.ok()) { return Result<ReorderSceneResult>::failure(sTextR.error()); }
        auto sParsed = schemas::parseSceneMeta(sTextR.value());
        if (!sParsed.ok()) { return Result<ReorderSceneResult>::failure(sParsed.error()); }
        sParsed.value().slug        = newKey + "-" + baseSlug;
        sParsed.value().contentPath = dstContentFile;
        auto sw = fs.atomicWriteTextFile(util::join(root, dstMetaRel),
                                         schemas::serializeSceneMeta(sParsed.value()));
        if (!sw.ok()) { return Result<ReorderSceneResult>::failure(sw.error()); }
    }

    // 6. Rebuild the affected chapters' scenes[] caches from disk (order is now on-disk).
    auto rbDst = rebuildChapterScenesIfInconsistent(fs, root, dstChMetaRel);
    if (!rbDst.ok()) { return Result<ReorderSceneResult>::failure(rbDst.error()); }
    if (!sameChapter) {
        auto rbSrc = rebuildChapterScenesIfInconsistent(fs, root, srcChMetaRel);
        if (!rbSrc.ok()) { return Result<ReorderSceneResult>::failure(rbSrc.error()); }
    }

    ReorderSceneResult result;
    result.sceneID             = request.sceneID;
    result.sourceChapterID     = request.sourceChapterID;
    result.targetChapterID     = request.targetChapterID;
    // The post-move paths (I-0081): the files now live at the new order-key stem in the
    // target chapter's folder — the caller's captured paths are stale.
    result.metadataPath        = dstMetaRel;
    result.contentPath         = dstContentRel;
    result.chapterMetadataPath = dstChMetaRel;
    result.reordered           = true;
    return Result<ReorderSceneResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
