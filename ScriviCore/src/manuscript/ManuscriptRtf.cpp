#include "manuscript/ManuscriptRtf.hpp"

#include "manuscript/ManuscriptImport.hpp"   // appendUtf8, fromWindows1252

#include <algorithm>
#include <cstdint>
#include <optional>

namespace scrivi::manuscript {

namespace {

// Deeper nesting than any writer's document is read as flat text: the stack never grows past it.
constexpr std::size_t kMaxDepth = 1024;

// Destinations whose text is never the writer's.
bool skippedDestination(std::string_view w) {
    static constexpr std::string_view k[] = {
        "fonttbl", "colortbl", "stylesheet", "info", "listtable", "listoverridetable", "revtbl", "rsidtbl", "generator",
        "xmlnstbl", "themedata", "colorschememapping", "latentstyles", "datastore", "mmathPr", "pict", "object", "shp",
        "nonshppict", "header", "headerl", "headerr", "headerf", "footer", "footerl", "footerr", "footerf", "footnote",
        "annotation", "atnid", "atnauthor", "atnref", "fldinst", "bkmkstart", "bkmkend", "pgdsctbl", "filetbl"};
    return std::find(std::begin(k), std::end(k), w) != std::end(k);
}

struct State {
    bool bold = false, italic = false;
    bool underline = false, strike = false, caps = false, script = false;
    int font = 0, size = 24, colour = 0;
    int uc = 1;                 // characters to skip after a \uN
    bool skip = false;          // inside a destination whose text is not kept
    bool fieldInstruction = false;
    bool colourTable = false;
    bool countPictures = true;  // false inside \nonshppict (a fallback copy of a picture already counted)
};

struct Reader {
    std::string_view in;
    std::size_t i = 0;
    std::vector<State> stack{State{}};
    RtfText out;
    RtfParagraph para;
    bool centred = false, inTable = false, prevTable = false;
    std::size_t pendingSkip = 0;          // characters still to skip after a \uN
    std::uint32_t highSurrogate = 0;
    std::string instruction;              // the field instruction being read
    std::vector<std::uint32_t> colours;     // the colour table, RGB; entry 0 (before the first `;`) is "auto"
    std::uint32_t colourBuilding = 0;
    bool groupStart = false, ignorable = false;

    State& st() { return stack.back(); }

    bool colourIsPlain(int index) const {
        if (index <= 0 || static_cast<std::size_t>(index) >= colours.size()) { return true; }
        return colours[static_cast<std::size_t>(index)] == 0;   // black
    }

    void text(std::string_view s) {
        if (s.empty()) { return; }
        State& t = st();
        if (t.fieldInstruction) { instruction += s; return; }
        if (t.skip) { return; }
        if (!para.runs.empty() && para.runs.back().bold == t.bold && para.runs.back().italic == t.italic) {
            para.runs.back().text += s;
        } else {
            para.runs.push_back({std::string(s), t.bold, t.italic});
        }
        if (s.find_first_not_of(" \t\n") == std::string_view::npos) { return; }
        if (t.underline || t.strike || t.caps || t.script || !colourIsPlain(t.colour)) { para.formatted = true; }
        // Only plain runs say what the face is: macOS writes italic and bold in their own fonts (Palatino-Italic).
        if (t.bold || t.italic) { return; }
        const std::string face = std::to_string(t.font) + "|" + std::to_string(t.size);
        para.faces.insert(face);
        out.faceChars[face] += s.size();
    }

    void codePoint(std::uint32_t cp) {
        if (cp >= 0xD800 && cp < 0xDC00) { highSurrogate = cp; return; }
        if (cp >= 0xDC00 && cp < 0xE000) {
            cp = highSurrogate != 0 ? 0x10000 + ((highSurrogate - 0xD800) << 10) + (cp - 0xDC00) : 0xFFFD;
        }
        highSurrogate = 0;
        std::string s;
        appendUtf8(s, cp);
        text(s);
    }

    void endParagraph() {
        para.table = inTable;
        if (inTable && !prevTable) { ++out.tables; }
        if (centred) { para.formatted = true; }
        const bool visible = std::any_of(para.runs.begin(), para.runs.end(), [](const Segment& r) {
            return r.text.find_first_not_of(" \t\n") != std::string::npos;
        });
        if (visible) {
            prevTable = inTable;
            out.paragraphs.push_back(std::move(para));
        }
        para = RtfParagraph{};
    }

