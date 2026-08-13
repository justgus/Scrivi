#include "manuscript/ChapterDeleter.hpp"

#include "manuscript/SceneIndex.hpp"
#include "objects/RelationshipStore.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace scrivi::manuscript {

ChapterDeleter::ChapterDeleter(CoreServices& services)
    : services_(services) {}

Result<DeleteChapterResult> ChapterDeleter::remove(const DeleteChapterRequest& request)
{
    if (request.chapterID.value.empty()) {
        return Result<DeleteChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string& root = request.projectRootPath;

    // 1. Read manuscript.meta.json
    const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<DeleteChapterResult>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<DeleteChapterResult>::failure(msParsed.error()); }

    auto& ms = msParsed.value();

    // 2. Find the ChapterRef
    auto it = std::find_if(ms.chapters.begin(), ms.chapters.end(),
        [&](const schemas::ChapterRef& ref) {
            return ref.chapterID.value == request.chapterID.value;
        });

    if (it == ms.chapters.end()) {
        return Result<DeleteChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID not found in manuscript: " + request.chapterID.value});
    }

    // 3. Read chapter.meta.json to count scenes before deleting
    const std::string chMetaAbsPath = util::join(root, it->path);
    int scenesDeleted = 0;

    // Every scene in this chapter is about to be removed with the directory, so
    // collect their IDs FIRST — after remove_all there is nothing left to read
    // them from, and each is a relationship endpoint that must be cascade-pruned
    // (T-0377, Doc 1 §5.5). Identity is filesystem-authoritative (EP-027), so
    // the IDs come from the scene sidecars rather than the chapter's cache.
    std::vector<std::string> sceneIDs;
    if (auto scenesR = listScenesByOrder(fs, root, it->path); scenesR.ok()) {
        for (const auto& s : scenesR.value()) { sceneIDs.push_back(s.sceneID.value); }
    }

    auto chTextR = fs.readTextFile(chMetaAbsPath);
    if (chTextR.ok()) {
        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (chParsed.ok()) {
            scenesDeleted = static_cast<int>(chParsed.value().scenes.size());
        }
    }

    // 4. Remove chapter from manuscript index and rewrite manuscript.meta.json atomically
    //    Do this before removing files so the index is consistent even if disk ops fail.
    ms.chapters.erase(it);
    auto writeMsR = fs.atomicWriteTextFile(msMetaPath,
                                           schemas::serializeManuscriptMeta(ms));
    if (!writeMsR.ok()) { return Result<DeleteChapterResult>::failure(writeMsR.error()); }

    // 5. Remove chapter directory from disk (contains all scene files + chapter.meta.json)
    //    chMetaAbsPath is "root/manuscript/chapter-NNN/chapter.meta.json"
    //    so the chapter directory is its parent.
    std::filesystem::path chapterDir =
        std::filesystem::path(chMetaAbsPath).parent_path();
    std::error_code ec;
    std::filesystem::remove_all(chapterDir, ec);

    // 6. Cascade-prune every deleted scene's edges (T-0377). Best-effort: the
    //    chapter is already gone, and a surviving edge is dangling, which
    //    load-time repair drops. Edges into an unavailable world are held, not
    //    pruned — cascadeDelete enforces that (Doc 3 §4.6).
    objects::RelationshipStore graph{services_};
    for (const auto& sceneID : sceneIDs) {
        (void)graph.cascadeDelete(root, sceneID);
    }

    DeleteChapterResult result;
    result.chapterID     = request.chapterID;
    result.scenesDeleted = scenesDeleted;
    result.deleted       = true;
    return Result<DeleteChapterResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
