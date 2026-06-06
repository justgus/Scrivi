#include "manuscript/SceneDeleter.hpp"

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

    // 1. Read manuscript.meta.json to find which chapter owns the scene
    const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<DeleteSceneResult>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<DeleteSceneResult>::failure(msParsed.error()); }

    const auto& chapters = msParsed.value().chapters;

    // 2. Search each chapter for the scene
    for (const auto& chRef : chapters) {
        const std::string chMetaAbsPath = util::join(root, chRef.path);

        auto chTextR = fs.readTextFile(chMetaAbsPath);
        if (!chTextR.ok()) { continue; }

        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) { continue; }

        auto& ch = chParsed.value();
        auto it = std::find_if(ch.scenes.begin(), ch.scenes.end(),
            [&](const schemas::SceneRef& ref) {
                return ref.sceneID.value == request.sceneID.value;
            });

        if (it == ch.scenes.end()) { continue; }

        // Found it — record file paths before erasing the ref
        const std::string metaAbsPath    = util::join(root, it->metadataPath);
        const std::string metaRelPath    = it->metadataPath;

        // Derive content path from metadata path (replace .meta.json with .md)
        std::string contentRelPath = metaRelPath;
        const std::string suffix = ".meta.json";
        if (contentRelPath.size() > suffix.size() &&
            contentRelPath.substr(contentRelPath.size() - suffix.size()) == suffix) {
            contentRelPath.replace(contentRelPath.size() - suffix.size(),
                                   suffix.size(), ".md");
        }
        const std::string contentAbsPath = util::join(root, contentRelPath);

        // 3. Remove scene from chapter scenes list and rewrite chapter.meta.json
        ch.scenes.erase(it);
        auto writeChR = fs.atomicWriteTextFile(chMetaAbsPath,
                                               schemas::serializeChapterMeta(ch));
        if (!writeChR.ok()) { return Result<DeleteSceneResult>::failure(writeChR.error()); }

        // 4. Delete scene files from disk (best-effort — ignore missing)
        std::error_code ec;
        std::filesystem::remove(metaAbsPath, ec);
        std::filesystem::remove(contentAbsPath, ec);

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
