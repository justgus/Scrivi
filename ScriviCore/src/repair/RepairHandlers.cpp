#include "repair/RepairHandlers.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

namespace scrivi::repair {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Build a success ApplyRepairResult.
static ApplyRepairResult okResult(const ApplyRepairRequest& req, std::string detail) {
    ApplyRepairResult r;
    r.issueID       = req.issueID;
    r.actionApplied = req.actionKind;
    r.resolved      = true;
    r.detail        = std::move(detail);
    return r;
}

// Build a failure Result with the standard repair error shape.
static Result<ApplyRepairResult> repairError(ErrorCode code, std::string message,
                                              const std::string& path = {})
{
    return Result<ApplyRepairResult>::failure({code, std::move(message), path});
}

// Read the scene metadata file identified by issue.path. issue.path is the
// absolute path to the .meta.json file.
static Result<schemas::SceneMetaData> readSceneMeta(
    const HandlerContext& ctx, const std::string& absMetaPath)
{
    auto textR = ctx.services.fileSystem->readTextFile(absMetaPath);
    if (!textR.ok())
        return Result<schemas::SceneMetaData>::failure(textR.error());
    return schemas::parseSceneMeta(textR.value());
}

// Write a .bak copy of a file before overwriting it.
// Ignores errors — a missing backup is not fatal.
static void writeBackup(FileSystem& fs, const AbsolutePath& path) {
    auto textR = fs.readTextFile(path);
    if (!textR.ok()) return;
    auto bakPath = path + ".bak";
    fs.atomicWriteTextFile(bakPath, textR.value());
}

// Locate the chapter metadata file that owns a given sceneID by scanning
// the chapter directories referenced from manuscript.meta.json.
// Returns the absolute path to the chapter .meta.json and the parsed data.
struct ChapterLocation {
    AbsolutePath          absPath;
    schemas::ChapterMetaData data;
    std::size_t           sceneIndex; // index of the scene in data.scenes
};

static Result<ChapterLocation> findChapterForScene(
    const HandlerContext& ctx, const SceneID& sceneID)
{
    auto& fs         = *ctx.services.fileSystem;
    const auto& root = ctx.request.projectRootPath;

    // Read manuscript.meta.json to get chapter list
    auto msPath  = util::join(root, "manuscript/manuscript.meta.json");
    auto msTextR = fs.readTextFile(msPath);
    if (!msTextR.ok())
        return Result<ChapterLocation>::failure(msTextR.error());

    // Parse manually (we only need the chapters array)
    // We'll iterate chapter paths and check each one.
    // Use ChapterMetaJson to parse each chapter file.
    // First get the list of chapter dirs from the manuscript directory.
    auto msDirPath = util::join(root, "manuscript");
    auto listR     = fs.listDirectory(msDirPath);
    if (!listR.ok())
        return Result<ChapterLocation>::failure(listR.error());

    for (auto& entry : listR.value()) {
        auto isDirR = fs.isDirectory(entry);
        if (!isDirR.ok() || !isDirR.value()) continue;

        // Look for chapter.meta.json inside this directory
        auto chMetaPath = util::join(entry, "chapter.meta.json");
        auto chTextR    = fs.readTextFile(chMetaPath);
        if (!chTextR.ok()) continue;

        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) continue;

        auto& scenes = chParsed.value().scenes;
        for (std::size_t i = 0; i < scenes.size(); ++i) {
            if (scenes[i].sceneID.value == sceneID.value) {
                ChapterLocation loc;
                loc.absPath   = chMetaPath;
                loc.data      = std::move(chParsed.value());
                loc.sceneIndex = i;
                return Result<ChapterLocation>::success(std::move(loc));
            }
        }
    }

    return Result<ChapterLocation>::failure({ErrorCode::invalidArgument,
        "could not find a chapter owning sceneID '" + sceneID.value + "'"});
}

