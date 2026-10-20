#include "manuscript/RangeOperations.hpp"

#include "domain/TextStats.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>
#include <unordered_set>

namespace scrivi::manuscript {

namespace {

Error invalid(std::string message) { return Error{.code = ErrorCode::invalidArgument, .message = std::move(message)}; }
Error internal(std::string message) { return Error{.code = ErrorCode::internalError, .message = std::move(message)}; }

std::string fileNameOf(const std::string& rel) {
    const auto slash = rel.find_last_of('/');
    return slash == std::string::npos ? rel : rel.substr(slash + 1);
}

std::string dirOf(const std::string& rel) {
    const auto slash = rel.find_last_of('/');
    return slash == std::string::npos ? std::string() : rel.substr(0, slash);
}

// "manuscript/chapter-<key>/chapter.meta.json" → <key>
std::string chapterKeyOf(const std::string& chapterMetaRel) {
    const std::string folder = fileNameOf(dirOf(chapterMetaRel));
    constexpr std::string_view prefix = "chapter-";
    return folder.starts_with(prefix) ? folder.substr(prefix.size()) : std::string();
}

std::string sceneKeyOf(const std::string& sceneMetaRel) { return sceneOrderKeyOf(fileNameOf(sceneMetaRel)); }

// ⚠️ Offsets are UTF-8 bytes (design §4.2): one that lands inside a multi-byte sequence would split a character on disk.
bool onUtf8Boundary(const std::string& s, std::size_t offset) {
    if (offset > s.size()) { return false; }
    return offset == s.size() || (static_cast<unsigned char>(s[offset]) & 0xC0) != 0x80;
}

bool authorIsValid(const AuthorshipRef& a) { return !a.identityID.value.empty() && !a.personaID.value.empty(); }

struct ChapterInfo {
    ChapterID    chapterID;
    std::string  title;
    RelativePath metadataPath;
};

// The chapters the index knows, in reading order. ⚠️ A chapter with no scenes has no entry in the index, so it is not
// here: a new chapter's folder is therefore PROBED before it is used (see `freeChapterKey`).
std::vector<ChapterInfo> chaptersInOrder(const ProjectIndex& index) {
    std::vector<ChapterInfo> out;
    for (const auto& s : index.scenesInOrder()) {
        if (out.empty() || out.back().chapterID.value != s.chapterID.value) {
            out.push_back({s.chapterID, s.chapterTitle, s.chapterMetadataPath});
        }
    }
    return out;
}

std::vector<SceneLocation> scenesOfChapter(const ProjectIndex& index, const ChapterID& chapterID) {
    std::vector<SceneLocation> out;
    for (const auto& s : index.scenesInOrder()) {
        if (s.chapterID.value == chapterID.value) { out.push_back(s); }
    }
    return out;
}

schemas::SceneMetaData newSceneMeta(const SceneID& id, const std::string& title, const std::string& slug,
                                    const std::string& contentFilename, const std::string& text,
                                    const AuthorshipRef& author, const std::string& now) {
    const auto stats = util::countText(text);
    schemas::SceneMetaData m;
    m.sceneID               = id;
    m.title                 = title;
    m.slug                  = slug;
    m.status                = "draft";
    m.createdAt             = now;
    m.createdByIdentityID   = author.identityID.value;
    m.createdByPersonaID    = author.personaID.value;
    m.createdByDisplayName  = author.displayName;
    m.modifiedAt            = now;
    m.modifiedByIdentityID  = author.identityID.value;
    m.modifiedByPersonaID   = author.personaID.value;
    m.modifiedByDisplayName = author.displayName;
    m.contentPath           = contentFilename;   // §8.1: a bare filename, resolved against the scene's own folder
    m.wordCount             = stats.wordCount;
    m.characterCount        = stats.characterCount;
    return m;
}

// The scene's sidecar after its text changed: modified stamp and counts, as `SceneWriter::save` writes them.
// One read, one write. ⛔ No `workspace-state.json` (that is the editor's own save's business).
Result<void> restampSceneSidecar(FileSystem& fs, const AbsolutePath& root, const RelativePath& metaRel,
                                 const std::string& text, const AuthorshipRef& author, const std::string& now) {
    const AbsolutePath path = util::join(root, metaRel);
    auto textR = fs.readTextFile(path);
    if (!textR.ok()) { return Result<void>::failure(textR.error()); }
    auto parsed = schemas::parseSceneMeta(textR.value());
    if (!parsed.ok()) { return Result<void>::failure(parsed.error()); }
    auto& meta = parsed.value();
    const auto stats = util::countText(text);
    meta.modifiedAt            = now;
    meta.modifiedByIdentityID  = author.identityID.value;
    meta.modifiedByPersonaID   = author.personaID.value;
    meta.modifiedByDisplayName = author.displayName;
    meta.wordCount             = stats.wordCount;
    meta.characterCount        = stats.characterCount;
    return fs.atomicWriteTextFile(path, schemas::serializeSceneMeta(meta));
}

// The chapter sidecar's `scenes[]` list, written from what the operation knows. One read (its other fields), one write.
Result<void> writeChapterScenes(FileSystem& fs, const AbsolutePath& root, const RelativePath& chapterMetaRel,
                                const std::vector<std::string>& metaFilenames) {
    const AbsolutePath path = util::join(root, chapterMetaRel);
    auto textR = fs.readTextFile(path);
    if (!textR.ok()) { return Result<void>::failure(textR.error()); }
    auto parsed = schemas::parseChapterMeta(textR.value());
    if (!parsed.ok()) { return Result<void>::failure(parsed.error()); }
    auto& ch = parsed.value();
    ch.scenes.clear();
    for (const auto& name : metaFilenames) { ch.scenes.push_back({.metadataFilename = name}); }
    return fs.atomicWriteTextFile(path, schemas::serializeChapterMeta(ch));
}

std::vector<std::string> metaFilenamesOf(const std::vector<SceneLocation>& scenes) {
    std::vector<std::string> out;
    out.reserve(scenes.size());
    for (const auto& s : scenes) { out.push_back(fileNameOf(s.metadataPath)); }
    return out;
}

// A chapter key strictly between `lo` and `hi` whose folder does not exist yet.
Result<std::string> freeChapterKey(FileSystem& fs, const AbsolutePath& root, std::string lo, const std::string& hi) {
    for (int attempt = 0; attempt < 64; ++attempt) {
        const std::string key = util::keyBetween(lo, hi);
        if (key.empty()) { break; }
        auto there = fs.exists(util::join(root, "manuscript/chapter-" + key));
        if (!there.ok()) { return Result<std::string>::failure(there.error()); }
        if (!there.value()) { return Result<std::string>::success(key); }
        lo = key;   // an empty chapter the index cannot see holds this key; move along
    }
    return Result<std::string>::failure(internal("could not find a free chapter order key"));
}

// A scene key strictly between `lo` and `hi` whose sidecar does not exist yet in `dir`.
Result<std::string> freeSceneKey(FileSystem& fs, const AbsolutePath& root, const std::string& dir, std::string lo,
                                 const std::string& hi) {
    for (int attempt = 0; attempt < 64; ++attempt) {
        const std::string key = util::keyBetween(lo, hi);
        if (key.empty()) { break; }
        auto there = fs.exists(util::join(root, dir + "/" + key + "-scene.meta.json"));
        if (!there.ok()) { return Result<std::string>::failure(there.error()); }
        if (!there.value()) { return Result<std::string>::success(key); }
        lo = key;
    }
    return Result<std::string>::failure(internal("could not find a free scene order key"));
}

Result<void> editManuscriptChapters(FileSystem& fs, const AbsolutePath& root,
                                    const std::function<void(std::vector<schemas::ChapterRef>&)>& edit) {
    const AbsolutePath path = util::join(root, "manuscript/manuscript.meta.json");
    auto textR = fs.readTextFile(path);
    if (!textR.ok()) { return Result<void>::failure(textR.error()); }
    auto parsed = schemas::parseManuscriptMeta(textR.value());
    if (!parsed.ok()) { return Result<void>::failure(parsed.error()); }
    edit(parsed.value().chapters);
    return fs.atomicWriteTextFile(path, schemas::serializeManuscriptMeta(parsed.value()));
}

}   // namespace

RangeOperations::RangeOperations(CoreServices& services, AbsolutePath projectRoot, ProjectIndex& index)
    : services_(services), root_(std::move(projectRoot)), index_(index) {}

// ── read ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────

Result<Range> RangeOperations::read(CoreServices& services, const AbsolutePath& projectRoot, const ProjectIndex& index,
                                    const SceneID& startScene, std::size_t startByte,
                                    const SceneID& endScene, std::size_t endByte)
{
    using R = Result<Range>;
    const auto start = index.findScene(startScene);
    const auto end   = index.findScene(endScene);
    if (!start) { return R::failure(invalid("range start scene not in manuscript: " + startScene.value)); }
    if (!end)   { return R::failure(invalid("range end scene not in manuscript: " + endScene.value)); }
    if (end->ordinal < start->ordinal) { return R::failure(invalid("the range ends before it starts")); }

    auto& fs = *services.fileSystem;
    Range out;
    for (std::size_t ord = start->ordinal; ord <= end->ordinal; ++ord) {
        const SceneLocation& loc = index.scenesInOrder()[ord];
        auto bodyR = fs.readTextFile(util::join(projectRoot, loc.contentPath));
        if (!bodyR.ok()) { return R::failure(bodyR.error()); }
        const std::string& body = bodyR.value();
        const std::size_t from = ord == start->ordinal ? startByte : 0;
        const std::size_t to   = ord == end->ordinal ? endByte : body.size();
        if (!onUtf8Boundary(body, from) || !onUtf8Boundary(body, to) || to < from) {
            return R::failure(invalid("range offsets are outside scene " + loc.sceneID.value +
                                      " or inside a UTF-8 sequence"));
        }
        RangeEntry e;
        e.sceneID      = loc.sceneID;
        e.chapterID    = loc.chapterID;
        e.chapterTitle = loc.chapterTitle;
        e.sceneTitle   = loc.title;
        e.start        = from;
        e.length       = to - from;
        e.sceneLength  = body.size();
        e.text         = body.substr(from, to - from);
        out.append(std::move(e));
    }
    return R::success(std::move(out));
}

// ── paste ────────────────────────────────────────────────────────────────────────────────────────────────────────────────

Result<RangeOperationResult> RangeOperations::paste(const Range& range, const SceneID& caretScene, std::size_t caretByte,
                                                    const std::string& caretSceneText, const AuthorshipRef& author)
{
    using R = Result<RangeOperationResult>;
    if (range.empty()) { return R::failure(invalid("an empty range: nothing to paste")); }
    if (!authorIsValid(author)) { return R::failure(invalid("AuthorshipRef must have non-empty identityID and personaID")); }
    const auto caret = index_.findScene(caretScene);
    if (!caret) { return R::failure(invalid("caret scene not in manuscript: " + caretScene.value)); }

    auto& fs = *services_.fileSystem;
    const std::string now = services_.clock->nowUTC();

    // READ 1: the caret scene's text — and the proof the editor is not stale.
    auto bodyR = fs.readTextFile(util::join(root_, caret->contentPath));
    if (!bodyR.ok()) { return R::failure(bodyR.error()); }
    const std::string body = bodyR.value();
    if (body != caretSceneText) {
        return R::failure(invalid("the caret scene's text is not what is on disk (the editor is stale): " +
                                  caretScene.value));
    }
    if (!onUtf8Boundary(body, caretByte)) {
        return R::failure(invalid("caret byte offset is past the scene's end or inside a UTF-8 sequence"));
    }
    const std::string head = body.substr(0, caretByte);
    const std::string tail = body.substr(caretByte);

    // Where the caret is, from the index.
    const auto chapters = chaptersInOrder(index_);
    const auto caretChapterIt = std::find_if(chapters.begin(), chapters.end(),
        [&](const ChapterInfo& c) { return c.chapterID.value == caret->chapterID.value; });
    const std::size_t caretChapterPos = static_cast<std::size_t>(caretChapterIt - chapters.begin());
    const std::string nextChapterKey  = caretChapterPos + 1 < chapters.size()
                                        ? chapterKeyOf(chapters[caretChapterPos + 1].metadataPath) : std::string();
    const std::vector<SceneLocation> caretChapterScenes = scenesOfChapter(index_, caret->chapterID);
    const auto caretInChapter = std::find_if(caretChapterScenes.begin(), caretChapterScenes.end(),
        [&](const SceneLocation& s) { return s.sceneID.value == caret->sceneID.value; });
    const std::vector<SceneLocation> before(caretChapterScenes.begin(), std::next(caretInChapter));
    const std::vector<SceneLocation> after(std::next(caretInChapter), caretChapterScenes.end());
    const std::string caretDir = dirOf(caret->chapterMetadataPath);

    std::vector<const RangeEntry*> entries;
    for (const auto& e : range.entries()) { entries.push_back(&e); }
    std::size_t lastOpening = 0;   // 0 = the range opens no chapter
    for (std::size_t i = 1; i < entries.size(); ++i) {
        if (entries[i]->chapterID.value != entries[i - 1]->chapterID.value) { lastOpening = i; }
    }
    // ✅ [I-0311]: the caret chapter's scenes after the caret follow the paste into its last chapter. They keep their file
    // names (their keys already sort after the caret's), so the scenes created in that chapter take keys BELOW the first.
    const bool movesAfter = lastOpening != 0 && !after.empty();
    const std::string movedFloor = movesAfter ? sceneKeyOf(after.front().metadataPath) : std::string();

    // ✅ Identity (user, 2026-10-20): the Range's own ID when the manuscript does not have it (a cut), else a fresh one.
    std::unordered_set<std::string> usedScenes, usedChapters;
    for (const auto& c : chapters) { usedChapters.insert(c.chapterID.value); }
    auto pickSceneID = [&](const SceneID& wanted) {
        if (!wanted.value.empty() && !wanted.value.starts_with("~") && !index_.findScene(wanted) &&
            usedScenes.insert(wanted.value).second) {
            return wanted;
        }
        SceneID fresh = services_.uuidProvider->newSceneID();
        usedScenes.insert(fresh.value);
        return fresh;
    };
    auto pickChapterID = [&](const ChapterID& wanted) {
        if (!wanted.value.empty() && !wanted.value.starts_with("~") && usedChapters.insert(wanted.value).second) {
            return wanted;
        }
        ChapterID fresh = services_.uuidProvider->newChapterID();
        usedChapters.insert(fresh.value);
        return fresh;
    };

    struct NewChapter {
        ChapterID                chapterID;
        std::string              title;
        std::string              dir;
        RelativePath             metadataPath;
        std::vector<std::string> sceneFiles;
    };
    struct NewScene {
        SceneLocation location;
        std::string   text;
    };
    std::vector<NewChapter> newChapters;
    std::vector<NewScene>   created;

    // The running insertion point: the chapter (and the key) the next entry goes after.
    ChapterID    runChapter      = caret->chapterID;
    std::string  runChapterTitle = caret->chapterTitle;
    RelativePath runChapterMeta  = caret->chapterMetadataPath;
    std::string  runChapterKey   = chapterKeyOf(caret->chapterMetadataPath);
    std::string  runDir          = caretDir;
    bool         runIsNew        = false;
    std::string  prevSceneKey    = sceneKeyOf(caret->metadataPath);
    std::string  sceneCeiling    = after.empty() ? std::string() : sceneKeyOf(after.front().metadataPath);
    std::size_t  chapterNumber   = caretChapterPos + 1;   // 1-based position of the running chapter

    for (std::size_t i = 1; i < entries.size(); ++i) {
        const RangeEntry& e = *entries[i];
        if (e.chapterID.value != entries[i - 1]->chapterID.value) {
            auto key = freeChapterKey(fs, root_, runChapterKey, nextChapterKey);
            if (!key.ok()) { return R::failure(key.error()); }
            ++chapterNumber;
            NewChapter ch;
            ch.chapterID    = pickChapterID(e.chapterID);
            ch.title        = e.chapterTitle.empty() ? "Chapter " + std::to_string(chapterNumber) : e.chapterTitle;
            ch.dir          = "manuscript/chapter-" + key.value();
            ch.metadataPath = ch.dir + "/chapter.meta.json";
            runChapter      = ch.chapterID;
            runChapterTitle = ch.title;
            runChapterMeta  = ch.metadataPath;
            runChapterKey   = key.value();
            runDir          = ch.dir;
            runIsNew        = true;
            prevSceneKey.clear();
            sceneCeiling    = (i == lastOpening && movesAfter) ? movedFloor : std::string();
            newChapters.push_back(std::move(ch));
        }
        // ✅ A new chapter's folder was probed free, so its scene keys need no probe; the caret chapter's might collide
        // with a file the index has never seen.
        Result<std::string> key = runIsNew
            ? Result<std::string>::success(util::keyBetween(prevSceneKey, sceneCeiling))
            : freeSceneKey(fs, root_, runDir, prevSceneKey, sceneCeiling);
        if (!key.ok()) { return R::failure(key.error()); }
        if (key.value().empty()) { return R::failure(internal("could not compute a scene order key between neighbours")); }
        const std::string slug = key.value() + "-scene";

        SceneLocation loc;
        loc.sceneID             = pickSceneID(e.sceneID);
        loc.chapterID           = runChapter;
        loc.title               = e.sceneTitle;
        loc.chapterTitle        = runChapterTitle;
        loc.slug                = slug;
        loc.status              = "draft";
        loc.metadataPath        = runDir + "/" + slug + ".meta.json";
        loc.contentPath         = runDir + "/" + slug + ".md";
        loc.chapterMetadataPath = runChapterMeta;
        if (runIsNew) { newChapters.back().sceneFiles.push_back(slug + ".meta.json"); }
        created.push_back({std::move(loc), e.text});
        prevSceneKey = key.value();
    }

    // The flat-document rule (v0.1 §4.2, kept): head + the first entry; the tail follows the whole pasted run.
    std::string caretText = head + entries.front()->text;
    if (created.empty()) { caretText += tail; } else { created.back().text += tail; }

    std::vector<SceneLocation> moved;
    if (movesAfter) {
        NewChapter& last = newChapters.back();
        for (const auto& a : after) {
            SceneLocation m       = a;
            m.chapterID           = last.chapterID;
            m.chapterTitle        = last.title;
            m.metadataPath        = last.dir + "/" + fileNameOf(a.metadataPath);
            m.contentPath         = last.dir + "/" + fileNameOf(a.contentPath);
            m.chapterMetadataPath = last.metadataPath;
            last.sceneFiles.push_back(fileNameOf(a.metadataPath));
            moved.push_back(std::move(m));
        }
    }

    // ── The writes: new files first, so a failure part-way never leaves the caret scene describing what is not there ──
    for (const auto& ch : newChapters) {
        auto dir = fs.createDirectories(util::join(root_, ch.dir));
        if (!dir.ok()) { return R::failure(dir.error()); }
        schemas::ChapterMetaData meta;
        meta.chapterID            = ch.chapterID;
        meta.title                = ch.title;
        meta.slug                 = fileNameOf(ch.dir);
        meta.displayLabel         = ch.title;
        meta.status               = "draft";
        meta.createdAt            = now;
        meta.createdByIdentityID  = author.identityID.value;
        meta.createdByPersonaID   = author.personaID.value;
        meta.createdByDisplayName = author.displayName;
        for (const auto& name : ch.sceneFiles) { meta.scenes.push_back({.metadataFilename = name}); }
        auto w = fs.atomicWriteTextFile(util::join(root_, ch.metadataPath), schemas::serializeChapterMeta(meta));
        if (!w.ok()) { return R::failure(w.error()); }
    }
    for (const auto& s : created) {
        auto md = fs.atomicWriteTextFile(util::join(root_, s.location.contentPath), s.text);
        if (!md.ok()) { return R::failure(md.error()); }
        const auto meta = newSceneMeta(s.location.sceneID, s.location.title, s.location.slug,
                                       fileNameOf(s.location.contentPath), s.text, author, now);
        auto mw = fs.atomicWriteTextFile(util::join(root_, s.location.metadataPath), schemas::serializeSceneMeta(meta));
        if (!mw.ok()) { return R::failure(mw.error()); }
    }
    {
        auto md = fs.atomicWriteTextFile(util::join(root_, caret->contentPath), caretText);
        if (!md.ok()) { return R::failure(md.error()); }
        auto side = restampSceneSidecar(fs, root_, caret->metadataPath, caretText, author, now);   // READ 2
        if (!side.ok()) { return R::failure(side.error()); }
    }
    for (std::size_t k = 0; k < moved.size(); ++k) {
        auto mv = fs.renamePath(util::join(root_, after[k].metadataPath), util::join(root_, moved[k].metadataPath));
        if (!mv.ok()) { return R::failure(mv.error()); }
        auto mc = fs.renamePath(util::join(root_, after[k].contentPath), util::join(root_, moved[k].contentPath));
        if (!mc.ok()) { return R::failure(mc.error()); }
    }
    // The caret chapter's own list, when its scenes changed.
    const bool createdInCaretChapter = !created.empty() && created.front().location.chapterID.value == caret->chapterID.value;
    if (createdInCaretChapter || movesAfter) {
        std::vector<std::string> names = metaFilenamesOf(before);
        for (const auto& s : created) {
            if (s.location.chapterID.value == caret->chapterID.value) { names.push_back(fileNameOf(s.location.metadataPath)); }
        }
        if (!movesAfter) {
            for (const auto& name : metaFilenamesOf(after)) { names.push_back(name); }
        }
        auto cw = writeChapterScenes(fs, root_, caret->chapterMetadataPath, names);   // READ 3
        if (!cw.ok()) { return R::failure(cw.error()); }
    }
    if (!newChapters.empty()) {
        auto mw = editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
            auto at = std::find_if(refs.begin(), refs.end(),
                [&](const schemas::ChapterRef& r) { return r.chapterID.value == caret->chapterID.value; });
            std::vector<schemas::ChapterRef> add;
            for (const auto& ch : newChapters) { add.push_back({.chapterID = ch.chapterID, .path = ch.metadataPath}); }
            refs.insert(at == refs.end() ? refs.end() : std::next(at), add.begin(), add.end());
        });
        if (!mw.ok()) { return R::failure(mw.error()); }
    }

    // ── The result ──
    RangeOperationResult out;
    std::vector<SceneLocation> createdLocations;
    for (const auto& s : created) { createdLocations.push_back(s.location); }
    out.indexCurrent = index_.insertScenesAfter(caret->sceneID, createdLocations);
    for (const auto& m : moved) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }

    const SceneID beforeCaret = caret->ordinal > 0 ? scenesBefore(caret->ordinal) : SceneID{};
    SceneLocation caretNow = *caret;
    out.delta.scenes.push_back({RangeSceneChange::Kind::Rewritten, caretNow, beforeCaret, caretText});
    SceneID previous = caret->sceneID;
    for (const auto& s : created) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Created, s.location, previous, s.text});
        previous = s.location.sceneID;
    }
    for (const auto& m : moved) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, m, previous, {}});
        previous = m.sceneID;
    }
    ChapterID previousChapter = caret->chapterID;
    for (const auto& ch : newChapters) {
        out.delta.createdChapters.push_back({ch.chapterID, ch.title, ch.metadataPath, previousChapter});
        previousChapter = ch.chapterID;
    }

    // The Range this paste inserted, under the real IDs — its undo cuts exactly this.
    RangeEntry first;
    first.sceneID      = caret->sceneID;
    first.chapterID    = caret->chapterID;
    first.chapterTitle = caret->chapterTitle;
    first.sceneTitle   = caret->title;
    first.start        = caretByte;
    first.length       = entries.front()->text.size();
    first.sceneLength  = caretText.size();
    first.text         = entries.front()->text;
    out.range.append(std::move(first));
    for (std::size_t k = 0; k < created.size(); ++k) {
        const auto& s = created[k];
        const std::string& pasted = entries[k + 1]->text;
        RangeEntry e;
        e.sceneID      = s.location.sceneID;
        e.chapterID    = s.location.chapterID;
        e.chapterTitle = s.location.chapterTitle;
        e.sceneTitle   = s.location.title;
        e.start        = 0;
        e.length       = pasted.size();
        e.sceneLength  = s.text.size();
        e.text         = pasted;
        out.range.append(std::move(e));
    }

    out.writtenTexts[caret->sceneID.value] = caretText;
    for (const auto& s : created) { out.writtenTexts[s.location.sceneID.value] = s.text; }
    return R::success(std::move(out));
}

