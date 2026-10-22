// RangeOperationBudgetTests.cpp — EP-053 S1 (SP-186, T-0614): the DISK BUDGET of a structural operation.
//
// ✅ R7 (user, 2026-10-17): "A two scene paste should take 4 file reads and 4 file writes. We must structure our in memory
// structures in such a way as to minimise the number of disk operations performed."
// Every filesystem call is counted (`CountingFileSystem`) on a real on-disk project — 6 chapters of 20 scenes, written in the
// shipped shape — so a budget here is absolute: it does not grow with the manuscript, and a walk that comes back fails it.
// ⚠️ Measured 2026-10-17 against `FragmentPaster` (EP-029) before the range operations existed: see the Sprint log.
// ✅ Budgets re-ruled 2026-10-20 (user): the chapter sidecar's scene list is kept current (paste 3 reads + 5 writes).

#include "mocks/CountingFileSystem.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "manuscript/ProjectIndex.hpp"
#include "manuscript/Range.hpp"
#include "manuscript/RangeOperations.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"
#include "util/OrderKey.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

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
                // ⚠️ Generated-shape keys ("B", "C", …), as the app makes them — not legacy "002": see [I-0312].
                const std::string key(1, char('A' + s - 1));
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

scrivi::AuthorshipRef author() {
    return {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};
}

std::string readBody(const Harness& h, const ms::SceneLocation& loc) {
    std::ifstream in(fs::path(h.root()) / loc.contentPath, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

}  // namespace

// ⛔ AC0 → AC3: the budget — a two-scene paste (head into the caret scene + ONE new scene) into a 120-scene manuscript.
// ✅ Re-ruled 2026-10-20 (user): ≤ 3 reads and ≤ 5 writes, no listing — R7's 8 file calls, with the chapter sidecar's scene
// list kept current. ⚠️ Measured 2026-10-17 through `FragmentPaster` + the index: 47 reads, 2 listings, 11 writes.
TEST_CASE("[EP-053] a two-scene paste stays within 3 reads and 5 writes", "[ep053][budget]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);   // what the C ABI holds in memory
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    const ms::SceneLocation source = index.scenesInOrder().at(40);   // chapter 3
    const ms::SceneLocation target = index.scenesInOrder().at(3);    // chapter 1's fourth scene
    const std::string caretText = readBody(h, target);

    ms::Range range;   // copied from chapter 3: its own scenes are still there, so the paste mints fresh IDs
    range.append({source.sceneID, source.chapterID, source.chapterTitle, source.title, 0, 11, 11, "pasted head"});
    range.append({index.scenesInOrder().at(41).sceneID, source.chapterID, source.chapterTitle, "", 0, 12, 20, "pasted scene"});
    h.counting.reset();

    ms::RangeOperations ops{h.services, h.root(), index};
    auto r = ops.paste(range, target.sceneID, 5, caretText, author());
    REQUIRE(r.ok());
    report("two-scene paste", h.counting);
    const auto& t = h.counting.total();
    CHECK(t.listDirectory == 0);
    CHECK(t.readTextFile <= 3);
    CHECK(writesOf(t) <= 5);
    CHECK(r.value().indexCurrent);
    CHECK(index.sceneCount() == 121);
}

// AC4: a two-scene cut (the end of one scene + the start of the next) — the start scene's text and sidecar, the end
// scene's text, the chapter sidecar; the start scene's two files written, the end scene's two removed, the chapter sidecar.
// ✅ [SP-191] Q3b: + ONE read — the removed scene's sidecar, which the Range carries so the cut's undo restores it exactly.
TEST_CASE("[EP-053] a two-scene cut stays within 5 reads and 5 writes", "[ep053][budget]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    auto read = ms::RangeOperations::read(h.services, h.root(), index,
                                          index.scenesInOrder().at(3).sceneID, 4,
                                          index.scenesInOrder().at(4).sceneID, 6);
    REQUIRE(read.ok());
    h.counting.reset();

    ms::RangeOperations ops{h.services, h.root(), index};
    auto r = ops.cut(read.value(), author());
    REQUIRE(r.ok());
    report("two-scene cut", h.counting);
    const auto& t = h.counting.total();
    CHECK(t.listDirectory == 0);
    CHECK(t.readTextFile <= 5);
    CHECK(writesOf(t) <= 5);
    CHECK(r.value().indexCurrent);
    CHECK(index.sceneCount() == 119);
}

