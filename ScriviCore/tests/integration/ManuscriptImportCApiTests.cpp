// EP-054 S6a (SP-218, T-0679) — a manuscript from Markdown or plain text, through the C ABI only ([I-0113]).
//
// What is proved: Markdown's chapters, scenes and kept headings (Q3) and the export's own title page; text kept literal,
// its scenes and its encodings (Q4); a hard-wrapped file reported (Q5); what Scrivi keeps only as text reported (Q8); a file
// that is not text, or too large, refused; the Range pasted at a caret and cut back (one undo step, Q2); a new project made
// from a file; and the round trip — exported, imported as a new project, exported again: the same bytes (AC3).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>

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

JsonDoc ok(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

const char* const kId = "identity-001";
const char* const kPersona = "persona-001";

struct ImportFixture {
    fs::path base, appSupport;

    ImportFixture() {
        static std::atomic<int> counter{0};
        base = fs::temp_directory_path() / ("scrivi-mimport-" + std::to_string(::getpid()) + "-" +
                                            std::to_string(counter.fetch_add(1)));
        fs::remove_all(base);
        appSupport = base / "support";
        fs::create_directories(appSupport);
    }
    ~ImportFixture() {
        for (const auto& e : fs::directory_iterator(base)) {
            if (e.path().extension() == ".scrivi") scrivi_free(scrivi_close_project(e.path().c_str()));
        }
        std::error_code ec;
        fs::remove_all(base, ec);
    }
    fs::path file(const std::string& name, const std::string& bytes) const {
        const fs::path p = base / name;
        std::ofstream(p, std::ios::binary) << bytes;
        return p;
    }
    JsonDoc read(const std::string& name, const std::string& bytes) {
        return ok(scrivi_import_manuscript(file(name, bytes).c_str(), ""));
    }
    std::string newProjectOptions(const fs::path& root) const {
        JsonDoc np;
        np.setString("projectRootPath", root.string());
        np.setString("appSupportRoot", appSupport.string());
        np.setString("identityID", kId);
        np.setString("personaID", kPersona);
        np.setString("displayName", "T");
        JsonDoc o;
        o.setSubDoc("newProject", std::move(np));
        return o.dump();
    }
    JsonDoc newProject(const fs::path& source, const fs::path& root) {
        return ok(scrivi_import_manuscript(source.c_str(), newProjectOptions(root).c_str()));
    }
    static std::string markdownOf(const fs::path& root) {
        return ok(scrivi_export_manuscript(root.c_str(), R"({"format":"markdown"})")).getString("text");
    }
};

// One entry of the Range: chapter placeholder, chapter title, text.
struct Entry { std::string chapter, title, text; };

std::vector<Entry> entries(const JsonDoc& result) {
    std::vector<Entry> out;
    const JsonDoc range = result.getSubDoc("range");
    CHECK(range.getString("schema") == "scrivi.range.v1");
    for (std::size_t i = 0; i < range.arraySize("scenes"); ++i) {
        const JsonDoc e = range.arrayItem("scenes", i);
        CHECK(e.getString("sceneID").empty());
        CHECK(e.getInt64("length") == static_cast<int64_t>(e.getString("text").size()));
        out.push_back({e.getString("chapterID"), e.getString("chapterTitle"), e.getString("text")});
    }
    return out;
}

std::string reportKinds(const JsonDoc& result) {
    std::string out;
    for (std::size_t i = 0; i < result.arraySize("report"); ++i) {
        out += (out.empty() ? "" : ",") + result.arrayItem("report", i).getString("kind");
    }
    return out;
}

} // namespace

