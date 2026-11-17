#pragma once
// EP-054 S6a (SP-218, T-0679) — a manuscript written elsewhere, from Markdown or plain text, as a Range to paste.
//
// Rulings (SP-218, user 2026-11-14): Q3 Markdown — `#` opens a chapter, `##`–`######` stay in the text, a thematic break
// block (`* * *`, `***`, `---`, `___`) opens a scene, text before the first `#` is an untitled first chapter; EXACTLY S3's
// title page (`# Title`, an optional subtitle, a `by …` line, then a `#`) is the title, not a chapter. Q4 text — literal (every
// escapable mark escaped, as typing stores it), paragraphs at blank lines, scenes at a line of only `* * *`, `***` or `#`, one
// chapter; UTF-8 (BOM stripped), UTF-16 with a BOM, else Windows-1252; line endings normalised. Q5 a single `\n` stays a line
// break; a hard-wrapped file is reported. Q8 what is kept only as text, or consumed, is reported.
// Empty scenes are left out (the export leaves them out too), except a chapter's only scene, so a chapter heading survives.
// Pure: the caller reads the file.

#include "manuscript/Range.hpp"
#include "scrivi/Result.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::manuscript {

struct ImportNote {
    std::string kind;      // encodingGuessed | hardWrapped | byline | link | image | code | blockQuote | html | table
    std::size_t count = 0;
    std::string example;   // the first line it was found on, shortened
};

struct ImportedManuscript {
    Range range;                 // chapterIDs are "~" placeholders (a paste mints real ones); sceneIDs are empty
    std::string title;           // from the title page; empty when there is none
    std::string subtitle;
    std::string encoding;        // utf-8 | utf-16le | utf-16be | windows-1252
    std::vector<ImportNote> report;
    std::size_t chapterCount = 0;
    bool opensChapter = false;   // the first chapter has its own heading (pasted at a caret, it opens a chapter there)
};

// invalidArgument "notText" when the bytes are not text (a NUL after decoding). `reportMarkup` false: the Markdown was
// written by Scrivi (a .docx's, SP-219), so its own marks are not reported as links, code or HTML.
[[nodiscard]] Result<ImportedManuscript> importManuscript(std::string_view bytes, bool markdown, bool reportMarkup = true);

} // namespace scrivi::manuscript
