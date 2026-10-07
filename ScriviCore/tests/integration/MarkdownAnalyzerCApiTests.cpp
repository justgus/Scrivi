#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <string>
#include <tuple>
#include <utility>
#include <vector>

// EP-048 L1 (SP-165, T-0594) — scrivi_analyze_markdown through its JSON envelope
// (`feedback_boundary_tests_not_facade`). Expectations are what Apple's
// `MarkdownBlocks.analyze` reports for the same block, in UTF-8 bytes; the L2 interop
// test checks the two analyzers against each other over the whole corpus.

using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

JsonDoc analyze(const std::string& block) {
    const char* raw = scrivi_analyze_markdown(block.c_str());
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    JsonDoc env = std::move(parsed.value());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

using Range = std::pair<int64_t, int64_t>;

Range range(const JsonDoc& d) { return {d.getInt64("start"), d.getInt64("end")}; }

std::vector<Range> ranges(const JsonDoc& r, const char* key, const char* sub = nullptr) {
    std::vector<Range> out;
    for (std::size_t i = 0; i < r.arraySize(key); ++i) {
        const auto item = r.arrayItem(key, i);
        out.push_back(sub ? range(item.getSubDoc(sub)) : range(item));
    }
    return out;
}

// [(start, end, bits)] from styleRuns.
std::vector<std::tuple<int64_t, int64_t, int>> styleRuns(const JsonDoc& r) {
    std::vector<std::tuple<int64_t, int64_t, int>> out;
    for (std::size_t i = 0; i < r.arraySize("styleRuns"); ++i) {
        const auto item = r.arrayItem("styleRuns", i);
        const auto [s, e] = range(item.getSubDoc("range"));
        out.emplace_back(s, e, item.getInt("bits"));
    }
    return out;
}

} // namespace

TEST_CASE("analyze_markdown: plain prose reports nothing", "[markdown][capi]") {
    const auto r = analyze("Just a sentence, nothing more.");
    CHECK(r.getInt64("length") == 30);
    CHECK_FALSE(r.contains("headings"));
    CHECK_FALSE(r.contains("markers"));
    CHECK_FALSE(r.contains("styleRuns"));
}

TEST_CASE("analyze_markdown: bold and italic — markers, styles, spans", "[markdown][capi]") {
    // 0         1
    // 0123456789012345678
    // **bold** and *it* x
    const auto r = analyze("**bold** and *it* x");
    CHECK(ranges(r, "markers", "range") == std::vector<Range>{{0, 2}, {6, 8}, {13, 14}, {16, 17}});
    REQUIRE(r.arraySize("markers") == 4);
    CHECK(r.arrayItem("markers", 0).getBool("opens"));
    CHECK_FALSE(r.arrayItem("markers", 1).getBool("opens"));
    CHECK(r.arrayItem("markers", 2).getBool("opens"));
    CHECK_FALSE(r.arrayItem("markers", 3).getBool("opens"));
    // Marker bytes carry the style of the text they enclose.
    CHECK(styleRuns(r) == std::vector<std::tuple<int64_t, int64_t, int>>{{0, 8, 2}, {13, 17, 1}});
    CHECK(ranges(r, "spans") == std::vector<Range>{{0, 8}, {13, 17}});
}

TEST_CASE("analyze_markdown: escaped and spaced stars are literal (the 2 * 3 * 4 class)", "[markdown][capi]") {
    CHECK_FALSE(analyze("2 * 3 * 4").contains("markers"));
    CHECK_FALSE(analyze("2 \\* 3 \\* 4").contains("markers"));
    CHECK_FALSE(analyze("\\*not italic\\*").contains("styleRuns"));
    CHECK_FALSE(analyze("_x_ and \\_y\\_").arraySize("markers") != 2);
}

