#include "manuscript/ManuscriptExport.hpp"

#include "markdown/Escapes.hpp"
#include "markdown/MarkdownAnalyzer.hpp"

#include <miniz.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>

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

// A run of visible characters in one style (kBold / kItalic bits), escapes resolved and nothing escaped yet.
struct Span {
    std::string text;
    std::uint8_t style = 0;
};
using Line = std::vector<Span>;

// One block as the writer sees it. Each emitter (HTML, .docx) turns these into its own markup.
struct Block {
    enum class Kind { paragraph, heading, item } kind = Kind::paragraph;
    int level = 0;            // heading
    bool ordered = false;     // item
    int number = 1;           // item
    bool newList = false;     // item: the first of a list
    bool groupStart = false;  // the first block after a blank line (or the scene's start)
    bool body = false;        // paragraph: its group starts with body text (not a heading, item, quote, table or fence)
    bool afterBody = false;   // paragraph: the group before it in this scene starts with body text
    std::vector<Line> lines;  // more than one: a single newline between them
};

// [s, e) of `block` as spans: hidden bytes (emphasis markers, heading and list prefixes) skipped, an escaped mark kept alone.
Line lineSpans(std::string_view block, std::size_t s, std::size_t e, const markdown::Analysis& a,
               const std::vector<bool>& hidden) {
    Line line;
    auto put = [&](char c, std::uint8_t style) {
        if (line.empty() || line.back().style != style) { line.push_back({{}, style}); }
        line.back().text += c;
    };
    for (std::size_t i = s; i < e;) {
        if (hidden[i]) { ++i; continue; }
        const std::uint8_t style = a.styles.empty() ? 0 : a.styles[i];
        if (block[i] == '\\' && i + 1 < e && markdown::escapes::isEscapable(static_cast<unsigned char>(block[i + 1]))) {
            put(block[i + 1], style);
            i += 2;
            continue;
        }
        put(block[i], style);
        ++i;
    }
    return line;
}

// A line ending in an odd run of backslashes ends in a hard break, whose backslash is hidden.
bool endsInHardBreak(std::string_view line) {
    std::size_t n = 0;
    while (n < line.size() && line[line.size() - 1 - n] == '\\') { ++n; }
    return n % 2 == 1;
}

// One blank-line-separated group. A line after a list item continues the item; one after a paragraph line continues it.
void groupBlocks(std::vector<Block>& out, std::string_view block) {
    const auto a = markdown::analyze(block);
    std::vector<bool> hidden(block.size(), false);
    auto hide = [&](markdown::ByteRange r) { for (auto i = r.start; i < r.end; ++i) { hidden[i] = true; } };
    for (const auto& m : a.markers) { hide(m.range); }
    for (const auto& h : a.headings) { hide(h.prefix); }
    for (const auto& li : a.listItems) { hide(li.prefix); }

    const std::size_t first = out.size();
    std::size_t ls = 0;
    while (ls <= block.size()) {
        auto le = block.find('\n', ls);
        if (le == std::string_view::npos) { le = block.size(); }
        std::size_t end = le;
        if (le != block.size() && endsInHardBreak(block.substr(ls, le - ls))) { --end; }

        const markdown::Heading* heading = nullptr;
        for (const auto& h : a.headings) { if (h.line.start == ls) { heading = &h; } }
        const markdown::ListItem* item = nullptr;
        for (const auto& li : a.listItems) { if (li.line.start == ls) { item = &li; } }
        const Block* prev = out.size() > first ? &out.back() : nullptr;

        if (heading) {
            out.push_back({.kind = Block::Kind::heading, .level = heading->level});
        } else if (item) {
            const bool same = prev && prev->kind == Block::Kind::item && prev->ordered == item->ordered;
            out.push_back({.kind = Block::Kind::item, .ordered = item->ordered, .number = item->number, .newList = !same});
        } else if (!prev || prev->kind == Block::Kind::heading) {
            out.push_back({.kind = Block::Kind::paragraph});
        }
        out.back().lines.push_back(lineSpans(block, ls, end, a, hidden));
        ls = le + 1;
    }
    if (out.size() > first) { out[first].groupStart = true; }
}

