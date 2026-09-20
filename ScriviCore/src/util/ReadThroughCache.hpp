#pragma once

// SP-144 / [I-0231] — a read-through file cache scoped to ONE core call.
//
// ⚠️ THE DEFECT THIS REMOVES, AS MEASURED (SP-144 AC1, not inferred).
// `ProjectOpener::open` runs six passes over the same manuscript, and each one
// re-reads sidecars the previous pass already read:
//
//     1-migrateChapterOrderKeys     :  12 calls
//     2-rebuildIndexIfInconsistent  :  12 calls
//     3-migrateScenes               : 241 calls  (210 reads)
//     4-relationTypes+repairDangling:   3 calls
//     5-validate                    : 203 calls  ( 71 reads + 132 exists)
//     6-resolveOrder                : 151 calls  (140 reads)
//
// ⚠️ Every chapter sidecar was read 8x and every scene sidecar 6x per open, for
// bytes that DO NOT CHANGE between the first read and the last. On local disk
// the page cache hides this (0.21 s); on a `cache=none` CIFS mount each repeat
// is a network round-trip, which is how one open reached 154 s.
//
// ✅ WHAT THIS IS. A cache that sits in front of `FileSystem` for the duration of
// ONE call, so the second and subsequent reads of an unchanged path are served
// from memory. ⛔ IT IS NOT A PERSISTENT CACHE and never outlives the call that
// creates it — there is no filesystem watching on any platform, so a cache that
// survived a call could not know when it went stale.
//
// ⚠️ THE RULE IT INHERITS, AND MUST NOT BREAK (EP-027, and `ProjectIndex`'s
// header states it for the location index): THE FILESYSTEM IS AUTHORITATIVE.
// This cache is DERIVED and DISPOSABLE. It is therefore:
//
//   ✅ WRITE-INVALIDATING — any write, rename, or removal through this wrapper
//      drops what it affects. A pass that REPAIRS a sidecar (migrateScenes
//      rewrites a stale `content.path`) must have its next read see the NEW
//      bytes, or the repair would be silently undone by a stale hit.
//   ✅ NEGATIVE-SAFE for reads — a read that FAILS is not cached, so a transient
//      I/O error is retried rather than remembered as a verdict.
//   ⚠️ ABSENCE-CACHING ONLY WITHIN A CALL, and only for `exists`. That is what
//      AC3 needs (188 ENOENT probes of one absent `binding.json` → 1), and it is
//      safe ONLY because a write through this wrapper invalidates the entry.
//      ⛔ *Absence is never deletion* ([I-0183]) — this caches a probe RESULT for
//      the length of one call; it never lets a caller conclude a file was
//      deleted.
//
// ⚠️ WHY A DECORATOR AND NOT AN EDIT TO EACH PASS. The passes are correct: each
// one genuinely needs the sidecar it reads, and each is independently testable
// and independently meaningful. Threading a shared parse result through six of
// them would couple them to each other and to their order — the coupling that
// [I-0196]'s quadratic came from. ✅ The redundancy is in the REPETITION, not in
// any one pass, so the fix belongs where the repetition is visible.

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "scrivi/Services.hpp"

namespace scrivi::util {

class ReadThroughCache final : public FileSystem {
public:
    explicit ReadThroughCache(FileSystem& inner) : inner_(inner) {}

    // --- cached: reads and existence probes --------------------------------

    Result<Utf8Text> readTextFile(const AbsolutePath& path) override {
        if (auto it = textCache_.find(path); it != textCache_.end()) {
            return Result<Utf8Text>::success(it->second);
        }
        auto r = inner_.readTextFile(path);
        // ⚠️ Only a SUCCESS is cached. Caching a failure would turn one transient
        // error into a verdict for the rest of the call.
        if (r.ok()) {
            textCache_.emplace(path, r.value());
            existsCache_[path] = true;   // it was read, so it exists
        }
        return r;
    }

    Result<bool> exists(const AbsolutePath& path) override {
        if (auto it = existsCache_.find(path); it != existsCache_.end()) {
            return Result<bool>::success(it->second);
        }
        auto r = inner_.exists(path);
        if (r.ok()) { existsCache_[path] = r.value(); }
        return r;
    }

