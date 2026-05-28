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
#include "schemas/ObjectJson.hpp"
#include "schemas/AssetMetaJson.hpp"
#include "schemas/CommentJson.hpp"

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

// ---------------------------------------------------------------------------
// ObjectJson round-trip tests (T-0034)
// ---------------------------------------------------------------------------

TEST_CASE("ObjectJson round-trips a minimal CharacterObject", "[schemas][T-0034]") {
    CharacterObject c;
    c.objectID.value         = "character_01ABC";
    c.slug                   = "ada";
    c.displayName            = "Ada Thornwood";
    c.status                 = "active";
    c.createdAt              = "2026-05-27T10:00:00Z";
    c.createdByIdentityID    = "identity_01";
    c.createdByPersonaID     = "persona_01";
    c.createdByDisplayName   = "Test Author";
    c.modifiedAt             = "2026-05-27T10:00:00Z";
    c.modifiedByIdentityID   = "identity_01";
    c.modifiedByPersonaID    = "persona_01";
    c.modifiedByDisplayName  = "Test Author";
    c.notes                  = "";

    const auto json = serializeCharacter(c);
    auto result = parseCharacter(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.objectID.value == "character_01ABC");
    REQUIRE(r.slug           == "ada");
    REQUIRE(r.displayName    == "Ada Thornwood");
    REQUIRE(r.status         == "active");
    REQUIRE(r.createdAt      == "2026-05-27T10:00:00Z");
    REQUIRE(r.createdByIdentityID  == "identity_01");
    REQUIRE(r.modifiedByDisplayName == "Test Author");
    REQUIRE(r.tags.empty());
    REQUIRE(r.attributes.empty());
}

TEST_CASE("ObjectJson round-trips a CharacterObject with tags and attributes", "[schemas][T-0034]") {
    CharacterObject c;
    c.objectID.value         = "character_02DEF";
    c.slug                   = "thomas";
    c.displayName            = "Thomas Belacroix";
    c.status                 = "active";
    c.createdAt              = "2026-05-27T11:00:00Z";
    c.createdByIdentityID    = "identity_02";
    c.createdByPersonaID     = "persona_02";
    c.createdByDisplayName   = "Author Two";
    c.modifiedAt             = "2026-05-27T12:00:00Z";
    c.modifiedByIdentityID   = "identity_02";
    c.modifiedByPersonaID    = "persona_02";
    c.modifiedByDisplayName  = "Author Two";
    c.notes                  = "Antagonist. Arrives in chapter 3.";
    c.tags                   = {"antagonist", "recurring"};
    c.attributes["age"]      = "47";
    c.attributes["faction"]  = "Obsidian Court";

    const auto json = serializeCharacter(c);
    auto result = parseCharacter(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.displayName == "Thomas Belacroix");
    REQUIRE(r.notes       == "Antagonist. Arrives in chapter 3.");
    REQUIRE(r.tags.size()       == 2);
    REQUIRE(r.tags[0]           == "antagonist");
    REQUIRE(r.tags[1]           == "recurring");
    REQUIRE(r.attributes.size() == 2);
    REQUIRE(r.attributes.at("age")     == "47");
    REQUIRE(r.attributes.at("faction") == "Obsidian Court");
}