// Scrivi's book rule (ManuscriptPresenter.isBody): a group is body text unless it opens with a heading, a list item, or a
// quote, table or fence the editor draws as typed.
bool opensWithBody(const Block& b, std::string_view group) {
    if (b.kind != Block::Kind::paragraph) { return false; }
    const auto text = group.substr(std::min(group.find_first_not_of(' '), group.size()));
    return !(text.starts_with('>') || text.starts_with('|') || text.starts_with("```") || text.starts_with("~~~"));
}

// One scene's text as blocks, in order.
std::vector<Block> sceneBlocks(std::string_view text) {
    std::vector<Block> out;
    bool prevBody = false;
    auto group = [&](std::string_view g) {
        const std::size_t first = out.size();
        groupBlocks(out, g);
        if (out.size() == first) { return; }
        const bool body = opensWithBody(out[first], g);
        for (auto i = first; i < out.size(); ++i) { out[i].body = body; out[i].afterBody = prevBody; }
        prevBody = body;
    };
    std::size_t ls = 0, blockStart = std::string_view::npos;
    while (ls <= text.size()) {
        auto le = text.find('\n', ls);
        if (le == std::string_view::npos) { le = text.size(); }
        const bool blank = isBlank(text.substr(ls, le - ls));
        if (!blank && blockStart == std::string_view::npos) { blockStart = ls; }
        if (blank && blockStart != std::string_view::npos) {
            group(text.substr(blockStart, ls - 1 - blockStart));
            blockStart = std::string_view::npos;
        }
        ls = le + 1;
    }
    if (blockStart != std::string_view::npos) { group(text.substr(blockStart)); }
    return out;
}

// A line as inline HTML: every tag opened and closed in nesting order (bold outside italic), all closed at its end.
void inlineHtml(std::string& out, const Line& line) {
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
    for (const auto& span : line) {
        style(span.style);
        for (char c : span.text) { escapeInto(out, c); }
    }
    style(0);
}

void linesHtml(std::string& out, const std::vector<Line>& lines) {
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) { out += "<br />"; }
        inlineHtml(out, lines[i]);
    }
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
    enum class List { none, ul, ol } list = List::none;
    auto closeList = [&] {
        if (list != List::none) { out += list == List::ol ? "</li>\n</ol>\n" : "</li>\n</ul>\n"; list = List::none; }
    };
    for (const auto& b : sceneBlocks(text)) {
        if (b.kind == Block::Kind::item && !b.newList) {
            out += "</li>\n<li>";
            linesHtml(out, b.lines);
            continue;
        }
        closeList();
        if (b.kind == Block::Kind::heading) {
            const auto tag = std::to_string(b.level);
            out += "<h" + tag + ">";
            linesHtml(out, b.lines);
            out += "</h" + tag + ">\n";
        } else if (b.kind == Block::Kind::item) {
            if (!b.ordered) { out += "<ul>\n"; }
            else if (b.number != 1) { out += "<ol start=\"" + std::to_string(b.number) + "\">\n"; }
            else { out += "<ol>\n"; }
            list = b.ordered ? List::ol : List::ul;
            out += "<li>";
            linesHtml(out, b.lines);
        } else {
            out += "<p>";
            linesHtml(out, b.lines);
            out += "</p>\n";
        }
    }
    closeList();
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

namespace {

// Text as WordprocessingML runs' content: XML-escaped, a tab its own element, other control characters dropped (XML has none).
void xmlText(std::string& out, std::string_view s) {
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default:
                if (static_cast<unsigned char>(c) >= 0x20) { out += c; }
        }
    }
}

std::string xml(std::string_view s) {
    std::string out;
    xmlText(out, s);
    return out;
}

void docxRun(std::string& out, std::string_view text, std::uint8_t style) {
    out += "<w:r>";
    if (style != 0) {
        out += "<w:rPr>";
        if (style & kBold) { out += "<w:b/>"; }
        if (style & kItalic) { out += "<w:i/>"; }
        out += "</w:rPr>";
    }
    std::size_t s = 0;
    while (s <= text.size()) {
        const auto tab = std::min(text.find('\t', s), text.size());
        if (tab > s) { out += "<w:t xml:space=\"preserve\">" + xml(text.substr(s, tab - s)) + "</w:t>"; }
        if (tab == text.size()) { break; }
        out += "<w:tab/>";
        s = tab + 1;
    }
    out += "</w:r>";
}

