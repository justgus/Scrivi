#include "markdown/MarkdownText.hpp"

#include "markdown/MarkdownAnalyzer.hpp"

namespace scrivi::markdown {

Units toUnits(std::string_view s) {
    Units out;
    out.reserve(s.size());
    std::size_t i = 0;
    while (i < s.size()) {
        const auto c = static_cast<unsigned char>(s[i]);
        char32_t cp = 0xFFFD;
        std::size_t n = 1;
        if (c < 0x80) { cp = c; }
        else if ((c >> 5) == 0x6 && i + 1 < s.size()) { cp = ((c & 0x1F) << 6) | (s[i + 1] & 0x3F); n = 2; }
        else if ((c >> 4) == 0xE && i + 2 < s.size()) {
            cp = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F); n = 3;
        } else if ((c >> 3) == 0x1E && i + 3 < s.size()) {
            cp = ((c & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) | ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F); n = 4;
        }
        if (cp >= 0x10000) {
            cp -= 0x10000;
            out.push_back(static_cast<char16_t>(0xD800 + (cp >> 10)));
            out.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));
        } else {
            out.push_back(static_cast<char16_t>(cp));
        }
        i += n;
    }
    return out;
}

std::string toUtf8(UnitsView u) {
    std::string out;
    out.reserve(u.size());
    for (std::size_t i = 0; i < u.size(); ++i) {
        char32_t cp = u[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < u.size() && u[i + 1] >= 0xDC00 && u[i + 1] <= 0xDFFF) {
            cp = 0x10000 + ((cp - 0xD800) << 10) + (u[i + 1] - 0xDC00);
            ++i;
        } else if (cp >= 0xD800 && cp <= 0xDFFF) {
            cp = 0xFFFD;   // a LONE surrogate (a range split a pair): U+FFFD, as Swift's String does — never invalid UTF-8
        }
        if (cp < 0x80) { out.push_back(static_cast<char>(cp)); }
        else if (cp < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

Analysis16 analyzeUnits(UnitsView block) {
    const std::string utf8 = toUtf8(block);
    const Analysis a = analyze(utf8);

    // byte → unit: every byte of a scalar → its FIRST unit; the end (utf8.size()) → block.size().
    std::vector<int> unitAt(utf8.size() + 1, static_cast<int>(block.size()));
    std::vector<int> scalarUnits(block.size() + 1, 1);   // units of the scalar starting at unit u
    {
        std::size_t b = 0;
        int u = 0;
        for (std::size_t i = 0; i < block.size(); ++i) {
            char32_t cp = block[i];
            const bool pair = cp >= 0xD800 && cp <= 0xDBFF && i + 1 < block.size()
                              && block[i + 1] >= 0xDC00 && block[i + 1] <= 0xDFFF;
            if (pair) { cp = 0x10000 + ((cp - 0xD800) << 10) + (block[i + 1] - 0xDC00); }
            const std::size_t nb = cp < 0x80 ? 1 : cp < 0x800 ? 2 : cp < 0x10000 ? 3 : 4;
            for (std::size_t k = 0; k < nb && b + k < unitAt.size(); ++k) { unitAt[b + k] = u; }
            scalarUnits[static_cast<std::size_t>(u)] = pair ? 2 : 1;
            b += nb;
            u += pair ? 2 : 1;
            if (pair) { ++i; }
        }
        if (b < unitAt.size()) { unitAt[b] = u; }
    }
    auto at = [&](std::size_t byte) { return byte < unitAt.size() ? unitAt[byte] : static_cast<int>(block.size()); };
    auto range = [&](const ByteRange& r) {
        const int s = at(r.start), e = at(r.end);
        return URange{s, e - s};
    };

    Analysis16 out;
    for (const auto& h : a.headings) { out.headings.push_back({range(h.line), range(h.prefix), h.level}); }
    for (const auto& li : a.listItems) { out.listItems.push_back({range(li.line), range(li.prefix), li.ordered, li.number}); }
    for (const auto& m : a.markers) { out.markers.push_back({range(m.range), m.opens}); }
    for (const auto& s : a.spans) { out.spans.push_back(range(s)); }
    if (!a.styles.empty()) {
        out.styles.assign(block.size(), 0);
        for (std::size_t byte = 0; byte < a.styles.size() && byte < utf8.size(); ++byte) {
            if (a.styles[byte] == 0) { continue; }
            const int u = at(byte);
            for (int k = 0; k < scalarUnits[static_cast<std::size_t>(u)] && u + k < static_cast<int>(block.size()); ++k) {
                out.styles[static_cast<std::size_t>(u + k)] = a.styles[byte];
            }
        }
    }
    return out;
}

} // namespace scrivi::markdown
