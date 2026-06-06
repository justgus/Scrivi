#include "manuscript/SceneReorderer.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

namespace scrivi::manuscript {

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

    // 1. Read manuscript.meta.json to resolve chapter metadata paths
    const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<ReorderSceneResult>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<ReorderSceneResult>::failure(msParsed.error()); }

    // Find source and target chapter metadata paths
    std::string srcChMetaPath, dstChMetaPath;
    for (const auto& chRef : msParsed.value().chapters) {
        if (chRef.chapterID.value == request.sourceChapterID.value)
            srcChMetaPath = util::join(root, chRef.path);
        if (chRef.chapterID.value == request.targetChapterID.value)
            dstChMetaPath = util::join(root, chRef.path);
    }

    if (srcChMetaPath.empty()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sourceChapterID not found: " + request.sourceChapterID.value});
    }
    if (dstChMetaPath.empty()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "targetChapterID not found: " + request.targetChapterID.value});
    }

    const bool sameChapter = (request.sourceChapterID.value == request.targetChapterID.value);

    // 2. Read source chapter
    auto srcTextR = fs.readTextFile(srcChMetaPath);
    if (!srcTextR.ok()) { return Result<ReorderSceneResult>::failure(srcTextR.error()); }

    auto srcParsed = schemas::parseChapterMeta(srcTextR.value());
    if (!srcParsed.ok()) { return Result<ReorderSceneResult>::failure(srcParsed.error()); }

    auto& srcCh = srcParsed.value();

    // 3. Extract the SceneRef from the source chapter
    auto srcIt = std::find_if(srcCh.scenes.begin(), srcCh.scenes.end(),
        [&](const schemas::SceneRef& ref) {
            return ref.sceneID.value == request.sceneID.value;
        });

    if (srcIt == srcCh.scenes.end()) {
        return Result<ReorderSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "sceneID not found in sourceChapter: " + request.sceneID.value});
    }

    schemas::SceneRef movingRef = *srcIt;
    srcCh.scenes.erase(srcIt);

    if (sameChapter) {
        // 4a. Reorder within same chapter — insert into the modified srcCh scenes list
        if (request.afterSceneID.value.empty()) {
            srcCh.scenes.insert(srcCh.scenes.begin(), std::move(movingRef));
        } else {
            auto insertAfter = std::find_if(srcCh.scenes.begin(), srcCh.scenes.end(),
                [&](const schemas::SceneRef& ref) {
                    return ref.sceneID.value == request.afterSceneID.value;
                });
            // If afterSceneID not found, append to end
            srcCh.scenes.insert(
                insertAfter != srcCh.scenes.end() ? std::next(insertAfter) : srcCh.scenes.end(),
                std::move(movingRef));
        }

        auto writeR = fs.atomicWriteTextFile(srcChMetaPath,
                                             schemas::serializeChapterMeta(srcCh));
        if (!writeR.ok()) { return Result<ReorderSceneResult>::failure(writeR.error()); }

    } else {
        // 4b. Cross-chapter move — write source first (scene removed), then target (scene added)
        auto writeSrcR = fs.atomicWriteTextFile(srcChMetaPath,
                                                schemas::serializeChapterMeta(srcCh));
        if (!writeSrcR.ok()) { return Result<ReorderSceneResult>::failure(writeSrcR.error()); }

        auto dstTextR = fs.readTextFile(dstChMetaPath);
        if (!dstTextR.ok()) { return Result<ReorderSceneResult>::failure(dstTextR.error()); }

        auto dstParsed = schemas::parseChapterMeta(dstTextR.value());
        if (!dstParsed.ok()) { return Result<ReorderSceneResult>::failure(dstParsed.error()); }

        auto& dstCh = dstParsed.value();

        if (request.afterSceneID.value.empty()) {
            dstCh.scenes.insert(dstCh.scenes.begin(), std::move(movingRef));
        } else {
            auto insertAfter = std::find_if(dstCh.scenes.begin(), dstCh.scenes.end(),
                [&](const schemas::SceneRef& ref) {
                    return ref.sceneID.value == request.afterSceneID.value;
                });
            dstCh.scenes.insert(
                insertAfter != dstCh.scenes.end() ? std::next(insertAfter) : dstCh.scenes.end(),
                std::move(movingRef));
        }

        auto writeDstR = fs.atomicWriteTextFile(dstChMetaPath,
                                                schemas::serializeChapterMeta(dstCh));
        if (!writeDstR.ok()) { return Result<ReorderSceneResult>::failure(writeDstR.error()); }
    }

    ReorderSceneResult result;
    result.sceneID         = request.sceneID;
    result.sourceChapterID = request.sourceChapterID;
    result.targetChapterID = request.targetChapterID;
    result.reordered       = true;
    return Result<ReorderSceneResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
