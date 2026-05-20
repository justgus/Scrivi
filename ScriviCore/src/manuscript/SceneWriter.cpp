#include "manuscript/SceneWriter.hpp"

#include "schemas/SceneMetaJson.hpp"
#include "util/Hash.hpp"
#include "util/PathUtils.hpp"
#include "util/TextStats.hpp"
#include "workspace/WorkspaceStateService.hpp"

namespace scrivi::manuscript {

SceneWriter::SceneWriter(CoreServices& services)
    : services_(services) {}

Result<SaveSceneResult> SceneWriter::save(const SaveSceneRequest& request)
{
    auto& fs  = *services_.fileSystem;
    auto& clk = *services_.clock;

    SaveSceneResult result;
    result.sceneID = request.sceneID;

    // 1. Compute hash of incoming content
    const std::string contentHash = util::sha256Hex(request.markdown);

    // 2. Idempotent check — skip write if content unchanged
    if (request.previouslyLoadedContentHash.has_value() &&
        *request.previouslyLoadedContentHash == contentHash)
    {
        // Content unchanged: still update workspace state
        workspace::WorkspaceStateService wsService{services_};
        WorkspaceState ws;
        ws.projectID.value      = request.projectID.value;
        ws.deviceID             = "device-local";
        ws.identityID           = request.author.identityID;
        ws.activePersonaID      = request.author.personaID;
        ws.lastOpenedAt         = clk.nowUTC();
        ws.lastWritingSurface   = LastWritingSurface{
            request.sceneID,
            request.sceneContentPath,
            request.selection,
            request.scroll
        };

        auto wsR = wsService.save(request.appSupportRoot, ws);
        if (!wsR.ok()) return Result<SaveSceneResult>::failure(wsR.error());

        result.saved                 = false;
        result.metadataUpdated       = false;
        result.workspaceStateUpdated = true;
        return Result<SaveSceneResult>::success(std::move(result));
    }

    // 3. Write content atomically
    auto contentAbsPath = util::join(request.projectRootPath, request.sceneContentPath);
    auto writeR = fs.atomicWriteTextFile(contentAbsPath, request.markdown);
    if (!writeR.ok()) return Result<SaveSceneResult>::failure(writeR.error());

    result.saved = true;

    // 4. Compute text stats
    auto stats = util::countText(request.markdown);
    result.wordCount      = stats.wordCount;
    result.characterCount = stats.characterCount;

    // 5. Read current scene metadata
    auto metaAbsPath = util::join(request.projectRootPath, request.sceneMetadataPath);
    auto metaTextR = fs.readTextFile(metaAbsPath);
    if (!metaTextR.ok()) return Result<SaveSceneResult>::failure(metaTextR.error());

    auto metaParsed = schemas::parseSceneMeta(metaTextR.value());
    if (!metaParsed.ok()) return Result<SaveSceneResult>::failure(metaParsed.error());

    // 6. Update metadata fields
    auto& meta             = metaParsed.value();
    meta.modifiedAt              = clk.nowUTC();
    meta.modifiedByIdentityID    = request.author.identityID.value;
    meta.modifiedByPersonaID     = request.author.personaID.value;
    meta.modifiedByDisplayName   = request.author.displayName;
    meta.wordCount               = stats.wordCount;
    meta.characterCount          = stats.characterCount;

    auto writeMetaR = fs.atomicWriteTextFile(metaAbsPath, schemas::serializeSceneMeta(meta));
    if (!writeMetaR.ok()) return Result<SaveSceneResult>::failure(writeMetaR.error());

    result.metadataUpdated = true;

    // 7. Update workspace state
    workspace::WorkspaceStateService wsService{services_};
    WorkspaceState ws;
    ws.projectID.value    = request.projectID.value;
    ws.deviceID           = "device-local";
    ws.identityID         = request.author.identityID;
    ws.activePersonaID    = request.author.personaID;
    ws.lastOpenedAt       = meta.modifiedAt;
    ws.lastWritingSurface = LastWritingSurface{
        request.sceneID,
        request.sceneContentPath,
        request.selection,
        request.scroll
    };

    auto wsR = wsService.save(request.appSupportRoot, ws);
    if (!wsR.ok()) return Result<SaveSceneResult>::failure(wsR.error());

    result.workspaceStateUpdated = true;

    return Result<SaveSceneResult>::success(std::move(result));
}

} // namespace scrivi::manuscript
