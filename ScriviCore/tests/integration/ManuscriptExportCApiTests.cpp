// EP-054 S3 (SP-207, T-0666) — the manuscript as Markdown and as HTML, through the C ABI only ([I-0113]).
//
// What is proved: the golden Markdown and HTML for a small project (AC2, AC3, AC4) — every paragraph its own <p>, a single
// newline a <br /> (Q4), the five formats as tags, every other character escaped, chapter titles / scene breaks / title page
// (Q5–Q7); one chapter (Q2); the body alone or a whole page (Q3); queued writes land first (AC1); bad options are refused.
// S4 (SP-208, T-0668): the .docx unzipped — the golden document.xml, the writer's typography in the styles (Q1), page and
// scene breaks (Q3, Q4), real lists (Q5), a newline a line break (Q6), the same bytes twice (Q7), nothing left on failure.
// AC9 (SP-212, T-0673): the byline under the title and the authors in the metadata, in every format; retired and unnamed
// authors never; a work with no named author exactly as before (the fixture's only author is "Scrivi", so every golden
// above the AC9 cases is that case).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <miniz.h>

#include <atomic>
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

bool failed(const char* raw) { return !envelope(raw).getBool("ok"); }

const char* const kId = "identity-001";
const char* const kPersona = "persona-001";

// Chapter 1 "The Three Presents": scene a1 (every format, every literal) and a2. Chapter 2, untitled: scene b1.
const std::string kA1 = "# Heading One\n\n"
                        "He said *hello* and **left**.\nA second line.\n\n"
                        "- one\n- two\n\n"
                        R"(A literal <3 & [x](y) and `tick` and \*not italic\*.)" "\n";
const std::string kA2 = "Second scene.";
const std::string kB1 = "1. first\n2. second\n\n";

struct ExportFixture {
    fs::path base, root, appSupport;
    std::string projectID, chapter2;

    ExportFixture() {
        static std::atomic<int> counter{0};
        base = fs::temp_directory_path() / ("scrivi-mexport-" + std::to_string(::getpid()) + "-" +
                                            std::to_string(counter.fetch_add(1)));
        fs::remove_all(base);
        root = base / "Dumas.scrivi";
        appSupport = base / "support";
        fs::create_directories(appSupport);
        const JsonDoc created = ok(scrivi_create_project(r(), as(), "The Three Musketeers", "dumas", kId, kPersona, "T"));
        projectID = created.getString("projectID");
        const JsonDoc first = created.getSubDoc("firstScene");
        const std::string a1 = first.getString("sceneID");
        const JsonDoc opened = ok(scrivi_open_project(r(), as(), kId));
        const std::string chapter1 = opened.arrayItem("scenes", 0).getString("chapterID");
        ok(scrivi_rename_chapter(r(), opened.arrayItem("scenes", 0).getString("chapterMetadataPath").c_str(),
                                 "The Three Presents"));
        save(a1, first.getString("metadataPath"), first.getString("contentPath"), kA1);
        const JsonDoc a2 = ok(scrivi_create_scene(r(), as(), pid(), chapter1.c_str(), a1.c_str(), "", kId, kPersona, "T"));
        save(a2.getString("sceneID"), a2.getString("metadataPath"), a2.getString("contentPath"), kA2);
        const JsonDoc c2 = ok(scrivi_create_chapter(r(), as(), pid(), kId, kPersona, "T", chapter1.c_str()));
        chapter2 = c2.getString("chapterID");
        save(c2.getString("firstSceneID"), c2.getString("firstSceneMetadataPath"), c2.getString("firstSceneContentPath"), kB1);
        ok(scrivi_put_project_settings(r(), R"({"subtitle":"A Romance"})"));
        authors(R"([{"displayName":"Scrivi"}])");   // unnamed: no byline (SP-212 Q1)
    }
    ~ExportFixture() {
        scrivi_free(scrivi_close_project(r()));
        std::error_code ec;
        fs::remove_all(base, ec);
    }
    [[nodiscard]] const char* r() const { return root.c_str(); }
    [[nodiscard]] const char* as() const { return appSupport.c_str(); }
    [[nodiscard]] const char* pid() const { return projectID.c_str(); }

