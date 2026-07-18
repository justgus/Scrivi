#include "manuscript/ManuscriptOrderResolver.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::manuscript {

ManuscriptOrderResolver::ManuscriptOrderResolver(CoreServices& services)
    : services_(services) {}

Result<std::vector<ResolvedScene>> ManuscriptOrderResolver::resolve(
    const AbsolutePath& projectRoot) const
{
    auto& fs = *services_.fileSystem;

    // EP-027 B3: manuscript order is FILESYSTEM-AUTHORITATIVE — iterate chapters in
    // order-key sort order straight from the on-disk `chapter-*` folders, NOT from the
    // manuscript.meta.json array (which is a rebuildable cache, and may be stale/corrupt,
    // e.g. the I-0072 phantom/duplicate entries). listChaptersByOrder reads each sidecar
    // for the authoritative chapterID, so a divergent index can never mis-order or lose a
    // chapter here.
    auto chaptersR = listChaptersByOrder(fs, projectRoot);
    if (!chaptersR.ok()) {
        return Result<std::vector<ResolvedScene>>::failure(chaptersR.error());
    }

    std::vector<ResolvedScene> scenes;

    for (auto& chapterEntry : chaptersR.value()) {
        const std::string& chapterRelPath = chapterEntry.chapterMetadataRelPath;
        auto chPath = util::join(projectRoot, chapterRelPath);
        auto chTextR = fs.readTextFile(chPath);
        if (!chTextR.ok()) { return Result<std::vector<ResolvedScene>>::failure(chTextR.error());
}

        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) { return Result<std::vector<ResolvedScene>>::failure(chParsed.error());
}

        // EP-027 §8.1: scene order within the chapter is FILESYSTEM-AUTHORITATIVE — the
        // order-key filename sort of the on-disk scene files, not the cache array.
        auto scenesR = listScenesByOrder(fs, projectRoot, chapterRelPath);
        if (!scenesR.ok()) { return Result<std::vector<ResolvedScene>>::failure(scenesR.error()); }

        const std::string chDir = chapterDirOf(chapterRelPath);

        for (auto& sceneEntry : scenesR.value()) {
            auto sTextR = fs.readTextFile(util::join(projectRoot, sceneEntry.metadataRelPath));
            if (!sTextR.ok()) { return Result<std::vector<ResolvedScene>>::failure(sTextR.error());
}

            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) { return Result<std::vector<ResolvedScene>>::failure(sParsed.error());
}

            ResolvedScene rs;
            rs.sceneID              = sParsed.value().sceneID;
            rs.chapterID            = chParsed.value().chapterID;
            rs.title                = sParsed.value().title;
            rs.chapterTitle         = chParsed.value().title;
            rs.slug                 = sParsed.value().slug;
            rs.status               = sParsed.value().status;
            rs.metadataPath         = sceneEntry.metadataRelPath;
            // §8.1: sidecar contentPath is a bare filename — resolve against the chapter dir.
            rs.contentPath          = chDir + "/" + sParsed.value().contentPath;
            rs.chapterMetadataPath  = chapterRelPath;
            scenes.push_back(std::move(rs));
        }
    }

    return Result<std::vector<ResolvedScene>>::success(std::move(scenes));
}

} // namespace scrivi::manuscript