// [SP-191] Q3e: a chapter merge is a RELABEL — the 20 scenes of chapter 2 move into chapter 1's folder. Every chapter's keys
// start from the same first key, so each scene is renamed there and its sidecar rewritten (slug, contentPath); its text is
// only moved. Reads: each moved sidecar, the emptied chapter's sidecar (it travels for the undo), chapter 1's sidecar, the
// manuscript sidecar. Writes: per scene, the text moved, the sidecar written anew and the old one removed; chapter 1's
// sidecar; the emptied chapter's sidecar removed; the manuscript sidecar. ⛔ No listing, no text read.
TEST_CASE("[SP-191] a chapter merge (relabel) of N scenes stays within N + 3 reads and 3N + 3 writes", "[ep053][budget]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    const scrivi::ChapterID first = index.scenesInOrder().at(0).chapterID;
    ms::Range run;
    for (std::size_t i = 20; i < 40; ++i) {
        const auto& s = index.scenesInOrder().at(i);
        run.append({s.sceneID, s.chapterID, s.chapterTitle, s.title, 0, 0, 0, ""});
    }
    h.counting.reset();

    ms::RangeOperations ops{h.services, h.root(), index};
    auto r = ops.relabel(run, first, author());
    REQUIRE(r.ok());
    report("chapter merge (relabel) of 20 scenes", h.counting);
    const auto& t = h.counting.total();
    CHECK(t.listDirectory == 0);
    CHECK(t.readTextFile <= 20 + 3);
    CHECK(writesOf(t) <= 3 * 20 + 3);
    CHECK(r.value().indexCurrent);

    // Its undo re-creates the chapter in an empty folder: the scenes keep their names, so nothing is read but the two
    // sidecars it edits. Writes: per scene, the two files moved; the folder, the new chapter's sidecar, chapter 1's, the
    // manuscript's.
    h.counting.reset();
    auto back = ops.relabel(r.value().range, r.value().fromChapterID, author());
    REQUIRE(back.ok());
    report("undo of the chapter merge", h.counting);
    const auto& u = h.counting.total();
    CHECK(u.listDirectory == 0);
    CHECK(u.readTextFile <= 2);
    CHECK(writesOf(u) <= 2 * 20 + 4);
    CHECK(back.value().indexCurrent);
}

// ✅ [SP-191] (user, 2026-10-22): MOVE THE SMALLER SIDE. Over a network share every move is a rename and renames do not
// overlap there (measured on a Docker SMB share), so a chapter split, a chapter merge and their undos each rename only the
// FEWER scenes: two renames a scene, never more than 2 × the smaller side. Chapter 2 (20 scenes) is split three scenes in,
// so the three before the caret move; then the split is undone, and the three move back.
TEST_CASE("[SP-191] a chapter split renames only the smaller side, and so does its undo", "[ep053][budget][sp191]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    for (const std::size_t at : {std::size_t(22), std::size_t(36)}) {   // 3 scenes before the caret, then 3 after it
        const ms::SceneLocation caret = index.scenesInOrder().at(at);
        const std::string text = readBody(h, caret);
        ms::Range split;   // an empty two-entry Range whose second entry opens a chapter (Q1)
        split.append({scrivi::SceneID{}, caret.chapterID, "", "", 0, 0, 0, ""});
        split.append({scrivi::SceneID{}, scrivi::ChapterID{}, "", "", 0, 0, 0, ""});
        h.counting.reset();
        ms::RangeOperations ops{h.services, h.root(), index};
        auto r = ops.paste(split, caret.sceneID, 5, text, author());
        REQUIRE(r.ok());
        report(at == 22 ? "chapter split, 3 scenes before the caret" : "chapter split, 3 scenes after the caret", h.counting);
        CHECK(h.counting.total().renamePath <= 2 * 3);
        CHECK(h.counting.total().listDirectory == 0);
        CHECK(r.value().indexCurrent);

        h.counting.reset();
        auto back = ops.cut(r.value().range, author());
        REQUIRE(back.ok());
        report("its undo", h.counting);
        CHECK(h.counting.total().renamePath <= 2 * 3);
        CHECK(back.value().indexCurrent);
    }
}

