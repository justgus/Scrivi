#include "project_package/ProjectCreator.hpp"

#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/ProjectJson.hpp"
#include "schemas/ProjectMembersJson.hpp"
#include "schemas/ProjectPersonasJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "schemas/TimelineMetaJson.hpp"
#include "schemas/WorkspaceStateJson.hpp"
#include "util/PathUtils.hpp"

#include <sstream>

namespace scrivi::project_package {

ProjectCreator::ProjectCreator(CoreServices& services)
    : services_(services) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string workspaceStatePath(const AbsolutePath& appSupportRoot,
                                      const ProjectID& projectID) {
    return util::join(
        util::join(
            util::join(appSupportRoot, "state"),
            "projects"),
        projectID.value) + "/workspace-state.json";
}

Result<void> ProjectCreator::writeGitignore(const AbsolutePath& root) const {
    const std::string content =
        "# Scrivi app-local files — not part of the canonical project\n"
        ".scrivi-cache/\n"
        ".scrivi-index/\n"
        "\n"
        "# OS noise\n"
        ".DS_Store\n"
        "Thumbs.db\n";
    return services_.fileSystem->atomicWriteTextFile(
        util::join(root, ".gitignore"), content);
}

Result<void> ProjectCreator::writeSnapshotMetadata(const AbsolutePath& root) const {
    const std::string content =
        "{\"schema\":\"scrivi-snapshots\",\"schemaVersion\":1,\"snapshots\":[]}\n";
    auto snapshotsDir = util::join(root, "snapshots");
    auto r = services_.fileSystem->createDirectories(snapshotsDir);
    if (!r.ok()) { return r;
}
    return services_.fileSystem->atomicWriteTextFile(
        util::join(snapshotsDir, "scrivi-snapshots.json"), content);
}

// ---------------------------------------------------------------------------
// create()
// ---------------------------------------------------------------------------

Result<CreateProjectResult> ProjectCreator::create(const CreateProjectRequest& request) {
    // Validate authorship
    if (request.author.identityID.value.empty() ||
        request.author.personaID.value.empty()) {
        return Result<CreateProjectResult>::failure(
            {.code=ErrorCode::invalidArgument,
             .message="AuthorshipRef must have non-empty identityID and personaID"});
    }

    auto& fs  = *services_.fileSystem;
    auto& ids = *services_.uuidProvider;
    auto& clk = *services_.clock;

    const std::string now = clk.nowUTC();

    // Generate IDs
    const ProjectID    projectID    = ids.newProjectID();
    const ManuscriptID manuscriptID = ids.newManuscriptID();
    const ChapterID    chapterID    = ids.newChapterID();
    const SceneID      sceneID      = ids.newSceneID();

    const std::string& root          = request.projectRootPath;
    // The initial chapter keeps its `chapter-001` slug (request.initialChapterSlug). Its
    // order-key is therefore "001" — a valid order key (all base-62 digits, not ending in
    // '0'), so it sorts first and ChapterCreator's keyAfter("001") appends after it
    // correctly. Only the APPEND path (ChapterCreator) had the count+1 collision bug
    // (I-0072); the initial chapter is created once and never collides, so it needs no
    // change. (EP-027 P2.)
    const std::string  chapterSlug   = request.initialChapterSlug;
    const std::string  sceneSlug     = request.initialSceneSlug;

    // Relative paths inside the package
    const std::string manuscriptDir  = "manuscript";
    const std::string chapterDir     = manuscriptDir + "/" + chapterSlug;
    const std::string sceneMdRel     = chapterDir + "/" + sceneSlug + ".md";
    const std::string sceneMetaRel   = chapterDir + "/" + sceneSlug + ".meta.json";
    const std::string msMeta         = manuscriptDir + "/manuscript.meta.json";
    const std::string chMeta         = chapterDir + "/chapter.meta.json";
    const std::string identitiesDir  = "identities";
    const std::string membersPath    = identitiesDir + "/project-members.json";
    const std::string personasPath   = identitiesDir + "/project-personas.json";

    // 1. Create package root
    auto r = fs.createDirectories(root);
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 2. project.json
    schemas::ProjectJsonData proj;
    proj.projectID             = projectID;
    proj.title                 = request.title;
    proj.slug                  = request.slug;
    proj.createdAt             = now;
    proj.createdByIdentityID   = request.author.identityID.value;
    proj.createdByPersonaID    = request.author.personaID.value;
    proj.createdByDisplayName  = request.author.displayName;
    proj.manuscriptPath        = manuscriptDir;
    proj.membersPath           = membersPath;
    proj.personasPath          = personasPath;
    proj.gitSnapshotsEnabled       = request.enableGitSnapshots;
    proj.defaultSceneDurationMs    = 3'600'000;
    proj.dragPositioningMode       = "proportional";

    r = fs.atomicWriteTextFile(util::join(root, "project.json"),
                               schemas::serializeProject(proj));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 3. manuscript/
    r = fs.createDirectories(util::join(root, manuscriptDir));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    schemas::ManuscriptMetaData ms;
    ms.manuscriptID            = manuscriptID;
    ms.title                   = request.title;
    ms.createdAt               = now;
    ms.createdByIdentityID     = request.author.identityID.value;
    ms.createdByPersonaID      = request.author.personaID.value;
    ms.createdByDisplayName    = request.author.displayName;
    ms.chapters.push_back({.chapterID=chapterID, .path=chapterDir + "/chapter.meta.json"});

    r = fs.atomicWriteTextFile(util::join(root, msMeta),
                               schemas::serializeManuscriptMeta(ms));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 4. chapter/
    r = fs.createDirectories(util::join(root, chapterDir));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    schemas::ChapterMetaData ch;
    ch.chapterID               = chapterID;
    ch.title                   = request.initialChapterTitle;
    ch.slug                    = chapterSlug;
    ch.displayLabel            = "Chapter 1";
    ch.status                  = "draft";
    ch.createdAt               = now;
    ch.createdByIdentityID     = request.author.identityID.value;
    ch.createdByPersonaID      = request.author.personaID.value;
    ch.createdByDisplayName    = request.author.displayName;
    // EP-027 §8.1: filename-only scene ref (identity in the sidecar). The initial scene's
    // slug prefix "001" is already a valid order key, so a fresh project is born in the new
    // scheme and needs no migration on first open.
    ch.scenes.push_back({.metadataFilename = sceneSlug + ".meta.json"});

    r = fs.atomicWriteTextFile(util::join(root, chMeta),
                               schemas::serializeChapterMeta(ch));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 5. scene .md (empty)
    r = fs.atomicWriteTextFile(util::join(root, sceneMdRel), "");
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 6. scene .meta.json
    schemas::SceneMetaData scene;
    scene.sceneID                = sceneID;
    scene.title                  = request.initialSceneTitle;
    scene.slug                   = sceneSlug;
    scene.status                 = "draft";
    scene.createdAt              = now;
    scene.createdByIdentityID    = request.author.identityID.value;
    scene.createdByPersonaID     = request.author.personaID.value;
    scene.createdByDisplayName   = request.author.displayName;
    scene.modifiedAt             = now;
    scene.modifiedByIdentityID   = request.author.identityID.value;
    scene.modifiedByPersonaID    = request.author.personaID.value;
    scene.modifiedByDisplayName  = request.author.displayName;
    scene.contentPath            = sceneSlug + ".md";   // §8.1: bare filename
    scene.wordCount              = 0;
    scene.characterCount         = 0;

    r = fs.atomicWriteTextFile(util::join(root, sceneMetaRel),
                               schemas::serializeSceneMeta(scene));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 7. identities/
    r = fs.createDirectories(util::join(root, identitiesDir));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    schemas::ProjectMembersData members;
    members.members.push_back({
        .identityID=request.author.identityID,
        .role="owner",
        .status="active",
        .defaultPersonaID=request.author.personaID.value,
        .joinedAt=now
    });

    r = fs.atomicWriteTextFile(util::join(root, membersPath),
                               schemas::serializeProjectMembers(members));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    schemas::ProjectPersonasData personas;
    personas.personas.push_back({
        .personaID=request.author.personaID,
        .displayName=request.author.displayName,
        .personaKind="individual",
        .controlledByIdentityID=request.author.identityID.value,
        .createdAt=now,
        .status="active"
    });

    r = fs.atomicWriteTextFile(util::join(root, personasPath),
                               schemas::serializeProjectPersonas(personas));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 8. inbox/dropped-files/
    r = fs.createDirectories(util::join(util::join(root, "inbox"), "dropped-files"));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 9. objects/timelines/timeline.meta.json
    r = fs.createDirectories(util::join(root, "objects/timelines"));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    schemas::TimelineMetaData tlMeta;
    tlMeta.timelineID.value = ids.newTimelineID().value;
    tlMeta.projectID        = projectID;
    tlMeta.createdAt        = now;
    tlMeta.epochLabel       = "Story Open";

    r = fs.atomicWriteTextFile(
        util::join(root, "objects/timelines/timeline.meta.json"),
        schemas::serializeTimelineMeta(tlMeta));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 10. app-local workspace state
    schemas::WorkspaceStateData ws;
    ws.projectID        = projectID;
    ws.deviceID         = "device-local";
    ws.identityID       = request.author.identityID.value;
    ws.activePersonaID  = request.author.personaID.value;
    ws.lastOpenedAt     = now;
    ws.hasLastWritingSurface = true;
    ws.lastSceneID      = sceneID.value;
    ws.lastContentPath  = sceneMdRel;
    ws.cursorAnchor     = 0;
    ws.cursorFocus      = 0;
    ws.scrollPosition   = 0.0;

    auto wsDir = util::join(
        util::join(
            util::join(request.appSupportRoot, "state"),
            "projects"),
        projectID.value);
    r = fs.createDirectories(wsDir);
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    r = fs.atomicWriteTextFile(
        util::join(wsDir, "workspace-state.json"),
        schemas::serializeWorkspaceState(ws));
    if (!r.ok()) { return Result<CreateProjectResult>::failure(r.error());
}

    // 11. Optional Git
    bool gitInitialized = false;
    std::optional<SnapshotID> initialSnapshotID;

    if (request.enableGitSnapshots && services_.gitProvider != nullptr) {
        auto& git = *services_.gitProvider;

        auto initR = git.initRepository(root);
        if (!initR.ok()) { return Result<CreateProjectResult>::failure(initR.error());
}

        auto ignR = writeGitignore(root);
        if (!ignR.ok()) { return Result<CreateProjectResult>::failure(ignR.error());
}

        auto snapR = writeSnapshotMetadata(root);
        if (!snapR.ok()) { return Result<CreateProjectResult>::failure(snapR.error());
}

        auto addR = git.addAll(root);
        if (!addR.ok()) { return Result<CreateProjectResult>::failure(addR.error());
}

        CommitRequest commitReq;
        commitReq.message = "Initial project";
        commitReq.author  = {
            .name=request.author.displayName,
            .email=request.author.identityID.value + "@scrivi.author"
        };

        auto commitR = git.commit(root, commitReq);
        if (!commitR.ok()) { return Result<CreateProjectResult>::failure(commitR.error());
}

        gitInitialized     = true;
        auto snapID        = ids.newSnapshotID();
        initialSnapshotID  = snapID;
    }

    // 12. Assemble result
    ProjectSummary summary;
    summary.projectID           = projectID;
    summary.title               = request.title;
    summary.slug                = request.slug;
    summary.rootPath            = root;
    summary.gitSnapshotsEnabled = gitInitialized;

    WorkspaceState wsState;
    wsState.projectID       = projectID;
    wsState.deviceID        = "device-local";
    wsState.identityID      = request.author.identityID;
    wsState.activePersonaID = request.author.personaID;
    wsState.lastOpenedAt    = now;
    wsState.lastWritingSurface = LastWritingSurface{
        .sceneID=sceneID, .contentPath=sceneMdRel, .selection={.anchor=0, .focus=0}, .scroll={0.0}
    };

    CreateProjectResult result;
    result.project                = summary;
    result.manuscriptID           = manuscriptID;
    result.firstChapterID         = chapterID;
    result.firstSceneID           = sceneID;
    result.firstSceneMetadataPath = sceneMetaRel;
    result.firstSceneContentPath  = sceneMdRel;
    result.workspaceState         = wsState;
    result.gitInitialized         = gitInitialized;
    result.initialSnapshotID      = initialSnapshotID;

    return Result<CreateProjectResult>::success(std::move(result));
}

} // namespace scrivi::project_package
