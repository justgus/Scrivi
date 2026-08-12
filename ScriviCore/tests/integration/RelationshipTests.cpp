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

TEST_CASE("upsert replaces by code", "[integration][T-0373]") {
    GraphFixture fix;
    RelationTypeStore store{fix.services};

    RelationType t;
    t.code = "cites"; t.forwardLabel = "references"; t.inverseLabel = "referenced by";
    REQUIRE(store.upsert(fix.root(), t).ok());

    auto all = store.load(fix.root());
    REQUIRE(all.ok());
    REQUIRE(all.value().size() == 4);                       // replaced, not appended
    REQUIRE(store.find(fix.root(), "cites").value().forwardLabel == "references");
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
    REQUIRE(fromScene.value()[0].label == "has characters");

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
        auto loc = fix.makeObject(ObjectKind::location, "Vault", "vault");
        // appears-in is character→scene; location→scene must be refused.
        REQUIRE_FALSE(store.create(fix.root(), loc.value, fix.firstSceneID.value,
                                   "appears-in", "").ok());
    }
    REQUIRE(store.load(fix.root()).value().empty());
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
