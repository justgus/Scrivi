// EP-031 SP-096 — the relationship graph.
// Design: Worldbuilding Object Model v0.2 §5.1–§5.4, §9 AC4/AC5/AC9.
//
// The load-bearing assertions here are §9 AC4's two duplicate cases. AC4 is
// explicit that "the symmetric case is the one that regresses silently," so
// both an asymmetric cross-kind type and a symmetric same-kind type are created
// from BOTH ends.

#include <catch2/catch_test_macros.hpp>

#include "objects/EndpointResolver.hpp"
#include "objects/RelationTypes.hpp"
#include "objects/RelationshipStore.hpp"
#include "worlds/WorldStore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iterator>
#include <string>

namespace fs = std::filesystem;
using namespace scrivi;
using namespace scrivi::objects;

namespace {

struct GraphFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    platform::LocalFileSystem        fileSystem;
    mocks::DeterministicUUIDProvider uuidProvider;
    mocks::FixedClock                clock{"2026-08-12T00:00:00Z"};
    mocks::MockGitProvider           gitProvider;
    mocks::MockSecureStore           secureStore;
    CoreServices                     services;
    ScriviCore                       core;

    SceneID firstSceneID;
    // SP-103: worldbuilding kinds are world-scoped, so the graph fixture needs a
    // world before it can create the objects it relates.
    std::string worldID;

    const AuthorshipRef author{
        IdentityID{"identity-001"}, PersonaID{"persona-001"}, "Test Author"};

    GraphFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-graph-" + std::to_string(
                         std::chrono::steady_clock::now().time_since_epoch().count())))
        , appSupportDir(projectDir / "appsupport")
        , services([&]{
            CoreServices svc;
            svc.fileSystem   = &fileSystem;
            svc.uuidProvider = &uuidProvider;
            svc.clock        = &clock;
            svc.gitProvider  = &gitProvider;
            svc.secureStore  = &secureStore;
            svc.logger       = nullptr;
            return svc;
          }())
        , core(services)
    {
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);

        CreateProjectRequest req;
        req.projectRootPath = projectDir.string();
        req.appSupportRoot  = appSupportDir.string();
        req.title           = "Graph Test Project";
        req.slug            = "graph-test";
        req.author          = author;
        auto r = core.createProject(req);
        REQUIRE(r.ok());
        firstSceneID = r.value().firstSceneID;

        worlds::WorldStore ws{services};
        auto w = ws.createWorld(projectDir.string(),
                                (projectDir / "Graph.scrivworld").string(),
                                "Graph World", "");
        REQUIRE(w.ok());
        worldID = w.value().worldID;
    }

    ~GraphFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
    }

    [[nodiscard]] std::string root() const { return projectDir.string(); }

    [[nodiscard]] fs::path logPath() const {
        return projectDir / "objects" / "relationships.jsonl";
    }

    [[nodiscard]] std::string readLog() const {
        std::ifstream in(logPath());
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }

    ObjectID makeObject(ObjectKind kind, const std::string& name, const std::string& slug) {
        CreateObjectRequest req;
        req.projectRootPath = root();
        req.objectKind      = kind;
        req.displayName     = name;
        req.slug            = slug;
        req.author          = author;
        // Every worldbuilding kind is world-scoped (SP-103); `source` is not.
        if (objectKindIsWorldScoped(kind)) { req.worldID = worldID; }
        auto r = core.createObject(req);
        REQUIRE(r.ok());
        return r.value().objectID;
    }
};

} // namespace

// ---------------------------------------------------------------------------
// T-0402 — endpoint resolution (the ID-prefix rule replacement)
// ---------------------------------------------------------------------------

TEST_CASE("endpoint kind comes from the index, NOT from an ID prefix",
          "[integration][T-0402]") {
    GraphFixture fix;
    EndpointResolver resolver{fix.services};

    auto locID = fix.makeObject(ObjectKind::location, "The Vault", "vault");

    // ⚠️ THE REGRESSION GUARD. SystemUUIDProvider::newObjectID() mints
    // "character_<uuid>" for EVERY kind, so a location's ID begins "character_"
    // in production. Doc 1 §5.2's original "distinguish by ID prefix" rule would
    // classify this object as a character — silently. Resolution must come from
    // objects/index.json instead.
    auto ep = resolver.resolve(fix.root(), locID.value);
    REQUIRE(ep.found);
    REQUIRE_FALSE(ep.isScene);
    REQUIRE(ep.kind == ObjectKind::location);      // NOT character
    REQUIRE(ep.displayName == "The Vault");
}

TEST_CASE("a scene endpoint resolves through the manuscript path",
          "[integration][T-0402]") {
    GraphFixture fix;
    EndpointResolver resolver{fix.services};

    auto ep = resolver.resolve(fix.root(), fix.firstSceneID.value);
    REQUIRE(ep.found);
    REQUIRE(ep.isScene);
}

