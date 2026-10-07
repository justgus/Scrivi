#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

// EP-048 L1 (SP-165, T-0594) — the BLOCK ANALYZER in the core, for platforms with no Markdown parser
// of their own (Linux). ✅ It reports what Apple's `MarkdownBlocks.analyze` reports
// (`Scrivi/Views/MarkdownBlocks.swift`) and follows the same rules; md4c replaces `AttributedString`.
// ⚠️ The two must agree — the L2 interop test checks it. A rule changed here must change there.
//
// A BLOCK is a maximal run of non-blank lines of scene text. Every range is a half-open range of
// UTF-8 BYTE offsets relative to the block (the ABI's offset convention, `scrivi.h`).
namespace scrivi::markdown {

struct ByteRange {
    std::size_t start = 0;
    std::size_t end = 0;
    bool operator==(const ByteRange&) const = default;
};

// One ATX heading line (without its newline); `prefix` is the `#…# ` hidden off the line.
struct Heading {
    ByteRange line;
    ByteRange prefix;
    int level = 0;
    bool operator==(const Heading&) const = default;
};

// One list item's first line; its prefix (`- `, `1. `) stays visible, dimmed.
struct ListItem {
    ByteRange line;
    ByteRange prefix;
    bool ordered = false;
    int number = 0;     // the stored number of an ordered item; 0 for a bullet
    bool operator==(const ListItem&) const = default;
};

// An emphasis delimiter run the parser used; `opens` = the caret's home is AFTER it.
struct Marker {
    ByteRange range;
    bool opens = false;
    bool operator==(const Marker&) const = default;
};

inline constexpr std::uint8_t kItalic = 1;
inline constexpr std::uint8_t kBold = 2;

struct Analysis {
    std::vector<Heading> headings;
    std::vector<ListItem> listItems;
    std::vector<Marker> markers;
    // Style bits for every byte of the block; EMPTY when the block has no emphasis.
    std::vector<std::uint8_t> styles;
    // Maximal stretches of emphasis — content and its markers (the span reveal).
    std::vector<ByteRange> spans;
};

[[nodiscard]] Analysis analyze(std::string_view block);

} // namespace scrivi::markdown
