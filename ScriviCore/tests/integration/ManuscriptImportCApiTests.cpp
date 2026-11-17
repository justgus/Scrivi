// EP-054 S6a (SP-218, T-0679) — a manuscript from Markdown or plain text, through the C ABI only ([I-0113]).
//
// What is proved: Markdown's chapters, scenes and kept headings (Q3) and the export's own title page; text kept literal,
// its scenes and its encodings (Q4); a hard-wrapped file reported (Q5); what Scrivi keeps only as text reported (Q8); a file
// that is not text, or too large, refused; the Range pasted at a caret and cut back (one undo step, Q2); a new project made
// from a file; and the round trip — exported, imported as a new project, exported again: the same bytes (AC3).
// S6b (SP-219, T-0682): a .docx through the same endpoint — SP-218 Q7 and SP-219 Q1–Q4, the refusals, Scrivi's own .docx
// back to the same manuscript (AC2), a textutil file, and mangled files under the sanitizers (AC3).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include "miniz.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <unistd.h>
#include <utility>
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

namespace {

const char* const kAuthors = R"({"authors":[{"displayName":"Ann Author"}]})";

// The round trips' source: two chapters, emphasis, a line break, a heading, both lists, a literal star, a subtitle, an author.
fs::path sourceBook(ImportFixture& f) {
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
    ok(scrivi_set_authors(root.c_str(), kAuthors));
    return root;
}

} // namespace

