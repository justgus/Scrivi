#pragma once
// EP-054 S6b (SP-219, T-0682) — a Word document as a manuscript: the .docx is written out as Scrivi Markdown, then read by
// S6a's `importManuscript`, so chapters, scenes, empty scenes and the title page follow exactly the Markdown rules.
//
// Rulings: SP-218 Q7 (user 2026-11-14) — Title / Heading 1 opens a chapter; Heading 2–6 stay as `##`–`######`; a scene break
// is S4's Scene Break paragraph, a `* * *` / `***` line (or a centred `#`), or a page break with no heading after it; italic
// and bold runs → `*` / `**`; numbered and bulleted paragraphs → lists; `<w:br/>` → `\n`; tracked changes read as accepted;
// pictures, tables, footnotes, endnotes, comments, text boxes, headers and footers REPORTED, never dropped silently.
// SP-219 (user 2026-11-15): Q1 S4's own title page (Title, optional Subtitle, then Author or a `by …` line) is the title;
// Q2 found by its bytes, a Word 97–2003 .doc refused; Q3 styles by NAME, following basedOn, else the outline level;
// Q4 formatting Scrivi has no place for kept as plain text, one note.
// Pure: the caller reads the file.

#include "manuscript/ManuscriptImport.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace scrivi::manuscript {

// A run of text with the emphasis Scrivi keeps (shared with the RTF reader, SP-220).
struct Segment {
    std::string text;
    bool bold = false, italic = false;
};
// Runs → one Markdown paragraph, the text kept literal: only what Scrivi would read as markup is escaped.
[[nodiscard]] std::string inlineMarkdown(const std::vector<Segment>& runs);

// A zip, by its first bytes (the caller then reads it as .docx).
[[nodiscard]] bool looksLikeZip(std::string_view bytes);
// Word 97–2003: an OLE compound file.
[[nodiscard]] bool looksLikeLegacyDoc(std::string_view bytes);

// invalidArgument "notText" when it is not a Word document (no word/document.xml, or XML that does not parse);
// "tooLarge" when a part is past the cap.
[[nodiscard]] Result<ImportedManuscript> importDocx(std::string_view bytes);

// The Markdown the document reads as, with the notes only a .docx has (exposed for the tests).
struct DocxMarkdown {
    std::string markdown;
    std::vector<ImportNote> notes;
};
[[nodiscard]] Result<DocxMarkdown> docxToMarkdown(std::string_view bytes);

} // namespace scrivi::manuscript
