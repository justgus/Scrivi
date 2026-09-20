// SP-144 — [I-0231] read amplification: LOCATE it (AC1), BOUND it (AC2/AC3),
// and GUARD it (AC7).
//
// ⚠️ WHY THIS FILE EXISTS AT ALL. [I-0231] measured 8,044 `openat` + 14,362
// `read` syscalls for ONE `scrivi_open_project` over a 150-file project — each
// scene sidecar opened ~96x, each chapter sidecar ~98x, and an ABSENT world
// `binding.json` probed 188 times, every one an ENOENT. On local disk the page
// cache absorbs all of it (`0.21 s`); on a `cache=none` CIFS mount the same
// call takes `154 s` (~730x).
//
// ⛔ THE PAGE CACHE IS WHY NO EXISTING SUITE CATCHES THIS. Every test here runs
// on local disk, so a wall-clock assertion passes today at 0.21 s while
// performing 55,574 reads. ✅ AC7 states the consequence: the regression guard
// must fail on READ COUNT, not on elapsed time. That is what these tests assert.
//
// ⚠️ AC1 forbids locating the defect by reading the code — [I-0181]'s three
// inference-driven narrowings are the cited warning. `CountingFileSystem` wraps
// the REAL filesystem and attributes every call to the pass that made it, so the
// answer below is measured, not argued.

#include "mocks/CountingFileSystem.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "scrivi/ScriviCore.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"

// The opener's own passes, driven individually for AC1's attribution.
#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "objects/RelationTypes.hpp"
#include "objects/RelationshipStore.hpp"
#include "project_package/ProjectValidator.hpp"
#include "worlds/WorldStore.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;

// ⚠️ THESE BOUNDS ARE SET FROM MEASUREMENT (AC1), NOT CHOSEN IN ADVANCE.
// SP-144 AC2 is explicit that "a number invented before the cause is located is
// a guess". They are filled in from the AC1 run and the reason for any value
// above 1 is recorded beside it.
constexpr std::size_t kMaxReadsPerFile             = SCRIVI_AMP_MAX_READS_PER_FILE;
constexpr std::size_t kMaxProbesForAbsentBinding   = SCRIVI_AMP_MAX_ABSENT_PROBES;
constexpr double      kScalingTolerance            = SCRIVI_AMP_SCALING_TOLERANCE;

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-amp-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    TempDir(const TempDir&)            = delete;
    TempDir& operator=(const TempDir&) = delete;
    [[nodiscard]] std::string str() const { return path.string(); }
};

// A project of `chapters` x `scenesPerChapter`, built through the REAL core so
// the on-disk shape is the shipped one — order-key slugs, sidecars, caches and
// all. ⚠️ Hand-built fixtures have drifted from the real layout before
// (CLAUDE.md's standing rule on restated kind lists is the same failure mode);
// creating through the core means this cannot.
struct Harness {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem          real;
    scrivi::testing::CountingFileSystem        counting{real};
    scrivi::mocks::FixedClock                  clock{"2026-09-20T00:00:00Z"};
    scrivi::mocks::DeterministicUUIDProvider   uuid;
    scrivi::mocks::MockSecureStore             secure;
    scrivi::mocks::MockGitProvider             git;

    scrivi::CoreServices services;

    std::string projectID;

    Harness() {
        services.fileSystem   = &counting;
        services.secureStore  = &secure;
        services.clock        = &clock;
        services.uuidProvider = &uuid;
        services.gitProvider  = &git;
    }

    [[nodiscard]] std::string root()       const { return projectDir.str(); }
    [[nodiscard]] std::string appSupport() const { return appSupportDir.str(); }