TEST_CASE("an unknown endpoint is unresolved, not an error",
          "[integration][T-0402]") {
    GraphFixture fix;
    EndpointResolver resolver{fix.services};

    REQUIRE_FALSE(resolver.resolve(fix.root(), "no-such-id").found);
    REQUIRE_FALSE(resolver.resolve(fix.root(), "").found);
}

// ---------------------------------------------------------------------------
// T-0373 — relation types
// ---------------------------------------------------------------------------

TEST_CASE("a new project seeds the four relation types", "[integration][T-0373]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};

    auto r = store.load(fix.root());
    REQUIRE(r.ok());
    REQUIRE(r.value().size() == 4);

    for (const char* code : {"appears-in", "located-at", "sibling-of", "cites"}) {
        CAPTURE(code);
        REQUIRE(store.find(fix.root(), code).ok());
    }
}

TEST_CASE("cites is unconstrained on BOTH ends", "[integration][T-0373]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};

    auto r = store.find(fix.root(), "cites");
    REQUIRE(r.ok());
    const auto& t = r.value();

    // §3.4: a citation may document any kind, so constraining either end is
    // wrong. "Any" is encoded as an ABSENT key — JsonDoc cannot distinguish an
    // absent key from an explicit null.
    REQUIRE_FALSE(t.sourceKind.has_value());
    REQUIRE_FALSE(t.targetKind.has_value());
    REQUIRE_FALSE(t.sourceIsScene);
    REQUIRE_FALSE(t.targetIsScene);
    REQUIRE(t.forwardLabel == "cites");
    REQUIRE(t.inverseLabel == "documented by");
}

TEST_CASE("a missing or corrupt relation-types.json is re-seeded",
          "[integration][T-0373]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};
    const auto p = fs::path(RelationTypeStore::path(fix.root()));

    SECTION("deleted")   { fs::remove(p); }
    SECTION("malformed") { std::ofstream(p, std::ios::trunc) << "{ not json"; }
    SECTION("wrong schema") {
        std::ofstream(p, std::ios::trunc) << R"({"schema":"other.v1","types":[]})";
    }

    // A project must always be able to relate things.
    auto r = store.load(fix.root());
    REQUIRE(r.ok());
    REQUIRE(r.value().size() == 4);
}

TEST_CASE("relation-type validation protects the duplicate rule",
          "[integration][T-0373]") {
    RelationType t;
    t.code         = "allied-with";
    t.forwardLabel = "allied with";
    t.inverseLabel = "allied with";
    t.symmetric    = true;

    // A symmetric type declaring source-to-target would normalize "A rel B" and
    // "B rel A" to two DIFFERENT canonical edges — reintroducing the very
    // duplication §5.3 exists to prevent.
    t.canonicalDirection = CanonicalDirection::sourceToTarget;
    REQUIRE_FALSE(RelationTypeStore::validate(t).ok());

    t.canonicalDirection = CanonicalDirection::lexical;
    REQUIRE(RelationTypeStore::validate(t).ok());

    // Symmetric types must read identically from both ends.
    t.inverseLabel = "different";
    REQUIRE_FALSE(RelationTypeStore::validate(t).ok());

    // And an asymmetric type cannot be lexical — sorting would discard the
    // direction that distinguishes its two labels.
    RelationType a;
    a.code = "cites"; a.forwardLabel = "cites"; a.inverseLabel = "documented by";
    a.canonicalDirection = CanonicalDirection::lexical;
    REQUIRE_FALSE(RelationTypeStore::validate(a).ok());
}

// ---------------------------------------------------------------------------
// T-0441 (SP-118) — ⚠️ T-0416: seeded relation types must reach EXISTING projects
//
// ⚠️ THESE TESTS MUST RUN AGAINST A DELIBERATELY DRIFTED FIXTURE. A fresh project
// is already correct, so a test that merely seeds normally passes VACUOUSLY —
// precisely how this defect survived from 2026-08-17 to 2026-08-21 with a green
// suite. `writeDriftedVocabulary` reproduces the exact bytes found on the rig in
// `the-twisted-remains-of-myself.scrivi`.
// ---------------------------------------------------------------------------

