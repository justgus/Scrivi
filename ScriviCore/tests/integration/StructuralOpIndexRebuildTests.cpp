// StructuralOpIndexRebuildTests.cpp
//
// [I-0213] round 2 — WHAT DOES A CHAPTER CREATE COST THE *CORE*?
//
// ⚠️ WORK ITEMS REFERENCED HERE, IN ONE LINE EACH — so this file can be read
// without opening the tracking docs:
//
//   [I-0213]  `[Apple]` Creating a chapter froze the app for ~2.7 s on a
//             1,174-scene manuscript. VERIFIED 2026-09-24 at `396.7 ms` of work
//             and ACCEPTED as a limitation; this file measures what remains.
//   [I-0231]  `[ScriviCore]` Project open re-read every sidecar ~96x (55,574
//             reads / 8,044 opens on a 150-file project). Invisible on local
//             disk (`0.21 s`), `154 s` on a `cache=none` mount. Located by
//             MEASUREMENT, which is the method this file reuses.
//   [I-0204]  `[Apple]` `TimelineStripView.buildClusters` burned the main thread
//             ~33 s per layout pass. Found by `sample`, AFTER three wrong
//             code-read diagnoses — the cited warning against inferring.
//   [EP-039]  `[Cross]` Project Load Performance. Goal: a project of ordinary
//             size opens without freezing, and the core stops answering
//             per-item questions with full-tree traversals. It replaced a
//             ~300 s frozen open.
//     └ AC4   `scrivi_list_story_times` — the SPARSE bulk call: ONE crossing
//             returning a record only for scenes whose story time is EXPLICITLY
//             set, replacing one C ABI call PER SCENE (`234-251 s` of that open
//             was spent discovering that nothing was set).
//     └ AC5b  The index-correctness clause: a failed index update DROPS THE
//             WHOLE INDEX, and the next query rebuilds and returns the CORRECT
//             answer. This is the guard that must survive any fix below.
//   [EP-040]  `[Apple]` The Editor Shell — build Scrivi as the kind of app it
//             actually is.
//     └ AC11  The carried performance clause: the manuscript surface's
//             remaining O(DOCUMENT) costs are addressed OR accepted as
//             limitations WITH a measurement. This file supplies the
//             measurement AC11 asked for.
//
// ⚠️ THE APPLE-SIDE STORY IS ALREADY MEASURED AND IS NOT REPEATED HERE.
// `createChapter WORK` fell 880 -> ~305 ms on internal storage (2026-09-15) and
// read `396.7 ms` steady-state on a USB mount (2026-09-24, the user's deliberate
// worst-case rig — chosen so the figure is a FLOOR, not a typical case).
// ✅ `reloadSceneDots` is ~two-thirds of what remains, and [EP-040] AC11 (the
// carried performance clause) asked for that remainder to be INSTRUMENTED rather
// than inferred.
//
// ⛔ THIS ISSUE HAS A DOCUMENTED HISTORY OF CODE-READ DIAGNOSES BEING WRONG:
// three of them, before `sample` found [I-0204] (the ~33 s clustering stall);
// and the "scene create costs 2.7 s" framing was wrong in BOTH the op and the
// attribution — it was CHAPTER create, and the keystroke was a passenger. So the question
// below is asked with an INSTRUMENT, not with an argument.
//
// ✅ THE QUESTION: `scrivi_list_story_times` ([EP-039] AC4 — the sparse bulk
// call) is served from a cached `ProjectIndex`. A structural op invalidates that
// index ON PURPOSE (`ProjectIndexInvalidation` — it fixed a REAL correctness
// defect, [EP-039] AC5b, where a write landed on disk and the next read reported
// `count:0`). ⚠️ So the FIRST `listStoryTimes` after any structural op pays a
// FULL MANUSCRIPT REBUILD.
//
// ⚠️ WALL CLOCK WOULD LIE HERE, exactly as it does in
// `OpenProjectReadAmplificationTests` ([I-0231]): on a warm local page cache the
// rebuild is cheap, while on the user's USB rig every read is real. ✅ So this measures
// FILESYSTEM CALL COUNT, which is the quantity that actually scales with the
// medium — and the medium is the whole point of the 2026-09-24 verification.

#include "mocks/CountingFileSystem.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "manuscript/ProjectIndex.hpp"

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

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-i0213-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    TempDir(const TempDir&)            = delete;
    TempDir& operator=(const TempDir&) = delete;
    [[nodiscard]] std::string str() const { return path.string(); }
};

// Built through the REAL core so the on-disk shape is the shipped one.
struct Harness {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        real;
    scrivi::testing::CountingFileSystem      counting{real};
    scrivi::mocks::FixedClock                clock{"2026-09-24T00:00:00Z"};
    scrivi::mocks::DeterministicUUIDProvider uuid;
    scrivi::mocks::MockSecureStore           secure;
    scrivi::mocks::MockGitProvider           git;

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

