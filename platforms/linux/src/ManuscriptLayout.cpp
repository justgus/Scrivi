#include "ManuscriptLayout.hpp"

#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>

QRectF ManuscriptLayout::blockBoundingRect(const QTextBlock& block) const
{
    ++calls_;
    const QRectF r = QPlainTextDocumentLayout::blockBoundingRect(block);   // lays the block out if Qt has not yet
    const auto* style = dynamic_cast<const ManuscriptLineStyle*>(block.userData());
    QTextLayout* tl = block.layout();
    if (style == nullptr || style->pitch <= 0 || tl == nullptr || tl->lineCount() == 0 || !block.isVisible()) {
        return r;
    }
    // ✅ Apple's fixed line height puts the extra space ABOVE the glyphs (TextKit's minimum = maximum line height), so a
    // line's glyphs sit at the bottom of its pitch.
    const double margin = document()->documentMargin();
    const double foot = block.next().isValid() ? 0 : margin;   // Qt's own bottom margin, after the last block
    const QTextLine first = tl->lineAt(0);
    const QPointF ours(margin + style->indent, style->pitch - first.height());
    if (qAbs(first.x() - ours.x()) < 0.01 && qAbs(first.y() - ours.y()) < 0.01) {
        return {r.topLeft(), QSizeF(r.width(), tl->lineCount() * style->pitch + foot)};   // laid out by us, and still current
    }
    // Qt has just laid it out (its lines at the margin, one under the other): place them again.
    const double width = first.width() + (first.x() - margin);
    const int before = tl->lineCount();
    ++relayouts_;
    tl->beginLayout();
    double y = 0;
    for (bool isFirst = true;; isFirst = false) {
        QTextLine line = tl->createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(width - (isFirst ? style->indent : 0));
        line.setPosition(QPointF(margin + (isFirst ? style->indent : 0), y + style->pitch - line.height()));
        y += style->pitch;
    }
    tl->endLayout();
    if (tl->lineCount() != before) {
        // An indent can wrap one more line: keep the document's line count (the scroll range) in step.
        QTextBlock(block).setLineCount(tl->lineCount());
        auto* self = const_cast<ManuscriptLayout*>(this);
        QMetaObject::invokeMethod(self, [self]() { emit self->documentSizeChanged(self->documentSize()); }, Qt::QueuedConnection);
    }
    return {r.topLeft(), QSizeF(r.width(), y + foot)};
}
