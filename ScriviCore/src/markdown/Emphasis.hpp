#pragma once
// SP-174 / T-0601 (EP-048 S3a) — EMPHASIS STAYS BALANCED THROUGH EDITS (EP-046 AC12). A port of Apple's `MarkdownEmphasis.swift`,
// line for line, in UTF-16 units; it replaces that file (both platforms call the core).
//
// The model: a stretch of source becomes TOKENS — every UTF-16 unit EXCEPT emphasis markers, each carrying its style (italic /
// bold bits, from the block's analysis). An edit is applied to tokens, and the stretch is written back with the FEWEST markers
// that give those styles. So a cut that crosses a span edge closes and re-opens the span, a span left empty disappears, and a
// paste of bold text into bold text merges instead of toggling. Only the stretch an edit touches is rewritten (the caller picks
// it); markup elsewhere keeps its bytes (EP-045 AC9).

#include "markdown/MarkdownText.hpp"

#include <cstdint>
#include <vector>

namespace scrivi::markdown::emphasis {

struct Token {
    char16_t unit = 0;
    std::uint8_t style = 0;
    bool isPrefix = false;   // a heading or list prefix unit: kept in the source, dropped from what the writer SEES
    bool operator==(const Token&) const = default;
};

// The blocks of a standalone fragment: maximal runs of non-blank lines.
[[nodiscard]] std::vector<URange> blocks(UnitsView u);

// Tokens of one block [from, to), given its analysis (offsets relative to the block).
[[nodiscard]] std::vector<Token> tokens(UnitsView block, const Analysis16& a, int from, int to);

// Tokens of a STANDALONE fragment (a paste, a typed run): read block by block.
[[nodiscard]] std::vector<Token> tokens(UnitsView fragment);

// True when `source` may carry emphasis markers: an `*` or `_` that is not escaped.
[[nodiscard]] bool mayHaveMarkers(UnitsView source);

// A span begins and ends on a VISIBLE character; whitespace takes the style both neighbours share; a paragraph break ends every
// span; a span edge cannot sit on punctuation glued to a letter.
[[nodiscard]] std::vector<Token> normalize(std::vector<Token> tokens);

struct Serialized {
    Units text;
    std::vector<int> ends;   // ends[i]: the output offset just after token i
};

// Write tokens back with the fewest markers. `open`: styles already open where the output is inserted; `leaveOpen`: styles to
// leave open at the end. Italic is written `*`, never `_`.
[[nodiscard]] Serialized serialize(const std::vector<Token>& tokens, std::uint8_t open = 0, std::uint8_t leaveOpen = 0);

// What the writer SEES of these tokens: no markers, no prefixes, no escape backslashes.
[[nodiscard]] Units presented(const std::vector<Token>& tokens);

// Token lists agree when their units match and every VISIBLE unit has the same style.
[[nodiscard]] bool sameRendering(const std::vector<Token>& a, const std::vector<Token>& b);

[[nodiscard]] bool isSpace(char16_t c);

} // namespace scrivi::markdown::emphasis