long twips(double points) { return std::lround(points * 20); }
long halfPoints(double points) { return std::lround(points * 2); }

constexpr const char* kNs = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
constexpr const char* kXmlHead = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

// The body's paragraphs, and the numbering instances its ordered lists need.
struct DocxBody {
    const DocxTypography& type;
    std::string out;
    std::vector<int> orderedStarts;   // numId 2 + i restarts at orderedStarts[i]; numId 1 is the bullets
    bool started = false;
    bool pageBreak = false;

    void paragraph(std::string_view style, const std::vector<Line>& lines, int numId = 0) {
        out += "<w:p><w:pPr><w:pStyle w:val=\"" + std::string(style) + "\"/>";
        if (pageBreak && started) { out += "<w:pageBreakBefore/>"; }
        if (numId != 0) { out += "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"" + std::to_string(numId) + "\"/></w:numPr>"; }
        out += "</w:pPr>";
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) { out += "<w:r><w:br/></w:r>"; }
            for (const auto& span : lines[i]) { docxRun(out, span.text, span.style); }
        }
        out += "</w:p>\n";
        started = true;
        pageBreak = false;
    }
    void plain(std::string_view style, std::string_view text) { paragraph(style, {Line{Span{std::string(text), 0}}}); }

    void scene(std::string_view text) {
        int numId = 0;
        for (const auto& b : sceneBlocks(text)) {
            if (b.kind == Block::Kind::heading) {
                paragraph("Heading" + std::to_string(std::clamp(b.level, 1, 3)), b.lines);
            } else if (b.kind == Block::Kind::item) {
                if (b.newList) {
                    if (b.ordered) { orderedStarts.push_back(b.number); }
                    numId = b.ordered ? 1 + static_cast<int>(orderedStarts.size()) : 1;
                }
                paragraph(b.ordered ? "ListNumber" : "ListBullet", b.lines, numId);
            } else {
                // Q1: the book rule indents a body paragraph that follows body text in the same scene (P10).
                const bool indented = type.indent == "none" ||
                                      (b.groupStart && b.body && (type.indent == "every" || b.afterBody));
                paragraph(indented ? "BodyText" : "FirstParagraph", b.lines);
            }
        }
    }
};