TEST_CASE("Manuscript import: Markdown chapters, scenes and the writer's own headings (Q3)", "[manuscript-import][capi]") {
    ImportFixture f;
    const JsonDoc r = f.read("book.md", "Before any chapter.\n\n"
                                        "# One\n\nHe said *hello*.\nA second line.\n\n## Kept\n\nMore.\n\n"
                                        "* * *\n\nScene two.\n\n---\n\n\n\n***\n\n"   // an empty scene is left out
                                        "# Two #\n\n___\n\nLast.\n");
    const auto e = entries(r);
    REQUIRE(e.size() == 4);
    // Text before the first `#` is an untitled chapter whose first scene joins the caret's (no empty head entry).
    CHECK(e[0].title.empty());
    CHECK(e[0].text == "Before any chapter.");
    CHECK(e[1].title == "One");
    CHECK(e[1].chapter != e[0].chapter);
    CHECK(e[1].text == "He said *hello*.\nA second line.\n\n## Kept\n\nMore.");
    CHECK(e[2].chapter == e[1].chapter);
    CHECK(e[2].text == "Scene two.");
    CHECK(e[3].title == "Two");   // closing hashes dropped
    CHECK(e[3].text == "Last.");
    CHECK(r.getInt64("chapterCount") == 3);
    CHECK(r.getInt64("sceneCount") == 4);
    CHECK(r.getString("encoding") == "utf-8");
    CHECK(r.getString("title").empty());
    CHECK(r.arraySize("report") == 0);
}

TEST_CASE("Manuscript import: a file that opens with a chapter, and a heading-only chapter", "[manuscript-import][capi]") {
    ImportFixture f;
    const auto e = entries(f.read("b.markdown", "# One\n\n# Two\n\nText.\n"));
    REQUIRE(e.size() == 3);
    // At a caret the first entry stays with the caret scene, so "One" opens a chapter there.
    CHECK(e[0].text.empty());
    CHECK(e[0].chapter == "~import-caret");
    CHECK(e[1].title == "One");
    CHECK(e[1].text.empty());   // its only scene, kept so the heading survives
    CHECK(e[2].title == "Two");
    CHECK(e[2].text == "Text.");
}

TEST_CASE("Manuscript import: exactly the export's title page is the title (Q3)", "[manuscript-import][capi]") {
    ImportFixture f;
    const JsonDoc r = f.read("t.md", "# The Title\n\nA Subtitle\n\nby Ann Author\n\n# Chapter A\n\nText.\n");
    CHECK(r.getString("title") == "The Title");
    CHECK(r.getString("subtitle") == "A Subtitle");
    CHECK(reportKinds(r) == "byline");
    CHECK(r.arrayItem("report", 0).getString("example") == "by Ann Author");
    const auto e = entries(r);
    REQUIRE(e.size() == 2);
    CHECK(e[1].title == "Chapter A");

    // Without the `by …` line nothing is guessed: the title is a chapter.
    const JsonDoc g = f.read("u.md", "# The Title\n\nA Subtitle\n\n# Chapter A\n\nText.\n");
    CHECK(g.getString("title").empty());
    CHECK(entries(g)[1].title == "The Title");
}

TEST_CASE("Manuscript import: plain text is literal, one chapter, scenes at its break lines (Q4)", "[manuscript-import][capi]") {
    ImportFixture f;
    const JsonDoc r = f.read("notes.txt", "# Not a heading\r\nwith *stars* and _under_ \\ back.\r\n\r\n"
                                          "  ***  \r\n\r\nSecond.\r\n\r\n#\r\n\r\nThird.\r\n\r\n* * *\r\n\r\nFourth.");
    const auto e = entries(r);
    REQUIRE(e.size() == 4);
    CHECK(e[0].text == "\\# Not a heading\nwith \\*stars\\* and \\_under\\_ \\\\ back\\.");
    CHECK(e[1].text == "Second\\.");
    CHECK(e[2].text == "Third\\.");
    CHECK(e[3].text == "Fourth\\.");
    for (const auto& x : e) { CHECK(x.chapter == e[0].chapter); }
    CHECK(r.getInt64("chapterCount") == 1);
}

