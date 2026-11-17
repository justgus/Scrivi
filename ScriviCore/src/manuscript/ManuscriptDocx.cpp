#include "manuscript/ManuscriptDocx.hpp"

#include "miniz.h"
#include "pugixml.hpp"

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace scrivi::manuscript {

namespace {

// A part far past any manuscript's is refused before it is inflated (the zip's own size is capped by the caller).
constexpr std::size_t kMaxPart = 256u << 20;

Error notDocx() {
    return Error{ErrorCode::invalidArgument, "the file is not a Word document Scrivi can read", {}, "notText"};
}

Error tooLarge() {
    return Error{ErrorCode::invalidArgument, "the document is too large to be a manuscript", {}, "tooLarge"};
}

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\n')) { s.remove_prefix(1); }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\n')) { s.remove_suffix(1); }
    return s;
}

std::string shortened(std::string_view line) {
    line = trim(line);
    if (line.size() <= 80) { return std::string(line); }
    std::size_t n = 80;
    while (n > 0 && (static_cast<unsigned char>(line[n]) & 0xC0) == 0x80) { --n; }
    return std::string(line.substr(0, n)) + "…";
}

std::string lower(std::string_view s) {
    std::string out(s);
    for (auto& c : out) { if (c >= 'A' && c <= 'Z') { c = static_cast<char>(c - 'A' + 'a'); } }
    return out;
}

// ---- XML by local name: producers agree on WordprocessingML, not always on its prefix.

std::string_view localName(const char* name) {
    const std::string_view s(name);
    const auto colon = s.find(':');
    return colon == std::string_view::npos ? s : s.substr(colon + 1);
}

bool is(const pugi::xml_node& n, std::string_view name) {
    return n.type() == pugi::node_element && localName(n.name()) == name;
}

pugi::xml_node child(const pugi::xml_node& n, std::string_view name) {
    for (auto c : n.children()) { if (is(c, name)) { return c; } }
    return {};
}

std::string_view attr(const pugi::xml_node& n, std::string_view name) {
    for (auto a : n.attributes()) { if (localName(a.name()) == name) { return a.value(); } }
    return {};
}

int number(std::string_view s, int fallback) {
    if (s.empty() || s.size() > 9 || !std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; })) {
        return fallback;
    }
    return std::stoi(std::string(s));
}

// An on/off property (w:i, w:b, w:pageBreakBefore): set when present, unless its value switches it off.
std::optional<bool> toggle(const pugi::xml_node& props, std::string_view name) {
    const auto p = child(props, name);
    if (!p) { return std::nullopt; }
    const auto v = attr(p, "val");
    return !(v == "0" || v == "false" || v == "off");
}

bool hasDescendant(const pugi::xml_node& n, std::string_view name) {
    return static_cast<bool>(n.find_node([name](const pugi::xml_node& d) { return is(d, name); }));
}

// ---- The zip.

struct Zip {
    mz_zip_archive z{};
    bool open = false;
    explicit Zip(std::string_view bytes) { open = mz_zip_reader_init_mem(&z, bytes.data(), bytes.size(), 0); }
    ~Zip() { if (open) { mz_zip_reader_end(&z); } }
    Zip(const Zip&) = delete;
    Zip& operator=(const Zip&) = delete;

    enum class Got { ok, absent, tooLarge, bad };
    Got read(int index, std::string& out) {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&z, static_cast<mz_uint>(index), &st)) { return Got::bad; }
        if (st.m_uncomp_size > kMaxPart) { return Got::tooLarge; }
        std::size_t size = 0;
        void* data = mz_zip_reader_extract_to_heap(&z, static_cast<mz_uint>(index), &size, 0);
        if (data == nullptr) { return Got::bad; }
        out.assign(static_cast<const char*>(data), size);
        mz_free(data);
        return Got::ok;
    }
    Got read(const char* name, std::string& out) {
        const int index = mz_zip_reader_locate_file(&z, name, nullptr, 0);
        return index < 0 ? Got::absent : read(index, out);
    }
};

