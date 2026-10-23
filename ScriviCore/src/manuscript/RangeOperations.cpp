#include "manuscript/RangeOperations.hpp"

#include "domain/TextStats.hpp"
#include "markdown/Editing.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <atomic>
#include <exception>
#include <filesystem>
#include <optional>
#include <thread>
#include <unordered_set>

namespace scrivi::manuscript {

namespace {

Error invalid(std::string message) { return Error{.code = ErrorCode::invalidArgument, .message = std::move(message)}; }
Error internal(std::string message) { return Error{.code = ErrorCode::internalError, .message = std::move(message)}; }

// ✅ [SP-191] (user, 2026-10-22): an operation's independent file calls are made CONCURRENTLY. Over a network share each
// call costs a round trip, and a chapter's scenes moved one after another, so a chapter split or merge took calls ×
// latency (2.4–3.3 s on the rig). A batch holds calls that touch DIFFERENT files and need nothing else in it; the batches
// still run in the operation's order (new files first, a chapter emptied last), so a failure part-way leaves the same
// kinds of state as before. ⛔ After the first failure no further call in the batch starts. ⚠️ The FileSystem must be safe
// to call from several threads (the ABI's LocalFileSystem is: `scrivi_read_scene_texts` reads with 8 at once).
constexpr std::size_t kConcurrentCalls = 8;

using FileJob = std::function<Result<void>()>;

Result<void> runConcurrently(const std::vector<FileJob>& jobs) {
    std::vector<std::optional<Error>> failed(jobs.size());
    std::atomic<std::size_t> next{0};
    std::atomic<bool> stop{false};
    auto work = [&]() {
        for (std::size_t i = next++; i < jobs.size() && !stop; i = next++) {
            try {
                if (auto r = jobs[i](); !r.ok()) { failed[i] = r.error(); stop = true; }
            } catch (const std::exception& e) {   // a jthread must not let one escape
                failed[i] = internal(e.what());
                stop = true;
            }
        }
    };
    {
        std::vector<std::jthread> pool;
        const std::size_t n = std::min(kConcurrentCalls, jobs.size());
        for (std::size_t t = 1; t < n; ++t) { pool.emplace_back(work); }
        work();   // this thread works too
    }             // jthreads join here
    for (const auto& f : failed) {
        if (f) { return Result<void>::failure(*f); }
    }
    return Result<void>::success();
}

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

// Moves one scene's two files, as two jobs (they touch different files). ⛔ The text is never rewritten. A scene whose names
// change gets its sidecar rewritten (slug, contentPath): one read, one write.
void addMoveJobs(std::vector<FileJob>& jobs, FileSystem& fs, const AbsolutePath& root, const SceneLocation& from,
                 const SceneLocation& to) {
    jobs.push_back([&fs, &root, &from, &to] {
        return fs.renamePath(util::join(root, from.contentPath), util::join(root, to.contentPath));
    });
    jobs.push_back([&fs, &root, &from, &to]() -> Result<void> {
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
    });
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

// `n` ascending keys strictly between `lo` and `hi` ("" = open), by halving, so they stay short. Fewer than `n` when the
// bounds leave no room.
std::vector<std::string> keysBetween(const std::string& lo, const std::string& hi, std::size_t n) {
    if (n == 0) { return {}; }
    const std::string mid = util::keyBetween(lo, hi);
    if (mid.empty()) { return {}; }
    std::vector<std::string> out = keysBetween(lo, mid, (n - 1) / 2);
    out.push_back(mid);
    for (auto& k : keysBetween(mid, hi, n - 1 - (n - 1) / 2)) { out.push_back(std::move(k)); }
    return out;
}

Result<schemas::ChapterMetaData> readChapterMeta(FileSystem& fs, const AbsolutePath& root, const RelativePath& metaRel) {
    auto textR = fs.readTextFile(util::join(root, metaRel));
    if (!textR.ok()) { return Result<schemas::ChapterMetaData>::failure(textR.error()); }
    return schemas::parseChapterMeta(textR.value());
}

// A chapter's sidecar written into `dir` (replacing whatever sidecar is there), listing `sceneFiles`.
Result<void> writeChapterMetaInto(FileSystem& fs, const AbsolutePath& root, const std::string& dir,
                                  schemas::ChapterMetaData meta, const std::vector<std::string>& sceneFiles) {
    meta.slug = fileNameOf(dir);
    meta.scenes.clear();
    for (const auto& name : sceneFiles) { meta.scenes.push_back({.metadataFilename = name}); }
    return fs.atomicWriteTextFile(util::join(root, dir + "/chapter.meta.json"), schemas::serializeChapterMeta(meta));
}

// The run, where its files already lie, in another chapter.
std::vector<SceneLocation> relabelledInPlace(const std::vector<SceneLocation>& run, const ChapterID& chapterID,
                                             const std::string& chapterTitle, const RelativePath& chapterMetaRel) {
    std::vector<SceneLocation> out = run;
    for (auto& s : out) {
        s.chapterID           = chapterID;
        s.chapterTitle        = chapterTitle;
        s.chapterMetadataPath = chapterMetaRel;
    }
    return out;
}

// ✅ [SP-188] Q3 (user, 2026-10-22): the cut's join, BALANCED. The plain join is `head + tail`; a cut that began inside an
// emphasis span (or ended inside one) leaves it open. The removal is re-done as the editor's balanced edit of ONE text —
// head + the removed edge pieces + tail, the pieces kept apart by a blank line as the scene break keeps them apart — so a
// cut across scenes balances exactly as a cut across paragraphs does. `survivor` becomes the balanced text; the result says
// how to turn it back into the plain join (`RangeJoin`), or nothing when the plain join is already balanced.
std::optional<RangeJoin> balanceJoin(const std::string& head, const std::string& firstText, const std::string* lastText,
                                     const std::string& tail, std::string& survivor) {
    namespace md = scrivi::markdown;
    const md::Units h = md::toUnits(head);
    md::Units removed = md::toUnits(firstText);
    if (lastText != nullptr) { removed += u"\n\n"; removed += md::toUnits(*lastText); }
    const md::Units t = md::toUnits(tail);
    const int r0 = static_cast<int>(h.size()), r1 = r0 + static_cast<int>(removed.size());
    const md::editing::Scene whole(h + removed + t);
    const auto e = md::editing::balancedEdit(whole, {r0, r1 - r0}, md::UnitsView());
    if (!e || e->range.location > r0 || e->range.end() < r1) { return std::nullopt; }
    const md::Units& text = whole.text();
    const auto a = static_cast<std::size_t>(e->range.location), b = static_cast<std::size_t>(e->range.end());
    const md::Units plain = text.substr(a, r0 - a) + text.substr(r1, b - r1);
    if (plain == e->replacement) { return std::nullopt; }
    RangeJoin join{.byte = md::toUtf8(text.substr(0, a)).size(), .balanced = md::toUtf8(e->replacement),
                   .plain = md::toUtf8(plain)};
    survivor = md::toUtf8(text.substr(0, a)) + join.balanced + md::toUtf8(text.substr(b));
    return join;
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
    std::string body = bodyR.value();
    if (body != caretSceneText) {
        return R::failure(invalid("the caret scene's text is not what is on disk (the editor is stale): " +
                                  caretScene.value));
    }
    // ✅ [SP-188] Q3: the undo of a cut whose join was balanced — the plain join first, then the paste (`caretByte` is in
    // the plain text). ⛔ Refused anywhere but at that survivor, as the cut left it: a join is an undo record.
    if (const auto& join = range.join()) {
        if (join->sceneID.value != caretScene.value || join->byte + join->balanced.size() > body.size() ||
            body.compare(join->byte, join->balanced.size(), join->balanced) != 0) {
            return R::failure(invalid("the range's join is not what is on disk at scene " + caretScene.value));
        }
        body.replace(join->byte, join->balanced.size(), join->plain);
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
    // ✅ [SP-191] (user, 2026-10-22): MOVE THE SMALLER SIDE (as `relabel` does). When the paste opens ONE chapter, at its
    // first new scene (a chapter split's shape: nothing new lands in the caret chapter), and the caret chapter's scenes up
    // to the caret are fewer than those after it, THEY move — into a new folder before the caret chapter's, with its
    // sidecar — and the new chapter takes the caret chapter's folder, where the scenes after the caret already are. Same
    // chapters, IDs and order; fewer renames, which do not overlap over a share.
    std::size_t openings = 0;
    for (std::size_t i = 1; i < n; ++i) {
        if (entries[i]->chapterID.value != entries[i - 1]->chapterID.value) { ++openings; }
    }
    const bool leadingMoves = movesAfter && openings == 1 && lastOpening == 1 && !prepends &&
                              leading.size() < trailing.size();
    std::string leadingDir;   // where the caret chapter goes when its leading scenes move
    std::optional<schemas::ChapterMetaData> caretChapterMeta;
    if (leadingMoves) {
        const std::string prevKey = caretChapterPos > 0 ? chapterKeyOf(chapters[caretChapterPos - 1].metadataPath)
                                                        : std::string();
        auto key = freeChapterKey(fs, root_, prevKey, chapterKeyOf(caret->chapterMetadataPath));
        if (!key.ok()) { return R::failure(key.error()); }
        leadingDir = "manuscript/chapter-" + key.value();
        auto meta = readChapterMeta(fs, root_, caret->chapterMetadataPath);   // read before any write: it follows its scenes
        if (!meta.ok()) { return R::failure(meta.error()); }
        caretChapterMeta = std::move(meta.value());
    }

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

    // Entry `i` opens a chapter: it is created right after the running one.
    auto openChapter = [&](std::size_t i) -> Result<void> {
        const RangeEntry& e = *entries[i];
        // When the leading scenes move, the new chapter takes the caret chapter's folder (its key is the caret chapter's).
        auto key = leadingMoves ? Result<std::string>::success(chapterKeyOf(caret->chapterMetadataPath))
                                : freeChapterKey(fs, root_, runChapterKey, nextChapterKey);
        if (!key.ok()) { return Result<void>::failure(key.error()); }
        NewChapter ch;
        ch.chapterID    = pickChapterID(e.chapterID);
        ch.title        = e.chapterTitle;   // [SP-191] Q10: "" = untitled ("Chapter N" is derived, never stored)
        // ✅ Q3b: the chapter keeps its own ID only when it was removed — then its own sidecar comes back with it.
        if (ch.chapterID.value == e.chapterID.value && !e.chapterMeta.empty()) {
            auto meta = schemas::parseChapterMeta(e.chapterMeta);
            if (!meta.ok()) { return Result<void>::failure(meta.error()); }
            ch.title    = meta.value().title;
            ch.restored = std::move(meta.value());
        }
        ch.dir          = leadingMoves ? dirOf(caret->chapterMetadataPath) : "manuscript/chapter-" + key.value();
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
            moved.push_back(movedInto(a, last.chapterID, last.title, last.metadataPath, ""));   // in place when `leadingMoves`
        }
    }
    std::vector<SceneLocation> leadingNow;   // `leadingMoves`: the caret chapter's scenes in their new folder
    if (leadingMoves) {
        for (const auto& a : leading) {
            leadingNow.push_back(movedInto(a, caret->chapterID, caret->chapterTitle, leadingDir + "/chapter.meta.json", ""));
        }
    }
    const SceneLocation caretNow = leadingMoves ? leadingNow.back() : prepends && movesAfter ? moved.front() : *caret;

    // ── The writes: new files first, so a failure part-way never leaves the caret scene describing what is not there ──
    // Batch 1: each new chapter's folder, then its sidecar.
    std::vector<FileJob> jobs;
    for (const auto& ch : newChapters) {
        if (leadingMoves) { break; }   // its folder is the caret chapter's: its sidecar is written last (batch 3)
        jobs.push_back([&]() -> Result<void> {
            auto dir = fs.createDirectories(util::join(root_, ch.dir));
            if (!dir.ok()) { return dir; }
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
            return fs.atomicWriteTextFile(util::join(root_, ch.metadataPath), schemas::serializeChapterMeta(meta));
        });
    }
    if (leadingMoves) {   // the caret chapter's new folder and its sidecar, which follows its scenes
        jobs.clear();
        jobs.push_back([&]() -> Result<void> {
            auto dir = fs.createDirectories(util::join(root_, leadingDir));
            if (!dir.ok()) { return dir; }
            return writeChapterMetaInto(fs, root_, leadingDir, *caretChapterMeta, metaFilenamesOf(leadingNow));
        });
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
    // Batch 2: the new scenes' files, and the moves. ⚠️ When the leading scenes move, they leave FIRST: a new scene's file
    // name in that folder may be one of theirs (keys are only unique within a chapter).
    jobs.clear();
    if (leadingMoves) {
        for (std::size_t k = 0; k < leading.size(); ++k) { addMoveJobs(jobs, fs, root_, leading[k], leadingNow[k]); }
        if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
        jobs.clear();
    }
    for (const auto& s : created) {
        jobs.push_back([&] { return fs.atomicWriteTextFile(util::join(root_, s.location.contentPath), s.text); });
        jobs.push_back([&] {
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
            return fs.atomicWriteTextFile(util::join(root_, s.location.metadataPath), schemas::serializeSceneMeta(meta));
        });
    }
    if (!leadingMoves) {
        for (std::size_t k = 0; k < moved.size(); ++k) { addMoveJobs(jobs, fs, root_, trailing[k], moved[k]); }
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
    // Batch 3: the caret scene, the caret chapter's list (when its scenes changed) and the manuscript's chapter list.
    jobs.clear();
    jobs.push_back([&] { return fs.atomicWriteTextFile(util::join(root_, caretNow.contentPath), caretText); });
    jobs.push_back([&] { return restampSceneSidecar(fs, root_, caretNow.metadataPath, caretText, author, now); });   // READ 2
    const bool createdInCaretChapter = !created.empty() && created.front().location.chapterID.value == caret->chapterID.value;
    std::vector<std::string> caretChapterNames;
    if (leadingMoves) {
        // The new chapter's sidecar replaces the caret chapter's in its folder (that one was written above, in its new one).
        jobs.push_back([&] {
            const NewChapter& ch = newChapters.front();
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
            return writeChapterMetaInto(fs, root_, ch.dir, meta, ch.sceneFiles);
        });
    } else if (createdInCaretChapter || movesAfter) {
        caretChapterNames = metaFilenamesOf(leading);
        for (const auto& s : created) {
            if (s.location.chapterID.value == caret->chapterID.value) {
                caretChapterNames.push_back(fileNameOf(s.location.metadataPath));
            }
        }
        if (!movesAfter) {
            for (const auto& name : metaFilenamesOf(trailing)) { caretChapterNames.push_back(name); }
        }
        jobs.push_back([&] { return writeChapterScenes(fs, root_, caret->chapterMetadataPath, caretChapterNames); });   // READ 3
    }
    if (!newChapters.empty()) {
        jobs.push_back([&] {
            return editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
                auto at = std::find_if(refs.begin(), refs.end(),
                    [&](const schemas::ChapterRef& r) { return r.chapterID.value == caret->chapterID.value; });
                if (leadingMoves && at != refs.end()) { at->path = leadingDir + "/chapter.meta.json"; }
                std::vector<schemas::ChapterRef> add;
                for (const auto& ch : newChapters) { add.push_back({.chapterID = ch.chapterID, .path = ch.metadataPath}); }
                refs.insert(at == refs.end() ? refs.end() : std::next(at), add.begin(), add.end());
            });
        });
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }

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
    for (const auto& m : leadingNow) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }

    // The delta, in reading order.
    SceneID previous = beforeCaret;
    if (leadingMoves) {   // the caret chapter's scenes before the caret moved too
        previous = scenesBefore(leading.front().ordinal);
        for (std::size_t k = 0; k + 1 < leadingNow.size(); ++k) {
            out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, leadingNow[k], previous, {}});
            previous = leadingNow[k].sceneID;
        }
    }
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
    std::string survivorText = firstBody.value().substr(0, first.start) + endBody.substr(last.start + last.length);
    // ✅ [SP-188] Q3: balanced — the undo puts the plain join back first (`RangeJoin`).
    std::optional<RangeJoin> join = balanceJoin(firstBody.value().substr(0, first.start), first.text,
                                                n > 1 ? &last.text : nullptr, endBody.substr(last.start + last.length),
                                                survivorText);

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
    // ✅ [SP-191] (user, 2026-10-22): MOVE THE SMALLER SIDE (as `relabel` does). When the cut empties ONE chapter and the
    // start chapter keeps fewer scenes than the end chapter hands over, the start chapter's scenes move instead — into the
    // end chapter's folder, below the scenes already there, with the start chapter's sidecar — and the emptied folder is
    // the start chapter's. Same chapters, IDs and order; fewer renames, which do not overlap over a share.
    std::vector<SceneLocation> staying;   // the start chapter's scenes the cut keeps
    {
        std::unordered_set<std::string> cutIDs;
        for (const auto& id : removed) { cutIDs.insert(id.value); }
        for (const auto& s : scenesOfChapter(index_, startLoc.chapterID)) {
            if (cutIDs.count(s.sceneID.value) == 0) { staying.push_back(s); }
        }
    }
    const bool stayingMoves = !endSurvives && removedChapters.size() == 1 && !leaving.empty() &&
                              staying.size() < leaving.size();
    const RelativePath keptChapterMeta = stayingMoves ? endLoc.chapterMetadataPath : startLoc.chapterMetadataPath;
    std::vector<SceneLocation> stayingNow;   // `stayingMoves`: the start chapter's scenes in the end chapter's folder
    std::optional<schemas::ChapterMetaData> startChapterMeta;
    if (stayingMoves) {
        const std::string hi = sceneKeyOf(leaving.front().metadataPath);
        std::vector<std::string> keys;   // empty = they keep their names (they sort below the scenes already there)
        if (sceneKeyOf(staying.back().metadataPath) >= hi) {
            keys = keysBetween("", hi, staying.size());
            if (keys.size() != staying.size()) { return R::failure(internal("could not compute scene order keys")); }
        }
        for (std::size_t i = 0; i < staying.size(); ++i) {
            stayingNow.push_back(movedInto(staying[i], startLoc.chapterID, startLoc.chapterTitle, keptChapterMeta,
                                           keys.empty() ? "" : keys[i]));
        }
        auto meta = readChapterMeta(fs, root_, startLoc.chapterMetadataPath);   // read before any write: it follows them
        if (!meta.ok()) { return R::failure(meta.error()); }
        startChapterMeta = std::move(meta.value());
    }
    // ✅ Their file names already sort after the anchor's (the usual case: their keys are larger) ⇒ a plain move. Otherwise
    // each takes a new key after the last, and its sidecar is rewritten. When the staying scenes move, these stay put.
    const bool keepNames = stayingMoves || leaving.empty() || sceneKeyOf(leaving.front().metadataPath) > anchorKey;
    std::vector<SceneLocation> moved;   // ⚠️ when the end scene survives across chapters, `moved[0]` is that scene
    std::string prevKey = anchorKey;
    for (const auto& a : leaving) {
        std::string key;
        if (!keepNames) {
            key = util::keyBetween(prevKey, "");
            if (key.empty()) { return R::failure(internal("could not compute a scene order key")); }
            prevKey = key;
        }
        moved.push_back(movedInto(a, startLoc.chapterID, startLoc.chapterTitle, keptChapterMeta, key));
    }
    const SceneLocation survivor = stayingMoves ? stayingNow.back()
                                 : !endSurvives ? startLoc : !moved.empty() ? moved.front() : endLoc;