    void save(const std::string& scene, const std::string& meta, const std::string& content, const std::string& text) {
        ok(scrivi_save_scene(pid(), r(), as(), scene.c_str(), meta.c_str(), content.c_str(), text.c_str(), 0, 0, 0.0, kId,
                             kPersona, "T"));
    }
    void authors(const std::string& list) { ok(scrivi_set_authors(r(), (R"({"authors":)" + list + "}").c_str())); }
    JsonDoc exportWith(const std::string& options) { return ok(scrivi_export_manuscript(r(), options.c_str())); }
};

const std::string kHtmlBody = "<h2>The Three Presents</h2>\n"
                              "<h1>Heading One</h1>\n"
                              "<p>He said <em>hello</em> and <strong>left</strong>.<br />A second line.</p>\n"
                              "<ul>\n<li>one</li>\n<li>two</li>\n</ul>\n"
                              "<p>A literal &lt;3 &amp; [x](y) and `tick` and *not italic*.</p>\n"
                              "<hr />\n"
                              "<p>Second scene.</p>\n"
                              "<h2>Chapter 2</h2>\n"
                              "<ol>\n<li>first</li>\n<li>second</li>\n</ol>\n";

// One part of a .docx, unzipped.
std::string docxPart(const fs::path& file, const char* name) {
    mz_zip_archive z{};
    REQUIRE(mz_zip_reader_init_file(&z, file.c_str(), 0));
    std::size_t n = 0;
    void* p = mz_zip_reader_extract_file_to_heap(&z, name, &n, 0);
    std::string out = p ? std::string(static_cast<const char*>(p), n) : std::string{};
    mz_free(p);
    mz_zip_reader_end(&z);
    return out;
}

std::string bytesOf(const fs::path& file) {
    std::ifstream in(file, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

// document.xml's pieces: a paragraph in `style` (with pPr extras) holding `runs`, and a run.
std::string para(const std::string& style, const std::string& runs, const std::string& extra = "") {
    return "<w:p><w:pPr><w:pStyle w:val=\"" + style + "\"/>" + extra + "</w:pPr>" + runs + "</w:p>\n";
}
std::string run(const std::string& text, const std::string& rPr = "") {
    return "<w:r>" + (rPr.empty() ? "" : "<w:rPr>" + rPr + "</w:rPr>") + "<w:t xml:space=\"preserve\">" + text + "</w:t></w:r>";
}
const std::string kBr = "<w:r><w:br/></w:r>";
const std::string kPageBreak = "<w:pageBreakBefore/>";
std::string num(int id) { return "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"" + std::to_string(id) + "\"/></w:numPr>"; }
std::string document(const std::string& body) {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<w:document "
           "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>\n" + body +
           "</w:body></w:document>\n";
}

} // namespace

TEST_CASE("Manuscript export: Markdown is the scenes as stored, with titles and breaks (AC3, Q5, Q6)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    const JsonDoc md = f.exportWith(R"({"format":"markdown"})");
    CHECK(md.getString("text") == "# The Three Musketeers\n\nA Romance\n\n"
                                  "# The Three Presents\n\n" + kA1 + "\n* * *\n\n" + kA2 + "\n\n"
                                  "# Chapter 2\n\n1. first\n2. second\n");
    CHECK(md.getInt64("chapterCount") == 2);
    CHECK(md.getInt64("sceneCount") == 3);
}

TEST_CASE("Manuscript export: HTML is what the writer sees, a whole page or the body alone (AC2, Q3, Q4, Q7)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    CHECK(f.exportWith(R"({"format":"html","document":"page"})").getString("text") ==
          "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>The Three Musketeers</title>\n</head>\n<body>\n"
          "<h1>The Three Musketeers</h1>\n<p>A Romance</p>\n" + kHtmlBody + "</body>\n</html>\n");
    // Copy as HTML: the body alone, no title page (Q6).
    CHECK(f.exportWith(R"({"format":"html","document":"body"})").getString("text") == kHtmlBody);
}