// ---------------------------------------------------------------------------
// relinkToFile
// ---------------------------------------------------------------------------
// Updates the scene metadata's contentPath to point at request.targetPath.
// issue.sceneID identifies the scene; we locate its .meta.json via the chapter
// index (same approach as markMissing), since issue.path for a missingContent
// issue is the missing content file path, not the metadata path.

Result<ApplyRepairResult> handleRelinkToFile(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    if (req.targetPath.empty())
        return repairError(ErrorCode::invalidArgument,
            "relinkToFile requires a non-empty targetPath");

    // Resolve the scene metadata path via the chapter index
    auto chLocR = findChapterForScene(ctx, issue.sceneID);
    if (!chLocR.ok())
        return Result<ApplyRepairResult>::failure(chLocR.error());

    const auto& chLoc    = chLocR.value();
    const auto& sceneRef = chLoc.data.scenes[chLoc.sceneIndex];
    auto absMetaPath     = util::join(req.projectRootPath, sceneRef.metadataPath);

    auto metaR = readSceneMeta(ctx, absMetaPath);
    if (!metaR.ok())
        return Result<ApplyRepairResult>::failure(metaR.error());

    auto meta = metaR.value();

    // Normalise targetPath to relative
    std::string relTarget = req.targetPath;
    if (relTarget.size() > req.projectRootPath.size() &&
        relTarget.substr(0, req.projectRootPath.size()) == req.projectRootPath)
    {
        relTarget = relTarget.substr(req.projectRootPath.size() + 1);
    }

    meta.contentPath             = relTarget;
    meta.modifiedAt              = ctx.services.clock->nowUTC();
    meta.modifiedByIdentityID    = req.author.identityID.value;
    meta.modifiedByPersonaID     = req.author.personaID.value;
    meta.modifiedByDisplayName   = req.author.displayName;

    writeBackup(*ctx.services.fileSystem, absMetaPath);

    auto writeR = ctx.services.fileSystem->atomicWriteTextFile(
        absMetaPath, schemas::serializeSceneMeta(meta));
    if (!writeR.ok())
        return Result<ApplyRepairResult>::failure(writeR.error());

    return Result<ApplyRepairResult>::success(
        okResult(req, "Scene relinked to '" + relTarget + "'"));
}

// ---------------------------------------------------------------------------
// createEmptyContentFile
// ---------------------------------------------------------------------------
// Writes an empty .md at the missing content path derived from the metadata.
// Requires: issue.path is the absolute path to the .meta.json (or the
//           expected absolute content path — we derive from issue.sceneID).

Result<ApplyRepairResult> handleCreateEmptyContentFile(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    // issue.path for missingContent is the absolute expected content path.
    const std::string& absContentPath = issue.path;

    // Ensure the parent directory exists
    auto parentDir = util::parent(absContentPath);
    auto mkdirR = ctx.services.fileSystem->createDirectories(parentDir);
    if (!mkdirR.ok())
        return Result<ApplyRepairResult>::failure(mkdirR.error());

    auto writeR = ctx.services.fileSystem->atomicWriteTextFile(absContentPath, "");
    if (!writeR.ok())
        return Result<ApplyRepairResult>::failure(writeR.error());

    return Result<ApplyRepairResult>::success(
        okResult(req, "Empty content file created at '" + absContentPath + "'"));
}

// ---------------------------------------------------------------------------
// markMissing
// ---------------------------------------------------------------------------
// Sets a `missingContent: true` flag in scene metadata. The field is written
// as a top-level JSON key. Since SceneMetaData doesn't carry this flag as a
// typed field, we store it by round-tripping: parse, set status to "missing",
// re-serialise. Status = "missing" is the canonical signal understood by the
// project opener.

