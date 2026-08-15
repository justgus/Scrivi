// EP-031 SP-097 — world packages, bindings, resolution, locking, epoch chain.
// Design: World Data Separation v0.1 §6.1–§6.5, §7; Doc 1 §7.0.
//
// The load-bearing assertions here are the identity check (§6.4 — a same-named
// package must never be silently substituted), the exclusive-create lock
// (§6.5 — exactly one winner), and the honest-status rule (§4.6 — "missing"
// only when positively established).

#include <catch2/catch_test_macros.hpp>

#include "objects/ObjectStore.hpp"
#include "objects/RelationshipStore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"
#include "worlds/WorldStore.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using namespace scrivi;
using namespace scrivi::worlds;

namespace {

struct WorldFixture {
    fs::path projectDir;
    fs::path appSupportDir;
    fs::path worldsRoot;          // where .scrivworld packages live, OUTSIDE the project

    platform::LocalFileSystem        fileSystem;
    mocks::DeterministicUUIDProvider uuidProvider;
    mocks::FixedClock                clock{"2026-08-12T00:00:00Z"};
    mocks::MockGitProvider           gitProvider;
    mocks::MockSecureStore           secureStore;
    CoreServices                     services;
    ScriviCore                       core;

    const AuthorshipRef author{
        IdentityID{"identity-001"}, PersonaID{"persona-001"}, "Test Author"};

    WorldFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-world-" + std::to_string(
                         std::chrono::steady_clock::now().time_since_epoch().count())))
        , appSupportDir(projectDir / "appsupport")
        , worldsRoot(projectDir.parent_path() /
                     (projectDir.filename().string() + "-worlds"))
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
        fs::create_directories(worldsRoot);

        CreateProjectRequest req;
        req.projectRootPath = projectDir.string();
        req.appSupportRoot  = appSupportDir.string();
        req.title           = "World Test Project";
        req.slug            = "world-test";
        req.author          = author;
        REQUIRE(core.createProject(req).ok());
    }

    ~WorldFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(worldsRoot, ec);
    }

    [[nodiscard]] std::string root() const { return projectDir.string(); }
    [[nodiscard]] std::string pkg(const std::string& name) const {
        return (worldsRoot / (name + ".scrivworld")).string();
    }

    WorldRecord makeWorld(const std::string& name = "Midgard",
                          const std::string& epoch = "The First Sundering") {
        WorldStore store{services};
        auto r = store.createWorld(root(), pkg(name), name, epoch);
        REQUIRE(r.ok());
        return r.value();
    }
};

} // namespace

// ---------------------------------------------------------------------------
// T-0403 — exclusive create
// ---------------------------------------------------------------------------