namespace {

// The pre-I-0125 `appears-in`, verbatim: `sourceKind: "character"` (the constraint
// I-0125 REMOVED) and the kind-specific `inverseLabel: "has characters"`. Carries
// a writer-authored type alongside it so the same fixture proves S2.
void writeDriftedVocabulary(const fs::path& p) {
    std::ofstream(p, std::ios::trunc) << R"({
  "schema": "scrivi.relation-types.v1",
  "types": [
    {
      "code": "appears-in",
      "forwardLabel": "appears in",
      "inverseLabel": "has characters",
      "sourceKind": "character",
      "targetKind": "scene",
      "canonicalDirection": "source-to-target",
      "symmetric": false
    },
    {
      "code": "located-at",
      "forwardLabel": "takes place at",
      "inverseLabel": "hosts",
      "sourceKind": "scene",
      "targetKind": "location",
      "canonicalDirection": "source-to-target",
      "symmetric": false
    },
    {
      "code": "sibling-of",
      "forwardLabel": "sibling of",
      "inverseLabel": "sibling of",
      "sourceKind": "character",
      "targetKind": "character",
      "canonicalDirection": "lexical",
      "symmetric": true
    },
    {
      "code": "cites",
      "forwardLabel": "cites",
      "inverseLabel": "documented by",
      "canonicalDirection": "source-to-target",
      "symmetric": false
    },
    {
      "code": "sworn-enemy-of",
      "forwardLabel": "sworn enemy of",
      "inverseLabel": "sworn enemy of",
      "canonicalDirection": "lexical",
      "symmetric": true
    }
  ]
})";
}

} // namespace

TEST_CASE("S1 — a DRIFTED seeded type is repaired on open, and the relate it "
          "broke then succeeds", "[integration][T-0441]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};
    const auto p = fs::path(RelationTypeStore::path(fix.root()));

    writeDriftedVocabulary(p);

    // ⚠️ Precondition: the fixture really is broken. Without this the test could
    // pass against an unchanged core simply because the drift never took.
    {
        auto raw = std::ifstream(p);
        const std::string text{std::istreambuf_iterator<char>(raw),
                               std::istreambuf_iterator<char>()};
        REQUIRE(text.find("has characters") != std::string::npos);
        REQUIRE(text.find("\"sourceKind\": \"character\"") != std::string::npos);
    }

    auto loaded = store.load(fix.root());
    REQUIRE(loaded.ok());

    // Repaired IN MEMORY …
    auto found = store.find(fix.root(), "appears-in");
    REQUIRE(found.ok());
    REQUIRE(found.value().inverseLabel == "features");
    REQUIRE_FALSE(found.value().sourceKind.has_value());   // I-0125's removal
    REQUIRE_FALSE(found.value().sourceIsScene);
    REQUIRE(found.value().targetIsScene);

    // … and ON DISK, so the repair survives a reopen rather than being recomputed
    // every time (the file, not a cache, is what the writer's other tools read).
    {
        auto raw = std::ifstream(p);
        const std::string text{std::istreambuf_iterator<char>(raw),
                               std::istreambuf_iterator<char>()};
        REQUIRE(text.find("has characters") == std::string::npos);
    }

    // ⚠️ THE ACTUAL SYMPTOM. `appears-in` is what EIGHT of the ten object cards
    // use; against the drifted vocabulary this failed with "endpoints do not
    // satisfy the kind constraints of relation type 'appears-in'" — after the
    // object had already been written to disk.
    //
    // ⚠️ This is the assertion that FAILS against the un-fixed core.
    RelationshipStore edges{fix.services};
    auto chronicleID = fix.makeObject(ObjectKind::chronicle, "The Long Winter",
                                      "long-winter");
    auto edge = edges.create(fix.root(), chronicleID.value,
                             fix.firstSceneID.value, "appears-in", "");
    REQUIRE(edge.ok());
}

TEST_CASE("S2 — reconciliation leaves writer-authored types alone and deletes "
          "nothing", "[integration][T-0441]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};

    writeDriftedVocabulary(fs::path(RelationTypeStore::path(fix.root())));

    auto loaded = store.load(fix.root());
    REQUIRE(loaded.ok());

    // ⚠️ The file is writer-editable by design. A type this build does not seed is
    // either hers or a LATER build's — removing it would destroy her vocabulary.
    auto mine = store.find(fix.root(), "sworn-enemy-of");
    REQUIRE(mine.ok());
    REQUIRE(mine.value().forwardLabel == "sworn enemy of");
    REQUIRE(mine.value().symmetric);
    REQUIRE(mine.value().canonicalDirection == CanonicalDirection::lexical);

    // 4 seeded + 1 authored. Nothing added twice, nothing dropped.
    REQUIRE(loaded.value().size() == 5);
}

TEST_CASE("S3 — a project whose vocabulary is already current is NOT rewritten",
          "[integration][T-0441]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};
    const auto p = fs::path(RelationTypeStore::path(fix.root()));

    // createProject already seeded it, so this project is current by construction.
    REQUIRE(store.load(fix.root()).ok());

    const auto before = fs::last_write_time(p);

    // ⚠️ S1 alone would pass with an unconditional rewrite. S3 is what stops the
    // fix from touching relation-types.json on EVERY open — churning mtimes and
    // Git status in every project, forever, for no reason.
    for (int i = 0; i < 3; ++i) {
        REQUIRE(store.load(fix.root()).ok());
    }

    REQUIRE((fs::last_write_time(p) == before));
}

