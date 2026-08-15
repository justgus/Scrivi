// EP-031 SP-098 — object CRUD, queries, promotion, and graph integrity AT THE C ABI.
//
// ⚠️ WHY THIS FILE EXISTS. I-0113 shipped because every world-scoped assertion
// lived in WorldTests.cpp, which calls the C++ facade directly. SP-097 widened
// CreateObjectRequest with `worldID` and routed storage through the world
// package — and the facade tests all passed — while the four scrivi_*_object
// entry points were never widened, so `req.worldID` was permanently empty at the
// boundary and world objects were unreachable through the ABI entirely.
//
// The rule this file enforces: a test for a BOUNDARY change goes through
// scrivi_*, never through core.createObject(). A facade test cannot see a
// boundary gap, by construction.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

JsonDoc envelope(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    return std::move(parsed.value());
}

JsonDoc okResult(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

JsonDoc errorOf(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE_FALSE(env.getBool("ok"));
    return env.getSubDoc("error");
}

// A real project + an out-of-package worlds root, both created through the C
// ABI and removed on destruction.
struct CApiFixture {
    fs::path    projectDir;
    fs::path    appSupportDir;
    fs::path    worldsRoot;
    std::string projectID;
    std::string firstSceneID;     // every new project is born with one scene
    std::string firstChapterID;   // ...inside one chapter

    CApiFixture() {
        static std::atomic<int> counter{0};
        const std::string stem =
            "scrivi-object-capi-" + std::to_string(counter.fetch_add(1)) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this));

        projectDir    = fs::temp_directory_path() / (stem + ".scrivi");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        worldsRoot    = fs::temp_directory_path() / (stem + "-worlds");

        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);
        fs::create_directories(worldsRoot);

        auto created = okResult(scrivi_create_project(root(), appSupport(), "Object C API",
                                                      "object-capi",
                                                      "identity-001", "persona-001",
                                                      "Test Author"));
        projectID    = created.getString("projectID");
        firstSceneID = created.getSubDoc("firstScene").getString("sceneID");
        REQUIRE_FALSE(firstSceneID.empty());

        // createProject reports the first SCENE but not its chapter; open does.
        auto opened = okResult(scrivi_open_project(root(), appSupport(), "identity-001"));
        REQUIRE(opened.arraySize("scenes") >= 1);
        firstChapterID = opened.arrayItem("scenes", 0).getString("chapterID");
        REQUIRE_FALSE(firstChapterID.empty());
    }

    ~CApiFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
        fs::remove_all(worldsRoot, ec);
        fs::remove_all(worldsRoot.string() + "-detached", ec);   // detachWorldsRoot
    }

    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] const char* appSupport() const { return appSupportDir.c_str(); }

    [[nodiscard]] std::string pkg(const std::string& name) const {
        return (worldsRoot / (name + ".scrivworld")).string();
    }

    // Creates a world THROUGH THE ABI and returns its worldID.
    std::string makeWorld(const std::string& name = "Midgard") {
        auto r = okResult(scrivi_create_world(root(), pkg(name).c_str(),
                                              name.c_str(), "The First Sundering"));
        auto id = r.getString("worldID");
        REQUIRE_FALSE(id.empty());
        return id;
    }

    // ⚠️ SP-103: every worldbuilding kind is world-scoped, so this helper now
    // routes them into a lazily-created default world. Only `source` is
    // project-scoped and still passes an empty worldID.
    //
    // The world is created on FIRST use rather than in the fixture ctor because
    // several tests here assert on a project that has no world bound yet
    // (`worldRequired`, pending-vs-dangling); eagerly creating one would
    // silently invalidate them.
    std::string defaultWorldID;

    std::string ensureDefaultWorld() {
        if (defaultWorldID.empty()) { defaultWorldID = makeWorld("Default"); }
        return defaultWorldID;
    }

    // ⚠️ SP-104: the index a world-scoped object actually lands in. Removing the
    // PROJECT index no longer forces a rebuild for these kinds — it silently
    // does nothing, so a test meaning to trigger a rescan would assert against
    // an index that was never rebuilt.
    fs::path worldIndexPath(const std::string& name = "Default") {
        (void)ensureDefaultWorld();
        return fs::path(pkg(name)) / "index.json";
    }

    // Creates an object through the ABI, returning its objectID.
    std::string makeObject(const std::string& kind, const std::string& displayName) {
        const std::string worldID = (kind == "source") ? std::string{} : ensureDefaultWorld();
        auto r = okResult(scrivi_create_object(root(), kind.c_str(), displayName.c_str(), "",
                                               "identity-001", "persona-001", "Test Author",
                                               worldID.c_str()));
        auto id = r.getString("objectID");
        REQUIRE_FALSE(id.empty());
        return id;
    }

    // Creates a world-scoped object through the ABI, returning its objectID.
    std::string makeWorldObject(const std::string& kind, const std::string& displayName,
                                const std::string& worldID) {
        auto r = okResult(scrivi_create_object(root(), kind.c_str(), displayName.c_str(), "",
                                               "identity-001", "persona-001", "Test Author",
                                               worldID.c_str()));
        auto id = r.getString("objectID");
        REQUIRE_FALSE(id.empty());
        return id;
    }

    // Makes a world UNAVAILABLE without destroying it — the world package is
    // moved aside, exactly as a writer moving a folder would do, and moved back
    // by reattachWorld.
    //
    // The reported status is "missing", not the generic "unavailable": the
    // containing folder IS present and readable and the package is not there,
    // which is precisely §4.6's condition for positively establishing missing.
    // ⚠️ The PENDING BEHAVIOUR IS IDENTICAL in all unavailable states — only the
    // reported status differs — so this exercises the pending path in full.
    static constexpr const char* kDetachedStatus = "missing";

    void detachWorld(const std::string& name = "Midgard") {
        fs::rename(pkg(name), pkg(name + "-detached"));
    }

    void reattachWorld(const std::string& name = "Midgard") {
        fs::rename(pkg(name + "-detached"), pkg(name));
    }

    // The other unavailability shape: the whole CONTAINING folder is gone, as an
    // unmounted volume would leave it. The cause is then undeterminable from
    // ScriviCore alone, so §4.6/AC-A6 requires the honest generic fallback
    // rather than a guess — a wrong "missing" is what invites a writer toward
    // destructive remedies.
    void detachWorldsRoot() {
        fs::rename(worldsRoot, worldsRoot.string() + "-detached");
    }
};

} // namespace

