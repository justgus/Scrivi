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

} // namespace scrivi::manuscript