    void endInstruction() {
        // `HYPERLINK "scrivlnk://…"` → the quoted target; any other field instruction is not a link.
        const auto h = instruction.find("HYPERLINK");
        if (h != std::string::npos) {
            const auto a = instruction.find('"', h);
            const auto b = a == std::string::npos ? a : instruction.find('"', a + 1);
            if (b != std::string::npos) { out.links.push_back(instruction.substr(a + 1, b - a - 1)); }
        }
        instruction.clear();
    }

    void word(std::string_view w, std::optional<long> param) {
        State& t = st();
        const long p = param.value_or(1);
        const bool first = groupStart;
        groupStart = false;
        if (w == "pict" && t.countPictures) { ++out.pictures; }
        if (w == "footnote") { ++out.footnotes; }
        if (w == "annotation") { ++out.annotations; }
        if (first && (skippedDestination(w) || ignorable)) {
            t.skip = true;
            if (w == "fldinst") { t.fieldInstruction = true; instruction.clear(); }
            if (w == "colortbl") { t.colourTable = true; colourBuilding = 0; }
            if (w == "nonshppict") { t.countPictures = false; }
            ignorable = false;
            return;
        }
        ignorable = false;
        if (t.colourTable) {
            const auto c = static_cast<std::uint32_t>(std::clamp(p, 0L, 255L));
            if (w == "red") { colourBuilding = (colourBuilding & 0x00FFFF) | (c << 16); }
            else if (w == "green") { colourBuilding = (colourBuilding & 0xFF00FF) | (c << 8); }
            else if (w == "blue") { colourBuilding = (colourBuilding & 0xFFFF00) | c; }
            return;
        }
        if (t.skip) { return; }   // a skipped destination's own words (a footnote's \par) are not the text's
        if (w == "par" || w == "sect" || w == "page") { endParagraph(); }
        else if (w == "cell") { endParagraph(); }
        else if (w == "line") { text("\n"); }
        else if (w == "tab") { text("\t"); }
        else if (w == "pard") { centred = false; inTable = false; }
        else if (w == "intbl") { inTable = true; }
        else if (w == "qc" || w == "qr") { centred = true; }
        else if (w == "ql" || w == "qj") { centred = false; }
        else if (w == "plain") {
            const State keep = t;
            t = State{};
            t.uc = keep.uc; t.skip = keep.skip; t.fieldInstruction = keep.fieldInstruction;
            t.colourTable = keep.colourTable; t.countPictures = keep.countPictures;
        }
        else if (w == "b") { t.bold = p != 0; }
        else if (w == "i") { t.italic = p != 0; }
        else if (w == "ul" || w == "uld" || w == "uldb" || w == "ulw" || w == "ulwave" || w == "ulth" || w == "uldash") {
            t.underline = p != 0;
        }
        else if (w == "ulnone") { t.underline = false; }
        else if (w == "strike" || w == "striked") { t.strike = p != 0; }
        else if (w == "caps" || w == "scaps") { t.caps = p != 0; }
        else if (w == "super" || w == "sub") { t.script = true; }
        else if (w == "up" || w == "dn") { t.script = p != 0; }
        else if (w == "nosupersub") { t.script = false; }
        else if (w == "f") { t.font = static_cast<int>(p); }
        else if (w == "fs") { t.size = static_cast<int>(p); }
        else if (w == "cf") { t.colour = static_cast<int>(p); }
        else if (w == "uc") { t.uc = static_cast<int>(std::clamp(p, 0L, 8L)); }
        else if (w == "u") {
            codePoint(static_cast<std::uint32_t>(p < 0 ? p + 65536 : std::min(p, 0x10FFFFL)));
            pendingSkip = static_cast<std::size_t>(t.uc);
        }
        else if (w == "emdash") { codePoint(0x2014); }
        else if (w == "endash") { codePoint(0x2013); }
        else if (w == "lquote") { codePoint(0x2018); }
        else if (w == "rquote") { codePoint(0x2019); }
        else if (w == "ldblquote") { codePoint(0x201C); }
        else if (w == "rdblquote") { codePoint(0x201D); }
        else if (w == "bullet") { codePoint(0x2022); }
        else if (w == "emspace" || w == "enspace" || w == "qmspace") { text(" "); }
    }

