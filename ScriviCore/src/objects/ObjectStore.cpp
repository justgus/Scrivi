#include "objects/ObjectStore.hpp"

#include "domain/Slug.hpp"
#include "schemas/ObjectJson.hpp"
#include "util/AtomicWrite.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::objects {

ObjectStore::ObjectStore(CoreServices& services)
    : services_(services) {}

AbsolutePath ObjectStore::kindDir(const AbsolutePath& projectRoot,
                                   ObjectKind kind) 
{
    return util::join(util::join(projectRoot, "objects"),
                      objectKindSubdir(kind));
}

Result<AbsolutePath> ObjectStore::findByID(const AbsolutePath& projectRoot,
                                            ObjectKind kind,
                                            const ObjectID& id) const
{
    auto& fs  = *services_.fileSystem;
    auto  dir = kindDir(projectRoot, kind);

    auto existsR = fs.exists(dir);
    if (!existsR.ok()) { return Result<AbsolutePath>::failure(existsR.error());
}
    if (!existsR.value()) {
        return Result<AbsolutePath>::failure(
            {.code=ErrorCode::ioError, .message=objectKindSubdir(kind) + " directory not found"});
}

    auto listR = fs.listDirectory(dir);
    if (!listR.ok()) { return Result<AbsolutePath>::failure(listR.error());
}

    for (const auto& entry : listR.value()) {
        if (util::extension(entry) != ".json") { continue;
}
        auto textR = fs.readTextFile(entry);
        if (!textR.ok()) { continue;
}
        auto parseR = schemas::parseWorldObject(textR.value(), kind);
        if (!parseR.ok()) { continue;
}
        if (worldObjectFields(parseR.value()).objectID.value == id.value) {
            return Result<AbsolutePath>::success(entry);
}
    }

    return Result<AbsolutePath>::failure(
        {.code=ErrorCode::ioError, .message="object not found: " + id.value});
}

// ---------------------------------------------------------------------------
// create
// ---------------------------------------------------------------------------