    // Builds the project, then RESETS the counters — construction cost is not
    // what is being measured.
    void build(int chapters, int scenesPerChapter) {
        scrivi::ScriviCore core{services};

        const scrivi::AuthorshipRef author{
            scrivi::IdentityID{"identity-001"},
            scrivi::PersonaID{"persona-001"},
            "Test Author"};

        scrivi::CreateProjectRequest req;
        req.projectRootPath = root();
        req.appSupportRoot  = appSupport();
        req.title           = "Amplification Novel";
        req.slug            = "amplification-novel";
        req.author          = author;

        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID.value;

        // Chapter 1 exists from creation with one scene; add the rest.
        for (int c = 2; c <= chapters; ++c) {
            scrivi::CreateChapterRequest cr;
            cr.projectRootPath = root();
            cr.appSupportRoot  = appSupport();
            cr.projectID       = scrivi::ProjectID{projectID};
            cr.author          = author;
            auto r = core.createChapter(cr);
            REQUIRE(r.ok());
        }

        // Scenes are written directly: going through createScene would be
        // correct but slow, and the SHAPE on disk is what matters here.
        int ci = 0;
        for (const auto& entry : fs::directory_iterator(projectDir.path / "manuscript")) {
            if (!entry.is_directory()) { continue; }
            ++ci;
            for (int s = 2; s <= scenesPerChapter; ++s) {
                char key[8];
                std::snprintf(key, sizeof key, "%03d", s);
                writeScene(entry.path().string(), key,
                           "scene-c" + std::to_string(ci) + "-" + key);
            }
        }

        counting.reset();
    }

    static void writeScene(const std::string& chapterDir,
                           const std::string& orderKey,
                           const std::string& sceneID) {
        scrivi::schemas::SceneMetaData meta;
        meta.sceneID.value         = sceneID;
        meta.title                 = "Scene " + orderKey;
        meta.slug                  = "scene-" + orderKey;
        meta.createdAt             = "2026-06-01T00:00:00Z";
        meta.modifiedAt            = "2026-06-01T00:00:00Z";
        meta.modifiedByIdentityID  = "identity-001";
        meta.modifiedByPersonaID   = "persona-001";
        meta.modifiedByDisplayName = "Test Author";
        meta.contentPath           = orderKey + "-scene.md";
        {
            std::ofstream f(chapterDir + "/" + orderKey + "-scene.meta.json", std::ios::binary);
            f << scrivi::schemas::serializeSceneMeta(meta);
        }
        {
            std::ofstream f(chapterDir + "/" + orderKey + "-scene.md", std::ios::binary);
            f << "Body of scene " << orderKey << ".\n";
        }
    }

    scrivi::Result<scrivi::OpenProjectResult> open() {
        scrivi::ScriviCore core{services};
        scrivi::OpenProjectRequest req;
        req.projectRootPath = root();
        req.appSupportRoot  = appSupport();
        return core.openProject(req);
    }
};

// Counts the files a project actually contains — the denominator for
// "reads per file", which is the unit [I-0231] is stated in.
std::size_t countFiles(const fs::path& root) {
    std::size_t n = 0;
    for (const auto& e : fs::recursive_directory_iterator(root)) {
        if (e.is_regular_file()) { ++n; }
    }
    return n;
}

// Prints the per-phase and per-path breakdown. ⚠️ This is AC1's DELIVERABLE, not
// debug chatter: the Sprint requires the per-site figures to be REPORTED, and a
// number that only ever lived in an assertion cannot be quoted in a Sprint doc.
void report(const scrivi::testing::CountingFileSystem& cfs,
            std::size_t fileCount,
            const std::string& label) {
    const auto& t = cfs.total();
    std::cout << "\n=== [I-0231] " << label << " ===\n"
              << "  files in project        : " << fileCount << "\n"
              << "  TOTAL filesystem calls  : " << t.total() << "\n"
              << "    readTextFile          : " << t.readTextFile << "\n"
              << "    listDirectory         : " << t.listDirectory << "\n"
              << "    exists                : " << t.exists << "\n"
              << "    isDirectory           : " << t.isDirectory << "\n"
              << "    deviceID              : " << t.deviceID << "\n"
              << "    writes (atomic/append): " << (t.atomicWrite + t.appendTextFile) << "\n";
    if (fileCount > 0) {
        std::cout << "  calls per file          : "
                  << (static_cast<double>(t.total()) / static_cast<double>(fileCount)) << "\n";
    }

    std::cout << "  -- by phase --\n";
    for (const auto& [phase, c] : cfs.byPhase()) {
        std::cout << "    " << phase << ": total=" << c.total()
                  << " reads=" << c.readTextFile << " lists=" << c.listDirectory
                  << " exists=" << c.exists << "\n";
    }

    std::cout << "  -- 12 hottest paths --\n";
    for (const auto& [path, c] : cfs.hottestPaths(12)) {
        std::cout << "    " << c.total() << "x (read=" << c.readTextFile
                  << " exists=" << c.exists << ") " << path << "\n";
    }
    std::cout << std::endl;
}

} // namespace

