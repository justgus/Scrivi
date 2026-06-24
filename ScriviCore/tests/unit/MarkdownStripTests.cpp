#include <catch2/catch_test_macros.hpp>
#include "util/MarkdownStrip.hpp"

using scrivi::util::stripMarkdown;

TEST_CASE("plain prose is unchanged", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("The quick brown fox") == "The quick brown fox");
}

TEST_CASE("ATX headings lose their hashes", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("# Title") == "Title");
    REQUIRE(stripMarkdown("### Deep heading") == "Deep heading");
    // A run of >6 hashes is not a heading — left as-is text.
    REQUIRE(stripMarkdown("####### not a heading") == "####### not a heading");
}

TEST_CASE("emphasis and strong markers are removed, text kept", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("*italic*") == "italic");
    REQUIRE(stripMarkdown("**bold**") == "bold");
    REQUIRE(stripMarkdown("_under_ and ~~strike~~") == "under and strike");
}

TEST_CASE("inline code keeps its text", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("use the `printf` call") == "use the printf call");
}

TEST_CASE("links keep visible text, drop the URL", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("see [the docs](https://example.com)") == "see the docs");
    REQUIRE(stripMarkdown("![alt text](img.png)") == "alt text");
    REQUIRE(stripMarkdown("a [ref][1] link") == "a ref link");
}

TEST_CASE("blockquote and list markers are removed", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("> quoted line") == "quoted line");
    REQUIRE(stripMarkdown("- bullet") == "bullet");
    REQUIRE(stripMarkdown("* bullet") == "bullet");
    REQUIRE(stripMarkdown("1. first") == "first");
    REQUIRE(stripMarkdown("12) twelfth") == "twelfth");
}

TEST_CASE("horizontal rules become blank, blank runs collapse", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("a\n\n\n\nb") == "a\nb");
    REQUIRE(stripMarkdown("a\n---\nb") == "a\nb");
}

TEST_CASE("fenced code keeps its text but drops the fences", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("```cpp\nint x = 1;\n```") == "int x = 1;");
}

TEST_CASE("CRLF input is normalised", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("line one\r\nline two") == "line one\nline two");
}

TEST_CASE("leading and trailing blank lines are trimmed", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("\n\n# H\n\nbody\n\n") == "H\nbody");
}

TEST_CASE("multi-element manuscript paragraph", "[MarkdownStrip]") {
    const std::string md =
        "# Chapter 1\n"
        "\n"
        "The **silver mines** of *Khaz'tul* ran [deep](url).\n"
        "\n"
        "> A miner's lament\n";
    REQUIRE(stripMarkdown(md) ==
            "Chapter 1\n"
            "The silver mines of Khaz'tul ran deep.\n"
            "A miner's lament");
}

TEST_CASE("empty input yields empty output", "[MarkdownStrip]") {
    REQUIRE(stripMarkdown("").empty());
    REQUIRE(stripMarkdown("\n\n\n").empty());
}
