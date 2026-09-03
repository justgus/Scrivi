#pragma once

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

#include <optional>
#include <string>
#include <vector>

namespace scrivi::objects {

// One row of objects/index.json.
struct ObjectIndexEntry {
    ObjectID    objectID;
    ObjectKind  kind = ObjectKind::character;
    Slug        slug;
    std::string displayName;
    std::string worldID;    // empty for project-scoped objects

    // T-0446 (SP-119): the object's image, so a card list knows an object HAS one
    // without opening it — D8's thumbnails would otherwise cost one file read per
    // row in a 280pt pane, and an image may live on a slow or absent volume.
    //
    // ⚠️ IDs ONLY — deliberately NOT a resolved path.
    //
    // Assets live at `assets/<category>/<filename>`, keyed by FILENAME, so an
    // assetID resolves to a path only by reading the asset sidecars. Doing that
    // during a rebuild would (a) persist a VOLUME-DEPENDENT value that goes stale
    // the moment the drive mounts elsewhere, and (b) make rebuilding the object
    // index depend on the assets directory being readable. ⚠️ **The index is a
    // mirror of the object files and nothing else.**
    //
    // ✅ RULED 2026-08-23: resolution happens at LIST time — `scrivi_list_objects`
    // scans assets ONCE per call and joins. One scan per list, not per row.
    std::string imageAssetID;
    std::string imageThumbnailAssetID;   // optional; preferred for a card row
};

// objects/index.json — the ID→object lookup backing findByID.
//
// A DERIVED CACHE, NEVER AUTHORITATIVE (Worldbuilding Object Model v0.2 §4.2).
// The <slug>.json files are the truth. When the index is missing, corrupt, or
// disagrees with disk, it is rebuilt by scanning objects/ — the same
// repair-before-validation pattern EP-027 used for scene identity. A
// hand-edited file or a resolved merge conflict costs one rebuild, never data.
//
// It is load-bearing rather than merely an optimization: EP-031's edges store
// bare {id} endpoints (v0.2 §5.2), so ID→kind resolution has no other source.
class ObjectIndex {
public:
    explicit ObjectIndex(CoreServices& services);

    // Absolute path of objects/index.json for a project.
    [[nodiscard]] static AbsolutePath indexPath(const AbsolutePath& projectRoot);

    // A world package keeps its own index at <package>/index.json, using the
    // SAME schema (Doc 3 §6.1), so a world is self-contained and its index
    // rebuilds by the same scan. These operate on that file instead.
    // ⚠️ I-0183 — `indeterminate`, when given, reports that the package's object
    // set could NOT be positively established: the index was unreadable AND the
    // directory scan hit I/O errors. ⚠️ An empty result with `indeterminate ==
    // true` means "I could not read this world", NEVER "this world is empty" —
    // and a caller that prunes must treat the two differently or it destroys
    // relationships into a world that is merely unreadable.
    [[nodiscard]] Result<std::vector<ObjectIndexEntry>> loadWorldIndex(
        const AbsolutePath& packagePath, bool* indeterminate = nullptr) const;
    [[nodiscard]] Result<void> upsertWorld(const AbsolutePath& packagePath,
                                           const ObjectIndexEntry& entry) const;
    [[nodiscard]] Result<void> eraseWorld(const AbsolutePath& packagePath,
                                          const ObjectID& id) const;

    // Loads the index, rebuilding from a directory scan if it is missing,
    // unreadable, malformed, or carries the wrong schema tag. Never fails
    // because of index contents — only a scan that cannot read objects/ fails.
    [[nodiscard]] Result<std::vector<ObjectIndexEntry>> load(
        const AbsolutePath& projectRoot) const;

    // Rebuilds by scanning every project-scoped kind directory, then writes
    // the result atomically. This is the repair path AND the initial build.
    // Scans a directory tree for object files (SP-103). `worldScoped` selects
    // which kinds may live there; shared by the project and world rebuilds.
    // ⚠️ I-0183: `sawIOError`, when given, is set true if ANY kind directory or
    // object file could not be read. The scan is best-effort by design (one bad
    // file must not cost the index) — ⚠️ but that same tolerance makes an
    // UNREADABLE tree scan as an EMPTY one, so callers that draw conclusions
    // from emptiness must ask.
    [[nodiscard]] std::vector<ObjectIndexEntry> scanDir(
        const AbsolutePath& baseDir, bool worldScoped,
        bool* sawIOError = nullptr) const;

    [[nodiscard]] Result<void> writeWorldIndex(
        const AbsolutePath& packagePath,
        const std::vector<ObjectIndexEntry>& entries) const;

    [[nodiscard]] Result<std::vector<ObjectIndexEntry>> rebuild(
        const AbsolutePath& projectRoot) const;

    // Writes entries to objects/index.json atomically.
    [[nodiscard]] Result<void> write(const AbsolutePath& projectRoot,
                                     const std::vector<ObjectIndexEntry>& entries) const;

    // Resolves an objectID through the index, falling back to a rebuild when
    // the entry is absent (the index may simply be behind disk).
    [[nodiscard]] Result<ObjectIndexEntry> find(const AbsolutePath& projectRoot,
                                                const ObjectID& id) const;

    // Every object the project can currently SEE: its own index plus the index
    // of each bound world that resolves (SP-098 T-0378).
    //
    // A world that is unavailable contributes nothing — deliberately. Listing an
    // object the writer cannot open would be worse than omitting it, and the
    // binding's cachedIndex exists to name PENDING EDGES, not to fake presence.
    // Absence from this listing therefore means "not visible right now", never
    // "deleted".
    [[nodiscard]] Result<std::vector<ObjectIndexEntry>> loadAllVisible(
        const AbsolutePath& projectRoot) const;

    // Mutations. Each reloads, applies, and rewrites atomically — callers run
    // these AFTER the object file write succeeds, never before: an entry for a
    // file that failed to write is silently wrong, whereas a missing entry for
    // a file that did write is repaired by the next rebuild.
    [[nodiscard]] Result<void> upsert(const AbsolutePath& projectRoot,
                                      const ObjectIndexEntry& entry) const;
    [[nodiscard]] Result<void> erase(const AbsolutePath& projectRoot,
                                     const ObjectID& id) const;

private:
    CoreServices& services_;

    // Parses index.json. Returns nullopt for any unusable content, which the
    // caller treats as "rebuild" rather than as an error.
    [[nodiscard]] std::optional<std::vector<ObjectIndexEntry>> parse(
        std::string_view json) const;
};

} // namespace scrivi::objects
