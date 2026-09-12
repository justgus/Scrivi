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
            rs.storyTime            = sParsed.value().storyTime;   // EP-039 AC2
            scenes.push_back(std::move(rs));
        }
    }

    return Result<std::vector<ResolvedScene>>::success(std::move(scenes));
}

Result<ResolvedScene> ManuscriptOrderResolver::findScene(
    const AbsolutePath& projectRoot, const SceneID& sceneID) const
{
    // [I-0196]. See the header for why this exists rather than calling resolve().
    auto& fs = *services_.fileSystem;

    auto chaptersR = listChaptersByOrder(fs, projectRoot);
    if (!chaptersR.ok()) { return Result<ResolvedScene>::failure(chaptersR.error()); }

    for (auto& chapterEntry : chaptersR.value()) {
        const std::string& chapterRelPath = chapterEntry.chapterMetadataRelPath;

        // ⚠️ The scene listing reads each scene sidecar for its AUTHORITATIVE
        // sceneID (EP-027 §8.1), so the match below is filesystem-authoritative
        // -- the same guarantee resolve() gives, not a weaker index lookup.
        auto scenesR = listScenesByOrder(fs, projectRoot, chapterRelPath);
        if (!scenesR.ok()) { return Result<ResolvedScene>::failure(scenesR.error()); }

        for (auto& sceneEntry : scenesR.value()) {
            if (sceneEntry.sceneID.value != sceneID.value) { continue; }

            // ✅ ONLY NOW do we pay for the chapter + scene sidecar parse -- once,
            // for the scene actually asked for, instead of once per scene in the
            // manuscript.
            auto chTextR = fs.readTextFile(util::join(projectRoot, chapterRelPath));
            if (!chTextR.ok()) { return Result<ResolvedScene>::failure(chTextR.error()); }
            auto chParsed = schemas::parseChapterMeta(chTextR.value());
            if (!chParsed.ok()) { return Result<ResolvedScene>::failure(chParsed.error()); }

            auto sTextR = fs.readTextFile(
                util::join(projectRoot, sceneEntry.metadataRelPath));
            if (!sTextR.ok()) { return Result<ResolvedScene>::failure(sTextR.error()); }
            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) { return Result<ResolvedScene>::failure(sParsed.error()); }

            ResolvedScene rs;
            rs.sceneID             = sParsed.value().sceneID;
            rs.chapterID           = chParsed.value().chapterID;
            rs.title               = sParsed.value().title;
            rs.chapterTitle        = chParsed.value().title;
            rs.slug                = sParsed.value().slug;
            rs.status              = sParsed.value().status;
            rs.metadataPath        = sceneEntry.metadataRelPath;
            // §8.1: sidecar contentPath is a bare filename — resolve against the
            // chapter dir, exactly as resolve() does.
            rs.contentPath         = chapterDirOf(chapterRelPath) + "/"
                                   + sParsed.value().contentPath;
            rs.chapterMetadataPath = chapterRelPath;
            return Result<ResolvedScene>::success(std::move(rs));
        }
    }

    return Result<ResolvedScene>::failure(
        Error{.code = ErrorCode::invalidArgument,
              .message = "Scene not found: " + sceneID.value});
}

} // namespace scrivi::manuscript
