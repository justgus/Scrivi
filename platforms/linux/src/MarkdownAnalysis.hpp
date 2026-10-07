#pragma once

// EP-048 L3 (SP-166, T-0595) — what one Markdown BLOCK is, as ScriviCore's analyzer reports it
// (`scrivi_analyze_markdown`, bound by `ScriviBridge::analyzeMarkdown`).
// ✅ The Linux shape of Apple's `MarkdownBlocks.Analysis` (`Scrivi/Views/MarkdownBlocks.swift`): the same
// fields, the same meaning. ⚠️ Offsets are UTF-16 code units RELATIVE TO THE BLOCK (QString indices) —
// the bridge converts from the ABI's UTF-8 bytes, as the L2 agreement test does.

#include <QVector>

#include <cstdint>

struct MarkdownRange {
    int start = 0;
    int end = 0;    // half-open
    bool contains(int i) const { return i >= start && i < end; }
    bool operator==(const MarkdownRange&) const = default;
};

struct MarkdownAnalysis {
    struct Heading { MarkdownRange line; MarkdownRange prefix; int level = 0; };
    struct ListItem { MarkdownRange line; MarkdownRange prefix; bool ordered = false; int number = 0; };
    struct Marker { MarkdownRange range; bool opens = false; };

    static constexpr std::uint8_t kItalic = 1;
    static constexpr std::uint8_t kBold = 2;

    QVector<Heading> headings;
    QVector<ListItem> listItems;
    QVector<Marker> markers;
    QVector<std::uint8_t> styles;   // per UTF-16 unit; EMPTY when the block has no emphasis
    QVector<MarkdownRange> spans;   // emphasis + its markers (the span reveal)

    std::uint8_t style(int i) const { return i >= 0 && i < styles.size() ? styles[i] : 0; }
    bool isEmpty() const { return headings.isEmpty() && listItems.isEmpty() && markers.isEmpty() && styles.isEmpty(); }
};