    // ✅ [SP-191] Q3b: what an undo needs to bring back what this cut removes — each removed scene's sidecar, and on the
    // entry that opens a removed chapter, that chapter's sidecar. Read before any write, like every other check.
    Range removedRange = range;
    if (join) { join->sceneID = survivor.sceneID; }
    removedRange.setJoin(std::move(join));   // ⚠️ never the join of the Range it was given (a redo's)
    std::vector<FileJob> jobs;
    {
        auto it = removedRange.entries().begin();
        for (std::size_t i = 0; i < n; ++i, ++it) {
            const SceneLocation& loc = order[start->ordinal + i];
            RangeEntry& e = *it;
            if (i >= removedFrom && i < removedTo) {
                jobs.push_back([&]() -> Result<void> {
                    auto side = fs.readTextFile(util::join(root_, loc.metadataPath));
                    if (!side.ok()) { return Result<void>::failure(side.error()); }
                    e.sceneMeta = side.value();
                    return Result<void>::success();
                });
            }
            if (i > 0 && loc.chapterID.value != order[start->ordinal + i - 1].chapterID.value) {
                jobs.push_back([&]() -> Result<void> {
                    auto chapter = fs.readTextFile(util::join(root_, loc.chapterMetadataPath));
                    if (!chapter.ok()) { return Result<void>::failure(chapter.error()); }
                    e.chapterMeta = chapter.value();
                    return Result<void>::success();
                });
            }
        }
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }

