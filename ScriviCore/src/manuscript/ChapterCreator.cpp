#include "manuscript/ChapterCreator.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::manuscript {

ChapterCreator::ChapterCreator(CoreServices& services)
    : services_(services) {}

Result<CreateChapterResult> ChapterCreator::create(const CreateChapterRequest& request)
{
    if (request.author.identityID.value.empty() ||
        request.author.personaID.value.empty()) {
        return Result<CreateChapterResult>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "AuthorshipRef must have non-empty identityID and personaID"});
    }

    auto& fs  = *services_.fileSystem;
    auto& ids = *services_.uuidProvider;
    auto& clk = *services_.clock;

    const std::string& root = request.projectRootPath;
    const std::string  now  = clk.nowUTC();

    // 1. Read manuscript.meta.json to determine existing chapter count
    const std::string msMetaPath = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<CreateChapterResult>::failure(msTextR.error()); }

    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<CreateChapterResult>::failure(msParsed.error()); }

    auto& ms = msParsed.value();
    const std::size_t newChapterOrdinal = ms.chapters.size() + 1;

    // 2. Generate chapter ID and slug
    const ChapterID newChapterID = ids.newChapterID();
    const std::string chapterSlug = "chapter-" +
        std::to_string(newChapterOrdinal).insert(
            0, 3 - std::min<std::size_t>(3, std::to_string(newChapterOrdinal).size()), '0');

    const std::string chapterDir     = "manuscript/" + chapterSlug;
    const std::string chMetaRelPath  = chapterDir + "/chapter.meta.json";
    const std::string chMetaAbsPath  = util::join(root, chMetaRelPath);

    // 3. Create chapter directory
    auto dirR = fs.createDirectories(util::join(root, chapterDir));
    if (!dirR.ok()) { return Result<CreateChapterResult>::failure(dirR.error()); }

    // 4. Generate first scene ID and paths
    const SceneID firstSceneID = ids.newSceneID();
    const std::string sceneSlug       = "001-scene";
    const std::string sceneMetaRel    = chapterDir + "/" + sceneSlug + ".meta.json";
    const std::string sceneContentRel = chapterDir + "/" + sceneSlug + ".md";

    // 5. Write empty scene content file
    auto writeContentR = fs.atomicWriteTextFile(util::join(root, sceneContentRel), "");
    if (!writeContentR.ok()) { return Result<CreateChapterResult>::failure(writeContentR.error()); }

    // 6. Write scene metadata file
    schemas::SceneMetaData sceneMeta;
    sceneMeta.sceneID               = firstSceneID;
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

    auto writeSceneMetaR = fs.atomicWriteTextFile(
        util::join(root, sceneMetaRel),
        schemas::serializeSceneMeta(sceneMeta));
    if (!writeSceneMetaR.ok()) { return Result<CreateChapterResult>::failure(writeSceneMetaR.error()); }

    // 7. Write chapter.meta.json
    schemas::ChapterMetaData ch;
    ch.chapterID              = newChapterID;
    ch.title                  = "Chapter " + std::to_string(newChapterOrdinal);
    ch.slug                   = chapterSlug;
    ch.displayLabel           = "Chapter " + std::to_string(newChapterOrdinal);
    ch.status                 = "draft";
    ch.createdAt              = now;
    ch.createdByIdentityID    = request.author.identityID.value;
    ch.createdByPersonaID     = request.author.personaID.value;
    ch.createdByDisplayName   = request.author.displayName;
    ch.scenes.push_back({.sceneID = firstSceneID, .metadataPath = sceneMetaRel});

    auto writeChR = fs.atomicWriteTextFile(chMetaAbsPath, schemas::serializeChapterMeta(ch));
    if (!writeChR.ok()) { return Result<CreateChapterResult>::failure(writeChR.error()); }

    // 8. Append new ChapterRef to manuscript.meta.json
    ms.chapters.push_back({.chapterID = newChapterID, .path = chMetaRelPath});
    auto writeMsR = fs.atomicWriteTextFile(msMetaPath, schemas::serializeManuscriptMeta(ms));
    if (!writeMsR.ok()) { return Result<CreateChapterResult>::failure(writeMsR.error()); }

    CreateChapterResult result;
    result.chapterID              = newChapterID;
    result.chapterMetadataPath    = chMetaRelPath;
    result.firstSceneID           = firstSceneID;
    result.firstSceneMetadataPath = sceneMetaRel;
    result.firstSceneContentPath  = sceneContentRel;
    return Result<CreateChapterResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