TEST_CASE("analyze_markdown: ATX headings — line, prefix, level", "[markdown][capi]") {
    const auto r = analyze("## Chapter Two");
    REQUIRE(r.arraySize("headings") == 1);
    const auto h = r.arrayItem("headings", 0);
    CHECK(h.getInt("level") == 2);
    CHECK(range(h.getSubDoc("line")) == Range{0, 14});
    CHECK(range(h.getSubDoc("prefix")) == Range{0, 3});

    // An escaped `#` inside the heading text is text, not prefix.
    const auto e = analyze("# Heading \\#5");
    REQUIRE(e.arraySize("headings") == 1);
    CHECK(range(e.arrayItem("headings", 0).getSubDoc("prefix")) == Range{0, 2});

    // `\#` at the start is not a heading at all.
    CHECK_FALSE(analyze("\\# not a heading").contains("headings"));
    // Seven hashes is not a heading.
    CHECK_FALSE(analyze("####### seven").contains("headings"));
}

TEST_CASE("analyze_markdown: a SETEXT heading has no prefix and is not reported", "[markdown][capi]") {
    CHECK_FALSE(analyze("Title\n=====").contains("headings"));
}

TEST_CASE("analyze_markdown: AC7 — a heading inside a quote or list is not rendered", "[markdown][capi]") {
    CHECK_FALSE(analyze("> # quoted").contains("headings"));
    CHECK_FALSE(analyze("- # in a list").contains("headings"));
}

TEST_CASE("analyze_markdown: AC7 — an indented block is prose, inline only", "[markdown][capi]") {
    // 4 spaces = a code block in CommonMark; demoted, its emphasis still renders at SOURCE offsets.
    const auto r = analyze("    some **bold** text");
    CHECK_FALSE(r.contains("headings"));
    CHECK(ranges(r, "markers", "range") == std::vector<Range>{{9, 11}, {15, 17}});
    CHECK(ranges(r, "spans") == std::vector<Range>{{9, 17}});
    // A tab does the same.
    CHECK(ranges(analyze("\tsome *it*"), "markers", "range") == std::vector<Range>{{6, 7}, {9, 10}});
    // An indented `# x` is NOT a heading after demotion (inline only).
    CHECK_FALSE(analyze("    # not a heading").contains("headings"));
}

TEST_CASE("analyze_markdown: AC7 — a table is drawn exactly as stored", "[markdown][capi]") {
    const auto r = analyze("| a | **b** |\n| - | - |\n| 1 | 2 |");
    CHECK_FALSE(r.contains("markers"));
    CHECK_FALSE(r.contains("styleRuns"));
}

TEST_CASE("analyze_markdown: list items — prefix, ordered, number", "[markdown][capi]") {
    const auto r = analyze("- one\n- two");
    REQUIRE(r.arraySize("listItems") == 2);
    CHECK(range(r.arrayItem("listItems", 0).getSubDoc("prefix")) == Range{0, 2});
    CHECK(range(r.arrayItem("listItems", 1).getSubDoc("line")) == Range{6, 11});
    CHECK_FALSE(r.arrayItem("listItems", 0).getBool("ordered"));

    const auto o = analyze("3. three\n4. four");
    REQUIRE(o.arraySize("listItems") == 2);
    CHECK(o.arrayItem("listItems", 0).getBool("ordered"));
    CHECK(o.arrayItem("listItems", 0).getInt("number") == 3);
    CHECK(o.arrayItem("listItems", 1).getInt("number") == 4);
    CHECK(range(o.arrayItem("listItems", 1).getSubDoc("prefix")) == Range{9, 12});

    // A `* item` bullet is a list item, and its `*` is not an emphasis marker.
    const auto s = analyze("* item");
    CHECK(s.arraySize("listItems") == 1);
    CHECK_FALSE(s.contains("markers"));
}

TEST_CASE("analyze_markdown: an EMPTY list item (what Return writes)", "[markdown][capi]") {
    const auto r = analyze("2. ");
    REQUIRE(r.arraySize("listItems") == 1);
    const auto li = r.arrayItem("listItems", 0);
    CHECK(li.getBool("ordered"));
    CHECK(li.getInt("number") == 2);
    CHECK(range(li.getSubDoc("prefix")) == Range{0, 3});
    CHECK(range(li.getSubDoc("line")) == Range{0, 3});
    CHECK(analyze("- ").arraySize("listItems") == 1);
}