    // ── The writes ──
    // Batch 1: the cut scenes go FIRST: one in the start chapter can hold the very file name a promoted scene keeps (keys are
    // only unique within a chapter). ⛔ No relationship prune (user, 2026-10-20): the cut's undo, or a paste, brings the IDs
    // back.
    jobs.clear();
    for (std::size_t i = removedFrom; i < removedTo; ++i) {
        const SceneLocation& loc = order[start->ordinal + i];
        jobs.push_back([&] { return fs.removeFile(util::join(root_, loc.metadataPath)); });
        jobs.push_back([&] { return fs.removeFile(util::join(root_, loc.contentPath)); });
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
    // Batch 2: the moves.
    jobs.clear();
    if (stayingMoves) {
        for (std::size_t k = 0; k < staying.size(); ++k) { addMoveJobs(jobs, fs, root_, staying[k], stayingNow[k]); }
    } else {
        for (std::size_t k = 0; k < moved.size(); ++k) { addMoveJobs(jobs, fs, root_, leaving[k], moved[k]); }
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
    // Batch 3: the survivor, the emptied chapters (now that their scenes have left), and the two lists.
    jobs.clear();
    jobs.push_back([&] { return fs.atomicWriteTextFile(util::join(root_, survivor.contentPath), survivorText); });
    jobs.push_back([&] { return restampSceneSidecar(fs, root_, survivor.metadataPath, survivorText, author, now); });   // READ 3
    if (stayingMoves) {   // the start chapter's folder is the emptied one; its sidecar now heads the end chapter's folder
        jobs.push_back([&] { return removeChapterFolder(fs, root_, startLoc.chapterMetadataPath); });
    } else {
        for (const auto& ch : removedChapters) {
            jobs.push_back([&] { return removeChapterFolder(fs, root_, ch.metadataPath); });
        }
    }
    std::vector<std::string> startChapterNames;
    if (stayingMoves) {
        startChapterNames = metaFilenamesOf(stayingNow);
        for (const auto& name : metaFilenamesOf(leaving)) { startChapterNames.push_back(name); }
        jobs.push_back([&] {
            return writeChapterMetaInto(fs, root_, dirOf(keptChapterMeta), *startChapterMeta, startChapterNames);
        });
    } else if (n > 1) {
        std::unordered_set<std::string> gone;
        for (const auto& id : removed) { gone.insert(id.value); }
        for (const auto& s : scenesOfChapter(index_, startLoc.chapterID)) {
            if (gone.count(s.sceneID.value) == 0) { startChapterNames.push_back(fileNameOf(s.metadataPath)); }
        }
        for (const auto& m : moved) { startChapterNames.push_back(fileNameOf(m.metadataPath)); }
        jobs.push_back([&] { return writeChapterScenes(fs, root_, startLoc.chapterMetadataPath, startChapterNames); });   // READ 4
    }
    std::unordered_set<std::string> goneChapters;
    for (const auto& ch : removedChapters) { goneChapters.insert(ch.chapterID.value); }
    if (!removedChapters.empty()) {
        jobs.push_back([&] {
            return editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
                std::erase_if(refs, [&](const schemas::ChapterRef& r) { return goneChapters.count(r.chapterID.value) > 0; });
                if (stayingMoves) {
                    for (auto& r : refs) {
                        if (r.chapterID.value == startLoc.chapterID.value) { r.path = keptChapterMeta; }
                    }
                }
            });
        });
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }

