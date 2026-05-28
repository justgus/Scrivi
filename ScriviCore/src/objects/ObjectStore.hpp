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

    Result<CreateObjectResult> create(const CreateObjectRequest& request);
    Result<OpenObjectResult>   open(const OpenObjectRequest& request);
    Result<SaveObjectResult>   save(const SaveObjectRequest& request);
    Result<DeleteObjectResult> remove(const DeleteObjectRequest& request);

private:
    CoreServices& services_;

    AbsolutePath kindDir(const AbsolutePath& projectRoot, ObjectKind kind) const;

    // Scans kindDir to find the .json file whose parsed objectID matches id.
    Result<AbsolutePath> findByID(const AbsolutePath& projectRoot,
                                   ObjectKind kind,
                                   const ObjectID& id) const;
};

} // namespace scrivi::objects
