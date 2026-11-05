#include "ManuscriptLayout.hpp"

#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>


QRectF ManuscriptLayout::blockBoundingRect(const QTextBlock& block) const
{
    ++calls_;
    const QRectF r = QPlainTextDocumentLayout::blockBoundingRect(block);   // lays the block out if Qt has not yet
    auto* style = dynamic_cast<ManuscriptLineStyle*>(block.userData());
    QTextLayout* tl = block.layout();
    if (style == nullptr || style->pitch <= 0 || tl == nullptr || tl->lineCount() == 0 || !block.isVisible()) {
        return r;
    }
    const double margin = document()->documentMargin();
    const double foot = block.next().isValid() ? 0 : margin;   // Qt's own bottom margin, after the last block
    if (tl->lineAt(0).x() == style->laidX && tl->lineAt(0).y() == style->laidY) {
        return {r.topLeft(), QSizeF(r.width(), tl->lineCount() * style->pitch + foot)};   // laid out by us, and still current
    }
    // Qt has just laid it out, or the style is new: place the lines again, across Qt's own width. ⛔ Never a width read back
    // from our own lines. Qt's lines start exactly at (margin, 0) and span its available width; a width change re-lays every
    // block through Qt first, so the width remembered here is current.
    const QTextLine first = tl->lineAt(0);
    if (first.x() == margin && first.y() == 0) {
        availableWidth_ = first.width();
    }
    const double width = availableWidth_ > 0 ? availableWidth_ : first.width() + (first.x() - margin);
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
        // ✅ Apple's fixed line height puts the extra space ABOVE the glyphs (TextKit's minimum = maximum line height).
        line.setPosition(QPointF(margin + (isFirst ? style->indent : 0), y + style->pitch - line.height()));
        y += style->pitch;
    }
    tl->endLayout();
    style->laidX = tl->lineAt(0).x();
    style->laidY = tl->lineAt(0).y();
    if (tl->lineCount() != before) {
        // An indent can wrap one more line: keep the document's line count (the scroll range) in step.
        QTextBlock(block).setLineCount(tl->lineCount());
        auto* self = const_cast<ManuscriptLayout*>(this);
        QMetaObject::invokeMethod(self, [self]() { emit self->documentSizeChanged(self->documentSize()); }, Qt::QueuedConnection);
    }
    return {r.topLeft(), QSizeF(r.width(), y + foot)};
}
