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
    [[nodiscard]] Result<std::vector<ObjectIndexEntry>> loadWorldIndex(
        const AbsolutePath& packagePath) const;
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
    [[nodiscard]] Result<std::vector<ObjectIndexEntry>> rebuild(
        const AbsolutePath& projectRoot) const;

    // Writes entries to objects/index.json atomically.
    [[nodiscard]] Result<void> write(const AbsolutePath& projectRoot,
                                     const std::vector<ObjectIndexEntry>& entries) const;

    // Resolves an objectID through the index, falling back to a rebuild when
    // the entry is absent (the index may simply be behind disk).
    [[nodiscard]] Result<ObjectIndexEntry> find(const AbsolutePath& projectRoot,
                                                const ObjectID& id) const;

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
