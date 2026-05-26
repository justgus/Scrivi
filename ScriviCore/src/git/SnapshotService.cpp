#include "git/SnapshotService.hpp"

#include "scrivi/Error.hpp"
#include "schemas/SnapshotMetadataJson.hpp"
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

    // Read and parse existing metadata, or start fresh
    schemas::SnapshotMetadataData metadata;
    auto existsR = fs.exists(snapshotsPath);
    if (existsR.ok() && existsR.value()) {
        auto readR = fs.readTextFile(snapshotsPath);
        if (readR.ok()) {
            auto parseR = schemas::parseSnapshotMetadata(readR.value());
            if (parseR.ok()) metadata = std::move(parseR.value());
            // On parse failure, start with an empty metadata (self-healing)
        }
    }

    // Append the new entry
    schemas::SnapshotEntryData entry;
    entry.snapshotID             = snapshotID;
    entry.commitID               = commitID.value;
    entry.label                  = label;
    entry.note                   = note;
    entry.createdAt              = timestamp;
    entry.createdByIdentityID    = author.identityID.value;
    entry.createdByPersonaID     = author.personaID.value;
    entry.createdByDisplayName   = author.displayName;
    metadata.snapshots.push_back(std::move(entry));

    auto snapshotsDir = util::join(projectRoot, "snapshots");
    auto dirR = fs.createDirectories(snapshotsDir);
    if (!dirR.ok()) return dirR;

    return fs.atomicWriteTextFile(snapshotsPath,
        schemas::serializeSnapshotMetadata(metadata));
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
