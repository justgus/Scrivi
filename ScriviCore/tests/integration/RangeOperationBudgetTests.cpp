// RangeOperationBudgetTests.cpp — EP-053 S1 (SP-186, T-0614): the DISK BUDGET of a structural operation.
//
// ✅ R7 (user, 2026-10-17): "A two scene paste should take 4 file reads and 4 file writes. We must structure our in memory
// structures in such a way as to minimise the number of disk operations performed."
// Every filesystem call is counted (`CountingFileSystem`) on a real on-disk project — 6 chapters of 20 scenes, written in the
// shipped shape — so a budget here is absolute: it does not grow with the manuscript, and a walk that comes back fails it.
// ⚠️ Measured 2026-10-17 against `FragmentPaster` (EP-029) before the range operations existed: see the Sprint log.

#include "mocks/CountingFileSystem.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "manuscript/FragmentPaster.hpp"
#include "manuscript/ProjectIndex.hpp"
#include "manuscript/Range.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"

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
namespace ms = scrivi::manuscript;

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-ep053-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    TempDir(const TempDir&)            = delete;
    TempDir& operator=(const TempDir&) = delete;
    [[nodiscard]] std::string str() const { return path.string(); }
};

// A project built through the REAL core (the shipped on-disk shape), then filled with scenes written directly.
struct Harness {
    TempDir projectDir;
    TempDir appSupportDir;
    scrivi::platform::LocalFileSystem        real;
    scrivi::testing::CountingFileSystem      counting{real};
    scrivi::mocks::FixedClock                clock{"2026-10-17T00:00:00Z"};
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
    [[nodiscard]] std::string root() const { return projectDir.str(); }
    [[nodiscard]] scrivi::AuthorshipRef author() const {
        return {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};
    }

    void build(int chapters, int scenesPerChapter) {
        scrivi::ScriviCore core{services};
        scrivi::CreateProjectRequest req;
        req.projectRootPath = root();
        req.appSupportRoot  = appSupportDir.str();
        req.title           = "Budget Novel";
        req.slug            = "budget-novel";
        req.author          = author();
        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID.value;
        for (int c = 2; c <= chapters; ++c) {
            scrivi::CreateChapterRequest cr;
            cr.projectRootPath = root();
            cr.appSupportRoot  = appSupportDir.str();
            cr.projectID       = scrivi::ProjectID{projectID};
            cr.author          = author();
            REQUIRE(core.createChapter(cr).ok());
        }
        int ci = 0;
        for (const auto& entry : fs::directory_iterator(projectDir.path / "manuscript")) {
            if (!entry.is_directory()) { continue; }
            ++ci;
            for (int s = 2; s <= scenesPerChapter; ++s) {
                char key[12];
                std::snprintf(key, sizeof key, "%03d", s);
                writeScene(entry.path().string(), key, "scene-c" + std::to_string(ci) + "-" + key);
            }
        }
    }
    static void writeScene(const std::string& chapterDir, const std::string& orderKey, const std::string& sceneID) {
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
        std::ofstream(chapterDir + "/" + orderKey + "-scene.meta.json", std::ios::binary)
            << scrivi::schemas::serializeSceneMeta(meta);
        std::ofstream(chapterDir + "/" + orderKey + "-scene.md", std::ios::binary) << "Body of scene " << orderKey << ".\n";
    }
};

std::size_t writesOf(const scrivi::testing::CountingFileSystem::Counts& c) {
    return c.atomicWrite + c.appendTextFile + c.createDirectories + c.createExclusive + c.removeFile + c.renamePath +
           c.copyFileInBlocks;
}

void report(const char* what, const scrivi::testing::CountingFileSystem& cfs) {
    const auto& t = cfs.total();
    std::cout << "\n=== [EP-053] " << what << " ===\n"
              << "  file reads  : " << t.readTextFile << "\n"
              << "  listings    : " << t.listDirectory << "\n"
              << "  probes      : " << t.probes() << "\n"
              << "  writes      : " << writesOf(t) << "\n"
              << "  TOTAL       : " << t.total() << "\n";
    for (const auto& [path, counts] : cfs.hottestPaths(8)) {
        std::cout << "    " << counts.total() << "  " << path << "\n";
    }
    std::cout << std::endl;
}

}  // namespace

// ⛔ AC0: the budget — a two-scene paste (head into the caret scene + ONE new scene) into a 120-scene manuscript.
// R7: ≤ 4 reads and ≤ 4 writes, and no listing.
TEST_CASE("[EP-053] a two-scene paste stays within 4 reads and 4 writes", "[ep053][budget]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);   // what the C ABI holds in memory
    REQUIRE(indexR.ok());
    const ms::ProjectIndex& index = indexR.value();
    const auto& target = index.scenesInOrder().at(3);               // chapter 1's fourth scene
    h.counting.reset();

    // ⚠️ TODAY'S BEST PATH (SP-185): `FragmentPaster` with the index lookup — measured, to show the guard can see the cost.
    ms::PasteFragmentRequest req;
    req.projectRootPath = h.root();
    req.appSupportRoot  = h.appSupportDir.str();
    req.projectID       = scrivi::ProjectID{h.projectID};
    req.author          = h.author();
    req.caretSceneID    = target.sceneID;
    req.caretByteOffset = 5;
    ms::FragmentPiece head;
    head.opensWith = ms::OpensWith::None;
    head.text      = "pasted head";
    ms::FragmentPiece next;
    next.opensWith = ms::OpensWith::Scene;
    next.text      = "pasted scene";
    req.fragment.pieces = {head, next};
    ms::FragmentPaster paster{h.services};
    auto r = paster.paste(req, [&](const scrivi::SceneID& id) -> std::optional<ms::ResolvedScene> {
        auto loc = index.findScene(id);
        if (!loc) { return std::nullopt; }
        ms::ResolvedScene s;
        s.sceneID = loc->sceneID; s.chapterID = loc->chapterID; s.title = loc->title; s.chapterTitle = loc->chapterTitle;
        s.slug = loc->slug; s.status = loc->status; s.metadataPath = loc->metadataPath; s.contentPath = loc->contentPath;
        s.chapterMetadataPath = loc->chapterMetadataPath;
        return s;
    });
    REQUIRE(r.ok());
    report("two-scene paste", h.counting);
    const auto& t = h.counting.total();
    CHECK(t.listDirectory == 0);
    CHECK(t.readTextFile <= 4);
    CHECK(writesOf(t) <= 4);
}