TEST_CASE("S1b — a MISSING seeded type is restored without disturbing the rest",
          "[integration][T-0441]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};
    const auto p = fs::path(RelationTypeStore::path(fix.root()));

    // A valid file that simply lacks `cites` — the "seeded code absent" branch,
    // which drift produces when a seed type is ADDED by a later build.
    std::ofstream(p, std::ios::trunc) << R"({
  "schema": "scrivi.relation-types.v1",
  "types": [
    {
      "code": "appears-in",
      "forwardLabel": "appears in",
      "inverseLabel": "features",
      "targetKind": "scene",
      "canonicalDirection": "source-to-target",
      "symmetric": false
    }
  ]
})";

    auto loaded = store.load(fix.root());
    REQUIRE(loaded.ok());
    REQUIRE(loaded.value().size() == 4);
    for (const char* code : {"appears-in", "located-at", "sibling-of", "cites"}) {
        CAPTURE(code);
        REQUIRE(store.find(fix.root(), code).ok());
    }
}

// ---------------------------------------------------------------------------
// I-0149 — ⚠️ the reconciliation must run ON OPEN, not merely on read
//
// ⚠️ T-0441's tests all called `store.load()` DIRECTLY. That proved the repair
// worked and proved NOTHING about whether anything invokes it when a project
// opens — which is what the 2026-08-21 ruling actually said. It does not: `load()`
// runs only when something asks for the vocabulary, and opening a project asks for
// none of those.
//
// ⚠️ Found on the real rig. `the-twisted-remains-of-myself.scrivi` opened cleanly
// with the fix compiled into the running app and its pre-I-0125 `appears-in`
// untouched — empty object index, no edge log, so nothing ever read the file.
//
// ⚠️ THIS TEST FAILS against T-0441-as-shipped. It is the one that was missing.
// ---------------------------------------------------------------------------

TEST_CASE("I-0149 — opening a project repairs a drifted vocabulary with NOTHING "
          "else touching it", "[integration][I-0149]") {
    GraphFixture fix;
    const auto p = fs::path(RelationTypeStore::path(fix.root()));

    writeDriftedVocabulary(p);

    // ⚠️ Precondition: the drift really is on disk.
    {
        std::ifstream in(p);
        const std::string text{std::istreambuf_iterator<char>(in),
                               std::istreambuf_iterator<char>()};
        REQUIRE(text.find("has characters") != std::string::npos);
    }

    // ⚠️ THE WHOLE POINT: open the project, and touch the vocabulary in no other
    // way. No RelationTypeStore, no find(), no createEdge — exactly what the app
    // does when a writer opens a project with no objects and no edges.
    OpenProjectRequest req;
    req.projectRootPath = fix.root();
    req.appSupportRoot  = fix.appSupportDir.string();
    auto opened = fix.core.openProject(req);
    REQUIRE(opened.ok());

    // Repaired ON DISK by the act of opening.
    {
        std::ifstream in(p);
        const std::string text{std::istreambuf_iterator<char>(in),
                               std::istreambuf_iterator<char>()};
        REQUIRE(text.find("has characters") == std::string::npos);
        REQUIRE(text.find("features") != std::string::npos);
    }

    // ⚠️ And the writer-authored type still survives the open — reconciliation
    // reaching a new call site must not have changed what it does.
    RelationTypeStore store{fix.services};
    REQUIRE(store.find(fix.root(), "sworn-enemy-of").ok());
}

TEST_CASE("I-0149 — opening a CURRENT project still does not rewrite the file",
          "[integration][I-0149]") {
    GraphFixture fix;
    const auto p = fs::path(RelationTypeStore::path(fix.root()));

    // ⚠️ S3's guarantee has to survive the new call site. Reconciling on EVERY
    // open is exactly the churn S3 exists to prevent — every project's
    // relation-types.json touched on every open, forever.
    const auto before = fs::last_write_time(p);

    for (int i = 0; i < 3; ++i) {
        OpenProjectRequest req;
        req.projectRootPath = fix.root();
        req.appSupportRoot  = fix.appSupportDir.string();
        REQUIRE(fix.core.openProject(req).ok());
    }

    REQUIRE((fs::last_write_time(p) == before));
}

