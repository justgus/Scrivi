#include "ManuscriptPresenter.hpp"

#include <QGuiApplication>
#include <QPalette>
#include <QTextBlock>
#include <QTextDocument>

#include "ManuscriptEscapes.hpp"
#include "SceneDocument.hpp"
#include "ThemeColours.hpp"

ManuscriptPresenter::ManuscriptPresenter(Analyzer analyzer, QObject* parent)
    : QSyntaxHighlighter(parent), analyzer_(std::move(analyzer))
{
}

ManuscriptPresenter::~ManuscriptPresenter()
{
    disconnect(edits_);
    setDocument(nullptr);
}

void ManuscriptPresenter::attach(QTextDocument* document)
{
    disconnect(edits_);
    revealedLines_.clear();
    revealedSpans_.clear();
    setDocument(document);
    if (document != nullptr) {
        // Connected AFTER QSyntaxHighlighter's own handler (setDocument connects first), so the edited
        // line is already re-highlighted when the rest of its Markdown block is.
        edits_ = connect(document, &QTextDocument::contentsChange, this, &ManuscriptPresenter::onContentsChange);
    }
}

double ManuscriptPresenter::headingScale(int level)
{
    switch (level) {
    case 1: return 22.0 / 13.0;
    case 2: return 18.0 / 13.0;
    case 3: return 16.0 / 13.0;
    default: return 1.0;
    }
}

int ManuscriptPresenter::sceneOf(const QTextBlock& b) const
{
    if (sceneDoc_ == nullptr) {
        return 0;
    }
    return sceneDoc_->sceneIndexForEditablePosition(b.position());
}

std::optional<ManuscriptPresenter::Block> ManuscriptPresenter::blockAt(int pos) const
{
    const QTextDocument* doc = document();
    if (doc == nullptr) {
        return std::nullopt;
    }
    const QTextBlock line = doc->findBlock(pos);
    const int scene = line.isValid() ? sceneOf(line) : -1;
    if (scene < 0 || ManuscriptEscapes::isBlankLine(line.text())) {
        return std::nullopt;
    }
    auto inBlock = [&](const QTextBlock& b) {
        return b.isValid() && !ManuscriptEscapes::isBlankLine(b.text()) && sceneOf(b) == scene;
    };
    QTextBlock first = line;
    while (inBlock(first.previous())) {
        first = first.previous();
    }
    QTextBlock last = line;
    while (inBlock(last.next())) {
        last = last.next();
    }
    Block out;
    out.start = first.position();
    for (QTextBlock b = first;; b = b.next()) {
        out.text += b.text();
        if (b == last) {
            break;
        }
        out.text += QLatin1Char('\n');
    }
    if (last.next().isValid()) {
        out.text += QLatin1Char('\n');
    }
    out.end = out.start + out.text.size();
    return out;
}

const ManuscriptPresenter::Info& ManuscriptPresenter::info(const QString& text) const
{
    auto hit = cache_.constFind(text);
    if (hit != cache_.constEnd()) {
        return *hit;
    }
    if (cache_.size() > 4096) {
        cache_.clear();
    }
    Info in;
    if (text.contains(QLatin1Char('\\'))) {
        // E1, unchanged (Apple's `info`): one line at a time; a line-end backslash is a hidden HARD BREAK
        // only when a line of the same block follows it (EP-045 AC6, Q1 = (a)).
        int start = 0;
        while (start < text.size()) {
            const int nl = text.indexOf(QLatin1Char('\n'), start);
            const int end = nl < 0 ? text.size() : nl + 1;
            const bool continues = end < text.size();
            for (int off : ManuscriptEscapes::hiddenBackslashes(text.mid(start, end - start), continues)) {
                in.escapes.insert(start + off);
            }
            start = end;
        }
    }
    ++analyzerCalls_;
    in.a = analyzer_ ? analyzer_(text) : MarkdownAnalysis{};
    return *cache_.insert(text, in);
}

std::optional<ManuscriptPresenter::Stop> ManuscriptPresenter::stopAt(int i) const
{
    const auto b = blockAt(i);
    if (!b || i >= b->end) {
        return std::nullopt;
    }
    const Info& in = info(b->text);
    const int rel = i - b->start;
    if (in.escapes.contains(rel)) {
        return Stop{i, i + 1, StopKind::Escape};
    }
    for (const auto& h : in.a.headings) {
        if (h.prefix.contains(rel)) {
            return Stop{b->start + h.prefix.start, b->start + h.prefix.end, StopKind::Prefix};
        }
    }
    for (const auto& li : in.a.listItems) {
        if (li.prefix.contains(rel)) {
            return Stop{b->start + li.prefix.start, b->start + li.prefix.end, StopKind::ListPrefix};
        }
    }
    for (const auto& m : in.a.markers) {
        if (m.range.contains(rel)) {
            return Stop{b->start + m.range.start, b->start + m.range.end, m.opens ? StopKind::Opener : StopKind::Closer};
        }
    }
    return std::nullopt;
}