std::string docxStyles(const DocxTypography& t) {
    const auto size = [&](double pt) { const auto h = std::to_string(halfPoints(pt));
                                       return "<w:sz w:val=\"" + h + "\"/><w:szCs w:val=\"" + h + "\"/>"; };
    const auto face = xml(t.typeface);
    const auto line = std::to_string(twips(t.size));
    std::string s = std::string(kXmlHead) + "<w:styles " + kNs + ">\n<w:docDefaults><w:rPrDefault><w:rPr>";
    if (!face.empty()) {
        s += "<w:rFonts w:ascii=\"" + face + "\" w:hAnsi=\"" + face + "\" w:eastAsia=\"" + face + "\" w:cs=\"" + face + "\"/>";
    }
    s += size(t.size) + "</w:rPr></w:rPrDefault><w:pPrDefault><w:pPr><w:spacing w:after=\"0\" w:line=\"240\" "
         "w:lineRule=\"auto\"/></w:pPr></w:pPrDefault></w:docDefaults>\n";
    s += "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>\n";
    // Q1: `none` has no indent and a line's space after each paragraph, as Scrivi draws the stored blank line.
    const auto firstLine = t.indent == "none" ? 0L : twips(t.indentEm * t.size);
    s += "<w:style w:type=\"paragraph\" w:styleId=\"BodyText\"><w:name w:val=\"Body Text\"/><w:basedOn w:val=\"Normal\"/>"
         "<w:qFormat/><w:pPr><w:spacing w:after=\"" + (t.indent == "none" ? line : std::string("0")) + "\"/>"
         "<w:ind w:firstLine=\"" + std::to_string(firstLine) + "\"/></w:pPr></w:style>\n";
    s += "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"FirstParagraph\"><w:name w:val=\"First Paragraph\"/>"
         "<w:basedOn w:val=\"BodyText\"/><w:next w:val=\"BodyText\"/><w:qFormat/><w:pPr><w:ind w:firstLine=\"0\"/></w:pPr>"
         "</w:style>\n";
    s += "<w:style w:type=\"paragraph\" w:styleId=\"Title\"><w:name w:val=\"Title\"/><w:basedOn w:val=\"Normal\"/>"
         "<w:next w:val=\"Subtitle\"/><w:qFormat/><w:pPr><w:spacing w:before=\"" + std::to_string(twips(t.size * 8)) +
         "\" w:after=\"" + line + "\"/><w:jc w:val=\"center\"/></w:pPr><w:rPr><w:b/>" + size(t.size * 33 / 13) +
         "</w:rPr></w:style>\n";
    s += "<w:style w:type=\"paragraph\" w:styleId=\"Subtitle\"><w:name w:val=\"Subtitle\"/><w:basedOn w:val=\"Normal\"/>"
         "<w:qFormat/><w:pPr><w:jc w:val=\"center\"/></w:pPr><w:rPr>" + size(t.size * 18 / 13) + "</w:rPr></w:style>\n";
    // The editor's heading sizes (ManuscriptTypography.headingSize: 22 / 18 / 16 over 13), bold.
    const double scale[] = {22.0 / 13, 18.0 / 13, 16.0 / 13};
    for (int level = 1; level <= 3; ++level) {
        const auto n = std::to_string(level);
        s += "<w:style w:type=\"paragraph\" w:styleId=\"Heading" + n + "\"><w:name w:val=\"heading " + n + "\"/>"
             "<w:basedOn w:val=\"Normal\"/><w:next w:val=\"FirstParagraph\"/><w:qFormat/><w:pPr><w:keepNext/>"
             "<w:spacing w:before=\"" + line + "\" w:after=\"" + std::to_string(twips(t.size / 2)) + "\"/>"
             "<w:outlineLvl w:val=\"" + std::to_string(level - 1) + "\"/></w:pPr><w:rPr><w:b/>" + size(t.size * scale[level - 1]) +
             "</w:rPr></w:style>\n";
    }
    s += "<w:style w:type=\"paragraph\" w:styleId=\"ListBullet\"><w:name w:val=\"List Bullet\"/><w:basedOn w:val=\"Normal\"/>"
         "<w:qFormat/></w:style>\n";
    s += "<w:style w:type=\"paragraph\" w:styleId=\"ListNumber\"><w:name w:val=\"List Number\"/><w:basedOn w:val=\"Normal\"/>"
         "<w:qFormat/></w:style>\n";
    s += "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"SceneBreak\"><w:name w:val=\"Scene Break\"/>"
         "<w:basedOn w:val=\"Normal\"/><w:next w:val=\"FirstParagraph\"/><w:qFormat/><w:pPr><w:spacing w:before=\"" + line +
         "\" w:after=\"" + line + "\"/><w:jc w:val=\"center\"/></w:pPr></w:style>\n";
    return s + "</w:styles>\n";
}

std::string docxNumbering(const std::vector<int>& orderedStarts) {
    auto lvl = [](std::string_view fmt, std::string_view text) {
        return "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"" + std::string(fmt) + "\"/><w:lvlText w:val=\"" +
               std::string(text) + "\"/><w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>";
    };
    std::string s = std::string(kXmlHead) + "<w:numbering " + kNs + ">\n";
    s += "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"singleLevel\"/>" + lvl("bullet", "\u2022") +
         "</w:abstractNum>\n";
    s += "<w:abstractNum w:abstractNumId=\"1\"><w:multiLevelType w:val=\"singleLevel\"/>" + lvl("decimal", "%1.") +
         "</w:abstractNum>\n";
    s += "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>\n";
    // Q5: each numbered list is its own instance, restarting at its first number.
    for (std::size_t i = 0; i < orderedStarts.size(); ++i) {
        s += "<w:num w:numId=\"" + std::to_string(i + 2) + "\"><w:abstractNumId w:val=\"1\"/><w:lvlOverride w:ilvl=\"0\">"
             "<w:startOverride w:val=\"" + std::to_string(orderedStarts[i]) + "\"/></w:lvlOverride></w:num>\n";
    }
    return s + "</w:numbering>\n";
}

} // namespace