// ⚠️ RIG CHECK — opt-in, hidden by default ([.rig]): points the real open path at
// a COPY of an actual project on disk. Run with:
//   ScriviCoreTests "[rig]" -- (env SCRIVI_RIG_PROJECT=/path/to/copy.scrivi)
// Skipped silently when the env var is unset, so CI is unaffected.
TEST_CASE("rig — opening a real drifted project repairs it", "[.rig]") {
    const char* rigPath = std::getenv("SCRIVI_RIG_PROJECT");
    if (rigPath == nullptr) { SUCCEED("SCRIVI_RIG_PROJECT unset — skipped"); return; }

    const auto appSupport = fs::temp_directory_path() / "scrivi-rig-appsupport";
    fs::create_directories(appSupport);

    platform::LocalFileSystem        fileSystem;
    mocks::DeterministicUUIDProvider uuidProvider;
    mocks::FixedClock                clock{"2026-08-22T00:00:00Z"};
    mocks::MockGitProvider           gitProvider;
    mocks::MockSecureStore           secureStore;
    CoreServices svc;
    svc.fileSystem = &fileSystem; svc.uuidProvider = &uuidProvider;
    svc.clock = &clock; svc.gitProvider = &gitProvider;
    svc.secureStore = &secureStore; svc.logger = nullptr;
    ScriviCore core{svc};

    OpenProjectRequest req;
    req.projectRootPath = rigPath;
    req.appSupportRoot  = appSupport.string();
    auto opened = core.openProject(req);
    INFO("openProject: " << (opened.ok() ? "ok" : opened.error().message));
    REQUIRE(opened.ok());

    std::ifstream in(fs::path(rigPath) / "objects" / "relation-types.json");
    const std::string text{std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>()};
    REQUIRE(text.find("has characters") == std::string::npos);
    REQUIRE(text.find("features") != std::string::npos);
}

TEST_CASE("upsert replaces by code", "[integration][T-0373]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};

    // ⚠️ Upserts a WRITER-AUTHORED code. This test used to edit the seeded `cites`
    // and assert the edit survived `load()`; T-0441's reconciliation now restores
    // seeded types on every open, so that assertion contradicts the 2026-08-21
    // ruling. The accepted consequence — "a seeded type a writer edited is
    // overwritten" — is asserted directly below rather than left implicit here.
    RelationType t;
    t.code = "references"; t.forwardLabel = "references"; t.inverseLabel = "referenced by";
    REQUIRE(store.upsert(fix.root(), t).ok());

    auto all = store.load(fix.root());
    REQUIRE(all.ok());
    REQUIRE(all.value().size() == 5);                       // appended
    REQUIRE(store.find(fix.root(), "references").value().forwardLabel == "references");

    // Replaced, not appended, on a second upsert of the same code.
    t.forwardLabel = "draws on";
    REQUIRE(store.upsert(fix.root(), t).ok());
    REQUIRE(store.load(fix.root()).value().size() == 5);
    REQUIRE(store.find(fix.root(), "references").value().forwardLabel == "draws on");
}

TEST_CASE("⚠️ ACCEPTED CONSEQUENCE — an edited SEEDED type is overwritten on open",
          "[integration][T-0441]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};

    // The user was asked to choose and chose self-healing (2026-08-21). This test
    // exists so the cost is visible in the suite rather than discovered by a
    // writer whose relabelled `cites` quietly reverted: the alternative — never
    // touching a modified seeded type — leaves a hand-edited `appears-in` broken
    // forever with no explanation, which is I-0125's live symptom.
    RelationType edited;
    edited.code = "cites";
    edited.forwardLabel = "references";
    edited.inverseLabel = "referenced by";
    REQUIRE(store.upsert(fix.root(), edited).ok());

    // upsert() writes AFTER load(), so her edit is on disk right now …
    REQUIRE(store.find(fix.root(), "cites").value().forwardLabel == "cites");
    // … and gone the moment anything reads the vocabulary again. `find()` itself
    // goes through load(), so the line above already reconciled it back.
    REQUIRE(store.find(fix.root(), "cites").value().inverseLabel == "documented by");
}

// ---------------------------------------------------------------------------
// T-0374 — the append-log
// ---------------------------------------------------------------------------

TEST_CASE("an edge round-trips create → reopen → list", "[integration][T-0374]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada = fix.makeObject(ObjectKind::character, "Ada", "ada");

    auto created = store.create(fix.root(), ada.value, fix.firstSceneID.value,
                                "appears-in", "first appearance");
    REQUIRE(created.ok());

    auto edges = store.load(fix.root());
    REQUIRE(edges.ok());
    REQUIRE(edges.value().size() == 1);
    REQUIRE(edges.value()[0].note == "first appearance");

    auto views = store.listFor(fix.root(), ada.value);
    REQUIRE(views.ok());
    REQUIRE(views.value().size() == 1);
    REQUIRE(views.value()[0].otherID == fix.firstSceneID.value);
}

TEST_CASE("ONE edge renders forward from one end and inverse from the other",
          "[integration][T-0374]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada = fix.makeObject(ObjectKind::character, "Ada", "ada");
    REQUIRE(store.create(fix.root(), ada.value, fix.firstSceneID.value,
                         "appears-in", "").ok());

    // §5.2: the two labels are two renderings of ONE record, not two records.
    auto fromAda = store.listFor(fix.root(), ada.value);
    REQUIRE(fromAda.ok());
    REQUIRE(fromAda.value().size() == 1);
    REQUIRE(fromAda.value()[0].isForward);
    REQUIRE(fromAda.value()[0].label == "appears in");

    auto fromScene = store.listFor(fix.root(), fix.firstSceneID.value);
    REQUIRE(fromScene.ok());
    REQUIRE(fromScene.value().size() == 1);
    REQUIRE_FALSE(fromScene.value()[0].isForward);
    // I-0125/R5: kind-neutral, since any kind may now appear in a scene.
    REQUIRE(fromScene.value()[0].label == "features");

    // Same edge, seen twice.
    REQUIRE(fromAda.value()[0].edge.edgeID == fromScene.value()[0].edge.edgeID);
    REQUIRE(store.load(fix.root()).value().size() == 1);
}