Result<CreateObjectResult> ObjectStore::create(const CreateObjectRequest& request) const
{
    auto& fs    = *services_.fileSystem;
    auto& uuid  = *services_.uuidProvider;
    auto& clock = *services_.clock;

    Slug slug = request.slug.empty()
        ? util::makeSlug(request.displayName)
        : request.slug;

    if (slug.empty()) {
        return Result<CreateObjectResult>::failure(
            {.code=ErrorCode::invalidArgument, .message="could not derive slug from displayName"});
}

    auto dir = kindDir(request.projectRootPath, request.objectKind);
    if (auto r = fs.createDirectories(dir); !r.ok()) {
        return Result<CreateObjectResult>::failure(r.error());
}

    auto destPath = util::join(dir, slug + ".json");
    auto existsR  = fs.exists(destPath);
    if (!existsR.ok()) { return Result<CreateObjectResult>::failure(existsR.error());
}
    if (existsR.value()) {
        return Result<CreateObjectResult>::failure(
            {.code=ErrorCode::invalidArgument,
             .message="an object with slug '" + slug + "' already exists"});
}

    WorldObjectFields fields;
    fields.objectID.value         = uuid.newObjectID().value;
    fields.slug                   = slug;
    fields.displayName            = request.displayName;
    fields.status                 = "active";
    fields.createdAt              = clock.nowUTC();
    fields.createdByIdentityID    = request.author.identityID.value;
    fields.createdByPersonaID     = request.author.personaID.value;
    fields.createdByDisplayName   = request.author.displayName;
    fields.modifiedAt             = fields.createdAt;
    fields.modifiedByIdentityID   = fields.createdByIdentityID;
    fields.modifiedByPersonaID    = fields.createdByPersonaID;
    fields.modifiedByDisplayName  = fields.createdByDisplayName;

    // Build the correctly-typed WorldObject and serialize it.
    WorldObject obj;
    switch (request.objectKind) {
        case ObjectKind::character: { CharacterObject t; static_cast<WorldObjectFields&>(t) = fields; obj = std::move(t); break; }
        case ObjectKind::location:  { LocationObject  t; static_cast<WorldObjectFields&>(t) = fields; obj = std::move(t); break; }
        case ObjectKind::item:      { ItemObject      t; static_cast<WorldObjectFields&>(t) = fields; obj = std::move(t); break; }
        case ObjectKind::rule:      { RuleObject      t; static_cast<WorldObjectFields&>(t) = fields; obj = std::move(t); break; }
        case ObjectKind::timeline:  { TimelineObject  t; static_cast<WorldObjectFields&>(t) = fields; obj = std::move(t); break; }
    }

    auto json = schemas::serializeWorldObject(obj);
    if (auto r = fs.atomicWriteTextFile(destPath, json); !r.ok()) {
        return Result<CreateObjectResult>::failure(r.error());
}

    CreateObjectResult result;
    result.objectID = fields.objectID;
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

    auto pathR = findByID(request.projectRootPath, request.objectKind, request.objectID);
    if (!pathR.ok()) { return Result<OpenObjectResult>::failure(pathR.error());
}

    auto textR = fs.readTextFile(pathR.value());
    if (!textR.ok()) { return Result<OpenObjectResult>::failure(textR.error());
}

    auto parseR = schemas::parseWorldObject(textR.value(), request.objectKind);
    if (!parseR.ok()) { return Result<OpenObjectResult>::failure(parseR.error());
}

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

    const auto& fields = worldObjectFields(request.object);
    const auto  kind   = std::visit([](const auto& o) -> ObjectKind {
        using T = std::decay_t<decltype(o)>;
        if constexpr (std::is_same_v<T, CharacterObject>) { return ObjectKind::character;
        } else if constexpr (std::is_same_v<T, LocationObject>) {  return ObjectKind::location;
        } else if constexpr (std::is_same_v<T, ItemObject>) {      return ObjectKind::item;
        } else if constexpr (std::is_same_v<T, RuleObject>) {      return ObjectKind::rule;
        } else {                                                    return ObjectKind::timeline;
}
    }, request.object);

    auto pathR = findByID(request.projectRootPath, kind, fields.objectID);
    if (!pathR.ok()) { return Result<SaveObjectResult>::failure(pathR.error());
}

    const auto& destPath = pathR.value();

    // Best-effort backup before overwriting
    auto textR = fs.readTextFile(destPath);
    if (textR.ok()) {
        (void)fs.atomicWriteTextFile(destPath + ".bak", textR.value());
}

    // Apply updated timestamps by mutating the concrete alternative via visit
    auto now = clock.nowUTC();
    WorldObject updated = request.object;
    std::visit([&](auto& o) {
        o.modifiedAt             = now;
        o.modifiedByIdentityID   = request.author.identityID.value;
        o.modifiedByPersonaID    = request.author.personaID.value;
        o.modifiedByDisplayName  = request.author.displayName;
    }, updated);

    auto json = schemas::serializeWorldObject(updated);
    if (auto r = fs.atomicWriteTextFile(destPath, json); !r.ok()) {
        return Result<SaveObjectResult>::failure(r.error());
}

    SaveObjectResult result;
    result.objectID = fields.objectID;
    result.saved    = true;
    return Result<SaveObjectResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// remove
// ---------------------------------------------------------------------------

Result<DeleteObjectResult> ObjectStore::remove(const DeleteObjectRequest& request)
{
    auto& fs = *services_.fileSystem;

    auto pathR = findByID(request.projectRootPath, request.objectKind, request.objectID);
    if (!pathR.ok()) { return Result<DeleteObjectResult>::failure(pathR.error());
}

    if (auto r = fs.removeFile(pathR.value()); !r.ok()) {
        return Result<DeleteObjectResult>::failure(r.error());
}

    DeleteObjectResult result;
    result.objectID = request.objectID;
    result.deleted  = true;
    return Result<DeleteObjectResult>::success(std::move(result));
}

} // namespace scrivi::objects