// ---------------------------------------------------------------------------
// AC1 — LOCATE the amplification: per-call-site counts for ONE open
// ---------------------------------------------------------------------------
//
// ⚠️ This test's JOB is to print the attribution table. It asserts only what
// must be true for the table to be meaningful (the open succeeded, and work was
// actually done), because AC1 is a MEASUREMENT criterion — ✅ the BOUNDS it
// reveals are asserted by the AC2/AC3 tests below, which is where a regression
// should fail.
TEST_CASE("[I-0231] AC1 — attribute every filesystem call in one project open",
          "[integration][SP-144][I-0231][AC1]")
{
    Harness h;
    h.build(/*chapters=*/10, /*scenesPerChapter=*/6);

    const auto files = countFiles(h.projectDir.path);

    auto opened = h.open();
    REQUIRE(opened.ok());
    REQUIRE(opened.value().mode != scrivi::OpenMode::repairRequired);

    report(h.counting, files, "one scrivi_open_project (10 chapters x 6 scenes)");

    // The measurement ran and the project really was read.
    REQUIRE(h.counting.total().total() > 0);
    REQUIRE(files > 0);

    // --- AC1's ATTRIBUTION -------------------------------------------------
    //
    // ⚠️ The totals above say the amplification is REAL but not WHERE it is
    // introduced. ✅ Re-running the opener's passes INDIVIDUALLY, in the order
    // `ProjectOpener::open` runs them, attributes each read to the pass that
    // makes it — without instrumenting production code, which would change the
    // thing being measured.
    //
    // ⚠️ These are the SAME free functions and types the opener calls; if the
    // opener's sequence changes, this drifts and must be re-derived. It is a
    // measurement scaffold, not a second implementation.
    Harness h2;
    h2.build(/*chapters=*/10, /*scenesPerChapter=*/6);

    {
        auto& fsx = h2.counting;

        fsx.beginPhase("1-migrateChapterOrderKeys");
        (void)scrivi::manuscript::migrateChapterOrderKeys(fsx, h2.root());

        fsx.beginPhase("2-rebuildIndexIfInconsistent");
        (void)scrivi::manuscript::rebuildIndexIfInconsistent(fsx, h2.root());

        fsx.beginPhase("3-migrateScenes");
        (void)scrivi::manuscript::migrateScenes(fsx, h2.root());

        fsx.beginPhase("4-relationTypes+repairDangling");
        {
            scrivi::objects::RelationTypeStore types{h2.services};
            (void)types.load(h2.root());
            scrivi::objects::RelationshipStore graph{h2.services};
            (void)graph.repairDangling(h2.root());
        }

        fsx.beginPhase("5-validate");
        {
            scrivi::project_package::ProjectValidator validator{h2.services};
            (void)validator.validate(h2.root());
        }

        fsx.beginPhase("6-resolveOrder");
        {
            scrivi::manuscript::ManuscriptOrderResolver resolver{h2.services};
            (void)resolver.resolve(h2.root());
        }

        fsx.endPhase();
    }

    report(h2.counting, files, "SAME open, attributed to the opener's passes");
}

// ---------------------------------------------------------------------------
// AC2 — each sidecar is read ONCE per open unless a reason is recorded
// ---------------------------------------------------------------------------
//
// ⚠️ THE BOUND IS PER-FILE, NOT TOTAL. A total-call budget would have to be
// re-tuned every time the manuscript grows, and would pass a 96x amplification
// on a small enough project. ✅ What [I-0231] is about is the MULTIPLE: the same
// file read over and over within one call.
//
// ⚠️ THE NUMBER BELOW IS DELIBERATELY NOT 1. Some repetition is legitimate — a
// migration pass reads a sidecar to decide whether it needs rewriting, and the
// resolver reads it to build the order. ✅ The bound is set from the MEASURED
// post-fix figure with headroom, and the Sprint requires the reason to be
// recorded rather than the bound to be quietly raised.
TEST_CASE("[I-0231] AC2 — no file is re-read an unbounded number of times",
          "[integration][SP-144][I-0231][AC2]")
{
    Harness h;
    h.build(/*chapters=*/10, /*scenesPerChapter=*/6);

    auto opened = h.open();
    REQUIRE(opened.ok());

    // The worst-repeated single path in the whole open.
    std::size_t worstReads = 0;
    std::string worstPath;
    for (const auto& [path, c] : h.counting.byPath()) {
        if (c.readTextFile > worstReads) { worstReads = c.readTextFile; worstPath = path; }
    }

    INFO("hottest path: " << worstPath << " read " << worstReads << " times");

    // ⚠️ [I-0231] measured ~96 reads of a single scene sidecar. This bound is
    // what must not come back.
    CHECK(worstReads <= kMaxReadsPerFile);
}