// ── The reveal ───────────────────────────────────────────────────────────────────────────────────

QVector<MarkdownRange> ManuscriptPresenter::revealSpans(int selStart, int selEnd) const
{
    // Apple's `revealSpans`: a span shows its markers while a selection end sits at its FIRST or LAST
    // character — right after an opening marker or right before a closing one.
    QVector<MarkdownRange> out;
    QSet<int> ends{selStart, selEnd};
    for (int c : ends) {
        const auto b = blockAt(c);
        if (!b) {
            continue;
        }
        const Info& in = info(b->text);
        const int rel = c - b->start;
        bool atEdge = false;
        for (const auto& m : in.a.markers) {
            if (m.opens ? m.range.end == rel : m.range.start == rel) {
                atEdge = true;
                break;
            }
        }
        if (!atEdge) {
            continue;
        }
        for (const auto& s : in.a.spans) {
            const MarkdownRange g{b->start + s.start, b->start + s.end};
            if (g.start < c && c < g.end && !out.contains(g)) {
                out.append(g);
            }
        }
    }
    return out;
}

void ManuscriptPresenter::reveal(int selStart, int selEnd)
{
    const QTextDocument* doc = document();
    if (doc == nullptr) {
        return;
    }
    QSet<int> lines{doc->findBlock(selStart).blockNumber(), doc->findBlock(selEnd).blockNumber()};
    const QVector<MarkdownRange> spans = revealSpans(selStart, selEnd);
    if (lines == revealedLines_ && spans == revealedSpans_) {
        return;
    }
    const QSet<int> previousLines = revealedLines_;
    const QVector<MarkdownRange> previousSpans = revealedSpans_;
    revealedLines_ = lines;
    revealedSpans_ = spans;
    // Only lines whose drawing changes: a line that gained or lost the reveal and carries a heading,
    // and every line of a span that gained or lost it.
    QSet<int> touched;
    for (int n : (previousLines - lines) + (lines - previousLines)) {
        const QTextBlock line = doc->findBlockByNumber(n);
        if (const auto b = line.isValid() ? blockAt(line.position()) : std::nullopt;
            b && !info(b->text).a.headings.isEmpty()) {
            touched.insert(n);
        }
    }
    for (const auto& r : previousSpans + spans) {
        if (previousSpans.contains(r) && spans.contains(r)) {
            continue;
        }
        for (QTextBlock line = doc->findBlock(r.start); line.isValid() && line.position() < r.end; line = line.next()) {
            touched.insert(line.blockNumber());
        }
    }
    for (int n : touched) {
        rehighlightBlock(doc->findBlockByNumber(n));
    }
}

