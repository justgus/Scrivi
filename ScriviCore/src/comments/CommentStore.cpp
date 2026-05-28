#include "comments/CommentStore.hpp"

#include "schemas/CommentJson.hpp"
#include "util/AtomicWrite.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::comments {

CommentStore::CommentStore(CoreServices& services)
    : services_(services) {}

AbsolutePath CommentStore::threadPath(const AbsolutePath& projectRoot,
                                       const std::string& scopeKind,
                                       const std::string& targetID) const
{
    auto dir = util::join(util::join(projectRoot, "comments"), scopeKind);
    return util::join(dir, targetID + ".comments.json");
}

// ---------------------------------------------------------------------------
// Shared helper: load or create a CommentThread from disk (atomic read).
// ---------------------------------------------------------------------------

static Result<CommentThread> loadThread(scrivi::FileSystem& fs,
                                         const AbsolutePath& path,
                                         const std::string& scopeKind,
                                         const std::string& targetID)
{
    auto existsR = fs.exists(path);
    if (!existsR.ok()) return Result<CommentThread>::failure(existsR.error());

    if (!existsR.value()) {
        // Brand-new thread file.
        CommentThread t;
        t.schema    = "scrivi.comments.v1";
        t.scopeKind = scopeKind;
        t.targetID  = targetID;
        return Result<CommentThread>::success(std::move(t));
    }

    auto textR = fs.readTextFile(path);
    if (!textR.ok()) return Result<CommentThread>::failure(textR.error());
    return schemas::parseCommentThread(textR.value());
}

// ---------------------------------------------------------------------------
// add
// ---------------------------------------------------------------------------

Result<AddCommentResult> CommentStore::add(const AddCommentRequest& request)
{
    auto& fs    = *services_.fileSystem;
    auto& uuid  = *services_.uuidProvider;
    auto& clock = *services_.clock;

    auto path = threadPath(request.projectRootPath, request.scopeKind, request.targetID);

    // Ensure parent directory exists.
    auto dir = util::parent(path);
    if (auto r = fs.createDirectories(dir); !r.ok())
        return Result<AddCommentResult>::failure(r.error());

    auto threadR = loadThread(fs, path, request.scopeKind, request.targetID);
    if (!threadR.ok()) return Result<AddCommentResult>::failure(threadR.error());
    auto thread = std::move(threadR.value());

    Comment c;
    c.commentID               = uuid.newObjectID().value;
    c.body                    = request.body;
    c.resolved                = false;
    c.createdAt               = clock.nowUTC();
    c.createdByIdentityID     = request.author.identityID.value;
    c.createdByPersonaID      = request.author.personaID.value;
    c.createdByDisplayName    = request.author.displayName;

    auto commentID = c.commentID;
    thread.comments.push_back(std::move(c));

    auto json = schemas::serializeCommentThread(thread);
    if (auto r = fs.atomicWriteTextFile(path, json); !r.ok())
        return Result<AddCommentResult>::failure(r.error());

    AddCommentResult result;
    result.commentID = commentID;
    result.added     = true;
    return Result<AddCommentResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// list
// ---------------------------------------------------------------------------

Result<ListCommentsResult> CommentStore::list(const ListCommentsRequest& request)
{
    auto& fs = *services_.fileSystem;

    auto path    = threadPath(request.projectRootPath, request.scopeKind, request.targetID);
    auto threadR = loadThread(fs, path, request.scopeKind, request.targetID);
    if (!threadR.ok()) return Result<ListCommentsResult>::failure(threadR.error());

    ListCommentsResult result;
    result.comments  = std::move(threadR.value().comments);
    result.scopeKind = request.scopeKind;
    result.targetID  = request.targetID;
    return Result<ListCommentsResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// resolve
// ---------------------------------------------------------------------------

Result<ResolveCommentResult> CommentStore::resolve(const ResolveCommentRequest& request)
{
    auto& fs    = *services_.fileSystem;
    auto& clock = *services_.clock;

    auto path    = threadPath(request.projectRootPath, request.scopeKind, request.targetID);
    auto threadR = loadThread(fs, path, request.scopeKind, request.targetID);
    if (!threadR.ok()) return Result<ResolveCommentResult>::failure(threadR.error());
    auto thread = std::move(threadR.value());

    bool found = false;
    for (auto& c : thread.comments) {
        if (c.commentID != request.commentID) continue;
        c.resolved               = true;
        c.resolvedAt             = clock.nowUTC();
        c.resolvedByIdentityID   = request.resolver.identityID.value;
        c.resolvedByPersonaID    = request.resolver.personaID.value;
        c.resolvedByDisplayName  = request.resolver.displayName;
        found = true;
        break;
    }

    if (!found)
        return Result<ResolveCommentResult>::failure(
            {ErrorCode::ioError, "comment not found: " + request.commentID});

    auto json = schemas::serializeCommentThread(thread);
    if (auto r = fs.atomicWriteTextFile(path, json); !r.ok())
        return Result<ResolveCommentResult>::failure(r.error());

    ResolveCommentResult result;
    result.commentID = request.commentID;
    result.resolved  = true;
    return Result<ResolveCommentResult>::success(std::move(result));
}

} // namespace scrivi::comments