// ---------------------------------------------------------------------------
// AC3 — an ABSENT world binding.json costs at most ONE probe per open
// ---------------------------------------------------------------------------
//
// ⚠️ [I-0231] measured 188 probes of a `binding.json` that DOES NOT EXIST — 188
// network round-trips, on a `cache=none` mount, for a negative answer that the
// first one already gave.
//
// ⛔ THE FIX IS NOT TO DELETE THE EMPTY WORLD DIRECTORY. AC3 says so explicitly:
// that is data the user owns and [I-0223] holds what it means. ✅ The probe count
// is the defect; the directory is not.
TEST_CASE("[I-0231] AC3 — an absent world binding.json is probed once per sweep",
          "[integration][SP-144][I-0231][AC3]")
{
    Harness h;
    h.build(/*chapters=*/4, /*scenesPerChapter=*/4);

    // A world that is BOUND but whose package is ABSENT — the rig's shape. The
    // binding exists (the project remembers the world); the package it names
    // does not (the volume is away, or the folder was moved).
    //
    // ⛔ NOT an empty directory with no binding: that shape is never resolved at
    // all, so a test built on it passes while measuring nothing. ✅ This one
    // makes `WorldStore::resolve` actually run.
    const std::string worldID = "world-amp-0001";
    {
        scrivi::worlds::WorldStore ws{h.services};
        scrivi::worlds::WorldBindingRecord b;
        b.worldID                        = worldID;
        b.displayName                    = "Eskandar";
        b.reference.lastKnownPath        = "../Eskandar.scrivworld";
        b.reference.lastKnownAbsolutePath =
            (h.projectDir.path.parent_path() / "Eskandar.scrivworld").string();
        REQUIRE(ws.saveBinding(h.root(), b).ok());
    }

    // Edges whose endpoints resolve against no object — every one sends
    // EndpointResolver through the bound-world loop, which is the multiplier
    // that turned one absent binding into 188 reads on the rig.
    //
    // ⚠️ Written to the log DIRECTLY rather than through `create`, which refuses
    // an endpoint that does not resolve (T-0380). The defect under test is in
    // REPAIR, which must cope with exactly the log a damaged project has.
    {
        const auto logPath = h.projectDir.path / "objects" / "relationships.jsonl";
        fs::create_directories(logPath.parent_path());
        std::ofstream log(logPath, std::ios::binary);
        for (int i = 1; i <= 12; ++i) {
            log << R"({"rec":"edge","seq":)" << i
                << R"(,"edgeID":"edge-)" << i
                << R"(","from":"ghost-a-)" << i
                << R"(","to":"ghost-b-)" << i
                << R"(","relationType":"cites","note":"","sortIndex":0.0})"
                << "\n";
        }
    }

    const auto binding =
        (h.projectDir.path / "worlds" / worldID / "binding.json").string();

    h.counting.reset();
    {
        scrivi::objects::RelationshipStore graph{h.services};
        (void)graph.repairDangling(h.root());
    }

    const auto it     = h.counting.byPath().find(binding);
    const auto reads  = (it == h.counting.byPath().end()) ? 0 : it->second.readTextFile;
    const auto probes = (it == h.counting.byPath().end()) ? 0 : it->second.probes();

    std::cout << "\n=== [I-0231] AC3 ===\n"
              << "  binding.json reads over one repairDangling sweep: " << reads << "\n"
              << "  binding.json probes                             : " << probes << "\n"
              << std::endl;

    // ⚠️ THE SWEEP IS THE UNIT, NOT THE CALL. 12 edges x 2 endpoints x 1 bound
    // world = 24 resolutions that each used to read this file. ✅ One cache for
    // the sweep makes it ONE read, and that is what must not regress.
    INFO("absent binding.json read " << reads << " times across the sweep");
    CHECK(reads <= kMaxProbesForAbsentBinding);
}