    Result<bool> isDirectory(const AbsolutePath& path) override {
        if (auto it = isDirCache_.find(path); it != isDirCache_.end()) {
            return Result<bool>::success(it->second);
        }
        auto r = inner_.isDirectory(path);
        if (r.ok()) { isDirCache_[path] = r.value(); }
        return r;
    }

    Result<std::vector<AbsolutePath>> listDirectory(const AbsolutePath& path) override {
        if (auto it = listCache_.find(path); it != listCache_.end()) {
            return Result<std::vector<AbsolutePath>>::success(it->second);
        }
        auto r = inner_.listDirectory(path);
        if (r.ok()) { listCache_.emplace(path, r.value()); }
        return r;
    }

    // ⚠️ NOT CACHED. `deviceID` is how [I-0181]'s "is this still a mount point"
    // question is asked, and the whole point of that question is that the answer
    // can change underfoot. ✅ It is also called a handful of times per open, so
    // it is not where the cost is.
    Result<std::uint64_t> deviceID(const AbsolutePath& path) override {
        return inner_.deviceID(path);
    }

    // --- mutating: forward, then INVALIDATE --------------------------------
    //
    // ⚠️ Invalidation is deliberately BLUNT. A write to any path drops that
    // path's entries and every directory listing, because a create or remove
    // changes a listing this cache may already hold. ✅ Over-invalidating costs a
    // re-read; under-invalidating serves stale bytes and silently reverts a
    // repair, which is the failure that must not happen.

    Result<void> atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text) override {
        auto r = inner_.atomicWriteTextFile(path, utf8Text);
        invalidatePath(path);
        invalidateAllListings();
        return r;
    }

    Result<void> createFileExclusive(const AbsolutePath& path, std::string_view utf8Text) override {
        auto r = inner_.createFileExclusive(path, utf8Text);
        invalidatePath(path);
        invalidateAllListings();
        return r;
    }

    Result<void> appendTextFile(const AbsolutePath& path, std::string_view utf8Text) override {
        auto r = inner_.appendTextFile(path, utf8Text);
        invalidatePath(path);
        invalidateAllListings();
        return r;
    }

    Result<void> removeFile(const AbsolutePath& path) override {
        auto r = inner_.removeFile(path);
        invalidatePath(path);
        invalidateAllListings();
        return r;
    }

    Result<void> createDirectories(const AbsolutePath& path) override {
        auto r = inner_.createDirectories(path);
        invalidatePath(path);
        invalidateAllListings();
        return r;
    }

    // ⚠️ A rename moves BOTH endpoints — the source ceases to exist and the
    // destination begins to. Both must be dropped, and this is exactly the call
    // `migrateScenes` makes when it reslugs a scene.
    Result<void> renamePath(const AbsolutePath& from, const AbsolutePath& to) override {
        auto r = inner_.renamePath(from, to);
        invalidatePath(from);
        invalidatePath(to);
        invalidateAllListings();
        return r;
    }

    Result<void> copyFileInBlocks(const AbsolutePath& from,
                                  const AbsolutePath& to,
                                  std::size_t blockSize,
                                  const std::function<Result<void>()>& onBlock) override {
        auto r = inner_.copyFileInBlocks(from, to, blockSize, onBlock);
        invalidatePath(to);
        invalidateAllListings();
        return r;
    }

private:
    void invalidatePath(const AbsolutePath& path) {
        textCache_.erase(path);
        existsCache_.erase(path);
        isDirCache_.erase(path);
        listCache_.erase(path);
    }

    void invalidateAllListings() { listCache_.clear(); }

    FileSystem& inner_;

    std::unordered_map<std::string, Utf8Text>                  textCache_;
    std::unordered_map<std::string, bool>                      existsCache_;
    std::unordered_map<std::string, bool>                      isDirCache_;
    std::unordered_map<std::string, std::vector<AbsolutePath>> listCache_;
};

} // namespace scrivi::util
