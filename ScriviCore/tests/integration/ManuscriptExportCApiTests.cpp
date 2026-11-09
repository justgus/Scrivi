// EP-054 S3 (SP-207, T-0666) — the manuscript as Markdown and as HTML, through the C ABI only ([I-0113]).
//
// What is proved: the golden Markdown and HTML for a small project (AC2, AC3, AC4) — every paragraph its own <p>, a single
// newline a <br /> (Q4), the five formats as tags, every other character escaped, chapter titles / scene breaks / title page
// (Q5–Q7); one chapter (Q2); the body alone or a whole page (Q3); queued writes land first (AC1); bad options are refused.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <filesystem>
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
