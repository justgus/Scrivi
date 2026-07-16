// ChapterSlugCollisionTests.cpp
// Regression for I-0072 (EP-027 P2): ChapterCreator must name new chapter folders with
// collision-free ORDER-KEY slugs, not `chapter-<count+1>`. The old scheme reused a slug
// after a delete left a gap, clobbering a live chapter's sidecar and duplicating the
// index. This drives the exact scenario through the public API and asserts no collision.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-slug-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};

scrivi::AuthorshipRef author() {
    return { scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Tester" };
}

} // namespace

TEST_CASE("ChapterCreator uses collision-free order-key slugs after a delete (I-0072)",
          "[Chapter][slug][EP-027][I-0072]") {
    TempDir projectDir, appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    scrivi::CoreServices services;
    services.fileSystem   = &lfs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;
    scrivi::ScriviCore core(services);

    scrivi::CreateProjectRequest preq;
    preq.projectRootPath = projectDir.str();
    preq.appSupportRoot  = appSupportDir.str();
    preq.title = "Novel"; preq.slug = "novel"; preq.author = author();
    auto created = core.createProject(preq);
    REQUIRE(created.ok());

    auto makeChapter = [&]() {
        scrivi::CreateChapterRequest r;
        r.projectRootPath = projectDir.str();
        r.appSupportRoot  = appSupportDir.str();
        r.projectID       = created.value().project.projectID;
        r.author          = author();
        auto res = core.createChapter(r);
        REQUIRE(res.ok());
        return res.value();
    };

    // Create three more chapters (project already has the initial one → 4 total).
    auto c2 = makeChapter();
    auto c3 = makeChapter();
    auto c4 = makeChapter();

    // Delete the middle one, leaving a "gap" — the exact condition that made count+1
    // collide under the old scheme.
    scrivi::DeleteChapterRequest del;
    del.projectRootPath = projectDir.str();
    del.chapterID       = c3.chapterID;
    REQUIRE(core.deleteChapter(del).ok());

    // Now create another chapter. Under the old `count+1` scheme this would compute a slug
    // that collides with an existing folder and clobber it. Under order-keys it must be
    // brand new.
    auto c5 = makeChapter();

    // Gather all chapter folders on disk and assert they are all distinct, and that the
    // new chapter's folder is not one of the previously-existing ones.
    auto listR = scrivi::manuscript::listChaptersByOrder(lfs, projectDir.str());
    REQUIRE(listR.ok());
    const auto& chapters = listR.value();

    std::set<std::string> slugs;
    std::set<std::string> ids;
    for (const auto& e : chapters) {
        // Distinct folder names (no collision) and distinct ids (no clobbered sidecar).
        REQUIRE(slugs.insert(e.slug).second);
        REQUIRE(ids.insert(e.chapterID.value).second);
    }

    // 4 survivors: initial, c2, c4, c5 (c3 deleted). No phantom/duplicate.
    REQUIRE(chapters.size() == 4);

    // The new chapter c5 exists on disk with its own unique folder + id.
    bool foundC5 = false;
    for (const auto& e : chapters) {
        if (e.chapterID.value == c5.chapterID.value) { foundC5 = true; }
    }
    REQUIRE(foundC5);

    // And its folder is order-key-named (chapter-<key>), not a recycled numeric slug of a
    // survivor — verified implicitly by the distinctness assertions above, and by the
    // reopened order being strictly increasing.
    for (std::size_t i = 1; i < chapters.size(); ++i) {
        REQUIRE(chapters[i - 1].orderKey < chapters[i].orderKey);
    }
}

TEST_CASE("rebuildIndexIfInconsistent self-heals an I-0072-corrupt index",
          "[Chapter][index][EP-027][I-0072]") {
    TempDir projectDir, appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    scrivi::CoreServices services;
    services.fileSystem   = &lfs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;
    scrivi::ScriviCore core(services);

    scrivi::CreateProjectRequest preq;
    preq.projectRootPath = projectDir.str();
    preq.appSupportRoot  = appSupportDir.str();
    preq.title = "Novel"; preq.slug = "novel"; preq.author = author();
    auto created = core.createProject(preq);
    REQUIRE(created.ok());

    scrivi::CreateChapterRequest cr;
    cr.projectRootPath = projectDir.str();
    cr.appSupportRoot  = appSupportDir.str();
    cr.projectID       = created.value().project.projectID;
    cr.author          = author();
    auto c2 = core.createChapter(cr);
    REQUIRE(c2.ok());

    // Corrupt the index the way I-0072 did: keep the real entries, but inject a PHANTOM
    // chapter (a path whose sidecar has a different id) and a DUPLICATE of a real path.
    const std::string msPath =
        projectDir.str() + "/manuscript/manuscript.meta.json";
    {
        std::ifstream in(msPath, std::ios::binary);
        std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        auto parsed = scrivi::schemas::parseManuscriptMeta(json);
        REQUIRE(parsed.ok());
        auto ms = parsed.value();
        const std::string realPath = ms.chapters.front().path;
        ms.chapters.push_back({scrivi::ChapterID{"chapter_phantom"}, realPath});  // phantom id, dup path
        std::ofstream out(msPath, std::ios::binary);
        out << scrivi::schemas::serializeManuscriptMeta(ms);
    }

    // Sanity: the index is now inconsistent (more entries than real folders).
    auto beforeR = scrivi::manuscript::listChaptersByOrder(lfs, projectDir.str());
    REQUIRE(beforeR.ok());
    const std::size_t realChapterCount = beforeR.value().size();   // 2 on disk

    // Self-heal.
    auto healed = scrivi::manuscript::rebuildIndexIfInconsistent(lfs, projectDir.str());
    REQUIRE(healed.ok());
    REQUIRE(healed.value() == true);   // it rewrote

    // The index now matches disk exactly: no phantom, no duplicate, correct order.
    std::ifstream in(msPath, std::ios::binary);
    std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto reparsed = scrivi::schemas::parseManuscriptMeta(json);
    REQUIRE(reparsed.ok());
    REQUIRE(reparsed.value().chapters.size() == realChapterCount);
    std::set<std::string> paths, ids;
    for (const auto& ref : reparsed.value().chapters) {
        REQUIRE(paths.insert(ref.path).second);            // no duplicate path
        REQUIRE(ids.insert(ref.chapterID.value).second);   // no duplicate id
        REQUIRE(ref.chapterID.value != "chapter_phantom"); // phantom gone
    }

    // Idempotent: a second call finds it consistent and does nothing.
    auto again = scrivi::manuscript::rebuildIndexIfInconsistent(lfs, projectDir.str());
    REQUIRE(again.ok());
    REQUIRE(again.value() == false);
}
