#include "manuscript/ManuscriptImport.hpp"

#include "markdown/MarkdownAnalyzer.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <optional>

namespace scrivi::manuscript {

namespace {

Error notText() {
    return Error{ErrorCode::invalidArgument, "the file is not text", {}, "notText"};
}

void appendUtf8(std::string& out, std::uint32_t cp) {
    if (cp < 0x80) { out += static_cast<char>(cp); }
    else if (cp < 0x800) { out += static_cast<char>(0xC0 | (cp >> 6)); out += static_cast<char>(0x80 | (cp & 0x3F)); }
    else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

bool validUtf8(std::string_view s) {
    for (std::size_t i = 0; i < s.size();) {
        const auto c = static_cast<unsigned char>(s[i]);
        const std::size_t n = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 0;
        if (n == 0 || i + n > s.size()) { return false; }
        for (std::size_t k = 1; k < n; ++k) {
            if ((static_cast<unsigned char>(s[i + k]) & 0xC0) != 0x80) { return false; }
        }
        i += n;
    }
    return true;
}

std::string fromUtf16(std::string_view b, bool bigEndian) {
    std::string out;
    auto unit = [&](std::size_t i) -> std::uint32_t {
        const auto a = static_cast<unsigned char>(b[i]), c = static_cast<unsigned char>(b[i + 1]);
        return bigEndian ? (a << 8 | c) : (c << 8 | a);
    };
    for (std::size_t i = 0; i + 1 < b.size(); i += 2) {
        std::uint32_t u = unit(i);
        if (u >= 0xD800 && u < 0xDC00 && i + 3 < b.size()) {
            const std::uint32_t lo = unit(i + 2);
            if (lo >= 0xDC00 && lo < 0xE000) { u = 0x10000 + ((u - 0xD800) << 10) + (lo - 0xDC00); i += 2; }
            else { u = 0xFFFD; }
        } else if (u >= 0xD800 && u < 0xE000) {
            u = 0xFFFD;   // a lone surrogate
        }
        appendUtf8(out, u);
    }
    return out;
}

std::string fromWindows1252(std::string_view b) {
    // 0x80–0x9F; the five unassigned bytes keep their C1 code points.
    static constexpr std::uint16_t k[32] = {
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D,
        0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A,
        0x0153, 0x009D, 0x017E, 0x0178};
    std::string out;
    for (char ch : b) {
        const auto c = static_cast<unsigned char>(ch);
        appendUtf8(out, c >= 0x80 && c < 0xA0 ? k[c - 0x80] : c);
    }
    return out;
}

// Q4: the bytes as UTF-8 with "\n" line endings.
std::string decode(std::string_view b, std::string& encoding) {
    std::string text;
    if (b.starts_with("\xEF\xBB\xBF")) { encoding = "utf-8"; text = b.substr(3); }
    else if (b.starts_with("\xFF\xFE")) { encoding = "utf-16le"; text = fromUtf16(b.substr(2), false); }
    else if (b.starts_with("\xFE\xFF")) { encoding = "utf-16be"; text = fromUtf16(b.substr(2), true); }
    else if (validUtf8(b)) { encoding = "utf-8"; text = b; }
    else { encoding = "windows-1252"; text = fromWindows1252(b); }
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r') {
            out += '\n';
            if (i + 1 < text.size() && text[i + 1] == '\n') { ++i; }
        } else {
            out += text[i];
        }
    }
    return out;
}

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) { s.remove_prefix(1); }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) { s.remove_suffix(1); }
    return s;
}

std::size_t codePoints(std::string_view s) {
    return static_cast<std::size_t>(std::count_if(s.begin(), s.end(), [](char c) { return (c & 0xC0) != 0x80; }));
}

std::string shortened(std::string_view line) {
    line = trim(line);
    if (line.size() <= 80) { return std::string(line); }
    std::size_t n = 80;
    while (n > 0 && (static_cast<unsigned char>(line[n]) & 0xC0) == 0x80) { --n; }
    return std::string(line.substr(0, n)) + "…";
}

// A block: a maximal run of non-blank lines.
using Block = std::vector<std::string_view>;