static std::string docxDocumentXml(const ExportInput& in, const DocxTypography& type, std::vector<int>* orderedStarts) {
    DocxBody body{type};
    if (in.titlePage) {
        body.plain("Title", in.title);
        if (!in.subtitle.empty()) { body.plain("Subtitle", in.subtitle); }
    }
    walk(in,
         [&](const ExportScene& s, bool newChapter, bool first) {
             if (newChapter) {
                 body.pageBreak = true;   // Q3: every chapter, and the first after the title page, on a new page
                 if (in.chapterTitles) { body.plain("Heading1", s.chapterHeading); }
             } else if (!first) {
                 body.plain("SceneBreak", "* * *");
             }
         },
         [&](std::string_view text) { body.scene(text); });
    if (!body.started) { body.out += "<w:p/>\n"; }
    if (orderedStarts) { *orderedStarts = body.orderedStarts; }
    return std::string(kXmlHead) + "<w:document " + kNs + "><w:body>\n" + body.out + "</w:body></w:document>\n";
}

std::string exportDocx(const ExportInput& in, const DocxTypography& type) {
    std::vector<int> orderedStarts;
    const auto document = docxDocumentXml(in, type, &orderedStarts);
    const std::pair<const char*, std::string> parts[] = {
        {"[Content_Types].xml",
         std::string(kXmlHead) +
             "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
             "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
             "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
             "<Override PartName=\"/word/document.xml\" "
             "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
             "<Override PartName=\"/word/styles.xml\" "
             "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
             "<Override PartName=\"/word/numbering.xml\" "
             "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml\"/>"
             "<Override PartName=\"/docProps/core.xml\" "
             "ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/></Types>\n"},
        {"_rels/.rels",
         std::string(kXmlHead) +
             "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
             "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/"
             "officeDocument\" Target=\"word/document.xml\"/>"
             "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/"
             "core-properties\" Target=\"docProps/core.xml\"/></Relationships>\n"},
        {"docProps/core.xml",
         std::string(kXmlHead) +
             "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" "
             "xmlns:dc=\"http://purl.org/dc/elements/1.1/\"><dc:title>" + xml(in.title) + "</dc:title></cp:coreProperties>\n"},
        {"word/_rels/document.xml.rels",
         std::string(kXmlHead) +
             "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
             "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" "
             "Target=\"styles.xml\"/>"
             "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/"
             "numbering\" Target=\"numbering.xml\"/></Relationships>\n"},
        {"word/document.xml", document},
        {"word/styles.xml", docxStyles(type)},
        {"word/numbering.xml", docxNumbering(orderedStarts)},
    };

    // Q7: a fixed time, so the same manuscript gives the same bytes. miniz converts it with localtime(), so it is made
    // from LOCAL 1980-01-02 00:00 and reads back as that date in every time zone.
    std::tm tm{};
    tm.tm_year = 80;
    tm.tm_mday = 2;
    tm.tm_isdst = -1;
    MZ_TIME_T fixed = std::mktime(&tm);

    mz_zip_archive z{};
    if (!mz_zip_writer_init_heap(&z, 0, 0)) { return {}; }
    bool ok = true;
    for (const auto& [name, text] : parts) {
        ok = ok && mz_zip_writer_add_mem_ex_v2(&z, name, text.data(), text.size(), nullptr, 0, MZ_DEFAULT_LEVEL, 0, 0,
                                               &fixed, nullptr, 0, nullptr, 0);
    }
    void* buf = nullptr;
    std::size_t len = 0;
    ok = ok && mz_zip_writer_finalize_heap_archive(&z, &buf, &len);
    std::string bytes = ok ? std::string(static_cast<const char*>(buf), len) : std::string{};
    if (buf) { z.m_pFree(z.m_pAlloc_opaque, buf); }   // finalize hands the buffer to the caller
    mz_zip_writer_end(&z);
    return bytes;
}

} // namespace scrivi::manuscript