bool parse(pugi::xml_document& doc, const std::string& xml) {
    // Whitespace-only text is kept: `<w:t xml:space="preserve"> </w:t>` is the space between two words. No DTD is read.
    return static_cast<bool>(doc.load_buffer(xml.data(), xml.size(), pugi::parse_default | pugi::parse_ws_pcdata));
}

// ---- styles.xml (Q3: by name, following basedOn).

enum class Kind { body, title, subtitle, author, heading, sceneBreak };

struct Style {
    std::string name, basedOn;
    pugi::xml_node pPr, rPr;
};

struct Styles {
    std::map<std::string, Style> byID;
    std::string defaultParagraph;

    // The chain from `id` (or the default paragraph style) up its basedOn, nearest first; bounded against a cycle.
    std::vector<const Style*> chain(std::string id) const {
        if (id.empty()) { id = defaultParagraph; }
        std::vector<const Style*> out;
        for (int i = 0; i < 16 && !id.empty(); ++i) {
            const auto it = byID.find(id);
            if (it == byID.end()) { break; }
            out.push_back(&it->second);
            id = it->second.basedOn;
        }
        return out;
    }
};

Styles readStyles(const pugi::xml_node& root) {
    Styles s;
    for (auto n : root.children()) {
        if (!is(n, "style")) { continue; }
        Style st;
        st.name = lower(attr(child(n, "name"), "val"));
        st.basedOn = attr(child(n, "basedOn"), "val");
        st.pPr = child(n, "pPr");
        st.rPr = child(n, "rPr");
        const std::string id(attr(n, "styleId"));
        if (attr(n, "type") == "paragraph" && (attr(n, "default") == "1" || attr(n, "default") == "true")) {
            s.defaultParagraph = id;
        }
        s.byID.emplace(id, std::move(st));
    }
    return s;
}

std::optional<std::pair<Kind, int>> kindOfName(const std::string& name) {
    if (name == "title") { return std::pair{Kind::title, 0}; }
    if (name == "subtitle") { return std::pair{Kind::subtitle, 0}; }
    if (name == "author") { return std::pair{Kind::author, 0}; }
    if (name == "scene break") { return std::pair{Kind::sceneBreak, 0}; }
    if (name == "heading") { return std::pair{Kind::heading, 1}; }   // Pages' first heading style
    if (name.size() == 9 && name.starts_with("heading ") && name[8] >= '1' && name[8] <= '6') {
        return std::pair{Kind::heading, name[8] - '0'};
    }
    return std::nullopt;
}

// What a paragraph's style chain says, nearest style first.
struct ParaStyle {
    Kind kind = Kind::body;
    int level = 0;
    int outline = -1;
    bool pageBreakBefore = false;
    std::string jc;
    int numID = 0, ilvl = 0;
    std::optional<bool> italic, bold;
};

ParaStyle resolve(const Styles& styles, const std::string& id) {
    ParaStyle p;
    bool named = false, outline = false, pbb = false, jc = false, num = false, i = false, b = false;
    for (const Style* s : styles.chain(id)) {
        if (!named) {
            if (auto k = kindOfName(s->name)) { p.kind = k->first; p.level = k->second; named = true; }
        }
        if (s->pPr) {
            if (const auto o = child(s->pPr, "outlineLvl"); o && !outline) { p.outline = number(attr(o, "val"), -1); outline = true; }
            if (auto v = toggle(s->pPr, "pageBreakBefore"); v && !pbb) { p.pageBreakBefore = *v; pbb = true; }
            if (const auto j = child(s->pPr, "jc"); j && !jc) { p.jc = attr(j, "val"); jc = true; }
            if (const auto n = child(s->pPr, "numPr"); n && !num) {
                p.numID = number(attr(child(n, "numId"), "val"), 0);
                p.ilvl = number(attr(child(n, "ilvl"), "val"), 0);
                num = true;
            }
        }
        if (s->rPr) {
            if (auto v = toggle(s->rPr, "i"); v && !i) { p.italic = v; i = true; }
            if (auto v = toggle(s->rPr, "b"); v && !b) { p.bold = v; b = true; }
        }
    }
    return p;
}

