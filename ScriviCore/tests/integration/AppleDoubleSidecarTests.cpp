// AppleDoubleSidecarTests.cpp
// Integration tests for I-0221: a project stored on a volume without native
// extended-attribute support must open normally.
//
// ⚠️ WHY THIS TEST EXISTS AND WHY IT IS AN INTEGRATION TEST. The defect was a
// project on a FAT32 USB volume failing to open with a JSON parse error naming no
// file. macOS stores xattrs on such volumes in "AppleDouble" sidecars named
// `._<original>`, so `001-scene.meta.json` acquired a binary companion
// `._001-scene.meta.json` -- which carries the `.meta.json` suffix, yields a
// non-empty order key, and then fails to parse. `listScenesByOrder` returned
// `failure` rather than skipping it, so ONE stray file aborted a 1,224-scene open.
//
// ⚠️ THE UNIT TESTS FOR THE PREDICATE CANNOT CATCH THIS. The bug was never in
// "does this name look like an artifact" -- it was in a scan that never asked. Only
// a real directory containing a real sidecar exercises the path that broke.
//
// ⚠️ AND THE EXISTING SUITE STRUCTURALLY COULD NOT CATCH IT: every fixture builds
// under `temp_directory_path()`, which on macOS is APFS, where the OS never creates
// these files. So the sidecars here are written EXPLICITLY -- the test does not
// depend on the host filesystem behaving like FAT.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

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

namespace {

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-appledouble-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
};

scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem&        lfs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock&                clock,
    scrivi::mocks::MockSecureStore&           store,
    scrivi::mocks::MockGitProvider&           git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    return s;
}

scrivi::AuthorshipRef testAuthor()
{
    return {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
}

// Writes a byte-accurate AppleDouble header. ⚠️ The magic `0x00051607` and the
// "Mac OS X" filler are what macOS actually writes -- copied from a real sidecar
// captured on the FAT32 volume that produced this defect, so the bytes the parser
// meets here are the bytes it met on the rig.
void writeAppleDoubleSidecar(const fs::path& realFile)
{
    const fs::path sidecar = realFile.parent_path() / ("._" + realFile.filename().string());
    std::ofstream out(sidecar, std::ios::binary);
    static const unsigned char kHeader[] = {
        0x00, 0x05, 0x16, 0x07, 0x00, 0x02, 0x00, 0x00,
        'M','a','c',' ','O','S',' ','X',' ',' ',' ',' ',' ',' ',' ',' ',
        0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00
    };
    out.write(reinterpret_cast<const char*>(kHeader), sizeof(kHeader));
    // Real sidecars are padded to 4096 bytes.
    const std::string padding(4096 - sizeof(kHeader), '\0');
    out.write(padding.data(), static_cast<std::streamsize>(padding.size()));
}

// Every `*.meta.json` and `*.md` under `manuscript/` gains a sidecar, which is what
// a FAT/exFAT/SMB volume produces once the app has written the package.
int litterManuscriptWithSidecars(const fs::path& projectRoot)
{
    int written = 0;
    const fs::path manuscript = projectRoot / "manuscript";
    REQUIRE(fs::exists(manuscript));

    std::vector<fs::path> realFiles;
    for (const auto& entry : fs::recursive_directory_iterator(manuscript)) {
        if (!entry.is_regular_file()) { continue; }
        const std::string name = entry.path().filename().string();
        if (name.rfind("._", 0) == 0) { continue; }
        realFiles.push_back(entry.path());
    }
    for (const auto& f : realFiles) {
        writeAppleDoubleSidecar(f);
        ++written;
    }
    return written;
}

} // namespace

TEST_CASE("openProject succeeds when AppleDouble sidecars litter the manuscript",
          "[integration][I-0221]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-09-17T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Sidecar Novel";
    req.slug            = "sidecar-novel";
    req.author          = testAuthor();

    auto created = core.createProject(req);
    REQUIRE(created.ok());

    // A clean open must work first -- otherwise a later failure proves nothing
    // about sidecars.
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.currentIdentityID = scrivi::IdentityID{"identity-001"};

    auto before = core.openProject(openReq);
    REQUIRE(before.ok());
    const std::size_t scenesBefore = before.value().scenes.size();
    REQUIRE(scenesBefore > 0);

    // Now make the package look like it has lived on a FAT volume.
    const int sidecars = litterManuscriptWithSidecars(fs::path(projectDir.str()));
    REQUIRE(sidecars > 0);

    auto after = core.openProject(openReq);

    // ⚠️ THE REGRESSION. Before the fix this returned `failure` with
    // "[json.exception.parse_error.101] ... attempting to parse an empty input".
    INFO("openProject must not fail on AppleDouble sidecars; wrote " << sidecars);
    REQUIRE(after.ok());

    // ⚠️ AND THE SCENE COUNT MUST BE UNCHANGED. Opening without error is not
    // enough: a filter that swallowed real scenes alongside the sidecars would
    // also "pass" the check above while silently emptying the manuscript.
    REQUIRE(after.value().scenes.size() == scenesBefore);
}

TEST_CASE("a single AppleDouble sidecar does not abort the open",
          "[integration][I-0221]")
{
    // ⚠️ THE MINIMAL CASE, and the one that matches how the defect actually
    // recurred: the app writes ONE file, the OS stamps it with a quarantine xattr,
    // one sidecar appears, and the NEXT open fails. A cleaned project re-broke
    // after a single world relink.
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-09-17T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "One Sidecar";
    req.slug            = "one-sidecar";
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.currentIdentityID = scrivi::IdentityID{"identity-001"};

    auto before = core.openProject(openReq);
    REQUIRE(before.ok());
    const std::size_t scenesBefore = before.value().scenes.size();

    // Find one real scene metadata file and give it a single companion.
    fs::path victim;
    for (const auto& entry :
         fs::recursive_directory_iterator(fs::path(projectDir.str()) / "manuscript")) {
        if (!entry.is_regular_file()) { continue; }
        const std::string name = entry.path().filename().string();
        if (name.rfind("._", 0) == 0) { continue; }
        if (name.size() > 10 && name.substr(name.size() - 10) == ".meta.json"
            && name != "chapter.meta.json") {
            victim = entry.path();
            break;
        }
    }
    REQUIRE_FALSE(victim.empty());
    writeAppleDoubleSidecar(victim);

    auto after = core.openProject(openReq);
    INFO("one sidecar beside " << victim.filename().string() << " must not abort the open");
    REQUIRE(after.ok());
    REQUIRE(after.value().scenes.size() == scenesBefore);
}
