#include "markdown/Editing.hpp"

#include "markdown/UnicodeClasses.hpp"

#include <algorithm>
#include <cstdio>
#include <set>
#include <string>

namespace scrivi::markdown::editing {

namespace em = scrivi::markdown::emphasis;
using em::Token;

namespace {

bool isSpaceUnit(char16_t c) { return c == u' ' || c == u'\t' || c == u'\n'; }

// Apple: `CharacterSet.alphanumerics` — a surrogate half counts as part of a letter.
bool isWordUnitApple(char16_t c) {
    if (c >= 0xD800 && c <= 0xDFFF) { return true; }
    return unicode::isAlphanumeric(c);
}
// Apple: `CharacterSet.punctuationCharacters ∪ symbols` — a surrogate half is not.
bool isPunctUnitApple(char16_t c) {
    if (c >= 0xD800 && c <= 0xDFFF) { return false; }
    return unicode::isPunctuationOrSymbol(c);
}

Units decimal(int n) {
    const std::string s = std::to_string(n);
    return Units(s.begin(), s.end());
}

bool blankOrNewlines(UnitsView u) {
    for (const char16_t c : u) {
        if (c != u' ' && c != u'\t' && c != u'\n' && c != u'\r') { return false; }
    }
    return true;
}

Units trimSpaces(UnitsView u) {
    std::size_t a = 0, b = u.size();
    while (a < b && (u[a] == u' ' || u[a] == u'\t')) { ++a; }
    while (b > a && (u[b - 1] == u' ' || u[b - 1] == u'\t')) { --b; }
    return Units(u.substr(a, b - a));
}

// THE REWRITE CORE (shared by edits and the Format commands): replace `range` by `inserted` tokens and write the stretch — the
// edit plus every span it touches — back with the fewest markers. Checked by the parser IN CONTEXT before it is applied. If it
// would not read back as intended: an EDIT is written without emphasis there (formatting lost, but no stray marker shows); a
// COMMAND (`refuseIfUnbalanced`) is refused instead — a command must never destroy formatting.
struct Rewrite {
    URange range;
    Units text;
    int caret = 0;            // the output offset just after the last inserted token
    std::vector<int> ends;    // ends[i]: the output offset just after output token i
    int leftCount = 0;
};

std::optional<Rewrite> rewrite(const Scene& s, URange range, const std::vector<Token>& inserted,
                               const std::vector<URange>& spans, bool refuseIfUnbalanced) {
    const int a = range.location, b = range.end();
    int w0 = a, w1 = b;
    for (const auto& sp : spans) {
        if (sp.location <= b && sp.end() >= a) { w0 = std::min(w0, sp.location); w1 = std::max(w1, sp.end()); }
    }
    const auto left = s.tokensIn({w0, a - w0});
    const auto right = s.tokensIn({b, w1 - b});
    const auto touched = s.blocksIn(w0, w1);
    const int r0 = touched.empty() ? w0 : std::min(touched.front().location, w0);
    const int r1 = touched.empty() ? w1 : std::max(touched.back().end(), w1);
    const auto before = s.tokensIn({r0, w0 - r0});
    const auto after = s.tokensIn({w1, r1 - w1});
    // Normalise WITH one token of context each side: whether a span edge can sit on punctuation depends on the character just
    // outside the stretch. The context itself is never rewritten.
    std::vector<Token> all;
    if (!before.empty()) { all.push_back(before.back()); }
    all.insert(all.end(), left.begin(), left.end());
    all.insert(all.end(), inserted.begin(), inserted.end());
    all.insert(all.end(), right.begin(), right.end());
    if (!after.empty()) { all.push_back(after.front()); }
    all = em::normalize(std::move(all));
    if (!before.empty()) { all.erase(all.begin()); }
    if (!after.empty()) { all.pop_back(); }
    auto ser = em::serialize(all);
    Units probe(s.slice({r0, w0 - r0}));
    probe += ser.text;
    probe += s.slice({w1, r1 - w1});
    std::vector<Token> expected = before;
    expected.insert(expected.end(), all.begin(), all.end());
    expected.insert(expected.end(), after.begin(), after.end());
    if (!em::sameRendering(expected, em::tokens(probe))) {
        if (refuseIfUnbalanced) { return std::nullopt; }
        for (auto& t : all) { t.style = 0; }
        ser = em::serialize(all);
    }
    Rewrite r;
    r.range = {w0, w1 - w0};
    r.text = std::move(ser.text);
    r.ends = std::move(ser.ends);
    r.leftCount = static_cast<int>(left.size());
    const int lastInserted = static_cast<int>(left.size() + inserted.size()) - 1;
    r.caret = lastInserted >= 0 && !r.ends.empty() ? r.ends[static_cast<std::size_t>(lastInserted)] : 0;
    return r;
}

enum class Kind { body, heading, bullet, numbered };
struct ParagraphKind {
    Kind kind = Kind::body;
    int level = 0;
    bool operator==(const ParagraphKind&) const = default;
};
struct KindAndPrefix {
    ParagraphKind kind;
    std::optional<URange> prefix;   // storage range of the first line's prefix
};

// The paragraph kind of a block (its first line) and that line's prefix.
KindAndPrefix kindOf(URange b, const Analysis16& a) {
    for (const auto& h : a.headings) {
        if (h.line.location == 0) { return {{Kind::heading, h.level}, URange{b.location + h.prefix.location, h.prefix.length}}; }
    }
    for (const auto& li : a.listItems) {
        if (li.line.location == 0) {
            return {{li.ordered ? Kind::numbered : Kind::bullet, 0}, URange{b.location + li.prefix.location, li.prefix.length}};
        }
    }
    return {{Kind::body, 0}, std::nullopt};
}

const ListItem16* firstLineItem(const Analysis16& a) {
    for (const auto& li : a.listItems) { if (li.line.location == 0) { return &li; } }
    return nullptr;
}

// The text from `from` to the end of the ordered items that follow block `b`, renumbered from `number` — and where it ends.
std::pair<Units, int> renumbered(const Scene& s, URange b, int from, int number) {
    int end = from;
    int p = b.end();
    Units tail;
    while (auto next = s.nextBlockAfter(p)) {
        const ListItem16* nli = firstLineItem(s.info(*next));
        if (nli == nullptr || !nli->ordered) { break; }
        tail += s.slice({end, next->location - end});
        Units text(s.slice(*next));
        text.replace(static_cast<std::size_t>(nli->prefix.location), static_cast<std::size_t>(nli->prefix.length),
                     decimal(number) + u". ");
        tail += text;
        ++number;
        end = next->end();
        p = end;
    }
    return {tail, end};
}

} // namespace

// MARK: — Scene

Scene::Line Scene::lineContaining(int loc) const {
    // NSString.paragraphRange: from after the previous newline through the next newline (inclusive).
    const int n = length();
    const int at = std::min(loc, n);
    int start = at;
    while (start > 0 && text_[static_cast<std::size_t>(start - 1)] != u'\n') { --start; }
    int end = at;
    while (end < n && text_[static_cast<std::size_t>(end)] != u'\n') { ++end; }
    if (end < n) { ++end; }
    // A scene whose text has no final newline ends on its last line: in the manuscript, the divider follows it directly.
    const bool endsScene = end == n && n > 0 && text_[static_cast<std::size_t>(n - 1)] != u'\n' && end > start;
    return {{start, end - start}, endsScene};
}

bool Scene::isBlank(URange r) const {
    for (int i = r.location; i < r.end(); ++i) {
        const char16_t c = at(i);
        if (c != u' ' && c != u'\t' && c != u'\n') { return false; }
    }
    return true;
}

std::optional<URange> Scene::blockAt(int loc) const {
    if (length() == 0) { return std::nullopt; }
    const Line first = lineContaining(loc);
    if (isBlank(first.line)) { return std::nullopt; }
    if (first.endsScene && loc >= first.line.end()) { return std::nullopt; }
    URange start = first.line;
    while (start.location > 0) {
        const Line prev = lineContaining(start.location - 1);
        if (prev.endsScene || isBlank(prev.line)) { break; }
        start = prev.line;
    }
    Line end = first;
    while (!end.endsScene && end.line.end() < length()) {
        const Line next = lineContaining(end.line.end());
        if (isBlank(next.line)) { break; }
        end = next;
    }
    return URange{start.location, end.line.end() - start.location};
}

const Analysis16& Scene::info(URange block) const {
    Units key(slice(block));
    auto it = cache_.find(key);
    if (it == cache_.end()) { it = cache_.emplace(key, analyzeUnits(key)).first; }
    return it->second;
}

std::vector<URange> Scene::blocksIn(int a, int b) const {
    std::vector<URange> out;
    int p = a;
    while (p <= b && p <= length()) {
        const auto blk = blockAt(p);
        if (blk && blk->end() >= p) {
            if (out.empty() || !(out.back() == *blk)) { out.push_back(*blk); }
            p = std::max(blk->end(), p + 1);
        } else {
            ++p;
        }
    }
    return out;
}

std::vector<Token> Scene::tokensIn(URange r) const {
    std::vector<Token> out;
    int p = r.location;
    const int end = r.end();
    while (p < end) {
        const auto b = blockAt(p);
        if (b && b->contains(p)) {
            const int stop = std::min(end, b->end());
            const auto more = em::tokens(slice(*b), info(*b), p - b->location, stop - b->location);
            out.insert(out.end(), more.begin(), more.end());
            p = stop;
        } else {
            out.push_back({at(p), 0, false});
            ++p;
        }
    }
    return out;
}

std::vector<URange> Scene::spansIn(int a, int b) const {
    std::vector<URange> out;
    for (const auto& blk : blocksIn(a, b)) {
        for (const auto& sp : info(blk).spans) { out.push_back({blk.location + sp.location, sp.length}); }
    }
    return out;
}

std::optional<URange> Scene::nextBlockAfter(int p) const {
    int q = p;
    while (q < length()) {
        const char16_t c = at(q);
        if (c != u'\n' && c != u' ' && c != u'\t') { break; }
        ++q;
    }
    if (q >= length()) { return std::nullopt; }
    return blockAt(q);
}

std::optional<URange> Scene::previousBlockBefore(int p) const {
    int q = p - 1;
    while (q >= 0) {
        const char16_t c = at(q);
        if (c != u'\n' && c != u' ' && c != u'\t') { break; }
        --q;
    }
    if (q < 0) { return std::nullopt; }
    return blockAt(q);
}

// MARK: — Words

std::optional<URange> wordRange(const Scene& s, int caret) {
    const int n = s.length();
    auto word = [&](int i) { return i >= 0 && i < n && isWordUnitApple(s.at(i)); };
    if (!word(caret - 1) && !word(caret)) { return std::nullopt; }
    int a = caret, e = caret;
    while (true) {
        if (word(a - 1)) { --a; continue; }
        if (a >= 3 && isPunctUnitApple(s.at(a - 1)) && s.at(a - 2) == u'\\' && word(a - 3) && word(a)) { a -= 2; continue; }
        break;
    }
    while (true) {
        if (word(e)) { ++e; continue; }
        if (e + 2 < n && s.at(e) == u'\\' && isPunctUnitApple(s.at(e + 1)) && word(e + 2) && word(e - 1)) { e += 2; continue; }
        break;
    }
    return URange{a, e - a};
}

// MARK: — Bold / Italic

std::optional<Edit> emphasisEdit(const Scene& s, URange sel, std::uint8_t bit, std::optional<int> caret) {
    if (sel.length <= 0) { return std::nullopt; }
    const auto mid = s.tokensIn(sel);
    bool anyVisible = false, allHave = true;
    for (const auto& t : mid) {
        if (t.isPrefix || isSpaceUnit(t.unit)) { continue; }
        anyVisible = true;
        if ((t.style & bit) == 0) { allHave = false; }
    }
    if (!anyVisible) { return std::nullopt; }
    std::vector<Token> changed = mid;
    for (auto& t : changed) {
        if (!t.isPrefix) {
            t.style = allHave ? static_cast<std::uint8_t>(t.style & ~bit) : static_cast<std::uint8_t>(t.style | bit);
        }
    }
    const int a = sel.location, b = sel.end();
    const auto r = rewrite(s, sel, changed, s.spansIn(a, b), true);
    if (!r) { return std::nullopt; }
    // The selection after: the formatted VISIBLE text, without the edge whitespace the span left out.
    int firstVisible = -1, lastVisible = -1;
    for (int i = 0; i < static_cast<int>(changed.size()); ++i) {
        if (!isSpaceUnit(changed[static_cast<std::size_t>(i)].unit) && !changed[static_cast<std::size_t>(i)].isPrefix) {
            if (firstVisible < 0) { firstVisible = i; }
            lastVisible = i;
        }
    }
    const int first = r->ends[static_cast<std::size_t>(r->leftCount + firstVisible)] - 1;
    const int last = r->ends[static_cast<std::size_t>(r->leftCount + lastVisible)];
    URange selection{r->range.location + first, last - first};
    if (caret) {
        // The caret sat after `k` tokens of the word: put it after the same `k` in the result.
        const int k = static_cast<int>(s.tokensIn({a, std::max(0, std::min(*caret, b) - a)}).size());
        const int at = k == 0 ? first : r->ends[static_cast<std::size_t>(r->leftCount + k - 1)];
        selection = {r->range.location + at, 0};
    }
    return Edit{r->range, r->text, selection};
}

// MARK: — Headings, body, lists

std::optional<Edit> paragraphEdit(const Scene& s, URange sel, ParagraphFormat format) {
    ParagraphKind target;
    switch (format) {
        case ParagraphFormat::heading1: target = {Kind::heading, 1}; break;
        case ParagraphFormat::heading2: target = {Kind::heading, 2}; break;
        case ParagraphFormat::heading3: target = {Kind::heading, 3}; break;
        case ParagraphFormat::body: target = {Kind::body, 0}; break;
        case ParagraphFormat::bulletList: target = {Kind::bullet, 0}; break;
        case ParagraphFormat::numberedList: target = {Kind::numbered, 0}; break;
    }
    const auto targets = s.blocksIn(sel.location, std::max(sel.location, sel.end() - (sel.length > 0 ? 1 : 0)));
    if (targets.empty()) { return std::nullopt; }
    bool allTarget = true;
    for (const auto& t : targets) { if (!(kindOf(t, s.info(t)).kind == target)) { allTarget = false; } }
    const ParagraphKind newKind = allTarget && target.kind != Kind::body ? ParagraphKind{Kind::body, 0} : target;

    // The edit runs from the first target to the end of the list that follows it (renumbering), and starts numbering after an
    // ordered item that precedes it.
    struct Item { URange range; KindAndPrefix kind; bool isTarget; };
    std::vector<Item> all;
    for (const auto& t : targets) { all.push_back({t, kindOf(t, s.info(t)), true}); }
    int p = all.back().range.end();
    while (p < s.length()) {
        const auto next = s.nextBlockAfter(p);
        if (!next || next->end() <= p) { break; }
        const auto k = kindOf(*next, s.info(*next));
        if (k.kind.kind != Kind::numbered) { break; }
        all.push_back({*next, k, false});
        p = next->end();
    }
    int counter = 1;
    if (const auto prev = s.previousBlockBefore(all.front().range.location)) {
        const ListItem16* li = firstLineItem(s.info(*prev));
        if (li != nullptr && li->ordered) { counter = li->number + 1; }
    }
    const int start = all.front().range.location;
    const int end = all.back().range.end();
    Units out(s.slice({start, end - start}));
    std::vector<std::pair<int, int>> deltaBefore;   // (at, delta)
    std::vector<int> numbers;                        // 0 = not numbered
    for (const auto& item : all) {
        const ParagraphKind k = item.isTarget ? newKind : item.kind.kind;
        if (k.kind == Kind::numbered) { numbers.push_back(counter); ++counter; } else { numbers.push_back(0); counter = 1; }
    }
    for (int i = static_cast<int>(all.size()) - 1; i >= 0; --i) {
        const auto& item = all[static_cast<std::size_t>(i)];
        const ParagraphKind k = item.isTarget ? newKind : item.kind.kind;
        Units prefix;
        switch (k.kind) {
            case Kind::body: break;
            case Kind::heading: prefix = Units(static_cast<std::size_t>(k.level), u'#') + u" "; break;
            case Kind::bullet: prefix = u"- "; break;
            case Kind::numbered: prefix = decimal(numbers[static_cast<std::size_t>(i)] != 0 ? numbers[static_cast<std::size_t>(i)] : 1) + u". "; break;
        }
        const URange old = item.kind.prefix.value_or(URange{item.range.location, 0});
        out.replace(static_cast<std::size_t>(old.location - start), static_cast<std::size_t>(old.length), prefix);
        deltaBefore.emplace_back(old.location, static_cast<int>(prefix.size()) - old.length);
    }
    // The selection keeps its text: shift it by the prefix changes before (and inside) it.
    auto map = [&](int x) {
        int d = 0;
        for (const auto& [at, delta] : deltaBefore) { if (at <= x) { d += delta; } }
        return x + d;
    };
    const URange selection{map(sel.location), std::max(0, map(sel.end()) - map(sel.location))};
    return Edit{{start, end - start}, out, selection};
}

// MARK: — Return in a list item

std::optional<Edit> listReturnEdit(const Scene& s, URange sel) {
    if (sel.length != 0) { return std::nullopt; }
    const auto b = s.blockAt(sel.location);
    if (!b) { return std::nullopt; }
    const Analysis16& info = s.info(*b);
    const ListItem16* li = firstLineItem(info);
    if (li == nullptr) { return std::nullopt; }
    const URange prefix{b->location + li->prefix.location, li->prefix.length};
    const int caret = sel.location;
    if (caret < prefix.end()) { return std::nullopt; }
    const UnitsView content = s.slice({prefix.end(), b->end() - prefix.end()});
    if (blankOrNewlines(content)) {
        // The ordered items after it close the gap — Markdown still reads them as the same list.
        Units tail;
        int end = prefix.end();
        if (li->ordered) { std::tie(tail, end) = renumbered(s, *b, prefix.end(), li->number); }
        return Edit{{prefix.location, end - prefix.location}, tail, {prefix.location, 0}};
    }
    const Units bulletText = trimSpaces(s.slice(prefix));
    Units nextPrefix;
    if (li->ordered) {
        nextPrefix = decimal(li->number + 1) + (bulletText.empty() ? Units(u".") : Units(1, bulletText.back())) + u" ";
    } else {
        nextPrefix = bulletText + u" ";
    }
    // The break itself, through the balancing rewrite (a Return inside bold closes and re-opens it).
    std::vector<Token> breakTokens{{u'\n', 0, false}, {u'\n', 0, false}};
    for (const char16_t c : nextPrefix) { breakTokens.push_back({c, 0, true}); }
    const auto r = rewrite(s, {caret, 0}, breakTokens, s.spansIn(caret, caret), false);
    if (!r) { return std::nullopt; }
    Units replacement = r->text;
    int end = r->range.end();
    if (li->ordered) {
        auto [tail, newEnd] = renumbered(s, *b, end, li->number + 2);
        replacement += tail;
        end = newEnd;
    }
    return Edit{{r->range.location, end - r->range.location}, replacement, {r->range.location + r->caret, 0}};
}

// MARK: — Balanced edits (EP-046 AC12)

std::uint8_t insertionStyle(const Scene& s, int a, const std::vector<Token>& selected) {
    if (!selected.empty()) { return selected.front().style; }
    bool afterOpener = false;
    for (const auto& blk : s.blocksIn(a, a)) {
        for (const auto& m : s.info(blk).markers) { if (m.opens && blk.location + m.range.end() == a) { afterOpener = true; } }
    }
    if (afterOpener) {
        const auto t = s.tokensIn({a, std::min(1, s.length() - a)});
        return t.empty() ? 0 : t.front().style;
    }
    if (a <= 0) { return 0; }
    // The nearest character before `a` that is not a marker.
    auto isMarker = [&](int k) {
        const auto b = s.blockAt(k);
        if (!b) { return false; }
        for (const auto& m : s.info(*b).markers) { if (m.range.contains(k - b->location)) { return true; } }
        return false;
    };
    int k = a - 1;
    while (k > 0 && isMarker(k)) { --k; }
    const auto t = s.tokensIn({k, 1});
    return t.empty() ? 0 : t.front().style;
}

std::uint8_t styleAtCaret(const Scene& s, int caret) { return insertionStyle(s, caret, {}); }

std::optional<BalancedEdit> balancedEdit(const Scene& s, URange range, UnitsView replacement, bool keepsOwnFormatting) {
    const int a = range.location, b = range.end();
    const auto touched = s.blocksIn(a, b);
    std::vector<URange> spans, markers;
    std::map<int, std::uint8_t> styleAt;
    bool atSpanEdge = false;
    for (const auto& blk : touched) {
        const Analysis16& info = s.info(blk);
        for (const auto& sp : info.spans) { spans.push_back({blk.location + sp.location, sp.length}); }
        for (const auto& m : info.markers) {
            const URange g{blk.location + m.range.location, m.range.length};
            markers.push_back(g);
            if (m.opens ? g.end() == a : g.location == b) { atSpanEdge = true; }
        }
        for (const int k : {a - 1, a}) { if (blk.contains(k)) { styleAt[k] = info.style(k - blk.location); } }
    }
    auto styleAtOr0 = [&](int k) { const auto it = styleAt.find(k); return it == styleAt.end() ? std::uint8_t{0} : it->second; };
    const bool fragment = em::mayHaveMarkers(replacement);
    bool crossesMarker = false;
    std::set<int> markerUnits;
    for (const auto& m : markers) {
        if (range.length > 0 && m.location < b && m.end() > a) { crossesMarker = true; }
        for (int k = m.location; k < m.end(); ++k) { markerUnits.insert(k); }
    }
    bool emptiesSpan = false;
    if (range.length > 0 && replacement.empty()) {
        for (const auto& sp : spans) {
            bool all = true;
            for (int k = sp.location; k < sp.end(); ++k) {
                if (markerUnits.count(k) == 0 && !(k >= a && k < b)) { all = false; break; }
            }
            if (all) { emptiesSpan = true; break; }
        }
    }
    // Only a PARAGRAPH break (a blank line) splits a span — a single newline (a hard break) may sit inside bold.
    const bool insideSpan = styleAtOr0(a) != 0 || styleAtOr0(a - 1) != 0;
    const bool splitsSpan = replacement.find(u"\n\n") != UnitsView::npos && insideSpan;
    const bool ownInsideSpan = keepsOwnFormatting && !replacement.empty() && insideSpan;
    if (!(fragment || crossesMarker || emptiesSpan || splitsSpan || ownInsideSpan || atSpanEdge)) { return std::nullopt; }

    const std::uint8_t insertStyle = keepsOwnFormatting ? 0 : insertionStyle(s, a, s.tokensIn(range));
    std::vector<Token> inserted = em::tokens(replacement);
    for (auto& t : inserted) {
        // A prefix written by the edit (a list's `- ` on Return) never carries emphasis.
        t.style = t.isPrefix ? 0 : static_cast<std::uint8_t>(t.style | insertStyle);
    }
    const auto r = rewrite(s, range, inserted, spans, false);
    if (!r) { return std::nullopt; }
    return BalancedEdit{r->range, r->text, r->range.location + r->caret};
}

Copy balancedCopy(const Scene& s, URange sel) {
    auto toks = em::normalize(s.tokensIn(sel));
    Units source = em::serialize(toks).text;
    if (!em::sameRendering(toks, em::tokens(source))) {
        for (auto& t : toks) { t.style = 0; }
        source = em::serialize(toks).text;
    }
    return {source, em::presented(toks)};
}

std::vector<Units> balancePastePieces(const std::vector<Units>& pieces, std::uint8_t caretStyle) {
    if (caretStyle == 0 || pieces.size() < 2) { return pieces; }
    auto styled = [](const Units& t) { return em::normalize(em::tokens(t)); };
    std::vector<Units> out = pieces;
    out.front() = em::serialize(styled(pieces.front()), caretStyle, 0).text;
    out.back() = em::serialize(styled(pieces.back()), 0, caretStyle).text;
    return out;
}

} // namespace scrivi::markdown::editing