    void openGroup() {
        if (stack.size() >= kMaxDepth) { return; }
        stack.push_back(st());
        groupStart = true;
        ignorable = false;
    }

    void closeGroup() {
        if (stack.size() <= 1) { return; }
        if (st().fieldInstruction && !stack[stack.size() - 2].fieldInstruction) { endInstruction(); }
        stack.pop_back();
        groupStart = false;
    }

    void run() {
        out.rtf = in.starts_with("{\\rtf");
        if (!out.rtf) { return; }
        while (i < in.size()) {
            const char c = in[i];
            if (c == '{') { ++i; openGroup(); continue; }
            if (c == '}') { ++i; closeGroup(); continue; }
            if (c == '\r' || c == '\n') { ++i; continue; }
            if (c != '\\') {
                const std::size_t start = i;
                while (i < in.size() && in[i] != '\\' && in[i] != '{' && in[i] != '}' && in[i] != '\r' && in[i] != '\n') { ++i; }
                groupStart = false;
                plain(in.substr(start, i - start));
                continue;
            }
            ++i;   // the backslash
            if (i >= in.size()) { break; }
            const char d = in[i];
            if ((d >= 'a' && d <= 'z') || (d >= 'A' && d <= 'Z')) {
                const std::size_t start = i;
                while (i < in.size() && i - start < 32 && ((in[i] >= 'a' && in[i] <= 'z') || (in[i] >= 'A' && in[i] <= 'Z'))) { ++i; }
                const std::string_view w = in.substr(start, i - start);
                std::optional<long> param;
                bool negative = false;
                if (i < in.size() && in[i] == '-') { negative = true; ++i; }
                long v = 0;
                std::size_t digits = 0;
                while (i < in.size() && in[i] >= '0' && in[i] <= '9') {
                    if (digits++ < 9) { v = v * 10 + (in[i] - '0'); }
                    ++i;
                }
                if (digits > 0) { param = negative ? -v : v; }
                if (i < in.size() && in[i] == ' ') { ++i; }
                word(w, param);
                continue;
            }
            ++i;
            groupStart = false;
            switch (d) {
                case '\'': {
                    if (i + 2 > in.size()) { i = in.size(); break; }
                    const auto hex = [](char h) -> int {
                        return h >= '0' && h <= '9' ? h - '0' : h >= 'a' && h <= 'f' ? h - 'a' + 10 : h >= 'A' && h <= 'F' ? h - 'A' + 10 : -1;
                    };
                    const int hi = hex(in[i]), lo = hex(in[i + 1]);
                    i += 2;
                    if (hi < 0 || lo < 0) { break; }
                    if (pendingSkip > 0) { --pendingSkip; break; }
                    const char byte = static_cast<char>(hi * 16 + lo);
                    text(fromWindows1252(std::string_view(&byte, 1)));
                    break;
                }
                case '\\': case '{': case '}': plain(std::string_view(&in[i - 1], 1)); break;
                case '~': codePoint(0x00A0); break;
                case '_': text("-"); break;
                case '*': ignorable = true; groupStart = true; break;
                case '\r': case '\n': endParagraph(); break;
                default: break;   // \- (optional hyphen), \: and the rest carry no text
            }
        }
        endParagraph();
    }

    // Literal characters: a \uN's fallback is skipped; the colour table's `;` ends an entry.
    void plain(std::string_view s) {
        while (pendingSkip > 0 && !s.empty()) { s.remove_prefix(1); --pendingSkip; }
        if (st().colourTable) {
            for (char c : s) {
                if (c == ';') { colours.push_back(colourBuilding); colourBuilding = 0; }
            }
            return;
        }
        text(fromWindows1252(s));
    }
};

} // namespace

RtfText readRtf(std::string_view rtf) {
    Reader r{rtf};
    r.run();
    return std::move(r.out);
}

} // namespace scrivi::manuscript