    scrivi::AuthorshipRef author() const {
        return scrivi::AuthorshipRef{scrivi::IdentityID{"identity-001"},
                                     scrivi::PersonaID{"persona-001"},
                                     "Test Author"};
    }

    void build(int chapters, int scenesPerChapter) {
        scrivi::ScriviCore core{services};

        scrivi::CreateProjectRequest req;
        req.projectRootPath = root();
        req.appSupportRoot  = appSupport();
        req.title           = "Index Rebuild Novel";
        req.slug            = "index-rebuild-novel";
        req.author          = author();

        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID.value;

        for (int c = 2; c <= chapters; ++c) {
            scrivi::CreateChapterRequest cr;
            cr.projectRootPath = root();
            cr.appSupportRoot  = appSupport();
            cr.projectID       = scrivi::ProjectID{projectID};
            cr.author          = author();
            REQUIRE(core.createChapter(cr).ok());
        }

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
};

std::size_t countScenes(const fs::path& root) {
    std::size_t n = 0;
    for (const auto& e : fs::recursive_directory_iterator(root)) {
        if (e.is_regular_file() &&
            e.path().filename().string().find("-scene.meta.json") != std::string::npos) { ++n; }
    }
    return n;
}

void report(const scrivi::testing::CountingFileSystem& cfs,
            const std::string& label, std::size_t scenes) {
    const auto& t = cfs.total();
    std::cout << "\n=== [I-0213] " << label << " ===\n"
              << "  scenes in manuscript   : " << scenes << "\n"
              << "  TOTAL filesystem calls : " << t.total() << "\n"
              << "    readTextFile         : " << t.readTextFile << "\n"
              << "    listDirectory        : " << t.listDirectory << "\n"
              << "    exists/isDir/devID   : " << t.probes() << "\n";
    if (scenes > 0) {
        std::cout << "  reads per scene        : "
                  << (static_cast<double>(t.reads()) / static_cast<double>(scenes)) << "\n";
    }
    std::cout << std::endl;
}

}  // namespace

// ---------------------------------------------------------------------------
// AC11 — what the FIRST listStoryTimes after a structural op actually costs.
// ---------------------------------------------------------------------------
TEST_CASE("[I-0213] one index build walks the whole manuscript; warm reads are free", "[i0213][perf]") {
    Harness h;
    h.build(/*chapters=*/20, /*scenesPerChapter=*/20);
    const auto scenes = countScenes(h.projectDir.path);

    // ⚠️ `listStoryTimes` is exposed ONLY at the C ABI — there is no facade
    // method — so the rebuild is driven through `ProjectIndex::build`, which is
    // exactly what `withProjectIndex` calls on a cache miss. ✅ Measuring the
    // build itself is the tighter measurement anyway: it excludes envelope
    // serialisation, which is not the suspected cost.
    auto svc = h.services;
    svc.sceneLocator = nullptr;   // as the ABI does — no recursion via the locator

    h.counting.reset();
    auto built = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(built.ok());
    report(h.counting, "ProjectIndex::build — ONE full manuscript rebuild", scenes);

    // ✅ The index, once built, answers story times with NO filesystem work.
    h.counting.reset();
    const auto explicitSet = built.value().explicitStoryTimes();
    report(h.counting, "explicitStoryTimes — served from the WARM index", scenes);
    CHECK(h.counting.total().total() == 0);
    CHECK(explicitSet.empty());   // default chain: nothing explicitly set
}

TEST_CASE("[I-0213] a structural op forces a FULL manuscript rebuild on the next read",
          "[i0213][perf]") {
    Harness h;
    h.build(/*chapters=*/20, /*scenesPerChapter=*/20);
    const auto scenes = countScenes(h.projectDir.path);

    scrivi::ScriviCore core{h.services};

    // The structural op the user actually performs.
    scrivi::CreateChapterRequest cr;
    cr.projectRootPath = h.root();
    cr.appSupportRoot  = h.appSupport();
    cr.projectID       = scrivi::ProjectID{h.projectID};
    cr.author          = h.author();

    h.counting.reset();
    REQUIRE(core.createChapter(cr).ok());
    const auto createCalls = h.counting.total().total();
    const auto createReads = h.counting.total().reads();
    report(h.counting, "createChapter — the op itself", scenes);

    // ⚠️ THE POINT: the op invalidates the index, so the NEXT story-time read
    // re-walks the WHOLE manuscript. That rebuild is the cost the timeline pays
    // inside `reloadSceneDots`, and it is what AC11 asked to see measured.
    auto svc = h.services;
    svc.sceneLocator = nullptr;

    h.counting.reset();
    auto rebuilt = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(rebuilt.ok());
    report(h.counting, "ProjectIndex::build — FORCED by the structural op", scenes);

    std::cout << "  [I-0213] createChapter itself : " << createCalls
              << " calls (" << createReads << " reads)\n"
              << "  [I-0213] forced rebuild       : " << h.counting.total().total()
              << " calls (" << h.counting.total().reads() << " reads)\n" << std::endl;

    // ⚠️ NOT an assertion of a target: this REPORTS the rebuild so AC11 has a
    // measured number instead of an inference. A bound is set in the round that
    // fixes it, FROM this figure — never invented in advance.
    CHECK(h.counting.total().reads() > 0);
}