bool ManuscriptPresenter::isHidden(int i) const
{
    const auto stop = stopAt(i);
    if (!stop) {
        return false;
    }
    switch (stop->kind) {
    case StopKind::Escape: return true;
    case StopKind::ListPrefix: return false;   // [SP-163] Q7: always visible, dimmed
    case StopKind::Prefix: return !lineRevealed(document()->findBlock(i).blockNumber());
    case StopKind::Opener:
    case StopKind::Closer:
        for (const auto& s : revealedSpans_) {
            if (s.contains(i)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// ── Drawing ──────────────────────────────────────────────────────────────────────────────────────

QTextCharFormat ManuscriptPresenter::hiddenFormat() const
{
    // W3 (design §2.4): 0.01 pt + transparent = 0.00 pt residue on `QPlainTextEdit`.
    QTextCharFormat f;
    f.setFontPointSize(0.01);
    f.setForeground(Qt::transparent);
    return f;
}

QTextCharFormat ManuscriptPresenter::dimmedFormat() const
{
    // Apple: `revealedPrefixAttributes` — the body font, `tertiaryLabelColor`. Linux's deemphasised colour.
    QTextCharFormat f;
    f.setFontPointSize(document()->defaultFont().pointSizeF());
    f.setFontWeight(QFont::Normal);
    f.setFontItalic(false);
    f.setForeground(ThemeColours::deemphasised(QGuiApplication::palette()));
    return f;
}

void ManuscriptPresenter::highlightBlock(const QString& text)
{
    const QTextBlock line = currentBlock();
    const auto b = blockAt(line.position());
    if (!b || text.isEmpty()) {
        return;
    }
    const Info& in = info(b->text);
    if (in.a.isEmpty() && in.escapes.isEmpty()) {
        return;
    }
    const MarkdownAnalysis& a = in.a;
    const int off = line.position() - b->start;   // this line, in block coordinates
    const int len = text.size();
    auto local = [&](MarkdownRange r) -> std::optional<std::pair<int, int>> {
        const int s = std::max(r.start, off), e = std::min(r.end, off + len);
        return e > s ? std::optional(std::pair{s - off, e - s}) : std::nullopt;
    };
    const double body = document()->defaultFont().pointSizeF();

    // 1. Fonts: heading level × inline style, in runs.
    auto level = [&](int k) {
        for (const auto& h : a.headings) {
            if (h.line.contains(k)) {
                return h.level;
            }
        }
        return 0;
    };
    for (int k = off; k < off + len;) {
        const int lv = level(k);
        const auto st = a.style(k);
        int j = k + 1;
        while (j < off + len && level(j) == lv && a.style(j) == st) {
            ++j;
        }
        if (lv != 0 || st != 0) {
            QTextCharFormat f;
            f.setFontPointSize(body * headingScale(lv));
            // ✅ [SP-162] live pass (Apple): bold is one real weight step above the text — body bold = Bold,
            // bold inside a heading (already bold) = ExtraBold.
            f.setFontWeight(lv != 0 ? ((st & MarkdownAnalysis::kBold) ? QFont::ExtraBold : QFont::Bold)
                                    : ((st & MarkdownAnalysis::kBold) ? QFont::Bold : QFont::Normal));
            f.setFontItalic((st & MarkdownAnalysis::kItalic) != 0);
            setFormat(k - off, j - k, f);
        }
        k = j;
    }
    // 2. Heading prefixes: hidden, or dimmed on a revealed line.
    for (const auto& h : a.headings) {
        if (const auto p = local(h.prefix)) {
            setFormat(p->first, p->second, lineRevealed(line.blockNumber()) ? dimmedFormat() : hiddenFormat());
        }
    }
    // 3. Inline markers: hidden, or dimmed inside a revealed span.
    for (const auto& m : a.markers) {
        const auto r = local(m.range);
        if (!r) {
            continue;
        }
        const MarkdownRange g{b->start + m.range.start, b->start + m.range.end};
        bool shown = false;
        for (const auto& s : revealedSpans_) {
            if (s.start <= g.start && g.end <= s.end) {
                shown = true;
            }
        }
        setFormat(r->first, r->second, shown ? dimmedFormat() : hiddenFormat());
    }
    // 4. Escape backslashes: always hidden.
    for (int e : in.escapes) {
        if (e >= off && e < off + len) {
            setFormat(e - off, 1, hiddenFormat());
        }
    }
    // 5. List prefixes: visible, dimmed ([SP-163] Q7). ⚠️ No hanging indent — a highlighter cannot set a
    // BLOCK format, and a real block format would be a document edit (SP-166 records the difference).
    for (const auto& li : a.listItems) {
        if (const auto p = local(li.prefix)) {
            setFormat(p->first, p->second, dimmedFormat());
        }
    }
}

// ── Edits ────────────────────────────────────────────────────────────────────────────────────────

void ManuscriptPresenter::rehighlightRange(int from, int to)
{
    const QTextDocument* doc = document();
    QTextBlock first = doc->findBlock(from);
    QTextBlock last = doc->findBlock(to);
    // One line either side: inserting or removing a blank line joins or splits Markdown blocks.
    if (first.previous().isValid()) {
        first = first.previous();
    }
    if (last.next().isValid()) {
        last = last.next();
    }
    // Widen to whole Markdown blocks.
    if (const auto b = blockAt(first.position())) {
        first = doc->findBlock(b->start);
    }
    if (const auto b = blockAt(last.position())) {
        last = doc->findBlock(std::max(b->start, b->end - 1));
    }
    rehighlighting_ = true;
    for (QTextBlock line = first; line.isValid(); line = line.next()) {
        rehighlightBlock(line);
        if (line == last) {
            break;
        }
    }
    rehighlighting_ = false;
}

void ManuscriptPresenter::onContentsChange(int from, int removed, int added)
{
    if (rehighlighting_ || (removed == 0 && added == 0)) {
        return;   // a format-only change (ours, or the highlighter's) — no text moved
    }
    rehighlightRange(from, from + added);
}