TEST_CASE("Manuscript export: one chapter (Q2)", "[manuscript-export][capi]") {
    ExportFixture f;
    const JsonDoc html = f.exportWith(R"({"format":"html","document":"body","scope":"chapter","chapterID":")" + f.chapter2 +
                                      "\"}");
    CHECK(html.getString("text") == "<h2>Chapter 2</h2>\n<ol>\n<li>first</li>\n<li>second</li>\n</ol>\n");
    CHECK(html.getInt64("chapterCount") == 1);
    CHECK(html.getInt64("sceneCount") == 1);
    const JsonDoc md = f.exportWith(R"({"format":"markdown","scope":"chapter","chapterID":")" + f.chapter2 + "\"}");
    CHECK(md.getString("text") == "# Chapter 2\n\n1. first\n2. second\n");
}

TEST_CASE("Manuscript export: chapter titles hidden — a chapter boundary is a scene break (Q5, Q6)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    ok(scrivi_put_project_settings(f.r(), R"({"showChapterTitles":false})"));
    const std::string text = f.exportWith(R"({"format":"html","document":"body"})").getString("text");
    CHECK(text.find("<h2>") == std::string::npos);
    CHECK(text.find("<p>Second scene.</p>\n<hr />\n<ol>") != std::string::npos);
}

TEST_CASE("Manuscript export: inline edge cases (Q4)", "[manuscript-export][capi]") {
    ExportFixture f;
    const JsonDoc opened = ok(scrivi_open_project(f.r(), f.as(), kId));
    const JsonDoc b1 = opened.arrayItem("scenes", 2);
    // Bold around italic, a hard-break backslash, an escaped backslash, an ordered list from 3, a paragraph after a list item.
    f.save(b1.getString("sceneID"), b1.getString("metadataPath"), b1.getString("contentPath"),
           "**bold *both* bold**\\\nnext\n\nends in \\\\\n\n3. three\n4. four\nlazy");
    const std::string text = f.exportWith(R"({"format":"html","document":"body","scope":"chapter","chapterID":")" +
                                          f.chapter2 + "\"}").getString("text");
    CHECK(text == "<h2>Chapter 2</h2>\n"
                  "<p><strong>bold <em>both</em> bold</strong><br />next</p>\n"
                  "<p>ends in \\</p>\n"
                  "<ol start=\"3\">\n<li>three</li>\n<li>four<br />lazy</li>\n</ol>\n");
}

TEST_CASE("Manuscript export: queued writes land before the export reads (AC1)", "[manuscript-export][capi]") {
    ExportFixture f;
    const JsonDoc opened = ok(scrivi_open_project(f.r(), f.as(), kId));
    const JsonDoc b1 = opened.arrayItem("scenes", 2);
    JsonDoc scene;
    scene.setString("sceneID", b1.getString("sceneID"));
    scene.setString("metadataPath", b1.getString("metadataPath"));
    scene.setString("contentPath", b1.getString("contentPath"));
    scene.setString("markdown", "Queued.");
    JsonDoc job;
    job.setString("projectID", f.projectID);
    job.setString("appSupportRoot", f.as());
    job.setString("identityID", kId);
    job.setString("personaID", kPersona);
    job.setString("displayName", "T");
    job.appendToArray("scenes", std::move(scene));
    ok(scrivi_enqueue_save_scenes(f.r(), job.dump(-1).c_str()));
    CHECK(f.exportWith(R"({"format":"markdown","scope":"chapter","chapterID":")" + f.chapter2 + "\"}").getString("text") ==
          "# Chapter 2\n\nQueued.\n");
}

