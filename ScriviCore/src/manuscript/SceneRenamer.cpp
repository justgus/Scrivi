#include "manuscript/SceneRenamer.hpp"

#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <cctype>

namespace scrivi::manuscript {

SceneRenamer::SceneRenamer(CoreServices& services)
    : services_(services) {}

static std::string normaliseTitle(const std::string& title) {
    const bool allSpace = std::all_of(title.begin(), title.end(),
        [](unsigned char c){ return std::isspace(c); });
    return allSpace ? "" : title;
}

Result<RenameSceneResult> SceneRenamer::rename(const RenameSceneRequest& request)
{
    if (request.metadataPath.empty()) {
        return Result<RenameSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "metadataPath must not be empty"});
    }

    auto& fs = *services_.fileSystem;
    const std::string savedTitle   = normaliseTitle(request.newTitle);
    const std::string metaAbsPath  = util::join(request.projectRootPath,
                                                request.metadataPath);

    // Read, update title, rewrite atomically.
    auto textR = fs.readTextFile(metaAbsPath);
    if (!textR.ok()) { return Result<RenameSceneResult>::failure(textR.error()); }

    auto parsedR = schemas::parseSceneMeta(textR.value());
    if (!parsedR.ok()) { return Result<RenameSceneResult>::failure(parsedR.error()); }

    auto& meta = parsedR.value();
    meta.title = savedTitle;

    auto writeR = fs.atomicWriteTextFile(metaAbsPath, schemas::serializeSceneMeta(meta));
    if (!writeR.ok()) { return Result<RenameSceneResult>::failure(writeR.error()); }

    RenameSceneResult result;
    result.metadataPath = request.metadataPath;
    result.newTitle     = savedTitle;
    result.renamed      = true;
    return Result<RenameSceneResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// findSceneMetadataPath — walks manuscript index to locate a scene's metadata
// path by sceneID. Not used by rename (caller holds the path), but available
// for future operations that need to resolve a path from an ID.
// ---------------------------------------------------------------------------

Result<RelativePath> SceneRenamer::findSceneMetadataPath(
    const AbsolutePath& projectRootPath, const SceneID& sceneID)
{
    auto& fs = *services_.fileSystem;

    const std::string msMetaPath = util::join(projectRootPath,
                                              "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<RelativePath>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<RelativePath>::failure(msParsed.error()); }

    // EP-027 §8.1: scan the on-disk scene files (identity from each sidecar); return the
    // scene's root-relative meta path.
    for (const auto& chRef : msParsed.value().chapters) {
        auto scenesR = listScenesByOrder(fs, projectRootPath, chRef.path);
        if (!scenesR.ok()) { continue; }
        for (const auto& e : scenesR.value()) {
            if (e.sceneID.value == sceneID.value) {
                return Result<RelativePath>::success(e.metadataRelPath);
            }
        }
    }

    return Result<RelativePath>::failure(
        {.code = ErrorCode::invalidArgument,
         .message = "sceneID not found in any chapter: " + sceneID.value});
}

} // namespace scrivi::manuscript