// ── AC1: the Range itself ────────────────────────────────────────────────────────────────────────────────────────────────

TEST_CASE("[EP-053] a Range round-trips scrivi.range.v1 and refuses a non-contiguous one", "[ep053][range]") {
    ms::Range r;
    r.append({scrivi::SceneID{"s1"}, scrivi::ChapterID{"c1"}, "One", "First", 6, 7, 13, "Bravo. "});
    r.append({scrivi::SceneID{"s2"}, scrivi::ChapterID{"c1"}, "One", "Second", 0, 8, 8, "Charlie."});
    r.append({scrivi::SceneID{"s3"}, scrivi::ChapterID{"c2"}, "Two", "Third", 0, 3, 6, "Del"});
    auto back = ms::Range::fromJsonString(r.toJsonString());
    REQUIRE(back.ok());
    REQUIRE(back.value().size() == 3);
    auto it = back.value().entries().begin();
    CHECK(it->sceneID.value == "s1");
    CHECK(it->start == 6);
    CHECK(it->text == "Bravo. ");
    ++it;
    CHECK(it->coversWholeScene());
    CHECK(!ms::Range::opensChapter(std::prev(it), it));
    ++it;
    CHECK(ms::Range::opensChapter(std::prev(it), it));
    CHECK(it->chapterTitle == "Two");

    // Contiguity: a middle entry that ends mid-scene is not a range.
    ms::Range gap;
    gap.append({scrivi::SceneID{"s1"}, scrivi::ChapterID{"c1"}, "", "", 0, 2, 5, "ab"});
    gap.append({scrivi::SceneID{"s2"}, scrivi::ChapterID{"c1"}, "", "", 0, 1, 1, "c"});
    CHECK(!ms::Range::fromJsonString(gap.toJsonString()).ok());
    // A length that is not the text's.
    ms::Range lying;
    lying.append({scrivi::SceneID{"s1"}, scrivi::ChapterID{"c1"}, "", "", 0, 9, 9, "short"});
    CHECK(!ms::Range::fromJsonString(lying.toJsonString()).ok());
}

TEST_CASE("[EP-053] a Range converts to and from scrivi.fragment.v1", "[ep053][range]") {
    ms::Range r;
    r.append({scrivi::SceneID{"s1"}, scrivi::ChapterID{"c1"}, "One", "First", 6, 7, 13, "Bravo. "});
    r.append({scrivi::SceneID{"s2"}, scrivi::ChapterID{"c1"}, "One", "Second", 0, 8, 8, "Charlie."});
    r.append({scrivi::SceneID{"s3"}, scrivi::ChapterID{"c2"}, "Two", "Third", 0, 3, 6, "Del"});
    const ms::Fragment f = r.toFragment();
    REQUIRE(f.pieces.size() == 3);
    CHECK(f.pieces[0].opensWith == ms::OpensWith::None);
    CHECK(f.pieces[0].partial == ms::Partial::Head);
    CHECK(f.pieces[1].opensWith == ms::OpensWith::Scene);
    CHECK(f.pieces[2].opensWith == ms::OpensWith::Chapter);
    CHECK(f.pieces[2].chapterTitle == "Two");
    CHECK(f.pieces[2].partial == ms::Partial::Tail);
    CHECK(f.plainText == "Bravo. \n\nCharlie.\n\nDel");

    const ms::Range again = ms::Range::fromFragment(f);
    REQUIRE(again.size() == 3);
    auto it = again.entries().begin();
    CHECK(it->text == "Bravo. ");
    ++it;
    CHECK(!ms::Range::opensChapter(std::prev(it), it));   // the boundaries survive
    CHECK(it->sceneTitle == "Second");
    ++it;
    CHECK(ms::Range::opensChapter(std::prev(it), it));
    CHECK(it->chapterTitle == "Two");
    // …and back to a fragment, the same pieces (text, boundaries, titles).
    const ms::Fragment f2 = again.toFragment();
    for (std::size_t i = 0; i < 3; ++i) {
        CHECK(f2.pieces[i].text == f.pieces[i].text);
        CHECK(f2.pieces[i].opensWith == f.pieces[i].opensWith);
        CHECK(f2.pieces[i].chapterTitle == f.pieces[i].chapterTitle);
        CHECK(f2.pieces[i].sceneTitle == f.pieces[i].sceneTitle);
    }
}