// ---- numbering.xml: which lists are ordered, and where they start.

struct Numbering {
    std::map<int, int> abstractOf;                              // numId → abstractNumId
    std::map<int, std::map<int, int>> startOverride;            // numId → ilvl → start
    std::map<int, std::map<int, std::pair<bool, int>>> levels;  // abstractNumId → ilvl → (ordered, start)

    std::pair<bool, int> level(int numID, int ilvl) const {
        std::pair<bool, int> out{false, 1};
        if (const auto a = abstractOf.find(numID); a != abstractOf.end()) {
            if (const auto l = levels.find(a->second); l != levels.end()) {
                if (const auto v = l->second.find(ilvl); v != l->second.end()) { out = v->second; }
            }
        }
        if (const auto o = startOverride.find(numID); o != startOverride.end()) {
            if (const auto v = o->second.find(ilvl); v != o->second.end()) { out.second = v->second; }
        }
        return out;
    }
};

Numbering readNumbering(const pugi::xml_node& root) {
    Numbering n;
    for (auto c : root.children()) {
        if (is(c, "abstractNum")) {
            const int id = number(attr(c, "abstractNumId"), -1);
            for (auto l : c.children()) {
                if (!is(l, "lvl")) { continue; }
                const auto fmt = attr(child(l, "numFmt"), "val");
                n.levels[id][number(attr(l, "ilvl"), 0)] = {!fmt.empty() && fmt != "bullet" && fmt != "none",
                                                            number(attr(child(l, "start"), "val"), 1)};
            }
        } else if (is(c, "num")) {
            const int id = number(attr(c, "numId"), -1);
            n.abstractOf[id] = number(attr(child(c, "abstractNumId"), "val"), -1);
            for (auto o : c.children()) {
                if (!is(o, "lvlOverride")) { continue; }
                if (const auto s = child(o, "startOverride")) {
                    n.startOverride[id][number(attr(o, "ilvl"), 0)] = number(attr(s, "val"), 1);
                }
            }
        }
    }
    return n;
}

// ---- Markdown that keeps Word's text literal.

// The marks Scrivi's Markdown would read as markup anywhere in a line.
std::string escapedText(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '*' || c == '_' || c == '`') { out += '\\'; }
        out += c;
    }
    return out;
}

// A line that would open a heading, a quote, a list or a rule is escaped at its start.
std::string escapedLineStart(std::string line) {
    if (line.empty()) { return line; }
    const char c = line[0];
    const bool rule = (c == '-' || c == '=') && line.find_first_not_of(std::string(1, c) + " ") == std::string::npos;
    if (c == '#' || c == '>' || rule ||
        ((c == '-' || c == '+') && (line.size() == 1 || line[1] == ' ' || line[1] == '\t'))) {
        return "\\" + line;
    }
    std::size_t k = 0;
    while (k < line.size() && k < 9 && line[k] >= '0' && line[k] <= '9') { ++k; }
    if (k > 0 && k < line.size() && (line[k] == '.' || line[k] == ')') && (k + 1 == line.size() || line[k + 1] == ' ')) {
        line.insert(k, "\\");
    }
    return line;
}

struct Segment {
    std::string text;
    bool bold = false, italic = false;
};