TEST_CASE("Manuscript export: bad options are refused", "[manuscript-export][capi]") {
    ExportFixture f;
    CHECK(failed(scrivi_export_manuscript(f.r(), R"({"format":"docx"})")));
    CHECK(failed(scrivi_export_manuscript(f.r(), R"({"format":"html","document":"poster"})")));
    CHECK(failed(scrivi_export_manuscript(f.r(), R"({"format":"markdown","scope":"chapter","chapterID":"nope"})")));
    CHECK(failed(scrivi_export_manuscript(f.r(), "not json")));
    CHECK(failed(scrivi_export_manuscript("", R"({"format":"markdown"})")));
}

TEST_CASE("Manuscript export: .docx is what the writer sees, in named styles (AC1, AC2, Q1, Q3-Q6)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    const auto file = f.base / "Dumas.docx";
    const JsonDoc r = f.exportWith(R"({"format":"docx","defaultTypeface":"Literata","destPath":")" + file.string() + "\"}");
    CHECK(r.getString("path") == file.string());
    CHECK(r.getInt64("chapterCount") == 2);
    CHECK(r.getInt64("sceneCount") == 3);
    CHECK_FALSE(r.contains("text"));
    CHECK(docxPart(file, "word/document.xml") ==
          document(para("Title", run("The Three Musketeers")) +
                   para("Subtitle", run("A Romance")) +
                   para("Heading1", run("The Three Presents"), kPageBreak) +
                   para("Heading1", run("Heading One")) +
                   para("FirstParagraph", run("He said ") + run("hello", "<w:i/>") + run(" and ") + run("left", "<w:b/>") +
                                              run(".") + kBr + run("A second line.")) +
                   para("ListBullet", run("one"), num(1)) +
                   para("ListBullet", run("two"), num(1)) +
                   para("FirstParagraph", run("A literal &lt;3 &amp; [x](y) and `tick` and *not italic*.")) +
                   para("SceneBreak", "") +
                   para("FirstParagraph", run("Second scene.")) +
                   para("Heading1", run("Chapter 2"), kPageBreak) +
                   para("ListNumber", run("first"), num(2)) +
                   para("ListNumber", run("second"), num(2))));
    // Q1: the project's settings are absent, so the app's face and Scrivi's defaults — 16 pt, book indent of 1.5 em — on the
    // screen's geometry: an exact 1.45 line (464 twips) and a third of one between paragraphs (155).
    const auto styles = docxPart(file, "word/styles.xml");
    CHECK(styles.find(R"(<w:rFonts w:ascii="Literata")") != std::string::npos);
    CHECK(styles.find(R"(<w:sz w:val="32"/>)") != std::string::npos);
    CHECK(styles.find(R"(<w:spacing w:after="0" w:line="464" w:lineRule="exact"/>)") != std::string::npos);
    CHECK(styles.find(R"(<w:name w:val="Body Text"/><w:basedOn w:val="Normal"/><w:qFormat/><w:pPr><w:spacing w:after="155"/>)"
                      R"(<w:ind w:firstLine="480"/>)") != std::string::npos);
    CHECK(docxPart(file, "docProps/core.xml").find("<dc:title>The Three Musketeers</dc:title>") != std::string::npos);
    CHECK(docxPart(file, "docProps/core.xml").find("dc:creator") == std::string::npos);   // AC9: no named author
    CHECK(styles.find("Author") == std::string::npos);
    CHECK(docxPart(file, "word/numbering.xml").find(R"(<w:num w:numId="2"><w:abstractNumId w:val="1"/>)") !=
          std::string::npos);
    CHECK_FALSE(fs::exists(file.string() + ".tmp"));
}