TEST_CASE("Manuscript import: export, import, export — the same bytes (AC3)", "[manuscript-import][capi]") {
    ImportFixture f;
    const fs::path root = sourceBook(f);
    const char* authors = kAuthors;

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

// ---- EP-054 S6b (SP-219, T-0682): a .docx, through the same endpoint. Rulings: SP-218 Q7, SP-219 Q1–Q4.

namespace {

constexpr const char* kW = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

// A .docx from its parts: document.xml's body, and optionally styles, numbering and other parts.
std::string docx(const std::string& body, const std::string& styles = {}, const std::string& numbering = {},
                 const std::vector<std::pair<std::string, std::string>>& extra = {}) {
    std::vector<std::pair<std::string, std::string>> parts{
        {"[Content_Types].xml", "<?xml version=\"1.0\"?><Types/>"},
        {"word/document.xml", std::string("<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:document xmlns:w=\"") + kW +
                                  "\" xmlns:r=\"r\"><w:body>" + body + "</w:body></w:document>"}};
    if (!styles.empty()) parts.emplace_back("word/styles.xml", std::string("<w:styles xmlns:w=\"") + kW + "\">" + styles + "</w:styles>");
    if (!numbering.empty())
        parts.emplace_back("word/numbering.xml", std::string("<w:numbering xmlns:w=\"") + kW + "\">" + numbering + "</w:numbering>");
    for (const auto& e : extra) parts.push_back(e);
    mz_zip_archive z{};
    REQUIRE(mz_zip_writer_init_heap(&z, 0, 0));
    for (const auto& [name, text] : parts)
        REQUIRE(mz_zip_writer_add_mem(&z, name.c_str(), text.data(), text.size(), MZ_DEFAULT_LEVEL));
    void* buf = nullptr;
    std::size_t len = 0;
    REQUIRE(mz_zip_writer_finalize_heap_archive(&z, &buf, &len));
    std::string out(static_cast<const char*>(buf), len);
    z.m_pFree(z.m_pAlloc_opaque, buf);
    mz_zip_writer_end(&z);
    return out;
}

std::string R(const std::string& text, const std::string& rPr = {}) {
    return "<w:r>" + (rPr.empty() ? "" : "<w:rPr>" + rPr + "</w:rPr>") + "<w:t xml:space=\"preserve\">" + text + "</w:t></w:r>";
}
std::string P(const std::string& style, const std::string& runs, const std::string& pPr = {}) {
    return "<w:p><w:pPr>" + (style.empty() ? "" : "<w:pStyle w:val=\"" + style + "\"/>") + pPr + "</w:pPr>" + runs + "</w:p>";
}
std::string S(const std::string& id, const std::string& name, const std::string& inner = {}) {
    return "<w:style w:type=\"paragraph\" w:styleId=\"" + id + "\"><w:name w:val=\"" + name + "\"/>" + inner + "</w:style>";
}

const std::string kStyles = S("Heading1", "heading 1", "<w:pPr><w:pageBreakBefore/></w:pPr>") + S("Heading2", "heading 2") +
                            S("SceneBreak", "Scene Break") + S("Title", "Title") + S("Subtitle", "Subtitle") +
                            S("Author", "Author");
const std::string kNumbering =
    R"(<w:abstractNum w:abstractNumId="0"><w:lvl w:ilvl="0"><w:numFmt w:val="bullet"/></w:lvl></w:abstractNum>)"
    R"(<w:abstractNum w:abstractNumId="1"><w:lvl w:ilvl="0"><w:start w:val="1"/><w:numFmt w:val="decimal"/></w:lvl></w:abstractNum>)"
    R"(<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num><w:num w:numId="2"><w:abstractNumId w:val="1"/></w:num>)";
const std::string kItem1 = R"(<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>)";
const std::string kItem2 = R"(<w:numPr><w:ilvl w:val="0"/><w:numId w:val="2"/></w:numPr>)";

std::string detail(const JsonDoc& env) { return env.getSubDoc("error").getString("detail"); }

} // namespace

TEST_CASE("Manuscript import (.docx): chapters, scenes, emphasis, lists, tracked changes (SP-218 Q7)", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const std::string body =
        P("Heading1", R("One")) +
        P("", R("She read ") + R("great", "<w:i/>") + R(" and ") + R("left", "<w:b/>") + R(". A * star and _under_.")) +
        P("", R("line one") + "<w:r><w:br/></w:r>" + R("line two")) +
        P("SceneBreak", "") +                                                     // S4's: empty, bordered
        P("", R("Second scene.")) + P("", R("* * *"), R"(<w:jc w:val="center"/>)") +
        P("", R("Third scene.")) + P("", R("#"), R"(<w:jc w:val="center"/>)") +
        P("", R("Fourth.")) + P("", R"(<w:r><w:br w:type="page"/></w:r>)") +    // a page break, no heading after it
        P("", R("Fifth, after a page break.")) +
        P("Heading2", R("Her Heading")) +
        P("", R("one"), kItem1) + P("", R("two"), kItem1) + P("", R("first"), kItem2) + P("", R("second"), kItem2) +
        P("", R("Kept ") + "<w:ins><w:r><w:t>inserted</w:t></w:r></w:ins><w:del><w:r><w:delText>deleted</w:delText></w:r></w:del>") +
        P("Heading1", R("Two")) +                                                 // its page break opens no scene
        P("", R("Chapter two text."));
    const JsonDoc r = f.read("book.docx", docx(body, kStyles, kNumbering));
    const auto e = entries(r);
    REQUIRE(e.size() == 7);
    CHECK(e[0].chapter == "~import-caret");
    CHECK(e[1].title == "One");
    CHECK(e[1].text == "She read *great* and **left**. A \\* star and \\_under\\_.\n\nline one\nline two");
    CHECK(e[2].text == "Second scene.");
    CHECK(e[3].text == "Third scene.");
    CHECK(e[4].text == "Fourth.");
    CHECK(e[5].text == "Fifth, after a page break.\n\n## Her Heading\n\n- one\n- two\n\n1. first\n2. second\n\nKept inserted");
    CHECK(e[6].title == "Two");
    CHECK(e[6].text == "Chapter two text.");
    CHECK(r.getInt64("chapterCount") == 2);
    CHECK(r.arraySize("report") == 0);
}

TEST_CASE("Manuscript import (.docx): S4's title page is the title; a Title elsewhere is a chapter (Q1)", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const JsonDoc r = f.read("t.docx", docx(P("Title", R("The Title")) + P("Subtitle", R("A Subtitle")) +
                                                P("Author", R("by Ann Author")) + P("Heading1", R("Chapter A")) +
                                                P("", R("Text.")) + P("Title", R("Part Two")) + P("", R("More.")),
                                            kStyles));
    CHECK(r.getString("title") == "The Title");
    CHECK(r.getString("subtitle") == "A Subtitle");
    CHECK(reportKinds(r) == "byline");
    const auto e = entries(r);
    REQUIRE(e.size() == 3);
    CHECK(e[1].title == "Chapter A");
    CHECK(e[2].title == "Part Two");

    // An Author paragraph without "by " is still the byline; with no byline the Title is a chapter, as Markdown's Q3.
    CHECK(f.read("a.docx", docx(P("Title", R("T")) + P("Author", R("Ann")) + P("Heading1", R("C")) + P("", R("x")), kStyles))
              .getString("title") == "T");
    const JsonDoc g = f.read("u.docx", docx(P("Title", R("The Title")) + P("Heading1", R("C")) + P("", R("x")), kStyles));
    CHECK(g.getString("title").empty());
    CHECK(entries(g)[1].title == "The Title");
}

TEST_CASE("Manuscript import (.docx): headings by style NAME, through basedOn, else the outline level (Q3)", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const std::string styles = S("P1", "Heading 1") + S("Mine", "My Chapter", "<w:basedOn w:val=\"P1\"/>") +
                               S("P2", "heading 3") + S("Body", "Body Text");
    const JsonDoc r = f.read("h.docx", docx(P("Mine", R("By basedOn")) + P("", R("a")) + P("P2", R("Third level")) +
                                                P("Body", R("Outlined"), R"(<w:outlineLvl w:val="0"/>)") + P("", R("b")) +
                                                P("Unknown", R("No such style")),
                                            styles));
    const auto e = entries(r);
    REQUIRE(e.size() == 3);
    CHECK(e[1].title == "By basedOn");
    CHECK(e[1].text == "a\n\n### Third level");
    CHECK(e[2].title == "Outlined");
    CHECK(e[2].text == "b\n\nNo such style");
}

TEST_CASE("Manuscript import (.docx): literal text stays literal; line starts that would be markup are escaped", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const auto e = entries(f.read("l.docx", docx(P("", R("# not a heading")) + P("", R("- not a list")) +
                                                 P("", R("1. not a list")) + P("", R("&gt; not a quote")) +
                                                 P("", R("back\\slash and `tick`")) + P("", R("  spaced  ")) +
                                                 P("", R("bold ", "<w:b/>") + R("then italic", "<w:i/>")))));
    REQUIRE(e.size() == 1);
    CHECK(e[0].text == "\\# not a heading\n\n\\- not a list\n\n1\\. not a list\n\n\\> not a quote\n\n"
                       "back\\\\slash and \\`tick\\`\n\nspaced\n\n**bold** *then italic*");
}

TEST_CASE("Manuscript import (.docx): what Scrivi cannot keep is reported, never dropped silently (Q7, Q4)", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const std::string body =
        P("", R("A picture here") + "<w:r><w:drawing><wp:inline xmlns:wp=\"wp\"/></w:drawing></w:r>") +
        "<w:tbl><w:tr><w:tc>" + P("", R("Cell A")) + "</w:tc><w:tc>" + P("", R("Cell B")) + "</w:tc></w:tr></w:tbl>" +
        P("", R("Noted") + "<w:r><w:footnoteReference w:id=\"1\"/></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r>") +
        P("", R("Commented") + "<w:r><w:commentReference w:id=\"0\"/></w:r>") +
        P("", R("Boxed") + "<w:r><w:pict><v:shape xmlns:v=\"v\"><w:txbxContent>" + P("", R("inside")) +
                  "</w:txbxContent></v:shape></w:pict></w:r>") +
        P("", R("A ") + R("line", "<w:u w:val=\"single\"/>") + R(" under")) +
        P("", R("Centred body"), R"(<w:jc w:val="center"/>)") +
        P("", R("Plain")) + P("", R("Plain again")) + P("", R("Odd face", R"(<w:rFonts w:ascii="Courier"/>)")) +
        P("", "<w:hyperlink r:id=\"rId9\">" + R("a link") + "</w:hyperlink>");
    const JsonDoc r = f.read("n.docx", docx(body, {}, {}, {{"word/header1.xml", "<w:hdr xmlns:w=\"" + std::string(kW) +
                                                                                     "\">" + P("", R("Running head")) + "</w:hdr>"},
                                                           {"word/footer1.xml", "<w:ftr xmlns:w=\"" + std::string(kW) +
                                                                                     "\"><w:p/></w:ftr>"}}));
    CHECK(reportKinds(r) == "picture,docxTable,footnote,endnote,comment,textBox,link,formatting,headerFooter");
    CHECK(r.arrayItem("report", 0).getString("example") == "A picture here");
    CHECK(r.arrayItem("report", 1).getString("example") == "Cell A");
    CHECK(r.arrayItem("report", 7).getInt64("count") == 3);   // the underline, the centred line, the other face
    CHECK(r.arrayItem("report", 7).getString("example") == "A line under");
    CHECK(r.arrayItem("report", 8).getInt64("count") == 1);   // the empty footer is not reported
    CHECK(r.arrayItem("report", 8).getString("example") == "Running head");
    const auto e = entries(r);
    REQUIRE(e.size() == 1);
    // The table's text is kept; the text box's is not (reported).
    CHECK(e[0].text.find("Cell A\n\nCell B") != std::string::npos);
    CHECK(e[0].text.find("inside") == std::string::npos);
    CHECK(e[0].text.find("A line under") != std::string::npos);
}

TEST_CASE("Manuscript import (.docx): found by its bytes; refused — a .doc, not Word, broken XML, a part too large (Q2)", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    // Named .md, read as the Word document it is.
    CHECK(entries(f.read("renamed.md", docx(P("", R("Hello."))))).at(0).text == "Hello.");

    const JsonDoc doc = envelope(scrivi_import_manuscript(f.file("old.doc", std::string("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1rest", 12)).c_str(), ""));
    CHECK_FALSE(doc.getBool("ok"));
    CHECK(detail(doc) == "legacyDoc");
    CHECK(doc.getSubDoc("error").getString("message").find("Save it as .docx") != std::string::npos);

    mz_zip_archive z{};
    REQUIRE(mz_zip_writer_init_heap(&z, 0, 0));
    REQUIRE(mz_zip_writer_add_mem(&z, "content.xml", "<x/>", 4, MZ_DEFAULT_LEVEL));
    void* buf = nullptr;
    std::size_t len = 0;
    REQUIRE(mz_zip_writer_finalize_heap_archive(&z, &buf, &len));
    const std::string odt(static_cast<const char*>(buf), len);
    z.m_pFree(z.m_pAlloc_opaque, buf);
    mz_zip_writer_end(&z);
    CHECK(detail(envelope(scrivi_import_manuscript(f.file("not-word.docx", odt).c_str(), ""))) == "notText");

    std::string broken = docx("<w:p><w:r><w:t>unclosed");
    CHECK(detail(envelope(scrivi_import_manuscript(f.file("broken.docx", broken).c_str(), ""))) == "notText");

    // The central directory claims document.xml inflates past the cap: refused before anything is inflated.
    std::string big = docx(P("", R("x")));
    const std::string name = "word/document.xml";
    for (std::size_t at = big.find("PK\1\2"); at != std::string::npos; at = big.find("PK\1\2", at + 4)) {
        const auto nameLen = static_cast<unsigned char>(big[at + 28]) | (static_cast<unsigned char>(big[at + 29]) << 8);
        if (big.compare(at + 46, nameLen, name) == 0) { big.replace(at + 24, 4, "\x00\x00\x00\x20", 4); }   // 512 MB
    }
    CHECK(detail(envelope(scrivi_import_manuscript(f.file("big.docx", big).c_str(), ""))) == "tooLarge");
}

TEST_CASE("Manuscript import (.docx): a DTD's entities are never expanded", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const std::string xml = std::string("<?xml version=\"1.0\"?><!DOCTYPE w:document [<!ENTITY boom \"BOOM\">"
                                        "<!ENTITY ext SYSTEM \"file:///etc/passwd\">]><w:document xmlns:w=\"") + kW +
                            "\"><w:body><w:p><w:r><w:t>a &boom; &ext; b</w:t></w:r></w:p></w:body></w:document>";
    mz_zip_archive z{};
    REQUIRE(mz_zip_writer_init_heap(&z, 0, 0));
    REQUIRE(mz_zip_writer_add_mem(&z, "word/document.xml", xml.data(), xml.size(), MZ_DEFAULT_LEVEL));
    void* buf = nullptr;
    std::size_t len = 0;
    REQUIRE(mz_zip_writer_finalize_heap_archive(&z, &buf, &len));
    const std::string bytes(static_cast<const char*>(buf), len);
    z.m_pFree(z.m_pAlloc_opaque, buf);
    mz_zip_writer_end(&z);
    const JsonDoc env = envelope(scrivi_import_manuscript(f.file("dtd.docx", bytes).c_str(), ""));
    const std::string all = env.dump();
    CHECK(all.find("BOOM") == std::string::npos);
    CHECK(all.find("root:") == std::string::npos);
}

TEST_CASE("Manuscript import (.docx): Scrivi's own .docx export imports back to the same manuscript (AC2)", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    const fs::path root = sourceBook(f);
    const fs::path file = f.base / "The Book.docx";
    ok(scrivi_export_manuscript(root.c_str(), (R"({"format":"docx","destPath":")" + file.string() + "\"}").c_str()));
    const fs::path copy = f.base / "Copy.scrivi";
    const JsonDoc r = f.newProject(file, copy);
    CHECK(r.getString("title") == "The Book");
    CHECK(reportKinds(r) == "byline");
    ok(scrivi_put_project_settings(copy.c_str(), R"({"subtitle":"A Romance"})"));
    ok(scrivi_set_authors(copy.c_str(), kAuthors));
    CHECK(ImportFixture::markdownOf(copy) == ImportFixture::markdownOf(root));
}

