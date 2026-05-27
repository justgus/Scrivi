#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::objects {

// Handles character object CRUD against objects/characters/ in the project package.
class ObjectStore {
public:
    explicit ObjectStore(CoreServices& services);

    Result<CreateObjectResult> create(const CreateObjectRequest& request);
    Result<OpenObjectResult>   open(const OpenObjectRequest& request);
    Result<SaveObjectResult>   save(const SaveObjectRequest& request);
    Result<DeleteObjectResult> remove(const DeleteObjectRequest& request);

private:
    CoreServices& services_;

    // Returns the absolute path to the characters directory.
    AbsolutePath charsDir(const AbsolutePath& projectRoot) const;

    // Scans charsDir to find the .json file whose parsed objectID matches id.
    // Returns notFound if no match.
    Result<AbsolutePath> findByID(const AbsolutePath& projectRoot,
                                   const ObjectID& id) const;
};

} // namespace scrivi::objects