SceneID RangeOperations::scenesBefore(std::size_t ordinal) const {
    return ordinal == 0 ? SceneID{} : index_.scenesInOrder()[ordinal - 1].sceneID;
}

// ── cut ──────────────────────────────────────────────────────────────────────────────────────────────────────────────────

Result<RangeOperationResult> RangeOperations::cut(const Range& range, const AuthorshipRef& author)
{
    using R = Result<RangeOperationResult>;
    if (range.empty()) { return R::failure(invalid("an empty range: nothing to cut")); }
    if (!authorIsValid(author)) { return R::failure(invalid("AuthorshipRef must have non-empty identityID and personaID")); }

    // P1: the Range must be what the index holds — consecutive in reading order, in the chapters it names. No I/O.
    std::vector<const RangeEntry*> entries;
    for (const auto& e : range.entries()) { entries.push_back(&e); }
    const auto start = index_.findScene(entries.front()->sceneID);
    if (!start) { return R::failure(invalid("range start scene not in manuscript: " + entries.front()->sceneID.value)); }
    const auto& order = index_.scenesInOrder();
    if (start->ordinal + entries.size() > order.size()) {
        return R::failure(invalid("the range runs past the end of the manuscript"));
    }
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        if (loc.sceneID.value != entries[i]->sceneID.value || loc.chapterID.value != entries[i]->chapterID.value) {
            return R::failure(invalid("the range is not what the manuscript holds at scene " + entries[i]->sceneID.value));
        }
    }
    const SceneLocation startLoc = order[start->ordinal];
    const SceneLocation endLoc   = order[start->ordinal + entries.size() - 1];
    const RangeEntry&   first    = *entries.front();
    const RangeEntry&   last     = *entries.back();

    auto& fs = *services_.fileSystem;
    const std::string now = services_.clock->nowUTC();

    // READS 1–2: the first and last scene, each checked against the Range (the inner scenes are deleted by ID, unread).
    auto checked = [&](const SceneLocation& loc, const RangeEntry& e) -> Result<std::string> {
        auto bodyR = fs.readTextFile(util::join(root_, loc.contentPath));
        if (!bodyR.ok()) { return bodyR; }
        const std::string& body = bodyR.value();
        if (body.size() != e.sceneLength || e.start + e.length > body.size() ||
            body.compare(e.start, e.length, e.text) != 0) {
            return Result<std::string>::failure(invalid("the range's text is not what is on disk for scene " +
                                                        loc.sceneID.value));
        }
        return bodyR;
    };
    auto firstBody = checked(startLoc, first);
    if (!firstBody.ok()) { return R::failure(firstBody.error()); }
    std::string endBody = firstBody.value();
    if (entries.size() > 1) {
        auto lastBody = checked(endLoc, last);
        if (!lastBody.ok()) { return R::failure(lastBody.error()); }
        endBody = lastBody.value();
    }
    // ✅ Joined DIRECTLY (no blank-line seam — the 2026-07-27 ruling for cuts).
    const std::string startText = firstBody.value().substr(0, first.start) + endBody.substr(last.start + last.length);

    std::vector<SceneID> removed;
    for (std::size_t i = 1; i < entries.size(); ++i) { removed.push_back(entries[i]->sceneID); }
    std::vector<ChapterInfo> removedChapters;
    for (std::size_t i = 1; i < entries.size(); ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        if (loc.chapterID.value != startLoc.chapterID.value &&
            (removedChapters.empty() || removedChapters.back().chapterID.value != loc.chapterID.value)) {
            removedChapters.push_back({loc.chapterID, loc.chapterTitle, loc.chapterMetadataPath});
        }
    }

    // Q2: the end chapter's scenes after the range join the start scene's chapter, after it.
    std::vector<SceneLocation> leaving;
    if (endLoc.chapterID.value != startLoc.chapterID.value) {
        for (std::size_t ord = start->ordinal + entries.size(); ord < order.size(); ++ord) {
            if (order[ord].chapterID.value != endLoc.chapterID.value) { break; }
            leaving.push_back(order[ord]);
        }
    }
    const std::string startDir = dirOf(startLoc.chapterMetadataPath);
    const std::string startKey = sceneKeyOf(startLoc.metadataPath);
    // ✅ Their file names already sort after the start scene's (the usual case: it is the start chapter's last scene now
    // and their keys are larger) ⇒ a plain move. Otherwise each takes a new key after the last, and its sidecar is rewritten.
    const bool keepNames = leaving.empty() || sceneKeyOf(leaving.front().metadataPath) > startKey;
    std::vector<SceneLocation> moved;
    std::string prevKey = startKey;
    for (const auto& a : leaving) {
        SceneLocation m       = a;
        m.chapterID           = startLoc.chapterID;
        m.chapterTitle        = startLoc.chapterTitle;
        m.chapterMetadataPath = startLoc.chapterMetadataPath;
        if (keepNames) {
            m.metadataPath = startDir + "/" + fileNameOf(a.metadataPath);
            m.contentPath  = startDir + "/" + fileNameOf(a.contentPath);
        } else {
            const std::string oldKey = sceneKeyOf(a.metadataPath);
            const std::string key = util::keyBetween(prevKey, "");
            if (key.empty()) { return R::failure(internal("could not compute a scene order key")); }
            const std::string metaName = fileNameOf(a.metadataPath);
            const std::string mdName   = fileNameOf(a.contentPath);
            m.metadataPath = startDir + "/" + key + metaName.substr(oldKey.size());
            m.contentPath  = startDir + "/" + (mdName.starts_with(oldKey) ? key + mdName.substr(oldKey.size())
                                                                          : key + "-" + mdName);
            m.slug         = a.slug.starts_with(oldKey) ? key + a.slug.substr(oldKey.size()) : a.slug;
            prevKey = key;
        }
        moved.push_back(std::move(m));
    }

    // ── The writes ──
    {
        auto md = fs.atomicWriteTextFile(util::join(root_, startLoc.contentPath), startText);
        if (!md.ok()) { return R::failure(md.error()); }
        auto side = restampSceneSidecar(fs, root_, startLoc.metadataPath, startText, author, now);   // READ 3
        if (!side.ok()) { return R::failure(side.error()); }
    }
    // The cut scenes go FIRST: one in the start chapter can hold the very file name a promoted scene keeps (keys are only
    // unique within a chapter). ⛔ No relationship prune (user, 2026-10-20): the cut's undo, or a paste, brings the IDs back.
    for (std::size_t i = 1; i < entries.size(); ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        auto rm = fs.removeFile(util::join(root_, loc.metadataPath));
        if (!rm.ok()) { return R::failure(rm.error()); }
        auto rc = fs.removeFile(util::join(root_, loc.contentPath));
        if (!rc.ok()) { return R::failure(rc.error()); }
    }
    for (std::size_t k = 0; k < moved.size(); ++k) {
        const SceneLocation& from = leaving[k];
        const SceneLocation& to   = moved[k];
        auto mc = fs.renamePath(util::join(root_, from.contentPath), util::join(root_, to.contentPath));
        if (!mc.ok()) { return R::failure(mc.error()); }
        if (keepNames) {
            auto mv = fs.renamePath(util::join(root_, from.metadataPath), util::join(root_, to.metadataPath));
            if (!mv.ok()) { return R::failure(mv.error()); }
            continue;
        }
        auto metaR = fs.readTextFile(util::join(root_, from.metadataPath));
        if (!metaR.ok()) { return R::failure(metaR.error()); }
        auto meta = schemas::parseSceneMeta(metaR.value());
        if (!meta.ok()) { return R::failure(meta.error()); }
        meta.value().slug        = to.slug;
        meta.value().contentPath = fileNameOf(to.contentPath);
        auto mw = fs.atomicWriteTextFile(util::join(root_, to.metadataPath), schemas::serializeSceneMeta(meta.value()));
        if (!mw.ok()) { return R::failure(mw.error()); }
        auto rm = fs.removeFile(util::join(root_, from.metadataPath));
        if (!rm.ok()) { return R::failure(rm.error()); }
    }
    for (const auto& ch : removedChapters) {
        auto rm = fs.removeFile(util::join(root_, ch.metadataPath));
        if (!rm.ok()) { return R::failure(rm.error()); }
        // Whatever else the folder held (as `ChapterDeleter` does): a folder left without its sidecar would fail the next
        // chapter listing.
        std::error_code ec;
        std::filesystem::remove_all(std::filesystem::path(util::join(root_, dirOf(ch.metadataPath))), ec);
    }
    if (entries.size() > 1) {
        std::unordered_set<std::string> gone;
        for (const auto& id : removed) { gone.insert(id.value); }
        std::vector<std::string> names;
        for (const auto& s : scenesOfChapter(index_, startLoc.chapterID)) {
            if (gone.count(s.sceneID.value) == 0) { names.push_back(fileNameOf(s.metadataPath)); }
        }
        for (const auto& m : moved) { names.push_back(fileNameOf(m.metadataPath)); }
        auto cw = writeChapterScenes(fs, root_, startLoc.chapterMetadataPath, names);   // READ 4
        if (!cw.ok()) { return R::failure(cw.error()); }
    }
    if (!removedChapters.empty()) {
        std::unordered_set<std::string> gone;
        for (const auto& ch : removedChapters) { gone.insert(ch.chapterID.value); }
        auto mw = editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
            std::erase_if(refs, [&](const schemas::ChapterRef& r) { return gone.count(r.chapterID.value) > 0; });
        });
        if (!mw.ok()) { return R::failure(mw.error()); }
    }

    // ── The result ──
    RangeOperationResult out;
    const SceneID beforeStart = scenesBefore(startLoc.ordinal);
    out.indexCurrent = index_.removeScenes(removed);
    for (const auto& m : moved) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }

    out.delta.scenes.push_back({RangeSceneChange::Kind::Rewritten, startLoc, beforeStart, startText});
    SceneID previous = startLoc.sceneID;
    for (const auto& m : moved) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, m, previous, {}});
        previous = m.sceneID;
    }
    out.delta.removedSceneIDs = removed;
    for (const auto& ch : removedChapters) { out.delta.removedChapterIDs.push_back(ch.chapterID); }
    out.range = range;
    out.writtenTexts[startLoc.sceneID.value] = startText;
    return R::success(std::move(out));
}

}   // namespace scrivi::manuscript