// ---------------------------------------------------------------------------
// I-0118 — world search records at the boundary
//
// ⚠️ THROUGH `scrivi_*`, NOT THE FACADE. `SearchableContentTests` covers the
// same behavior against the C++ facade and stayed green all through I-0113 —
// which is exactly how a boundary gap ships unnoticed. The per-item
// `domainIdentifier` and `worldDomainIdentifiers` are new fields, and a
// serializer that forgets one is invisible to any facade test.
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: world search items carry their own domain and a world deep link",
          "[integration][I-0118]") {
    CApiFixture fix;
    const auto worldID = fix.ensureDefaultWorld();
    const auto vance   = fix.makeObject("character", "Vance");
    const auto notes   = fix.makeObject("source",    "Field Notes");

    auto res = okResult(scrivi_extract_searchable_text(fix.root()));

    // The world's domain crosses the boundary, so the donor can index it apart
    // from the project's. Q1: it is advertised, never presented as deletable.
    auto domains = res.getStringArray("worldDomainIdentifiers");
    REQUIRE(domains.size() == 1);
    REQUIRE(domains[0] == worldID);

    bool sawCharacter = false;
    bool sawSource    = false;
    for (std::size_t i = 0; i < res.arraySize("items"); ++i) {
        auto item = res.arrayItem("items", i);
        const auto uid = item.getString("uniqueIdentifier");

        if (uid == "character:" + vance) {
            sawCharacter = true;
            // Q1 — the WORLD's domain, not the project's.
            REQUIRE(item.getString("domainIdentifier") == worldID);
            // Q2 — a world-scoped deep link.
            REQUIRE(item.getString("deepLink") ==
                    "scrivi://open?world=" + worldID + "&item=" + uid);
        }
        if (uid == "source:" + notes) {
            sawSource = true;
            // `source` is project-scoped: no per-item domain (⇒ the project's)
            // and a project-scoped link. The two halves must not collapse.
            REQUIRE(item.getString("domainIdentifier").empty());
            REQUIRE(item.getString("deepLink").find("project=") != std::string::npos);
        }
    }
    REQUIRE(sawCharacter);
    REQUIRE(sawSource);
}

// ---------------------------------------------------------------------------
// T-0405 / I-0113 — worldID at the boundary
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: a world-scoped object round-trips create → open → delete (I-0113)",
          "[integration][T-0405][I-0113]") {
    CApiFixture fix;
    const auto worldID = fix.makeWorld();

    // ⚠️ THE PROBE FROM THE SP-098 AUDIT. Before T-0405 this returned
    // {"ok":false,"message":"kind 'artifact' is world-scoped; a worldID is
    // required"} — with a world created and bound moments earlier, in this very
    // process — because the entry point had no way to carry the worldID.
    auto created = okResult(scrivi_create_object(fix.root(), "artifact", "Sword of Dawn", "",
                                                 "identity-001", "persona-001", "Test Author",
                                                 worldID.c_str()));
    const auto objectID = created.getString("objectID");
    REQUIRE_FALSE(objectID.empty());

    // It landed in the WORLD package, never under objects/ (Doc 3 §7). Matched
    // by package NAME rather than by full prefix: WorldStore::resolve canonicalizes
    // the binding's relative path, so on macOS the returned path is rooted at
    // /private/var while the fixture's is /var — the same directory, spelled two
    // ways, which a prefix compare would call a mismatch.
    const auto path = created.getString("path");
    REQUIRE(path.find("Midgard.scrivworld/artifacts/") != std::string::npos);
    REQUIRE(path.find("/objects/") == std::string::npos);
    REQUIRE(fs::exists(path));

    auto opened = okResult(scrivi_open_object(fix.root(), "artifact", objectID.c_str(),
                                              worldID.c_str()));
    auto objJson = parseJson(opened.getString("objectJson"));
    REQUIRE(objJson.ok());
    REQUIRE(objJson.value().getString("objectID")    == objectID);
    REQUIRE(objJson.value().getString("displayName") == "Sword of Dawn");
    // The object carries its owning world, which is what makes scrivi_save_object
    // able to stay un-widened.
    REQUIRE(objJson.value().getString("worldID") == worldID);

    auto deleted = okResult(scrivi_delete_object(fix.root(), "artifact", objectID.c_str(),
                                                 worldID.c_str()));
    REQUIRE(deleted.getBool("deleted"));
    REQUIRE_FALSE(fs::exists(path));
}

TEST_CASE("C ABI: an empty worldID still creates project-scoped kinds unchanged",
          "[integration][T-0405]") {
    CApiFixture fix;

    // ⚠️ SP-103: `character` became world-scoped, so `source` is now the kind
    // that exercises this ABI property — an empty worldID meaning project scope.
    // The property under test is unchanged; only the kind that still has project
    // scope has changed.
    auto created = okResult(scrivi_create_object(fix.root(), "source", "Principia", "",
                                                 "identity-001", "persona-001", "Test Author",
                                                 ""));
    const auto objectID = created.getString("objectID");
    const auto path     = created.getString("path");
    REQUIRE(path.find("/objects/sources/") != std::string::npos);

    auto opened = okResult(scrivi_open_object(fix.root(), "source", objectID.c_str(), ""));
    auto objJson = parseJson(opened.getString("objectJson"));
    REQUIRE(objJson.ok());
    // Project-scoped objects carry NO worldID — the key is omitted entirely, so
    // a pre-SP-095 file still round-trips byte-identically.
    REQUIRE(objJson.value().getString("worldID").empty());

    REQUIRE(okResult(scrivi_delete_object(fix.root(), "source", objectID.c_str(), ""))
                .getBool("deleted"));
}

