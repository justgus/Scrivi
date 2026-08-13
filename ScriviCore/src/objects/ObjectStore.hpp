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

    // Promotion / demotion between scopes (SP-098 T-0379; §3.1).
    //
    //   item → artifact   moves objects/items/<slug>.json into the world's
    //                     artifacts/ and SETS worldID
    //   artifact → item   the exact inverse, through this same call
    //
    // ⚠️ `objectID` IS PRESERVED AND NO EDGE IS TOUCHED. This is the safety proof
    // for the bare-endpoint ruling (§5.2): endpoints store `{id}` with no `kind`
    // precisely so that promotion cannot stale them. An endpoint recording
    // `{kind:"item", id:…}` would go wrong on every edge the moment the object
    // changed kind; here the edge log is not even opened.
    struct PromoteResult {
        ObjectID     objectID;
        ObjectKind   kind = ObjectKind::item;
        std::string  worldID;      // empty after demotion
        AbsolutePath fromPath;
        AbsolutePath toPath;
    };
    [[nodiscard]] Result<PromoteResult> promote(const AbsolutePath& projectRoot,
                                                 const ObjectID& objectID,
                                                 ObjectKind targetKind,
                                                 const std::string& worldID) const;

private:
    CoreServices& services_;

    // The directory a kind's files live in. PROJECT-scoped kinds resolve under
    // <projectRoot>/objects/<subdir>/; WORLD-scoped kinds resolve inside the
    // bound world package (SP-097), never under objects/.
    [[nodiscard]] Result<AbsolutePath> kindDirFor(const AbsolutePath& projectRoot,
                                                   ObjectKind kind,
                                                   const std::string& worldID) const;

    // Project-scoped directory only. Retained for the callers that never touch
    // world scope (the index rebuild scan).
    [[nodiscard]] static AbsolutePath kindDir(const AbsolutePath& projectRoot, ObjectKind kind);

    // Rejects kinds that cannot be stored at all: the `world` container kind
    // (created by scrivi_create_world, not here). World-scoped kinds are no
    // longer refused outright — since SP-097 they are storable given a bound
    // world, which kindDirFor enforces.
    [[nodiscard]] static Result<void> checkKindStorable(ObjectKind kind);

    // Resolves an object's file path via objects/index.json, falling back to a
    // directory scan only when the index cannot answer (ObjectIndex::find
    // rebuilds internally). `kind` is advisory — the index is authoritative on
    // ID→kind, which is what lets SP-096 store bare {id} edge endpoints.
    // `worldID` is a fallback only — the index carries the object's own world,
    // which wins when present.
    [[nodiscard]] Result<AbsolutePath> findByID(const AbsolutePath& projectRoot,
                                                 ObjectKind kind,
                                                 const ObjectID& id,
                                                 const std::string& worldID = {}) const;

    // Legacy directory scan. Retained as the resolution path for objects the
    // index cannot place (e.g. a slug renamed on disk under a kind whose
    // rebuild skipped an unparseable sibling).
    [[nodiscard]] Result<AbsolutePath> scanForID(const AbsolutePath& projectRoot,
                                                  ObjectKind kind,
                                                  const ObjectID& id) const;

    // The scan itself, against an explicit directory — shared by the project
    // and world paths so there is one implementation, not two.
    [[nodiscard]] Result<AbsolutePath> scanDirForID(const AbsolutePath& dir,
                                                     ObjectKind kind,
                                                     const ObjectID& id) const;
};

} // namespace scrivi::objects