TEST_CASE("Manuscript import: UTF-8 with a BOM, UTF-16 and Windows-1252 (Q4)", "[manuscript-import][capi]") {
    ImportFixture f;
    const JsonDoc bom = f.read("a.txt", "\xEF\xBB\xBF" "caf\xC3\xA9");
    CHECK(bom.getString("encoding") == "utf-8");
    CHECK(entries(bom)[0].text == "caf\xC3\xA9");

    const std::string le("\xFF\xFE" "c\0a\0f\0\xE9\0=\xD8\0\xDE", 14);   // "café" + U+1F600 as a surrogate pair
    const JsonDoc u16 = f.read("b.txt", le);
    CHECK(u16.getString("encoding") == "utf-16le");
    CHECK(entries(u16)[0].text == "caf\xC3\xA9\xF0\x9F\x98\x80");
    const std::string be("\xFE\xFF\0h\0i", 6);
    CHECK(f.read("c.txt", be).getString("encoding") == "utf-16be");

    const JsonDoc w = f.read("d.txt", "\x93" "caf\xE9" "\x94");
    CHECK(w.getString("encoding") == "windows-1252");
    CHECK(entries(w)[0].text == "\xE2\x80\x9C" "caf\xC3\xA9\xE2\x80\x9D");
    CHECK(reportKinds(w) == "encodingGuessed");
}

TEST_CASE("Manuscript import: a hard-wrapped file keeps its line breaks and is reported (Q5)", "[manuscript-import][capi]") {
    ImportFixture f;
    std::string text;
    for (int p = 0; p < 6; ++p) {
        for (int l = 0; l < 5; ++l) { text += "A line of a wrapped paragraph, well under eighty characters.\n"; }
        text += "\n";
    }
    const JsonDoc r = f.read("wrapped.txt", text);
    CHECK(reportKinds(r) == "hardWrapped");
    CHECK(entries(r)[0].text.find("characters\\.\nA line") != std::string::npos);
    // Prose that is not wrapped is not reported.
    CHECK(f.read("prose.txt", "One paragraph.\n\nAnother.\n").arraySize("report") == 0);
}

TEST_CASE("Manuscript import: what Scrivi keeps only as text is reported (Q8)", "[manuscript-import][capi]") {
    ImportFixture f;
    const JsonDoc r = f.read("m.md", "A [link](http://x) and [another](y).\n\n![alt](pic.png)\n\n> quoted\n\n"
                                     "```\ncode\n```\n\n<div>html</div> and a heart <3\n\n| a | b |\n|---|---|\n");
    CHECK(reportKinds(r) == "link,image,blockQuote,code,html,table");
    CHECK(r.arrayItem("report", 0).getInt64("count") == 1);   // counted per line
    CHECK(r.arrayItem("report", 0).getString("example") == "A [link](http://x) and [another](y).");
    CHECK(r.arrayItem("report", 3).getInt64("count") == 2);   // both fence lines
    // The text itself is kept.
    CHECK(entries(r)[0].text.find("![alt](pic.png)") != std::string::npos);
}

TEST_CASE("Manuscript import: refused — not text, too large, no file, a bad format", "[manuscript-import][capi]") {
    ImportFixture f;
    const JsonDoc bin = envelope(scrivi_import_manuscript(f.file("x.md", std::string("PK\3\4\0\0", 6)).c_str(), ""));
    CHECK_FALSE(bin.getBool("ok"));
    CHECK(bin.getSubDoc("error").getString("detail") == "notText");

    const fs::path big = f.base / "big.md";
    { std::ofstream(big) << "x"; }
    fs::resize_file(big, (64u << 20) + 1);   // sparse: nothing is written
    const JsonDoc large = envelope(scrivi_import_manuscript(big.c_str(), ""));
    CHECK_FALSE(large.getBool("ok"));
    CHECK(large.getSubDoc("error").getString("detail") == "tooLarge");

    CHECK_FALSE(envelope(scrivi_import_manuscript((f.base / "none.md").c_str(), "")).getBool("ok"));
    CHECK_FALSE(envelope(scrivi_import_manuscript(f.file("y.md", "x").c_str(), R"({"format":"docx"})")).getBool("ok"));
    CHECK_FALSE(envelope(scrivi_import_manuscript("", "")).getBool("ok"));
}