TEST_CASE("Manuscript export: .docx — the book rule, a list from 3, one chapter, no title page (Q1, Q2, Q5)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    const JsonDoc opened = ok(scrivi_open_project(f.r(), f.as(), kId));
    const JsonDoc b1 = opened.arrayItem("scenes", 2);
    f.save(b1.getString("sceneID"), b1.getString("metadataPath"), b1.getString("contentPath"),
           "**bold *both* bold**\\\nnext\n\nends in \\\\\tTab\n\n3. three\n4. four\nlazy");
    const auto file = f.base / "Chapter 2.docx";
    f.exportWith(R"({"format":"docx","scope":"chapter","chapterID":")" + f.chapter2 + R"(","destPath":")" + file.string() + "\"}");
    CHECK(docxPart(file, "word/document.xml") ==
          document(para("Heading1", run("Chapter 2")) +
                   para("FirstParagraph", run("bold ", "<w:b/>") + run("both", "<w:b/><w:i/>") + run(" bold", "<w:b/>") + kBr +
                                              run("next")) +
                   para("BodyText", "<w:r><w:t xml:space=\"preserve\">ends in \\</w:t><w:tab/>"
                                    "<w:t xml:space=\"preserve\">Tab</w:t></w:r>") +
                   para("ListNumber", run("three"), num(2)) +
                   para("ListNumber", run("four") + kBr + run("lazy"), num(2))));
    CHECK(docxPart(file, "word/numbering.xml").find(R"(<w:startOverride w:val="3"/>)") != std::string::npos);
}

TEST_CASE("Manuscript export: .docx — the writer's typography, titles hidden (Q1, Q3)", "[manuscript-export][capi]") {
    ExportFixture f;
    ok(scrivi_put_project_settings(f.r(), R"({"subtitle":"A Romance","showChapterTitles":false,"typeface":"Atkinson Hyperlegible",)"
                                          R"("textSize":12,"paragraphIndent":"none","indentEm":2})"));
    const auto file = f.base / "Dumas.docx";
    f.exportWith(R"({"format":"docx","defaultTypeface":"Literata","destPath":")" + file.string() + "\"}");
    const auto doc = docxPart(file, "word/document.xml");
    CHECK(doc.find("The Three Presents") == std::string::npos);
    CHECK(doc.find("Chapter 2") == std::string::npos);
    // Each chapter still starts on a new page — the first one after the title page; scenes still break with the rule.
    CHECK(doc.find(para("Subtitle", run("A Romance")) + para("Heading1", run("Heading One"), kPageBreak)) != std::string::npos);
    CHECK(doc.find(para("SceneBreak", "") + para("BodyText", run("Second scene."))) != std::string::npos);
    CHECK(doc.find(para("ListNumber", run("first"), kPageBreak + num(2))) != std::string::npos);
    const auto styles = docxPart(file, "word/styles.xml");
    CHECK(styles.find(R"(w:ascii="Atkinson Hyperlegible")") != std::string::npos);
    CHECK(styles.find(R"(<w:sz w:val="24"/>)") != std::string::npos);
    CHECK(styles.find(R"(<w:spacing w:after="348"/><w:ind w:firstLine="0"/>)") != std::string::npos);   // a whole 1.45 line
}

TEST_CASE("Manuscript export: .docx — the same bytes twice; a failed write leaves nothing (AC1, Q7)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    const auto a = f.base / "a.docx", b = f.base / "b.docx";
    f.exportWith(R"({"format":"docx","destPath":")" + a.string() + "\"}");
    f.exportWith(R"({"format":"docx","destPath":")" + b.string() + "\"}");
    CHECK(bytesOf(a) == bytesOf(b));
    CHECK(bytesOf(a).starts_with("PK"));
    const auto nowhere = f.base / "no-such-folder" / "x.docx";
    CHECK(failed(scrivi_export_manuscript(f.r(), (R"({"format":"docx","destPath":")" + nowhere.string() + "\"}").c_str())));
    CHECK_FALSE(fs::exists(nowhere.parent_path()));
}