std::vector<Block> blocksOf(std::string_view text) {
    std::vector<Block> blocks;
    Block current;
    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = std::min(text.find('\n', start), text.size());
        const std::string_view line = text.substr(start, end - start);
        if (trim(line).empty()) {
            if (!current.empty()) { blocks.push_back(std::move(current)); current.clear(); }
        } else {
            current.push_back(line);
        }
        start = end + 1;
    }
    if (!current.empty()) { blocks.push_back(std::move(current)); }
    return blocks;
}

// A level-1 ATX heading, by the analyzer the editor uses; its title without the `#` prefix or closing hashes.
std::optional<std::string> chapterHeading(std::string_view line) {
    const auto a = markdown::analyze(line);
    if (a.headings.empty() || a.headings.front().level != 1) { return std::nullopt; }
    const auto& h = a.headings.front();
    std::string_view title = trim(line.substr(h.prefix.end, h.line.end - h.prefix.end));
    const auto hashes = title.find_last_not_of('#');
    if (hashes == std::string_view::npos) { title = {}; }
    else if (hashes + 1 < title.size() && (title[hashes] == ' ' || title[hashes] == '\t')) { title = trim(title.substr(0, hashes)); }
    return std::string(title);
}

// Q3: `* * *`, `***`, `---`, `___` — three or more of one mark, spaces allowed.
bool thematicBreak(std::string_view line) {
    line = trim(line);
    if (line.empty() || (line[0] != '*' && line[0] != '-' && line[0] != '_')) { return false; }
    std::size_t marks = 0;
    for (char c : line) {
        if (c == line[0]) { ++marks; }
        else if (c != ' ' && c != '\t') { return false; }
    }
    return marks >= 3;
}

// Q4: what typing stores — every escapable mark behind a backslash.
std::string escaped(std::string_view line) {
    static constexpr std::string_view kMarks = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    std::string out;
    out.reserve(line.size());
    for (char c : line) {
        if (kMarks.find(c) != std::string_view::npos) { out += '\\'; }
        out += c;
    }
    return out;
}

std::string joined(const Block& lines, std::size_t from = 0) {
    std::string out;
    for (std::size_t i = from; i < lines.size(); ++i) { out += (i > from ? "\n" : "") + std::string(lines[i]); }
    return out;
}

struct Notes {
    std::map<std::string, ImportNote> byKind;
    std::vector<std::string> order;
    void add(const std::string& kind, std::string_view line) {
        auto [it, inserted] = byKind.try_emplace(kind, ImportNote{kind, 0, shortened(line)});
        if (inserted) { order.push_back(kind); }
        ++it->second.count;
    }
};

// Q8: Markdown Scrivi keeps only as text.
void noteMarkdown(std::string_view line, Notes& notes) {
    const std::string_view t = trim(line);
    if (t.starts_with("```") || t.starts_with("~~~") || std::count(t.begin(), t.end(), '`') >= 2) { notes.add("code", line); }
    if (t.starts_with(">")) { notes.add("blockQuote", line); }
    if (t.find("![") != std::string_view::npos) { notes.add("image", line); }
    else if (t.find("](") != std::string_view::npos) { notes.add("link", line); }
    if (t.find('|') != std::string_view::npos && t.find("---") != std::string_view::npos &&
        t.find_first_not_of("|:- \t") == std::string_view::npos) {
        notes.add("table", line);
    }
    for (std::size_t i = t.find('<'); i != std::string_view::npos && i + 1 < t.size(); i = t.find('<', i + 1)) {
        const char c = t[i + 1];
        if ((std::isalpha(static_cast<unsigned char>(c)) || c == '/' || c == '!') && t.find('>', i) != std::string_view::npos) {
            notes.add("html", line);
            break;
        }
    }
}

struct Chapter {
    std::string title;
    bool explicitHeading = false;
    std::vector<std::string> scenes{std::string()};
    void paragraph(std::string text) {
        auto& s = scenes.back();
        s += (s.empty() ? "" : "\n\n") + std::move(text);
    }
    [[nodiscard]] bool empty() const {
        return std::all_of(scenes.begin(), scenes.end(), [](const std::string& s) { return s.empty(); });
    }
};

} // namespace

