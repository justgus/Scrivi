#pragma once
// SP-174 / T-0601 (EP-048 S3a) — the manuscript's Markdown text model in UTF-16 units, the unit both platforms index text in
// (NSString, QString). The editing logic ported from Apple's Swift (MarkdownEmphasis, ManuscriptCommands, the presenter's
// balancing core) is DEFINED on UTF-16 units; working in them keeps its results byte-for-byte.
//
// The md4c analysis (`MarkdownAnalyzer`, UTF-8 bytes) is mapped here ONCE: every byte of a scalar maps to the scalar's FIRST
// unit; a range END maps to the unit after the scalar, so a range covers both units of a surrogate pair (the rule Linux's
// bridge used before this moved into the core).

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::markdown {

using Units = std::u16string;
using UnitsView = std::u16string_view;

// A range of UTF-16 units — NSRange's shape, so the Swift port reads line for line.
struct URange {
    int location = 0;
    int length = 0;
    [[nodiscard]] int end() const { return location + length; }
    [[nodiscard]] bool contains(int i) const { return i >= location && i < end(); }
    bool operator==(const URange&) const = default;
};

struct Heading16 {
    URange line;     // the heading's line, without its newline
    URange prefix;   // the `#…# ` prefix
    int level = 0;
    bool operator==(const Heading16&) const = default;
};

struct ListItem16 {
    URange line;
    URange prefix;
    bool ordered = false;
    int number = 0;  // the stored number of an ordered item; 0 for a bullet
    bool operator==(const ListItem16&) const = default;
};

struct Marker16 {
    URange range;
    bool opens = false;
    bool operator==(const Marker16&) const = default;
};

inline constexpr std::uint8_t kItalicBit = 1;
inline constexpr std::uint8_t kBoldBit = 2;

// One block's analysis in UTF-16 units (Apple's `MarkdownBlocks.Analysis`).
struct Analysis16 {
    std::vector<Heading16> headings;
    std::vector<ListItem16> listItems;
    std::vector<Marker16> markers;
    std::vector<std::uint8_t> styles;   // per unit; empty when the block has no emphasis
    std::vector<URange> spans;

    [[nodiscard]] std::uint8_t style(int i) const {
        return styles.empty() || i < 0 || i >= static_cast<int>(styles.size()) ? 0 : styles[static_cast<std::size_t>(i)];
    }
};

[[nodiscard]] Units toUnits(std::string_view utf8);
[[nodiscard]] std::string toUtf8(UnitsView units);

// The analysis of ONE block (a maximal run of non-blank lines), in UTF-16 units.
[[nodiscard]] Analysis16 analyzeUnits(UnitsView block);

} // namespace scrivi::markdown