// The same for a relabel: chapter 2's last 17 scenes go into a new chapter (a split at a scene's start, Q8) — the 3 left
// behind move — and the merge back folds those 3 into the 17's folder.
TEST_CASE("[SP-191] a relabel into a new chapter, and the merge back, rename only the smaller side", "[ep053][budget][sp191]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    ms::Range run;
    for (std::size_t i = 23; i < 40; ++i) {
        const auto& s = index.scenesInOrder().at(i);
        run.append({s.sceneID, s.chapterID, s.chapterTitle, s.title, 0, 0, 0, ""});
    }
    h.counting.reset();
    ms::RangeOperations ops{h.services, h.root(), index};
    auto r = ops.relabel(run, scrivi::ChapterID{}, author());
    REQUIRE(r.ok());
    report("relabel of 17 scenes into a new chapter (3 left behind)", h.counting);
    CHECK(h.counting.total().renamePath <= 2 * 3);
    CHECK(r.value().indexCurrent);

    h.counting.reset();
    auto back = ops.relabel(r.value().range, r.value().fromChapterID, author());
    REQUIRE(back.ok());
    report("the merge back (3 into 17)", h.counting);
    CHECK(h.counting.total().renamePath <= 2 * 3);
    CHECK(back.value().indexCurrent);
}

// ✅ [SP-191]: a file call that fails inside a CONCURRENT batch fails the operation, as a sequential one did. One of the
// scenes a chapter split must move has lost its text file behind the index's back, so its rename fails mid-batch.
TEST_CASE("[SP-191] a file call that fails in a concurrent batch fails the operation", "[ep053][budget][sp191]") {
    Harness h;
    h.build(2, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    const ms::SceneLocation caret = index.scenesInOrder().at(4);    // 5 scenes up to the caret, 15 after: the 5 move
    fs::remove(fs::path(h.root()) / index.scenesInOrder().at(2).contentPath);
    ms::Range split;
    split.append({scrivi::SceneID{}, caret.chapterID, "", "", 0, 0, 0, ""});
    split.append({scrivi::SceneID{}, scrivi::ChapterID{}, "", "", 0, 0, 0, ""});
    ms::RangeOperations ops{h.services, h.root(), index};
    auto r = ops.paste(split, caret.sceneID, 5, readBody(h, caret), author());
    CHECK(!r.ok());
}

// A FileSystem whose renames are SLOW, as over a share — so a write sent in the same batch as a rename overtakes it.
class SlowRenames final : public scrivi::FileSystem {
public:
    explicit SlowRenames(scrivi::FileSystem& inner) : inner_(inner) {}
    scrivi::Result<bool> exists(const scrivi::AbsolutePath& p) override { return inner_.exists(p); }
    scrivi::Result<bool> isDirectory(const scrivi::AbsolutePath& p) override { return inner_.isDirectory(p); }
    scrivi::Result<std::uint64_t> deviceID(const scrivi::AbsolutePath& p) override { return inner_.deviceID(p); }
    scrivi::Result<void> createDirectories(const scrivi::AbsolutePath& p) override { return inner_.createDirectories(p); }
    scrivi::Result<scrivi::Utf8Text> readTextFile(const scrivi::AbsolutePath& p) override { return inner_.readTextFile(p); }
    scrivi::Result<void> atomicWriteTextFile(const scrivi::AbsolutePath& p, std::string_view t) override {
        return inner_.atomicWriteTextFile(p, t);
    }
    scrivi::Result<void> createFileExclusive(const scrivi::AbsolutePath& p, std::string_view t) override {
        return inner_.createFileExclusive(p, t);
    }
    scrivi::Result<void> appendTextFile(const scrivi::AbsolutePath& p, std::string_view t) override {
        return inner_.appendTextFile(p, t);
    }
    scrivi::Result<std::vector<scrivi::AbsolutePath>> listDirectory(const scrivi::AbsolutePath& p) override {
        return inner_.listDirectory(p);
    }
    scrivi::Result<void> removeFile(const scrivi::AbsolutePath& p) override { return inner_.removeFile(p); }
    scrivi::Result<void> renamePath(const scrivi::AbsolutePath& from, const scrivi::AbsolutePath& to) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        return inner_.renamePath(from, to);
    }
    scrivi::Result<void> copyFileInBlocks(const scrivi::AbsolutePath& from, const scrivi::AbsolutePath& to, std::size_t block,
                                          const std::function<scrivi::Result<void>()>& onBlock) override {
        return inner_.copyFileInBlocks(from, to, block, onBlock);
    }

private:
    scrivi::FileSystem& inner_;
};