TEST_CASE("createFileExclusive: exactly one of two callers wins",
          "[integration][T-0403]") {
    WorldFixture fix;
    platform::LocalFileSystem fs_;
    const auto path = (fix.projectDir / "race.txt").string();

    // ⚠️ THE POINT OF T-0403. atomicWriteTextFile is temp-write → rename, and
    // rename OVERWRITES — both callers would "succeed" and both would believe
    // they held the lock. Exclusivity must come from the create itself.
    REQUIRE(fs_.createFileExclusive(path, "first").ok());

    auto second = fs_.createFileExclusive(path, "second");
    REQUIRE_FALSE(second.ok());
    REQUIRE(second.error().detail == "alreadyExists");

    // The winner's content is intact — the loser wrote nothing.
    std::ifstream in(path);
    std::string   body((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    REQUIRE(body == "first");
}

// ---------------------------------------------------------------------------
// T-0381 — the package
// ---------------------------------------------------------------------------

TEST_CASE("a world package is created and round-trips", "[integration][T-0381]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    REQUIRE(!w.worldID.empty());
    REQUIRE(w.displayName == "Midgard");
    REQUIRE(w.epochLabel  == "The First Sundering");

    // Self-contained (§6.1): its own identity, index, and kind directories.
    REQUIRE(fs::exists(fs::path(fix.pkg("Midgard")) / "world.json"));
    REQUIRE(fs::exists(fs::path(fix.pkg("Midgard")) / "index.json"));
    for (const char* sub : {"artifacts", "rules", "chronicles", "factions", "assets"}) {
        REQUIRE(fs::is_directory(fs::path(fix.pkg("Midgard")) / sub));
    }

    // ...and the project got its binding in the same operation (§7).
    WorldStore store{fix.services};
    auto b = store.loadBinding(fix.root(), w.worldID);
    REQUIRE(b.ok());
    REQUIRE(b.value().worldID == w.worldID);
    REQUIRE(b.value().epochOffsetMs == 0);   // a new binding sits on the project epoch
}

TEST_CASE("creating over an existing package is refused", "[integration][T-0381]") {
    WorldFixture fix;
    fix.makeWorld();

    // Would otherwise silently adopt or destroy someone else's world.
    WorldStore store{fix.services};
    REQUIRE_FALSE(store.createWorld(fix.root(), fix.pkg("Midgard"), "Other", "x").ok());
}

TEST_CASE("an existing package can be added to a project", "[integration][T-0381]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    // Drop the binding, then re-add the same package — adding is read-only
    // toward the world and takes no lock.
    WorldStore store{fix.services};
    REQUIRE(store.removeReference(fix.root(), w.worldID).ok());
    REQUIRE_FALSE(store.loadBinding(fix.root(), w.worldID).ok());

    auto added = store.addWorld(fix.root(), fix.pkg("Midgard"));
    REQUIRE(added.ok());
    REQUIRE(added.value().worldID == w.worldID);   // same identity, not a new world
}

// ---------------------------------------------------------------------------
// T-0382 — resolution and relink
// ---------------------------------------------------------------------------

TEST_CASE("a bound world resolves to its package", "[integration][T-0382]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    WorldStore store{fix.services};
    auto res = store.resolve(fix.root(), w.worldID);
    REQUIRE(res.status == WorldStatus::available);
    REQUIRE(res.world.worldID == w.worldID);
}

TEST_CASE("⚠️ a same-named package with a DIFFERENT worldID is not the world",
          "[integration][T-0382]") {
    WorldFixture fix;
    auto w = fix.makeWorld("Midgard");

    // Replace the package with a different world of the same display name —
    // exactly the substitution §6.4's identity check exists to prevent. Without
    // it, every edge into the original world would silently re-target.
    fs::remove_all(fix.pkg("Midgard"));
    {
        WorldStore other{fix.services};
        // Build a second world elsewhere, then move it into the first's path.
        auto w2 = other.createWorld(fix.root(), fix.pkg("Impostor"), "Midgard", "x");
        REQUIRE(w2.ok());
        REQUIRE(w2.value().worldID != w.worldID);
        fs::rename(fix.pkg("Impostor"), fix.pkg("Midgard"));
    }

    WorldStore store{fix.services};
    auto res = store.resolve(fix.root(), w.worldID);
    REQUIRE(res.status != WorldStatus::available);   // refused, NOT substituted
    REQUIRE(res.packagePath.empty());
}

TEST_CASE("an absent package reports a status, never an error",
          "[integration][T-0382]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    fs::remove_all(fix.pkg("Midgard"));

    WorldStore store{fix.services};
    auto res = store.resolve(fix.root(), w.worldID);
    REQUIRE(res.status != WorldStatus::available);

    // The containing folder is present and readable and the package is gone, so
    // "missing" is POSITIVELY ESTABLISHED here and is the honest report.
    REQUIRE(res.status == WorldStatus::missing);
}

TEST_CASE("an unresolvable reference falls back to 'unavailable', never a guess",
          "[integration][T-0382]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    // Point the binding at a path whose CONTAINER does not exist either — the
    // cause cannot be determined, so the honest answer is the generic status.
    WorldStore store{fix.services};
    auto b = store.loadBinding(fix.root(), w.worldID);
    REQUIRE(b.ok());
    auto binding = b.value();
    binding.reference.lastKnownPath         = "";
    binding.reference.lastKnownAbsolutePath =
        (fix.worldsRoot / "no-such-dir" / "Ghost.scrivworld").string();
    REQUIRE(store.saveBinding(fix.root(), binding).ok());

    auto res = store.resolve(fix.root(), w.worldID);
    // ⚠️ Doc 3 §4.6: a wrong "missing" invites destructive remedies (clearing
    // references, restoring from backup) when the world may be perfectly intact
    // on an unreachable volume. NEVER GUESS.
    REQUIRE(res.status == WorldStatus::unavailable);
}

TEST_CASE("a new world package has a directory for EVERY world-scoped kind",
          "[integration][SP-104]") {
    WorldFixture fix;
    auto w = fix.makeWorld();
    const auto pkg = fix.pkg("Midgard");

    // ⚠️ The skeleton was seven hardcoded literals naming the PRE-SP-104 scope,
    // so a world created after T-0409 had no characters/ directory. Asserting
    // the DERIVED set means a kind whose scope changes cannot leave it stale —
    // this is the assertion the four earlier restatements never had.
    for (auto kind : kAllStorableKinds) {
        if (!objectKindIsWorldScoped(kind)) { continue; }
        INFO("world-scoped kind: " << objectKindName(kind));
        REQUIRE(fs::is_directory(fs::path(pkg) / objectKindSubdir(kind)));
    }

    // ...and the project-scoped kind must NOT get one here.
    REQUIRE_FALSE(fs::exists(fs::path(pkg) / objectKindSubdir(ObjectKind::source)));
}

TEST_CASE("a PRESENT but UNREADABLE package is never reported as 'missing'",
          "[integration][SP-104]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    // ⚠️ THE SANDBOX CASE. The package is intact on disk, but unreadable — which
    // is exactly what the macOS App Sandbox produces for a world outside the
    // granted paths. Simulated here by removing read permission.
    //
    // This previously reported `missing`, because a failed read plus a readable
    // PARENT was taken as proof of absence. A writer was told an intact world
    // was gone — the precise error §4.6 forbids, since "missing" invites
    // destructive remedies (clear the reference, restore from backup).
    const auto pkg = fix.pkg("Midgard");
    std::error_code ec;
    fs::permissions(pkg, fs::perms::none, ec);
    if (ec) { SUCCEED("cannot drop permissions here"); return; }

    WorldStore store{fix.services};
    auto res = store.resolve(fix.root(), w.worldID);

    // Restore before asserting, so a failure cannot leave the tree unremovable.
    fs::permissions(pkg, fs::perms::owner_all, ec);

    REQUIRE(res.status != WorldStatus::available);
    REQUIRE(res.status != WorldStatus::missing);      // the defect
    REQUIRE(res.status == WorldStatus::unavailable);  // the honest answer
}

TEST_CASE("relink verifies identity before accepting a new path",
          "[integration][T-0382]") {
    WorldFixture fix;
    auto w     = fix.makeWorld("Midgard");
    auto other = fix.makeWorld("Vanaheim");

    WorldStore store{fix.services};

    // Pointing Midgard's binding at Vanaheim's package must be refused.
    auto bad = store.relink(fix.root(), w.worldID, fix.pkg("Vanaheim"));
    REQUIRE_FALSE(bad.ok());
    REQUIRE(bad.error().detail == "worldIDMismatch");

    // Moving the real package and relinking to it succeeds.
    fs::rename(fix.pkg("Midgard"), fix.pkg("Midgard-moved"));
    REQUIRE(store.relink(fix.root(), w.worldID, fix.pkg("Midgard-moved")).ok());
    REQUIRE(store.resolve(fix.root(), w.worldID).status == WorldStatus::available);
    (void)other;
}

TEST_CASE("removing a reference leaves the world package untouched",
          "[integration][T-0382]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    WorldStore store{fix.services};
    REQUIRE(store.removeReference(fix.root(), w.worldID).ok());

    // Worlds are shared, not owned by any one project.
    REQUIRE(fs::exists(fs::path(fix.pkg("Midgard")) / "world.json"));
}

TEST_CASE("a project with no worlds does nothing world-related",
          "[integration][T-0382]") {
    WorldFixture fix;   // no world created

    WorldStore store{fix.services};
    auto ids = store.listBoundWorldIDs(fix.root());
    REQUIRE(ids.ok());
    REQUIRE(ids.value().empty());

    auto list = store.listWorlds(fix.root());
    REQUIRE(list.ok());
    REQUIRE(list.value().empty());
}

// ---------------------------------------------------------------------------
// T-0383 — write locking
// ---------------------------------------------------------------------------

TEST_CASE("exactly one writer holds the world lock", "[integration][T-0383]") {
    WorldFixture fix;
    fix.makeWorld();
    const auto path = fix.pkg("Midgard");

    WorldLock a{fix.services, path};
    WorldLock b{fix.services, path};

    REQUIRE(a.acquire("project-a").ok());

    auto lost = b.acquire("project-b");
    REQUIRE_FALSE(lost.ok());
    REQUIRE(lost.error().detail == "worldLocked");   // reports; never hangs

    // Released, the next writer gets it.
    REQUIRE(a.release().ok());
    REQUIRE(b.acquire("project-b").ok());
    REQUIRE(b.release().ok());
}

TEST_CASE("a stale lock is broken; a fresh one is not", "[integration][T-0383]") {
    WorldFixture fix;
    fix.makeWorld();
    const auto path = fix.pkg("Midgard");

    SECTION("fresh lock survives") {
        WorldLock held{fix.services, path};
        REQUIRE(held.acquire("project-a").ok());

        WorldLock other{fix.services, path};
        REQUIRE_FALSE(other.acquire("project-b").ok());
    }

    SECTION("stale lock is broken") {
        // Hand-write a lock whose heartbeat is well over the 60 s bound. A
        // crashed writer must not lock a world permanently (§6.5).
        std::ofstream out(fs::path(path) / ".lock", std::ios::trunc);
        out << R"({"schema":"scrivi.world-lock.v1","lockID":"lock_dead",)"
            << R"("holder":{"host":"gone","pid":1,"projectID":"p"},)"
            << R"("acquiredAt":"2026-08-11T00:00:00Z",)"
            << R"("heartbeatAt":"2026-08-11T00:00:00Z"})";
        out.close();

        WorldLock fresh{fix.services, path};
        REQUIRE(fresh.acquire("project-b").ok());
        REQUIRE(fresh.release().ok());
    }
}

TEST_CASE("an unparseable lock file does not lock a world forever",
          "[integration][T-0383]") {
    WorldFixture fix;
    fix.makeWorld();
    const auto path = fix.pkg("Midgard");

    { std::ofstream out(fs::path(path) / ".lock", std::ios::trunc); out << "{ garbage"; }

    WorldLock lock{fix.services, path};
    REQUIRE(lock.acquire("project-a").ok());
    REQUIRE(lock.release().ok());
}

// ---------------------------------------------------------------------------
// T-0384 — the epoch chain
// ---------------------------------------------------------------------------

TEST_CASE("the epoch chain resolves timeline → world → project",
          "[integration][T-0384]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    WorldStore store{fix.services};
    REQUIRE(store.setWorldEpochOffset(fix.root(), w.worldID, -94'608'000'000LL).ok());
    REQUIRE(store.setTimelineEpochOffset(fix.root(), w.worldID, "tl-republic",
                                         31'536'000'000LL).ok());

    auto off = store.resolveTimelineProjectOffset(fix.root(), w.worldID, "tl-republic");
    REQUIRE(off.ok());
    // timeline.epochOffsetMs + binding.epochOffsetMs
    REQUIRE(off.value() == 31'536'000'000LL + -94'608'000'000LL);
}

TEST_CASE("two timelines in one world relate WITHOUT any project",
          "[integration][T-0384]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    WorldStore store{fix.services};
    REQUIRE(store.setTimelineEpochOffset(fix.root(), w.worldID, "tl-a", 1'000).ok());
    REQUIRE(store.setTimelineEpochOffset(fix.root(), w.worldID, "tl-b", 4'000).ok());

    auto a = store.timelineEpochOffset(fix.root(), w.worldID, "tl-a");
    auto b = store.timelineEpochOffset(fix.root(), w.worldID, "tl-b");
    REQUIRE(a.ok());
    REQUIRE(b.ok());

    // Pure world-relative arithmetic — the cross-history comparison the ruling
    // exists to support. No binding, no project, involved.
    REQUIRE(b.value() - a.value() == 3'000);
}

TEST_CASE("rebinding a world changes exactly ONE number",
          "[integration][T-0384]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    WorldStore store{fix.services};
    REQUIRE(store.setTimelineEpochOffset(fix.root(), w.worldID, "tl-a", 1'000).ok());
    REQUIRE(store.setTimelineEpochOffset(fix.root(), w.worldID, "tl-b", 4'000).ok());
    REQUIRE(store.setWorldEpochOffset(fix.root(), w.worldID, 500).ok());

    const auto before_a = store.timelineEpochOffset(fix.root(), w.worldID, "tl-a").value();
    const auto before_b = store.timelineEpochOffset(fix.root(), w.worldID, "tl-b").value();

    // Rebind.
    REQUIRE(store.setWorldEpochOffset(fix.root(), w.worldID, -9'000).ok());

    // Every timeline follows correctly, and NO timeline offset was rewritten.
    REQUIRE(store.timelineEpochOffset(fix.root(), w.worldID, "tl-a").value() == before_a);
    REQUIRE(store.timelineEpochOffset(fix.root(), w.worldID, "tl-b").value() == before_b);
    REQUIRE(store.resolveTimelineProjectOffset(fix.root(), w.worldID, "tl-a").value()
            == before_a + -9'000);
}

TEST_CASE("editing the binding offset never mutates world.json",
          "[integration][T-0384]") {
    WorldFixture fix;
    auto w = fix.makeWorld();
    const auto worldJson = (fs::path(fix.pkg("Midgard")) / "world.json").string();

    std::string before;
    { std::ifstream in(worldJson);
      before.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()); }

    WorldStore store{fix.services};
    REQUIRE(store.setWorldEpochOffset(fix.root(), w.worldID, 123'456).ok());

    std::string after;
    { std::ifstream in(worldJson);
      after.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()); }

    // The world's own epoch is intrinsic and travels with it when shared; two
    // projects may bind the same world at different offsets.
    REQUIRE(before == after);
}

// ---------------------------------------------------------------------------
// T-0385 / T-0404 — world-scoped kinds become creatable
// ---------------------------------------------------------------------------

TEST_CASE("world-scoped kinds round-trip IN WORLD SCOPE (closes AC1's gated half)",
          "[integration][T-0385]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    for (auto kind : {ObjectKind::artifact, ObjectKind::chronicle,
                      ObjectKind::faction,  ObjectKind::rule}) {
        CAPTURE(objectKindName(kind));

        CreateObjectRequest req;
        req.projectRootPath = fix.root();
        req.objectKind      = kind;
        req.displayName     = "The " + objectKindName(kind);
        req.author          = fix.author;
        req.worldID         = w.worldID;

        auto created = fix.core.createObject(req);
        REQUIRE(created.ok());

        // The file lands INSIDE the world package, never under objects/.
        // (Compared by suffix rather than prefix: resolve() canonicalizes, and
        // macOS canonicalizes /tmp to /private/tmp, so the absolute prefixes
        // legitimately differ.)
        REQUIRE(created.value().path.find("Midgard.scrivworld") != std::string::npos);
        REQUIRE(created.value().path.find(objectKindSubdir(kind)) != std::string::npos);
        REQUIRE(created.value().path.find("/objects/") == std::string::npos);

        OpenObjectRequest oreq;
        oreq.projectRootPath = fix.root();
        oreq.objectKind      = kind;
        oreq.objectID        = created.value().objectID;
        oreq.worldID         = w.worldID;
        auto opened = fix.core.openObject(oreq);
        REQUIRE(opened.ok());
        REQUIRE(worldObjectFields(opened.value().object).worldID == w.worldID);
    }
}

TEST_CASE("`rule` is world-scoped and no longer lives under objects/",
          "[integration][T-0404]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    // T-0404: rules govern an ENVIRONMENT, not a manuscript (Doc 1 §3,
    // Doc 3 §7.2). No migration was written — nothing has shipped — so the only
    // requirement is that new rules land in world scope.
    REQUIRE(objectKindIsWorldScoped(ObjectKind::rule));

    CreateObjectRequest req;
    req.projectRootPath = fix.root();
    req.objectKind      = ObjectKind::rule;
    req.displayName     = "Magic System";
    req.author          = fix.author;
    req.worldID         = w.worldID;
    auto created = fix.core.createObject(req);
    REQUIRE(created.ok());

    REQUIRE(created.value().path.find("Midgard.scrivworld") != std::string::npos);
    REQUIRE(created.value().path.find("/rules/")            != std::string::npos);
    REQUIRE(created.value().path.find("/objects/")          == std::string::npos);

    // And the project package never grows an objects/rules/ directory.
    REQUIRE_FALSE(fs::exists(fix.projectDir / "objects" / "rules"));
}

TEST_CASE("⚠️ faction↔faction symmetric duplicate rejection (closes AC3's last clause)",
          "[integration][T-0385][T-0375]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    // Doc 1 §9 AC4 names THIS case specifically and warns it is "the one that
    // regresses silently". It was deferred from SP-096 because `faction` could
    // not exist yet.
    auto makeFaction = [&](const std::string& name) {
        CreateObjectRequest req;
        req.projectRootPath = fix.root();
        req.objectKind      = ObjectKind::faction;
        req.displayName     = name;
        req.author          = fix.author;
        req.worldID         = w.worldID;
        auto r = fix.core.createObject(req);
        REQUIRE(r.ok());
        return r.value().objectID.value;
    };

    const auto vance = makeFaction("House Vance");
    const auto ordo  = makeFaction("House Ordo");

    objects::RelationTypeStore types{fix.services};
    objects::RelationType atWar;
    atWar.code               = "at-war-with";
    atWar.forwardLabel       = "at war with";
    atWar.inverseLabel       = "at war with";
    atWar.sourceKind         = ObjectKind::faction;
    atWar.targetKind         = ObjectKind::faction;
    atWar.canonicalDirection = objects::CanonicalDirection::lexical;
    atWar.symmetric          = true;
    REQUIRE(types.upsert(fix.root(), atWar).ok());

    objects::RelationshipStore edges{fix.services};
    REQUIRE(edges.create(fix.root(), vance, ordo, "at-war-with", "").ok());

    // Same relationship, stated the other way round — must be ONE edge.
    auto dup = edges.create(fix.root(), ordo, vance, "at-war-with", "");
    REQUIRE_FALSE(dup.ok());
    REQUIRE(dup.error().detail == "duplicateEdge");
    REQUIRE(edges.load(fix.root()).value().size() == 1);
}

TEST_CASE("a cross-partition edge resolves like a same-partition one (AC10)",
          "[integration][T-0385]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    // ⚠️ SP-104: the project-side endpoint is now `source`. A `character` is
    // world-scoped since T-0409, so a character↔artifact pair is same-partition
    // and could not exercise AC10 at all — the assertion would have passed
    // while testing nothing. `source` is the sole project-scoped kind, which is
    // what makes this pair genuinely cross-partition.
    CreateObjectRequest creq;
    creq.projectRootPath = fix.root();
    creq.objectKind      = ObjectKind::source;
    creq.displayName     = "Ada's Field Notes";
    creq.author          = fix.author;
    auto ada = fix.core.createObject(creq);
    REQUIRE(ada.ok());

    // …related to a WORLD-scoped artifact.
    CreateObjectRequest areq;
    areq.projectRootPath = fix.root();
    areq.objectKind      = ObjectKind::artifact;
    areq.displayName     = "Sword of Dawn";
    areq.author          = fix.author;
    areq.worldID         = w.worldID;
    auto sword = fix.core.createObject(areq);
    REQUIRE(sword.ok());

    objects::RelationshipStore edges{fix.services};
    auto e = edges.create(fix.root(), ada.value().objectID.value,
                          sword.value().objectID.value, "cites", "");
    REQUIRE(e.ok());

    auto views = edges.listFor(fix.root(), ada.value().objectID.value);
    REQUIRE(views.ok());
    REQUIRE(views.value().size() == 1);
    // The far endpoint resolves by NAME across the partition boundary.
    REQUIRE(views.value()[0].otherDisplayName == "Sword of Dawn");
}

TEST_CASE("the cached index names world objects for pending display",
          "[integration][T-0385]") {
    WorldFixture fix;
    auto w = fix.makeWorld();

    CreateObjectRequest req;
    req.projectRootPath = fix.root();
    req.objectKind      = ObjectKind::artifact;
    req.displayName     = "Sword of Dawn";
    req.author          = fix.author;
    req.worldID         = w.worldID;
    REQUIRE(fix.core.createObject(req).ok());

    WorldStore store{fix.services};
    auto res = store.resolve(fix.root(), w.worldID);
    REQUIRE(res.status == WorldStatus::available);
    REQUIRE(store.refreshCachedIndex(fix.root(), w.worldID, res.packagePath).ok());

    auto b = store.loadBinding(fix.root(), w.worldID);
    REQUIRE(b.ok());

    // §6.3: the cache exists so a pending entry can read "⟨Midgard: Sword of
    // Dawn⟩" instead of a bare UUID — a writer asked whether to clear world
    // references cannot decide blind.
    bool found = false;
    for (const auto& e : b.value().cachedIndex) {
        if (e.displayName == "Sword of Dawn") { found = true; }
    }
    REQUIRE(found);
}