TEST_CASE("C ABI: a NULL worldID is treated as project scope, not as a crash",
          "[integration][T-0405]") {
    CApiFixture fix;
    // The ABI's S() maps NULL → "" everywhere; a caller in a language without a
    // natural empty string must not have to synthesize one.
    auto created = okResult(scrivi_create_object(fix.root(), "source", "Ordo Annals", "",
                                                 "identity-001", "persona-001", "Test Author",
                                                 nullptr));
    REQUIRE_FALSE(created.getString("objectID").empty());
}

TEST_CASE("C ABI: a world-scoped kind without a worldID is still refused",
          "[integration][T-0405]") {
    CApiFixture fix;
    fix.makeWorld();

    // Widening the signature did NOT weaken the scope rule — an artifact with no
    // world would be stranded under objects/ with no migration to rescue it
    // (Doc 3 §7 writes none).
    auto err = errorOf(scrivi_create_object(fix.root(), "artifact", "Orphan Blade", "",
                                            "identity-001", "persona-001", "Test Author",
                                            ""));
    REQUIRE(err.getString("detail") == "worldRequired");
}

TEST_CASE("C ABI: a world-scoped object saves through the un-widened save endpoint",
          "[integration][T-0405]") {
    CApiFixture fix;
    const auto worldID  = fix.makeWorld();
    const auto objectID = fix.makeWorldObject("artifact", "Sword of Dawn", worldID);

    auto opened  = okResult(scrivi_open_object(fix.root(), "artifact", objectID.c_str(),
                                               worldID.c_str()));
    auto objJson = parseJson(opened.getString("objectJson"));
    REQUIRE(objJson.ok());

    // scrivi_save_object takes no worldID by design: the JSON carries it.
    auto edited = std::move(objJson.value());
    edited.setString("subtitle", "Forged at the Sundering");

    auto saved = okResult(scrivi_save_object(fix.root(), "artifact", edited.dump().c_str(),
                                             "identity-001", "persona-001", "Test Author"));
    REQUIRE(saved.getBool("saved"));

    auto reopened = okResult(scrivi_open_object(fix.root(), "artifact", objectID.c_str(),
                                                worldID.c_str()));
    auto reread = parseJson(reopened.getString("objectJson"));
    REQUIRE(reread.ok());
    REQUIRE(reread.value().getString("subtitle") == "Forged at the Sundering");
}

// ---------------------------------------------------------------------------
// T-0380 — ⚠️ pending vs. dangling (Doc 3 §4.6, AC-A1–A7)
//
// The distinction these tests exist to protect: an endpoint that will not
// resolve because its WORLD is away is *pending* — held, never pruned. One that
// will not resolve while its world IS present is *dangling* — genuinely gone.
// Treat the first as the second and a writer's relationships are destroyed
// silently, which is the only unrecoverable failure in EP-031.
// ---------------------------------------------------------------------------

namespace {

// A character in the project + an artifact in a world, related by `cites` (the
// one seeded type unconstrained on both ends, so it spans partitions freely).
struct CrossPartitionGraph {
    CApiFixture fix;
    std::string worldID;
    std::string characterID;
    std::string artifactID;
    std::string edgeID;

    CrossPartitionGraph() {
        worldID = fix.makeWorld();
        // ⚠️ SP-103: `character` is world-scoped now, so a character would sit
        // in a world too and this would no longer be CROSS-partition — and
        // detaching one world would not make it pending. `source` is the sole
        // project-scoped kind, so it is now the project-side endpoint. The
        // `cites` type is unconstrained on both ends, which is exactly why it
        // was chosen here originally.
        characterID = fix.makeObject("source", "Vance's Memoirs");
        artifactID  = fix.makeWorldObject("artifact", "Sword of Dawn", worldID);

        auto e = okResult(scrivi_create_edge(fix.root(), characterID.c_str(),
                                             artifactID.c_str(), "cites", ""));
        edgeID = e.getString("edgeID");
        REQUIRE_FALSE(edgeID.empty());
    }
};

// The live edges touching an endpoint, by edgeID.
std::vector<std::string> edgeIDsFor(const CApiFixture& fix, const std::string& endpointID) {
    auto res = okResult(scrivi_list_edges_for(fix.root(), endpointID.c_str()));
    std::vector<std::string> out;
    for (std::size_t i = 0; i < res.arraySize("edges"); ++i) {
        out.push_back(res.arrayItem("edges", i).getString("edgeID"));
    }
    return out;
}

} // namespace

TEST_CASE("AC-A5: world present + endpoint missing is DANGLING, and prunes",
          "[integration][T-0380][AC-A5]") {
    CrossPartitionGraph g;

    // The world is right there; the artifact is genuinely deleted. This is the
    // branch where pruning IS correct.
    okResult(scrivi_delete_object(g.fix.root(), "artifact", g.artifactID.c_str(),
                                  g.worldID.c_str()));

    auto pending = okResult(scrivi_list_pending_edges(g.fix.root()));
    REQUIRE(pending.arraySize("pending") == 0);   // absent ≠ pending

    // Cascade-prune fired on the delete (T-0377) — the edge is gone.
    REQUIRE(edgeIDsFor(g.fix, g.characterID).empty());
}

TEST_CASE("⚠️ AC-A5: world absent is PENDING, and does NOT prune",
          "[integration][T-0380][AC-A5]") {
    CrossPartitionGraph g;
    g.fix.detachWorld();

    // The artifact has not been deleted — its world is merely unreachable. The
    // edge must survive untouched.
    auto edges = edgeIDsFor(g.fix, g.characterID);
    REQUIRE(edges.size() == 1);
    REQUIRE(edges[0] == g.edgeID);

    auto pending = okResult(scrivi_list_pending_edges(g.fix.root()));
    REQUIRE(pending.arraySize("pending") == 1);
    auto row = pending.arrayItem("pending", 0);
    REQUIRE(row.getString("edgeID")            == g.edgeID);
    REQUIRE(row.getString("pendingEndpointID") == g.artifactID);
    REQUIRE(row.getString("worldID")           == g.worldID);
    REQUIRE(row.getString("worldStatus")       == CApiFixture::kDetachedStatus);
}