// textutil (macOS's own converter) writes no styles, no numbering and no tables: its headings are bold text in a larger size,
// its lists are tab-and-bullet text, its table cells plain paragraphs. Nothing is guessed (Q3); the sizes and the underline
// are the formatting note (Q4).
TEST_CASE("Manuscript import (.docx): a textutil file — no styles, so its headings are body text, reported (Q3, Q5)", "[manuscript-import][docx][capi]") {
    const fs::path file = fs::path(SCRIVI_TEST_FIXTURES_DIR) / "docx" / "sp219-textutil.docx";
    const JsonDoc r = ok(scrivi_import_manuscript(file.c_str(), ""));
    const auto e = entries(r);
    REQUIRE(e.size() == 2);   // one untitled chapter; its * * * opens the second scene
    CHECK(e[0].text.starts_with("**Chapter One**\n\nShe read the note with *great* care and **left at once**. "
                                "A literal \\* star and \\_under\\_ stay as typed.\n\n"
                                "The second paragraph has a line break here\nand goes on after it."));
    CHECK(e[1].text == "The second scene opens in the rain.\n\n**A Heading of Her Own**\n\n\u2022\tbullet one\n\n"
                       "\u2022\tbullet two\n\n1\tfirst step\n\n2\tsecond step\n\n**Chapter Two**\n\n"
                       "A word is underlined here, which Scrivi keeps as plain text.\n\nCell A\n\nCell B\n\nCell C\n\n"
                       "Cell D\n\nThe last paragraph of chapter two.");
    CHECK(reportKinds(r) == "formatting");
    CHECK(r.arrayItem("report", 0).getInt64("count") == 4);   // three headings in a larger size, one underline
    CHECK(r.arrayItem("report", 0).getString("example") == "Chapter One");
}

