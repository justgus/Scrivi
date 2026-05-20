#include "workspace/WorkspaceStateService.hpp"

#include "schemas/WorkspaceStateJson.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::workspace {

WorkspaceStateService::WorkspaceStateService(CoreServices& services)
    : services_(services) {}

AbsolutePath WorkspaceStateService::stateFilePath(
    const AbsolutePath& appSupportRoot,
    const ProjectID& projectID) const
{
    return util::join(
        util::join(
            util::join(appSupportRoot, "state"),
            "projects"),
        projectID.value) + "/workspace-state.json";
}

Result<std::optional<WorkspaceState>> WorkspaceStateService::load(
    const AbsolutePath& appSupportRoot,
    const ProjectID&    projectID)
{
    auto path = stateFilePath(appSupportRoot, projectID);

    auto existsR = services_.fileSystem->exists(path);
    if (!existsR.ok()) return Result<std::optional<WorkspaceState>>::failure(existsR.error());
    if (!existsR.value()) return Result<std::optional<WorkspaceState>>::success(std::nullopt);

    auto textR = services_.fileSystem->readTextFile(path);
    if (!textR.ok()) return Result<std::optional<WorkspaceState>>::failure(textR.error());

    auto parseR = schemas::parseWorkspaceState(textR.value());
    if (!parseR.ok()) return Result<std::optional<WorkspaceState>>::failure(parseR.error());

    auto& d = parseR.value();

    WorkspaceState state;
    state.projectID.value  = d.projectID.value;
    state.deviceID         = d.deviceID;
    state.identityID.value = d.identityID;
    state.activePersonaID.value = d.activePersonaID;
    state.lastOpenedAt     = d.lastOpenedAt;

    if (d.hasLastWritingSurface) {
        LastWritingSurface lws;
        lws.sceneID.value   = d.lastSceneID;
        lws.contentPath     = d.lastContentPath;
        lws.selection       = {d.cursorAnchor, d.cursorFocus};
        lws.scroll          = {d.scrollPosition};
        state.lastWritingSurface = lws;
    }

    return Result<std::optional<WorkspaceState>>::success(std::move(state));
}

Result<void> WorkspaceStateService::save(
    const AbsolutePath& appSupportRoot,
    const WorkspaceState& state)
{
    auto dir = util::join(
        util::join(
            util::join(appSupportRoot, "state"),
            "projects"),
        state.projectID.value);

    auto dirR = services_.fileSystem->createDirectories(dir);
    if (!dirR.ok()) return dirR;

    schemas::WorkspaceStateData d;
    d.projectID.value  = state.projectID.value;
    d.deviceID         = state.deviceID;
    d.identityID       = state.identityID.value;
    d.activePersonaID  = state.activePersonaID.value;
    d.lastOpenedAt     = state.lastOpenedAt;

    if (state.lastWritingSurface.has_value()) {
        auto& lws = *state.lastWritingSurface;
        d.hasLastWritingSurface = true;
        d.lastSceneID      = lws.sceneID.value;
        d.lastContentPath  = lws.contentPath;
        d.cursorAnchor     = lws.selection.anchor;
        d.cursorFocus      = lws.selection.focus;
        d.scrollPosition   = lws.scroll.value;
    }

    auto path = stateFilePath(appSupportRoot, state.projectID);
    return services_.fileSystem->atomicWriteTextFile(path, schemas::serializeWorkspaceState(d));
}

} // namespace scrivi::workspace