TEST_CASE("delete appends a tombstone and the edge stops listing",
          "[integration][T-0374]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada = fix.makeObject(ObjectKind::character, "Ada", "ada");
    auto e   = store.create(fix.root(), ada.value, fix.firstSceneID.value, "appears-in", "");
    REQUIRE(e.ok());

    REQUIRE(store.remove(fix.root(), e.value().edgeID).ok());

    // The tombstone is on disk immediately after the delete. Check this BEFORE
    // load(), because load() compacts: 1 edge + 1 tombstone is 50% > 30%, so a
    // load would legitimately erase the tombstone it is asked to look for.
    REQUIRE(fix.readLog().find("tomb") != std::string::npos);

    REQUIRE(store.load(fix.root()).value().empty());

    // Deleting an unknown edge is an error, not a silent no-op.
    REQUIRE_FALSE(store.remove(fix.root(), "edge_nope").ok());
}

TEST_CASE("a torn final line does not lose the records before it",
          "[integration][T-0374]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada  = fix.makeObject(ObjectKind::character, "Ada",  "ada");
    auto bram = fix.makeObject(ObjectKind::character, "Bram", "bram");
    REQUIRE(store.create(fix.root(), ada.value,  fix.firstSceneID.value, "appears-in", "").ok());
    REQUIRE(store.create(fix.root(), bram.value, fix.firstSceneID.value, "appears-in", "").ok());
    REQUIRE(store.load(fix.root()).value().size() == 2);

    // Simulate a partial write (power loss mid-append).
    {
        std::ofstream out(fix.logPath(), std::ios::app);
        out << R"({"rec":"edge","seq":3,"edgeID":"edge_tor)";
    }

    auto edges = store.load(fix.root());
    REQUIRE(edges.ok());
    REQUIRE(edges.value().size() == 2);      // both intact
}

TEST_CASE("seq is strictly increasing across mixed create/delete traffic",
          "[integration][T-0374]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada  = fix.makeObject(ObjectKind::character, "Ada",  "ada");
    auto bram = fix.makeObject(ObjectKind::character, "Bram", "bram");
    auto cora = fix.makeObject(ObjectKind::character, "Cora", "cora");

    auto e1 = store.create(fix.root(), ada.value,  fix.firstSceneID.value, "appears-in", "");
    REQUIRE(e1.ok());
    auto e2 = store.create(fix.root(), bram.value, fix.firstSceneID.value, "appears-in", "");
    REQUIRE(e2.ok());
    REQUIRE(store.remove(fix.root(), e1.value().edgeID).ok());
    REQUIRE(store.create(fix.root(), cora.value, fix.firstSceneID.value, "appears-in", "").ok());

    // ⚠️ HistoryStore.cpp:194 documents a shipped bug where 13 records were
    // written with seq 1 because the value was read before assignment. Replay
    // survived only because order comes from file position. Guard against a
    // repeat here.
    std::istringstream ss(fix.readLog());
    std::string line;
    long long   prev = 0;
    int         seen = 0;
    while (std::getline(ss, line)) {
        if (line.empty()) { continue; }
        const auto pos = line.find("\"seq\":");
        REQUIRE(pos != std::string::npos);
        const long long seq = std::stoll(line.substr(pos + 6));
        REQUIRE(seq > prev);
        prev = seq;
        ++seen;
    }
    REQUIRE(seen == 4);      // 3 creates + 1 tombstone
}

// ---------------------------------------------------------------------------
// T-0375 — canonical normalization + duplicate rejection (§9 AC4)
// ---------------------------------------------------------------------------

TEST_CASE("ASYMMETRIC cross-kind: one canonical edge from either end",
          "[integration][T-0375]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada = fix.makeObject(ObjectKind::character, "Ada", "ada");

    // "Ada appears-in Scene 1"
    REQUIRE(store.create(fix.root(), ada.value, fix.firstSceneID.value,
                         "appears-in", "").ok());

    // The SAME relationship stated from the scene end. §5.3: it normalizes to
    // the same canonical edge and must be rejected, not written twice.
    auto dup = store.create(fix.root(), fix.firstSceneID.value, ada.value,
                            "appears-in", "");
    REQUIRE_FALSE(dup.ok());
    REQUIRE(dup.error().detail == "duplicateEdge");

    REQUIRE(store.load(fix.root()).value().size() == 1);
}