TEST_CASE("Manuscript import: into a project at the caret, and cut back as one step (Q2)", "[manuscript-import][capi]") {
    ImportFixture f;
    const fs::path root = f.base / "Host.scrivi";
    const JsonDoc created = ok(scrivi_create_project(root.c_str(), f.appSupport.c_str(), "Host", "host", kId, kPersona, "T"));
    const JsonDoc first = created.getSubDoc("firstScene");
    const std::string pid = created.getString("projectID"), scene = first.getString("sceneID");
    ok(scrivi_save_scene(pid.c_str(), root.c_str(), f.appSupport.c_str(), scene.c_str(),
                         first.getString("metadataPath").c_str(), first.getString("contentPath").c_str(), "Hello.", 0, 0,
                         0.0, kId, kPersona, "T"));
    const std::string before = ImportFixture::markdownOf(root);

    const JsonDoc r = f.read("in.md", "# Imported\n\nIts text.\n\n* * *\n\nIts second scene.\n");
    const std::string range = r.getSubDoc("range").dump();
    const JsonDoc pasted = ok(scrivi_range_paste(root.c_str(), range.c_str(), scene.c_str(), 6, "Hello.", kId, kPersona, "T"));
    CHECK(ImportFixture::markdownOf(root) ==
          "# Host\n\nby T\n\n# Chapter 1\n\nHello.\n\n# Imported\n\nIts text.\n\n* * *\n\nIts second scene.\n");
    // Its undo: the Range the paste inserted, cut.
    ok(scrivi_range_cut(root.c_str(), pasted.getSubDoc("range").dump().c_str(), kId, kPersona, "T"));
    CHECK(ImportFixture::markdownOf(root) == before);
}

TEST_CASE("Manuscript import: into a project whose chapters have pre-T-0358 lowercase keys", "[manuscript-import][capi]") {
    // SP-218 live pass: "could not find a free chapter order key" — its chapter folders were chapter-k and chapter-s.
    ImportFixture f;
    const fs::path root = f.base / "Old.scrivi";
    const JsonDoc created = ok(scrivi_create_project(root.c_str(), f.appSupport.c_str(), "Old", "old", kId, kPersona, "T"));
    const JsonDoc first = created.getSubDoc("firstScene");
    const std::string pid = created.getString("projectID"), scene = first.getString("sceneID");
    ok(scrivi_save_scene(pid.c_str(), root.c_str(), f.appSupport.c_str(), scene.c_str(),
                         first.getString("metadataPath").c_str(), first.getString("contentPath").c_str(), "Hello.", 0, 0,
                         0.0, kId, kPersona, "T"));
    scrivi_free(scrivi_close_project(root.c_str()));
    // Its one chapter's folder becomes "chapter-k", as a pre-T-0358 project's are.
    const fs::path manuscript = root / "manuscript";
    fs::path chapterDir;
    for (const auto& e : fs::directory_iterator(manuscript)) { if (e.is_directory()) chapterDir = e.path(); }
    REQUIRE(!chapterDir.empty());
    const std::string oldName = chapterDir.filename().string();
    fs::rename(chapterDir, manuscript / "chapter-k");
    const fs::path metaPath = manuscript / "manuscript.meta.json";
    std::string meta;
    { std::ifstream in(metaPath); meta.assign(std::istreambuf_iterator<char>(in), {}); }
    const auto at = meta.find("manuscript/" + oldName + "/");
    REQUIRE(at != std::string::npos);
    meta.replace(at, oldName.size() + 11, "manuscript/chapter-k");
    std::ofstream(metaPath, std::ios::trunc) << meta;
    ok(scrivi_open_project(root.c_str(), f.appSupport.c_str(), kId));
    const std::string before = ImportFixture::markdownOf(root);

    const JsonDoc r = f.read("in.md", "# One\n\nIts text.\n\n# Two\n\nMore.\n\n* * *\n\nAnd more.\n");
    const JsonDoc pasted = ok(scrivi_range_paste(root.c_str(), r.getSubDoc("range").dump().c_str(), scene.c_str(), 6,
                                                 "Hello.", kId, kPersona, "T"));
    CHECK(ImportFixture::markdownOf(root) ==
          "# Old\n\nby T\n\n# Chapter 1\n\nHello.\n\n# One\n\nIts text.\n\n# Two\n\nMore.\n\n* * *\n\nAnd more.\n");
    ok(scrivi_range_cut(root.c_str(), pasted.getSubDoc("range").dump().c_str(), kId, kPersona, "T"));
    CHECK(ImportFixture::markdownOf(root) == before);
}

