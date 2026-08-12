#include "objects/ObjectStore.hpp"

#include "domain/Slug.hpp"
#include "objects/ObjectIndex.hpp"
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

Result<void> ObjectStore::checkKindStorable(ObjectKind kind) {
    // World-scoped kinds are declared in ObjectKind so the index and the
    // relationship graph know the full kind set, but they have nowhere to live
    // until SP-098 delivers world packages. Refusing here is deliberate: World
    // Data Separation v0.1 §7 writes no migration code, so an artifact created
    // under objects/ today could never be moved to world scope later.
    if (objectKindIsWorldScoped(kind)) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "kind '" + objectKindName(kind) +
                        "' is world-scoped; a world is required (available in SP-098)"});
    }

    if (kind == ObjectKind::world) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "kind 'world' is a container; create it with scrivi_create_world"});
    }

    return Result<void>::success();
}

Result<AbsolutePath> ObjectStore::scanForID(const AbsolutePath& projectRoot,
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

Result<AbsolutePath> ObjectStore::findByID(const AbsolutePath& projectRoot,
                                            ObjectKind kind,
                                            const ObjectID& id) const
{
    auto& fs = *services_.fileSystem;

    // Index first — one lookup instead of a directory listing with a JSON
    // parse per file (Worldbuilding Object Model v0.2 §4.2).
    ObjectIndex index{services_};
    if (auto entryR = index.find(projectRoot, id); entryR.ok()) {
        const auto& entry = entryR.value();
        auto path = util::join(kindDir(projectRoot, entry.kind), entry.slug + ".json");

        // The index names a slug; confirm the file is actually there before
        // trusting it. A stale slug falls through to the scan rather than
        // returning a path that does not exist.
        auto existsR = fs.exists(path);
        if (existsR.ok() && existsR.value()) {
            return Result<AbsolutePath>::success(std::move(path));
        }
    }

    return scanForID(projectRoot, kind, id);
}

// ---------------------------------------------------------------------------
// create
// ---------------------------------------------------------------------------

Result<CreateObjectResult> ObjectStore::create(const CreateObjectRequest& request) const
{
    auto& fs    = *services_.fileSystem;
    auto& uuid  = *services_.uuidProvider;
    auto& clock = *services_.clock;

    if (auto r = checkKindStorable(request.objectKind); !r.ok()) {
        return Result<CreateObjectResult>::failure(r.error());
}

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
    {
        auto typed = schemas::makeWorldObject(request.objectKind, std::move(fields));
        obj = std::move(typed);
    }
    const auto& stored = worldObjectFields(obj);

    auto json = schemas::serializeWorldObject(obj);
    if (auto r = fs.atomicWriteTextFile(destPath, json); !r.ok()) {
        return Result<CreateObjectResult>::failure(r.error());
}

    // Index AFTER the object write succeeds — an entry for a file that never
    // landed is silently wrong, while a missing entry self-heals on rebuild.
    ObjectIndex index{services_};
    ObjectIndexEntry entry;
    entry.objectID    = stored.objectID;
    entry.kind        = request.objectKind;
    entry.slug        = stored.slug;
    entry.displayName = stored.displayName;
    entry.worldID     = stored.worldID;
    (void)index.upsert(request.projectRootPath, entry);

    CreateObjectResult result;
    result.objectID = stored.objectID;
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

    if (auto r = checkKindStorable(request.objectKind); !r.ok()) {
        return Result<OpenObjectResult>::failure(r.error());
}

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
    const auto  kind   = worldObjectKind(request.object);

    if (auto r = checkKindStorable(kind); !r.ok()) {
        return Result<SaveObjectResult>::failure(r.error());
}

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

    // Refresh the index — displayName and worldID are both editable, and both
    // are carried in the index for the inspector's benefit.
    ObjectIndex index{services_};
    const auto& saved = worldObjectFields(updated);
    ObjectIndexEntry entry;
    entry.objectID    = saved.objectID;
    entry.kind        = kind;
    entry.slug        = saved.slug;
    entry.displayName = saved.displayName;
    entry.worldID     = saved.worldID;
    (void)index.upsert(request.projectRootPath, entry);

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

    if (auto r = checkKindStorable(request.objectKind); !r.ok()) {
        return Result<DeleteObjectResult>::failure(r.error());
}

    auto pathR = findByID(request.projectRootPath, request.objectKind, request.objectID);
    if (!pathR.ok()) { return Result<DeleteObjectResult>::failure(pathR.error());
}

    if (auto r = fs.removeFile(pathR.value()); !r.ok()) {
        return Result<DeleteObjectResult>::failure(r.error());
}

    ObjectIndex index{services_};
    (void)index.erase(request.projectRootPath, request.objectID);

    DeleteObjectResult result;
    result.objectID = request.objectID;
    result.deleted  = true;
    return Result<DeleteObjectResult>::success(std::move(result));
}

} // namespace scrivi::objects