TEST_CASE("AC-A6: an undeterminable cause reports generic 'unavailable', never a guess",
          "[integration][T-0380][AC-A6]") {
    CrossPartitionGraph g;
    // The containing folder itself is gone — as an unmounted volume leaves it.
    // ScriviCore cannot tell unmounted from offline from deleted, and §4.6 is
    // explicit that a wrong "missing" is materially worse than an honest
    // "unavailable", because only "missing" invites destructive remedies.
    g.fix.detachWorldsRoot();

    auto pending = okResult(scrivi_list_pending_edges(g.fix.root()));
    REQUIRE(pending.arraySize("pending") == 1);
    REQUIRE(pending.arrayItem("pending", 0).getString("worldStatus") == "unavailable");

    // Pending behaviour is IDENTICAL across every unavailable state: still held,
    // still named, still frozen.
    REQUIRE(pending.arrayItem("pending", 0).getString("displayName") == "Sword of Dawn");
    REQUIRE(edgeIDsFor(g.fix, g.characterID).size() == 1);
}

TEST_CASE("AC-A7: a pending endpoint displays its NAME, not a bare ID",
          "[integration][T-0380][AC-A7]") {
    CrossPartitionGraph g;
    g.fix.detachWorld();

    // The reason binding.cachedIndex exists at all (§6.3): a writer asked
    // whether to clear references cannot decide against a UUID.
    auto pending = okResult(scrivi_list_pending_edges(g.fix.root()));
    REQUIRE(pending.arrayItem("pending", 0).getString("displayName") == "Sword of Dawn");

    // ...and the same name reaches the inspector through the edge list.
    auto listed = okResult(scrivi_list_edges_for(g.fix.root(), g.characterID.c_str()));
    auto row    = listed.arrayItem("edges", 0);
    REQUIRE(row.getBool("otherPending"));
    REQUIRE(row.getString("otherDisplayName") == "Sword of Dawn");
    REQUIRE(row.getString("otherWorldStatus") == CApiFixture::kDetachedStatus);
}

TEST_CASE("⚠️ AC-A4: the graph is FROZEN toward an unavailable world — both directions",
          "[integration][T-0380][AC-A4]") {
    CrossPartitionGraph g;
    const auto secondID = g.fix.makeObject("character", "Ordo");
    g.fix.detachWorld();

    // Adding an edge INTO the pending world is refused explicitly — with the
    // world's status, not a bogus "not found" that would tell the writer their
    // artifact was deleted.
    auto addErr = errorOf(scrivi_create_edge(g.fix.root(), secondID.c_str(),
                                             g.artifactID.c_str(), "cites", ""));
    REQUIRE(addErr.getString("detail") ==
            std::string("worldPending:") + CApiFixture::kDetachedStatus);
    REQUIRE(addErr.getString("message").find("Sword of Dawn") != std::string::npos);

    // Removing an existing edge is exactly as frozen: a tombstone is not
    // reversible, and the writer cannot see what they would be discarding.
    auto delErr = errorOf(scrivi_delete_edge(g.fix.root(), g.edgeID.c_str()));
    REQUIRE(delErr.getString("detail") ==
            std::string("worldPending:") + CApiFixture::kDetachedStatus);

    // Neither refusal was a silent drop — the edge is still there, alone.
    REQUIRE(edgeIDsFor(g.fix, g.characterID).size() == 1);
}

TEST_CASE("AC-A1/A2: pending edges survive open and save verbatim",
          "[integration][T-0380][AC-A1][AC-A2]") {
    CrossPartitionGraph g;

    // Byte-for-byte baseline of the edge log BEFORE the world goes away.
    const auto logPath = (fs::path(g.fix.projectDir) / "objects" / "relationships.jsonl");
    std::string before;
    {
        std::ifstream in(logPath);
        before.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }

    g.fix.detachWorld();

    // Open the project (AC-A1: opens without blocking, prunes nothing) and save
    // an unrelated object, which is what a writer would actually be doing.
    okResult(scrivi_open_project(g.fix.root(), g.fix.appSupport(), "identity-001"));
    // ⚠️ SP-104: this created a `location` with an empty worldID. Since T-0409
    // that is world-scoped, and the world is DETACHED two lines above — so the
    // create now fails and the test never reached its real assertion. `source`
    // is the sole project-scoped kind, so it is the only "unrelated object" a
    // writer can still save while a world is away, which is exactly the
    // situation this test is about.
    okResult(scrivi_create_object(g.fix.root(), "source", "Ordo Keep Notes", "",
                                  "identity-001", "persona-001", "Test Author", ""));

    std::string after;
    {
        std::ifstream in(logPath);
        after.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }
    // AC-A2: "preserves every pending edge VERBATIM" — not merely "still
    // resolves". A tombstone appended and compacted away would satisfy the
    // weaker reading and still have lost the writer's data.
    REQUIRE(after == before);
}

TEST_CASE("AC-A3: reattaching the world restores pending edges with no repair pass",
          "[integration][T-0380][AC-A3]") {
    CrossPartitionGraph g;
    g.fix.detachWorld();
    REQUIRE(okResult(scrivi_list_pending_edges(g.fix.root())).arraySize("pending") == 1);

    g.fix.reattachWorld();

    // No repair call, no writer action — the edge simply resolves again.
    REQUIRE(okResult(scrivi_list_pending_edges(g.fix.root())).arraySize("pending") == 0);

    auto listed = okResult(scrivi_list_edges_for(g.fix.root(), g.characterID.c_str()));
    REQUIRE(listed.arraySize("edges") == 1);
    auto row = listed.arrayItem("edges", 0);
    REQUIRE(row.getString("edgeID") == g.edgeID);
    REQUIRE_FALSE(row.getBool("otherPending"));
    REQUIRE(row.getString("otherDisplayName") == "Sword of Dawn");

    // ...and the graph is unfrozen: edits toward that world work again.
    const auto secondID = g.fix.makeObject("character", "Ordo");
    REQUIRE_FALSE(okResult(scrivi_create_edge(g.fix.root(), secondID.c_str(),
                                              g.artifactID.c_str(), "cites", ""))
                      .getString("edgeID").empty());
}

