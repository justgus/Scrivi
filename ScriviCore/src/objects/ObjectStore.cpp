#include "objects/ObjectStore.hpp"

#include "domain/Slug.hpp"
#include "schemas/ObjectJson.hpp"
#include "util/AtomicWrite.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::objects {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static Result<void> internalError(std::string msg) {
    return Result<void>::failure({ErrorCode::internalError, std::move(msg)});
}

ObjectStore::ObjectStore(CoreServices& services)
    : services_(services) {}

AbsolutePath ObjectStore::charsDir(const AbsolutePath& projectRoot) const {
    return util::join(util::join(projectRoot, "objects"), "characters");
}

Result<AbsolutePath> ObjectStore::findByID(const AbsolutePath& projectRoot,
                                            const ObjectID&     id) const
{
    auto& fs   = *services_.fileSystem;
    auto  dir  = charsDir(projectRoot);

    auto existsR = fs.exists(dir);
    if (!existsR.ok()) return Result<AbsolutePath>::failure(existsR.error());
    if (!existsR.value())
        return Result<AbsolutePath>::failure({ErrorCode::ioError, "characters directory not found"});

    auto listR = fs.listDirectory(dir);
    if (!listR.ok()) return Result<AbsolutePath>::failure(listR.error());

    for (const auto& entry : listR.value()) {
        if (util::extension(entry) != ".json") continue;
        auto textR = fs.readTextFile(entry);
        if (!textR.ok()) continue;
        auto parseR = schemas::parseCharacter(textR.value());
        if (!parseR.ok()) continue;
        if (parseR.value().objectID.value == id.value)
            return Result<AbsolutePath>::success(entry);
    }

    return Result<AbsolutePath>::failure({ErrorCode::ioError,
        "character not found: " + id.value});
}

// ---------------------------------------------------------------------------
// create
// ---------------------------------------------------------------------------

Result<CreateObjectResult> ObjectStore::create(const CreateObjectRequest& request)
{
    auto& fs    = *services_.fileSystem;
    auto& uuid  = *services_.uuidProvider;
    auto& clock = *services_.clock;

    // Derive slug
    Slug slug = request.slug.empty()
        ? util::makeSlug(request.displayName)
        : request.slug;

    if (slug.empty())
        return Result<CreateObjectResult>::failure(
            {ErrorCode::invalidArgument, "could not derive slug from displayName"});

    // Ensure characters directory exists
    auto dir = charsDir(request.projectRootPath);
    if (auto r = fs.createDirectories(dir); !r.ok())
        return Result<CreateObjectResult>::failure(r.error());

    // Check slug uniqueness
    auto destPath = util::join(dir, slug + ".json");
    auto existsR  = fs.exists(destPath);
    if (!existsR.ok()) return Result<CreateObjectResult>::failure(existsR.error());
    if (existsR.value())
        return Result<CreateObjectResult>::failure(
            {ErrorCode::invalidArgument, "a character with slug '" + slug + "' already exists"});

    // Build object
    CharacterObject obj;
    obj.objectID.value          = uuid.newObjectID().value;
    obj.slug                    = slug;
    obj.displayName             = request.displayName;
    obj.status                  = "active";
    obj.createdAt               = clock.nowUTC();
    obj.createdByIdentityID     = request.author.identityID.value;
    obj.createdByPersonaID      = request.author.personaID.value;
    obj.createdByDisplayName    = request.author.displayName;
    obj.modifiedAt              = obj.createdAt;
    obj.modifiedByIdentityID    = obj.createdByIdentityID;
    obj.modifiedByPersonaID     = obj.createdByPersonaID;
    obj.modifiedByDisplayName   = obj.createdByDisplayName;

    // Write
    auto json = schemas::serializeCharacter(obj);
    if (auto r = fs.atomicWriteTextFile(destPath, json); !r.ok())
        return Result<CreateObjectResult>::failure(r.error());

    CreateObjectResult result;
    result.objectID = obj.objectID;
    result.slug     = slug;
    result.path     = destPath;
    return Result<CreateObjectResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// open
// ---------------------------------------------------------------------------

Result<OpenObjectResult> ObjectStore::open(const OpenObjectRequest& request)
{
    auto& fs = *services_.fileSystem;

    auto pathR = findByID(request.projectRootPath, request.objectID);
    if (!pathR.ok()) return Result<OpenObjectResult>::failure(pathR.error());

    auto textR = fs.readTextFile(pathR.value());
    if (!textR.ok()) return Result<OpenObjectResult>::failure(textR.error());

    auto parseR = schemas::parseCharacter(textR.value());
    if (!parseR.ok()) return Result<OpenObjectResult>::failure(parseR.error());

    OpenObjectResult result;
    result.object = std::move(parseR.value());
    result.path   = pathR.value();
    return Result<OpenObjectResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// save
// ---------------------------------------------------------------------------

Result<SaveObjectResult> ObjectStore::save(const SaveObjectRequest& request)
{
    auto& fs    = *services_.fileSystem;
    auto& clock = *services_.clock;

    auto pathR = findByID(request.projectRootPath, request.object.objectID);
    if (!pathR.ok()) return Result<SaveObjectResult>::failure(pathR.error());

    const auto& destPath = pathR.value();

    // Write .bak before overwriting
    auto textR = fs.readTextFile(destPath);
    if (textR.ok()) {
        auto bakPath = destPath + ".bak";
        (void)fs.atomicWriteTextFile(bakPath, textR.value()); // best-effort
    }

    // Apply updated timestamps
    CharacterObject updated          = request.object;
    updated.modifiedAt               = clock.nowUTC();
    updated.modifiedByIdentityID     = request.author.identityID.value;
    updated.modifiedByPersonaID      = request.author.personaID.value;
    updated.modifiedByDisplayName    = request.author.displayName;

    auto json = schemas::serializeCharacter(updated);
    if (auto r = fs.atomicWriteTextFile(destPath, json); !r.ok())
        return Result<SaveObjectResult>::failure(r.error());

    SaveObjectResult result;
    result.objectID = updated.objectID;
    result.saved    = true;
    return Result<SaveObjectResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// remove
// ---------------------------------------------------------------------------

Result<DeleteObjectResult> ObjectStore::remove(const DeleteObjectRequest& request)
{
    auto& fs = *services_.fileSystem;

    auto pathR = findByID(request.projectRootPath, request.objectID);
    if (!pathR.ok()) return Result<DeleteObjectResult>::failure(pathR.error());

    if (auto r = fs.removeFile(pathR.value()); !r.ok())
        return Result<DeleteObjectResult>::failure(r.error());

    DeleteObjectResult result;
    result.objectID = request.objectID;
    result.deleted  = true;
    return Result<DeleteObjectResult>::success(std::move(result));
}

} // namespace scrivi::objects
