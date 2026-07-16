#include "manuscript/ChapterReorderer.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <vector>

namespace scrivi::manuscript {

ChapterReorderer::ChapterReorderer(CoreServices& services)
    : services_(services) {}

Result<ReorderChapterResult> ChapterReorderer::reorder(const ReorderChapterRequest& request)
{
    if (request.chapterID.value.empty()) {
        return Result<ReorderChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string& root = request.projectRootPath;

    // EP-027 B3: chapter order is the on-disk `chapter-<orderKey>` folder sort, so a
    // reorder is NOT an index-array shuffle — it assigns the moved chapter a NEW order-key
    // strictly between its target neighbours and RENAMES that one folder (renamePath).
    // Only the moved folder changes; the index (manuscript.meta.json) is a cache updated
    // to reflect the new path.

    // 1. The current disk order (authoritative).
    auto chaptersR = listChaptersByOrder(fs, root);
    if (!chaptersR.ok()) { return Result<ReorderChapterResult>::failure(chaptersR.error()); }
    auto chapters = std::move(chaptersR.value());

    // 2. Locate the moving chapter and the "after" anchor in disk order.
    auto findById = [&](const std::string& id) {
        return std::find_if(chapters.begin(), chapters.end(),
            [&](const ChapterEntry& e) { return e.chapterID.value == id; });
    };
    auto movingIt = findById(request.chapterID.value);
    if (movingIt == chapters.end()) {
        return Result<ReorderChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID not found on disk: " + request.chapterID.value});
    }
    const ChapterEntry moving = *movingIt;   // copy before we reason about neighbours

    // 3. Determine the key window (lo, hi) the moved chapter lands between, from the
    //    order EXCLUDING the mover. afterChapterID empty → move to the front (lo = "").
    std::vector<ChapterEntry> others;
    others.reserve(chapters.size());
    for (const auto& e : chapters) {
        if (e.chapterID.value != moving.chapterID.value) others.push_back(e);
    }

    std::string lo, hi;
    if (request.afterChapterID.value.empty()) {
        lo = std::string();
        hi = others.empty() ? std::string() : others.front().orderKey;
    } else {
        auto afterIt = std::find_if(others.begin(), others.end(),
            [&](const ChapterEntry& e) { return e.chapterID.value == request.afterChapterID.value; });
        if (afterIt == others.end()) {
            // Unknown anchor → append to the end (after the last chapter).
            lo = others.empty() ? std::string() : others.back().orderKey;
            hi = std::string();
        } else {
            lo = afterIt->orderKey;
            auto nextIt = std::next(afterIt);
            hi = (nextIt == others.end()) ? std::string() : nextIt->orderKey;
        }
    }

    // 4. If the mover is already correctly placed (its key is already in (lo,hi)), it's a
    //    no-op — avoid a needless rename.
    const bool alreadyBetween =
        (lo.empty() || lo < moving.orderKey) && (hi.empty() || moving.orderKey < hi);
    if (alreadyBetween) {
        ReorderChapterResult result;
        result.chapterID = request.chapterID;
        result.reordered = true;
        return Result<ReorderChapterResult>::success(std::move(result));
    }

    // 5. New key strictly between the neighbours, and the new folder path.
    const std::string newKey = util::keyBetween(lo, hi);
    if (newKey.empty()) {
        return Result<ReorderChapterResult>::failure(
            {.code = ErrorCode::internalError,
             .message = "could not compute an order key between neighbours"});
    }
    const std::string oldDir = "manuscript/chapter-" + moving.orderKey;
    const std::string newDir = "manuscript/chapter-" + newKey;
    const std::string newChMetaRel = newDir + "/chapter.meta.json";

    // 6. Rename the folder (atomic, never clobbers — the new key is unique by construction).
    auto renameR = fs.renamePath(util::join(root, oldDir), util::join(root, newDir));
    if (!renameR.ok()) { return Result<ReorderChapterResult>::failure(renameR.error()); }

    // 7. The folder moved, so every relative path that referenced the OLD folder must be
    //    rewritten to the NEW folder: the chapter sidecar's slug + each scene's
    //    metadataPath, and each scene sidecar's own contentPath. (Paths are relative to
    //    the project root and embed the `manuscript/chapter-<key>/` prefix.)
    auto rewritePrefix = [&](std::string p) {
        if (p.rfind(oldDir + "/", 0) == 0) {
            return newDir + p.substr(oldDir.size());
        }
        return p;
    };
    {
        auto chTextR = fs.readTextFile(util::join(root, newChMetaRel));
        if (!chTextR.ok()) { return Result<ReorderChapterResult>::failure(chTextR.error()); }
        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) { return Result<ReorderChapterResult>::failure(chParsed.error()); }
        auto& ch = chParsed.value();
        ch.slug = "chapter-" + newKey;

        for (auto& sceneRef : ch.scenes) {
            const std::string oldSceneMetaRel = sceneRef.metadataPath;
            const std::string newSceneMetaRel = rewritePrefix(oldSceneMetaRel);
            sceneRef.metadataPath = newSceneMetaRel;

            // Rewrite the scene sidecar's own contentPath (the .md moved with the folder).
            auto sTextR = fs.readTextFile(util::join(root, newSceneMetaRel));
            if (!sTextR.ok()) { return Result<ReorderChapterResult>::failure(sTextR.error()); }
            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) { return Result<ReorderChapterResult>::failure(sParsed.error()); }
            sParsed.value().contentPath = rewritePrefix(sParsed.value().contentPath);
            auto sw = fs.atomicWriteTextFile(util::join(root, newSceneMetaRel),
                                             schemas::serializeSceneMeta(sParsed.value()));
            if (!sw.ok()) { return Result<ReorderChapterResult>::failure(sw.error()); }
        }

        auto wR = fs.atomicWriteTextFile(util::join(root, newChMetaRel),
                                         schemas::serializeChapterMeta(ch));
        if (!wR.ok()) { return Result<ReorderChapterResult>::failure(wR.error()); }
    }

    // 8. Update the index cache (manuscript.meta.json) to point at the new path. Best
    //    effort — order is now derived from disk, so a stale index self-heals on load;
    //    but keep it consistent when we can.
    {
        const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
        auto msTextR = fs.readTextFile(msMetaPath);
        if (msTextR.ok()) {
            auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
            if (msParsed.ok()) {
                for (auto& ref : msParsed.value().chapters) {
                    if (ref.chapterID.value == moving.chapterID.value) {
                        ref.path = newChMetaRel;
                    }
                }
                fs.atomicWriteTextFile(msMetaPath,
                                       schemas::serializeManuscriptMeta(msParsed.value()));
            }
        }
    }

    ReorderChapterResult result;
    result.chapterID  = request.chapterID;
    result.reordered  = true;
    return Result<ReorderChapterResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