// ---------------------------------------------------------------------------
// AC7 — the guard fails on READ COUNT, and scales sub-linearly in re-reads
// ---------------------------------------------------------------------------
//
// ⚠️ THE RATIO IS THE ASSERTION. Doubling the manuscript must roughly double the
// work, not quadruple it: [I-0231]'s amplification is the per-file multiple, and
// a per-file multiple that GROWS with project size is the quadratic signature
// that [I-0196] already paid for once ("invisible at 16 scenes, fatal at 1,152").
TEST_CASE("[I-0231] AC7 — calls per file do not grow with manuscript size",
          "[integration][SP-144][I-0231][AC7]")
{
    Harness small;
    small.build(/*chapters=*/4, /*scenesPerChapter=*/4);
    const auto smallFiles = countFiles(small.projectDir.path);
    REQUIRE(small.open().ok());
    const double smallPerFile =
        static_cast<double>(small.counting.total().total()) / static_cast<double>(smallFiles);

    Harness large;
    large.build(/*chapters=*/10, /*scenesPerChapter=*/10);
    const auto largeFiles = countFiles(large.projectDir.path);
    REQUIRE(large.open().ok());
    const double largePerFile =
        static_cast<double>(large.counting.total().total()) / static_cast<double>(largeFiles);

    std::cout << "\n=== [I-0231] AC7 scaling ===\n"
              << "  small: " << smallFiles << " files, "
              << small.counting.total().total() << " calls, "
              << smallPerFile << " per file\n"
              << "  large: " << largeFiles << " files, "
              << large.counting.total().total() << " calls, "
              << largePerFile << " per file\n" << std::endl;

    INFO("small=" << smallPerFile << " large=" << largePerFile);

    // ⚠️ Not "equal": a larger project legitimately pays a little more per file
    // (deeper directory walks, more index entries). ✅ What must NOT happen is
    // the per-file cost RISING with size, which is what re-resolving per scene
    // looks like from the outside.
    CHECK(largePerFile <= smallPerFile * kScalingTolerance);
}

// ---------------------------------------------------------------------------
// SP-144 / [I-0232] AC5 — WHY THE APP MUST NOT OPEN A PROJECT TWICE
// ---------------------------------------------------------------------------
//
// ⚠️ THIS MEASURES THE CORE, AND THE CORE HAS NOT CHANGED: two `openProject`
// calls on the same root cost roughly the same each time. ✅ THAT IS THE POINT —
// it is the evidence for why the APP layer must open once and hand the result
// on, which is what [I-0232] AC5 now does.
//
// ⛔ NOTHING ABSORBS A SECOND OPEN, and this is the part that is easy to assume
// wrongly:
//   * `ProjectIndex` accelerates `openScene` (via `CoreServices::sceneLocator`)
//     — ⚠️ `ProjectOpener` NEVER CONSULTS IT.
//   * [I-0231]'s `ReadThroughCache` dies with the call that built it, by design:
//     the filesystem is authoritative (EP-027) and Scrivi does no filesystem
//     watching, so a cache that outlived the call could not know when it went
//     stale.
//
// ✅ So "open once" has to be an APP-LAYER discipline. Apple's
// `ProjectSession.loadAsync` always had it; Linux now hands `Landing.qml`'s
// envelope through to `EditorShell::load` rather than re-opening.
//
// ⚠️ IF A FUTURE CHANGE MAKES A SECOND OPEN CHEAP (a cross-call cache, say),
// THIS TEST WILL FAIL — and that is a prompt to re-read the EP-027 authority
// ruling before relaxing anything, not to delete the assertion.
TEST_CASE("[I-0232] AC5 — a second openProject is NOT free, so the app opens once",
          "[integration][SP-144][I-0232][AC5]")
{
    Harness h;
    h.build(/*chapters=*/10, /*scenesPerChapter=*/6);

    h.counting.reset();
    REQUIRE(h.open().ok());
    const auto first = h.counting.total().total();

    h.counting.reset();
    REQUIRE(h.open().ok());
    const auto second = h.counting.total().total();

    std::cout << "\n=== [I-0232] AC5 — cost of a SECOND open ===\n"
              << "  first  openProject : " << first  << " filesystem calls\n"
              << "  second openProject : " << second << " filesystem calls\n"
              << "  ⚠️ not free — which is why the app hands the envelope over\n"
              << std::endl;

    INFO("first=" << first << " second=" << second);
    CHECK(second >= first / 2);
}

