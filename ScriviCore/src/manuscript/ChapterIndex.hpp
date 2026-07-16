#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::manuscript {

// ChapterIndex — filesystem-authoritative chapter identity & ordering helpers (EP-027
// P2, B3). The on-disk `chapter-<orderKey>` folders ARE the source of truth: their
// order-key sort is manuscript order, and each folder's `chapter.meta.json` carries the
// authoritative `chapterID`. `manuscript.meta.json` is a derived cache (an ordered list
// of paths), never trusted over disk.
//
// These helpers let consumers resolve a chapter's identity/order/path from disk WITHOUT
// each one hand-rolling a directory scan + sidecar read. They replace the old pattern of
// matching `ChapterRef.chapterID` inside the index (which could diverge from the sidecar
// — the I-0072 class).

// One chapter as found on disk: its order-key (from the folder name), its chapterID
// (from the sidecar), and the relative path to its chapter.meta.json.
struct ChapterEntry {
    std::string orderKey;                 // the `<orderKey>` from `chapter-<orderKey>`
    ChapterID   chapterID;                // authoritative, read from the sidecar
    std::string chapterMetadataRelPath;   // e.g. "manuscript/chapter-a0m/chapter.meta.json"
    std::string slug;                     // the full folder name, e.g. "chapter-a0m"
};

// List the manuscript's chapters IN READING ORDER — scan `manuscript/`, take every
// `chapter-*` directory, and sort by its order-key (ascending byte order == manuscript
// order). Reads each sidecar for the chapterID. This is the authoritative order (B3);
// the index array order is ignored. A malformed/missing sidecar for a folder makes that
// folder fail the read (surfaced as an error) — repair/migration handles legacy shapes.
Result<std::vector<ChapterEntry>> listChaptersByOrder(
    FileSystem& fs, const AbsolutePath& projectRoot);

// Resolve the chapterID stored in the sidecar at `chapterMetadataRelPath` (relative to
// projectRoot). The sidecar is the sole owner of identity (B3).
Result<ChapterID> resolveChapterID(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& chapterMetadataRelPath);

// Find a chapter by its id: returns its ChapterEntry (path + order-key) by scanning the
// on-disk folders and matching the sidecar chapterID. Fails (invalidArgument) if no
// chapter on disk has that id. Consumers that receive a chapterID in a request (reorder,
// rename, delete) use this instead of matching the index.
Result<ChapterEntry> findChapterByID(
    FileSystem& fs, const AbsolutePath& projectRoot, const ChapterID& chapterID);

// Rename a chapter's folder from `chapter-<oldKey>` to `chapter-<newKey>`: renamePath the
// directory (atomic, never clobbers) and rewrite every relative path that embedded the
// old folder — the sidecar `slug`, each scene's `metadataPath` in the chapter sidecar, and
// each scene sidecar's own `contentPath` — so scene bodies stay resolvable. Returns the
// new chapter.meta.json rel-path. Shared by reorder and migration (the folder-move
// primitive of the order-key scheme).
Result<std::string> renameChapterFolder(
    FileSystem& fs, const AbsolutePath& projectRoot,
    const std::string& oldKey, const std::string& newKey);

// Self-heal the manuscript.meta.json chapter index if it disagrees with the on-disk
// folders (EP-027 B3 rebuild-on-inconsistency). Compares the index's `chapters[]`
// (chapterID + path pairs) against the authoritative on-disk chapters in order-key order;
// if they differ in any way — order, membership, a path whose sidecar id doesn't match
// (I-0072 phantom), or a duplicate — the index is rewritten from disk so it becomes a
// faithful, de-duplicated, correctly-ordered cache. Returns true if a rewrite happened.
// A missing/unreadable index is treated as inconsistent and rebuilt. Best-effort: a
// write failure surfaces as an error, but callers may ignore it (order is disk-derived
// regardless).
Result<bool> rebuildIndexIfInconsistent(
    FileSystem& fs, const AbsolutePath& projectRoot);

// Migrate a project whose chapter FOLDER order-key sort does not reproduce its intended
// reading order (EP-027 P3). The intended order for a legacy/old-format project is the
// `manuscript.meta.json` `chapters[]` ARRAY order (the last authoritative order under the
// pre-EP-027 scheme, where reorder shuffled the index array, not the folders). This walks
// the index array in order, assigns each chapter a fresh ascending order-key, and
// `renamePath`s its folder to `chapter-<newKey>` (rewriting the sidecar slug + every
// embedded scene metadataPath/contentPath), so that afterwards the folder-key sort equals
// the index-array order and the filesystem becomes the source of truth (B3).
//
// It is a NO-OP when the folder-key sort already equals the index-array order — which is
// the case for any project already created/reordered under the new scheme, and for a
// legacy project whose numeric folder order already matches its reading order. So this is
// safe (and cheap) to call on every open. Idempotent and resumable: interrupted mid-way,
// a re-run completes it (it only renames folders still out of position, and never
// clobbers because new keys are unique). Returns true if any folder was renamed.
Result<bool> migrateChapterOrderKeys(
    FileSystem& fs, const AbsolutePath& projectRoot);

} // namespace scrivi::manuscript
