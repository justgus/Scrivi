#include "manuscript/ManuscriptExport.hpp"

#include "markdown/Escapes.hpp"
#include "markdown/MarkdownAnalyzer.hpp"

#include <cstdint>

namespace scrivi::manuscript {

namespace {

using markdown::kBold;
using markdown::kItalic;

void escapeInto(std::string& out, char c) {
    switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        default: out += c;
    }
}

std::string escaped(std::string_view s) {
    std::string out;
    for (char c : s) { escapeInto(out, c); }
    return out;
}

bool isBlank(std::string_view line) { return line.find_first_not_of(" \t\r") == std::string_view::npos; }

std::string_view trimmed(std::string_view s) {
    while (!s.empty() && (s.front() == '\n' || s.front() == '\r')) { s.remove_prefix(1); }
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) { s.remove_suffix(1); }
    return s;
}

// [s, e) of `block` as inline HTML: hidden bytes (emphasis markers, heading and list prefixes) skipped, an escaped mark
// shown alone, every tag opened and closed in nesting order (bold outside italic).
void inlineHtml(std::string& out, std::string_view block, std::size_t s, std::size_t e,
                const markdown::Analysis& a, const std::vector<bool>& hidden) {
    std::vector<std::uint8_t> stack;
    std::uint8_t open = 0;
    auto style = [&](std::uint8_t want) {
        while ((open & ~want) != 0) {
            const auto b = stack.back();
            stack.pop_back();
            out += b == kBold ? "</strong>" : "</em>";
            open &= static_cast<std::uint8_t>(~b);
        }
        for (const auto b : {kBold, kItalic}) {
            if ((want & b) && !(open & b)) {
                stack.push_back(b);
                out += b == kBold ? "<strong>" : "<em>";
                open |= b;
            }
        }
    };
    for (std::size_t i = s; i < e;) {
        if (hidden[i]) { ++i; continue; }
        style(a.styles.empty() ? 0 : a.styles[i]);
        if (block[i] == '\\' && i + 1 < e && markdown::escapes::isEscapable(static_cast<unsigned char>(block[i + 1]))) {
            escapeInto(out, block[i + 1]);
            i += 2;
            continue;
        }
        escapeInto(out, block[i]);
        ++i;
    }
    style(0);
}

// A line ending in an odd run of backslashes ends in a hard break, whose backslash is hidden.
bool endsInHardBreak(std::string_view line) {
    std::size_t n = 0;
    while (n < line.size() && line[line.size() - 1 - n] == '\\') { ++n; }
    return n % 2 == 1;
}

void blockHtml(std::string& out, std::string_view block) {
    const auto a = markdown::analyze(block);
    std::vector<bool> hidden(block.size(), false);
    auto hide = [&](markdown::ByteRange r) { for (auto i = r.start; i < r.end; ++i) { hidden[i] = true; } };
    for (const auto& m : a.markers) { hide(m.range); }
    for (const auto& h : a.headings) { hide(h.prefix); }
    for (const auto& li : a.listItems) { hide(li.prefix); }

    enum class List { none, ul, ol } list = List::none;
    bool inItem = false, inPara = false;
    auto closePara = [&] { if (inPara) { out += "</p>\n"; inPara = false; } };
    auto closeItem = [&] { if (inItem) { out += "</li>\n"; inItem = false; } };
    auto closeList = [&] {
        closeItem();
        if (list != List::none) { out += list == List::ol ? "</ol>\n" : "</ul>\n"; list = List::none; }
    };

    std::size_t ls = 0;
    while (ls <= block.size()) {
        auto le = block.find('\n', ls);
        if (le == std::string_view::npos) { le = block.size(); }
        const bool last = le == block.size();
        std::size_t end = le;
        if (!last && endsInHardBreak(block.substr(ls, le - ls))) { --end; }

        const markdown::Heading* heading = nullptr;
        for (const auto& h : a.headings) { if (h.line.start == ls) { heading = &h; } }
        const markdown::ListItem* item = nullptr;
        for (const auto& li : a.listItems) { if (li.line.start == ls) { item = &li; } }

        if (heading) {
            closePara();
            closeList();
            const auto tag = std::to_string(heading->level);
            out += "<h" + tag + ">";
            inlineHtml(out, block, ls, end, a, hidden);
            out += "</h" + tag + ">\n";
        } else if (item) {
            closePara();
            closeItem();
            const auto want = item->ordered ? List::ol : List::ul;
            if (list != want) {
                closeList();
                if (!item->ordered) { out += "<ul>\n"; }
                else if (item->number != 1) { out += "<ol start=\"" + std::to_string(item->number) + "\">\n"; }
                else { out += "<ol>\n"; }
                list = want;
            }
            out += "<li>";
            inlineHtml(out, block, ls, end, a, hidden);
            inItem = true;
        } else if (inItem) {
            out += "<br />";
            inlineHtml(out, block, ls, end, a, hidden);
        } else {
            out += inPara ? "<br />" : "<p>";
            inPara = true;
            inlineHtml(out, block, ls, end, a, hidden);
        }
        ls = le + 1;
    }
    closePara();
    closeList();
}

