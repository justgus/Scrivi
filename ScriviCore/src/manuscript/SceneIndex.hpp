#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::manuscript {

// SceneIndex — filesystem-authoritative scene identity & ordering helpers (EP-027 §8,
// the scene analogue of ChapterIndex). Within a chapter folder, the scene `.meta.json`
// FILES are the source of truth: their order-key filename sort is scene reading order,
// and each file's own `sceneID` is the authoritative identity. The chapter sidecar's
// `scenes[]` array is a derived, rebuildable cache (an ordered list of bare filenames),
// never trusted over disk.
//
// A scene's filename is `<orderKey>-<slug>.meta.json` (+ a matching `.md`), reusing the
// same base-62 order-key alphabet as chapters (util/OrderKey). References in the chapter
// sidecar are BARE FILENAMES resolved against the chapter's own folder, so a chapter
// folder rename touches ZERO scene fields (the fix for the B3/B4/C6 orphaning class).

// One scene as found on disk inside a chapter folder.
struct SceneEntry {
    std::string orderKey;            // the `<orderKey>` prefix of the filename
    SceneID     sceneID;             // authoritative, read from the scene sidecar
    std::string metadataFilename;    // bare filename, e.g. "a0-scene.meta.json"
    std::string metadataRelPath;     // root-relative path (chapterDir + "/" + filename)
    std::string contentFilename;     // bare `.md` filename (from the sidecar contentPath)
};

// The chapter directory (root-relative) that owns `chapterMetadataRelPath`
// (e.g. "manuscript/chapter-a0/chapter.meta.json" -> "manuscript/chapter-a0").
std::string chapterDirOf(const std::string& chapterMetadataRelPath);

// Resolve a scene reference (bare filename) to its root-relative meta path, against the
// chapter directory derived from `chapterMetadataRelPath`.
std::string sceneMetaRelPath(const std::string& chapterMetadataRelPath,
                             const std::string& metadataFilename);

// If `filename` is `<orderKey>-<rest>.meta.json`, return <orderKey>; else empty. A legacy
// numeric prefix (`001-scene.meta.json`) yields "001" — a valid, if non-order-key, prefix
// that still sorts; isOrderKey distinguishes it for migration.
std::string sceneOrderKeyOf(const std::string& filename);

// List a chapter's scenes IN READING ORDER by scanning its folder for `*.meta.json` files
// (excluding `chapter.meta.json`) and sorting by the filename's order-key prefix. Reads
// each sidecar for the authoritative sceneID + contentPath filename. This is the
// authoritative scene order (B3); the chapter sidecar `scenes[]` array order is ignored.
Result<std::vector<SceneEntry>> listScenesByOrder(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& chapterMetadataRelPath);

// Rebuild a chapter sidecar's `scenes[]` cache from its on-disk scene files if it disagrees
// with disk (order, membership, or a filename whose sidecar can't be read). Returns true if
// a rewrite happened. The chapter's other fields are preserved.
Result<bool> rebuildChapterScenesIfInconsistent(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& chapterMetadataRelPath);

// Migrate + orphan-repair every chapter's scenes (EP-027 §8.2). For each chapter:
//   - relocate orphans: a scene file referenced by this chapter but physically absent from
//     its folder is found by sceneID scan across the manuscript and MOVED into this folder
//     (repairs the pre-EP-027 SceneReorderer damage — the C6 "Missing scene.meta.json");
//   - reslug legacy numeric filenames (`NNN-*`) to order-key filenames in current array
//     order, renaming the `.meta.json` + `.md`;
//   - rebuild the `scenes[]` cache from the folder scan.
// Lazy / idempotent / resumable / no-op when already migrated. Returns true if anything
// changed. Called on open (after chapter migration, before resolve).
Result<bool> migrateScenes(FileSystem& fs, const AbsolutePath& projectRoot);

} // namespace scrivi::manuscript