TEST_CASE("Manuscript import (.docx): truncated and mangled files never crash", "[manuscript-import][docx][capi]") {
    ImportFixture f;
    std::ifstream in(fs::path(SCRIVI_TEST_FIXTURES_DIR) / "docx" / "sp219-textutil.docx", std::ios::binary);
    const std::string good{std::istreambuf_iterator<char>(in), {}};
    REQUIRE(good.size() > 1000);
    int refused = 0;
    for (std::size_t cut = 4; cut < good.size(); cut += 97) {
        const JsonDoc env = envelope(scrivi_import_manuscript(f.file("cut.docx", good.substr(0, cut)).c_str(), ""));
        refused += env.getBool("ok") ? 0 : 1;
    }
    CHECK(refused > 0);
    for (std::size_t at = 0; at < good.size(); at += 131) {
        std::string bad = good;
        bad[at] = static_cast<char>(bad[at] ^ 0x5A);
        (void)envelope(scrivi_import_manuscript(f.file("flip.docx", bad).c_str(), ""));   // ok or refused: either, no crash
    }
}

// The live pass's own file (in ~/ScriviLinux/projects/imports too): every case once, so its notes sheet names every kind.
TEST_CASE("Manuscript import (.docx): the every-case fixture", "[manuscript-import][docx][capi]") {
    const fs::path file = fs::path(SCRIVI_TEST_FIXTURES_DIR) / "docx" / "sp219-every-case.docx";
    const JsonDoc r = ok(scrivi_import_manuscript(file.c_str(), ""));
    CHECK(r.getString("title") == "The Fixture Book");
    CHECK(r.getString("subtitle") == "Every Case Once");
    CHECK(reportKinds(r) == "byline,docxTable,footnote,comment,formatting,headerFooter");
    const auto e = entries(r);
    REQUIRE(e.size() == 6);   // the caret's head, four scenes of Chapter One, Chapter Two
    CHECK(e[1].title == "Chapter One");
    CHECK(e[2].text == "A scene after a Scene Break paragraph.");
    CHECK(e[3].text == "A scene after a centred \\* \\* \\*.");
    CHECK(e[4].text == "A scene after a page break.\n\n## Her Own Heading\n\n- bullet one\n- bullet two\n\n"
                       "1. first step\n2. second step\n\nKept, an accepted insertion.");
    CHECK(e[5].title == "Chapter Two");
    CHECK(e[5].text == "A word is underlined here.\n\nCell A\n\nCell B\n\nA footnote here and a comment.\n\nThe last paragraph.");
}

// ---- S7 (SP-220, T-0683): a Scrivener project — Q1–Q6, Q3a; the fixtures made in Scrivener 3.5.2 (Q7a).

namespace {

// A synthetic `.scriv`: the binder's items, and each document's files by their path inside the project.
struct Scriv {
    std::string binder;   // the Draft's <Children> content
    std::string outside;  // BinderItems after the Draft (Research, Trash)
    std::vector<std::pair<std::string, std::string>> files;
    std::string version = "2.0";
    std::string lists;    // StatusSettings / Keywords
};

fs::path writeScriv(const ImportFixture& f, const std::string& name, const Scriv& s) {
    const fs::path root = f.base / (name + ".scriv");
    fs::create_directories(root);
    std::ofstream(root / (name + ".scrivx"))
        << R"(<?xml version="1.0" encoding="UTF-8"?><ScrivenerProject Version=")" << s.version << R"("><Binder>)"
        << R"(<BinderItem UUID="00000000-0000-0000-0000-00000000D0D0" ID="0" Type="DraftFolder"><Title>Draft</Title><Children>)"
        << s.binder << "</Children></BinderItem>" << s.outside << "</Binder>" << s.lists << "</ScrivenerProject>";
    for (const auto& [rel, bytes] : s.files) {
        fs::create_directories((root / rel).parent_path());
        std::ofstream(root / rel, std::ios::binary) << bytes;
    }
    return root;
}

std::string item(const std::string& type, const std::string& id, const std::string& title, const std::string& inner = {},
                 const std::string& meta = "<IncludeInCompile>Yes</IncludeInCompile>", const std::string& extra = {}) {
    return R"(<BinderItem UUID=")" + id + R"(" ID=")" + id + R"(" Type=")" + type + R"("><Title>)" + title +
           "</Title><MetaData>" + meta + "</MetaData>" + extra + (inner.empty() ? "" : "<Children>" + inner + "</Children>") +
           "</BinderItem>";
}

std::string rtf(const std::string& body) {
    return "{\\rtf1\\ansi\\ansicpg1252\\cocoartf2909\n{\\fonttbl\\f0\\froman\\fcharset0 Palatino-Roman;}\n"
           "{\\colortbl;\\red255\\green255\\blue255;\\red0\\green0\\blue0;\\red200\\green0\\blue0;}\n"
           "\\pard\\pardirnatural\\partightenfactor0\n\\f0\\fs26 \\cf0 " + body + "}";
}

std::string data(const std::string& uuid, const std::string& file) { return "Files/Data/" + uuid + "/" + file; }

// Each entry with its scene title and seed.
struct Scene { std::string chapter, chapterTitle, title, text, seed; };

std::vector<Scene> scenes(const JsonDoc& result) {
    std::vector<Scene> out;
    const JsonDoc range = result.getSubDoc("range");
    for (std::size_t i = 0; i < range.arraySize("scenes"); ++i) {
        const JsonDoc e = range.arrayItem("scenes", i);
        CHECK(e.getString("sceneID").empty());
        out.push_back({e.getString("chapterID"), e.getString("chapterTitle"), e.getString("sceneTitle"), e.getString("text"),
                       e.getString("sceneMeta")});
    }
    return out;
}

JsonDoc note(const JsonDoc& result, const std::string& kind) {
    for (std::size_t i = 0; i < result.arraySize("report"); ++i) {
        if (result.arrayItem("report", i).getString("kind") == kind) return result.arrayItem("report", i);
    }
    FAIL("no " << kind << " note in " << result.dump());
    return {};
}

fs::path fixture(const std::string& name) { return fs::path(SCRIVI_TEST_FIXTURES_DIR) / "scrivener" / name; }

} // namespace

