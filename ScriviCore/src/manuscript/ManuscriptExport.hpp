#pragma once
// EP-054 S3 (SP-207, T-0666) — the manuscript as Markdown, or as HTML to paste into AO3's submission box.
//
// Rulings (SP-207, user 2026-11-09): Q4 the HTML is what the writer SEES — italic, bold, headings and lists become tags,
// every other character is escaped text, a single newline inside a paragraph is <br />; Q5 scene breaks are `<hr />` /
// `* * *`; Q6 chapter titles follow showChapterTitles (`<h2>` / `#`), the title page only in a whole-manuscript file;
// Q7 <em> / <strong>; Q1 one block per line, so the source reads and edits by hand.
// Pure: the caller reads the project; nothing here touches a file.

#include <string>
#include <string_view>
#include <vector>

namespace scrivi::manuscript {

struct ExportScene {
    std::string chapterID;
    std::string chapterHeading;   // the title, or "Chapter N" when untitled (as the editor shows it)
    std::string text;             // the scene's Markdown, as stored
};

struct ExportInput {
    std::string title;
    std::string subtitle;
    bool titlePage = false;
    bool chapterTitles = true;
    std::vector<ExportScene> scenes;   // manuscript order
};

[[nodiscard]] std::string exportMarkdown(const ExportInput& in);
[[nodiscard]] std::string exportHtml(const ExportInput& in, bool wholePage);

// One scene's text as HTML blocks, one per line.
[[nodiscard]] std::string sceneHtml(std::string_view text);

} // namespace scrivi::manuscript