// Runs → one Markdown paragraph: emphasis per piece of a line, spaces kept outside the marks, each line trimmed.
std::string inlineMarkdown(const std::vector<Segment>& runs) {
    std::vector<Segment> merged;
    for (const auto& r : runs) {
        if (!merged.empty() && merged.back().bold == r.bold && merged.back().italic == r.italic) { merged.back().text += r.text; }
        else { merged.push_back(r); }
    }
    std::string out;
    for (const auto& seg : merged) {
        const std::string mark = seg.bold && seg.italic ? "***" : seg.bold ? "**" : seg.italic ? "*" : "";
        std::size_t start = 0;
        while (start <= seg.text.size()) {
            const std::size_t end = std::min(seg.text.find('\n', start), seg.text.size());
            const std::string piece = escapedText(std::string_view(seg.text).substr(start, end - start));
            const std::string_view core = trim(piece);
            if (mark.empty() || core.empty()) { out += piece; }
            else {
                const std::size_t lead = static_cast<std::size_t>(core.data() - piece.data());
                out += piece.substr(0, lead) + mark + std::string(core) + mark + piece.substr(lead + core.size());
            }
            if (end == seg.text.size()) { break; }
            out += '\n';
            start = end + 1;
        }
    }
    std::string result;
    std::size_t start = 0;
    while (start <= out.size()) {
        const std::size_t end = std::min(out.find('\n', start), out.size());
        const std::string_view line = trim(std::string_view(out).substr(start, end - start));
        if (!line.empty()) { result += (result.empty() ? "" : "\n") + escapedLineStart(std::string(line)); }
        if (end == out.size()) { break; }
        start = end + 1;
    }
    return result;
}

std::string oneLine(std::string_view s) {
    std::string out(trim(s));
    std::replace(out.begin(), out.end(), '\n', ' ');
    return out;
}

// ---- The walk.

struct Notes {
    std::map<std::string, ImportNote> byKind;
    std::vector<std::string> order;
    void add(const std::string& kind, std::string_view example, std::size_t count = 1) {
        auto [it, inserted] = byKind.try_emplace(kind, ImportNote{kind, 0, shortened(example)});
        if (inserted) { order.push_back(kind); }
        it->second.count += count;
    }
};

struct Writer {
    const Styles& styles;
    const Numbering& numbering;
    std::string md;
    Notes notes;
    bool started = false, pendingPage = false;
    int prevList = 0;   // the list the last paragraph was an item of; 0 when none
    int titlePage = 0;   // 0 nothing yet; 1 after the Title; 2 after the Subtitle; -1 past the title page
    std::map<std::pair<int, int>, int> counters;
    // Q4: the face and size each body paragraph is in; the document's own (the most common) is not formatting.
    struct Faces { std::set<std::string> keys; bool other = false; std::string example; };
    std::vector<Faces> faces;
    std::map<std::string, std::size_t> faceChars;

    // An item of the same list as the paragraph before stays on the next line, so the two are one Markdown list.
    void emit(const std::string& block, int list = 0) {
        if (!md.empty()) { md += list != 0 && list == prevList ? "\n" : "\n\n"; }
        md += block;
        prevList = list;
        started = true;
    }
    void sceneBreak() {
        if (started) { emit("* * *"); }
        pendingPage = false;
    }

    // One paragraph's runs, with what it holds that Scrivi cannot.
    struct Collected {
        std::vector<Segment> runs;
        std::vector<std::string> notes;
        bool formatted = false;
        bool pageBefore = false, pageAfter = false;
        std::set<std::string> faceKeys;
        std::size_t chars = 0;
    };