TEST_CASE("Manuscript import (Scrivener): only a folder is a chapter; texts and their children are scenes (Q3, Q3a)",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    Scriv s;
    s.binder = item("Text", "A1", "Prologue") +
               item("Folder", "B1", "Part", item("Folder", "C1", "One",
                    item("Text", "D1", "Parent", item("Text", "E1", "Child")) + item("Folder", "C2", "Nested",
                         item("Text", "F1", "Inside")) + item("Text", "G1", "Back in One"))) +
               item("Folder", "H1", "Empty") + item("Text", "I1", "Epilogue");
    for (const char* id : {"A1", "D1", "E1", "F1", "G1", "I1"}) s.files.push_back({data(id, "content.rtf"), rtf(std::string(id) + "\\\n")});
    const JsonDoc r = ok(scrivi_import_manuscript(writeScriv(f, "Book", s).c_str(), ""));
    INFO(r.dump());
    const auto e = scenes(r);
    REQUIRE(e.size() == 7);
    // The Draft's own documents: an untitled chapter; so the paste does not open one.
    CHECK((e[0].chapterTitle == "" && e[0].title == "Prologue" && e[0].text == "A1"));
    CHECK((e[1].chapterTitle == "One" && e[1].title == "Parent" && e[2].title == "Child" && e[1].chapter == e[2].chapter));
    CHECK((e[3].chapterTitle == "Nested" && e[3].title == "Inside"));
    // Back in One after a nested folder: One again, as its own run (reading order is kept).
    CHECK((e[4].chapterTitle == "One" && e[4].title == "Back in One" && e[4].chapter != e[1].chapter));
    CHECK((e[5].chapterTitle == "Empty" && e[5].text.empty()));   // an empty folder is an empty chapter
    CHECK((e[6].chapterTitle == "" && e[6].title == "Epilogue"));
    CHECK(r.getInt64("chapterCount") == 6);
    CHECK(note(r, "parts").getString("example") == "Part");
}

TEST_CASE("Manuscript import (Scrivener): the RTF reader — emphasis, characters, skipped destinations, breaks (Q5)",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    Scriv s;
    s.binder = item("Text", "A1", "One");
    s.files.push_back({data("A1", "content.rtf"), rtf(
        "Plain {\\i italic} and {\\b bold\\b0 } then \\i on\\i0 .\\par "
        "Caf\\'e9 \\u8212? \\uc0\\u8226 \\uc1 \\emdash \\ldblquote q\\rdblquote  \\u-10179?\\u-8704? a\\tab b\\line c\\~d \\{x\\} \\\\\\par "
        "Text{\\footnote\\pard foot text\\par} goes on; "
        "{\\*\\expandedcolortbl;;}{\\*\\unknownthing hidden}{\\*\\shppict{\\pict\\pngblip 0011}}"
        "{\\field{\\*\\fldinst{HYPERLINK \"scrivlnk://AB\"}}{\\fldrslt linked}} and {\\field{\\*\\fldinst{HYPERLINK \"https://x\"}}{\\fldrslt web}}\\par "
        "* * *\\par "
        "After the break: 5 * 3 _x_ `c`\\par "
        "# not a heading\\par "
        "{\\cf3 red} words\\par "
        "{\\cf2 black} words\\par "
        "\\trowd\\pard\\intbl Cell A\\cell Cell B\\cell\\row\\pard After the table.\\par ")});
    const JsonDoc r = ok(scrivi_import_manuscript(writeScriv(f, "Rtf", s).c_str(), ""));
    INFO(r.dump());
    const auto e = scenes(r);
    REQUIRE(e.size() == 2);   // a `* * *` paragraph opens a scene inside the document
    CHECK(e[0].title == "One");
    CHECK(e[0].text == "Plain *italic* and **bold** then *on*.\n\n"
                       "Café — •—“q” \xF0\x9F\x98\x80 a\tb\nc\xC2\xA0" "d {x} \\\\\n\n"
                       "Text goes on; linked and web");   // a footnote's own \\par never ends the paragraph it sits in
    CHECK(e[1].title.empty());
    CHECK(e[1].text == "After the break: 5 \\* 3 \\_x\\_ \\`c\\`\n\n\\# not a heading\n\nred words\n\nblack words\n\n"
                       "Cell A\n\nCell B\n\nAfter the table.");
    CHECK(note(r, "footnote").getInt64("count") == 1);
    CHECK(note(r, "picture").getInt64("count") == 1);
    CHECK(note(r, "documentLink").getInt64("count") == 1);
    CHECK(note(r, "link").getInt64("count") == 1);
    CHECK(note(r, "docxTable").getInt64("count") == 1);
    CHECK(note(r, "formatting").getString("example") == "red words");   // black is not formatting
    CHECK(note(r, "formatting").getInt64("count") == 1);
}

