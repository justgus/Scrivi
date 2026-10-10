// SP-173 Plan 6 / I-0285 AC3 — a project open must not rebuild and rewrite the object index once per relationship
// endpoint. ⛔ Measured on the rig (dumas, its world unavailable over the share): the open's relationship repair rebuilt AND
// rewrote `objects/index.json` 270 times — 24 s of a 57 s open. Fixture through the C ABI (worlds and edges are not on the
// facade); the open through the facade with a CountingFileSystem, which attributes every call to its path.

#include "mocks/CountingFileSystem.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "scrivi/ScriviCore.hpp"
#include "scrivi/ObjectTypes.hpp"
#include "scrivi/scrivi.h"
#include "platform/LocalFileSystem.hpp"
#include "util/Json.hpp"
#include "util/ReadThroughCache.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace {

namespace fs = std::filesystem;
using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

JsonDoc okResult(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    INFO("envelope: " << parsed.value().dump());
    REQUIRE(parsed.value().getBool("ok"));
    return parsed.value().getSubDoc("result");
}

struct Dirs {
    fs::path project, support, world;
    Dirs() {
        static std::atomic<int> n{0};
        const std::string stem = "scrivi-index-rebuild-" + std::to_string(n.fetch_add(1)) + "-" +
                                 std::to_string(reinterpret_cast<std::uintptr_t>(this));
        project = fs::temp_directory_path() / (stem + ".scrivi");
        support = fs::temp_directory_path() / (stem + "-support");
        world   = fs::temp_directory_path() / (stem + "-World.scrivworld");
        fs::create_directories(project);
        fs::create_directories(support);
    }
    ~Dirs() {
        std::error_code ec;
        fs::remove_all(project, ec);
        fs::remove_all(support, ec);
        fs::remove_all(world, ec);
        fs::remove_all(fs::path(world.string() + ".away"), ec);
    }
};

}  // namespace

TEST_CASE("[SP-173] an open with many edges into an UNAVAILABLE world does not rewrite the object index",
          "[sp173][index]") {
    Dirs d;
    const char* root = d.project.c_str();
    okResult(scrivi_create_project(root, d.support.c_str(), "Rebuild", "rebuild", "identity-001", "persona-001", "Author"));
    const JsonDoc opened = okResult(scrivi_open_project(root, d.support.c_str(), "identity-001"));
    const std::string sceneID = opened.getSubDoc("activeScene").getString("sceneID");
    REQUIRE_FALSE(sceneID.empty());

    const std::string worldID = okResult(scrivi_create_world(root, d.world.c_str(), "World", "Year One")).getString("worldID");
    constexpr int kEdges = 20;
    for (int i = 0; i < kEdges; ++i) {
        const std::string name = "Character " + std::to_string(i);
        const std::string id = okResult(scrivi_create_object(root, "character", name.c_str(), "", "identity-001",
                                                             "persona-001", "Author", worldID.c_str()))
                                   .getString("objectID");
        okResult(scrivi_create_edge(root, id.c_str(), sceneID.c_str(), "appears-in", ""));
    }
    // Project-scoped objects too (dumas has them), so a rebuild has a kind directory to LIST — otherwise a rebuild costs
    // almost nothing here and repeating it could not be seen.
    for (int i = 0; i < 3; ++i) {
        const std::string name = "Source " + std::to_string(i);
        okResult(scrivi_create_object(root, "source", name.c_str(), "", "identity-001", "persona-001", "Author", ""));
    }
    // The world goes away (a drive not attached; a project opened on another machine).
    fs::rename(d.world, fs::path(d.world.string() + ".away"));

    scrivi::platform::LocalFileSystem        real;
    scrivi::testing::CountingFileSystem      counting{real};
    scrivi::mocks::FixedClock                clock{"2026-10-10T00:00:00Z"};
    scrivi::mocks::DeterministicUUIDProvider uuid;
    scrivi::mocks::MockSecureStore           secure;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices services;
    services.fileSystem = &counting; services.clock = &clock; services.uuidProvider = &uuid;
    services.secureStore = &secure;  services.gitProvider = &git;
    scrivi::ScriviCore core{services};

    scrivi::OpenProjectRequest req;
    req.projectRootPath = d.project.string();
    req.appSupportRoot  = d.support.string();
    REQUIRE(core.openProject(req).ok());

    const std::string indexPath = (d.project / "objects" / "index.json").string();
    const std::string objectsDir = (d.project / "objects").string();
    std::size_t indexWrites = 0, objectScans = 0;
    for (const auto& [path, c] : counting.byPath()) {
        if (path == indexPath) { indexWrites += c.atomicWrite; }
        // A rebuild's scan probes each kind's directory under objects/ (exists / isDirectory) and lists the ones present.
        if (path.rfind(objectsDir + "/", 0) == 0 && path != indexPath) { objectScans += c.listDirectory + c.probes(); }
    }
    // One scan's worth, measured: what a single rebuild costs here (one probe per storable kind directory).
    const std::size_t oneScan = std::size(scrivi::kAllStorableKinds) * 2;
    INFO("index writes " << indexWrites << ", scan calls under objects/ " << objectScans << " (one scan ≈ " << oneScan << ")");
    CHECK(indexWrites == 0);                  // nothing changed on disk: nothing to write
    CHECK(objectScans <= oneScan);            // repeat rebuilds are served by the open's read-through cache
}

