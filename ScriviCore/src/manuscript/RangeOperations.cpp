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
#include <optional>
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

// Where a scene lands when it moves to another chapter: under its own file names (`newKey` empty), or renamed to `newKey`
// when its own key would collide there (keys are only unique within a chapter).
SceneLocation movedInto(const SceneLocation& a, const ChapterID& chapterID, const std::string& chapterTitle,
                        const RelativePath& chapterMetaRel, const std::string& newKey) {
    const std::string dir = dirOf(chapterMetaRel);
    SceneLocation m       = a;
    m.chapterID           = chapterID;
    m.chapterTitle        = chapterTitle;
    m.chapterMetadataPath = chapterMetaRel;
    const std::string metaName = fileNameOf(a.metadataPath);
    const std::string mdName   = fileNameOf(a.contentPath);
    if (newKey.empty()) {
        m.metadataPath = dir + "/" + metaName;
        m.contentPath  = dir + "/" + mdName;
        return m;
    }
    const std::string oldKey = sceneKeyOf(a.metadataPath);
    m.metadataPath = dir + "/" + newKey + metaName.substr(oldKey.size());
    m.contentPath  = dir + "/" + (mdName.starts_with(oldKey) ? newKey + mdName.substr(oldKey.size()) : newKey + "-" + mdName);
    m.slug         = a.slug.starts_with(oldKey) ? newKey + a.slug.substr(oldKey.size()) : a.slug;
    return m;
}

// Moves one scene's two files. ⛔ The text is never rewritten. A scene whose names change gets its sidecar rewritten (slug,
// contentPath): one read, one write.
Result<void> moveSceneFiles(FileSystem& fs, const AbsolutePath& root, const SceneLocation& from, const SceneLocation& to) {
    auto mc = fs.renamePath(util::join(root, from.contentPath), util::join(root, to.contentPath));
    if (!mc.ok()) { return mc; }
    if (fileNameOf(from.metadataPath) == fileNameOf(to.metadataPath)) {
        return fs.renamePath(util::join(root, from.metadataPath), util::join(root, to.metadataPath));
    }
    auto metaR = fs.readTextFile(util::join(root, from.metadataPath));
    if (!metaR.ok()) { return Result<void>::failure(metaR.error()); }
    auto meta = schemas::parseSceneMeta(metaR.value());
    if (!meta.ok()) { return Result<void>::failure(meta.error()); }
    meta.value().slug        = to.slug;
    meta.value().contentPath = fileNameOf(to.contentPath);
    auto mw = fs.atomicWriteTextFile(util::join(root, to.metadataPath), schemas::serializeSceneMeta(meta.value()));
    if (!mw.ok()) { return mw; }
    return fs.removeFile(util::join(root, from.metadataPath));
}

