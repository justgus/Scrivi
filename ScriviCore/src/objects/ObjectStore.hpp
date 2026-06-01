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

    // Scans kindDir to find the .json file whose parsed objectID matches id.
    [[nodiscard]] Result<AbsolutePath> findByID(const AbsolutePath& projectRoot,
                                                 ObjectKind kind,
                                                 const ObjectID& id) const;
};

} // namespace scrivi::objects