TEST_CASE("Manuscript import (Scrivener): the metadata — synopsis, keywords, status as a seed; the rest reported (Q2, Q4)",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    Scriv s;
    s.lists = R"(<StatusSettings><StatusItems><Status ID="-1">No Status</Status><Status ID="0">To Do</Status>)"
              R"(<Status ID="2">First Draft</Status><Status ID="3">Revised Draft</Status><Status ID="5">Done</Status>)"
              R"(</StatusItems></StatusSettings><Keywords><Keyword ID="0"><Title>harbour</Title></Keyword>)"
              R"(<Keyword ID="1"><Title>night</Title></Keyword></Keywords>)";
    s.binder = item("Folder", "C1", "Chapter",
                    item("Text", "A1", "Revised", {},
                         "<IncludeInCompile>Yes</IncludeInCompile><StatusID>3</StatusID><LabelID>2</LabelID>",
                         R"(<Keywords><KeywordID>1</KeywordID><KeywordID>0</KeywordID></Keywords>)") +
                    item("Text", "A2", "Done", {}, "<IncludeInCompile>Yes</IncludeInCompile><StatusID>5</StatusID>") +
                    item("Text", "A3", "To do", {}, "<IncludeInCompile>No</IncludeInCompile><StatusID>0</StatusID>"
                         "<CustomMetaData><MetaDataItem><FieldID>x</FieldID><Value>y</Value></MetaDataItem></CustomMetaData>") +
                    item("Text", "A4", "Plain") + item("Image", "A5", "A picture"));
    s.outside = item("ResearchFolder", "R0", "Research", item("Text", "R1", "Notes on ships")) +
                item("TrashFolder", "T0", "Trash", item("Text", "T1", "Gone"));
    s.files = {{data("A1", "content.rtf"), rtf("Text.\\par ")}, {data("A1", "synopsis.txt"), "They arrive.\n"},
               {data("A1", "notes.rtf"), rtf("Check the tide.\\par ")},
               {data("A1", "content.comments"), R"(<Comments><Comment ID="a"><![CDATA[x]]></Comment>)"
                                                R"(<Comment ID="b" Footnote="Yes"><![CDATA[y]]></Comment></Comments>)"},
               {"Snapshots/A2.snapshots/index.xml", "<SnapshotIndexes/>"}};
    const JsonDoc r = ok(scrivi_import_manuscript(writeScriv(f, "Meta", s).c_str(), ""));
    INFO(r.dump());
    const auto e = scenes(r);
    // A titled first chapter: the first entry is the caret's empty head, so a paste opens the chapter (as S6a's).
    REQUIRE(e.size() == 5);
    CHECK((e[0].text.empty() && e[0].title.empty() && e[0].seed.empty()));
    const auto seed = [](const std::string& json) { auto p = parseJson(json); REQUIRE(p.ok()); return JsonDoc(std::move(p.value())); };
    const JsonDoc revised = seed(e[1].seed);
    CHECK(revised.getString("schema") == "scrivi.scene.v1");
    CHECK(revised.getString("outline") == "They arrive.");
    CHECK(revised.getString("status") == "revised");
    REQUIRE(revised.arraySize("tags") == 2);
    CHECK((revised.arrayItem("tags", 0).dump() == R"("night")" && revised.arrayItem("tags", 1).dump() == R"("harbour")"));
    CHECK(seed(e[2].seed).getString("status") == "final");   // Done
    CHECK(e[3].seed.empty());                                // To Do has no place: draft, reported
    CHECK(e[4].seed.empty());                                // nothing to seed
    CHECK(note(r, "status").getString("example") == "“To Do” on To do");
    CHECK(note(r, "label").getString("example") == "Revised");
    CHECK(note(r, "documentNotes").getString("example") == "Revised");
    CHECK(note(r, "comment").getInt64("count") == 1);
    CHECK(note(r, "footnote").getInt64("count") == 1);
    CHECK(note(r, "snapshots").getString("example") == "Done");
    CHECK(note(r, "notCompiled").getString("example") == "To do");   // imported, and named
    CHECK(e[3].title == "To do");
    CHECK(note(r, "customMetadata").getString("example") == "To do");
    CHECK(note(r, "media").getString("example") == "A picture");
    CHECK(note(r, "outsideDraft").getInt64("count") == 2);
    CHECK(note(r, "outsideDraft").getString("example") == "Notes on ships");
}

TEST_CASE("Manuscript import (Scrivener): refused — not a project, a version it does not read, no Draft (Q1)",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    const auto detail = [](const fs::path& p) {
        const JsonDoc env = envelope(scrivi_import_manuscript(p.c_str(), ""));
        CHECK_FALSE(env.getBool("ok"));
        return env.getSubDoc("error").getString("detail");
    };
    fs::create_directories(f.base / "Empty.scriv");
    CHECK(detail(f.base / "Empty.scriv") == "notScrivener");
    Scriv old;
    old.version = "3.0";
    CHECK(detail(writeScriv(f, "Future", old)) == "notScrivener");
    const fs::path bad = f.base / "Bad.scriv";
    fs::create_directories(bad);
    std::ofstream(bad / "Bad.scrivx") << "<ScrivenerProject Version=\"2.0\"><Binder></Binder></ScrivenerProject>";
    CHECK(detail(bad) == "notScrivener");   // no Draft
    std::ofstream(bad / "Bad.scrivx", std::ios::trunc) << "<ScrivenerProject";
    CHECK(detail(bad) == "notScrivener");   // not XML
    CHECK(detail(f.file("notes.zip", "PK\x03\x04 not a zip")) == "notScrivener");

    // A document past the cap is refused, never read (a sparse file: nothing is written).
    Scriv big;
    big.binder = item("Text", "A1", "Big");
    const fs::path project = writeScriv(f, "Big", big);
    fs::create_directories(project / "Files/Data/A1");
    { std::ofstream(project / "Files/Data/A1/content.rtf"); }
    fs::resize_file(project / "Files/Data/A1/content.rtf", (64u << 20) + 1);
    CHECK(detail(project) == "tooLarge");
}