Result<ApplyRepairResult> handleMarkMissing(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    // For missingContent, the scene .meta.json path is derivable:
    // issue contains sceneID; we need to find the metadata file.
    // The issue.path is the missing *content* path; the metadata path is
    // stored in the chapter. Find the chapter and get the metadataPath.
    auto chLocR = findChapterForScene(ctx, issue.sceneID);
    if (!chLocR.ok())
        return Result<ApplyRepairResult>::failure(chLocR.error());

    const auto& chLoc   = chLocR.value();
    const auto& sceneRef = chLoc.data.scenes[chLoc.sceneIndex];
    auto absMetaPath = util::join(req.projectRootPath, sceneRef.metadataPath);

    auto metaR = readSceneMeta(ctx, absMetaPath);
    if (!metaR.ok())
        return Result<ApplyRepairResult>::failure(metaR.error());

    auto meta    = metaR.value();
    meta.status  = "missing";
    meta.modifiedAt            = ctx.services.clock->nowUTC();
    meta.modifiedByIdentityID  = req.author.identityID.value;
    meta.modifiedByPersonaID   = req.author.personaID.value;
    meta.modifiedByDisplayName = req.author.displayName;

    writeBackup(*ctx.services.fileSystem, absMetaPath);

    auto writeR = ctx.services.fileSystem->atomicWriteTextFile(
        absMetaPath, schemas::serializeSceneMeta(meta));
    if (!writeR.ok())
        return Result<ApplyRepairResult>::failure(writeR.error());

    return Result<ApplyRepairResult>::success(
        okResult(req, "Scene '" + meta.sceneID.value + "' marked as missing"));
}

// ---------------------------------------------------------------------------
// removeFromProject
// ---------------------------------------------------------------------------
// Removes the scene entry from its chapter metadata file.
// The content file and .meta.json are NOT deleted (preserved on disk).

Result<ApplyRepairResult> handleRemoveFromProject(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    auto chLocR = findChapterForScene(ctx, issue.sceneID);
    if (!chLocR.ok())
        return Result<ApplyRepairResult>::failure(chLocR.error());

    auto chLoc = chLocR.value();
    auto& scenes = chLoc.data.scenes;

    // Remove the scene entry at chLoc.sceneIndex
    scenes.erase(scenes.begin() + static_cast<std::ptrdiff_t>(chLoc.sceneIndex));

    writeBackup(*ctx.services.fileSystem, chLoc.absPath);

    auto writeR = ctx.services.fileSystem->atomicWriteTextFile(
        chLoc.absPath, schemas::serializeChapterMeta(chLoc.data));
    if (!writeR.ok())
        return Result<ApplyRepairResult>::failure(writeR.error());

    return Result<ApplyRepairResult>::success(
        okResult(req, "Scene '" + issue.sceneID.value +
                      "' removed from chapter index (files preserved on disk)"));
}

// ---------------------------------------------------------------------------
// moveToInbox
// ---------------------------------------------------------------------------
// Moves the file at issue.path to projectRoot/inbox/dropped-files/.
// Implemented as read + write to new location + remove original, so it works
// within the FileSystem interface without a native rename/move method.

Result<ApplyRepairResult> handleMoveToInbox(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    const std::string& srcPath = issue.path;

    // Ensure inbox directory exists
    auto inboxDir = util::join(req.projectRootPath, "inbox/dropped-files");
    auto mkdirR   = ctx.services.fileSystem->createDirectories(inboxDir);
    if (!mkdirR.ok())
        return Result<ApplyRepairResult>::failure(mkdirR.error());

    // Build destination path: inbox/dropped-files/<filename>
    auto fname   = util::filename(srcPath);
    auto dstPath = util::join(inboxDir, fname);

    // Read source
    auto textR = ctx.services.fileSystem->readTextFile(srcPath);
    if (!textR.ok())
        return Result<ApplyRepairResult>::failure(textR.error());

    // Write to destination
    auto writeR = ctx.services.fileSystem->atomicWriteTextFile(dstPath, textR.value());
    if (!writeR.ok())
        return Result<ApplyRepairResult>::failure(writeR.error());

    // Remove source
    auto removeR = ctx.services.fileSystem->removeFile(srcPath);
    if (!removeR.ok())
        return Result<ApplyRepairResult>::failure(removeR.error());

    return Result<ApplyRepairResult>::success(
        okResult(req, "File moved to inbox: '" + dstPath + "'"));
}

