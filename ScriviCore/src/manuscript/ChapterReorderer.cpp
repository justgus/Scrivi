#include "manuscript/ChapterReorderer.hpp"

#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

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

    // 1. Read manuscript.meta.json
    const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<ReorderChapterResult>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<ReorderChapterResult>::failure(msParsed.error()); }

    auto& chapters = msParsed.value().chapters;

    // 2. Extract the ChapterRef
    auto srcIt = std::find_if(chapters.begin(), chapters.end(),
        [&](const schemas::ChapterRef& ref) {
            return ref.chapterID.value == request.chapterID.value;
        });

    if (srcIt == chapters.end()) {
        return Result<ReorderChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID not found in manuscript: " + request.chapterID.value});
    }

    schemas::ChapterRef movingRef = *srcIt;
    chapters.erase(srcIt);

    // 3. Insert at new position
    if (request.afterChapterID.value.empty()) {
        chapters.insert(chapters.begin(), std::move(movingRef));
    } else {
        auto insertAfter = std::find_if(chapters.begin(), chapters.end(),
            [&](const schemas::ChapterRef& ref) {
                return ref.chapterID.value == request.afterChapterID.value;
            });
        // If afterChapterID not found, append to end
        chapters.insert(
            insertAfter != chapters.end() ? std::next(insertAfter) : chapters.end(),
            std::move(movingRef));
    }

    // 4. Rewrite manuscript.meta.json atomically
    auto writeR = fs.atomicWriteTextFile(msMetaPath,
                                         schemas::serializeManuscriptMeta(msParsed.value()));
    if (!writeR.ok()) { return Result<ReorderChapterResult>::failure(writeR.error()); }

    ReorderChapterResult result;
    result.chapterID  = request.chapterID;
    result.reordered  = true;
    return Result<ReorderChapterResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
