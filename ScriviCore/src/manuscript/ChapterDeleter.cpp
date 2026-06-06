#include "manuscript/ChapterDeleter.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>

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

    DeleteChapterResult result;
    result.chapterID     = request.chapterID;
    result.scenesDeleted = scenesDeleted;
    result.deleted       = true;
    return Result<DeleteChapterResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
