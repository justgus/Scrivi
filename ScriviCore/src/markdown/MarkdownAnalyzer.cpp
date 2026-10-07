#include "markdown/MarkdownAnalyzer.hpp"

#include <md4c.h>

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>

// Each rule below is ported from `MarkdownBlocks.swift`; the comment names the rule, the Swift file
// says why. ✅ SP-165 spike (md4c 0.5.2): a text callback points INTO the input, so the bytes no text
// callback covers are exactly the syntax — the same covered/uncovered split Apple reads from
// `markdownSourcePosition`. An escaped mark is covered text at its own offset; its backslash is not.
namespace scrivi::markdown {
namespace {

// Apple parses with `.full` (EP-045 AC8): cmark-gfm, so tables and strikethrough are syntax.
constexpr unsigned kFlags = MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH;

bool isSpaceTab(char c) { return c == ' ' || c == '\t'; }
bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isAsciiPunct(char c) {
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
}

struct Walk {
    const char* base = nullptr;
    std::size_t n = 0;
    std::vector<bool> covered;
    std::vector<std::uint8_t> styles;
    std::vector<std::size_t> lineStarts;   // byte offset of each line

    // The open block chain, outermost first.
    struct Open { MD_BLOCKTYPE type; int level; bool ordered; int item; };
    std::vector<Open> chain;
    int em = 0, strong = 0;
    int nextItem = 0;
    std::set<int> seenItems;
    std::map<int, int> levels;       // 0-based line → heading level
    std::map<int, bool> itemLines;   // 0-based line of an item's FIRST text → ordered
    bool codeBlock = false, table = false;

    int lineOf(std::size_t off) const {
        auto it = std::upper_bound(lineStarts.begin(), lineStarts.end(), off);
        return static_cast<int>(it - lineStarts.begin()) - 1;
    }

