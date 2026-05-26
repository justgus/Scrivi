#include <catch2/catch_test_macros.hpp>
#include "util/Json.hpp"
#include "schemas/ProjectJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "schemas/ProjectMembersJson.hpp"
#include "schemas/ProjectPersonasJson.hpp"
#include "schemas/WorkspaceStateJson.hpp"
#include "schemas/SnapshotMetadataJson.hpp"
#include "schemas/RepairIssueJson.hpp"

using namespace scrivi;
using namespace scrivi::util;
using namespace scrivi::schemas;

TEST_CASE("parse valid JSON succeeds", "[Json]") {
    auto result = parseJson(R"({"title":"My Story","draft":true})");
    REQUIRE(result.ok());
    REQUIRE(result.value().getString("title") == "My Story");
    REQUIRE(result.value().getBool("draft") == true);
}

TEST_CASE("parse malformed JSON returns error", "[Json]") {
    auto result = parseJson("{not valid json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == scrivi::ErrorCode::parseError);
}

TEST_CASE("missing key returns default value", "[Json]") {
    auto result = parseJson(R"({"a":"b"})");
    REQUIRE(result.ok());
    REQUIRE(result.value().getString("missing", "default") == "default");
    REQUIRE(result.value().getBool("missing", false) == false);
}

TEST_CASE("set and dump round-trips string field", "[Json]") {
    auto result = parseJson("{}");
    REQUIRE(result.ok());
    auto doc = std::move(result.value());
    doc.setString("slug", "my-story");
    auto json = parseJson(doc.dump());
    REQUIRE(json.ok());
    REQUIRE(json.value().getString("slug") == "my-story");
}

TEST_CASE("contains returns correct presence", "[Json]") {
    auto result = parseJson(R"({"x":1})");
    REQUIRE(result.ok());
    REQUIRE(result.value().contains("x") == true);
    REQUIRE(result.value().contains("y") == false);
}

// ---------------------------------------------------------------------------
// Schema round-trip tests (merged from SchemaTests.cpp — T-0017)
// ---------------------------------------------------------------------------

TEST_CASE("ProjectJson round-trips required fields", "[schemas]") {
    ProjectJsonData d;
    d.projectID.value        = "proj-001";
    d.title                  = "The Door Beneath the Hill";
    d.slug                   = "the-door-beneath-the-hill";
    d.createdAt              = "2026-05-19T12:00:00Z";
    d.createdByIdentityID    = "identity-001";
    d.createdByPersonaID     = "persona-001";
    d.createdByDisplayName   = "Rhozwyn Darius";
    d.manuscriptPath         = "manuscript/manuscript.meta.json";
    d.membersPath            = "identities/project-members.json";
    d.personasPath           = "identities/project-personas.json";
    d.gitSnapshotsEnabled    = false;

    auto json   = serializeProject(d);
    auto result = parseProject(json);
    REQUIRE(result.ok());
    auto& p = result.value();
    REQUIRE(p.projectID.value == "proj-001");
    REQUIRE(p.title           == "The Door Beneath the Hill");
    REQUIRE(p.slug            == "the-door-beneath-the-hill");
    REQUIRE(p.createdAt       == "2026-05-19T12:00:00Z");
    REQUIRE(p.manuscriptPath  == "manuscript/manuscript.meta.json");
    REQUIRE(p.membersPath     == "identities/project-members.json");
    REQUIRE(p.personasPath    == "identities/project-personas.json");
    REQUIRE(p.gitSnapshotsEnabled == false);
}

TEST_CASE("ProjectJson rejects wrong schema tag", "[schemas]") {
    auto result = parseProject(R"({"schema":"scrivi.chapter.v1","projectID":"x","title":"y","createdAt":"z"})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("ProjectJson rejects corrupt JSON", "[schemas]") {
    auto result = parseProject("{not json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::parseError);
}

TEST_CASE("ProjectJson rejects missing required field", "[schemas]") {
    auto result = parseProject(R"({"schema":"scrivi.project.v1","title":"T","createdAt":"now"})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("ManuscriptMetaJson round-trips with chapters", "[schemas]") {
    ManuscriptMetaData d;
    d.manuscriptID.value     = "ms-001";
    d.title                  = "The Door Beneath the Hill";
    d.createdAt              = "2026-05-19T12:00:00Z";
    d.createdByIdentityID    = "identity-001";
    d.createdByPersonaID     = "persona-001";
    d.createdByDisplayName   = "Rhozwyn Darius";
    d.chapters.push_back({ChapterID{"ch-001"}, "chapter-001/chapter.meta.json"});

    auto json   = serializeManuscriptMeta(d);
    auto result = parseManuscriptMeta(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().manuscriptID.value == "ms-001");
    REQUIRE(result.value().title              == "The Door Beneath the Hill");
    REQUIRE(result.value().chapters.size()    == 1);
    REQUIRE(result.value().chapters[0].chapterID.value == "ch-001");
    REQUIRE(result.value().chapters[0].path   == "chapter-001/chapter.meta.json");
}

TEST_CASE("ManuscriptMetaJson rejects corrupt JSON", "[schemas]") {
    REQUIRE_FALSE(parseManuscriptMeta("{bad}").ok());
}

TEST_CASE("ChapterMetaJson round-trips with scenes", "[schemas]") {
    ChapterMetaData d;
    d.chapterID.value        = "ch-001";
    d.title                  = "Chapter 1";
    d.slug                   = "chapter-001";
    d.displayLabel           = "Chapter 1";
    d.status                 = "draft";
    d.createdAt              = "2026-05-19T12:00:00Z";
    d.createdByIdentityID    = "identity-001";
    d.createdByPersonaID     = "persona-001";
    d.createdByDisplayName   = "Rhozwyn Darius";
    d.scenes.push_back({SceneID{"scene-001"}, "001-opening-scene.meta.json"});

    auto json   = serializeChapterMeta(d);
    auto result = parseChapterMeta(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().chapterID.value     == "ch-001");
    REQUIRE(result.value().status              == "draft");
    REQUIRE(result.value().scenes.size()       == 1);
    REQUIRE(result.value().scenes[0].sceneID.value == "scene-001");
}

TEST_CASE("ChapterMetaJson rejects missing required field", "[schemas]") {
    auto result = parseChapterMeta(R"({"schema":"scrivi.chapter.v1","chapterID":"x"})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("SceneMetaJson round-trips with stats", "[schemas]") {
    SceneMetaData d;
    d.sceneID.value          = "scene-001";
    d.title                  = "Opening Scene";
    d.slug                   = "001-opening-scene";
    d.status                 = "draft";
    d.createdAt              = "2026-05-19T12:00:00Z";
    d.createdByIdentityID    = "identity-001";
    d.createdByPersonaID     = "persona-001";
    d.createdByDisplayName   = "Rhozwyn Darius";
    d.modifiedAt             = "2026-05-19T12:00:00Z";
    d.modifiedByIdentityID   = "identity-001";
    d.modifiedByPersonaID    = "persona-001";
    d.modifiedByDisplayName  = "Rhozwyn Darius";
    d.contentPath            = "001-opening-scene.md";
    d.wordCount              = 42;
    d.characterCount         = 200;

    auto json   = serializeSceneMeta(d);
    auto result = parseSceneMeta(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().sceneID.value  == "scene-001");
    REQUIRE(result.value().title          == "Opening Scene");
    REQUIRE(result.value().contentPath    == "001-opening-scene.md");
    REQUIRE(result.value().wordCount      == 42);
    REQUIRE(result.value().characterCount == 200);
}

TEST_CASE("SceneMetaJson rejects corrupt JSON", "[schemas]") {
    REQUIRE_FALSE(parseSceneMeta("not json").ok());
}

TEST_CASE("ProjectMembersJson round-trips members", "[schemas]") {
    ProjectMembersData d;
    MemberEntry m;
    m.identityID.value = "identity-001";
    m.role             = "owner";
    m.status           = "active";
    m.defaultPersonaID = "persona-001";
    m.joinedAt         = "2026-05-19T12:00:00Z";
    d.members.push_back(m);

    auto json   = serializeProjectMembers(d);
    auto result = parseProjectMembers(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().members.size()               == 1);
    REQUIRE(result.value().members[0].identityID.value  == "identity-001");
    REQUIRE(result.value().members[0].role              == "owner");
    REQUIRE(result.value().members[0].status            == "active");
}

TEST_CASE("ProjectMembersJson rejects missing members field", "[schemas]") {
    auto result = parseProjectMembers(R"({"schema":"scrivi.projectMembers.v1"})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("ProjectPersonasJson round-trips personas", "[schemas]") {
    ProjectPersonasData d;
    PersonaEntry p;
    p.personaID.value          = "persona-001";
    p.displayName              = "Rhozwyn Darius";
    p.personaKind              = "individual";
    p.controlledByIdentityID   = "identity-001";
    p.createdAt                = "2026-05-19T12:00:00Z";
    p.status                   = "active";
    d.personas.push_back(p);

    auto json   = serializeProjectPersonas(d);
    auto result = parseProjectPersonas(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().personas.size()                     == 1);
    REQUIRE(result.value().personas[0].personaID.value         == "persona-001");
    REQUIRE(result.value().personas[0].displayName             == "Rhozwyn Darius");
    REQUIRE(result.value().personas[0].controlledByIdentityID  == "identity-001");
}

TEST_CASE("WorkspaceStateJson round-trips with last writing surface", "[schemas]") {
    WorkspaceStateData d;
    d.projectID.value      = "proj-001";
    d.deviceID             = "device-001";
    d.identityID           = "identity-001";
    d.activePersonaID      = "persona-001";
    d.lastOpenedAt         = "2026-05-19T12:00:00Z";
    d.hasLastWritingSurface= true;
    d.lastSceneID          = "scene-001";
    d.lastContentPath      = "manuscript/chapter-001/001-opening-scene.md";
    d.cursorAnchor         = 10;
    d.cursorFocus          = 10;
    d.scrollPosition       = 0.5;

    auto json   = serializeWorkspaceState(d);
    auto result = parseWorkspaceState(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().projectID.value       == "proj-001");
    REQUIRE(result.value().hasLastWritingSurface  == true);
    REQUIRE(result.value().lastSceneID            == "scene-001");
    REQUIRE(result.value().cursorAnchor           == 10);
}

TEST_CASE("WorkspaceStateJson round-trips without last writing surface", "[schemas]") {
    WorkspaceStateData d;
    d.projectID.value      = "proj-001";
    d.deviceID             = "device-001";
    d.identityID           = "identity-001";
    d.activePersonaID      = "persona-001";
    d.lastOpenedAt         = "2026-05-19T12:00:00Z";
    d.hasLastWritingSurface= false;

    auto json   = serializeWorkspaceState(d);
    auto result = parseWorkspaceState(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().hasLastWritingSurface == false);
}

TEST_CASE("WorkspaceStateJson rejects corrupt JSON", "[schemas]") {
    REQUIRE_FALSE(parseWorkspaceState("{bad}").ok());
}

// ---------------------------------------------------------------------------
// SnapshotMetadataJson tests (T-0020)
// ---------------------------------------------------------------------------

TEST_CASE("SnapshotMetadataJson round-trips with one entry", "[schemas]") {
    SnapshotMetadataData d;
    SnapshotEntryData e;
    e.snapshotID             = "snap-001";
    e.commitID               = "abc123";
    e.label                  = "Chapter 1 draft";
    e.note                   = "Before the big edit";
    e.createdAt              = "2026-05-26T10:00:00Z";
    e.createdByIdentityID    = "identity-001";
    e.createdByPersonaID     = "persona-001";
    e.createdByDisplayName   = "Rhozwyn Darius";
    d.snapshots.push_back(e);

    auto json   = serializeSnapshotMetadata(d);
    auto result = parseSnapshotMetadata(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().snapshots.size()                     == 1);
    REQUIRE(result.value().snapshots[0].snapshotID              == "snap-001");
    REQUIRE(result.value().snapshots[0].commitID                == "abc123");
    REQUIRE(result.value().snapshots[0].label                   == "Chapter 1 draft");
    REQUIRE(result.value().snapshots[0].note                    == "Before the big edit");
    REQUIRE(result.value().snapshots[0].createdAt               == "2026-05-26T10:00:00Z");
    REQUIRE(result.value().snapshots[0].createdByIdentityID     == "identity-001");
    REQUIRE(result.value().snapshots[0].createdByPersonaID      == "persona-001");
    REQUIRE(result.value().snapshots[0].createdByDisplayName    == "Rhozwyn Darius");
}

TEST_CASE("SnapshotMetadataJson round-trips empty snapshots array", "[schemas]") {
    SnapshotMetadataData d; // no entries
    auto json   = serializeSnapshotMetadata(d);
    auto result = parseSnapshotMetadata(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().snapshots.empty());
}

TEST_CASE("SnapshotMetadataJson rejects corrupt JSON", "[schemas]") {
    auto result = parseSnapshotMetadata("{not valid json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::parseError);
}

// ---------------------------------------------------------------------------
// RepairIssueJson tests (T-0028)
// ---------------------------------------------------------------------------

TEST_CASE("RepairIssueJson round-trips empty issue list", "[schemas]") {
    std::vector<RepairIssue> issues;
    auto json   = serializeRepairIssues(issues);
    auto result = parseRepairIssues(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().empty());
}

TEST_CASE("RepairIssueJson round-trips missingContent issue with one action", "[schemas]") {
    RepairIssue issue;
    issue.issueID     = "issue-001";
    issue.severity    = RepairSeverity::blocking;
    issue.category    = RepairCategory::missingContent;
    issue.title       = "Missing content file";
    issue.message     = "The .md file for this scene cannot be found.";
    issue.path        = "manuscript/ch1/001-opening.md";
    issue.relatedPath = "manuscript/ch1/001-opening.meta.json";
    issue.projectID.value = "proj-001";
    issue.chapterID.value = "ch-001";
    issue.sceneID.value   = "scene-001";

    RepairAction action;
    action.kind   = RepairActionKind::createEmptyContentFile;
    action.label  = "Create empty file";
    action.detail = "Writes a blank .md file at the expected path.";
    issue.suggestedActions.push_back(action);

    auto json   = serializeRepairIssues({issue});
    auto result = parseRepairIssues(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().size() == 1);

    const auto& r = result.value()[0];
    REQUIRE(r.issueID       == "issue-001");
    REQUIRE(r.severity      == RepairSeverity::blocking);
    REQUIRE(r.category      == RepairCategory::missingContent);
    REQUIRE(r.title         == "Missing content file");
    REQUIRE(r.message       == "The .md file for this scene cannot be found.");
    REQUIRE(r.path          == "manuscript/ch1/001-opening.md");
    REQUIRE(r.relatedPath   == "manuscript/ch1/001-opening.meta.json");
    REQUIRE(r.projectID.value == "proj-001");
    REQUIRE(r.chapterID.value == "ch-001");
    REQUIRE(r.sceneID.value   == "scene-001");
    REQUIRE(r.suggestedActions.size() == 1);
    REQUIRE(r.suggestedActions[0].kind  == RepairActionKind::createEmptyContentFile);
    REQUIRE(r.suggestedActions[0].label == "Create empty file");
}

TEST_CASE("RepairIssueJson round-trips corruptMetadata issue with multiple actions", "[schemas]") {
    RepairIssue issue;
    issue.issueID  = "issue-002";
    issue.severity = RepairSeverity::warning;
    issue.category = RepairCategory::corruptMetadata;
    issue.title    = "Corrupt metadata";
    issue.message  = "The .meta.json file cannot be parsed.";
    issue.path     = "manuscript/ch1/001-opening.meta.json";

    RepairAction a1;
    a1.kind   = RepairActionKind::regenerateMetadata;
    a1.label  = "Regenerate";
    a1.detail = "Rebuilds metadata from file name.";

    RepairAction a2;
    a2.kind   = RepairActionKind::removeFromProject;
    a2.label  = "Remove from project";
    a2.detail = "Removes the entry from the chapter.";

    issue.suggestedActions = {a1, a2};

    auto json   = serializeRepairIssues({issue});
    auto result = parseRepairIssues(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().size() == 1);

    const auto& r = result.value()[0];
    REQUIRE(r.category == RepairCategory::corruptMetadata);
    REQUIRE(r.suggestedActions.size() == 2);
    REQUIRE(r.suggestedActions[0].kind == RepairActionKind::regenerateMetadata);
    REQUIRE(r.suggestedActions[1].kind == RepairActionKind::removeFromProject);
}

TEST_CASE("RepairIssueJson round-trips unregisteredManuscriptFile issue", "[schemas]") {
    RepairIssue issue;
    issue.issueID  = "issue-003";
    issue.severity = RepairSeverity::info;
    issue.category = RepairCategory::unregisteredManuscriptFile;
    issue.title    = "Unregistered file";
    issue.path     = "manuscript/ch1/stray-file.md";

    RepairAction a;
    a.kind   = RepairActionKind::importAsNewScene;
    a.label  = "Import as new scene";
    a.detail = "Registers this file as a new scene in the chapter.";
    issue.suggestedActions.push_back(a);

    auto json   = serializeRepairIssues({issue});
    auto result = parseRepairIssues(json);
    REQUIRE(result.ok());
    const auto& r = result.value()[0];
    REQUIRE(r.category == RepairCategory::unregisteredManuscriptFile);
    REQUIRE(r.severity == RepairSeverity::info);
    REQUIRE(r.suggestedActions[0].kind == RepairActionKind::importAsNewScene);
}

TEST_CASE("RepairIssueJson rejects corrupt JSON", "[schemas]") {
    auto result = parseRepairIssues("{not valid json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::parseError);
}

TEST_CASE("RepairIssueJson rejects wrong schema tag", "[schemas]") {
    auto result = parseRepairIssues(
        R"({"schema":"wrong-schema","schemaVersion":1,"issues":[]})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}