// ---------------------------------------------------------------------------
// SP-144 — WHERE THE TIME ACTUALLY GOES IN A FULL APP LOAD
// ---------------------------------------------------------------------------
//
// ⚠️ THE USER'S OBSERVATION THAT FORCED THIS MEASUREMENT (2026-09-20): "none of
// the projects I've loaded lately have been large", ⚠️ **yet the load still takes
// too long.** ⛔ That falsifies "it is slow because the manuscript is big".
//
// ✅ The app's load is NOT one call. It is `openProject` followed by ONE
// `openScene` PER SCENE (`EditorShell.cpp:455`), because the Linux editor builds
// a CONTINUOUS viewport holding every scene's body. ⚠️ The rig measured the whole
// load at 371 s of which the landing open was 155 s — ✅ so the per-scene loop is
// the LARGER half, and it scales with SCENE COUNT, not project size on disk.
//
// ⚠️ THIS TEST DOES NOT ASSERT A BUDGET. It PRINTS the split, so the next fix is
// aimed by evidence rather than at whichever half was most recently discussed.
TEST_CASE("[SP-144] where a full app load spends its filesystem calls",
          "[integration][SP-144][load-shape]")
{
    Harness h;
    h.build(/*chapters=*/10, /*scenesPerChapter=*/6);

    // Phase 1 — the open itself.
    h.counting.reset();
    auto opened = h.open();
    REQUIRE(opened.ok());
    const auto openCalls  = h.counting.total().total();
    const auto sceneCount = opened.value().scenes.size();

    // Phase 2 — the per-scene body reads the editor performs next.
    h.counting.reset();
    {
        scrivi::ScriviCore core{h.services};
        for (const auto& s : opened.value().scenes) {
            scrivi::OpenSceneRequest req;
            req.projectRootPath = h.root();
            req.appSupportRoot  = h.appSupport();
            req.projectID       = scrivi::ProjectID{h.projectID};
            req.sceneID         = s.sceneID;
            (void)core.openScene(req);
        }
    }
    const auto sceneCalls = h.counting.total().total();

    // ⚠️ WHERE those per-scene calls go — the hottest paths across the whole loop.
    std::cout << "\n  -- 8 hottest paths across the openScene loop --\n";
    for (const auto& [path, c] : h.counting.hottestPaths(8)) {
        std::cout << "    " << c.total() << "x (read=" << c.readTextFile
                  << " exists=" << c.exists << " list=" << c.listDirectory << ") "
                  << path.substr(path.find("/manuscript") == std::string::npos
                                 ? 0 : path.find("/manuscript"))
                  << "\n";
    }

    std::cout << "\n=== [SP-144] full app load shape ===\n"
              << "  scenes                       : " << sceneCount << "\n"
              << "  openProject                  : " << openCalls  << " calls\n"
              << "  openScene x " << sceneCount << " (the loop)    : " << sceneCalls << " calls\n"
              << "  per scene                    : "
              << (sceneCount ? static_cast<double>(sceneCalls) / static_cast<double>(sceneCount) : 0.0)
              << " calls\n"
              << "  TOTAL                        : " << (openCalls + sceneCalls) << " calls\n"
              << "  ⚠️ the loop is "
              << (openCalls ? static_cast<double>(sceneCalls) / static_cast<double>(openCalls) : 0.0)
              << "x the open\n" << std::endl;

    REQUIRE(sceneCount > 0);
    REQUIRE(sceneCalls > 0);
}

