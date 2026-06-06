#include "manuscript/ChapterRenamer.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <cctype>

namespace scrivi::manuscript {

ChapterRenamer::ChapterRenamer(CoreServices& services)
    : services_(services) {}

static std::string normaliseTitle(const std::string& title) {
    const bool allSpace = std::all_of(title.begin(), title.end(),
        [](unsigned char c){ return std::isspace(c); });
    return allSpace ? "" : title;
}

Result<RenameChapterResult> ChapterRenamer::rename(const RenameChapterRequest& request)
{
    if (request.metadataPath.empty()) {
        return Result<RenameChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "metadataPath must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string savedTitle  = normaliseTitle(request.newTitle);
    const std::string metaAbsPath = util::join(request.projectRootPath,
                                               request.metadataPath);

    // Read, update title, rewrite atomically.
    auto textR = fs.readTextFile(metaAbsPath);
    if (!textR.ok()) { return Result<RenameChapterResult>::failure(textR.error()); }

    auto parsedR = schemas::parseChapterMeta(textR.value());
    if (!parsedR.ok()) { return Result<RenameChapterResult>::failure(parsedR.error()); }

    auto& ch = parsedR.value();
    ch.title = savedTitle;

    auto writeR = fs.atomicWriteTextFile(metaAbsPath, schemas::serializeChapterMeta(ch));
    if (!writeR.ok()) { return Result<RenameChapterResult>::failure(writeR.error()); }

    RenameChapterResult result;
    result.metadataPath = request.metadataPath;
    result.newTitle     = savedTitle;
    result.renamed      = true;
    return Result<RenameChapterResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// findChapterMetadataPath — walks manuscript index to locate a chapter's
// metadata path by chapterID. Not used by rename (caller holds the path),
// but available for future operations that need to resolve a path from an ID.
// ---------------------------------------------------------------------------

Result<RelativePath> ChapterRenamer::findChapterMetadataPath(
    const AbsolutePath& projectRootPath, const ChapterID& chapterID)
{
    auto& fs = *services_.fileSystem;

    const std::string msMetaPath = util::join(projectRootPath,
                                              "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<RelativePath>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<RelativePath>::failure(msParsed.error()); }

    for (const auto& chRef : msParsed.value().chapters) {
        if (chRef.chapterID.value == chapterID.value) {
            return Result<RelativePath>::success(chRef.path);
        }
    }

    return Result<RelativePath>::failure(
        {.code = ErrorCode::invalidArgument,
         .message = "chapterID not found in manuscript: " + chapterID.value});
}

} // namespace scrivi::manuscript