TEST_CASE("Manuscript import (Scrivener): an ID that is not a UUID or a number is never made into a path",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    std::ofstream(f.base / "secret.rtf") << rtf("Secret.\\par ");   // Escape.scriv/Files/Docs/../../../secret.rtf
    Scriv s;
    s.version = "1.5";   // Files/Docs/<ID>.rtf
    s.files = {{"Files/Docs/1.rtf", rtf("Another.\\par ")}};   // Files/Docs must exist for the path to resolve
    s.binder = R"(<BinderItem ID="../../../secret" Type="Text"><Title>Escape</Title><MetaData><IncludeInCompile>Yes</IncludeInCompile></MetaData></BinderItem>)";
    const JsonDoc r = ok(scrivi_import_manuscript(writeScriv(f, "Escape", s).c_str(), ""));
    const auto e = scenes(r);
    REQUIRE(e.size() == 1);
    CHECK(e[0].title == "Escape");
    CHECK(e[0].text.empty());
}

namespace {

// The every-case fixture, as both layouts must read it.
void checkEveryCase(const JsonDoc& r) {
    INFO(r.dump());
    const auto e = scenes(r);
    REQUIRE(e.size() == 10);   // the first chapter is the Draft's own (untitled): no caret head
    CHECK((e[0].chapterTitle == "" && e[0].title == "Before Anything" && e[0].text == "A note before the first part."));
    CHECK((e[1].chapterTitle == "The Arrival" && e[1].title == "Dock"));
    CHECK(e[1].text == "The ship came in *late* and **nobody** waited.  Café, naïve, “quotes” - and 5 \\* 3 = 15.");
    auto dockParsed = parseJson(e[1].seed);
    REQUIRE(dockParsed.ok());
    const JsonDoc dock = std::move(dockParsed.value());
    CHECK(dock.getString("outline") == "They Arrive");
    REQUIRE(dock.arraySize("tags") == 2);
    CHECK((dock.arrayItem("tags", 0).dump() == R"("harbour")" && dock.arrayItem("tags", 1).dump() == R"("night")"));
    CHECK(dock.getString("status") == "draft");
    CHECK((e[2].title == "Customs" && e[3].title == "Stamp" && e[4].title == "Delay" && e[4].chapter == e[1].chapter));
    CHECK((e[5].chapterTitle == "The Market" && e[5].title == "Stalls"));
    CHECK(e[5].text == "Bread, then fish.\n\n•\t“Apples”\n\n•\t“Pears”");
    CHECK((e[6].chapterTitle == "Empty Chapter" && e[6].text.empty()));
    CHECK((e[7].chapterTitle == "The Return" && e[7].title == "Voyage"));
    CHECK((e[8].title == "Left Out" && e[8].chapter == e[7].chapter));
    CHECK((e[9].chapterTitle == "" && e[9].title == "After Everything" && e[9].text == "The end."));
    CHECK(r.getInt64("chapterCount") == 6);
    CHECK(note(r, "parts").getInt64("count") == 2);
    CHECK(note(r, "status").getString("example") == "“To Do” on Customs");
    CHECK(note(r, "label").getInt64("count") == 2);
    CHECK(note(r, "customMetadata").getString("example") == "Before Anything");
    CHECK(note(r, "documentNotes").getString("example") == "Dock");
    CHECK(note(r, "comment").getString("example") == "Stalls");
    CHECK(note(r, "footnote").getString("example") == "Stalls");
    CHECK(note(r, "picture").getString("example") == "Voyage");
    CHECK(note(r, "docxTable").getString("example") == "Voyage");
    CHECK(note(r, "documentLink").getString("example") == "Voyage");
    // Only the underlined "sea": Dock's italic and bold are in Palatino-Italic and -Bold, which is not formatting.
    CHECK(note(r, "formatting").getInt64("count") == 1);
    CHECK(note(r, "formatting").getString("example") == "The sea was underlined (underline “sea”).");
    CHECK(note(r, "notCompiled").getString("example") == "Left Out");
    CHECK(note(r, "outsideDraft").getInt64("count") == 3);
}

} // namespace

TEST_CASE("Manuscript import (Scrivener): the every-case fixture, Scrivener 3 and its Scrivener 2 export (Q1, Q7a)",
          "[manuscript-import][scrivener][capi]") {
    const JsonDoc v3 = ok(scrivi_import_manuscript(fixture("sp220-every-case.scriv").c_str(), ""));
    checkEveryCase(v3);
    const JsonDoc v2 = ok(scrivi_import_manuscript(fixture("sp220-every-case-v2.scriv").c_str(), ""));
    checkEveryCase(v2);
    CHECK(v2.getSubDoc("range").dump() == v3.getSubDoc("range").dump());   // the same project, either layout
    // A trailing `/`, as a folder picker may give it.
    CHECK(ok(scrivi_import_manuscript((fixture("sp220-every-case.scriv").string() + "/").c_str(), "")).getSubDoc("range").dump() ==
          v3.getSubDoc("range").dump());
}

TEST_CASE("Manuscript import (Scrivener): a zipped backup reads as its folder (Q6)", "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    const fs::path zip = f.base / "sp220-every-case.zip";
    mz_zip_archive z{};
    REQUIRE(mz_zip_writer_init_file(&z, zip.c_str(), 0));
    const fs::path project = fixture("sp220-every-case.scriv");
    for (const auto& entry : fs::recursive_directory_iterator(project)) {
        if (!entry.is_regular_file()) continue;
        const std::string name = "sp220-every-case.scriv/" + fs::relative(entry.path(), project).generic_string();
        REQUIRE(mz_zip_writer_add_file(&z, name.c_str(), entry.path().c_str(), nullptr, 0, MZ_DEFAULT_COMPRESSION));
    }
    const std::string junk = "x";
    REQUIRE(mz_zip_writer_add_mem(&z, "__MACOSX/sp220-every-case.scriv/._sp220-every-case.scrivx", junk.data(), junk.size(), 0));
    REQUIRE(mz_zip_writer_finalize_archive(&z));
    mz_zip_writer_end(&z);
    const JsonDoc r = ok(scrivi_import_manuscript(zip.c_str(), ""));
    checkEveryCase(r);
    CHECK(r.getSubDoc("range").dump() ==
          ok(scrivi_import_manuscript(project.c_str(), "")).getSubDoc("range").dump());
}