    void run(const pugi::xml_node& r, const ParaStyle& ps, Collected& c) {
        const auto rPr = child(r, "rPr");
        std::optional<bool> italic = ps.italic, bold = ps.bold;
        if (const auto rs = attr(child(rPr, "rStyle"), "val"); !rs.empty()) {
            for (const Style* s : styles.chain(std::string(rs))) {
                if (!s->rPr) { continue; }
                if (auto v = toggle(s->rPr, "i")) { italic = v; break; }
            }
            for (const Style* s : styles.chain(std::string(rs))) {
                if (!s->rPr) { continue; }
                if (auto v = toggle(s->rPr, "b")) { bold = v; break; }
            }
        }
        if (auto v = toggle(rPr, "i")) { italic = v; }
        if (auto v = toggle(rPr, "b")) { bold = v; }
        if (rPr) {
            const auto u = child(rPr, "u");
            const auto hl = attr(child(rPr, "highlight"), "val");
            const auto color = lower(attr(child(rPr, "color"), "val"));
            if ((u && attr(u, "val") != "none") || toggle(rPr, "strike").value_or(false) ||
                toggle(rPr, "dstrike").value_or(false) || toggle(rPr, "caps").value_or(false) ||
                toggle(rPr, "smallCaps").value_or(false) || (!hl.empty() && hl != "none") ||
                (!color.empty() && color != "auto" && color != "000000") ||
                (child(rPr, "vertAlign") && attr(child(rPr, "vertAlign"), "val") != "baseline")) {
                c.formatted = true;
            }
        }
        std::string text;
        for (auto k : r.children()) {
            if (is(k, "t")) { text += k.text().get(); }
            else if (is(k, "tab") || is(k, "ptab")) { text += '\t'; }
            else if (is(k, "br")) {
                const auto type = attr(k, "type");
                if (type == "page") {
                    if (c.chars == 0 && text.empty()) { c.pageBefore = true; } else { c.pageAfter = true; }
                } else if (type != "column") {
                    text += '\n';
                }
            }
            else if (is(k, "cr")) { text += '\n'; }
            else if (is(k, "noBreakHyphen")) { text += '-'; }
            else if (is(k, "drawing") || is(k, "pict") || is(k, "object")) {
                c.notes.push_back(hasDescendant(k, "txbxContent") ? "textBox" : "picture");
            }
            else if (is(k, "footnoteReference")) { c.notes.push_back("footnote"); }
            else if (is(k, "endnoteReference")) { c.notes.push_back("endnote"); }
            else if (is(k, "commentReference")) { c.notes.push_back("comment"); }
            // delText (tracked deletions), instrText (field codes), softHyphen, sym and the rest carry no text to keep.
        }
        if (text.empty()) { return; }
        if (!trim(text).empty()) {
            const std::string face = std::string(attr(child(rPr, "rFonts"), "ascii")) + "|" +
                                     std::string(attr(child(rPr, "sz"), "val"));
            c.faceKeys.insert(face);
            faceChars[face] += text.size();
        }
        c.chars += text.size();
        c.runs.push_back({std::move(text), bold.value_or(false), italic.value_or(false)});
    }

    void collect(const pugi::xml_node& n, const ParaStyle& ps, Collected& c) {
        for (auto k : n.children()) {
            if (is(k, "r")) { run(k, ps, c); }
            else if (is(k, "hyperlink")) {
                if (!attr(k, "id").empty()) { c.notes.push_back("link"); }
                collect(k, ps, c);
            }
            // Tracked insertions read as accepted; deletions (del, moveFrom) are left out.
            else if (is(k, "ins") || is(k, "moveTo") || is(k, "smartTag") || is(k, "fldSimple") || is(k, "customXml") ||
                     is(k, "dir") || is(k, "bdo")) {
                collect(k, ps, c);
            }
            else if (is(k, "sdt")) { collect(child(k, "sdtContent"), ps, c); }
            else if (is(k, "oMath") || is(k, "oMathPara")) { c.notes.push_back("equation"); }
        }
    }

