#include "manuscript/ChapterIndex.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::manuscript {

namespace {

constexpr std::string_view kChapterPrefix = "chapter-";

// If `folderName` is a `chapter-<key>` directory, return <key>; else empty.
std::string orderKeyOf(std::string_view folderName) {
    if (folderName.size() <= kChapterPrefix.size() ||
        folderName.substr(0, kChapterPrefix.size()) != kChapterPrefix) {
        return {};
    }
    return std::string(folderName.substr(kChapterPrefix.size()));
}

} // namespace

Result<std::string> renameChapterFolder(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& oldKey, const std::string& newKey)
{
    const std::string oldDir = "manuscript/chapter-" + oldKey;
    const std::string newDir = "manuscript/chapter-" + newKey;
    const std::string newChMetaRel = newDir + "/chapter.meta.json";

    auto renameR = fs.renamePath(util::join(projectRoot, oldDir),
                                 util::join(projectRoot, newDir));
    if (!renameR.ok()) { return Result<std::string>::failure(renameR.error()); }

    // EP-027 §8.1: scene references are BARE FILENAMES (resolved against the chapter's own
    // folder) and scene contentPaths are bare filenames too, so a chapter-folder rename
    // touches NO scene fields — the whole coupling that broke B3/B4/C6 is gone. Only the
    // chapter sidecar's own `slug` still embeds the folder name and must be rewritten.
    auto chTextR = fs.readTextFile(util::join(projectRoot, newChMetaRel));
    if (!chTextR.ok()) { return Result<std::string>::failure(chTextR.error()); }
    auto chParsed = schemas::parseChapterMeta(chTextR.value());
    if (!chParsed.ok()) { return Result<std::string>::failure(chParsed.error()); }
    auto& ch = chParsed.value();
    ch.slug = "chapter-" + newKey;

    auto wR = fs.atomicWriteTextFile(util::join(projectRoot, newChMetaRel),
                                     schemas::serializeChapterMeta(ch));
    if (!wR.ok()) { return Result<std::string>::failure(wR.error()); }
    return Result<std::string>::success(newChMetaRel);
}

Result<std::vector<ChapterEntry>> listChaptersByOrder(
    FileSystem& fs, const AbsolutePath& projectRoot)
{
    const AbsolutePath manuscriptDir = util::join(projectRoot, "manuscript");
    auto listR = fs.listDirectory(manuscriptDir);
    if (!listR.ok()) {
        return Result<std::vector<ChapterEntry>>::failure(listR.error());
    }

    std::vector<ChapterEntry> entries;
    for (const auto& absEntry : listR.value()) {
        const std::string folderName = util::filename(absEntry);
        const std::string key = orderKeyOf(folderName);
        if (key.empty()) {
            continue;   // not a chapter-<key> folder (e.g. manuscript.meta.json)
        }

        // Read the sidecar for the authoritative chapterID.
        const std::string chMetaRel =
            "manuscript/" + folderName + "/chapter.meta.json";
        auto chTextR = fs.readTextFile(util::join(projectRoot, chMetaRel));
        if (!chTextR.ok()) {
            return Result<std::vector<ChapterEntry>>::failure(chTextR.error());
        }
        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) {
            return Result<std::vector<ChapterEntry>>::failure(chParsed.error());
        }

        ChapterEntry e;
        e.orderKey               = key;
        e.chapterID              = chParsed.value().chapterID;
        e.chapterMetadataRelPath = chMetaRel;
        e.slug                   = folderName;
        entries.push_back(std::move(e));
    }

    // Reading order == order-key ascending (byte order). listDirectory does not promise
    // order, so we sort explicitly — this is the authoritative manuscript order (B3).
    std::sort(entries.begin(), entries.end(),
              [](const ChapterEntry& a, const ChapterEntry& b) {
                  return a.orderKey < b.orderKey;
              });
    return Result<std::vector<ChapterEntry>>::success(std::move(entries));
}

Result<ChapterID> resolveChapterID(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& chapterMetadataRelPath)
{
    auto chTextR = fs.readTextFile(util::join(projectRoot, chapterMetadataRelPath));
    if (!chTextR.ok()) {
        return Result<ChapterID>::failure(chTextR.error());
    }
    auto chParsed = schemas::parseChapterMeta(chTextR.value());
    if (!chParsed.ok()) {
        return Result<ChapterID>::failure(chParsed.error());
    }
    return Result<ChapterID>::success(chParsed.value().chapterID);
}

Result<ChapterEntry> findChapterByID(
    FileSystem& fs, const AbsolutePath& projectRoot, const ChapterID& chapterID)
{
    auto listR = listChaptersByOrder(fs, projectRoot);
    if (!listR.ok()) {
        return Result<ChapterEntry>::failure(listR.error());
    }
    for (auto& e : listR.value()) {
        if (e.chapterID.value == chapterID.value) {
            return Result<ChapterEntry>::success(std::move(e));
        }
    }
    return Result<ChapterEntry>::failure(
        {.code = ErrorCode::invalidArgument,
         .message = "chapterID not found on disk: " + chapterID.value});
}