TEST_CASE("SYMMETRIC same-kind: one canonical edge from either end",
          "[integration][T-0375]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    // §9 AC4 names faction↔faction "at war with" as the symmetric case and warns
    // it is "the one that regresses silently". `faction` is WORLD-SCOPED and not
    // creatable until SP-098 (SP-095 R1), so the same-kind symmetric SHAPE is
    // exercised here with `sibling-of` (character↔character, lexical), which is
    // what AC4 is actually about. The faction-specific case is added in SP-098.
    auto ada  = fix.makeObject(ObjectKind::character, "Ada",  "ada");
    auto bram = fix.makeObject(ObjectKind::character, "Bram", "bram");

    REQUIRE(store.create(fix.root(), ada.value, bram.value, "sibling-of", "").ok());

    auto dup = store.create(fix.root(), bram.value, ada.value, "sibling-of", "");
    REQUIRE_FALSE(dup.ok());
    REQUIRE(dup.error().detail == "duplicateEdge");

    REQUIRE(store.load(fix.root()).value().size() == 1);
}

TEST_CASE("symmetric edges are stored in lexical endpoint order",
          "[integration][T-0375]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada  = fix.makeObject(ObjectKind::character, "Ada",  "ada");
    auto bram = fix.makeObject(ObjectKind::character, "Bram", "bram");

    // Create in the "wrong" order; storage must still be canonical.
    const auto hi = std::max(ada.value, bram.value);
    const auto lo = std::min(ada.value, bram.value);
    auto e = store.create(fix.root(), hi, lo, "sibling-of", "");
    REQUIRE(e.ok());
    REQUIRE(e.value().fromID == lo);
    REQUIRE(e.value().toID   == hi);
}

TEST_CASE("create rejects bad input", "[integration][T-0375]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada = fix.makeObject(ObjectKind::character, "Ada", "ada");

    SECTION("unknown relation type") {
        REQUIRE_FALSE(store.create(fix.root(), ada.value, fix.firstSceneID.value,
                                   "no-such-type", "").ok());
    }
    SECTION("self-edge") {
        REQUIRE_FALSE(store.create(fix.root(), ada.value, ada.value, "cites", "").ok());
    }
    SECTION("unresolvable endpoint") {
        REQUIRE_FALSE(store.create(fix.root(), ada.value, "ghost-id", "cites", "").ok());
    }
    SECTION("kind-constraint violation") {
        // ⚠️ AMENDED (I-0125 / SP-102 R5, user-ruled 2026-08-17). This asserted that
        // `appears-in` REFUSES location→scene, because the type was declared
        // character→scene. That constraint was the defect: the Apple layer gives
        // `appears-in` to eight of the ten object cards, so creating a chronicle,
        // faction, artifact (etc.) from its card wrote the object and then failed at
        // the edge — reported to the writer as a failed creation. `appears-in` is now
        // unconstrained on its source: ANY kind may appear in a scene.
        // The constraint machinery must still bite, or this section asserts nothing:
        // `sibling-of` is character↔character, so a location on either end is
        // refused. (That `appears-in` now ACCEPTS any kind is asserted in its own
        // test below, which can commit an edge — this one ends by requiring the log
        // to be empty.)
        auto loc = fix.makeObject(ObjectKind::location, "Vault", "vault");
        REQUIRE_FALSE(store.create(fix.root(), loc.value, ada.value,
                                   "sibling-of", "").ok());
    }
    REQUIRE(store.load(fix.root()).value().empty());
}

// ⚠️ I-0125 (SP-102 R5): every worldbuilding kind must be linkable to a scene.
//
// Regression for a defect that made EIGHT of the ten object cards unable to create
// anything: `appears-in` was declared character→scene while the Apple layer offers it
// for chronicle, building, vehicle, item, map, artifact, faction and rule. The object
// was written to disk, the edge was refused, and the writer was told creation failed.
TEST_CASE("appears-in accepts every kind, not just characters (I-0125)",
          "[integration][I-0125]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    for (const auto kind : {ObjectKind::chronicle, ObjectKind::faction,
                            ObjectKind::artifact, ObjectKind::location,
                            ObjectKind::item, ObjectKind::building}) {
        const auto name = objectKindName(kind);
        auto obj = fix.makeObject(kind, name, name);
        INFO("kind = " << name);
        REQUIRE(store.create(fix.root(), obj.value, fix.firstSceneID.value,
                             "appears-in", "").ok());
    }
}

TEST_CASE("an unconstrained type relates ANY two kinds", "[integration][T-0375]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    // `cites` has null on both ends (§3.4), so it must accept combinations no
    // other seeded type allows.
    auto ada = fix.makeObject(ObjectKind::character, "Ada",   "ada");
    auto loc = fix.makeObject(ObjectKind::location,  "Vault", "vault");
    auto map = fix.makeObject(ObjectKind::map,       "Chart", "chart");

    REQUIRE(store.create(fix.root(), ada.value, loc.value, "cites", "").ok());
    REQUIRE(store.create(fix.root(), map.value, ada.value, "cites", "").ok());
    REQUIRE(store.load(fix.root()).value().size() == 2);
}

