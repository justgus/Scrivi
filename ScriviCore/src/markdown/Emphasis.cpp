#include "markdown/Emphasis.hpp"

#include "markdown/Escapes.hpp"
#include "markdown/UnicodeClasses.hpp"

#include <algorithm>
#include <set>

namespace scrivi::markdown::emphasis {

namespace {

bool isPunct(char16_t c) {
    return (c >= 0x21 && c <= 0x2F) || (c >= 0x3A && c <= 0x40) || (c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7E)
           || (c >= 0x2010 && c <= 0x205E);   // general punctuation (dashes, quotes, …)
}
bool isHigh(char16_t c) { return c >= 0xD800 && c <= 0xDBFF; }
bool isLow(char16_t c) { return c >= 0xDC00 && c <= 0xDFFF; }

// One character of the token stream: where it starts, how many units, and whether it is punctuation / a word character.
// ✅ A SUPPLEMENTARY-plane scalar (an emoji) is classified as md4c does — P* or S* is punctuation, so `x**👋` cannot open
// (SP-174). Apple's own code saw two "letter" units and wrote `x**👋`, which Apple's parser renders but md4c does not.
struct Char { std::size_t start = 0, count = 1; bool punct = false, word = false; };
Char charAt(const std::vector<Token>& t, std::size_t start, std::size_t count) {
    Char c{start, count};
    if (count == 2) {
        const char32_t cp = 0x10000 + ((char32_t(t[start].unit) - 0xD800) << 10) + (char32_t(t[start + 1].unit) - 0xDC00);
        c.punct = unicode::isPunctuationOrSymbolScalar(cp);
    } else {
        c.punct = isPunct(t[start].unit);
    }
    c.word = !c.punct && !isSpace(t[start].unit);
    return c;
}
// The character that STARTS at k / ENDS just before k (a surrogate pair is one character).
Char charFrom(const std::vector<Token>& t, std::size_t k) {
    return charAt(t, k, k + 1 < t.size() && isHigh(t[k].unit) && isLow(t[k + 1].unit) ? 2 : 1);
}
Char charBefore(const std::vector<Token>& t, std::size_t k) {
    return k >= 2 && isLow(t[k - 1].unit) && isHigh(t[k - 2].unit) ? charAt(t, k - 2, 2) : charAt(t, k - 1, 1);
}

// Whitespace takes the style both neighbours share; a PARAGRAPH break (two newlines) ends every span, while a single newline
// (a soft or hard line break) may sit inside one. Returns whether anything changed.
bool whitespacePass(std::vector<Token>& t) {
    bool changed = false;
    std::size_t i = 0;
    while (i < t.size()) {
        if (!isSpace(t[i].unit)) { ++i; continue; }
        std::size_t j = i;
        while (j < t.size() && isSpace(t[j].unit)) { ++j; }
        const std::uint8_t left = i > 0 ? t[i - 1].style : 0;
        const std::uint8_t right = j < t.size() ? t[j].style : 0;
        int newlines = 0;
        for (std::size_t k = i; k < j; ++k) { if (t[k].unit == u'\n') { ++newlines; } }
        const std::uint8_t style = newlines >= 2 ? 0 : static_cast<std::uint8_t>(left & right);
        for (std::size_t k = i; k < j; ++k) {
            if (t[k].style != style) { t[k].style = style; changed = true; }
        }
        i = j;
    }
    return changed;
}

// CommonMark cannot open a span on PUNCTUATION that follows a letter (`x*,*` is not emphasis), nor close one on punctuation
// that precedes a letter. There the span edge moves past that one punctuation mark (with its escape backslash), which loses its
// style — the only representable result.
bool punctuationPass(std::vector<Token>& t) {
    bool changed = false;
    for (std::size_t k = 1; k < t.size(); ++k) {
        const Token before = t[k - 1], here = t[k];
        const auto opening = static_cast<std::uint8_t>(here.style & ~before.style);
        const auto closing = static_cast<std::uint8_t>(before.style & ~here.style);
        if ((opening | closing) == 0) { continue; }
        const Char prev = charBefore(t, k), next = charFrom(t, k);
        if (opening != 0 && prev.word && next.punct) {
            const std::size_t e = std::min(k + (here.unit == u'\\' && k + 1 < t.size() ? 2 : next.count), t.size());
            for (std::size_t m = k; m < e; ++m) { t[m].style = static_cast<std::uint8_t>(t[m].style & ~opening); }
            changed = true;
        }
        if (closing != 0 && prev.punct && next.word) {
            std::size_t s = prev.start;
            if (s > 0 && t[s - 1].unit == u'\\') { --s; }
            for (std::size_t m = s; m < k; ++m) { t[m].style = static_cast<std::uint8_t>(t[m].style & ~closing); }
            changed = true;
        }
    }
    return changed;
}

constexpr std::u16string_view kBoldMarker = u"**";
constexpr std::u16string_view kItalicMarker = u"*";

} // namespace

bool isSpace(char16_t c) { return c == u' ' || c == u'\t' || c == u'\n'; }

std::vector<URange> blocks(UnitsView u) {
    std::vector<URange> out;
    const int n = static_cast<int>(u.size());
    int i = 0;
    int blockStart = -1;
    while (i < n) {
        int j = i;
        while (j < n && u[static_cast<std::size_t>(j)] != u'\n') { ++j; }
        bool blank = true;
        for (int k = i; k < j; ++k) {
            const char16_t c = u[static_cast<std::size_t>(k)];
            if (c != u' ' && c != u'\t') { blank = false; break; }
        }
        const int lineEnd = std::min(j + 1, n);
        if (blank) {
            if (blockStart >= 0) { out.push_back({blockStart, i - blockStart}); blockStart = -1; }
        } else if (blockStart < 0) {
            blockStart = i;
        }
        i = lineEnd;
    }
    if (blockStart >= 0) { out.push_back({blockStart, n - blockStart}); }
    return out;
}

std::vector<Token> tokens(UnitsView block, const Analysis16& a, int from, int to) {
    std::set<int> markerUnits;
    for (const auto& m : a.markers) { for (int k = m.range.location; k < m.range.end(); ++k) { markerUnits.insert(k); } }
    std::set<int> prefixUnits;
    for (const auto& h : a.headings) { for (int k = h.prefix.location; k < h.prefix.end(); ++k) { prefixUnits.insert(k); } }
    for (const auto& li : a.listItems) { for (int k = li.prefix.location; k < li.prefix.end(); ++k) { prefixUnits.insert(k); } }
    std::vector<Token> out;
    for (int k = from; k < to; ++k) {
        if (markerUnits.count(k) != 0) { continue; }
        out.push_back({block[static_cast<std::size_t>(k)], a.style(k), prefixUnits.count(k) != 0});
    }
    return out;
}

std::vector<Token> tokens(UnitsView u) {
    std::vector<Token> out;
    int p = 0;
    for (const auto& b : blocks(u)) {
        while (p < b.location) { out.push_back({u[static_cast<std::size_t>(p)], 0, false}); ++p; }
        const UnitsView block = u.substr(static_cast<std::size_t>(b.location), static_cast<std::size_t>(b.length));
        const auto more = tokens(block, analyzeUnits(block), 0, b.length);
        out.insert(out.end(), more.begin(), more.end());
        p = b.end();
    }
    while (p < static_cast<int>(u.size())) { out.push_back({u[static_cast<std::size_t>(p)], 0, false}); ++p; }
    return out;
}

bool mayHaveMarkers(UnitsView source) {
    int backslashes = 0;
    for (const char16_t c : source) {
        if ((c == u'*' || c == u'_') && backslashes % 2 == 0) { return true; }
        backslashes = c == u'\\' ? backslashes + 1 : 0;
    }
    return false;
}

std::vector<Token> normalize(std::vector<Token> t) {
    // The two passes repeat until nothing changes: moving an edge off punctuation can leave whitespace at the span's new edge,
    // which the whitespace pass then takes out.
    bool changed = true;
    while (changed) {
        changed = whitespacePass(t);
        if (punctuationPass(t)) { changed = true; }
    }
    return t;
}

Serialized serialize(const std::vector<Token>& tokens, std::uint8_t open, std::uint8_t leaveOpen) {
    Serialized s;
    s.text.reserve(tokens.size() + 8);
    std::vector<std::uint8_t> stack;
    for (const std::uint8_t bit : {kBoldBit, kItalicBit}) { if ((open & bit) != 0) { stack.push_back(bit); } }
    auto persists = [&](std::uint8_t bit, std::size_t i) {
        std::size_t j = i;
        while (j < tokens.size() && (tokens[j].style & bit) != 0) { ++j; }
        return j - i;
    };
    auto move = [&](std::uint8_t next, std::size_t i, bool opens) {
        auto k = std::find_if(stack.begin(), stack.end(), [&](std::uint8_t b) { return (next & b) == 0; });
        if (k != stack.end()) {
            for (auto it = stack.end(); it != k;) {
                --it;
                s.text += *it == kBoldBit ? kBoldMarker : kItalicMarker;
            }
            stack.erase(k, stack.end());
        }
        if (!opens) { return; }
        std::vector<std::uint8_t> opening;
        for (const std::uint8_t bit : {kBoldBit, kItalicBit}) {
            if ((next & bit) != 0 && std::find(stack.begin(), stack.end(), bit) == stack.end()) { opening.push_back(bit); }
        }
        // When two styles open together, the one that LASTS LONGER opens outside (Swift's `sorted`, longest first).
        std::stable_sort(opening.begin(), opening.end(),
                         [&](std::uint8_t a, std::uint8_t b) { return persists(a, i) > persists(b, i); });
        for (const std::uint8_t bit : opening) {
            s.text += bit == kBoldBit ? kBoldMarker : kItalicMarker;
            stack.push_back(bit);
        }
    };
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        // A span never OPENS on whitespace (`** x` is not emphasis): a style that has to re-open after an inner span closes
        // waits for the next visible character.
        move(tokens[i].style, i, !isSpace(tokens[i].unit));
        s.text.push_back(tokens[i].unit);
        s.ends.push_back(static_cast<int>(s.text.size()));
    }
    move(leaveOpen, tokens.size(), true);
    return s;
}

Units presented(const std::vector<Token>& tokens) {
    Units u;
    for (const auto& t : tokens) { if (!t.isPrefix) { u.push_back(t.unit); } }
    return escapes::map(u).presented;
}

bool sameRendering(const std::vector<Token>& a, const std::vector<Token>& b) {
    if (a.size() != b.size()) { return false; }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].unit != b[i].unit) { return false; }
        if (!isSpace(a[i].unit) && a[i].style != b[i].style) { return false; }
    }
    return true;
}

} // namespace scrivi::markdown::emphasis
