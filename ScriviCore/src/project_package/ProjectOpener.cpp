#include "project_package/ProjectOpener.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/SceneReader.hpp"
#include "project_package/ProjectValidator.hpp"
#include "schemas/ProjectJson.hpp"
#include "util/PathUtils.hpp"
#include "workspace/WorkspaceStateService.hpp"

namespace scrivi::project_package {

ProjectOpener::ProjectOpener(CoreServices& services)
    : services_(services) {}

Result<OpenProjectResult> ProjectOpener::open(const OpenProjectRequest& request)
{
    auto& fs = *services_.fileSystem;

    // 1. Validate project package structure
    ProjectValidator validator{services_};
    auto issuesR = validator.validate(request.projectRootPath);
    if (!issuesR.ok()) { return Result<OpenProjectResult>::failure(issuesR.error());
}

    auto& issues = issuesR.value();

    // Check for blocking issues
    bool hasBlocking = false;
    for (auto& issue : issues) {
        if (issue.severity == RepairSeverity::blocking) {
            hasBlocking = true;
            break;
        }
    }

    if (hasBlocking) {
        OpenProjectResult result;
        result.mode        = OpenMode::repairRequired;
        result.repairIssues = std::move(issues);
        return Result<OpenProjectResult>::success(std::move(result));
    }

    // 2. Read project.json
    auto projTextR = fs.readTextFile(util::join(request.projectRootPath, "project.json"));
    if (!projTextR.ok()) { return Result<OpenProjectResult>::failure(projTextR.error());
}

    auto projParsed = schemas::parseProject(projTextR.value());
    if (!projParsed.ok()) { return Result<OpenProjectResult>::failure(projParsed.error());
}

    auto& proj = projParsed.value();

    ProjectSummary summary;
    summary.projectID.value    = proj.projectID.value;
    summary.title              = proj.title;
    summary.slug               = proj.slug;
    summary.rootPath           = request.projectRootPath;
    summary.gitSnapshotsEnabled = proj.gitSnapshotsEnabled;

    // 2a. EP-027 P3 migration: an old-format project (legacy `chapter-NNN` folders whose
    //     numeric sort doesn't reproduce the intended reading order held in the index
    //     array) is migrated to order-key slugs so the folder-key sort == reading order.
    //     Idempotent no-op for new-scheme / already-in-order projects. Runs BEFORE the
    //     resolver reads order (which is now folder-sort based). Best-effort.
    manuscript::migrateChapterOrderKeys(fs, request.projectRootPath);

    // 2b. EP-027 B3 self-heal: if manuscript.meta.json's chapter index disagrees with the
    //     on-disk `chapter-*` folders (stale order, phantom/duplicate entries — the I-0072
    //     damage), rewrite it from disk so the cache matches truth. Best-effort: order is
    //     derived from disk regardless, so a write failure here doesn't block the open.
    manuscript::rebuildIndexIfInconsistent(fs, request.projectRootPath);

    // 3. Resolve manuscript order
    manuscript::ManuscriptOrderResolver resolver{services_};
    auto scenesR = resolver.resolve(request.projectRootPath);
    if (!scenesR.ok()) { return Result<OpenProjectResult>::failure(scenesR.error());
}

    auto& scenes = scenesR.value();
    if (scenes.empty()) {
        OpenProjectResult result;
        result.mode    = OpenMode::repairRequired;
        result.project = summary;
        RepairIssue issue;
        issue.severity = RepairSeverity::blocking;
        issue.category = RepairCategory::missingContent;
        issue.title    = "Project has no scenes";
        result.repairIssues.push_back(std::move(issue));
        return Result<OpenProjectResult>::success(std::move(result));
    }

    // 4. Load workspace state
    workspace::WorkspaceStateService wsService{services_};
    auto wsR = wsService.load(request.appSupportRoot, proj.projectID);
    if (!wsR.ok()) { return Result<OpenProjectResult>::failure(wsR.error());
}

    // 5. Determine active scene (from workspace state or fall back to first)
    const manuscript::ResolvedScene* activeScene = scenes.data();

    std::optional<WorkspaceState> workspaceState;
    TextSelection  restoredSelection;
    ScrollPosition restoredScroll;

    const auto& wsOpt = wsR.value();
    if (wsOpt.has_value()) {
        workspaceState = *wsOpt;
        auto& ws = *workspaceState;

        if (ws.lastWritingSurface.has_value()) {
            auto& lws = *ws.lastWritingSurface;
            restoredSelection = lws.selection;
            restoredScroll    = lws.scroll;

            // Find the matching scene in the ordered list
            for (auto& s : scenes) {
                if (s.sceneID.value == lws.sceneID.value) {
                    activeScene = &s;
                    break;
                }
            }
        }
    }

    // 6. Read active scene markdown
    manuscript::SceneReader reader{services_};
    auto mdR = reader.readContent(request.projectRootPath, activeScene->contentPath);
    if (!mdR.ok()) { return Result<OpenProjectResult>::failure(mdR.error());
}

    // 7. Assemble result
    SceneSummary sceneSummary;
    sceneSummary.sceneID             = activeScene->sceneID;
    sceneSummary.chapterID           = activeScene->chapterID;
    sceneSummary.title               = activeScene->title;
    sceneSummary.chapterTitle        = activeScene->chapterTitle;
    sceneSummary.slug                = activeScene->slug;
    sceneSummary.metadataPath        = activeScene->metadataPath;
    sceneSummary.contentPath         = activeScene->contentPath;
    sceneSummary.chapterMetadataPath = activeScene->chapterMetadataPath;

    std::vector<SceneSummary> allScenes;
    allScenes.reserve(scenes.size());
    for (auto& s : scenes) {
        SceneSummary ss;
        ss.sceneID             = s.sceneID;
        ss.chapterID           = s.chapterID;
        ss.title               = s.title;
        ss.chapterTitle        = s.chapterTitle;
        ss.slug                = s.slug;
        ss.metadataPath        = s.metadataPath;
        ss.contentPath         = s.contentPath;
        ss.chapterMetadataPath = s.chapterMetadataPath;
        allScenes.push_back(std::move(ss));
    }

    OpenProjectResult result;
    result.mode               = issues.empty() ? OpenMode::normalEdit : OpenMode::editWithWarnings;
    result.project            = summary;
    result.workspaceState     = workspaceState;
    result.activeScene        = sceneSummary;
    result.activeSceneMarkdown = std::move(mdR.value());
    result.scenes             = std::move(allScenes);
    result.restoredSelection  = restoredSelection;
    result.restoredScroll     = restoredScroll;
    result.repairIssues       = std::move(issues);

    return Result<OpenProjectResult>::success(std::move(result));
}

} // namespace scrivi::project_package