// ---------------------------------------------------------------------------
// T-0376 — compaction (§9 AC9: BOTH triggers, independently)
// ---------------------------------------------------------------------------

TEST_CASE("compaction fires on the RATIO trigger (small graph over 30%)",
          "[integration][T-0376]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    // 4 creates + 2 deletes = 6 records, 2 tombstones = 33% > 30%, and well
    // under the 1,000 absolute bound — so only the ratio can have fired.
    std::vector<std::string> ids;
    for (int i = 0; i < 4; ++i) {
        auto o = fix.makeObject(ObjectKind::character, "C" + std::to_string(i),
                                "c" + std::to_string(i));
        auto e = store.create(fix.root(), o.value, fix.firstSceneID.value, "appears-in", "");
        REQUIRE(e.ok());
        ids.push_back(e.value().edgeID);
    }
    REQUIRE(store.remove(fix.root(), ids[0]).ok());
    REQUIRE(store.remove(fix.root(), ids[1]).ok());

    const auto before = fix.readLog();
    REQUIRE(before.find("tomb") != std::string::npos);

    auto edges = store.load(fix.root());     // compaction runs on open
    REQUIRE(edges.ok());
    REQUIRE(edges.value().size() == 2);

    const auto after = fix.readLog();
    REQUIRE(after.find("tomb") == std::string::npos);   // dead records gone
    REQUIRE(after.size() < before.size());

    // A compacted log replays to the identical graph.
    REQUIRE(store.load(fix.root()).value().size() == 2);
}

TEST_CASE("compaction fires on the COUNT trigger (large graph under 30%)",
          "[integration][T-0376]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    // Hand-build a log with 4,000 edges and 1,001 tombstones: 20% — under the
    // ratio — but over the absolute 1,000 bound. Without the absolute bound a
    // large graph would accumulate tens of thousands of dead records first.
    {
        std::ofstream out(fix.logPath(), std::ios::trunc);
        long long seq = 0;
        for (int i = 0; i < 4000; ++i) {
            out << R"({"rec":"edge","seq":)" << ++seq
                << R"(,"edgeID":"edge_)" << i
                << R"(","from":"a","to":"b","relationType":"cites","note":"","sortIndex":0.0})"
                << "\n";
        }
        for (int i = 0; i < 1001; ++i) {
            out << R"({"rec":"tomb","seq":)" << ++seq
                << R"(,"edgeID":"edge_)" << i << R"("})" << "\n";
        }
    }

    auto edges = store.load(fix.root());
    REQUIRE(edges.ok());
    REQUIRE(edges.value().size() == 2999);
    REQUIRE(fix.readLog().find("tomb") == std::string::npos);
}

TEST_CASE("a graph below both thresholds is NOT compacted",
          "[integration][T-0376]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    // 4 creates + 1 delete = 5 records, 1 tombstone = 20% < 30%, 1 < 1000.
    std::vector<std::string> ids;
    for (int i = 0; i < 4; ++i) {
        auto o = fix.makeObject(ObjectKind::character, "C" + std::to_string(i),
                                "c" + std::to_string(i));
        auto e = store.create(fix.root(), o.value, fix.firstSceneID.value, "appears-in", "");
        REQUIRE(e.ok());
        ids.push_back(e.value().edgeID);
    }
    REQUIRE(store.remove(fix.root(), ids[0]).ok());

    REQUIRE(store.load(fix.root()).value().size() == 3);
    REQUIRE(fix.readLog().find("tomb") != std::string::npos);   // still there
}

TEST_CASE("a stray compaction temp file never displaces the real log",
          "[integration][T-0376]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    auto ada = fix.makeObject(ObjectKind::character, "Ada", "ada");
    REQUIRE(store.create(fix.root(), ada.value, fix.firstSceneID.value,
                         "appears-in", "").ok());

    // Compaction is write-to-".tmp" → rename (util::AtomicWrite), so a crash
    // mid-compaction leaves a stray .tmp and the PREVIOUS log intact. Simulate
    // the leftover and assert the real log still replays.
    {
        std::ofstream out(fix.logPath().string() + ".tmp", std::ios::trunc);
        out << R"({"rec":"edge","seq":1,"edgeID":"edge_par)";   // half-written
    }

    auto edges = store.load(fix.root());
    REQUIRE(edges.ok());
    REQUIRE(edges.value().size() == 1);
    REQUIRE(edges.value()[0].fromID == ada.value);
}

TEST_CASE("a missing relationships.jsonl is an empty graph, not an error",
          "[integration][T-0374]") {
    GraphFixture fix;
    RelationshipStore store{fix.services};

    REQUIRE_FALSE(fs::exists(fix.logPath()));
    auto r = store.load(fix.root());
    REQUIRE(r.ok());
    REQUIRE(r.value().empty());
}