// ---------------------------------------------------------------------------
// T-0549 (SP-150) — AC1, AC4: Class A patches the index instead of dropping it.
//
// ⚠️ MEASURED THROUGH THE INDEX LAYER, and AC3 is proven separately through the
// C ABI by EP-039's own AC5b test ("a story-time write through the core
// invalidates the index"), which asserts through `scrivi_list_story_times` —
// an INDEX-SERVED endpoint — rather than through `openScene`, whose
// validate-on-use fallback would repair staleness and hide the defect.
// ---------------------------------------------------------------------------
TEST_CASE("[T-0549] patchStoryTime updates one entry with ZERO filesystem calls",
          "[i0213][t0549][perf]") {
    Harness h;
    h.build(/*chapters=*/20, /*scenesPerChapter=*/20);

    auto svc = h.services;
    svc.sceneLocator = nullptr;
    auto built = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(built.ok());
    auto& index = built.value();

    // A scene that really is in this manuscript.
    const auto& anyScene = index.scenesInOrder().front();
    const auto  sceneID  = anyScene.sceneID;

    // ✅ Nothing is explicitly set on a freshly built default-chain project.
    REQUIRE(index.explicitStoryTimes().empty());

    scrivi::manuscript::SceneStoryTime st;
    st.offsetMs       = 123456;
    st.offsetSource   = "manual";      // ⚠️ this is what makes it "explicitly set"
    st.durationMs     = 3600000;
    st.durationSource = "manual";

    h.counting.reset();
    const bool patched = index.patchStoryTime(sceneID, st);
    const auto callsDuringPatch = h.counting.total().total();

    CHECK(patched);
    // ✅ AC1: the patch itself touches the filesystem not at all.
    CHECK(callsDuringPatch == 0);

    // ✅ And the index now ANSWERS with the written value — a patch that did not
    // change the answer would be worse than useless.
    const auto explicitSet = index.explicitStoryTimes();
    REQUIRE(explicitSet.size() == 1);
    CHECK(explicitSet.front().first.value == sceneID.value);
    CHECK(explicitSet.front().second.offsetMs == 123456);

    std::cout << "\n=== [T-0549] patch vs rebuild ===\n"
              << "  patchStoryTime          : " << callsDuringPatch << " filesystem calls\n"
              << "  (a forced rebuild costs : 866 on this fixture)\n" << std::endl;
}

TEST_CASE("[T-0549] an unknown scene is REFUSED, so the caller drops the index",
          "[i0213][t0549]") {
    Harness h;
    h.build(/*chapters=*/5, /*scenesPerChapter=*/5);

    auto svc = h.services;
    svc.sceneLocator = nullptr;
    auto built = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(built.ok());

    scrivi::manuscript::SceneStoryTime st;
    st.offsetSource = "manual";

    // ⚠️ AC4 — THE FALLBACK IS EXERCISED, NOT MERELY WRITTEN. A sceneID the index
    // does not know means the index is ALREADY wrong about something, and that is
    // not patchable. ✅ Refusing here is what makes the ABI guard drop the whole
    // index instead of cancelling its destructor.
    CHECK_FALSE(built.value().patchStoryTime(scrivi::SceneID{"scene-does-not-exist"}, st));

    // ⛔ And the refusal changed NOTHING — a failed patch must not half-apply.
    CHECK(built.value().explicitStoryTimes().empty());
}

TEST_CASE("[T-0549] an INVALID index refuses to be patched", "[i0213][t0549]") {
    // ⚠️ A default-constructed index is invalid: it describes nothing, so there is
    // no entry to correct. ✅ The caller drops it and the next query rebuilds.
    scrivi::manuscript::ProjectIndex empty;
    scrivi::manuscript::SceneStoryTime st;
    st.offsetSource = "manual";
    CHECK_FALSE(empty.patchStoryTime(scrivi::SceneID{"scene-anything"}, st));
}