TEST_CASE("AC-A1: a project with no worlds pays no world cost and reports no pending",
          "[integration][T-0380][AC-A1]") {
    CApiFixture fix;
    // ⚠️ SP-103: every worldbuilding kind is world-scoped, so a project with NO
    // world can only hold `source` objects. That makes `source` the only way to
    // exercise "a project that never used worlds" — which is exactly the case
    // §4.5 is about, and it still must cost nothing.
    const auto a = fix.makeObject("source", "On the Origin of Species");
    const auto b = fix.makeObject("source", "Principia");
    okResult(scrivi_create_edge(fix.root(), a.c_str(), b.c_str(), "cites", ""));

    // §4.5: a project that never used worlds does no world work at all.
    REQUIRE(okResult(scrivi_list_pending_edges(fix.root())).arraySize("pending") == 0);
    REQUIRE_FALSE(fs::exists(fs::path(fix.projectDir) / "worlds"));
    REQUIRE(fix.defaultWorldID.empty());   // nothing lazily created one
}

// ---------------------------------------------------------------------------
// T-0377 — cascade-prune + load-time repair (Doc 1 §5.5)
//
// ⚠️ This task reaches into EP-027's shipped delete paths. The prune step is
// purely additive and best-effort: the scene or object is already gone by the
// time it runs, so failing the delete over the edge log would report a failure
// that did not occur.
// ---------------------------------------------------------------------------

TEST_CASE("deleting an object tombstones every edge referencing it",
          "[integration][T-0377]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");
    const auto ordo  = fix.makeObject("character", "Ordo");
    const auto keep  = fix.makeObject("location",  "Ordo Keep");

    okResult(scrivi_create_edge(fix.root(), vance.c_str(), ordo.c_str(), "sibling-of", ""));
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), keep.c_str(), "cites", ""));
    REQUIRE(edgeIDsFor(fix, vance).size() == 2);

    okResult(scrivi_delete_object(fix.root(), "character", vance.c_str(),
                                 fix.ensureDefaultWorld().c_str()));

    // Both edges go in the SAME operation — the graph is never observed
    // referencing a file that is already gone.
    REQUIRE(edgeIDsFor(fix, vance).empty());
    REQUIRE(edgeIDsFor(fix, ordo).empty());
    REQUIRE(edgeIDsFor(fix, keep).empty());
}

TEST_CASE("deleting a SCENE cascade-prunes its edges too (§5.5)",
          "[integration][T-0377]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");

    // Scenes are first-class edge endpoints, so scene delete must prune exactly
    // as object delete does — the clause that makes this reach into EP-027.
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), fix.firstSceneID.c_str(),
                                "appears-in", ""));
    REQUIRE(edgeIDsFor(fix, vance).size() == 1);

    // A chapter must keep at least one scene, so add a second and delete it.
    auto second = okResult(scrivi_create_scene(fix.root(), fix.appSupport(),
                                               fix.projectID.c_str(),
                                               fix.firstChapterID.c_str(), "", "",
                                               "identity-001", "persona-001", "Test Author"));
    const auto secondSceneID = second.getString("sceneID");
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), secondSceneID.c_str(),
                                "appears-in", ""));
    REQUIRE(edgeIDsFor(fix, vance).size() == 2);

    okResult(scrivi_delete_scene(fix.root(), secondSceneID.c_str()));

    auto remaining = edgeIDsFor(fix, vance);
    REQUIRE(remaining.size() == 1);                        // the other edge is untouched
    REQUIRE(edgeIDsFor(fix, secondSceneID).empty());
}

TEST_CASE("deleting a CHAPTER prunes the edges of every scene it contained",
          "[integration][T-0377]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");

    auto chapter = okResult(scrivi_create_chapter(fix.root(), fix.appSupport(),
                                                  fix.projectID.c_str(),
                                                  "identity-001", "persona-001",
                                                  "Chapter Two", ""));
    const auto chapterID = chapter.getString("chapterID");
    const auto sceneID   = chapter.getString("firstSceneID");   // born with one scene
    REQUIRE_FALSE(sceneID.empty());

    okResult(scrivi_create_edge(fix.root(), vance.c_str(), sceneID.c_str(), "appears-in", ""));
    REQUIRE(edgeIDsFor(fix, vance).size() == 1);

    // remove_all takes the whole directory, so the scene IDs must be collected
    // BEFORE the delete — afterwards there is nothing left to read them from.
    okResult(scrivi_delete_chapter(fix.root(), chapterID.c_str()));

    REQUIRE(edgeIDsFor(fix, vance).empty());
    REQUIRE(edgeIDsFor(fix, sceneID).empty());
}

TEST_CASE("load-time repair drops a genuinely dangling edge",
          "[integration][T-0377]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");
    const auto ordo  = fix.makeObject("character", "Ordo");
    auto edge = okResult(scrivi_create_edge(fix.root(), vance.c_str(), ordo.c_str(),
                                            "sibling-of", ""));

    // Delete Ordo's file BEHIND the API's back — the crash-between-write-and-
    // tombstone case, and the hand-edited-project case. Cascade-prune never ran,
    // so only the repair pass can catch this.
    auto opened = okResult(scrivi_open_object(fix.root(), "character", ordo.c_str(),
                                              fix.ensureDefaultWorld().c_str()));
    fs::remove(opened.getString("path"));
    // ⚠️ SP-104: characters index in the WORLD's index since T-0409, so removing
    // the project index forced no rebuild at all and the stale entry survived.
    fs::remove(fix.worldIndexPath());   // force a rebuild

    REQUIRE(edgeIDsFor(fix, vance).size() == 1);   // still there, still dangling

    okResult(scrivi_open_project(fix.root(), fix.appSupport(), "identity-001"));

    REQUIRE(edgeIDsFor(fix, vance).empty());
}