TEST_CASE("Manuscript export: the byline and the authors, in every format (AC9, SP-212 Q3, Q4)", "[manuscript-export][capi]") {
    ExportFixture f;
    f.authors(R"([{"displayName":"Alexandre Dumas"},{"displayName":"Auguste Maquet"}])");
    CHECK(f.exportWith(R"({"format":"markdown"})").getString("text")
              .starts_with("# The Three Musketeers\n\nA Romance\n\nby Alexandre Dumas and Auguste Maquet\n\n# The Three Presents\n"));
    CHECK(f.exportWith(R"({"format":"html","document":"page"})").getString("text") ==
          "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n"
          "<meta name=\"author\" content=\"Alexandre Dumas; Auguste Maquet\">\n<title>The Three Musketeers</title>\n</head>\n"
          "<body>\n<h1>The Three Musketeers</h1>\n<p>A Romance</p>\n<p>by Alexandre Dumas and Auguste Maquet</p>\n" + kHtmlBody +
          "</body>\n</html>\n");
    CHECK(f.exportWith(R"({"format":"html","document":"body"})").getString("text") == kHtmlBody);   // Copy as HTML: none
    const auto file = f.base / "Dumas.docx";
    f.exportWith(R"({"format":"docx","destPath":")" + file.string() + "\"}");
    CHECK(docxPart(file, "word/document.xml").find(para("Subtitle", run("A Romance")) +
                                                   para("Author", run("by Alexandre Dumas and Auguste Maquet")) +
                                                   para("Heading1", run("The Three Presents"), kPageBreak)) != std::string::npos);
    // Q4: centred on the Subtitle's line (16 pt × 1.5 heading scale × 1.25 = 600 twips), the body size, not bold.
    CHECK(docxPart(file, "word/styles.xml").find(
              R"(<w:style w:type="paragraph" w:customStyle="1" w:styleId="Author"><w:name w:val="Author"/><w:basedOn w:val="Normal"/>)"
              R"(<w:qFormat/><w:pPr><w:spacing w:line=")") != std::string::npos);
    CHECK(docxPart(file, "docProps/core.xml").find("<dc:creator>Alexandre Dumas; Auguste Maquet</dc:creator>") !=
          std::string::npos);
}

TEST_CASE("Manuscript export: the byline — one, three, retired and unnamed authors left out (AC9, Q1)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    auto titlePage = [&] {
        const auto text = f.exportWith(R"({"format":"markdown"})").getString("text");
        return text.substr(0, text.find("# The Three Presents"));
    };
    f.authors(R"([{"displayName":"Dumas"}])");
    CHECK(titlePage() == "# The Three Musketeers\n\nA Romance\n\nby Dumas\n\n");
    f.authors(R"([{"displayName":"Dumas"},{"displayName":"Maquet"},{"displayName":"Scrivi"},{"displayName":"Hugo"}])");
    CHECK(titlePage() == "# The Three Musketeers\n\nA Romance\n\nby Dumas, Maquet and Hugo\n\n");
    // A list without IDs retires every earlier author (never deletes): the four above are on disk, retired, and left out.
    f.authors(R"([{"displayName":"Hugo"},{"displayName":"Dumas"}])");
    CHECK(titlePage() == "# The Three Musketeers\n\nA Romance\n\nby Hugo and Dumas\n\n");
}

TEST_CASE("Manuscript export: a one-chapter file — no byline, the authors in its metadata (AC9, Q2)",
          "[manuscript-export][capi]") {
    ExportFixture f;
    f.authors(R"([{"displayName":"Dumas"},{"displayName":"Maquet"}])");
    const std::string chapter = R"(","scope":"chapter","chapterID":")" + f.chapter2;
    CHECK(f.exportWith(R"({"format":"markdown)" + chapter + "\"}").getString("text") == "# Chapter 2\n\n1. first\n2. second\n");
    const auto page = f.exportWith(R"({"format":"html","document":"page)" + chapter + "\"}").getString("text");
    CHECK(page.find("<meta name=\"author\" content=\"Dumas; Maquet\">") != std::string::npos);
    CHECK(page.find("by Dumas") == std::string::npos);
    const auto file = f.base / "Chapter 2.docx";
    f.exportWith(R"({"format":"docx)" + chapter + R"(","destPath":")" + file.string() + "\"}");
    CHECK(docxPart(file, "word/document.xml").find("Author") == std::string::npos);
    CHECK(docxPart(file, "docProps/core.xml").find("<dc:creator>Dumas; Maquet</dc:creator>") != std::string::npos);
}