// ---------------------------------------------------------------------------
// reloadExternalVersion
// ---------------------------------------------------------------------------
// No write. Returns the current on-disk content in the detail field so the
// caller (Swift layer) can present it to the user for manual acceptance.

Result<ApplyRepairResult> handleReloadExternalVersion(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    auto textR = ctx.services.fileSystem->readTextFile(issue.path);
    if (!textR.ok())
        return Result<ApplyRepairResult>::failure(textR.error());

    ApplyRepairResult result = okResult(req, textR.value());
    // resolved = true signals the issue is handled; the caller decides what
    // to do with the content returned in 'detail'.
    return Result<ApplyRepairResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// regenerateMetadata
// ---------------------------------------------------------------------------
// Handles both scene and chapter metadata regeneration, distinguished by
// whether issue.sceneID is set (scene) or empty (chapter).
//
// Scene: reconstructs .meta.json from the content file name + inferred title.
// Chapter: reconstructs chapter.meta.json from the folder contents.

Result<ApplyRepairResult> handleRegenerateMetadata(const HandlerContext& ctx) {
    const auto& req   = ctx.request;
    const auto& issue = ctx.issue;

    // Distinguish scene vs chapter by sceneID
    const bool isScene = !issue.sceneID.value.empty();

    if (isScene) {
        // --- Scene metadata regeneration ---
        // issue.path is the absolute path to the corrupt/missing .meta.json.
        const std::string& absMetaPath = issue.path;

        // Derive content path from metadata path: strip .meta.json → .md
        std::string relMeta = absMetaPath;
        if (relMeta.size() > req.projectRootPath.size() &&
            relMeta.substr(0, req.projectRootPath.size()) == req.projectRootPath)
        {
            relMeta = relMeta.substr(req.projectRootPath.size() + 1);
        }
        // relMeta is now e.g. "manuscript/chapter-001/001-opening-scene.meta.json"
        auto relContent = util::replaceExtension(
            util::replaceExtension(relMeta, ""), ".md");
        auto absContentPath = util::join(req.projectRootPath, relContent);

        // Confirm content file exists
        auto existsR = ctx.services.fileSystem->exists(absContentPath);
        if (!existsR.ok())
            return Result<ApplyRepairResult>::failure(existsR.error());
        if (!existsR.value())
            return repairError(ErrorCode::ioError,
                "Cannot regenerate metadata: content file not found at '" +
                absContentPath + "'");

        // Generate a fresh sceneID for the regenerated metadata
        auto newSceneID = ctx.services.uuidProvider->newSceneID();

        // Infer title from filename (strip extension, replace hyphens/underscores)
        auto fname = util::filename(absContentPath);
        // Strip .md
        auto stem = util::replaceExtension(fname, "");
        // Replace hyphens/underscores with spaces for a human-readable title
        std::string inferredTitle = stem;
        for (char& c : inferredTitle)
            if (c == '-' || c == '_') c = ' ';

        auto now = ctx.services.clock->nowUTC();

        schemas::SceneMetaData meta;
        meta.sceneID.value           = newSceneID.value;
        meta.title                   = inferredTitle;
        meta.slug                    = stem;
        meta.status                  = "draft";
        meta.createdAt               = now;
        meta.createdByIdentityID     = req.author.identityID.value;
        meta.createdByPersonaID      = req.author.personaID.value;
        meta.createdByDisplayName    = req.author.displayName;
        meta.modifiedAt              = now;
        meta.modifiedByIdentityID    = req.author.identityID.value;
        meta.modifiedByPersonaID     = req.author.personaID.value;
        meta.modifiedByDisplayName   = req.author.displayName;
        meta.contentPath             = relContent;
        meta.wordCount               = 0;
        meta.characterCount          = 0;

        // Back up if it exists (corrupt case)
        auto existsMeta = ctx.services.fileSystem->exists(absMetaPath);
        if (existsMeta.ok() && existsMeta.value())
            writeBackup(*ctx.services.fileSystem, absMetaPath);

        // Ensure parent directory exists
        auto parentDir = util::parent(absMetaPath);
        ctx.services.fileSystem->createDirectories(parentDir);

        auto writeR = ctx.services.fileSystem->atomicWriteTextFile(
            absMetaPath, schemas::serializeSceneMeta(meta));
        if (!writeR.ok())
            return Result<ApplyRepairResult>::failure(writeR.error());

        return Result<ApplyRepairResult>::success(
            okResult(req, "Scene metadata regenerated at '" + absMetaPath +
                          "' with new sceneID '" + newSceneID.value + "'"));

    } else {
        // --- Chapter metadata regeneration ---
        // issue.path is the absolute path to the corrupt/missing chapter.meta.json.
        const std::string& absChMetaPath = issue.path;
        auto chapterDir = util::parent(absChMetaPath);

        // Scan the chapter directory for .meta.json files (scene metadata)
        auto listR = ctx.services.fileSystem->listDirectory(chapterDir);
        if (!listR.ok())
            return Result<ApplyRepairResult>::failure(listR.error());

        std::vector<schemas::SceneRef> sceneRefs;
        for (auto& entry : listR.value()) {
            // A scene metadata file ends in .meta.json (but is not chapter.meta.json)
            auto fname = util::filename(entry);
            if (fname == "chapter.meta.json") continue;
            if (util::extension(entry) != ".json") continue;
            // Quick check: try to parse it as scene metadata
            auto textR = ctx.services.fileSystem->readTextFile(entry);
            if (!textR.ok()) continue;
            auto parsed = schemas::parseSceneMeta(textR.value());
            if (!parsed.ok()) continue;

            // Build relative path from projectRoot
            std::string relMeta = entry;
            if (relMeta.size() > req.projectRootPath.size() &&
                relMeta.substr(0, req.projectRootPath.size()) == req.projectRootPath)
            {
                relMeta = relMeta.substr(req.projectRootPath.size() + 1);
            }
            sceneRefs.push_back({parsed.value().sceneID, relMeta});
        }

        // Generate new chapterID
        auto newChapterID = ctx.services.uuidProvider->newChapterID();

        // Infer title from directory name
        auto dirName       = util::filename(chapterDir);
        std::string inferredTitle = dirName;
        for (char& c : inferredTitle)
            if (c == '-' || c == '_') c = ' ';

        auto now = ctx.services.clock->nowUTC();

        schemas::ChapterMetaData chData;
        chData.chapterID.value       = newChapterID.value;
        chData.title                 = inferredTitle;
        chData.slug                  = dirName;
        chData.displayLabel          = "Chapter";
        chData.status                = "draft";
        chData.createdAt             = now;
        chData.createdByIdentityID   = req.author.identityID.value;
        chData.createdByPersonaID    = req.author.personaID.value;
        chData.createdByDisplayName  = req.author.displayName;
        chData.scenes                = std::move(sceneRefs);

        // Back up if the file exists (corrupt case)
        auto existsCh = ctx.services.fileSystem->exists(absChMetaPath);
        if (existsCh.ok() && existsCh.value())
            writeBackup(*ctx.services.fileSystem, absChMetaPath);

        auto writeR = ctx.services.fileSystem->atomicWriteTextFile(
            absChMetaPath, schemas::serializeChapterMeta(chData));
        if (!writeR.ok())
            return Result<ApplyRepairResult>::failure(writeR.error());

        return Result<ApplyRepairResult>::success(
            okResult(req, "Chapter metadata regenerated at '" + absChMetaPath +
                          "' with " + std::to_string(chData.scenes.size()) +
                          " scene(s) discovered"));
    }
}

} // namespace scrivi::repair