// A chapter's folder and everything in it, after its sidecar (as `ChapterDeleter` does): a folder left without its sidecar
// would fail the next chapter listing.
Result<void> removeChapterFolder(FileSystem& fs, const AbsolutePath& root, const RelativePath& chapterMetaRel) {
    auto rm = fs.removeFile(util::join(root, chapterMetaRel));
    if (!rm.ok()) { return rm; }
    std::error_code ec;
    std::filesystem::remove_all(std::filesystem::path(util::join(root, dirOf(chapterMetaRel))), ec);
    return Result<void>::success();
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

    std::vector<const RangeEntry*> entries;
    for (const auto& e : range.entries()) { entries.push_back(&e); }
    const std::size_t n = entries.size();
    // ✅ [SP-191] Q7 (user, 2026-10-21): "A range paste at scene offset 0 should paste everything before the scene, creating
    // any new scenes before it as are necessary. The last part … is merged with the beginning of the current scene." So at
    // a scene's very start the caret scene keeps its text, identity and sidecar, and takes the LAST entry; the others become
    // new scenes before it. Elsewhere it keeps the head and takes the FIRST entry; the others become new scenes after it.
    // Either way the scene whose text survives keeps its identity — and the undo, a cut, keeps it too (`cut`).
    const bool prepends = caretByte == 0 && n > 1;

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
    // The caret chapter's scenes before the paste (`leading`) and after it (`trailing`). ⚠️ When the paste prepends, the
    // caret scene itself is the first of `trailing`.
    const auto split = prepends ? caretInChapter : std::next(caretInChapter);
    const std::vector<SceneLocation> leading(caretChapterScenes.begin(), split);
    const std::vector<SceneLocation> trailing(split, caretChapterScenes.end());

    std::size_t lastOpening = 0;   // 0 = the range opens no chapter
    for (std::size_t i = 1; i < n; ++i) {
        if (entries[i]->chapterID.value != entries[i - 1]->chapterID.value) { lastOpening = i; }
    }
    // ✅ [I-0311]: the caret chapter's scenes after the paste follow it into its last chapter. They keep their file names
    // (their keys already sort after the scenes before them), so the scenes created in that chapter take keys BELOW the first.
    const bool movesAfter = lastOpening != 0 && !trailing.empty();
    const std::string movedFloor = movesAfter ? sceneKeyOf(trailing.front().metadataPath) : std::string();

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
        std::optional<schemas::ChapterMetaData> restored;   // Q3b: the removed chapter's own sidecar, brought back
    };
    struct NewScene {
        SceneLocation location;
        std::string   text;
        std::optional<schemas::SceneMetaData> restored;     // Q3b: the removed scene's own sidecar, brought back
    };
    std::vector<NewChapter> newChapters;
    std::vector<NewScene>   created;

    // The running insertion point: the chapter (and the keys) the next new scene goes between.
    ChapterID    runChapter      = caret->chapterID;
    std::string  runChapterTitle = caret->chapterTitle;
    RelativePath runChapterMeta  = caret->chapterMetadataPath;
    std::string  runChapterKey   = chapterKeyOf(caret->chapterMetadataPath);
    std::string  runDir          = dirOf(caret->chapterMetadataPath);
    bool         runIsNew        = false;
    std::string  prevSceneKey    = leading.empty() ? std::string() : sceneKeyOf(leading.back().metadataPath);
    std::string  sceneCeiling    = trailing.empty() ? std::string() : sceneKeyOf(trailing.front().metadataPath);
    std::size_t  chapterNumber   = caretChapterPos + 1;   // 1-based position of the running chapter

    // Entry `i` opens a chapter: it is created right after the running one.
    auto openChapter = [&](std::size_t i) -> Result<void> {
        const RangeEntry& e = *entries[i];
        auto key = freeChapterKey(fs, root_, runChapterKey, nextChapterKey);
        if (!key.ok()) { return Result<void>::failure(key.error()); }
        ++chapterNumber;
        NewChapter ch;
        ch.chapterID    = pickChapterID(e.chapterID);
        ch.title        = e.chapterTitle.empty() ? "Chapter " + std::to_string(chapterNumber) : e.chapterTitle;
        // ✅ Q3b: the chapter keeps its own ID only when it was removed — then its own sidecar comes back with it.
        if (ch.chapterID.value == e.chapterID.value && !e.chapterMeta.empty()) {
            auto meta = schemas::parseChapterMeta(e.chapterMeta);
            if (!meta.ok()) { return Result<void>::failure(meta.error()); }
            ch.title    = meta.value().title;
            ch.restored = std::move(meta.value());
        }
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
        return Result<void>::success();
    };

    // The entries that become NEW scenes: after the caret's head, entries 1…n-1; at a scene's start, entries 0…n-2.
    const std::size_t firstNew = prepends ? 0 : 1;
    const std::size_t endNew   = prepends ? n - 1 : n;
    for (std::size_t i = firstNew; i < endNew; ++i) {
        const RangeEntry& e = *entries[i];
        if (i > 0 && e.chapterID.value != entries[i - 1]->chapterID.value) {
            auto opened = openChapter(i);
            if (!opened.ok()) { return R::failure(opened.error()); }
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
        // ✅ Q3b: likewise a scene — its own ID only when it was removed, and then its own sidecar.
        std::optional<schemas::SceneMetaData> restored;
        if (loc.sceneID.value == e.sceneID.value && !e.sceneMeta.empty()) {
            auto meta = schemas::parseSceneMeta(e.sceneMeta);
            if (!meta.ok()) { return R::failure(meta.error()); }
            loc.title  = meta.value().title;
            loc.status = meta.value().status;
            restored   = std::move(meta.value());
        }
        loc.metadataPath        = runDir + "/" + slug + ".meta.json";
        loc.contentPath         = runDir + "/" + slug + ".md";
        loc.chapterMetadataPath = runChapterMeta;
        if (runIsNew) { newChapters.back().sceneFiles.push_back(slug + ".meta.json"); }
        created.push_back({std::move(loc), e.text, std::move(restored)});
        prevSceneKey = key.value();
    }
    // At a scene's start the caret scene takes the last entry — and opens the last chapter when that entry does.
    if (prepends && entries[n - 1]->chapterID.value != entries[n - 2]->chapterID.value) {
        auto opened = openChapter(n - 1);
        if (!opened.ok()) { return R::failure(opened.error()); }
    }

    // The flat-document rule (v0.1 §4.2, kept): head + the first entry; the tail follows the whole pasted run. At a scene's
    // start the head is empty, so the caret scene is the last entry + its whole text.
    std::string caretText;
    if (prepends) {
        caretText = entries.back()->text + body;
    } else {
        caretText = body.substr(0, caretByte) + entries.front()->text;
        if (created.empty()) { caretText += body.substr(caretByte); } else { created.back().text += body.substr(caretByte); }
    }

    std::vector<SceneLocation> moved;   // ⚠️ at a scene's start, `moved[0]` is the caret scene itself
    if (movesAfter) {
        NewChapter& last = newChapters.back();
        for (const auto& a : trailing) {
            last.sceneFiles.push_back(fileNameOf(a.metadataPath));
            moved.push_back(movedInto(a, last.chapterID, last.title, last.metadataPath, ""));
        }
    }
    const SceneLocation caretNow = prepends && movesAfter ? moved.front() : *caret;

    // ── The writes: new files first, so a failure part-way never leaves the caret scene describing what is not there ──
    for (const auto& ch : newChapters) {
        auto dir = fs.createDirectories(util::join(root_, ch.dir));
        if (!dir.ok()) { return R::failure(dir.error()); }
        schemas::ChapterMetaData meta;
        if (ch.restored) {
            meta = *ch.restored;
        } else {
            meta.chapterID            = ch.chapterID;
            meta.title                = ch.title;
            meta.displayLabel         = ch.title;
            meta.status               = "draft";
            meta.createdAt            = now;
            meta.createdByIdentityID  = author.identityID.value;
            meta.createdByPersonaID   = author.personaID.value;
            meta.createdByDisplayName = author.displayName;
        }
        meta.slug = fileNameOf(ch.dir);
        meta.scenes.clear();
        for (const auto& name : ch.sceneFiles) { meta.scenes.push_back({.metadataFilename = name}); }
        auto w = fs.atomicWriteTextFile(util::join(root_, ch.metadataPath), schemas::serializeChapterMeta(meta));
        if (!w.ok()) { return R::failure(w.error()); }
    }
    for (const auto& s : created) {
        auto md = fs.atomicWriteTextFile(util::join(root_, s.location.contentPath), s.text);
        if (!md.ok()) { return R::failure(md.error()); }
        schemas::SceneMetaData meta;
        if (s.restored) {
            // Where it lives now, and what its text now counts; everything else exactly as it was.
            meta             = *s.restored;
            meta.slug        = s.location.slug;
            meta.contentPath = fileNameOf(s.location.contentPath);
            const auto stats = util::countText(s.text);
            meta.wordCount      = stats.wordCount;
            meta.characterCount = stats.characterCount;
        } else {
            meta = newSceneMeta(s.location.sceneID, s.location.title, s.location.slug,
                                fileNameOf(s.location.contentPath), s.text, author, now);
        }
        auto mw = fs.atomicWriteTextFile(util::join(root_, s.location.metadataPath), schemas::serializeSceneMeta(meta));
        if (!mw.ok()) { return R::failure(mw.error()); }
    }
    for (std::size_t k = 0; k < moved.size(); ++k) {
        auto mv = moveSceneFiles(fs, root_, trailing[k], moved[k]);
        if (!mv.ok()) { return R::failure(mv.error()); }
    }
    {
        auto md = fs.atomicWriteTextFile(util::join(root_, caretNow.contentPath), caretText);
        if (!md.ok()) { return R::failure(md.error()); }
        auto side = restampSceneSidecar(fs, root_, caretNow.metadataPath, caretText, author, now);   // READ 2
        if (!side.ok()) { return R::failure(side.error()); }
    }
    // The caret chapter's own list, when its scenes changed.
    const bool createdInCaretChapter = !created.empty() && created.front().location.chapterID.value == caret->chapterID.value;
    if (createdInCaretChapter || movesAfter) {
        std::vector<std::string> names = metaFilenamesOf(leading);
        for (const auto& s : created) {
            if (s.location.chapterID.value == caret->chapterID.value) { names.push_back(fileNameOf(s.location.metadataPath)); }
        }
        if (!movesAfter) {
            for (const auto& name : metaFilenamesOf(trailing)) { names.push_back(name); }
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
    const SceneID beforeCaret = scenesBefore(caret->ordinal);
    if (!prepends) {
        out.indexCurrent = index_.insertScenesAfter(caret->sceneID, createdLocations);
    } else if (beforeCaret.value.empty()) {
        out.indexCurrent = index_.insertScenesBefore(caret->sceneID, createdLocations);
    } else {
        out.indexCurrent = index_.insertScenesAfter(beforeCaret, createdLocations);
    }
    for (const auto& s : created) {   // a restored sidecar brings its story time back: the index must say so too
        if (s.restored) { out.indexCurrent = out.indexCurrent && index_.patchStoryTime(s.location.sceneID, s.restored->storyTime); }
    }
    for (const auto& m : moved) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }

    // The delta, in reading order.
    SceneID previous = beforeCaret;
    if (!prepends) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Rewritten, caretNow, previous, caretText});
        previous = caret->sceneID;
    }
    for (const auto& s : created) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Created, s.location, previous, s.text});
        previous = s.location.sceneID;
    }
    if (prepends) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Rewritten, caretNow, previous, caretText});
        previous = caret->sceneID;
    }
    for (const auto& m : moved) {
        if (m.sceneID.value == caret->sceneID.value) { continue; }   // reported above, with its text
        out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, m, previous, {}});
        previous = m.sceneID;
    }
    ChapterID previousChapter = caret->chapterID;
    for (const auto& ch : newChapters) {
        out.delta.createdChapters.push_back({ch.chapterID, ch.title, ch.metadataPath, previousChapter});
        previousChapter = ch.chapterID;
    }

    // The Range this paste inserted, under the real IDs — its undo cuts exactly this.
    auto inserted = [](const SceneLocation& s, std::size_t start, const std::string& pasted, std::size_t whole) {
        RangeEntry e;
        e.sceneID      = s.sceneID;
        e.chapterID    = s.chapterID;
        e.chapterTitle = s.chapterTitle;
        e.sceneTitle   = s.title;
        e.start        = start;
        e.length       = pasted.size();
        e.sceneLength  = whole;
        e.text         = pasted;
        return e;
    };
    if (!prepends) { out.range.append(inserted(caretNow, caretByte, entries.front()->text, caretText.size())); }
    for (std::size_t k = 0; k < created.size(); ++k) {
        out.range.append(inserted(created[k].location, 0, entries[k + firstNew]->text, created[k].text.size()));
    }
    if (prepends) { out.range.append(inserted(caretNow, 0, entries.back()->text, caretText.size())); }

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
    const std::size_t n = entries.size();
    const auto start = index_.findScene(entries.front()->sceneID);
    if (!start) { return R::failure(invalid("range start scene not in manuscript: " + entries.front()->sceneID.value)); }
    const auto& order = index_.scenesInOrder();
    if (start->ordinal + n > order.size()) {
        return R::failure(invalid("the range runs past the end of the manuscript"));
    }
    for (std::size_t i = 0; i < n; ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        if (loc.sceneID.value != entries[i]->sceneID.value || loc.chapterID.value != entries[i]->chapterID.value) {
            return R::failure(invalid("the range is not what the manuscript holds at scene " + entries[i]->sceneID.value));
        }
    }
    const SceneLocation startLoc = order[start->ordinal];
    const SceneLocation endLoc   = order[start->ordinal + n - 1];
    const RangeEntry&   first    = *entries.front();
    const RangeEntry&   last     = *entries.back();
    // ✅ [SP-191] Q7: the scene whose text survives keeps its identity. A cut that begins at the start scene's very first
    // character leaves nothing of it, so the END scene survives (with its tail); otherwise the START scene does (with its
    // head and the end scene's tail). This is also exactly the inverse of a paste at a scene's start (`paste`).
    const bool endSurvives = n > 1 && first.start == 0;

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
    if (n > 1) {
        auto lastBody = checked(endLoc, last);
        if (!lastBody.ok()) { return R::failure(lastBody.error()); }
        endBody = lastBody.value();
    }
    // ✅ Joined DIRECTLY (no blank-line seam — the 2026-07-27 ruling for cuts; [SP-191] Q3a: a scene merge too).
    const std::string survivorText = firstBody.value().substr(0, first.start) + endBody.substr(last.start + last.length);

    // The scenes the cut removes, by their place in the Range: all but the survivor.
    const std::size_t removedFrom = endSurvives ? 0 : 1;
    const std::size_t removedTo   = endSurvives ? n - 1 : n;
    std::vector<SceneID> removed;
    for (std::size_t i = removedFrom; i < removedTo; ++i) { removed.push_back(entries[i]->sceneID); }
    std::vector<ChapterInfo> removedChapters;
    for (std::size_t i = 1; i < n; ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        if (loc.chapterID.value != startLoc.chapterID.value &&
            (removedChapters.empty() || removedChapters.back().chapterID.value != loc.chapterID.value)) {
            removedChapters.push_back({loc.chapterID, loc.chapterTitle, loc.chapterMetadataPath});
        }
    }

    // Q2: the end chapter's scenes after the range — and the end scene itself when it survives — join the start scene's
    // chapter, after the last of its scenes that stay.
    std::vector<SceneLocation> leaving;
    if (endLoc.chapterID.value != startLoc.chapterID.value) {
        for (std::size_t ord = start->ordinal + n - (endSurvives ? 1 : 0); ord < order.size(); ++ord) {
            if (order[ord].chapterID.value != endLoc.chapterID.value) { break; }
            leaving.push_back(order[ord]);
        }
    }
    std::string anchorKey;   // the last scene of the start chapter that stays, before the ones that join it
    if (!endSurvives) {
        anchorKey = sceneKeyOf(startLoc.metadataPath);
    } else if (start->ordinal > 0 && order[start->ordinal - 1].chapterID.value == startLoc.chapterID.value) {
        anchorKey = sceneKeyOf(order[start->ordinal - 1].metadataPath);
    }
    // ✅ Their file names already sort after the anchor's (the usual case: their keys are larger) ⇒ a plain move. Otherwise
    // each takes a new key after the last, and its sidecar is rewritten.
    const bool keepNames = leaving.empty() || sceneKeyOf(leaving.front().metadataPath) > anchorKey;
    std::vector<SceneLocation> moved;   // ⚠️ when the end scene survives across chapters, `moved[0]` is that scene
    std::string prevKey = anchorKey;
    for (const auto& a : leaving) {
        std::string key;
        if (!keepNames) {
            key = util::keyBetween(prevKey, "");
            if (key.empty()) { return R::failure(internal("could not compute a scene order key")); }
            prevKey = key;
        }
        moved.push_back(movedInto(a, startLoc.chapterID, startLoc.chapterTitle, startLoc.chapterMetadataPath, key));
    }
    const SceneLocation survivor = !endSurvives ? startLoc : !moved.empty() ? moved.front() : endLoc;

    // ✅ [SP-191] Q3b: what an undo needs to bring back what this cut removes — each removed scene's sidecar, and on the
    // entry that opens a removed chapter, that chapter's sidecar. Read before any write, like every other check.
    Range removedRange = range;
    {
        auto it = removedRange.entries().begin();
        for (std::size_t i = 0; i < n; ++i, ++it) {
            const SceneLocation& loc = order[start->ordinal + i];
            if (i >= removedFrom && i < removedTo) {
                auto side = fs.readTextFile(util::join(root_, loc.metadataPath));
                if (!side.ok()) { return R::failure(side.error()); }
                it->sceneMeta = side.value();
            }
            if (i > 0 && loc.chapterID.value != order[start->ordinal + i - 1].chapterID.value) {
                auto chapter = fs.readTextFile(util::join(root_, loc.chapterMetadataPath));
                if (!chapter.ok()) { return R::failure(chapter.error()); }
                it->chapterMeta = chapter.value();
            }
        }
    }

    // ── The writes ──
    // The cut scenes go FIRST: one in the start chapter can hold the very file name a promoted scene keeps (keys are only
    // unique within a chapter). ⛔ No relationship prune (user, 2026-10-20): the cut's undo, or a paste, brings the IDs back.
    for (std::size_t i = removedFrom; i < removedTo; ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        auto rm = fs.removeFile(util::join(root_, loc.metadataPath));
        if (!rm.ok()) { return R::failure(rm.error()); }
        auto rc = fs.removeFile(util::join(root_, loc.contentPath));
        if (!rc.ok()) { return R::failure(rc.error()); }
    }
    for (std::size_t k = 0; k < moved.size(); ++k) {
        auto mv = moveSceneFiles(fs, root_, leaving[k], moved[k]);
        if (!mv.ok()) { return R::failure(mv.error()); }
    }
    {
        auto md = fs.atomicWriteTextFile(util::join(root_, survivor.contentPath), survivorText);
        if (!md.ok()) { return R::failure(md.error()); }
        auto side = restampSceneSidecar(fs, root_, survivor.metadataPath, survivorText, author, now);   // READ 3
        if (!side.ok()) { return R::failure(side.error()); }
    }
    for (const auto& ch : removedChapters) {
        auto rm = removeChapterFolder(fs, root_, ch.metadataPath);
        if (!rm.ok()) { return R::failure(rm.error()); }
    }
    if (n > 1) {
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

    out.delta.scenes.push_back({RangeSceneChange::Kind::Rewritten, survivor, beforeStart, survivorText});
    SceneID previous = survivor.sceneID;
    for (const auto& m : moved) {
        if (m.sceneID.value == survivor.sceneID.value) { continue; }   // reported above, with its text
        out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, m, previous, {}});
        previous = m.sceneID;
    }
    out.delta.removedSceneIDs = removed;
    for (const auto& ch : removedChapters) { out.delta.removedChapterIDs.push_back(ch.chapterID); }
    out.range = std::move(removedRange);
    // Its undo: a paste of `range` at the survivor's seam — at the start scene's cut point, or at the very start of the end
    // scene when that one survived (a paste there goes before it).
    out.undoCaretSceneID = survivor.sceneID;
    out.undoCaretByte    = first.start;   // 0 when the end scene survived (that is when it does)
    out.writtenTexts[survivor.sceneID.value] = survivorText;
    return R::success(std::move(out));
}

