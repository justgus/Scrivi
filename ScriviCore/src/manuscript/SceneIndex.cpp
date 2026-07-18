#include "manuscript/SceneIndex.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::manuscript {

namespace {

constexpr std::string_view kMetaSuffix = ".meta.json";

// The `.md` filename paired with a `<...>.meta.json` filename.
std::string contentFilenameFor(const std::string& metaFilename) {
    if (metaFilename.size() > kMetaSuffix.size() &&
        metaFilename.compare(metaFilename.size() - kMetaSuffix.size(),
                             kMetaSuffix.size(), kMetaSuffix) == 0) {
        return metaFilename.substr(0, metaFilename.size() - kMetaSuffix.size()) + ".md";
    }
    return metaFilename;   // defensive; shouldn't happen for a scene meta
}

} // namespace

std::string chapterDirOf(const std::string& chapterMetadataRelPath) {
    return std::filesystem::path(chapterMetadataRelPath).parent_path().string();
}

std::string sceneMetaRelPath(const std::string& chapterMetadataRelPath,
                             const std::string& metadataFilename) {
    return chapterDirOf(chapterMetadataRelPath) + "/" + metadataFilename;
}

std::string sceneOrderKeyOf(const std::string& filename) {
    // Take the substring before the first '-'. Empty if there is none.
    const auto dash = filename.find('-');
    if (dash == std::string::npos || dash == 0) { return {}; }
    return filename.substr(0, dash);
}

Result<std::vector<SceneEntry>> listScenesByOrder(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& chapterMetadataRelPath)
{
    const std::string chDir = chapterDirOf(chapterMetadataRelPath);
    auto listR = fs.listDirectory(util::join(projectRoot, chDir));
    if (!listR.ok()) { return Result<std::vector<SceneEntry>>::failure(listR.error()); }

    std::vector<SceneEntry> entries;
    for (const auto& absEntry : listR.value()) {
        const std::string name = util::filename(absEntry);
        if (name == "chapter.meta.json") { continue; }
        if (name.size() <= kMetaSuffix.size() ||
            name.compare(name.size() - kMetaSuffix.size(),
                         kMetaSuffix.size(), kMetaSuffix) != 0) {
            continue;   // not a *.meta.json file (e.g. the paired .md)
        }
        const std::string key = sceneOrderKeyOf(name);
        if (key.empty()) { continue; }   // not an ordered scene file

        const std::string relPath = chDir + "/" + name;
        auto sTextR = fs.readTextFile(util::join(projectRoot, relPath));
        if (!sTextR.ok()) { return Result<std::vector<SceneEntry>>::failure(sTextR.error()); }
        auto sParsed = schemas::parseSceneMeta(sTextR.value());
        if (!sParsed.ok()) { return Result<std::vector<SceneEntry>>::failure(sParsed.error()); }

        SceneEntry e;
        e.orderKey         = key;
        e.sceneID          = sParsed.value().sceneID;
        e.metadataFilename = name;
        e.metadataRelPath  = relPath;
        e.contentFilename  = sParsed.value().contentPath;
        entries.push_back(std::move(e));
    }

    // Reading order == order-key ascending byte order (B3). listDirectory does not promise
    // order, so sort explicitly.
    std::sort(entries.begin(), entries.end(),
              [](const SceneEntry& a, const SceneEntry& b) {
                  return a.orderKey < b.orderKey;
              });
    return Result<std::vector<SceneEntry>>::success(std::move(entries));
}