TEST_CASE("[SP-173] the open checks scene files against one chapter listing, not one probe per file", "[sp173][open]") {
    // ⛔ Measured on the rig (dumas over the share): ProjectValidator stat'ed every scene text file — 1,186 network round
    // trips, ~7–9 s of the open. One listing per chapter answers the same question. Scenes made through the C ABI, so each
    // is REGISTERED in its chapter (the validator checks registered scenes).
    Dirs d;
    const char* root = d.project.c_str();
    okResult(scrivi_create_project(root, d.support.c_str(), "Probe", "probe", "identity-001", "persona-001", "Author"));
    const JsonDoc opened = okResult(scrivi_open_project(root, d.support.c_str(), "identity-001"));
    const std::string projectID = opened.getString("projectID");
    const std::string chapterID = opened.arrayItem("scenes", 0).getString("chapterID");
    std::string after = opened.getSubDoc("activeScene").getString("sceneID");
    for (int i = 1; i < 30; ++i) {
        after = okResult(scrivi_create_scene(root, d.support.c_str(), projectID.c_str(), chapterID.c_str(), after.c_str(),
                                             "", "identity-001", "persona-001", "Author")).getString("sceneID");
    }

    scrivi::platform::LocalFileSystem        real;
    scrivi::testing::CountingFileSystem      counting{real};
    scrivi::mocks::FixedClock                clock{"2026-10-10T00:00:00Z"};
    scrivi::mocks::DeterministicUUIDProvider uuid;
    scrivi::mocks::MockSecureStore           secure;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices services;
    services.fileSystem = &counting; services.clock = &clock; services.uuidProvider = &uuid;
    services.secureStore = &secure;  services.gitProvider = &git;
    scrivi::ScriviCore core{services};
    scrivi::OpenProjectRequest req;
    req.projectRootPath = d.project.string();
    req.appSupportRoot  = d.support.string();
    REQUIRE(core.openProject(req).ok());

    std::size_t sceneTextProbes = 0, sceneTexts = 0;
    for (const auto& [path, c] : counting.byPath()) {
        if (path.size() > 3 && path.compare(path.size() - 3, 3, ".md") == 0 && path.find("/manuscript/") != std::string::npos) {
            ++sceneTexts;
            sceneTextProbes += c.probes();
        }
    }
    INFO("scene text files touched: " << sceneTexts << ", existence probes of them: " << sceneTextProbes);
    CHECK(sceneTextProbes == 0);
}

TEST_CASE("[SP-173] ReadThroughCache::prefetch: prefetched files are served from memory; failures are not stored; a write "
          "still invalidates",
          "[sp173][prefetch]") {
    Dirs d;
    std::vector<std::string> paths;
    for (int i = 0; i < 50; ++i) {
        const auto p = (d.project / ("f" + std::to_string(i) + ".meta.json")).string();
        std::ofstream(p, std::ios::binary) << "content " << i;
        paths.push_back(p);
    }
    const std::string missing = (d.project / "missing.meta.json").string();
    paths.push_back(missing);

    scrivi::platform::LocalFileSystem   real;
    scrivi::testing::CountingFileSystem counting{real};
    scrivi::util::ReadThroughCache      cache{counting};
    cache.prefetch(paths, real, 8);                      // read in parallel through the thread-safe reader

    for (int i = 0; i < 50; ++i) {
        auto r = cache.readTextFile(paths[i]);
        REQUIRE(r.ok());
        CHECK(r.value() == "content " + std::to_string(i));
    }
    CHECK(counting.total().readTextFile == 0);           // every prefetched file came from memory

    CHECK_FALSE(cache.readTextFile(missing).ok());       // a failed prefetch was not stored as an answer…
    CHECK(counting.total().readTextFile == 1);           // …so the read went to disk

    REQUIRE(cache.atomicWriteTextFile(paths[0], "changed").ok());
    CHECK(cache.readTextFile(paths[0]).value() == "changed");   // a write after a prefetch still invalidates
}