// ✅ [SP-191]: when a chapter split moves the scenes before the caret, they leave their folder in a batch of their own,
// BEFORE the new tail scene is written there — its file name can be one of theirs (keys are only unique within a chapter).
// Scene 2's key is made the very one the tail gets; with slow renames, a write in the same batch would land first and then
// be carried off by the move.
TEST_CASE("[SP-191] the scenes before the caret leave before the new scene takes one of their names", "[ep053][budget][sp191]") {
    Harness h;
    h.build(1, 6);   // keys 001, B, C, D, E, F
    const fs::path dir = [&] {
        for (const auto& e : fs::directory_iterator(fs::path(h.root()) / "manuscript")) {
            if (e.is_directory()) { return e.path(); }
        }
        return fs::path();
    }();
    const std::string key = scrivi::util::keyBetween("", "C");   // what the tail scene of a split in scene 2 is given
    REQUIRE(key > "001");
    REQUIRE(key < "C");
    fs::remove(dir / "B-scene.meta.json");
    fs::rename(dir / "B-scene.md", dir / (key + "-scene.md"));
    Harness::writeScene(dir.string(), key, "scene-c1-B");   // the same scene, under that key (its text written anew)
    std::ofstream(dir / (key + "-scene.md"), std::ios::binary) << "Body of scene B.\n";

    SlowRenames slow{h.real};
    h.services.fileSystem = &slow;
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    ms::ProjectIndex& index = indexR.value();
    const ms::SceneLocation caret = index.scenesInOrder().at(1);
    REQUIRE(caret.sceneID.value == "scene-c1-B");
    ms::Range split;
    split.append({scrivi::SceneID{}, caret.chapterID, "", "", 0, 0, 0, ""});
    split.append({scrivi::SceneID{}, scrivi::ChapterID{}, "", "", 0, 0, 0, ""});
    ms::RangeOperations ops{h.services, h.root(), index};
    auto r = ops.paste(split, caret.sceneID, 5, readBody(h, caret), author());
    REQUIRE(r.ok());

    auto fresh = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(fresh.ok());
    const auto& order = fresh.value().scenesInOrder();
    REQUIRE(order.size() == 7);
    CHECK(order[1].sceneID.value == "scene-c1-B");
    CHECK(readBody(h, order[1]) == "Body ");
    CHECK(readBody(h, order[2]) == "of scene B.\n");   // the tail, in its own files
    CHECK(order[2].chapterID.value != order[1].chapterID.value);
}

// AC2: a range read reads its scenes' texts and nothing else.
TEST_CASE("[EP-053] a range read reads only its scenes", "[ep053][budget]") {
    Harness h;
    h.build(6, 20);
    auto indexR = ms::ProjectIndex::build(h.root(), h.services);
    REQUIRE(indexR.ok());
    const ms::ProjectIndex& index = indexR.value();
    h.counting.reset();
    auto r = ms::RangeOperations::read(h.services, h.root(), index,
                                       index.scenesInOrder().at(18).sceneID, 2,
                                       index.scenesInOrder().at(22).sceneID, 3);   // across a chapter boundary
    REQUIRE(r.ok());
    CHECK(r.value().size() == 5);
    const auto& t = h.counting.total();
    CHECK(t.listDirectory == 0);
    CHECK(t.readTextFile == 5);
    CHECK(writesOf(t) == 0);
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
