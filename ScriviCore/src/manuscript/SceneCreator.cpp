#include "manuscript/SceneCreator.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
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

    auto& ch = chParsed.value();

    // 3. Generate new scene ID and slug
    const SceneID newSceneID = ids.newSceneID();
    const std::size_t newOrdinal = ch.scenes.size() + 1;
    const std::string sceneSlug = std::to_string(newOrdinal).insert(
        0, 3 - std::min<std::size_t>(3, std::to_string(newOrdinal).size()), '0')
        + "-scene";

    const std::string sceneMetaRel    = chapterDir + "/" + sceneSlug + ".meta.json";
    const std::string sceneContentRel = chapterDir + "/" + sceneSlug + ".md";

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
    sceneMeta.contentPath           = sceneContentRel;
    sceneMeta.wordCount             = 0;
    sceneMeta.characterCount        = 0;

    auto writeMetaR = fs.atomicWriteTextFile(
        util::join(root, sceneMetaRel),
        schemas::serializeSceneMeta(sceneMeta));
    if (!writeMetaR.ok()) { return Result<CreateSceneResult>::failure(writeMetaR.error()); }

    // 6. Insert the new SceneRef into the chapter's scenes list.
    //    If afterSceneID is empty or not found, append to end.
    schemas::SceneRef newRef{.sceneID = newSceneID, .metadataPath = sceneMetaRel};

    if (!request.afterSceneID.value.empty()) {
        auto insertAfter = std::find_if(ch.scenes.begin(), ch.scenes.end(),
            [&](const schemas::SceneRef& ref) {
                return ref.sceneID.value == request.afterSceneID.value;
            });
        if (insertAfter != ch.scenes.end()) {
            ch.scenes.insert(std::next(insertAfter), std::move(newRef));
        } else {
            ch.scenes.push_back(std::move(newRef));
        }
    } else {
        ch.scenes.push_back(std::move(newRef));
    }

    // 7. Rewrite chapter.meta.json atomically
    auto writeChR = fs.atomicWriteTextFile(chMetaAbsPath, schemas::serializeChapterMeta(ch));
    if (!writeChR.ok()) { return Result<CreateSceneResult>::failure(writeChR.error()); }

    CreateSceneResult result;
    result.sceneID      = newSceneID;
    result.chapterID    = request.chapterID;
    result.metadataPath = sceneMetaRel;
    result.contentPath  = sceneContentRel;
    return Result<CreateSceneResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