TEST_CASE("ObjectJson rejects wrong schema tag", "[schemas][T-0034]") {
    auto result = parseCharacter(R"({"schema":"wrong.schema","objectID":"x"})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

// ---------------------------------------------------------------------------
// Remaining object type schema round-trip tests (T-0037)
// ---------------------------------------------------------------------------

static WorldObjectFields makeTestFields(const std::string& id = "obj-001") {
    WorldObjectFields f;
    f.objectID.value         = id;
    f.slug                   = "test-slug";
    f.displayName            = "Test Object";
    f.status                 = "active";
    f.createdAt              = "2026-05-28T00:00:00Z";
    f.createdByIdentityID    = "identity-001";
    f.createdByPersonaID     = "persona-001";
    f.createdByDisplayName   = "Test Author";
    f.modifiedAt             = "2026-05-28T00:00:00Z";
    f.modifiedByIdentityID   = "identity-001";
    f.modifiedByPersonaID    = "persona-001";
    f.modifiedByDisplayName  = "Test Author";
    f.notes                  = "Some notes.";
    f.tags                   = {"tag-a", "tag-b"};
    f.attributes             = {{"key1", "val1"}};
    return f;
}

TEST_CASE("LocationObject round-trips correctly", "[schemas][T-0037]") {
    LocationObject lo;
    static_cast<WorldObjectFields&>(lo) = makeTestFields("loc-001");
    lo.displayName = "Old Watchtower";

    const auto json = serializeLocation(lo);
    auto result = parseLocation(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.objectID.value == "loc-001");
    REQUIRE(r.displayName    == "Old Watchtower");
    REQUIRE(r.slug           == "test-slug");
    REQUIRE(r.tags.size()    == 2);
    REQUIRE(r.attributes.at("key1") == "val1");
}

TEST_CASE("ItemObject round-trips correctly", "[schemas][T-0037]") {
    ItemObject io;
    static_cast<WorldObjectFields&>(io) = makeTestFields("item-001");
    io.displayName = "Brass Key";

    const auto json = serializeItem(io);
    auto result = parseItem(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.objectID.value == "item-001");
    REQUIRE(r.displayName    == "Brass Key");
}

TEST_CASE("RuleObject round-trips correctly", "[schemas][T-0037]") {
    RuleObject ro;
    static_cast<WorldObjectFields&>(ro) = makeTestFields("rule-001");
    ro.displayName = "Magic System";

    const auto json = serializeRule(ro);
    auto result = parseRule(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.objectID.value == "rule-001");
    REQUIRE(r.displayName    == "Magic System");
}

TEST_CASE("TimelineObject round-trips correctly", "[schemas][T-0037]") {
    TimelineObject to;
    static_cast<WorldObjectFields&>(to) = makeTestFields("timeline-001");
    to.displayName = "Main Timeline";

    const auto json = serializeTimeline(to);
    auto result = parseTimeline(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.objectID.value == "timeline-001");
    REQUIRE(r.displayName    == "Main Timeline");
}

TEST_CASE("parseLocation rejects character schema tag", "[schemas][T-0037]") {
    LocationObject lo;
    static_cast<WorldObjectFields&>(lo) = makeTestFields();
    const auto json = serializeCharacter(
        []{ CharacterObject c; static_cast<WorldObjectFields&>(c) = makeTestFields(); return c; }());
    auto result = parseLocation(json);
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("serializeWorldObject / parseWorldObject round-trip for each kind",
          "[schemas][T-0037]")
{
    auto fields = makeTestFields("wobj-001");

    auto test = [&](ObjectKind kind, const std::string& expectedDisplay) {
        WorldObject obj;
        switch (kind) {
            case ObjectKind::character: { CharacterObject t; static_cast<WorldObjectFields&>(t) = fields; t.displayName = expectedDisplay; obj = t; break; }
            case ObjectKind::location:  { LocationObject  t; static_cast<WorldObjectFields&>(t) = fields; t.displayName = expectedDisplay; obj = t; break; }
            case ObjectKind::item:      { ItemObject      t; static_cast<WorldObjectFields&>(t) = fields; t.displayName = expectedDisplay; obj = t; break; }
            case ObjectKind::rule:      { RuleObject      t; static_cast<WorldObjectFields&>(t) = fields; t.displayName = expectedDisplay; obj = t; break; }
            case ObjectKind::timeline:  { TimelineObject  t; static_cast<WorldObjectFields&>(t) = fields; t.displayName = expectedDisplay; obj = t; break; }
        }
        const auto json = serializeWorldObject(obj);
        auto result = parseWorldObject(json, kind);
        REQUIRE(result.ok());
        REQUIRE(worldObjectFields(result.value()).displayName == expectedDisplay);
    };

    test(ObjectKind::character, "A Character");
    test(ObjectKind::location,  "A Location");
    test(ObjectKind::item,      "An Item");
    test(ObjectKind::rule,      "A Rule");
    test(ObjectKind::timeline,  "A Timeline");
}

// ---------------------------------------------------------------------------
// AssetMetaJson tests (T-0040)
// ---------------------------------------------------------------------------

TEST_CASE("AssetMetaJson round-trips a minimal AssetMeta", "[schemas][T-0040]") {
    AssetMeta meta;
    meta.assetID                    = "asset_01";
    meta.slug                       = "map-of-the-realm";
    meta.filename                   = "map.png";
    meta.category                   = AssetCategory::image;
    meta.mimeType                   = "image/png";
    meta.importedAt                 = "2026-05-28T10:00:00Z";
    meta.importedByIdentityID       = "identity_01";
    meta.importedByPersonaID        = "persona_01";
    meta.importedByDisplayName      = "Test Author";
    meta.title                      = "Map of the Realm";
    meta.notes                      = "";

    const auto json = serializeAssetMeta(meta);
    auto result = parseAssetMeta(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.assetID                   == "asset_01");
    REQUIRE(r.slug                      == "map-of-the-realm");
    REQUIRE(r.filename                  == "map.png");
    REQUIRE(r.category                  == AssetCategory::image);
    REQUIRE(r.mimeType                  == "image/png");
    REQUIRE(r.importedAt                == "2026-05-28T10:00:00Z");
    REQUIRE(r.importedByIdentityID      == "identity_01");
    REQUIRE(r.importedByPersonaID       == "persona_01");
    REQUIRE(r.importedByDisplayName     == "Test Author");
    REQUIRE(r.title                     == "Map of the Realm");
    REQUIRE(r.notes                     == "");
    REQUIRE(r.tags.empty());
}

TEST_CASE("AssetMetaJson round-trips with tags and all categories", "[schemas][T-0040]") {
    auto testCategory = [](AssetCategory cat, const std::string& mime) {
        AssetMeta meta;
        meta.assetID               = "asset_cat";
        meta.slug                  = "test-asset";
        meta.filename              = "file.bin";
        meta.category              = cat;
        meta.mimeType              = mime;
        meta.importedAt            = "2026-05-28T12:00:00Z";
        meta.importedByIdentityID  = "id";
        meta.importedByPersonaID   = "pid";
        meta.importedByDisplayName = "Author";
        meta.title                 = "Test";
        meta.tags                  = {"tag1", "tag2"};

        auto json   = serializeAssetMeta(meta);
        auto result = parseAssetMeta(json);
        REQUIRE(result.ok());
        REQUIRE(result.value().category    == cat);
        REQUIRE(result.value().tags.size() == 2);
        REQUIRE(result.value().tags[0]     == "tag1");
        REQUIRE(result.value().tags[1]     == "tag2");
    };

    testCategory(AssetCategory::image,    "image/png");
    testCategory(AssetCategory::audio,    "audio/mpeg");
    testCategory(AssetCategory::video,    "video/mp4");
    testCategory(AssetCategory::document, "application/pdf");
    testCategory(AssetCategory::other,    "application/octet-stream");
}

TEST_CASE("AssetMetaJson rejects wrong schema tag", "[schemas][T-0040]") {
    auto result = parseAssetMeta(
        R"({"schema":"wrong.schema","assetID":"x","slug":"","filename":"","category":"image","mimeType":"","importedAt":"","importedBy":{},"title":"","notes":"","tags":[]})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("AssetMetaJson rejects corrupt JSON", "[schemas][T-0040]") {
    auto result = parseAssetMeta("{not valid json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::parseError);
}

// ---------------------------------------------------------------------------
// CommentJson tests (T-0043)
// ---------------------------------------------------------------------------

TEST_CASE("CommentJson round-trips an empty thread", "[schemas][T-0043]") {
    CommentThread thread;
    thread.scopeKind = "scene";
    thread.targetID  = "scene_01";

    const auto json = serializeCommentThread(thread);
    auto result = parseCommentThread(json);
    REQUIRE(result.ok());
    REQUIRE(result.value().scopeKind     == "scene");
    REQUIRE(result.value().targetID      == "scene_01");
    REQUIRE(result.value().comments.empty());
}

TEST_CASE("CommentJson round-trips a thread with an unresolved comment", "[schemas][T-0043]") {
    Comment c;
    c.commentID              = "comment_01";
    c.body                   = "This paragraph needs revision.";
    c.resolved               = false;
    c.createdAt              = "2026-05-28T10:00:00Z";
    c.createdByIdentityID    = "identity_01";
    c.createdByPersonaID     = "persona_01";
    c.createdByDisplayName   = "Test Author";

    CommentThread thread;
    thread.scopeKind = "scene";
    thread.targetID  = "scene_01";
    thread.comments.push_back(c);

    const auto json = serializeCommentThread(thread);
    auto result = parseCommentThread(json);
    REQUIRE(result.ok());
    const auto& r = result.value();
    REQUIRE(r.comments.size()               == 1);
    REQUIRE(r.comments[0].commentID         == "comment_01");
    REQUIRE(r.comments[0].body              == "This paragraph needs revision.");
    REQUIRE(r.comments[0].resolved          == false);
    REQUIRE(r.comments[0].createdAt         == "2026-05-28T10:00:00Z");
    REQUIRE(r.comments[0].createdByIdentityID == "identity_01");
    REQUIRE(!r.comments[0].resolvedAt.has_value());
    REQUIRE(!r.comments[0].resolvedByIdentityID.has_value());
}

TEST_CASE("CommentJson round-trips a resolved comment", "[schemas][T-0043]") {
    Comment c;
    c.commentID              = "comment_02";
    c.body                   = "Fixed.";
    c.resolved               = true;
    c.createdAt              = "2026-05-28T10:00:00Z";
    c.createdByIdentityID    = "identity_01";
    c.createdByPersonaID     = "persona_01";
    c.createdByDisplayName   = "Author";
    c.resolvedAt             = "2026-05-28T11:00:00Z";
    c.resolvedByIdentityID   = "identity_02";
    c.resolvedByPersonaID    = "persona_02";
    c.resolvedByDisplayName  = "Reviewer";

    CommentThread thread;
    thread.scopeKind = "object";
    thread.targetID  = "character_01";
    thread.comments.push_back(c);

    const auto json = serializeCommentThread(thread);
    auto result = parseCommentThread(json);
    REQUIRE(result.ok());
    const auto& r = result.value().comments[0];
    REQUIRE(r.resolved                       == true);
    REQUIRE(r.resolvedAt.has_value());
    REQUIRE(r.resolvedAt.value()             == "2026-05-28T11:00:00Z");
    REQUIRE(r.resolvedByIdentityID.has_value());
    REQUIRE(r.resolvedByIdentityID.value()   == "identity_02");
    REQUIRE(r.resolvedByDisplayName.has_value());
    REQUIRE(r.resolvedByDisplayName.value()  == "Reviewer");
}

TEST_CASE("CommentJson rejects wrong schema tag", "[schemas][T-0043]") {
    auto result = parseCommentThread(
        R"({"schema":"wrong.schema","scopeKind":"scene","targetID":"x","comments":[]})");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::validationError);
}

TEST_CASE("CommentJson rejects corrupt JSON", "[schemas][T-0043]") {
    auto result = parseCommentThread("{not valid json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == ErrorCode::parseError);
}