// Calls `scene` for each scene with text, and `boundary(isNewChapter)` before every one but the first.
template <class Boundary, class Scene>
void walk(const ExportInput& in, Boundary boundary, Scene scene) {
    const std::string* chapter = nullptr;
    for (const auto& s : in.scenes) {
        const auto text = trimmed(s.text);
        if (text.empty()) { continue; }
        const bool newChapter = chapter == nullptr || *chapter != s.chapterID;
        boundary(s, newChapter, chapter == nullptr);
        chapter = &s.chapterID;
        scene(text);
    }
}

} // namespace

std::string sceneHtml(std::string_view text) {
    std::string out;
    std::size_t ls = 0, blockStart = std::string_view::npos;
    while (ls <= text.size()) {
        auto le = text.find('\n', ls);
        if (le == std::string_view::npos) { le = text.size(); }
        const bool blank = isBlank(text.substr(ls, le - ls));
        if (!blank && blockStart == std::string_view::npos) { blockStart = ls; }
        if (blank && blockStart != std::string_view::npos) {
            blockHtml(out, text.substr(blockStart, ls - 1 - blockStart));
            blockStart = std::string_view::npos;
        }
        ls = le + 1;
    }
    if (blockStart != std::string_view::npos) { blockHtml(out, text.substr(blockStart)); }
    return out;
}

std::string exportMarkdown(const ExportInput& in) {
    std::string out;
    if (in.titlePage) {
        out += "# " + in.title + "\n\n";
        if (!in.subtitle.empty()) { out += in.subtitle + "\n\n"; }
    }
    walk(in,
         [&](const ExportScene& s, bool newChapter, bool first) {
             if (newChapter && in.chapterTitles) { out += "# " + s.chapterHeading + "\n\n"; }
             else if (!first) { out += "* * *\n\n"; }
         },
         [&](std::string_view text) { out.append(text); out += "\n\n"; });
    while (out.size() > 1 && out.ends_with("\n\n")) { out.pop_back(); }
    return out;
}

std::string exportHtml(const ExportInput& in, bool wholePage) {
    std::string out;
    if (wholePage) {
        out += "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>" + escaped(in.title) +
               "</title>\n</head>\n<body>\n";
    }
    if (in.titlePage) {
        out += "<h1>" + escaped(in.title) + "</h1>\n";
        if (!in.subtitle.empty()) { out += "<p>" + escaped(in.subtitle) + "</p>\n"; }
    }
    walk(in,
         [&](const ExportScene& s, bool newChapter, bool first) {
             if (newChapter && in.chapterTitles) { out += "<h2>" + escaped(s.chapterHeading) + "</h2>\n"; }
             else if (!first) { out += "<hr />\n"; }
         },
         [&](std::string_view text) { out += sceneHtml(text); });
    if (wholePage) { out += "</body>\n</html>\n"; }
    return out;
}

} // namespace scrivi::manuscript