TEST_CASE("⚠️ load-time repair NEVER prunes an edge into an unavailable world",
          "[integration][T-0377][AC-A1]") {
    CrossPartitionGraph g;
    g.fix.detachWorld();

    // The endpoint does not resolve — exactly as a dangling one does not. The
    // ONLY thing separating them is the world's status, and reading this as
    // dangling destroys the relationship with no error and no way back.
    okResult(scrivi_open_project(g.fix.root(), g.fix.appSupport(), "identity-001"));
    REQUIRE(edgeIDsFor(g.fix, g.characterID).size() == 1);

    // Repeated opens must not erode it either — the writer may work for weeks
    // with the world unmounted.
    for (int i = 0; i < 3; ++i) {
        okResult(scrivi_open_project(g.fix.root(), g.fix.appSupport(), "identity-001"));
    }
    REQUIRE(edgeIDsFor(g.fix, g.characterID).size() == 1);

    g.fix.reattachWorld();
    REQUIRE(edgeIDsFor(g.fix, g.characterID).size() == 1);
}

TEST_CASE("⚠️ cascade-prune holds an edge whose FAR endpoint is in an unavailable world",
          "[integration][T-0377]") {
    CrossPartitionGraph g;
    g.fix.detachWorld();

    // Deleting the project-side character is legitimate and must succeed — but
    // its edge to the pending artifact is NOT pruned: with the world away we
    // cannot know what that relationship still means, and a tombstone is final.
    okResult(scrivi_delete_object(g.fix.root(), "source", g.characterID.c_str(), ""));

    auto pending = okResult(scrivi_list_pending_edges(g.fix.root()));
    REQUIRE(pending.arraySize("pending") == 1);
    REQUIRE(pending.arrayItem("pending", 0).getString("edgeID") == g.edgeID);

    // Once the world returns, the edge is finally knowable — and now genuinely
    // dangling, so the ordinary repair pass retires it.
    g.fix.reattachWorld();
    okResult(scrivi_open_project(g.fix.root(), g.fix.appSupport(), "identity-001"));
    REQUIRE(edgeIDsFor(g.fix, g.artifactID).empty());
}

// ---------------------------------------------------------------------------
// T-0378 — object + orphan queries (§5.5)
// ---------------------------------------------------------------------------

namespace {

std::vector<std::string> objectIDsIn(const JsonDoc& result) {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < result.arraySize("objects"); ++i) {
        out.push_back(result.arrayItem("objects", i).getString("objectID"));
    }
    return out;
}

bool contains(const std::vector<std::string>& v, const std::string& id) {
    return std::find(v.begin(), v.end(), id) != v.end();
}

} // namespace

TEST_CASE("list_objects returns every project object, and kindOrNull filters",
          "[integration][T-0378]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");
    const auto ordo  = fix.makeObject("character", "Ordo");
    const auto keep  = fix.makeObject("location",  "Ordo Keep");

    auto all = objectIDsIn(okResult(scrivi_list_objects(fix.root(), "")));
    REQUIRE(all.size() == 3);
    REQUIRE(contains(all, vance));
    REQUIRE(contains(all, keep));

    auto characters = objectIDsIn(okResult(scrivi_list_objects(fix.root(), "character")));
    REQUIRE(characters.size() == 2);
    REQUIRE(contains(characters, vance));
    REQUIRE(contains(characters, ordo));
    REQUIRE_FALSE(contains(characters, keep));

    // NULL is the same as "" — every kind.
    REQUIRE(objectIDsIn(okResult(scrivi_list_objects(fix.root(), nullptr))).size() == 3);
}

TEST_CASE("list_objects rejects an unknown kind rather than returning nothing",
          "[integration][T-0378]") {
    CApiFixture fix;
    fix.makeObject("character", "Vance");
    // An empty listing would read as "you have no dragons" — indistinguishable
    // from a correct answer, which is exactly the wrong report for a typo.
    REQUIRE_FALSE(errorOf(scrivi_list_objects(fix.root(), "dragon"))
                      .getString("message").empty());
}

TEST_CASE("list_objects includes world objects when the world is available",
          "[integration][T-0378]") {
    CApiFixture fix;
    const auto worldID  = fix.makeWorld();
    const auto vance    = fix.makeObject("character", "Vance");
    const auto artifact = fix.makeWorldObject("artifact", "Sword of Dawn", worldID);

    auto all = objectIDsIn(okResult(scrivi_list_objects(fix.root(), "")));
    REQUIRE(contains(all, vance));
    REQUIRE(contains(all, artifact));   // the index spans both partitions (§4.2)

    auto artifacts = okResult(scrivi_list_objects(fix.root(), "artifact"));
    REQUIRE(artifacts.arraySize("objects") == 1);
    REQUIRE(artifacts.arrayItem("objects", 0).getString("worldID") == worldID);

    // With the world away the object is simply not visible — NOT deleted, and
    // not faked into the listing from the binding's cache either.
    fix.detachWorld();
    REQUIRE_FALSE(contains(objectIDsIn(okResult(scrivi_list_objects(fix.root(), ""))), artifact));
}

TEST_CASE("an object SURVIVES deletion of its last edge and is then an orphan",
          "[integration][T-0378]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");
    const auto ordo  = fix.makeObject("character", "Ordo");

    auto edge = okResult(scrivi_create_edge(fix.root(), vance.c_str(), ordo.c_str(),
                                            "sibling-of", ""));
    // Related, so neither is an orphan.
    REQUIRE(objectIDsIn(okResult(scrivi_list_orphaned_objects(fix.root()))).empty());

    okResult(scrivi_delete_edge(fix.root(), edge.getString("edgeID").c_str()));

    // ⚠️ Pruning the EDGE must not take the OBJECTS with it. An object with no
    // relationships is a legitimate creative state — a character sketched before
    // they have a scene — so both survive and become findable as orphans.
    auto orphans = objectIDsIn(okResult(scrivi_list_orphaned_objects(fix.root())));
    REQUIRE(orphans.size() == 2);
    REQUIRE(contains(orphans, vance));
    REQUIRE(contains(orphans, ordo));

    // ...and they are still fully present, not merely listed.
    REQUIRE(objectIDsIn(okResult(scrivi_list_objects(fix.root(), ""))).size() == 2);
}