TEST_CASE("Manuscript import: a new project from a file — its title, chapters and subtitle", "[manuscript-import][capi]") {
    ImportFixture f;
    const fs::path root = f.base / "Novel.scrivi";
    const JsonDoc r = f.newProject(f.file("My Novel.md", "# Opening\n\nFirst.\n\n* * *\n\nSecond.\n\n# Next\n\nThird.\n"), root);
    CHECK(r.getString("projectRoot") == root.string());
    CHECK_FALSE(r.getString("projectID").empty());
    CHECK(r.getString("title") == "My Novel");   // no title page: the file's name
    CHECK_FALSE(r.contains("range"));
    CHECK(ImportFixture::markdownOf(root) ==
          "# My Novel\n\nby T\n\n# Opening\n\nFirst.\n\n* * *\n\nSecond.\n\n# Next\n\nThird.\n");
    ok(scrivi_open_project(root.c_str(), f.appSupport.c_str(), kId));   // it opens as any project does

    // A plain-text file: one untitled chapter, the project's own.
    const fs::path notes = f.base / "Notes.scrivi";
    f.newProject(f.file("Notes.txt", "Just *this*.\n"), notes);
    CHECK(ImportFixture::markdownOf(notes) == "# Notes\n\nby T\n\n# Chapter 1\n\nJust \\*this\\*\\.\n");

    // A folder already there is never written over.
    const std::string before = ImportFixture::markdownOf(root);
    const JsonDoc again = envelope(scrivi_import_manuscript(f.file("again.md", "x").c_str(), f.newProjectOptions(root).c_str()));
    CHECK_FALSE(again.getBool("ok"));
    CHECK(again.getSubDoc("error").getString("detail") == "exists");
    CHECK(ImportFixture::markdownOf(root) == before);
}