Result<bool> rebuildIndexIfInconsistent(
    FileSystem& fs, const AbsolutePath& projectRoot)
{
    // Authoritative on-disk chapters, in order-key order.
    auto diskR = listChaptersByOrder(fs, projectRoot);
    if (!diskR.ok()) {
        return Result<bool>::failure(diskR.error());
    }
    const auto& disk = diskR.value();

    const std::string msMetaPath =
        util::join(projectRoot, "manuscript/manuscript.meta.json");

    // Read + parse the current index. A missing/unreadable/malformed index counts as
    // inconsistent and is rebuilt (we still need its non-chapters fields, so if it can't
    // be read at all we cannot rebuild in place — surface that error).
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) {
        return Result<bool>::failure(msTextR.error());
    }
    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) {
        return Result<bool>::failure(msParsed.error());
    }
    auto& ms = msParsed.value();

    // Consistent iff the index lists exactly the disk chapters, same ids, same paths, in
    // the same (order-key) order.
    bool consistent = ms.chapters.size() == disk.size();
    if (consistent) {
        for (std::size_t i = 0; i < disk.size(); ++i) {
            if (ms.chapters[i].chapterID.value != disk[i].chapterID.value ||
                ms.chapters[i].path != disk[i].chapterMetadataRelPath) {
                consistent = false;
                break;
            }
        }
    }
    if (consistent) {
        return Result<bool>::success(false);   // nothing to do
    }

    // Rebuild chapters[] from disk (preserve all other manuscript fields).
    ms.chapters.clear();
    ms.chapters.reserve(disk.size());
    for (const auto& e : disk) {
        schemas::ChapterRef ref;
        ref.chapterID = e.chapterID;
        ref.path      = e.chapterMetadataRelPath;
        ms.chapters.push_back(std::move(ref));
    }

    auto writeR = fs.atomicWriteTextFile(msMetaPath, schemas::serializeManuscriptMeta(ms));
    if (!writeR.ok()) {
        return Result<bool>::failure(writeR.error());
    }
    return Result<bool>::success(true);
}

Result<bool> migrateChapterOrderKeys(
    FileSystem& fs, const AbsolutePath& projectRoot)
{
    // The legacy/intended order is the index ARRAY order. Read it.
    const std::string msMetaPath =
        util::join(projectRoot, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msMetaPath);
    if (!msTextR.ok()) { return Result<bool>::failure(msTextR.error()); }
    auto msParsed = schemas::parseManuscriptMeta(msTextR.value());
    if (!msParsed.ok()) { return Result<bool>::failure(msParsed.error()); }
    const auto& indexChapters = msParsed.value().chapters;

    // Current on-disk chapters, keyed by chapterID → order-key. (Authoritative identity.)
    auto diskR = listChaptersByOrder(fs, projectRoot);
    if (!diskR.ok()) { return Result<bool>::failure(diskR.error()); }

    // Map chapterID → current order-key from disk.
    auto keyForID = [&](const std::string& id) -> std::string {
        for (const auto& e : diskR.value()) {
            if (e.chapterID.value == id) return e.orderKey;
        }
        return {};   // in index but not on disk (a phantom entry) — skip it
    };

    // The intended order as a sequence of order-keys the chapters CURRENTLY have, walking
    // the index array. Only chapters that actually exist on disk participate.
    std::vector<std::string> intendedIDs;
    std::vector<std::string> currentKeysInIntendedOrder;
    for (const auto& ref : indexChapters) {
        const std::string k = keyForID(ref.chapterID.value);
        if (k.empty()) continue;   // phantom index entry — ignore (self-heal drops it)
        intendedIDs.push_back(ref.chapterID.value);
        currentKeysInIntendedOrder.push_back(k);
    }

    // Already migrated iff those current keys are already strictly ascending — i.e. the
    // folder-key sort already reproduces the index-array order. No-op then.
    bool ascending = true;
    for (std::size_t i = 1; i < currentKeysInIntendedOrder.size(); ++i) {
        if (!(currentKeysInIntendedOrder[i - 1] < currentKeysInIntendedOrder[i])) {
            ascending = false;
            break;
        }
    }
    if (ascending) {
        return Result<bool>::success(false);   // nothing to migrate
    }

    // Reassign fresh ascending order-keys in index-array order, renaming each folder whose
    // current key differs from its target. We generate keys with keyAfter() from a moving
    // cursor so the result is strictly ascending and collision-free.
    bool renamedAny = false;
    std::string prevKey;   // empty → keyAfter("") for the first
    for (std::size_t i = 0; i < intendedIDs.size(); ++i) {
        const std::string targetKey = util::keyAfter(prevKey);
        const std::string currentKey = keyForID(intendedIDs[i]);
        if (currentKey != targetKey) {
            // Rename this chapter's folder to targetKey. currentKey is re-read each time
            // (a prior rename doesn't touch this folder, so keyForID is still valid here —
            // but re-read to be safe against any interaction).
            auto rr = renameChapterFolder(fs, projectRoot, currentKey, targetKey);
            if (!rr.ok()) { return Result<bool>::failure(rr.error()); }
            renamedAny = true;
        }
        prevKey = targetKey;
    }

    // Rebuild the index cache to match the freshly-ordered disk state.
    if (renamedAny) {
        auto healR = rebuildIndexIfInconsistent(fs, projectRoot);
        if (!healR.ok()) { return Result<bool>::failure(healR.error()); }
    }
    return Result<bool>::success(renamedAny);
}

} // namespace scrivi::manuscript