// ---------------------------------------------------------------------------
// T-0550 (SP-150) — AC2, AC4: Class B rebuilds ONLY the affected chapter.
// ---------------------------------------------------------------------------
TEST_CASE("[T-0550] a per-chapter rebuild costs a FRACTION of a full rebuild",
          "[i0213][t0550][perf]") {
    Harness h;
    h.build(/*chapters=*/20, /*scenesPerChapter=*/20);
    const auto scenes = countScenes(h.projectDir.path);

    auto svc = h.services;
    svc.sceneLocator = nullptr;

    // Baseline: what a FULL rebuild costs on this fixture.
    h.counting.reset();
    auto full = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(full.ok());
    const auto fullCalls = h.counting.total().total();
    report(h.counting, "ProjectIndex::build — FULL rebuild (baseline)", scenes);

    // The same index, updated for ONE chapter.
    const auto someChapter = full.value().scenesInOrder().front().chapterID;

    h.counting.reset();
    const bool ok = full.value().rebuildChapters(
        scrivi::AbsolutePath{h.root()}, svc, {someChapter});
    const auto partialCalls = h.counting.total().total();
    REQUIRE(ok);
    report(h.counting, "rebuildChapters — ONE chapter", scenes);

    std::cout << "  [T-0550] full rebuild   : " << fullCalls << " calls\n"
              << "  [T-0550] one chapter    : " << partialCalls << " calls\n"
              << "  [T-0550] ratio          : "
              << (static_cast<double>(fullCalls) / static_cast<double>(partialCalls))
              << "x cheaper\n" << std::endl;

    // ⚠️ AC2 — THE BOUND IS SET FROM THIS MEASUREMENT, NOT INVENTED IN ADVANCE.
    // ✅ On 20 chapters the partial walk re-lists all chapters (cheap) and re-reads
    // ONE chapter's scenes, so it must be well under half the full cost.
    CHECK(partialCalls < fullCalls / 2);

    // ✅ And the result is still a CORRECT index — cheapness that loses scenes is
    // not a win.
    CHECK(full.value().sceneCount() == scenes);
}

TEST_CASE("[T-0550] a per-chapter rebuild keeps manuscript order and ordinals honest",
          "[i0213][t0550]") {
    Harness h;
    h.build(/*chapters=*/6, /*scenesPerChapter=*/5);

    auto svc = h.services;
    svc.sceneLocator = nullptr;
    auto built = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(built.ok());

    // Capture the full-build answer, which is the reference.
    std::vector<std::string> beforeOrder;
    for (const auto& loc : built.value().scenesInOrder()) {
        beforeOrder.push_back(loc.sceneID.value);
    }

    const auto midChapter = built.value().scenesInOrder()[12].chapterID;
    REQUIRE(built.value().rebuildChapters(
        scrivi::AbsolutePath{h.root()}, svc, {midChapter}));

    // ✅ Nothing changed on disk, so a partial rebuild must reproduce the SAME order.
    std::vector<std::string> afterOrder;
    for (const auto& loc : built.value().scenesInOrder()) {
        afterOrder.push_back(loc.sceneID.value);
    }
    CHECK(afterOrder == beforeOrder);

    // ⚠️ ORDINALS MUST MATCH POSITION — this is the requirement a partial rebuild is
    // most likely to get wrong, because it splices rather than walking from 0.
    const auto& scenesInOrder = built.value().scenesInOrder();
    for (std::size_t i = 0; i < scenesInOrder.size(); ++i) {
        REQUIRE(scenesInOrder[i].ordinal == i);
    }

    // ✅ And the id→ordinal map agrees with the vector.
    for (const auto& loc : scenesInOrder) {
        const auto found = built.value().findScene(loc.sceneID);
        REQUIRE(found.has_value());
        CHECK(found->ordinal == loc.ordinal);
    }
}

TEST_CASE("[T-0550] an INVALID index and an EMPTY change-set behave correctly",
          "[i0213][t0550]") {
    Harness h;
    h.build(/*chapters=*/3, /*scenesPerChapter=*/3);
    auto svc = h.services;
    svc.sceneLocator = nullptr;

    // ⚠️ AC4 — an invalid index is NOT patchable: the caller must drop it.
    scrivi::manuscript::ProjectIndex empty;
    CHECK_FALSE(empty.rebuildChapters(scrivi::AbsolutePath{h.root()}, svc,
                                      {scrivi::ChapterID{"chapter-anything"}}));

    // ✅ An EMPTY change-set means "nothing changed", which is trivially true — but
    // the ABI guard never calls it that way (it drops on an empty set, because an
    // unknown scope is not the same as no change).
    auto built = scrivi::manuscript::ProjectIndex::build(
        scrivi::AbsolutePath{h.root()}, svc);
    REQUIRE(built.ok());
    const auto before = built.value().sceneCount();
    CHECK(built.value().rebuildChapters(scrivi::AbsolutePath{h.root()}, svc, {}));
    CHECK(built.value().sceneCount() == before);
}
