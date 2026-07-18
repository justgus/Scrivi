#include "manuscript/SceneCreator.hpp"

#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>

namespace scrivi::manuscript {

SceneCreator::SceneCreator(CoreServices& services)
    : services_(services) {}

Result<CreateSceneResult> SceneCreator::create(const CreateSceneRequest& request)
{
    if (request.author.identityID.value.empty() ||
        request.author.personaID.value.empty()) {
        return Result<CreateSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "AuthorshipRef must have non-empty identityID and personaID"});
    }

    auto& fs  = *services_.fileSystem;
    auto& ids = *services_.uuidProvider;
    auto& clk = *services_.clock;

    const std::string& root = request.projectRootPath;
    const std::string  now  = clk.nowUTC();

    // 1. Locate the chapter directory by searching manuscript.meta.json for the
    //    chapterID, then reading that chapter's meta to find its directory.
    const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<CreateSceneResult>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<CreateSceneResult>::failure(msParsed.error()); }

    // Find the ChapterRef for the requested chapterID
    const auto& chapters = msParsed.value().chapters;
    auto chIt = std::find_if(chapters.begin(), chapters.end(),
        [&](const schemas::ChapterRef& ref) {
            return ref.chapterID.value == request.chapterID.value;
        });
    if (chIt == chapters.end()) {
        return Result<CreateSceneResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "chapterID not found in manuscript: " + request.chapterID.value});
    }

    // chIt->path is relative to root and points to chapter.meta.json
    const std::string chMetaRelPath = chIt->path;
    const std::string chMetaAbsPath = util::join(root, chMetaRelPath);

    // The chapter directory is the directory containing chapter.meta.json
    const std::string chapterDir =
        std::filesystem::path(chMetaRelPath).parent_path().string();

    // 2. Read chapter metadata
    auto chTextR = fs.readTextFile(chMetaAbsPath);
    if (!chTextR.ok()) { return Result<CreateSceneResult>::failure(chTextR.error()); }

    auto chParsed = schemas::parseChapterMeta(chTextR.value());
    if (!chParsed.ok()) { return Result<CreateSceneResult>::failure(chParsed.error()); }

    // 3. Generate new scene ID and an ORDER-KEY filename (EP-027 §8.1). The key lands
    //    strictly between the caret's neighbours in DISK order (the folder scan), not the
    //    cache array — so the sorted filenames equal reading order. afterSceneID empty →
    //    insert at the front.
    const SceneID newSceneID = ids.newSceneID();

    auto onDiskR = listScenesByOrder(fs, root, chMetaRelPath);
    if (!onDiskR.ok()) { return Result<CreateSceneResult>::failure(onDiskR.error()); }
    const auto& onDisk = onDiskR.value();

    // Contract (matches pre-EP-027 SceneCreator): an empty OR unknown afterSceneID APPENDS
    // to the end of the chapter; a known anchor inserts immediately after it.
    std::string lo, hi;
    if (request.afterSceneID.value.empty()) {
        lo = onDisk.empty() ? std::string() : onDisk.back().orderKey;
        hi = std::string();
    } else {
        auto afterIt = std::find_if(onDisk.begin(), onDisk.end(),
            [&](const SceneEntry& e) { return e.sceneID.value == request.afterSceneID.value; });
        if (afterIt == onDisk.end()) {
            // Unknown anchor → append after the last scene.
            lo = onDisk.empty() ? std::string() : onDisk.back().orderKey;
            hi = std::string();
        } else {
            lo = afterIt->orderKey;
            auto nextIt = std::next(afterIt);
            hi = (nextIt == onDisk.end()) ? std::string() : nextIt->orderKey;
        }
    }
    const std::string orderKey = util::keyBetween(lo, hi);
    if (orderKey.empty()) {
        return Result<CreateSceneResult>::failure(
            {.code = ErrorCode::internalError,
             .message = "could not compute a scene order key between neighbours"});
    }

    const std::string sceneSlug       = orderKey + "-scene";
    const std::string metaFilename    = sceneSlug + ".meta.json";
    const std::string contentFilename = sceneSlug + ".md";
    const std::string sceneMetaRel    = chapterDir + "/" + metaFilename;
    const std::string sceneContentRel = chapterDir + "/" + contentFilename;

    // 4. Write empty scene content file
    auto writeR = fs.atomicWriteTextFile(util::join(root, sceneContentRel), "");
    if (!writeR.ok()) { return Result<CreateSceneResult>::failure(writeR.error()); }

    // 5. Write scene metadata file
    schemas::SceneMetaData sceneMeta;
    sceneMeta.sceneID               = newSceneID;
    sceneMeta.title                 = "";
    sceneMeta.slug                  = sceneSlug;
    sceneMeta.status                = "draft";
    sceneMeta.createdAt             = now;
    sceneMeta.createdByIdentityID   = request.author.identityID.value;
    sceneMeta.createdByPersonaID    = request.author.personaID.value;
    sceneMeta.createdByDisplayName  = request.author.displayName;
    sceneMeta.modifiedAt            = now;
    sceneMeta.modifiedByIdentityID  = request.author.identityID.value;
    sceneMeta.modifiedByPersonaID   = request.author.personaID.value;
    sceneMeta.modifiedByDisplayName = request.author.displayName;
    sceneMeta.contentPath           = contentFilename;   // §8.1: bare filename, resolved
                                                          // against the scene's own folder
    sceneMeta.wordCount             = 0;
    sceneMeta.characterCount        = 0;

    auto writeMetaR = fs.atomicWriteTextFile(
        util::join(root, sceneMetaRel),
        schemas::serializeSceneMeta(sceneMeta));
    if (!writeMetaR.ok()) { return Result<CreateSceneResult>::failure(writeMetaR.error()); }

    // 6. Rebuild the chapter's scenes[] cache from disk. The scene's on-disk order-key
    //    filename is now authoritative for order (B3), so we regenerate the ordered
    //    filename-only cache from the folder scan rather than hand-splicing the array.
    auto rebuildR = rebuildChapterScenesIfInconsistent(fs, root, chMetaRelPath);
    if (!rebuildR.ok()) { return Result<CreateSceneResult>::failure(rebuildR.error()); }

    CreateSceneResult result;
    result.sceneID      = newSceneID;
    result.chapterID    = request.chapterID;
    result.metadataPath = sceneMetaRel;
    result.contentPath  = sceneContentRel;
    return Result<CreateSceneResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