// ── relabel ──────────────────────────────────────────────────────────────────────────────────────────────────────────────

Result<RangeOperationResult> RangeOperations::relabel(const Range& range, const ChapterID& target,
                                                      const AuthorshipRef& author)
{
    using R = Result<RangeOperationResult>;
    if (range.empty()) { return R::failure(invalid("an empty range: nothing to relabel")); }
    if (!authorIsValid(author)) { return R::failure(invalid("AuthorshipRef must have non-empty identityID and personaID")); }

    // The run must be what the index holds: consecutive, whole scenes, all in one chapter. No I/O.
    std::vector<const RangeEntry*> entries;
    for (const auto& e : range.entries()) { entries.push_back(&e); }
    const auto start = index_.findScene(entries.front()->sceneID);
    if (!start) { return R::failure(invalid("range start scene not in manuscript: " + entries.front()->sceneID.value)); }
    const auto& order = index_.scenesInOrder();
    if (start->ordinal + entries.size() > order.size()) {
        return R::failure(invalid("the range runs past the end of the manuscript"));
    }
    std::vector<SceneLocation> run;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        if (loc.sceneID.value != entries[i]->sceneID.value || loc.chapterID.value != start->chapterID.value) {
            return R::failure(invalid("the range is not one chapter's consecutive scenes at scene " +
                                      entries[i]->sceneID.value));
        }
        run.push_back(loc);
    }
    const ChapterID source = start->chapterID;
    if (target.value == source.value) { return R::failure(invalid("the scenes are already in that chapter")); }

    const auto chapters = chaptersInOrder(index_);
    const auto sourceIt = std::find_if(chapters.begin(), chapters.end(),
        [&](const ChapterInfo& c) { return c.chapterID.value == source.value; });
    const std::size_t pos = static_cast<std::size_t>(sourceIt - chapters.begin());
    const auto sourceScenes = scenesOfChapter(index_, source);
    const bool opensChapter  = sourceScenes.front().sceneID.value == run.front().sceneID.value;
    const bool closesChapter = sourceScenes.back().sceneID.value == run.back().sceneID.value;
    const bool intoPrevious  = pos > 0 && chapters[pos - 1].chapterID.value == target.value;
    const bool intoNext      = pos + 1 < chapters.size() && chapters[pos + 1].chapterID.value == target.value;
    const bool intoNew       = !intoPrevious && !intoNext &&
                               std::none_of(chapters.begin(), chapters.end(),
                                   [&](const ChapterInfo& c) { return c.chapterID.value == target.value; });
    if (intoPrevious && !opensChapter) {
        return R::failure(invalid("only the scenes that open a chapter can move into the chapter before it"));
    }
    if ((intoNext || intoNew) && !closesChapter) {
        return R::failure(invalid("only the scenes that close a chapter can move into the chapter after it"));
    }
    if (!intoPrevious && !intoNext && !intoNew) {
        return R::failure(invalid("a relabel moves scenes into the chapter before or after theirs, or into a new one"));
    }
    const bool emptiesSource = opensChapter && closesChapter;

    auto& fs = *services_.fileSystem;
    const std::string now = services_.clock->nowUTC();

    // ── The destination, and where each scene lands in it ──
    ChapterInfo dest;
    std::vector<SceneLocation> destScenes;   // what the destination already holds
    std::optional<schemas::ChapterMetaData> newChapter;
    std::string newChapterDir;
    std::vector<SceneLocation> moved;
    if (intoNew) {
        const std::string nextKey = pos + 1 < chapters.size() ? chapterKeyOf(chapters[pos + 1].metadataPath) : std::string();
        auto key = freeChapterKey(fs, root_, chapterKeyOf(sourceIt->metadataPath), nextKey);
        if (!key.ok()) { return R::failure(key.error()); }
        newChapterDir = "manuscript/chapter-" + key.value();
        schemas::ChapterMetaData meta;
        const RangeEntry& first = *entries.front();
        if (!first.chapterMeta.empty()) {   // ✅ Q3b: an undo brings the chapter back as it was
            auto parsed = schemas::parseChapterMeta(first.chapterMeta);
            if (!parsed.ok()) { return R::failure(parsed.error()); }
            meta = std::move(parsed.value());
        } else {
            meta.title                = first.chapterTitle.empty() ? "Chapter " + std::to_string(pos + 2) : first.chapterTitle;
            meta.displayLabel         = meta.title;
            meta.status               = "draft";
            meta.createdAt            = now;
            meta.createdByIdentityID  = author.identityID.value;
            meta.createdByPersonaID   = author.personaID.value;
            meta.createdByDisplayName = author.displayName;
        }
        meta.chapterID = target.value.empty() || target.value.starts_with("~") ? services_.uuidProvider->newChapterID()
                                                                               : target;
        meta.slug = fileNameOf(newChapterDir);
        dest = {meta.chapterID, meta.title, newChapterDir + "/chapter.meta.json"};
        newChapter = std::move(meta);
        // An empty folder: every scene keeps its file names.
        for (const auto& a : run) { moved.push_back(movedInto(a, dest.chapterID, dest.title, dest.metadataPath, "")); }
    } else {
        dest       = chapters[intoPrevious ? pos - 1 : pos + 1];
        destScenes = scenesOfChapter(index_, dest.chapterID);
        // ✅ The run keeps its file names when they already sort on the right side of the destination's; else each takes a
        // new key there (the usual case: every chapter's keys start from the same first key).
        const std::string lo = intoPrevious ? sceneKeyOf(destScenes.back().metadataPath) : std::string();
        const std::string hi = intoNext ? sceneKeyOf(destScenes.front().metadataPath) : std::string();
        const bool keepNames = intoPrevious ? sceneKeyOf(run.front().metadataPath) > lo
                                            : sceneKeyOf(run.back().metadataPath) < hi;
        std::string prevKey = lo;
        for (const auto& a : run) {
            std::string key;
            if (!keepNames) {
                key = util::keyBetween(prevKey, hi);
                if (key.empty()) { return R::failure(internal("could not compute a scene order key")); }
                prevKey = key;
            }
            moved.push_back(movedInto(a, dest.chapterID, dest.title, dest.metadataPath, key));
        }
    }

    // ✅ Q3b: the source chapter's own sidecar, when the relabel empties it — read before any write.
    std::string sourceMeta;
    if (emptiesSource) {
        auto chapter = fs.readTextFile(util::join(root_, sourceIt->metadataPath));
        if (!chapter.ok()) { return R::failure(chapter.error()); }
        sourceMeta = chapter.value();
    }

    // ── The writes: the destination first, then the moves, and the emptied chapter LAST ([I-0083]) ──
    if (newChapter) {
        auto dir = fs.createDirectories(util::join(root_, newChapterDir));
        if (!dir.ok()) { return R::failure(dir.error()); }
        newChapter->scenes.clear();
        for (const auto& m : moved) { newChapter->scenes.push_back({.metadataFilename = fileNameOf(m.metadataPath)}); }
        auto w = fs.atomicWriteTextFile(util::join(root_, dest.metadataPath), schemas::serializeChapterMeta(*newChapter));
        if (!w.ok()) { return R::failure(w.error()); }
    }
    for (std::size_t k = 0; k < run.size(); ++k) {
        auto mv = moveSceneFiles(fs, root_, run[k], moved[k]);
        if (!mv.ok()) { return R::failure(mv.error()); }
    }
    if (!newChapter) {
        std::vector<std::string> names = intoPrevious ? metaFilenamesOf(destScenes) : std::vector<std::string>{};
        for (const auto& m : moved) { names.push_back(fileNameOf(m.metadataPath)); }
        if (intoNext) {
            for (const auto& name : metaFilenamesOf(destScenes)) { names.push_back(name); }
        }
        auto cw = writeChapterScenes(fs, root_, dest.metadataPath, names);
        if (!cw.ok()) { return R::failure(cw.error()); }
    }
    if (emptiesSource) {
        auto rm = removeChapterFolder(fs, root_, sourceIt->metadataPath);
        if (!rm.ok()) { return R::failure(rm.error()); }
    } else {
        std::vector<SceneLocation> rest;
        for (const auto& s : sourceScenes) {
            if (std::none_of(run.begin(), run.end(), [&](const SceneLocation& r) { return r.sceneID.value == s.sceneID.value; })) {
                rest.push_back(s);
            }
        }
        auto cw = writeChapterScenes(fs, root_, sourceIt->metadataPath, metaFilenamesOf(rest));
        if (!cw.ok()) { return R::failure(cw.error()); }
    }
    if (newChapter || emptiesSource) {
        auto mw = editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
            if (newChapter) {
                auto at = std::find_if(refs.begin(), refs.end(),
                    [&](const schemas::ChapterRef& r) { return r.chapterID.value == source.value; });
                refs.insert(at == refs.end() ? refs.end() : std::next(at),
                            schemas::ChapterRef{.chapterID = dest.chapterID, .path = dest.metadataPath});
            }
            if (emptiesSource) {
                std::erase_if(refs, [&](const schemas::ChapterRef& r) { return r.chapterID.value == source.value; });
            }
        });
        if (!mw.ok()) { return R::failure(mw.error()); }
    }

    // ── The result ── (the reading order is unchanged: only chapters and paths move)
    RangeOperationResult out;
    out.indexCurrent = true;
    for (const auto& m : moved) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }
    SceneID previous = scenesBefore(start->ordinal);
    for (const auto& m : moved) {
        out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, m, previous, {}});
        previous = m.sceneID;
    }
    if (newChapter) { out.delta.createdChapters.push_back({dest.chapterID, dest.title, dest.metadataPath, source}); }
    if (emptiesSource) { out.delta.removedChapterIDs.push_back(source); }

    // The run as it is now; its inverse relabels it back into `source`, re-creating that chapter from `chapterMeta`.
    for (std::size_t k = 0; k < moved.size(); ++k) {
        RangeEntry e;
        e.sceneID      = moved[k].sceneID;
        e.chapterID    = dest.chapterID;
        e.chapterTitle = sourceIt->title;   // what the inverse names a chapter it has to re-create
        e.sceneTitle   = moved[k].title;
        if (k == 0) { e.chapterMeta = sourceMeta; }
        out.range.append(std::move(e));
    }
    out.fromChapterID = source;
    return R::success(std::move(out));
}

}   // namespace scrivi::manuscript
