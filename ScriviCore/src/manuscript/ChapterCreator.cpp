#include "manuscript/ChapterCreator.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

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

    // 2. Generate chapter ID and an ORDER-KEY slug (EP-027 A4b). The folder name is
    //    `chapter-<orderKey>`; sorting folders by key == manuscript order. The new chapter
    //    is created IN PLACE at its final position (EP-027 P6): the key is chosen strictly
    //    between `afterChapterID` and its successor, so the folder is born with its final
    //    name and never needs a rename (which was the source of the split's stale-path bug).
    //    afterChapterID empty (or unknown) → append at the end. Collision-free by
    //    construction — the fix for the I-0072 `count+1` slug reuse-after-delete corruption.
    const ChapterID newChapterID = ids.newChapterID();

    auto chaptersR = listChaptersByOrder(fs, root);
    if (!chaptersR.ok()) { return Result<CreateChapterResult>::failure(chaptersR.error()); }
    const auto& existing = chaptersR.value();

    // Compute the (lo, hi) key window the new chapter lands between (mirrors
    // ChapterReorderer so the two paths can't diverge). No chapter is being moved here, so
    // `existing` needs no filtering.
    std::string lo, hi;
    if (request.afterChapterID.value.empty()) {
        lo = existing.empty() ? std::string() : existing.back().orderKey;
        hi = std::string();
    } else {
        auto afterIt = std::find_if(existing.begin(), existing.end(),
            [&](const ChapterEntry& e) { return e.chapterID.value == request.afterChapterID.value; });
        if (afterIt == existing.end()) {
            // Unknown anchor → append at the end (defensive; matches reorder).
            lo = existing.empty() ? std::string() : existing.back().orderKey;
            hi = std::string();
        } else {
            lo = afterIt->orderKey;
            auto nextIt = std::next(afterIt);
            hi = (nextIt == existing.end()) ? std::string() : nextIt->orderKey;
        }
    }

    const std::string orderKey = util::keyBetween(lo, hi);
    if (orderKey.empty()) {
        return Result<CreateChapterResult>::failure(
            {.code = ErrorCode::internalError,
             .message = "could not compute a chapter order key between neighbours"});
    }

    const std::string chapterSlug = "chapter-" + orderKey;
    const std::string chapterDir     = "manuscript/" + chapterSlug;
    const std::string chMetaRelPath  = chapterDir + "/chapter.meta.json";
    const std::string chMetaAbsPath  = util::join(root, chMetaRelPath);

    // 3. Create chapter directory
    auto dirR = fs.createDirectories(util::join(root, chapterDir));
    if (!dirR.ok()) { return Result<CreateChapterResult>::failure(dirR.error()); }

    // 4. Generate first scene ID and paths
    const SceneID firstSceneID = ids.newSceneID();
    // The first scene's slug prefix "001" is a valid order key, so the new chapter is born
    // in the new scheme (EP-027 §8.1) — no migration on first open.
    const std::string sceneSlug       = "001-scene";
    const std::string sceneFilename   = sceneSlug + ".meta.json";
    const std::string sceneContentFilename = sceneSlug + ".md";
    const std::string sceneMetaRel    = chapterDir + "/" + sceneFilename;
    const std::string sceneContentRel = chapterDir + "/" + sceneContentFilename;

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
    sceneMeta.contentPath           = sceneContentFilename;   // §8.1: bare filename
    sceneMeta.wordCount             = 0;
    sceneMeta.characterCount        = 0;

    auto writeSceneMetaR = fs.atomicWriteTextFile(
        util::join(root, sceneMetaRel),
        schemas::serializeSceneMeta(sceneMeta));
    if (!writeSceneMetaR.ok()) { return Result<CreateChapterResult>::failure(writeSceneMetaR.error()); }

    // 7. Write chapter.meta.json. The display ordinal is the new chapter's POSITION
    //    (it appends last → existing count + 1), derived from order — not from a folder
    //    number. (The durable "Chapter N" numbering is order-derived in the display layer;
    //    this stored title is a starting label, kept here to preserve current behavior.)
    const std::size_t newChapterOrdinal = existing.size() + 1;
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
    ch.scenes.push_back({.metadataFilename = sceneFilename});   // §8.1: filename-only

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
