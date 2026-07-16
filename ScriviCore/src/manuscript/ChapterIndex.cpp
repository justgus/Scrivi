#include "manuscript/ChapterIndex.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
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

} // namespace scrivi::manuscript