Result<bool> rebuildChapterScenesIfInconsistent(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& chapterMetadataRelPath)
{
    auto onDiskR = listScenesByOrder(fs, projectRoot, chapterMetadataRelPath);
    if (!onDiskR.ok()) { return Result<bool>::failure(onDiskR.error()); }
    const auto& onDisk = onDiskR.value();

    const std::string chAbs = util::join(projectRoot, chapterMetadataRelPath);
    auto chTextR = fs.readTextFile(chAbs);
    if (!chTextR.ok()) { return Result<bool>::failure(chTextR.error()); }
    auto chParsed = schemas::parseChapterMeta(chTextR.value());
    if (!chParsed.ok()) { return Result<bool>::failure(chParsed.error()); }
    auto& ch = chParsed.value();

    // The cache is a PURE MIRROR of the chapter folder: the on-disk scene files in order-key
    // order, nothing else. A ref whose file is not physically in this folder is dropped — it
    // is NOT preserved. (Preserving it was unsafe: after a scene reorders OUT of this chapter
    // its ref must disappear, and order-key scene filenames repeat across chapters, so a
    // stale ref would let orphan-repair "find" a same-named file in another chapter and steal
    // it — the "Sentinel of Centauri" corruption. A genuine loss is surfaced by the
    // ProjectValidator against DISK, and true cross-folder orphans are relocated by sceneID
    // in migrateScenes, not by preserving a filename-only ref here.)
    std::vector<schemas::SceneRef> rebuilt;
    rebuilt.reserve(onDisk.size());
    for (const auto& e : onDisk) {
        rebuilt.push_back(schemas::SceneRef{.metadataFilename = e.metadataFilename});
    }

    // Consistent iff the cache already equals the rebuilt array (same members, same order).
    bool consistent = (ch.scenes.size() == rebuilt.size());
    for (std::size_t i = 0; consistent && i < rebuilt.size(); ++i) {
        if (ch.scenes[i].metadataFilename != rebuilt[i].metadataFilename) {
            consistent = false;
        }
    }
    if (consistent) { return Result<bool>::success(false); }

    ch.scenes = std::move(rebuilt);
    auto wR = fs.atomicWriteTextFile(chAbs, schemas::serializeChapterMeta(ch));
    if (!wR.ok()) { return Result<bool>::failure(wR.error()); }
    return Result<bool>::success(true);
}

namespace {

// Rename a scene's `.meta.json` + `.md` within (or into) a chapter folder, giving them a
// new `<newKey>-<slug>` stem. Rewrites the sidecar `slug` + `contentPath` (bare filename).
// `srcMetaRel`/`dstMetaRel` are root-relative. Returns the new content filename.
Result<void> renameSceneFiles(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& srcMetaRel, const std::string& srcContentFilename,
    const std::string& dstMetaRel, const std::string& newKey,
    const std::string& baseSlug)
{
    const std::string dstDir = std::filesystem::path(dstMetaRel).parent_path().string();
    const std::string srcDir = std::filesystem::path(srcMetaRel).parent_path().string();
    const std::string dstMetaFilename = util::filename(dstMetaRel);
    const std::string dstContentFilename =
        newKey + "-" + baseSlug + ".md";
    const std::string srcContentRel = srcDir + "/" + srcContentFilename;
    const std::string dstContentRel = dstDir + "/" + dstContentFilename;

    // Move the content file first, then the meta file. If the meta already sits at the
    // destination (same path), skip the rename but still rewrite its fields.
    if (srcContentRel != dstContentRel) {
        auto mv = fs.renamePath(util::join(projectRoot, srcContentRel),
                                util::join(projectRoot, dstContentRel));
        if (!mv.ok()) { return Result<void>::failure(mv.error()); }
    }
    if (srcMetaRel != dstMetaRel) {
        auto mv = fs.renamePath(util::join(projectRoot, srcMetaRel),
                                util::join(projectRoot, dstMetaRel));
        if (!mv.ok()) { return Result<void>::failure(mv.error()); }
    }

    // Rewrite the sidecar: slug + content filename now match the new stem.
    auto sTextR = fs.readTextFile(util::join(projectRoot, dstMetaRel));
    if (!sTextR.ok()) { return Result<void>::failure(sTextR.error()); }
    auto sParsed = schemas::parseSceneMeta(sTextR.value());
    if (!sParsed.ok()) { return Result<void>::failure(sParsed.error()); }
    sParsed.value().slug        = newKey + "-" + baseSlug;
    sParsed.value().contentPath = dstContentFilename;
    auto wR = fs.atomicWriteTextFile(util::join(projectRoot, dstMetaRel),
                                     schemas::serializeSceneMeta(sParsed.value()));
    if (!wR.ok()) { return Result<void>::failure(wR.error()); }
    return Result<void>::success();
}

// The slug portion after the `<key>-` prefix, sans the ".meta.json" suffix. For
// "001-scene.meta.json" -> "scene"; for "a0-my-scene.meta.json" -> "my-scene".
std::string baseSlugOf(const std::string& metaFilename) {
    std::string stem = metaFilename;
    if (stem.size() > kMetaSuffix.size() &&
        stem.compare(stem.size() - kMetaSuffix.size(),
                     kMetaSuffix.size(), kMetaSuffix) == 0) {
        stem = stem.substr(0, stem.size() - kMetaSuffix.size());
    }
    const auto dash = stem.find('-');
    return (dash == std::string::npos) ? stem : stem.substr(dash + 1);
}

} // namespace

