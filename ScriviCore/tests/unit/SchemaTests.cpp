#include <catch2/catch_test_macros.hpp>

#include "schemas/ProjectJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "schemas/ProjectMembersJson.hpp"
#include "schemas/ProjectPersonasJson.hpp"
#include "schemas/WorkspaceStateJson.hpp"

using namespace scrivi;
using namespace scrivi::schemas;

// ---------------------------------------------------------------------------
// ProjectJson
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

// ---------------------------------------------------------------------------
// ManuscriptMetaJson
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// ChapterMetaJson
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// SceneMetaJson
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// ProjectMembersJson
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// ProjectPersonasJson
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// WorkspaceStateJson
// ---------------------------------------------------------------------------

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