namespace {

// Every scene sidecar's title → its outline, tags and status, as written.
std::map<std::string, std::string> sidecars(const fs::path& root) {
    std::map<std::string, std::string> out;
    for (const auto& e : fs::recursive_directory_iterator(root / "manuscript")) {
        const std::string name = e.path().filename().string();
        if (!name.ends_with(".meta.json") || name == "chapter.meta.json" || name == "manuscript.meta.json") continue;
        std::ifstream in(e.path());
        auto parsed = parseJson(std::string(std::istreambuf_iterator<char>(in), {}));
        if (!parsed.ok()) continue;
        const JsonDoc& meta = parsed.value();
        if (meta.getString("schema") != "scrivi.scene.v1") continue;
        std::string tags;
        for (std::size_t i = 0; i < meta.arraySize("tags"); ++i) tags += (tags.empty() ? "" : ",") + meta.arrayItem("tags", i).dump();
        out[meta.getString("title")] = meta.getString("outline") + "|" + tags + "|" + meta.getString("status");
    }
    return out;
}

} // namespace

TEST_CASE("Manuscript import (Scrivener): a new project — titles, chapters and each scene's seed (Q4)",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    const fs::path root = f.base / "Voyage.scrivi";
    const JsonDoc r = f.newProject(fixture("sp220-every-case.scriv"), root);
    CHECK(r.getString("title") == "sp220-every-case");   // the folder's name
    const std::string md = ImportFixture::markdownOf(root);
    INFO(md);
    CHECK(md.starts_with("# sp220-every-case\n\nby T\n\n# Chapter 1\n\nA note before the first part.\n\n# The Arrival\n\nThe ship came in"));
    CHECK(md.ends_with("# Chapter 6\n\nThe end.\n"));
    const auto meta = sidecars(root);
    CHECK(meta.at("Before Anything") == "||draft");   // the project's first scene takes the first document's title
    CHECK(meta.at("Dock") == R"(They Arrive|"harbour","night"|draft)");
    CHECK(meta.at("Customs") == "||draft");
    CHECK(meta.contains("Left Out"));
    ok(scrivi_open_project(root.c_str(), f.appSupport.c_str(), kId));
}

TEST_CASE("Manuscript import (Scrivener): into a project at the caret — seeds kept, cut back as one step",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    const fs::path root = f.base / "Host.scrivi";
    const JsonDoc created = ok(scrivi_create_project(root.c_str(), f.appSupport.c_str(), "Host", "host", kId, kPersona, "T"));
    const JsonDoc first = created.getSubDoc("firstScene");
    const std::string pid = created.getString("projectID"), scene = first.getString("sceneID");
    ok(scrivi_save_scene(pid.c_str(), root.c_str(), f.appSupport.c_str(), scene.c_str(),
                         first.getString("metadataPath").c_str(), first.getString("contentPath").c_str(), "Hello.", 0, 0,
                         0.0, kId, kPersona, "T"));
    const std::string before = ImportFixture::markdownOf(root);
    const JsonDoc r = ok(scrivi_import_manuscript(fixture("sp220-every-case.scriv").c_str(), ""));
    const JsonDoc pasted = ok(scrivi_range_paste(root.c_str(), r.getSubDoc("range").dump().c_str(), scene.c_str(), 6, "Hello.",
                                                 kId, kPersona, "T"));
    const auto meta = sidecars(root);
    CHECK(meta.at("Dock") == R"(They Arrive|"harbour","night"|draft)");
    CHECK(meta.contains("After Everything"));
    CHECK(ImportFixture::markdownOf(root).find("# The Arrival\n\nThe ship came in *late*") != std::string::npos);
    ok(scrivi_range_cut(root.c_str(), pasted.getSubDoc("range").dump().c_str(), kId, kPersona, "T"));
    CHECK(ImportFixture::markdownOf(root) == before);
}

TEST_CASE("Manuscript import (Scrivener): truncated and mangled project files and RTF never crash",
          "[manuscript-import][scrivener][capi]") {
    ImportFixture f;
    const auto slurp = [](const fs::path& p) {
        std::ifstream in(p, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(in), {});
    };
    const fs::path src = fixture("sp220-every-case.scriv");
    const std::string scrivx = slurp(src / "sp220-every-case.scrivx");
    std::string voyage;
    for (const auto& e : fs::directory_iterator(src / "Files/Data")) {
        if (e.path().filename().string().starts_with("78B5697F")) voyage = slurp(e.path() / "content.rtf");
    }
    REQUIRE(voyage.size() > 1000);
    const fs::path work = f.base / "Work.scriv";
    fs::copy(src, work, fs::copy_options::recursive);
    fs::path voyagePath;
    for (const auto& e : fs::directory_iterator(work / "Files/Data")) {
        if (e.path().filename().string().starts_with("78B5697F")) voyagePath = e.path() / "content.rtf";
    }
    const auto run = [&] { (void)envelope(scrivi_import_manuscript(work.c_str(), "")); };   // ok or refused, never a crash
    for (std::size_t cut = 0; cut < scrivx.size(); cut += 211) {
        std::ofstream(work / "sp220-every-case.scrivx", std::ios::trunc | std::ios::binary) << scrivx.substr(0, cut);
        run();
    }
    std::ofstream(work / "sp220-every-case.scrivx", std::ios::trunc | std::ios::binary) << scrivx;
    for (std::size_t cut = 0; cut < voyage.size(); cut += 4099) {
        std::ofstream(voyagePath, std::ios::trunc | std::ios::binary) << voyage.substr(0, cut);
        run();
    }
    for (std::size_t at = 0; at < 20000 && at < voyage.size(); at += 37) {
        std::string bad = voyage;
        bad[at] = static_cast<char>(bad[at] ^ 0x5A);
        std::ofstream(voyagePath, std::ios::trunc | std::ios::binary) << bad;
        run();
    }
    std::ofstream(voyagePath, std::ios::trunc | std::ios::binary) << "{\\rtf1 " + std::string(5000, '{') + "deep" + std::string(5000, '}') + "}";
    run();
}