TEST_CASE("analyze_markdown: a link inside bold is bold", "[markdown][capi]") {
    // 0         1         2
    // 012345678901234567890123
    // **see [here](x.md) now**
    const auto r = analyze("**see [here](x.md) now**");
    CHECK(styleRuns(r) == std::vector<std::tuple<int64_t, int64_t, int>>{{0, 24, 2}});
    CHECK(ranges(r, "spans") == std::vector<Range>{{0, 24}});
}

TEST_CASE("analyze_markdown: offsets are UTF-8 BYTES", "[markdown][capi]") {
    // "é" is two bytes, "👋" four: `**` opens at byte 7, not character 4.
    const auto r = analyze("é👋 **b**");
    CHECK(ranges(r, "markers", "range") == std::vector<Range>{{7, 9}, {10, 12}});
    CHECK(r.getInt64("length") == 12);
}

TEST_CASE("analyze_markdown: bold-italic and nesting", "[markdown][capi]") {
    const auto r = analyze("***both***");
    CHECK(styleRuns(r) == std::vector<std::tuple<int64_t, int64_t, int>>{{0, 10, 3}});
    const auto n = analyze("**bold *both* bold**");
    CHECK(n.arraySize("markers") == 4);
    CHECK(ranges(n, "spans") == std::vector<Range>{{0, 20}});
}

TEST_CASE("analyze_markdown: NULL and empty input", "[markdown][capi]") {
    const char* raw = scrivi_analyze_markdown(nullptr);
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().getBool("ok"));
    CHECK(analyze("").getInt64("length") == 0);
}

TEST_CASE("analyze_markdown: an uncovered star with the same style on both sides is not a marker", "[markdown][capi]") {
    // A `* ` bullet is syntax the parser does not cover, but no style changes across it. ⚠️ It needs
    // emphasis in the same block: with none, the analysis ends before the marker rule runs.
    const auto r = analyze("* **x**");
    CHECK(ranges(r, "markers", "range") == std::vector<Range>{{2, 4}, {5, 7}});
    CHECK(r.arraySize("listItems") == 1);
}

TEST_CASE("analyze_markdown: one delimiter run that closes italic and opens bold", "[markdown][capi]") {
    // 0         1
    // 012345678901
    // *it***bold**  — the `***` at 3 ends the italic and starts the bold: it does NOT open.
    const auto r = analyze("*it***bold**");
    REQUIRE(r.arraySize("markers") == 3);
    CHECK(range(r.arrayItem("markers", 1).getSubDoc("range")) == Range{3, 6});
    CHECK_FALSE(r.arrayItem("markers", 1).getBool("opens"));
}

TEST_CASE("analyze_markdown: a soft break stops the side search", "[markdown][capi]") {
    // The `*` after the newline OPENS: what lies before the uncovered newline does not count.
    const auto r = analyze("**a**\n*b*");
    REQUIRE(r.arraySize("markers") == 4);
    CHECK(range(r.arrayItem("markers", 2).getSubDoc("range")) == Range{6, 7});
    CHECK(r.arrayItem("markers", 2).getBool("opens"));
}

TEST_CASE("analyze_markdown: [I-0280] a FENCED block is drawn as stored — no endless demotion", "[markdown][capi]") {
    // Stripping indentation cannot remove a fence, so demoting it again recursed until the stack overflowed.
    for (const char* b : {"~~~\n*x*", "```\nfenced *x*\n```", "  ```\n# h\n- item\n```"}) {
        const auto r = analyze(b);
        CHECK_FALSE(r.contains("markers"));
        CHECK_FALSE(r.contains("headings"));
        CHECK_FALSE(r.contains("listItems"));
    }
}

TEST_CASE("analyze_markdown: an escape backslash inside emphasis takes its mark's style (L2)", "[markdown][capi]") {
    // 0         1
    // 0123456789012
    // **\(and was**  — Apple's source position for `(` starts AT the backslash; md4c leaves it uncovered.
    const auto r = analyze("**\\(and was**");
    CHECK(styleRuns(r) == std::vector<std::tuple<int64_t, int64_t, int>>{{0, 13, 2}});
    // A backslash before a LETTER is literal text (no escape), covered as text already.
    CHECK(styleRuns(analyze("*\\a*")) == std::vector<std::tuple<int64_t, int64_t, int>>{{0, 4, 1}});
}