    void paragraph(const pugi::xml_node& p) {
        const auto pPr = child(p, "pPr");
        ParaStyle ps = resolve(styles, std::string(attr(child(pPr, "pStyle"), "val")));
        if (const auto o = child(pPr, "outlineLvl")) { ps.outline = number(attr(o, "val"), -1); }
        if (auto v = toggle(pPr, "pageBreakBefore")) { ps.pageBreakBefore = *v; }
        if (const auto j = child(pPr, "jc")) { ps.jc = attr(j, "val"); }
        if (const auto n = child(pPr, "numPr")) {
            ps.numID = number(attr(child(n, "numId"), "val"), 0);
            ps.ilvl = std::clamp(number(attr(child(n, "ilvl"), "val"), 0), 0, 8);
        }
        if (ps.kind == Kind::body && ps.outline >= 0 && ps.outline <= 5) { ps.kind = Kind::heading; ps.level = ps.outline + 1; }

        Collected c;
        collect(p, ps, c);
        std::string plain;
        for (const auto& r : c.runs) { plain += r.text; }
        const std::string_view text = trim(plain);
        const bool sectionEnds = static_cast<bool>(child(pPr, "sectPr"));
        for (const auto& k : c.notes) { notes.add(k, text.empty() ? std::string_view("(an empty paragraph)") : text); }

        if (ps.kind == Kind::sceneBreak) { sceneBreak(); return; }   // S4's: empty, with a border
        if (text.empty()) {
            pendingPage = pendingPage || ps.pageBreakBefore || c.pageBefore || c.pageAfter || sectionEnds;
            return;
        }
        const bool heading = ps.kind == Kind::title || ps.kind == Kind::heading;
        if ((ps.pageBreakBefore || c.pageBefore || pendingPage) && !heading) { sceneBreak(); }
        pendingPage = c.pageAfter || sectionEnds;
        const bool centred = ps.jc == "center";
        if (text == "* * *" || text == "***" || (text == "#" && centred)) { sceneBreak(); return; }

        // Q1: S4's title page — Title, an optional Subtitle, then Author or a `by …` line — kept raw for S6a to consume.
        if (ps.kind == Kind::title) {
            titlePage = started ? -1 : 1;
            emit("# " + oneLine(text));
            return;
        }
        if (titlePage == 1 && ps.kind == Kind::subtitle) { emit(oneLine(text)); titlePage = 2; return; }
        if ((titlePage == 1 || titlePage == 2) && (ps.kind == Kind::author || text.starts_with("by "))) {
            const std::string line = oneLine(text);
            emit(line.starts_with("by ") ? line : "by " + line);
            titlePage = -1;
            return;
        }
        titlePage = -1;

        if (ps.kind == Kind::heading) {
            // A chapter title is stored raw; a writer's own heading is Markdown, so its text stays literal.
            emit(ps.level <= 1 ? "# " + oneLine(text) : std::string(static_cast<std::size_t>(ps.level), '#') + " " +
                                                           escapedText(oneLine(text)));
            return;
        }

        std::string body = inlineMarkdown(c.runs);
        if (body.empty()) { return; }
        const bool list = ps.numID > 0 && numbering.abstractOf.contains(ps.numID);
        if (list) {
            const auto [ordered, start] = numbering.level(ps.numID, ps.ilvl);
            std::string prefix(static_cast<std::size_t>(ps.ilvl) * 2, ' ');
            if (ordered) {
                auto [it, fresh] = counters.try_emplace({ps.numID, ps.ilvl}, start);
                prefix += std::to_string(it->second++) + ". ";
            } else {
                prefix += "- ";
            }
            for (auto it = counters.begin(); it != counters.end();) {   // a deeper level restarts under a new item
                it = it->first.first == ps.numID && it->first.second > ps.ilvl ? counters.erase(it) : std::next(it);
            }
            emit(prefix + body, ps.numID);
        } else {
            emit(body);
        }
        faces.push_back({c.faceKeys, c.formatted || ps.jc == "center" || ps.jc == "right" || ps.jc == "end",
                         std::string(text)});
    }

    void blocks(const pugi::xml_node& container) {
        for (auto k : container.children()) {
            if (is(k, "p")) { paragraph(k); }
            else if (is(k, "tbl")) {
                // A table's text is kept, cell by cell, as paragraphs.
                std::string first;
                k.find_node([&first](const pugi::xml_node& d) {
                    if (is(d, "t") && !trim(d.text().get()).empty()) { first = d.text().get(); return true; }
                    return false;
                });
                notes.add("docxTable", first.empty() ? std::string_view("(an empty table)") : std::string_view(first));
                for (auto row : k.children()) {
                    if (!is(row, "tr")) { continue; }
                    for (auto cell : row.children()) { if (is(cell, "tc")) { blocks(cell); } }
                }
            }
            else if (is(k, "sdt")) { blocks(child(k, "sdtContent")); }
            else if (is(k, "customXml") || is(k, "ins") || is(k, "moveTo")) { blocks(k); }
        }
    }