    void text(const char* s, std::size_t len) {
        if (s < base || s >= base + n) return;          // BR / SOFTBR / NUL: static strings, not source
        auto start = static_cast<std::size_t>(s - base);
        const auto end = std::min(start + len, n);
        if (end <= start) return;
        // An ESCAPED mark: md4c reports the mark and leaves its backslash uncovered; Apple's source
        // position for the run starts AT the backslash. ✅ SP-165 L2: cover it, with the mark's style.
        if (start > 0 && base[start - 1] == '\\' && !covered[start - 1] && isAsciiPunct(base[start])) --start;
        int level = 0;
        bool nested = false, quoted = false;
        std::optional<int> item;
        std::optional<bool> ordered;
        // Innermost → outermost, as Apple walks `presentationIntent.components`.
        for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
            switch (it->type) {
            case MD_BLOCK_H: level = it->level; break;
            case MD_BLOCK_CODE: nested = true; quoted = true; break;
            case MD_BLOCK_TABLE: case MD_BLOCK_THEAD: case MD_BLOCK_TBODY:
            case MD_BLOCK_TR: case MD_BLOCK_TH: case MD_BLOCK_TD: nested = true; break;
            case MD_BLOCK_QUOTE: nested = true; quoted = true; break;
            case MD_BLOCK_LI: nested = true; if (!item) item = it->item; break;
            case MD_BLOCK_OL: nested = true; if (item && !ordered) ordered = true; break;
            case MD_BLOCK_UL: nested = true; if (item && !ordered) ordered = false; break;
            default: break;
            }
        }
        const int line = lineOf(start);
        if (item && !quoted && !seenItems.contains(*item)) {
            seenItems.insert(*item);
            itemLines[line] = ordered.value_or(false);
        }
        // EP-045 AC7: a heading inside a code block, quote, list or table is NOT rendered.
        if (level > 0 && !nested) levels[line] = level;
        std::uint8_t bits = 0;
        if (em > 0) bits |= kItalic;
        if (strong > 0) bits |= kBold;
        for (auto i = start; i < end; ++i) { covered[i] = true; styles[i] = bits; }
    }
};

int enterBlock(MD_BLOCKTYPE type, void* detail, void* user) {
    auto& w = *static_cast<Walk*>(user);
    Walk::Open o{type, 0, false, -1};
    if (type == MD_BLOCK_H) o.level = static_cast<int>(static_cast<MD_BLOCK_H_DETAIL*>(detail)->level);
    if (type == MD_BLOCK_LI) o.item = w.nextItem++;
    if (type == MD_BLOCK_CODE) w.codeBlock = true;
    if (type == MD_BLOCK_TABLE) w.table = true;
    w.chain.push_back(o);
    return 0;
}
int leaveBlock(MD_BLOCKTYPE, void*, void* user) { static_cast<Walk*>(user)->chain.pop_back(); return 0; }
int enterSpan(MD_SPANTYPE type, void*, void* user) {
    auto& w = *static_cast<Walk*>(user);
    if (type == MD_SPAN_EM) ++w.em;
    if (type == MD_SPAN_STRONG) ++w.strong;
    return 0;
}
int leaveSpan(MD_SPANTYPE type, void*, void* user) {
    auto& w = *static_cast<Walk*>(user);
    if (type == MD_SPAN_EM) --w.em;
    if (type == MD_SPAN_STRONG) --w.strong;
    return 0;
}
int onText(MD_TEXTTYPE, const MD_CHAR* s, MD_SIZE len, void* user) {
    static_cast<Walk*>(user)->text(s, len);
    return 0;
}

// The end of line `li` (excluding its newline).
std::size_t lineEnd(const std::vector<std::size_t>& starts, std::size_t li, std::size_t n) {
    return li + 1 < starts.size() ? starts[li + 1] - 1 : n;
}

// `^ {0,3}#{1,6}(?:[ \t]+|$)` on [s, e): the ATX prefix's end, or nullopt.
std::optional<std::size_t> atxPrefix(std::string_view b, std::size_t s, std::size_t e) {
    auto i = s;
    while (i < e && i - s < 3 && b[i] == ' ') ++i;
    const auto h = i;
    while (i < e && b[i] == '#') ++i;
    if (i == h || i - h > 6) return std::nullopt;
    if (i == e) return i;
    if (!isSpaceTab(b[i])) return std::nullopt;
    while (i < e && isSpaceTab(b[i])) ++i;
    return i;
}

struct ListPrefix { std::size_t end; bool hasDigits; int number; };

// `^[ \t]*(?:[-+*]|(\d{1,9})[.)])` on [s, e), then the caller's tail. Returns the marker's end.
std::optional<ListPrefix> listMarker(std::string_view b, std::size_t s, std::size_t e) {
    auto i = s;
    while (i < e && isSpaceTab(b[i])) ++i;
    if (i < e && (b[i] == '-' || b[i] == '+' || b[i] == '*')) return ListPrefix{i + 1, false, 0};
    const auto d = i;
    while (i < e && isDigit(b[i])) ++i;
    if (i == d || i - d > 9 || i >= e || (b[i] != '.' && b[i] != ')')) return std::nullopt;
    return ListPrefix{i + 1, true, std::stoi(std::string(b.substr(d, i - d)))};
}

// True when a line starts (after indentation) like a list item — the fast path must look for it.
bool hasListLine(std::string_view u) {
    std::size_t i = 0;
    const auto n = u.size();
    while (i < n) {
        auto j = i;
        while (j < n && isSpaceTab(u[j])) ++j;
        if (j < n) {
            if ((u[j] == '-' || u[j] == '+') && j + 1 < n && isSpaceTab(u[j + 1])) return true;
            auto k = j;
            while (k < n && isDigit(u[k])) ++k;
            if (k > j && k + 1 < n && (u[k] == '.' || u[k] == ')') && isSpaceTab(u[k + 1])) return true;
        }
        while (i < n && u[i] != '\n') ++i;
        ++i;
    }
    return false;
}

Analysis analyzeImpl(std::string_view block, bool allowHeadings);

// AC7: an indented block is prose. Strip each line's indentation, analyse INLINE only, shift back.
Analysis demoteIndented(std::string_view block) {
    std::string stripped;
    struct Shift { std::size_t dst; std::size_t delta; };
    std::vector<Shift> shifts;
    std::size_t src = 0;
    while (true) {
        const auto nl = block.find('\n', src);
        const auto end = nl == std::string_view::npos ? block.size() : nl;
        auto lead = src;
        while (lead < end && isSpaceTab(block[lead])) ++lead;
        shifts.push_back({stripped.size(), lead - stripped.size()});
        stripped.append(block.substr(lead, end - lead));
        if (nl == std::string_view::npos) break;
        stripped.push_back('\n');
        src = nl + 1;
    }
    const auto inner = analyzeImpl(stripped, false);
    auto map = [&](std::size_t i) {
        std::size_t delta = 0;
        for (const auto& s : shifts) if (s.dst <= i) delta = s.delta;
        return i + delta;
    };
    // A range keeps its length, as Apple's `map(_: NSRange)` does.
    auto mapR = [&](ByteRange r) { const auto s = map(r.start); return ByteRange{s, s + (r.end - r.start)}; };
    Analysis out;
    for (const auto& m : inner.markers) out.markers.push_back({mapR(m.range), m.opens});
    for (const auto& s : inner.spans) out.spans.push_back(mapR(s));
    if (!inner.styles.empty()) {
        out.styles.assign(block.size(), 0);
        for (std::size_t i = 0; i < inner.styles.size(); ++i)
            if (inner.styles[i] != 0) out.styles[map(i)] = inner.styles[i];
    }
    return out;
}

Analysis analyzeImpl(std::string_view block, bool allowHeadings) {
    Analysis out;
    const auto n = block.size();
    // Fast path: no `#`, `*` or `_` and no list line — nothing to render.
    if (block.find_first_of("#*_") == std::string_view::npos && !hasListLine(block)) return out;

    Walk w;
    w.base = block.data();
    w.n = n;
    w.covered.assign(n, false);
    w.styles.assign(n, 0);
    w.lineStarts.push_back(0);
    for (std::size_t i = 0; i < n; ++i) if (block[i] == '\n') w.lineStarts.push_back(i + 1);

    MD_PARSER parser{};
    parser.flags = kFlags;
    parser.enter_block = enterBlock;
    parser.leave_block = leaveBlock;
    parser.enter_span = enterSpan;
    parser.leave_span = leaveSpan;
    parser.text = onText;
    if (md_parse(block.data(), static_cast<MD_SIZE>(n), &parser, &w) != 0) return out;

    // AC7: an INDENTED code block is prose; a TABLE is drawn exactly as stored.
    // ⛔ [I-0280]: demote ONCE. A code block that survives stripping is FENCED — drawn as stored; demoting
    // again recursed until the stack overflowed (on Apple too, in MarkdownBlocks).
    if (w.codeBlock) return allowHeadings ? demoteIndented(block) : out;
    if (w.table) return out;

    const auto& starts = w.lineStarts;
    if (allowHeadings) {
        for (const auto& [li, level] : w.levels) {
            const auto s = starts[li], e = lineEnd(starts, li, n);
            // Only an ATX line: a SETEXT heading has no prefix to hide.
            if (auto p = atxPrefix(block, s, e)) out.headings.push_back({{s, e}, {s, *p}, level});
        }
    }

    // An EMPTY list item (`2. ` alone) gives the parser no text to report: a one-line block that is
    // only a list prefix — `^[ \t]*(?:[-+*]|(\d{1,9})[.)])[ \t]*\n?$`.
    if (allowHeadings && w.itemLines.empty()) {
        auto body = block;
        while (!body.empty() && body.back() == '\n') body.remove_suffix(1);
        while (!body.empty() && body.front() == '\n') body.remove_prefix(1);
        if (body.find('\n') == std::string_view::npos) {
            const auto lineLen = n - (n > 0 && block[n - 1] == '\n' ? 1 : 0);
            if (auto m = listMarker(block, 0, lineLen)) {
                auto i = m->end;
                while (i < lineLen && isSpaceTab(block[i])) ++i;
                if (i == lineLen) {
                    const ByteRange line{0, lineLen};
                    out.listItems.push_back({line, line, m->hasDigits, m->number});
                    return out;
                }
            }
        }
    }
    if (allowHeadings) {
        for (const auto& [li, ordered] : w.itemLines) {
            const auto s = starts[li], e = lineEnd(starts, li, n);
            // `listPrefix`: the marker, then at least one space or tab.
            auto m = listMarker(block, s, e);
            if (!m || m->end >= e || !isSpaceTab(block[m->end])) continue;
            auto p = m->end;
            while (p < e && isSpaceTab(block[p])) ++p;
            out.listItems.push_back({{s, e}, {s, p}, ordered, m->number});
        }
    }

    auto& styles = w.styles;
    if (std::none_of(styles.begin(), styles.end(), [](auto s) { return s != 0; })) return out;

    // The style a position sees on each side: the nearest COVERED byte, skipping uncovered
    // non-whitespace (link syntax, a neighbouring delimiter) but stopping at whitespace/newlines.
    auto side = [&](std::ptrdiff_t from, int step) -> std::uint8_t {
        for (auto i = from; i >= 0 && i < static_cast<std::ptrdiff_t>(n); i += step) {
            if (w.covered[i]) return styles[i];
            const char c = block[i];
            if (c == ' ' || c == '\t' || c == '\n') return 0;
        }
        return 0;
    };
    auto isDelim = [&](std::size_t i) { return !w.covered[i] && (block[i] == '*' || block[i] == '_'); };
    // Delimiter runs: maximal runs of UNCOVERED `*` / `_`; a marker only when the style differs across it.
    std::vector<bool> markerByte(n, false);
    for (std::size_t i = 0; i < n;) {
        if (!isDelim(i)) { ++i; continue; }
        auto j = i;
        while (j < n && isDelim(j)) ++j;
        const auto left = side(static_cast<std::ptrdiff_t>(i) - 1, -1);
        const auto right = side(static_cast<std::ptrdiff_t>(j), 1);
        if (left != right) {
            const bool opens = (right & ~left) != 0 && (left & ~right) == 0;
            out.markers.push_back({{i, j}, opens});
            for (auto k = i; k < j; ++k) { styles[k] = left | right; markerByte[k] = true; }
        }
        i = j;
    }
    // Uncovered bytes that are not markers take the style both sides share (a link inside bold is bold).
    // `side` reads only COVERED bytes, so the bytes this writes never feed it.
    for (std::size_t k = 0; k < n; ++k)
        if (!w.covered[k] && !markerByte[k])
            styles[k] = side(static_cast<std::ptrdiff_t>(k) - 1, -1) & side(static_cast<std::ptrdiff_t>(k) + 1, 1);
    out.styles = styles;
    // Spans: maximal stretches of (marker ∪ styled), each containing a marker.
    for (std::size_t k = 0; k < n;) {
        if (styles[k] == 0 && !markerByte[k]) { ++k; continue; }
        auto j = k;
        bool hasMarker = false;
        while (j < n && (styles[j] != 0 || markerByte[j])) { hasMarker = hasMarker || markerByte[j]; ++j; }
        if (hasMarker) out.spans.push_back({k, j});
        k = j;
    }
    return out;
}

} // namespace

Analysis analyze(std::string_view block) { return analyzeImpl(block, true); }

} // namespace scrivi::markdown