    // ── The result ──
    RangeOperationResult out;
    const SceneID beforeStart = scenesBefore(startLoc.ordinal);
    out.indexCurrent = index_.removeScenes(removed);
    for (const auto& m : moved) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }
    for (const auto& m : stayingNow) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }

    SceneID before = beforeStart;
    if (stayingMoves) {   // the start chapter's scenes before the survivor moved too
        before = scenesBefore(staying.front().ordinal);
        for (std::size_t k = 0; k + 1 < stayingNow.size(); ++k) {
            out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, stayingNow[k], before, {}});
            before = stayingNow[k].sceneID;
        }
    }
    out.delta.scenes.push_back({RangeSceneChange::Kind::Rewritten, survivor, before, survivorText});
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

    // The sidecar of the chapter the run goes into when that is a new one: the chapter an undo re-creates, else a fresh one.
    auto newChapterMeta = [&]() -> Result<schemas::ChapterMetaData> {
        schemas::ChapterMetaData meta;
        const RangeEntry& first = *entries.front();
        if (!first.chapterMeta.empty()) {   // ✅ Q3b: an undo brings the chapter back as it was
            auto parsed = schemas::parseChapterMeta(first.chapterMeta);
            if (!parsed.ok()) { return parsed; }
            meta = std::move(parsed.value());
        } else {
            meta.title                = first.chapterTitle;   // [SP-191] Q10: "" = untitled
            meta.displayLabel         = meta.title;
            meta.status               = "draft";
            meta.createdAt            = now;
            meta.createdByIdentityID  = author.identityID.value;
            meta.createdByPersonaID   = author.personaID.value;
            meta.createdByDisplayName = author.displayName;
        }
        meta.chapterID = target.value.empty() || target.value.starts_with("~") ? services_.uuidProvider->newChapterID()
                                                                               : target;
        return Result<schemas::ChapterMetaData>::success(std::move(meta));
    };

    // ✅ [SP-191] (user, 2026-10-22): MOVE THE SMALLER SIDE. A relabel moves a run of whole scenes into the chapter next to
    // theirs; when the scenes on the OTHER side of the new boundary are fewer, those move instead — into the other folder,
    // with their chapter's sidecar — and the run stays where it lies, relabelled. The chapters, their IDs, their sidecars
    // and the reading order come out the same; only which folder holds which chapter differs. Over a network share every
    // move is a rename and renames do not overlap there (measured, Docker SMB share), so this is what halves the cost.
    //   • a chapter merge (the run is all of its chapter, into the one before): the earlier chapter's scenes, if fewer,
    //     move into the run's folder, below the run; that chapter's sidecar replaces the run's. The emptied folder is the
    //     earlier one.
    //   • a run into a new chapter after its own (a chapter split at a scene's start; a merge's undo): the scenes left
    //     behind, if fewer, move into a new folder BEFORE the source's, with the source's sidecar; the new chapter's sidecar
    //     takes the source's folder.
    std::vector<SceneLocation> rest;   // the source's scenes the run leaves behind
    for (const auto& s : sourceScenes) {
        if (std::none_of(run.begin(), run.end(), [&](const SceneLocation& r) { return r.sceneID.value == s.sceneID.value; })) {
            rest.push_back(s);
        }
    }
    const std::vector<SceneLocation> earlier = intoPrevious ? scenesOfChapter(index_, chapters[pos - 1].chapterID)
                                                            : std::vector<SceneLocation>{};
    const bool foldEarlier = intoPrevious && emptiesSource && earlier.size() < run.size();
    const bool splitOffRest = intoNew && !emptiesSource && rest.size() < run.size();
    if (foldEarlier || splitOffRest) {
        const RelativePath sourceMetaRel = sourceIt->metadataPath;
        const std::string  sourceDir     = dirOf(sourceMetaRel);
        const ChapterInfo  kept          = foldEarlier ? chapters[pos - 1] : *sourceIt;   // the chapter whose scenes move
        const std::vector<SceneLocation>& movers = foldEarlier ? earlier : rest;

        // Read before any write: the moving chapter's sidecar (it follows its scenes), and the source's when the merge
        // removes it (Q3b: its undo re-creates it).
        auto keptMeta = readChapterMeta(fs, root_, kept.metadataPath);
        if (!keptMeta.ok()) { return R::failure(keptMeta.error()); }
        std::string sourceMeta;
        if (foldEarlier) {
            auto chapter = fs.readTextFile(util::join(root_, sourceMetaRel));
            if (!chapter.ok()) { return R::failure(chapter.error()); }
            sourceMeta = chapter.value();
        }

        std::string toDir;                              // where the movers go
        std::optional<schemas::ChapterMetaData> runMeta;   // the new chapter's sidecar, around the run (split)
        ChapterID   runChapter;
        std::string runChapterTitle;
        std::vector<std::string> keys;                  // the movers' new keys; empty = they keep their names
        if (foldEarlier) {
            toDir           = sourceDir;
            runChapter      = kept.chapterID;
            runChapterTitle = kept.title;
            const std::string hi = sceneKeyOf(run.front().metadataPath);
            if (sceneKeyOf(movers.back().metadataPath) >= hi) {   // the usual case: every chapter's keys start alike
                keys = keysBetween("", hi, movers.size());
                if (keys.size() != movers.size()) { return R::failure(internal("could not compute scene order keys")); }
            }
        } else {
            const std::string prevKey = pos > 0 ? chapterKeyOf(chapters[pos - 1].metadataPath) : std::string();
            auto key = freeChapterKey(fs, root_, prevKey, chapterKeyOf(sourceMetaRel));
            if (!key.ok()) { return R::failure(key.error()); }
            toDir = "manuscript/chapter-" + key.value();
            auto metaR = newChapterMeta();
            if (!metaR.ok()) { return R::failure(metaR.error()); }
            runMeta         = std::move(metaR.value());
            runChapter      = runMeta->chapterID;
            runChapterTitle = runMeta->title;
        }
        const RelativePath keptMetaRel = toDir + "/chapter.meta.json";
        std::vector<SceneLocation> movedTo;
        for (std::size_t i = 0; i < movers.size(); ++i) {
            movedTo.push_back(movedInto(movers[i], kept.chapterID, kept.title, keptMetaRel, keys.empty() ? "" : keys[i]));
        }
        const std::vector<SceneLocation> runNow = relabelledInPlace(run, runChapter, runChapterTitle, sourceMetaRel);

        // ── The writes: a new folder first, then the moves, then the sidecars, and an emptied folder LAST ([I-0083]) ──
        if (splitOffRest) {
            auto dir = fs.createDirectories(util::join(root_, toDir));
            if (!dir.ok()) { return R::failure(dir.error()); }
        }
        std::vector<FileJob> jobs;
        for (std::size_t i = 0; i < movers.size(); ++i) { addMoveJobs(jobs, fs, root_, movers[i], movedTo[i]); }
        if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
        jobs.clear();
        std::vector<std::string> keptNames = metaFilenamesOf(movedTo);
        if (foldEarlier) {
            for (const auto& name : metaFilenamesOf(run)) { keptNames.push_back(name); }
            jobs.push_back([&] { return removeChapterFolder(fs, root_, kept.metadataPath); });
        } else {
            jobs.push_back([&] { return writeChapterMetaInto(fs, root_, sourceDir, *runMeta, metaFilenamesOf(run)); });
        }
        jobs.push_back([&] { return writeChapterMetaInto(fs, root_, toDir, keptMeta.value(), keptNames); });
        jobs.push_back([&] {
            return editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
                for (auto& r : refs) {
                    if (r.chapterID.value == kept.chapterID.value) { r.path = keptMetaRel; }
                }
                if (foldEarlier) {
                    std::erase_if(refs, [&](const schemas::ChapterRef& r) { return r.chapterID.value == source.value; });
                } else {
                    auto at = std::find_if(refs.begin(), refs.end(),
                        [&](const schemas::ChapterRef& r) { return r.chapterID.value == source.value; });
                    refs.insert(at == refs.end() ? refs.end() : std::next(at),
                                schemas::ChapterRef{.chapterID = runChapter, .path = sourceMetaRel});
                }
            });
        });
        if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }

        // ── The result ── (the reading order is unchanged: the movers come first in it, then the run)
        RangeOperationResult out;
        out.indexCurrent = true;
        for (const auto& m : movedTo) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }
        for (const auto& m : runNow) { out.indexCurrent = out.indexCurrent && index_.relocateScene(m); }
        SceneID previous = scenesBefore(movers.front().ordinal);
        for (const auto* list : std::initializer_list<const std::vector<SceneLocation>*>{&movedTo, &runNow}) {
            for (const auto& m : *list) {
                out.delta.scenes.push_back({RangeSceneChange::Kind::Moved, m, previous, {}});
                previous = m.sceneID;
            }
        }
        if (foldEarlier) {
            out.delta.removedChapterIDs.push_back(source);
        } else {
            out.delta.createdChapters.push_back({runChapter, runChapterTitle, sourceMetaRel, source});
        }
        for (std::size_t k = 0; k < runNow.size(); ++k) {   // the run as it is now, as below
            RangeEntry e;
            e.sceneID      = runNow[k].sceneID;
            e.chapterID    = runChapter;
            e.chapterTitle = sourceIt->title;
            e.sceneTitle   = runNow[k].title;
            if (k == 0) { e.chapterMeta = sourceMeta; }
            out.range.append(std::move(e));
        }
        out.fromChapterID = source;
        return R::success(std::move(out));
    }

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
        auto metaR = newChapterMeta();
        if (!metaR.ok()) { return R::failure(metaR.error()); }
        schemas::ChapterMetaData meta = std::move(metaR.value());
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
    // Batch 1: a new destination's folder and sidecar.
    if (newChapter) {
        auto dir = fs.createDirectories(util::join(root_, newChapterDir));
        if (!dir.ok()) { return R::failure(dir.error()); }
        newChapter->scenes.clear();
        for (const auto& m : moved) { newChapter->scenes.push_back({.metadataFilename = fileNameOf(m.metadataPath)}); }
        auto w = fs.atomicWriteTextFile(util::join(root_, dest.metadataPath), schemas::serializeChapterMeta(*newChapter));
        if (!w.ok()) { return R::failure(w.error()); }
    }
    // Batch 2: the moves.
    std::vector<FileJob> jobs;
    for (std::size_t k = 0; k < run.size(); ++k) { addMoveJobs(jobs, fs, root_, run[k], moved[k]); }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }
    // Batch 3: the destination's list, the source (its list, or the emptied chapter removed) and the manuscript's list.
    jobs.clear();
    std::vector<std::string> destNames;
    if (!newChapter) {
        destNames = intoPrevious ? metaFilenamesOf(destScenes) : std::vector<std::string>{};
        for (const auto& m : moved) { destNames.push_back(fileNameOf(m.metadataPath)); }
        if (intoNext) {
            for (const auto& name : metaFilenamesOf(destScenes)) { destNames.push_back(name); }
        }
        jobs.push_back([&] { return writeChapterScenes(fs, root_, dest.metadataPath, destNames); });
    }
    if (emptiesSource) {
        jobs.push_back([&] { return removeChapterFolder(fs, root_, sourceIt->metadataPath); });
    } else {
        jobs.push_back([&] { return writeChapterScenes(fs, root_, sourceIt->metadataPath, metaFilenamesOf(rest)); });
    }
    if (newChapter || emptiesSource) {
        jobs.push_back([&] {
            return editManuscriptChapters(fs, root_, [&](std::vector<schemas::ChapterRef>& refs) {
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
        });
    }
    if (auto b = runConcurrently(jobs); !b.ok()) { return R::failure(b.error()); }

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
