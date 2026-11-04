#include "ManuscriptFind.hpp"

#include <QElapsedTimer>
#include <QSet>
#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>

#include "ManuscriptPresenter.hpp"
#include "SceneDocument.hpp"

namespace ManuscriptFind {

namespace {

// The document text of [start, end), paragraph separators as '\n' (one unit each, so offsets are kept).
QString textOf(QTextDocument* doc, int start, int end)
{
    QTextCursor c(doc);
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    QString t = c.selectedText();
    t.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    return t;
}

bool isApostrophe(QChar c) { return c == QLatin1Char('\'') || c == QChar(0x2019); }

// A word character: a letter or digit, or an apostrophe between two letters ("don't" is one word).
bool isWordAt(const QString& t, int i)
{
    if (i < 0 || i >= t.size()) {
        return false;
    }
    if (t[i].isLetterOrNumber()) {
        return true;
    }
    return isApostrophe(t[i]) && i > 0 && i + 1 < t.size() && t[i - 1].isLetter() && t[i + 1].isLetter();
}

}   // namespace

PresentedText PresentedText::build(const SceneDocument& doc, const ManuscriptPresenter* presenter)
{
    PresentedText out;
    out.refresh(doc, presenter);
    return out;
}

int PresentedText::refresh(const SceneDocument& sceneDoc, const ManuscriptPresenter* presenter)
{
    // ⚠️ `document()` is non-const on SceneDocument; reading text does not change it.
    QTextDocument* doc = const_cast<SceneDocument&>(sceneDoc).document();
    const auto& segs = sceneDoc.segments();
    scenes_.resize(segs.size());
    pageStarts_.resize(segs.size());
    pageLength_ = 0;
    int rebuilt = 0;
    for (int i = 0; i < segs.size(); ++i) {
        rebuilt += refreshScene(i, sceneDoc, doc, presenter) ? 1 : 0;
        // EP-050 S4: the boundary before the body. A scene after the first of its chapter has a divider (the separator
        // `SceneDocument::sceneSeparatorPositions` draws); a chapter's first scene, its title.
        const int gapStart = i > 0 ? segs[i - 1].bodyStart + segs[i - 1].bodyLength : 0;
        buildLead(scenes_[i], doc, gapStart, segs[i].bodyStart, i > 0 && segs[i].chapterID == segs[i - 1].chapterID);
        pageStarts_[i] = pageLength_;
        pageLength_ += int(scenes_[i].lead.size() + scenes_[i].text.size());
    }
    docEnd_ = doc->characterCount() - 1;
    return rebuilt;
}

void PresentedText::buildLead(Scene& scene, QTextDocument* doc, int gapStart, int bodyStart, bool sceneBreak)
{
    const int gapLen = bodyStart - gapStart;
    if (sceneBreak && gapLen == 2 && scene.gapStart == gapStart && !scene.lead.isEmpty()) {
        return;   // a divider's words do not change; a chapter title is re-read (one per chapter)
    }
    scene.gapStart = gapStart;
    scene.lead.clear();
    scene.leadRel.clear();
    if (sceneBreak && gapLen == 2) {
        // The two separators around the divider's blank block; the words are the blank block (Apple: the divider character).
        scene.lead = QStringLiteral("\nScene break\n");
        scene.leadRel.fill(1, scene.lead.size());
        scene.leadRel[0] = 0;
        return;
    }
    scene.lead = textOf(doc, gapStart, bodyStart);
    scene.leadRel.reserve(gapLen);
    for (int k = 0; k < gapLen; ++k) {
        scene.leadRel.append(k);
    }
}

int PresentedText::refreshSlice(const SceneDocument& sceneDoc, const ManuscriptPresenter* presenter, int from, int budgetMs)
{
    QTextDocument* doc = const_cast<SceneDocument&>(sceneDoc).document();
    const int n = int(sceneDoc.segments().size());
    scenes_.resize(n);   // ⚠️ the manuscript may have changed since the last slice; `refresh()`'s checks catch it after
    QElapsedTimer t;
    t.start();
    int i = std::max(0, from);
    for (; i < n; ++i) {
        if (i > from && t.elapsed() >= budgetMs) {
            break;
        }
        refreshScene(i, sceneDoc, doc, presenter);
    }
    return i;
}

bool PresentedText::refreshScene(int i, const SceneDocument& sceneDoc, QTextDocument* doc, const ManuscriptPresenter* presenter)
{
    const SceneSegment& seg = sceneDoc.segments()[i];
    Scene& scene = scenes_[i];
    bool rebuilt = false;
    if (scene.edited || scene.sceneID != seg.sceneID || scene.bodyLength != seg.bodyLength) {
        scene.sceneID = seg.sceneID;
        buildScene(scene, doc, seg.bodyStart, seg.bodyLength, presenter);
        rebuilt = true;
    }
    scene.bodyStart = seg.bodyStart;
    return rebuilt;
}

void PresentedText::markEdited(const SceneDocument& doc, int pos)
{
    const int i = doc.sceneIndexForEditablePosition(pos);
    if (i >= 0 && i < scenes_.size()) {
        scenes_[i].edited = true;
    }
}

void PresentedText::buildScene(Scene& scene, QTextDocument* doc, int bodyStart, int bodyLength,
                               const ManuscriptPresenter* presenter)
{
    const int bodyEnd = bodyStart + bodyLength;
    const QString body = textOf(doc, bodyStart, bodyEnd);
    scene.bodyLength = bodyLength;
    scene.edited = false;
    scene.text.clear();
    scene.rel.clear();
    scene.escaped.clear();
    scene.text.reserve(body.size());
    scene.rel.reserve(body.size());
    scene.escaped.reserve(body.size());
    auto append = [&](QChar c, int pos, bool escaped) {
        scene.text.append(c);
        scene.rel.append(pos - bodyStart);
        scene.escaped.append(escaped);
    };
    int pos = bodyStart;
    while (pos < bodyEnd) {
        const auto b = presenter != nullptr ? presenter->blockAt(pos) : std::nullopt;
        if (!b || b->start != pos) {
            // A blank line (no Markdown block), or drawn as stored: presented as it is.
            append(body[pos - bodyStart], pos, false);
            ++pos;
            continue;
        }
        const QVector<int> hidden = presenter->hiddenAtRest(b->text);
        const QSet<int> escapes = presenter->escapesOf(b->text);
        const int stop = std::min(b->end, bodyEnd);
        int h = 0;
        for (int k = 0; k < stop - b->start; ++k) {
            while (h < hidden.size() && hidden[h] < k) {
                ++h;
            }
            if (h < hidden.size() && hidden[h] == k) {
                continue;
            }
            append(b->text[k], b->start + k, escapes.contains(k - 1));
        }
        pos = stop;
    }
}

QVector<MarkdownRange> PresentedText::find(const QString& query, const Options& options) const
{
    QVector<MarkdownRange> out;
    if (query.isEmpty()) {
        return out;
    }
    const Qt::CaseSensitivity cs = options.ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive;
    const int n = int(query.size());
    for (const Scene& s : scenes_) {
        for (int k = s.text.indexOf(query, 0, cs); k >= 0; k = s.text.indexOf(query, k, cs)) {
            const bool starts = !isWordAt(s.text, k - 1);
            const bool ends = !isWordAt(s.text, k + n);
            const bool ok = options.mode == Mode::Contains || (options.mode == Mode::StartsWith && starts)
                            || (options.mode == Mode::WholeWord && starts && ends);
            if (!ok) {
                ++k;
                continue;
            }
            out.append({s.bodyStart + s.rel[k] - (s.escaped[k] ? 1 : 0), s.bodyStart + s.rel[k + n - 1] + 1});
            k += n;   // matches do not overlap (Apple's find bar)
        }
    }
    return out;
}

QString PresentedText::presentedOf(int start, int end) const
{
    QStringList parts;
    for (const Scene& s : scenes_) {
        if (s.rel.isEmpty() || s.bodyStart + s.rel.last() < start || s.bodyStart + s.rel.first() >= end) {
            continue;
        }
        const auto a = std::lower_bound(s.rel.begin(), s.rel.end(), start - s.bodyStart);
        const auto b = std::lower_bound(s.rel.begin(), s.rel.end(), end - s.bodyStart);
        if (a < b) {
            parts.append(s.text.mid(int(a - s.rel.begin()), int(b - a)));
        }
    }
    return parts.join(QLatin1Char('\n'));
}

QString PresentedText::pageText(int start, int end) const
{
    start = std::max(0, start);
    end = std::min(end, pageLength_);
    QString out;
    if (start >= end) {
        return out;
    }
    auto i = int(std::upper_bound(pageStarts_.begin(), pageStarts_.end(), start) - pageStarts_.begin()) - 1;
    for (; i < scenes_.size() && pageStarts_[i] < end; ++i) {
        const Scene& s = scenes_[i];
        const int a = std::max(start - pageStarts_[i], 0);
        const int b = end - pageStarts_[i];
        const int leadLen = int(s.lead.size());
        if (a < leadLen) {
            out += s.lead.mid(a, std::min(b, leadLen) - a);
        }
        if (b > leadLen) {
            const int ta = std::max(a - leadLen, 0);
            out += s.text.mid(ta, b - leadLen - ta);
        }
    }
    return out;
}

int PresentedText::pageIndex(int pos) const
{
    if (scenes_.isEmpty() || pos <= 0) {
        return 0;
    }
    if (pos >= docEnd_) {
        return pageLength_;
    }
    // The last scene whose boundary starts at or before `pos`.
    auto it = std::upper_bound(scenes_.begin(), scenes_.end(), pos, [](int p, const Scene& s) { return p < s.gapStart; });
    const auto i = int(it - scenes_.begin()) - 1;
    if (i < 0) {
        return 0;
    }
    const Scene& s = scenes_[i];
    if (pos < s.bodyStart) {
        const auto j = int(std::lower_bound(s.leadRel.begin(), s.leadRel.end(), pos - s.gapStart) - s.leadRel.begin());
        return pageStarts_[i] + j;
    }
    const auto j = int(std::lower_bound(s.rel.begin(), s.rel.end(), pos - s.bodyStart) - s.rel.begin());
    return pageStarts_[i] + int(s.lead.size()) + j;
}

int PresentedText::documentPosition(int k) const
{
    if (k >= pageLength_ || scenes_.isEmpty()) {
        return docEnd_;
    }
    k = std::max(k, 0);
    const auto i = int(std::upper_bound(pageStarts_.begin(), pageStarts_.end(), k) - pageStarts_.begin()) - 1;
    const Scene& s = scenes_[i];
    const int local = k - pageStarts_[i];
    if (local < s.lead.size()) {
        return s.gapStart + s.leadRel[local];
    }
    return s.bodyStart + s.rel[local - int(s.lead.size())];
}

void PresentedText::markEditedRange(const SceneDocument& doc, int start, int end)
{
    const auto& segs = doc.segments();
    for (int i = 0; i < segs.size() && i < scenes_.size(); ++i) {
        if (segs[i].bodyStart <= end && segs[i].bodyStart + segs[i].bodyLength >= start) {
            scenes_[i].edited = true;
        }
    }
}

int PresentedText::length() const
{
    int n = 0;
    for (const Scene& s : scenes_) {
        n += int(s.text.size());
    }
    return n;
}

}   // namespace ManuscriptFind