Result<ImportedManuscript> importManuscript(std::string_view bytes, bool markdown) {
    using R = Result<ImportedManuscript>;
    ImportedManuscript out;
    const std::string text = decode(bytes, out.encoding);
    if (text.find('\0') != std::string::npos) { return R::failure(notText()); }

    Notes notes;
    if (out.encoding == "windows-1252") { notes.add("encodingGuessed", "windows-1252"); }
    const std::vector<Block> blocks = blocksOf(text);

    // Q5: hard-wrapped — most lines sit in multi-line paragraphs, nearly all of those no longer than 80 characters.
    std::size_t lines = 0, wrapped = 0, short80 = 0;
    std::string_view wrapExample;
    for (const auto& b : blocks) {
        lines += b.size();
        if (b.size() < 2) { continue; }
        wrapped += b.size();
        for (auto l : b) { short80 += codePoints(l) <= 80 ? 1 : 0; }
        if (wrapExample.empty()) { wrapExample = b.front(); }
    }
    if (lines >= 20 && wrapped * 2 >= lines && short80 * 10 >= wrapped * 9) { notes.add("hardWrapped", wrapExample); }

    std::size_t first = 0;
    // Q3: exactly S3's title page — `# Title`, an optional subtitle, `by …`, then a chapter heading.
    if (markdown && !blocks.empty() && blocks[0].size() == 1) {
        if (auto title = chapterHeading(blocks[0][0])) {
            std::size_t i = 1;
            std::string subtitle;
            if (i < blocks.size() && blocks[i].size() == 1 && !trim(blocks[i][0]).starts_with("by ") &&
                !chapterHeading(blocks[i][0])) {
                subtitle = trim(blocks[i][0]);
                ++i;
            }
            if (i + 1 < blocks.size() && blocks[i].size() == 1 && trim(blocks[i][0]).starts_with("by ") &&
                chapterHeading(blocks[i + 1][0])) {
                out.title = *title;
                out.subtitle = subtitle;
                notes.add("byline", blocks[i][0]);
                first = i + 1;
            }
        }
    }

    std::vector<Chapter> chapters(1);
    for (std::size_t bi = first; bi < blocks.size(); ++bi) {
        const Block& b = blocks[bi];
        if (b.size() == 1 && (markdown ? thematicBreak(b[0]) : (trim(b[0]) == "* * *" || trim(b[0]) == "***" || trim(b[0]) == "#"))) {
            chapters.back().scenes.emplace_back();
            continue;
        }
        if (!markdown) {
            std::string para;
            for (auto l : b) { para += (para.empty() ? "" : "\n") + escaped(l); }
            chapters.back().paragraph(std::move(para));
            continue;
        }
        std::size_t from = 0;
        for (std::size_t li = 0; li < b.size(); ++li) {
            noteMarkdown(b[li], notes);
            auto heading = chapterHeading(b[li]);
            if (!heading) { continue; }
            if (li > from) { chapters.back().paragraph(joined(Block(b.begin() + from, b.begin() + li))); }
            from = li + 1;
            chapters.push_back(Chapter{*heading, true});
        }
        if (from < b.size()) { chapters.back().paragraph(joined(b, from)); }
    }
    if (chapters.size() > 1 && !chapters[0].explicitHeading && chapters[0].empty()) { chapters.erase(chapters.begin()); }

    for (std::size_t ci = 0; ci < chapters.size(); ++ci) {
        auto& c = chapters[ci];
        std::erase_if(c.scenes, [](const std::string& s) { return s.empty(); });
        if (c.scenes.empty()) {
            if (!c.explicitHeading) { continue; }   // nothing to bring in
            c.scenes.emplace_back();                // a chapter's only scene keeps its heading
        }
        ++out.chapterCount;
        for (auto& s : c.scenes) {
            RangeEntry e;
            e.chapterID = ChapterID{"~import-" + std::to_string(ci)};
            e.chapterTitle = c.title;
            e.length = e.sceneLength = s.size();
            e.text = std::move(s);
            out.range.append(std::move(e));
        }
    }
    out.opensChapter = !out.range.empty() && chapters.front().explicitHeading;
    for (const auto& k : notes.order) { out.report.push_back(notes.byKind[k]); }
    return R::success(std::move(out));
}

} // namespace scrivi::manuscript
