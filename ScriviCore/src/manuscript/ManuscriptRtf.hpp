#pragma once
// EP-054 S7 (SP-220, T-0683) — RTF as Scrivener writes it, read into paragraphs of runs for Scrivi Markdown.
//
// Ruling Q5 (user 2026-11-17): our own small reader in the core. Groups, `\par` / `\line`, `\i` / `\b` and their `0` forms,
// `\uN` with `\ucN`, `\'hh` (Windows-1252), the quote and dash words; the `\*` destinations and the font, colour, style,
// list and info tables are skipped. What Scrivi cannot keep is counted for the report (S6b's Q4 rule): pictures, tables
// (their text kept, cell by cell), footnotes, annotations, links, and formatting — underline, strikethrough, caps,
// super/subscript, colour, centred or right alignment, and faces (font and size) the caller finds uncommon.
// Never fails: input that is not RTF reads as `rtf == false`; truncated or mangled RTF reads as far as it goes. Pure.

#include "manuscript/ManuscriptDocx.hpp"   // Segment

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::manuscript {

struct RtfParagraph {
    std::vector<Segment> runs;
    std::set<std::string> faces;   // "font|size" of each PLAIN run with visible text (italic and bold have their own fonts)
    bool formatted = false;        // formatting Scrivi has no place for, on visible text or the paragraph
    bool table = false;            // a table cell's paragraph
};

struct RtfText {
    bool rtf = false;                                  // the bytes began `{\rtf`
    std::vector<RtfParagraph> paragraphs;              // empty paragraphs left out
    std::map<std::string, std::size_t> faceChars;      // visible bytes per face, for the caller's most common face
    std::size_t pictures = 0, tables = 0, footnotes = 0, annotations = 0;
    std::vector<std::string> links;                    // each field's target: `HYPERLINK "…"`'s quoted text
};

[[nodiscard]] RtfText readRtf(std::string_view rtf);

} // namespace scrivi::manuscript
