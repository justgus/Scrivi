#include "git/SnapshotService.hpp"

#include "scrivi/Error.hpp"
#include "util/Json.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::git {

SnapshotService::SnapshotService(CoreServices& services)
    : services_(services) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

Result<void> SnapshotService::writeGitignore(const AbsolutePath& projectRoot) {
    const std::string content =
        "# Scrivi app-local files — not part of the canonical project\n"
        ".scrivi-cache/\n"
        ".scrivi-index/\n"
        "\n"
        "# OS noise\n"
        ".DS_Store\n"
        "Thumbs.db\n";
    return services_.fileSystem->atomicWriteTextFile(
        util::join(projectRoot, ".gitignore"), content);
}

Result<void> SnapshotService::appendSnapshotMetadata(
    const AbsolutePath& projectRoot,
    const std::string& snapshotID,
    const CommitID&    commitID,
    const std::string& label,
    const std::string& note,
    const std::string& timestamp,
    const AuthorshipRef& author)
{
    auto& fs = *services_.fileSystem;
    auto snapshotsPath = util::join(projectRoot, "snapshots/scrivi-snapshots.json");

    // Read existing file or start fresh
    std::string existing;
    auto existsR = fs.exists(snapshotsPath);
    if (existsR.ok() && existsR.value()) {
        auto readR = fs.readTextFile(snapshotsPath);
        if (readR.ok()) existing = readR.value();
    }

    // Parse existing JSON to append — use simple string manipulation since
    // we need to stay light. If parse fails, start fresh.
    util::JsonDoc doc;
    bool parsed = false;
    if (!existing.empty()) {
        auto parseR = util::parseJson(existing);
        if (parseR.ok()) {
            doc    = std::move(parseR.value());
            parsed = true;
        }
    }

    if (!parsed) {
        doc.setString("schema",     "scrivi-snapshots");
        doc.setInt("schemaVersion", 1);
    }

    // Build the new snapshot entry
    util::JsonDoc entry;
    entry.setString("snapshotID", snapshotID);
    entry.setString("commitID",   commitID.value);
    entry.setString("label",      label);
    entry.setString("note",       note);
    entry.setString("createdAt",  timestamp);
    entry.setString("createdByIdentityID",  author.identityID.value);
    entry.setString("createdByPersonaID",   author.personaID.value);
    entry.setString("createdByDisplayName", author.displayName);

    doc.appendToArray("snapshots", std::move(entry));

    auto snapshotsDir = util::join(projectRoot, "snapshots");
    auto dirR = fs.createDirectories(snapshotsDir);
    if (!dirR.ok()) return dirR;

    return fs.atomicWriteTextFile(snapshotsPath, doc.dump());
}

// ---------------------------------------------------------------------------
// enable()
// ---------------------------------------------------------------------------

Result<EnableGitResult> SnapshotService::enable(const EnableGitRequest& request) {
    if (services_.gitProvider == nullptr) {
        return Result<EnableGitResult>::failure(
            {ErrorCode::gitUnavailable, "No GitProvider configured"});
    }

    auto& git = *services_.gitProvider;
    auto& ids = *services_.uuidProvider;
    auto& clk = *services_.clock;

    EnableGitResult result;

    // Check if already a repo
    auto isRepoR = git.isRepository(request.projectRootPath);
    if (!isRepoR.ok()) return Result<EnableGitResult>::failure(isRepoR.error());

    if (isRepoR.value()) {
        result.alreadyRepository = true;
    } else {
        auto initR = git.initRepository(request.projectRootPath);
        if (!initR.ok()) return Result<EnableGitResult>::failure(initR.error());
    }

    // Write .gitignore
    auto ignR = writeGitignore(request.projectRootPath);
    if (!ignR.ok()) return Result<EnableGitResult>::failure(ignR.error());

    // Write initial snapshot metadata file
    auto snapshotsDir = util::join(request.projectRootPath, "snapshots");
    auto dirR = services_.fileSystem->createDirectories(snapshotsDir);
    if (!dirR.ok()) return Result<EnableGitResult>::failure(dirR.error());

    const auto snapshotID = ids.newSnapshotID();
    const auto now        = clk.nowUTC();

    // Stage all files
    auto addR = git.addAll(request.projectRootPath);
    if (!addR.ok()) return Result<EnableGitResult>::failure(addR.error());

    // Initial commit
    CommitRequest commitReq;
    commitReq.message = request.initialSnapshotLabel;
    commitReq.author  = {
        request.author.displayName,
        request.author.identityID.value + "@scrivi.author"
    };

    auto commitR = git.commit(request.projectRootPath, commitReq);
    if (!commitR.ok()) return Result<EnableGitResult>::failure(commitR.error());

    // Write snapshot metadata
    auto metaR = appendSnapshotMetadata(
        request.projectRootPath,
        snapshotID.value,
        commitR.value(),
        request.initialSnapshotLabel,
        "",
        now,
        request.author);
    if (!metaR.ok()) return Result<EnableGitResult>::failure(metaR.error());

    result.gitInitialized    = true;
    result.initialSnapshotID = snapshotID;
    result.initialCommitID   = commitR.value();

    return Result<EnableGitResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// createSnapshot()
// ---------------------------------------------------------------------------

Result<CreateSnapshotResult> SnapshotService::createSnapshot(
    const CreateSnapshotRequest& request)
{
    if (services_.gitProvider == nullptr) {
        return Result<CreateSnapshotResult>::failure(
            {ErrorCode::gitUnavailable, "No GitProvider configured"});
    }

    auto& git = *services_.gitProvider;
    auto& ids = *services_.uuidProvider;
    auto& clk = *services_.clock;

    const auto snapshotID = ids.newSnapshotID();
    const auto now        = clk.nowUTC();

    // Stage all changes
    auto addR = git.addAll(request.projectRootPath);
    if (!addR.ok()) return Result<CreateSnapshotResult>::failure(addR.error());

    // Commit
    CommitRequest commitReq;
    commitReq.message = request.label;
    commitReq.author  = {
        request.author.displayName,
        request.author.identityID.value + "@scrivi.author"
    };

    auto commitR = git.commit(request.projectRootPath, commitReq);
    if (!commitR.ok()) return Result<CreateSnapshotResult>::failure(commitR.error());

    // Write snapshot metadata
    auto metaR = appendSnapshotMetadata(
        request.projectRootPath,
        snapshotID.value,
        commitR.value(),
        request.label,
        request.note,
        now,
        request.author);
    if (!metaR.ok()) return Result<CreateSnapshotResult>::failure(metaR.error());

    CreateSnapshotResult result;
    result.snapshotID = snapshotID;
    result.commitID   = commitR.value();
    result.createdAt  = now;
    result.created    = true;

    return Result<CreateSnapshotResult>::success(std::move(result));
}

} // namespace scrivi::git
