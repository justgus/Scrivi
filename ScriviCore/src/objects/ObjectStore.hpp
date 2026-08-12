#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::objects {

// Handles world-object CRUD against objects/<kind>/ in the project package.
// Dispatches to the correct subdirectory and schema based on ObjectKind.
class ObjectStore {
public:
    explicit ObjectStore(CoreServices& services);

    [[nodiscard]] Result<CreateObjectResult> create(const CreateObjectRequest& request) const;
    Result<OpenObjectResult>   open(const OpenObjectRequest& request);
    Result<SaveObjectResult>   save(const SaveObjectRequest& request);
    Result<DeleteObjectResult> remove(const DeleteObjectRequest& request);

private:
    CoreServices& services_;

    [[nodiscard]] static AbsolutePath kindDir(const AbsolutePath& projectRoot, ObjectKind kind);

    // Rejects kinds that cannot be stored under objects/ in this project:
    // world-scoped kinds (no world package until SP-098) and the `world`
    // container kind (created by scrivi_create_world, not here).
    [[nodiscard]] static Result<void> checkKindStorable(ObjectKind kind);

    // Resolves an object's file path via objects/index.json, falling back to a
    // directory scan only when the index cannot answer (ObjectIndex::find
    // rebuilds internally). `kind` is advisory — the index is authoritative on
    // ID→kind, which is what lets SP-096 store bare {id} edge endpoints.
    [[nodiscard]] Result<AbsolutePath> findByID(const AbsolutePath& projectRoot,
                                                 ObjectKind kind,
                                                 const ObjectID& id) const;

    // Legacy directory scan. Retained as the resolution path for objects the
    // index cannot place (e.g. a slug renamed on disk under a kind whose
    // rebuild skipped an unparseable sibling).
    [[nodiscard]] Result<AbsolutePath> scanForID(const AbsolutePath& projectRoot,
                                                  ObjectKind kind,
                                                  const ObjectID& id) const;
};

} // namespace scrivi::objects