    void formattingNote() {
        std::string common;
        std::size_t most = 0;
        for (const auto& [key, n] : faceChars) { if (n > most) { most = n; common = key; } }
        for (const auto& f : faces) {
            const bool otherFace = std::any_of(f.keys.begin(), f.keys.end(), [&](const std::string& k) { return k != common; });
            if (f.other || otherFace) { notes.add("formatting", f.example); }
        }
    }
};

} // namespace

bool looksLikeZip(std::string_view bytes) {
    return bytes.starts_with(std::string_view("PK\x03\x04", 4));
}

bool looksLikeLegacyDoc(std::string_view bytes) {
    return bytes.starts_with(std::string_view("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8));
}

Result<DocxMarkdown> docxToMarkdown(std::string_view bytes) {
    using R = Result<DocxMarkdown>;
    Zip zip(bytes);
    if (!zip.open) { return R::failure(notDocx()); }
    auto part = [&](const char* name, pugi::xml_document& doc, bool required) -> std::optional<Error> {
        std::string xml;
        switch (zip.read(name, xml)) {
            case Zip::Got::ok: break;
            case Zip::Got::absent: return required ? std::optional<Error>(notDocx()) : std::nullopt;
            case Zip::Got::tooLarge: return tooLarge();
            case Zip::Got::bad: return notDocx();
        }
        if (!parse(doc, xml)) { return required ? std::optional<Error>(notDocx()) : std::nullopt; }
        return std::nullopt;
    };
    pugi::xml_document document, stylesDoc, numberingDoc;
    if (auto e = part("word/document.xml", document, true)) { return R::failure(*e); }
    if (auto e = part("word/styles.xml", stylesDoc, false)) { return R::failure(*e); }
    if (auto e = part("word/numbering.xml", numberingDoc, false)) { return R::failure(*e); }
    const Styles styles = readStyles(stylesDoc.document_element());
    const Numbering numbering = readNumbering(numberingDoc.document_element());

    const auto body = child(document.document_element(), "body");
    if (!body) { return R::failure(notDocx()); }
    Writer w{styles, numbering};
    w.blocks(body);
    w.formattingNote();

    // Headers and footers are parts of their own: reported when any holds text.
    const auto files = mz_zip_reader_get_num_files(&zip.z);
    for (mz_uint i = 0; i < files; ++i) {
        char name[256];
        if (mz_zip_reader_get_filename(&zip.z, i, name, sizeof name) == 0) { continue; }
        const std::string_view n(name);
        if (!(n.starts_with("word/header") || n.starts_with("word/footer")) || !n.ends_with(".xml")) { continue; }
        std::string xml;
        pugi::xml_document doc;
        if (zip.read(static_cast<int>(i), xml) != Zip::Got::ok || !parse(doc, xml)) { continue; }
        std::string text;
        doc.find_node([&text](const pugi::xml_node& d) {
            if (is(d, "t")) { text += d.text().get(); }
            return false;
        });
        if (!trim(text).empty()) { w.notes.add("headerFooter", text); }
    }

    DocxMarkdown out;
    out.markdown = std::move(w.md);
    for (const auto& k : w.notes.order) { out.notes.push_back(w.notes.byKind[k]); }
    return R::success(std::move(out));
}

Result<ImportedManuscript> importDocx(std::string_view bytes) {
    using R = Result<ImportedManuscript>;
    auto converted = docxToMarkdown(bytes);
    if (!converted.ok()) { return R::failure(converted.error()); }
    auto imported = importManuscript(converted.value().markdown, true, false);
    if (!imported.ok()) { return imported; }
    auto& m = imported.value();
    m.encoding = "utf-8";
    for (auto& n : converted.value().notes) { m.report.push_back(std::move(n)); }
    return imported;
}

} // namespace scrivi::manuscript
