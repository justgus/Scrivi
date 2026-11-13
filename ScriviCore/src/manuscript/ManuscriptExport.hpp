#pragma once
// EP-054 S3 (SP-207, T-0666) — the manuscript as Markdown, or as HTML to paste into AO3's submission box.
//
// Rulings (SP-207, user 2026-11-09): Q4 the HTML is what the writer SEES — italic, bold, headings and lists become tags,
// every other character is escaped text, a single newline inside a paragraph is <br />; Q5 scene breaks are `<hr />` /
// `* * *`; Q6 chapter titles follow showChapterTitles (`<h2>` / `#`), the title page only in a whole-manuscript file;
// Q7 <em> / <strong>; Q1 one block per line, so the source reads and edits by hand.
// EP-054 S4 (SP-208, T-0668) — the same manuscript as a Word .docx. Rulings (SP-208, user 2026-11-10): Q1 the writer's
// typeface, size and indent rule in named styles (Body Text, First Paragraph, Heading 1–3, Title, Subtitle, List Bullet /
// Number, Scene Break); Q3 each chapter on a new page; Q4 a centred "* * *"; Q5 chapter titles Heading 1, real Word lists;
// Q6 a single newline a line break; Q7 the title alone in the document properties, and a fixed zip time, so the same
// manuscript gives the same bytes.
// EP-054 AC9 (SP-212, T-0673) — the byline: "by A, B and C" under the title and subtitle (`.docx`: an Author style), and
// the authors as "A; B" in HTML <meta name="author"> and `.docx` dc:creator, on a one-chapter file too (Q2, Q3).
// EP-054 S5 (SP-213, T-0674) — an EPUB 3 of the whole manuscript (Q5–Q7): a title page with the byline, a contents nav,
// one XHTML file per chapter, a stylesheet with the writer's size and indent rule but no face; `mimetype` first and stored.
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
    std::vector<std::string> authors;   // the named, active authors in byline order; empty: no byline, no metadata
    bool titlePage = false;
    bool chapterTitles = true;
    std::vector<ExportScene> scenes;   // manuscript order
};

// The writer's typography (project-settings.json, as ManuscriptTypography reads it).
struct DocxTypography {
    std::string typeface;
    double size = 16;
    std::string indent = "book";   // book | every | none
    double indentEm = 1.5;
};

[[nodiscard]] std::string exportMarkdown(const ExportInput& in);
[[nodiscard]] std::string exportHtml(const ExportInput& in, bool wholePage);
// The .docx file's bytes; empty only when the zip could not be built.
[[nodiscard]] std::string exportDocx(const ExportInput& in, const DocxTypography& type);

// The ePub's package metadata the input does not hold (Q6).
struct EpubMeta {
    std::string identifier;   // urn:uuid:<projectID>
    std::string language;     // BCP 47, from the host
    std::string modified;     // the export time, UTC: 2026-11-12T16:00:00Z
};

// The .epub file's bytes; empty only when the zip could not be built.
[[nodiscard]] std::string exportEpub(const ExportInput& in, const DocxTypography& type, const EpubMeta& meta);

// The print page (EP-054 S5b): an HTML page with the writer's typography, the title page alone on its page and each chapter
// on a new one, for the platforms to paginate into a PDF (WebKit on Apple, QTextDocument on Linux).
[[nodiscard]] std::string exportPrint(const ExportInput& in, const DocxTypography& type);

// One scene's text as HTML blocks, one per line.
[[nodiscard]] std::string sceneHtml(std::string_view text);

} // namespace scrivi::manuscript