TEST_CASE("Manuscript import: export, import, export — the same bytes (AC3)", "[manuscript-import][capi]") {
    ImportFixture f;
    const fs::path root = f.base / "Source.scrivi";
    const JsonDoc created = ok(scrivi_create_project(root.c_str(), f.appSupport.c_str(), "The Book", "book", kId, kPersona, "T"));
    const std::string pid = created.getString("projectID");
    const JsonDoc firstScene = created.getSubDoc("firstScene");
    auto save = [&](const fs::path& p, const JsonDoc& s, const std::string& id, const std::string& text) {
        ok(scrivi_save_scene(pid.c_str(), p.c_str(), f.appSupport.c_str(), id.c_str(), s.getString("metadataPath").c_str(),
                             s.getString("contentPath").c_str(), text.c_str(), 0, 0, 0.0, kId, kPersona, "T"));
    };
    save(root, firstScene, firstScene.getString("sceneID"),
         "He said *hello* and **left**.\nA second line.\n\n## A Heading\n\n- one\n- two\n\nA literal \\*star\\*.");
    const JsonDoc opened = ok(scrivi_open_project(root.c_str(), f.appSupport.c_str(), kId));
    ok(scrivi_rename_chapter(root.c_str(), opened.arrayItem("scenes", 0).getString("chapterMetadataPath").c_str(), "Begin"));
    const std::string chapter1 = opened.arrayItem("scenes", 0).getString("chapterID");
    const JsonDoc s2 = ok(scrivi_create_scene(root.c_str(), f.appSupport.c_str(), pid.c_str(), chapter1.c_str(),
                                              firstScene.getString("sceneID").c_str(), "", kId, kPersona, "T"));
    save(root, s2, s2.getString("sceneID"), "Second scene.");
    const JsonDoc c2 = ok(scrivi_create_chapter(root.c_str(), f.appSupport.c_str(), pid.c_str(), kId, kPersona, "T",
                                                chapter1.c_str()));
    JsonDoc c2scene;
    c2scene.setString("metadataPath", c2.getString("firstSceneMetadataPath"));
    c2scene.setString("contentPath", c2.getString("firstSceneContentPath"));
    save(root, c2scene, c2.getString("firstSceneID"), "1. first\n2. second");
    ok(scrivi_put_project_settings(root.c_str(), R"({"subtitle":"A Romance"})"));
    const char* authors = R"({"authors":[{"displayName":"Ann Author"}]})";
    ok(scrivi_set_authors(root.c_str(), authors));

    const std::string exported = ImportFixture::markdownOf(root);
    const fs::path copy = f.base / "Copy.scrivi";
    const JsonDoc r = f.newProject(f.file("The Book.md", exported), copy);
    CHECK(r.getString("title") == "The Book");
    CHECK(reportKinds(r) == "byline");
    ok(scrivi_set_authors(copy.c_str(), authors));   // authors are identities (EP-058), never read from a byline
    CHECK(ImportFixture::markdownOf(copy) == exported);
}

TEST_CASE("Manuscript import: a 1,000-scene book as a new project (AC1, timed)", "[.import-perf]") {
    ImportFixture f;
    std::string text;
    const std::string para(1800, 'x');
    for (int c = 0; c < 100; ++c) {
        text += "# Chapter " + std::to_string(c + 1) + "\n\n";
        for (int s = 0; s < 10; ++s) { text += (s ? "* * *\n\n" : "") + para + "\n\n"; }
    }
    const fs::path source = f.file("big.md", text);
    const auto t0 = std::chrono::steady_clock::now();
    const JsonDoc read = ok(scrivi_import_manuscript(source.c_str(), ""));
    const auto t1 = std::chrono::steady_clock::now();
    const JsonDoc made = f.newProject(source, f.base / "Big.scrivi");
    const auto t2 = std::chrono::steady_clock::now();
    using ms = std::chrono::duration<double, std::milli>;
    WARN("read " << ms(t1 - t0).count() << " ms; new project " << ms(t2 - t1).count() << " ms; range "
                 << read.getSubDoc("range").dump().size() << " bytes");
    CHECK(made.getInt64("sceneCount") == 1000);

    // Into a project at the caret, and its undo (the host's structured paste, then the cut of what it inserted).
    const fs::path host = f.base / "Host.scrivi";
    const JsonDoc created = ok(scrivi_create_project(host.c_str(), f.appSupport.c_str(), "Host", "host", kId, kPersona, "T"));
    const std::string scene = created.getSubDoc("firstScene").getString("sceneID");
    const auto t3 = std::chrono::steady_clock::now();
    const JsonDoc pasted = ok(scrivi_range_paste(host.c_str(), read.getSubDoc("range").dump().c_str(), scene.c_str(), 0, "",
                                                 kId, kPersona, "T"));
    const auto t4 = std::chrono::steady_clock::now();
    ok(scrivi_range_cut(host.c_str(), pasted.getSubDoc("range").dump().c_str(), kId, kPersona, "T"));
    const auto t5 = std::chrono::steady_clock::now();
    WARN("paste at a caret " << ms(t4 - t3).count() << " ms; its undo " << ms(t5 - t4).count() << " ms");
}
