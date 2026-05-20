#include "manuscript/ManuscriptOrderResolver.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::manuscript {

ManuscriptOrderResolver::ManuscriptOrderResolver(CoreServices& services)
    : services_(services) {}

Result<std::vector<ResolvedScene>> ManuscriptOrderResolver::resolve(
    const AbsolutePath& projectRoot)
{
    auto& fs = *services_.fileSystem;

    // Read manuscript.meta.json
    auto msPath = util::join(projectRoot, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msPath);
    if (!msTextR.ok()) return Result<std::vector<ResolvedScene>>::failure(msTextR.error());

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) return Result<std::vector<ResolvedScene>>::failure(msParsed.error());

    std::vector<ResolvedScene> scenes;

    for (auto& chapterRef : msParsed.value().chapters) {
        // chapterRef.path is relative to projectRoot and points to chapter.meta.json
        auto chPath = util::join(projectRoot, chapterRef.path);
        auto chTextR = fs.readTextFile(chPath);
        if (!chTextR.ok()) return Result<std::vector<ResolvedScene>>::failure(chTextR.error());

        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) return Result<std::vector<ResolvedScene>>::failure(chParsed.error());

        for (auto& sceneRef : chParsed.value().scenes) {
            // sceneRef.metadataPath is relative to projectRoot
            auto sMetaPath = util::join(projectRoot, sceneRef.metadataPath);
            auto sTextR = fs.readTextFile(sMetaPath);
            if (!sTextR.ok()) return Result<std::vector<ResolvedScene>>::failure(sTextR.error());

            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) return Result<std::vector<ResolvedScene>>::failure(sParsed.error());

            ResolvedScene rs;
            rs.sceneID      = sParsed.value().sceneID;
            rs.chapterID    = chParsed.value().chapterID;
            rs.title        = sParsed.value().title;
            rs.slug         = sParsed.value().slug;
            rs.status       = sParsed.value().status;
            rs.metadataPath = sceneRef.metadataPath;
            rs.contentPath  = sParsed.value().contentPath;
            scenes.push_back(std::move(rs));
        }
    }

    return Result<std::vector<ResolvedScene>>::success(std::move(scenes));
}

} // namespace scrivi::manuscript