TEST_CASE("cascade-prune leaves the surviving endpoint as a retained orphan",
          "[integration][T-0378]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");
    const auto ordo  = fix.makeObject("character", "Ordo");
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), ordo.c_str(), "sibling-of", ""));

    okResult(scrivi_delete_object(fix.root(), "character", vance.c_str(),
                                 fix.ensureDefaultWorld().c_str()));

    // Deleting Vance prunes the edge — and leaves Ordo, now unrelated, intact.
    auto orphans = objectIDsIn(okResult(scrivi_list_orphaned_objects(fix.root())));
    REQUIRE(orphans.size() == 1);
    REQUIRE(orphans[0] == ordo);
}

TEST_CASE("a scene endpoint counts as a relationship for orphan purposes",
          "[integration][T-0378]") {
    CApiFixture fix;
    const auto vance = fix.makeObject("character", "Vance");
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), fix.firstSceneID.c_str(),
                                "appears-in", ""));

    // A character who appears in a scene is related, even though the far
    // endpoint is not an object at all.
    REQUIRE(objectIDsIn(okResult(scrivi_list_orphaned_objects(fix.root()))).empty());
}

// ---------------------------------------------------------------------------
// T-0379 — promotion / demotion (§3.1, §9 AC8)
//
// ⚠️ THE PROMOTION-SAFETY PROOF FOR THE WHOLE BARE-ENDPOINT RULING. Edge
// endpoints dropped `kind` (§5.2) for exactly one reason: so that promoting an
// object could not stale them. The assertion that matters is therefore not
// "edges still resolve" — it is that the edge log is BYTE-IDENTICAL across the
// promotion. A rewrite that happened to produce working edges would still have
// falsified the premise.
// ---------------------------------------------------------------------------

namespace {

std::string readEdgeLog(const CApiFixture& fix) {
    std::ifstream in(fs::path(fix.projectDir) / "objects" / "relationships.jsonl");
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("⚠️ promotion preserves objectID and rewrites ZERO edges (AC8)",
          "[integration][T-0379][AC8]") {
    CApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto vance   = fix.makeObject("character", "Vance");
    const auto key     = fix.makeObject("item",      "Brass Key");

    // Give the item a life in the graph first: an edge to a character and one to
    // a scene, so both endpoint flavours are covered.
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), key.c_str(), "cites", ""));
    okResult(scrivi_create_edge(fix.root(), key.c_str(), fix.firstSceneID.c_str(),
                                "cites", ""));
    const auto before = readEdgeLog(fix);
    REQUIRE_FALSE(before.empty());

    auto promoted = okResult(scrivi_promote_object(fix.root(), key.c_str(),
                                                   "artifact", worldID.c_str()));

    // Identity survives — same objectID, new kind and scope.
    REQUIRE(promoted.getString("objectID") == key);
    REQUIRE(promoted.getString("kind")     == "artifact");
    REQUIRE(promoted.getString("worldID")  == worldID);

    // The file MOVED: out of items/, into artifacts/.
    // ⚠️ SP-103: `item` is world-scoped now, so promotion is world→world rather
    // than project→world. The move is between subdirectories of the SAME
    // package; what promotion preserves (objectID, and an untouched edge log)
    // is unchanged by that.
    REQUIRE(promoted.getString("fromPath").find("/items/") != std::string::npos);
    REQUIRE(promoted.getString("path").find("Midgard.scrivworld/artifacts/")
                != std::string::npos);
    REQUIRE_FALSE(fs::exists(promoted.getString("fromPath")));
    REQUIRE(fs::exists(promoted.getString("path")));

    // ⚠️ THE ASSERTION. Not "edges still resolve" — byte-identical.
    REQUIRE(readEdgeLog(fix) == before);

    // ...and they do still resolve, through the index rather than through kind.
    auto listed = okResult(scrivi_list_edges_for(fix.root(), key.c_str()));
    REQUIRE(listed.arraySize("edges") == 2);
    for (std::size_t i = 0; i < listed.arraySize("edges"); ++i) {
        REQUIRE_FALSE(listed.arrayItem("edges", i).getString("otherDisplayName").empty());
    }

    // The promoted object opens as an artifact, under its preserved ID.
    auto opened = okResult(scrivi_open_object(fix.root(), "artifact", key.c_str(),
                                              worldID.c_str()));
    auto objJson = parseJson(opened.getString("objectJson"));
    REQUIRE(objJson.ok());
    REQUIRE(objJson.value().getString("objectID")    == key);
    REQUIRE(objJson.value().getString("displayName") == "Brass Key");
    REQUIRE(objJson.value().getString("worldID")     == worldID);
}

TEST_CASE("demotion is the exact inverse through the same endpoint",
          "[integration][T-0379]") {
    CApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto vance   = fix.makeObject("character", "Vance");
    const auto key     = fix.makeObject("item",      "Brass Key");
    okResult(scrivi_create_edge(fix.root(), vance.c_str(), key.c_str(), "cites", ""));
    const auto before = readEdgeLog(fix);

    okResult(scrivi_promote_object(fix.root(), key.c_str(), "artifact", worldID.c_str()));
    // ⚠️ SP-103: `item` is world-scoped now, so demotion needs a destination
    // world just as promotion does — the scope is no longer CLEARED on the way
    // back down. What demotion still guarantees is the inverse KIND change with
    // the same objectID and an untouched edge log.
    auto demoted = okResult(scrivi_promote_object(fix.root(), key.c_str(), "item",
                                                  worldID.c_str()));

    REQUIRE(demoted.getString("objectID") == key);
    REQUIRE(demoted.getString("kind")     == "item");
    REQUIRE(demoted.getString("worldID")  == worldID);
    REQUIRE(demoted.getString("path").find("/items/") != std::string::npos);

    // A full round trip leaves the graph exactly as it found it.
    REQUIRE(readEdgeLog(fix) == before);

    // ⚠️ SP-104: opened from the world it was demoted INTO — `worldID` above, not
    // the fixture's lazily-created default, which is a different world entirely.
    auto opened  = okResult(scrivi_open_object(fix.root(), "item", key.c_str(),
                                               worldID.c_str()));
    auto objJson = parseJson(opened.getString("objectJson"));
    REQUIRE(objJson.ok());
    // The last assertion still demanded a CLEARED scope, contradicting this
    // test's own comment above: `item` is world-scoped since T-0409, so a
    // demoted object keeps its world rather than losing it.
    REQUIRE(objJson.value().getString("worldID") == worldID);
}