Result<bool> migrateScenes(FileSystem& fs, const AbsolutePath& projectRoot)
{
    auto chaptersR = listChaptersByOrder(fs, projectRoot);
    if (!chaptersR.ok()) { return Result<bool>::failure(chaptersR.error()); }

    bool changed = false;

    for (const auto& chapter : chaptersR.value()) {
        const std::string chDir = chapterDirOf(chapter.chapterMetadataRelPath);
        const std::string chAbs = util::join(projectRoot, chapter.chapterMetadataRelPath);

        auto chTextR = fs.readTextFile(chAbs);
        if (!chTextR.ok()) { return Result<bool>::failure(chTextR.error()); }
        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) { return Result<bool>::failure(chParsed.error()); }

        // NOTE — no filename-based cross-folder "orphan relocation" pass. Order-key scene
        // filenames REPEAT across chapters (keys restart per chapter: every chapter has an
        // `001-…`, and often a `V-…`, `k-…`), so a filename like `k-scene.meta.json` is NOT a
        // manuscript-unique key. Relocating "the file with this name found in another
        // chapter" therefore steals the WRONG scene — the "Sentinel of Centauri" corruption
        // (a Ch2 reorder-out left a stale ref, and repair dragged Ch1's identically-named
        // scene into Ch2). The scene sidecar carries no owning-chapter field, so a moved file
        // cannot be re-homed by identity either. Instead we rely on: (a) SceneReorderer
        // always MOVING files atomically (new-scheme projects never orphan), and (b) the
        // cache being a pure disk mirror (pass 3) so every project stays internally
        // consistent and OPENABLE — a scene physically present in a folder is owned by that
        // folder's chapter, full stop. A genuinely lost file surfaces via the
        // ProjectValidator (against disk), not by a filename guess.

        // Pass 2 — reslug legacy numeric filenames to order keys. Driven by the ON-DISK
        // scene files of THIS folder (not the cache), so a scene physically present here is
        // owned here and a stale/foreign ref can never derail it. A file whose filename
        // prefix isn't a well-formed order key (e.g. legacy "010-…", which ends in '0') is
        // renamed to a fresh ascending key, in current on-disk order. New-scheme files are
        // untouched (no-op).
        auto diskScenesR = listScenesByOrder(fs, projectRoot, chapter.chapterMetadataRelPath);
        if (!diskScenesR.ok()) { return Result<bool>::failure(diskScenesR.error()); }

        std::string lastKey;
        for (const auto& e : diskScenesR.value()) {
            const std::string key = e.orderKey;
            const std::string metaRel = chDir + "/" + e.metadataFilename;

            if (util::isOrderKey(key)) {
                lastKey = key;   // already migrated — keep its key as the running anchor
                continue;
            }
            const std::string newKey = util::keyAfter(lastKey);
            const std::string baseSlug = baseSlugOf(e.metadataFilename);
            const std::string newFilename = newKey + "-" + baseSlug + ".meta.json";
            const std::string dstMetaRel = chDir + "/" + newFilename;

            auto rn = renameSceneFiles(
                fs, projectRoot, metaRel,
                contentFilenameFor(e.metadataFilename),
                dstMetaRel, newKey, baseSlug);
            if (!rn.ok()) { return Result<bool>::failure(rn.error()); }
            lastKey = newKey;
            changed = true;
        }

        // Pass 3 — rebuild the scenes[] cache from disk (also normalises any legacy-shape
        // refs that were read as bare filenames into the new schema on write).
        auto rb = rebuildChapterScenesIfInconsistent(
            fs, projectRoot, chapter.chapterMetadataRelPath);
        if (!rb.ok()) { return Result<bool>::failure(rb.error()); }
        if (rb.value()) { changed = true; }
    }

    return Result<bool>::success(changed);
}

} // namespace scrivi::manuscript