// ---------------------------------------------------------------------------
// SP-144 — the bulk-load variant performs NO workspace write
// ---------------------------------------------------------------------------
//
// ⚠️ WHAT THIS DEFENDS. `openScene` records the opened scene as the project's
// last writing surface — correct for a scene the writer NAVIGATED TO, and the
// dominant cost of a bulk load, where it fires once per scene to record a value
// only the last of which survives. ✅ MEASURED through the shipped C ABI (61
// scenes, Linux/strace): 62 opens + 61 `.tmp` opens + 61 renames of
// `workspace-state.json`, against 3 opens of any manuscript sidecar.
//
// ⛔ THE RESTORE MUST SURVIVE THE FIX. Suppressing the write must not suppress
// the READ — a bulk load still has to return the writer's cursor and scroll, or
// the optimisation would silently cost them their place in the manuscript.
TEST_CASE("[SP-144] bulk-load openScene writes nothing, but still restores",
          "[integration][SP-144][bulk-load]")
{
    Harness h;
    h.build(/*chapters=*/4, /*scenesPerChapter=*/4);

    auto opened = h.open();
    REQUIRE(opened.ok());
    REQUIRE_FALSE(opened.value().scenes.empty());
    const auto sceneID = opened.value().scenes.front().sceneID;

    scrivi::ScriviCore core{h.services};

    auto makeRequest = [&](bool record) {
        scrivi::OpenSceneRequest req;
        req.projectRootPath = h.root();
        req.appSupportRoot  = h.appSupport();
        req.projectID       = scrivi::ProjectID{h.projectID};
        req.sceneID         = sceneID;
        req.recordAsWritingSurface = record;
        return req;
    };

    // --- the DEFAULT still writes (no existing caller changes behaviour) -----
    h.counting.reset();
    auto recorded = core.openScene(makeRequest(/*record=*/true));
    REQUIRE(recorded.ok());
    // ⚠️ Compared against the RECORDING path, so "empty" can never be blamed on
    // the flag when it is really a property of the fixture.
    const bool recordedMarkdownEmpty = recorded.value().markdown.empty();
    const auto writesWhenRecording =
        h.counting.total().atomicWrite + h.counting.total().renamePath;
    INFO("writes when recording: " << writesWhenRecording);
    CHECK(writesWhenRecording > 0);

    // --- the BULK variant writes NOTHING AT ALL ------------------------------
    h.counting.reset();
    auto bulk = core.openScene(makeRequest(/*record=*/false));
    REQUIRE(bulk.ok());

    const auto& t = h.counting.total();
    const auto writes = t.atomicWrite + t.appendTextFile + t.renamePath
                      + t.createExclusive + t.removeFile;

    std::cout << "\n=== [SP-144] bulk-load openScene ===\n"
              << "  writes when RECORDING : " << writesWhenRecording << "\n"
              << "  writes when BULK      : " << writes << "\n"
              << "  total calls when BULK : " << t.total() << "\n" << std::endl;

    // ⚠️ THE WHOLE POINT: a bulk read is a READ.
    CHECK(writes == 0);

    // ⛔ ...and the bulk variant returns EXACTLY what the recording one does.
    // ⚠️ Asserted as EQUIVALENCE, not as non-emptiness: these fixture scenes are
    // created through the core and their bodies are legitimately empty, so
    // "not empty" would be testing the fixture rather than the change.
    CHECK(bulk.value().markdown.empty() == recordedMarkdownEmpty);
    CHECK(bulk.value().scene.sceneID.value == sceneID.value);
}

// ---------------------------------------------------------------------------
// SP-144 — the SAVE path still records the writing surface
// ---------------------------------------------------------------------------
//
// ⚠️ WHAT THIS DEFENDS, AND WHY IT IS NOT OBVIOUS. [I-0234] stopped the bulk
// load stamping `lastWritingSurface` once per scene. ⛔ That is only safe because
// something ELSE still records where the writer is — `saveScene` does, on every
// save (`SceneWriter.cpp:37`), which is what both platforms' teardown paths rely
// on (Apple: `stampWritingSurfaceBlocking`, [I-0058]/[I-0131]; Linux: the
// `stampWritingSurface` added alongside this change).
//
// ⛔ IF THIS EVER STOPS BEING TRUE, RESUME BREAKS SILENTLY: the writer reopens
// their project and lands on the wrong scene, with no error anywhere. ✅ A test
// here is much cheaper than rediscovering that from a bug report.
TEST_CASE("[SP-144] saveScene records lastWritingSurface (what resume depends on)",
          "[integration][SP-144][I-0234]")
{
    Harness h;
    h.build(/*chapters=*/3, /*scenesPerChapter=*/3);

    auto opened = h.open();
    REQUIRE(opened.ok());
    REQUIRE(opened.value().scenes.size() > 1);

    // Pick a scene that is NOT the one the open already made active, so a pass
    // cannot be an artefact of openProject's own stamping.
    const auto& target = opened.value().scenes.back();
    REQUIRE(target.sceneID.value != opened.value().activeScene->sceneID.value);

    scrivi::ScriviCore core{h.services};

    scrivi::SaveSceneRequest save;
    save.projectID         = scrivi::ProjectID{h.projectID};
    save.projectRootPath   = h.root();
    save.appSupportRoot    = h.appSupport();
    save.sceneID           = target.sceneID;
    save.sceneMetadataPath = target.metadataPath;
    save.sceneContentPath  = target.contentPath;
    save.markdown          = "The writer typed this.\n";
    save.selection         = {7, 7};
    save.scroll            = {0.5};
    save.author            = {scrivi::IdentityID{"identity-001"},
                              scrivi::PersonaID{"persona-001"},
                              "Test Author"};
    REQUIRE(core.saveScene(save).ok());

    // Reopening must now resume on the SAVED scene, not the original active one.
    auto reopened = h.open();
    REQUIRE(reopened.ok());
    REQUIRE(reopened.value().activeScene.has_value());

    INFO("resumed on " << reopened.value().activeScene->sceneID.value
         << ", expected " << target.sceneID.value);
    CHECK(reopened.value().activeScene->sceneID.value == target.sceneID.value);
}