TEST_CASE("a promoted object appears under its new kind in list_objects",
          "[integration][T-0379]") {
    CApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto key     = fix.makeObject("item", "Brass Key");

    REQUIRE(objectIDsIn(okResult(scrivi_list_objects(fix.root(), "item"))).size() == 1);

    okResult(scrivi_promote_object(fix.root(), key.c_str(), "artifact", worldID.c_str()));

    // The index entry MOVED between partitions — it is not in both, and not in
    // neither.
    REQUIRE(objectIDsIn(okResult(scrivi_list_objects(fix.root(), "item"))).empty());
    auto artifacts = objectIDsIn(okResult(scrivi_list_objects(fix.root(), "artifact")));
    REQUIRE(artifacts.size() == 1);
    REQUIRE(artifacts[0] == key);
}

TEST_CASE("promotion without a worldID is refused, and changes nothing",
          "[integration][T-0379]") {
    CApiFixture fix;
    fix.makeWorld();
    const auto key = fix.makeObject("item", "Brass Key");

    auto err = errorOf(scrivi_promote_object(fix.root(), key.c_str(), "artifact", ""));
    REQUIRE(err.getString("detail") == "worldRequired");

    // A refused promotion is a no-op: the item is untouched where it was.
    auto items = objectIDsIn(okResult(scrivi_list_objects(fix.root(), "item")));
    REQUIRE(items.size() == 1);
    REQUIRE(items[0] == key);
}

TEST_CASE("promoting to the kind an object already has is refused",
          "[integration][T-0379]") {
    CApiFixture fix;
    const auto key = fix.makeObject("item", "Brass Key");
    // The sameKind guard must fire BEFORE the worldRequired check, so passing a
    // valid world still reports the real reason rather than a scope complaint.
    REQUIRE(errorOf(scrivi_promote_object(fix.root(), key.c_str(), "item",
                                          fix.defaultWorldID.c_str()))
                .getString("detail") == "sameKind");
}

// ---------------------------------------------------------------------------
// T-0406 — the `source` object kind (closes EP-031 AC1)
//
// PROJECT-scoped: a citation belongs to the work that makes it, not to a world
// that may be shared with other projects. The `cites` relation type it pairs
// with already shipped in SP-096 (T-0373), unconstrained on both ends.
// ---------------------------------------------------------------------------

TEST_CASE("a source round-trips through the ABI into objects/sources/",
          "[integration][T-0406]") {
    CApiFixture fix;

    auto created = okResult(scrivi_create_object(fix.root(), "source",
                                                 "Ferrier, A History of Salt", "",
                                                 "identity-001", "persona-001", "Test Author",
                                                 ""));
    const auto sourceID = created.getString("objectID");
    REQUIRE(created.getString("path").find("/objects/sources/") != std::string::npos);

    auto opened  = okResult(scrivi_open_object(fix.root(), "source", sourceID.c_str(), ""));
    auto objJson = parseJson(opened.getString("objectJson"));
    REQUIRE(objJson.ok());
    REQUIRE(objJson.value().getString("schema") == "scrivi.object.source.v1");
    REQUIRE(objJson.value().getString("worldID").empty());   // project-scoped

    // It participates in the index like any other project kind.
    auto sources = objectIDsIn(okResult(scrivi_list_objects(fix.root(), "source")));
    REQUIRE(sources.size() == 1);
    REQUIRE(sources[0] == sourceID);

    REQUIRE(okResult(scrivi_delete_object(fix.root(), "source", sourceID.c_str(), ""))
                .getBool("deleted"));
}

TEST_CASE("a `cites` edge relates a source to any object kind",
          "[integration][T-0406]") {
    CApiFixture fix;
    const auto worldID  = fix.makeWorld();
    const auto source   = fix.makeObject("source",    "Ferrier, A History of Salt");
    const auto vance    = fix.makeObject("character", "Vance");
    const auto artifact = fix.makeWorldObject("artifact", "Sword of Dawn", worldID);

    // `cites` is unconstrained on BOTH ends (§3.4), so one source documents a
    // project character and a world artifact alike — across the partition.
    okResult(scrivi_create_edge(fix.root(), source.c_str(), vance.c_str(), "cites", ""));
    okResult(scrivi_create_edge(fix.root(), source.c_str(), artifact.c_str(), "cites", ""));

    auto listed = okResult(scrivi_list_edges_for(fix.root(), source.c_str()));
    REQUIRE(listed.arraySize("edges") == 2);
    for (std::size_t i = 0; i < 2; ++i) {
        auto row = listed.arrayItem("edges", i);
        REQUIRE(row.getString("relationType") == "cites");
        REQUIRE(row.getString("label")        == "cites");   // forward projection
        REQUIRE_FALSE(row.getString("otherDisplayName").empty());
    }

    // Read from the far end, the SAME stored edge reads as the inverse.
    auto fromVance = okResult(scrivi_list_edges_for(fix.root(), vance.c_str()));
    REQUIRE(fromVance.arrayItem("edges", 0).getString("label") == "documented by");
}

TEST_CASE("a source with no citations is a retained orphan, like any object",
          "[integration][T-0406]") {
    CApiFixture fix;
    const auto source = fix.makeObject("source", "An Uncited Work");
    auto orphans = objectIDsIn(okResult(scrivi_list_orphaned_objects(fix.root())));
    REQUIRE(orphans.size() == 1);
    REQUIRE(orphans[0] == source);
}