// ---------------------------------------------------------------------------
// SP-144 — a full load's WRITES must be O(1), not O(scenes)
// ---------------------------------------------------------------------------
//
// ⚠️ THIS IS THE LARGE-PROJECT GUARD, and it is about WRITES specifically.
// [I-0231]'s bound (AC2/AC7) covers reads-per-file; this covers the other axis,
// which is what actually scaled with manuscript size.
//
// ⚠️ MEASURED on the shipped ABI (Linux, `strace -c -f`) BEFORE the fix, writes
// grew exactly with scene count — 60 scenes: 62 writes; 120: 122; 240: 242 —
// because every `openScene` re-stamped `lastWritingSurface`. ✅ AFTER: 1 write at
// every size.
//
// ⚠️ On a `cache=none` network volume each eliminated write was a temp-create +
// write + rename ROUND-TRIP, so this is the axis a large project feels most.
//
// ⛔ A TOTAL-CALL BUDGET WOULD NOT CATCH THIS: reads legitimately grow with the
// manuscript, so the regression would hide inside a rising total. ✅ The
// assertion is that writes DO NOT grow with scene count.
TEST_CASE("[SP-144] a bulk load's writes do not grow with scene count",
          "[integration][SP-144][I-0234][scaling]")
{
    auto writesForFullLoad = [](int chapters, int scenesPerChapter,
                                std::size_t& sceneCountOut) {
        Harness h;
        h.build(chapters, scenesPerChapter);

        auto opened = h.open();
        REQUIRE(opened.ok());
        sceneCountOut = opened.value().scenes.size();

        // Count ONLY the per-scene loop, which is where the growth was.
        h.counting.reset();
        scrivi::ScriviCore core{h.services};
        for (const auto& s : opened.value().scenes) {
            scrivi::OpenSceneRequest req;
            req.projectRootPath = h.root();
            req.appSupportRoot  = h.appSupport();
            req.projectID       = scrivi::ProjectID{h.projectID};
            req.sceneID         = s.sceneID;
            req.recordAsWritingSurface = false;   // the bulk-load path
            (void)core.openScene(req);
        }
        const auto& t = h.counting.total();
        return t.atomicWrite + t.appendTextFile + t.renamePath
             + t.createExclusive + t.removeFile;
    };

    std::size_t smallScenes = 0, largeScenes = 0;
    const auto smallWrites = writesForFullLoad(3, 3, smallScenes);
    const auto largeWrites = writesForFullLoad(10, 8, largeScenes);

    std::cout << "\n=== [SP-144] write scaling across a bulk load ===\n"
              << "  " << smallScenes << " scenes → " << smallWrites << " writes\n"
              << "  " << largeScenes << " scenes → " << largeWrites << " writes\n"
              << "  ⚠️ before the fix this was ~1 write PER SCENE\n" << std::endl;

    REQUIRE(largeScenes > smallScenes * 2);   // the sizes really do differ

    // ✅ THE ASSERTION: writes are independent of manuscript size.
    INFO("small=" << smallWrites << " (" << smallScenes << " scenes), "
         << "large=" << largeWrites << " (" << largeScenes << " scenes)");
    CHECK(largeWrites == smallWrites);
}
