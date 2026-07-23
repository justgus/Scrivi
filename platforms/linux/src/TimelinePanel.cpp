#include "TimelinePanel.hpp"

#include <QContextMenuEvent>
#include <QHelpEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QToolTip>

#include <algorithm>

namespace {
// Geometry. The strip is short; dots sit on a horizontal baseline centred
// vertically. Insets keep the first/last dot off the panel edges. Matches the
// spirit of Apple's TimelineStripView spacing (not pixel-identical).
constexpr int    kMinHeight    = 80;
constexpr double kSideInset    = 28.0;   // left/right padding for the baseline
constexpr double kDotRadius    = 6.0;
constexpr double kHitRadius    = 12.0;   // generous click/hover target
constexpr double kDragThreshold = 4.0;   // px of movement before a press becomes a drag

constexpr qint64 kMsPerSecond = 1000;
constexpr qint64 kMsPerMinute = 60 * kMsPerSecond;
constexpr qint64 kMsPerHour   = 60 * kMsPerMinute;
constexpr qint64 kMsPerDay    = 24 * kMsPerHour;
} // namespace

TimelinePanel::TimelinePanel(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(kMinHeight);
    setMouseTracking(true);   // hover tooltips without a pressed button
}

void TimelinePanel::setTimeline(const QString& epochLabel, const QList<Dot>& dots)
{
    epochLabel_ = epochLabel.isEmpty() ? QStringLiteral("Story Open") : epochLabel;
    dots_       = dots;

    // Window over story-time: from the first dot's offset to the last dot's end
    // (offset + duration), so the whole span maps across the strip. Guard against a
    // zero-width window (single scene / all-coincident) so xForOffset never /0.
    if (dots_.isEmpty()) {
        minMs_ = 0;
        maxMs_ = 1;
    } else {
        minMs_ = dots_.first().offsetMs;
        maxMs_ = dots_.first().offsetMs + std::max<qint64>(dots_.first().durationMs, 1);
        for (const Dot& d : dots_) {
            minMs_ = std::min(minMs_, d.offsetMs);
            maxMs_ = std::max(maxMs_, d.offsetMs + std::max<qint64>(d.durationMs, 1));
        }
        if (maxMs_ <= minMs_) {
            maxMs_ = minMs_ + 1;
        }
    }

    update();
}

void TimelinePanel::setBands(const QList<Band>& bands,
                             const QHash<QString, QString>& sceneBands)
{
    bands_      = bands;
    sceneBands_ = sceneBands;
    update();
}

void TimelinePanel::setActiveScene(const QString& sceneID)
{
    if (activeSceneID_ == sceneID) {
        return;
    }
    activeSceneID_ = sceneID;
    update();
}

// --- SP-081 band geometry -------------------------------------------------

double TimelinePanel::bandLabelRowHeight() const
{
    // A short row along the top for band labels + the assignment drop target.
    return bands_.isEmpty() ? 0.0 : 22.0;
}

QList<double> TimelinePanel::effectiveProportions() const
{
    if (draggingBorder_ >= 0 && dragProportions_.size() == bands_.size()) {
        return dragProportions_;
    }
    QList<double> props;
    props.reserve(bands_.size());
    for (const Band& b : bands_) {
        props.append(b.proportion);
    }
    return props;
}

double TimelinePanel::bandLeftX(int i) const
{
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const QList<double> props = effectiveProportions();
    double acc = 0.0;
    for (int k = 0; k < i && k < props.size(); ++k) {
        acc += props.at(k);
    }
    return kSideInset + acc * usable;
}

double TimelinePanel::bandRightX(int i) const
{
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const QList<double> props = effectiveProportions();
    double acc = 0.0;
    for (int k = 0; k <= i && k < props.size(); ++k) {
        acc += props.at(k);
    }
    return kSideInset + acc * usable;
}

int TimelinePanel::bandIndexAtX(double x) const
{
    for (int i = 0; i < bands_.size(); ++i) {
        if (x >= bandLeftX(i) && x < bandRightX(i)) {
            return i;
        }
    }
    return bands_.isEmpty() ? -1 : bands_.size() - 1;   // clamp to the last band
}

int TimelinePanel::borderIndexNearX(double x) const
{
    // Borders between adjacent bands (i .. i+1) sit at bandRightX(i). Grab within 5px.
    constexpr double kGrab = 5.0;
    for (int i = 0; i < bands_.size() - 1; ++i) {
        if (std::abs(x - bandRightX(i)) <= kGrab) {
            return i;
        }
    }
    return -1;
}

double TimelinePanel::xForOffset(qint64 offsetMs) const
{
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const double frac =
        static_cast<double>(offsetMs - minMs_) / static_cast<double>(maxMs_ - minMs_);
    return kSideInset + std::clamp(frac, 0.0, 1.0) * usable;
}

qint64 TimelinePanel::offsetForX(double x) const
{
    // Inverse of xForOffset: map a panel x back to a story-time offset, clamped to
    // the current window and never negative (a scene can't start before the epoch).
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const double frac = std::clamp((x - kSideInset) / usable, 0.0, 1.0);
    const qint64 offset =
        minMs_ + static_cast<qint64>(frac * static_cast<double>(maxMs_ - minMs_));
    return std::max<qint64>(offset, 0);
}

int TimelinePanel::dotIndexAt(const QPoint& p) const
{
    const double cy = height() / 2.0;
    for (int i = 0; i < dots_.size(); ++i) {
        const double cx = xForOffset(dots_.at(i).offsetMs);
        const double dx = p.x() - cx;
        const double dy = p.y() - cy;
        if (dx * dx + dy * dy <= kHitRadius * kHitRadius) {
            return i;
        }
    }
    return -1;
}

QString TimelinePanel::humanStoryTime(qint64 offsetMs) const
{
    const qint64 rel = offsetMs - minMs_;
    if (rel <= 0) {
        return tr("at %1").arg(epochLabel_);
    }

    // Largest two non-zero units (days, hours, minutes, seconds), like Apple's
    // human-readable duration.
    struct Unit { qint64 ms; const char* one; const char* many; };
    const Unit units[] = {
        {kMsPerDay,    QT_TR_NOOP("day"),    QT_TR_NOOP("days")},
        {kMsPerHour,   QT_TR_NOOP("hour"),   QT_TR_NOOP("hours")},
        {kMsPerMinute, QT_TR_NOOP("minute"), QT_TR_NOOP("minutes")},
        {kMsPerSecond, QT_TR_NOOP("second"), QT_TR_NOOP("seconds")},
    };

    QStringList parts;
    qint64 remaining = rel;
    for (const Unit& u : units) {
        if (parts.size() >= 2) {
            break;
        }
        const qint64 n = remaining / u.ms;
        if (n > 0) {
            parts << tr("%1 %2").arg(n).arg(n == 1 ? tr(u.one) : tr(u.many));
            remaining -= n * u.ms;
        }
    }
    if (parts.isEmpty()) {
        return tr("at %1").arg(epochLabel_);
    }
    return tr("%1 after %2").arg(parts.join(QStringLiteral(", ")), epochLabel_);
}

void TimelinePanel::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPalette& pal = palette();

    // Empty-state: no scenes → centred hint, no baseline.
    if (dots_.isEmpty()) {
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("No scenes yet — the timeline is empty."));
        return;
    }

    const double cy = height() / 2.0;

    // --- Story-structure bands (behind everything, SP-081) ----------------
    // Translucent colored proportional slices + a label near the top; a subtle
    // separator at each border. Painted first so dots + baseline sit on top.
    if (!bands_.isEmpty()) {
        const double top    = 1.0;
        const double bottom = height() - 1.0;
        for (int i = 0; i < bands_.size(); ++i) {
            const double left  = bandLeftX(i);
            const double right = bandRightX(i);
            QColor c(bands_.at(i).color);
            if (!c.isValid()) { c = pal.color(QPalette::Mid); }
            c.setAlpha(48);   // translucent so the dots read clearly over it
            painter.fillRect(QRectF(left, top, right - left, bottom - top), c);

            // Band label near the top-left of its slice, elided to the slice width.
            painter.setPen(pal.color(QPalette::Text));
            const QRectF labelRect(left + 4.0, top, right - left - 8.0,
                                   bandLabelRowHeight());
            const QString elided = painter.fontMetrics().elidedText(
                bands_.at(i).label, Qt::ElideRight,
                static_cast<int>(labelRect.width()));
            painter.drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

            // Border separator (skip the outer right edge).
            if (i < bands_.size() - 1) {
                QColor border = pal.color(QPalette::Mid);
                if (draggingBorder_ == i) { border = pal.color(QPalette::Highlight); }
                painter.setPen(QPen(border, draggingBorder_ == i ? 2.0 : 1.0));
                painter.drawLine(QPointF(right, top), QPointF(right, bottom));
            }
        }
    }

    // Baseline (a faint horizontal rule, theme-aware).
    painter.setPen(QPen(pal.color(QPalette::Mid), 1.0));
    painter.drawLine(QPointF(kSideInset, cy),
                     QPointF(width() - kSideInset, cy));

    // Dots. The active scene's dot is drawn filled with the highlight colour and a
    // ring; the rest use the text colour. An ASSIGNED dot (SP-081) shows a ring in its
    // band's color. While dragging, the dragged dot follows the live pointer x (dragX_)
    // for a horizontal drag, or the pointer for a drag-to-band.
    const QColor dotColor    = pal.color(QPalette::Text);
    const QColor activeColor = pal.color(QPalette::Highlight);
    for (int i = 0; i < dots_.size(); ++i) {
        const Dot& d = dots_.at(i);
        const bool isHDrag = (dragMode_ == DragMode::DotHorizontal && i == pressedDot_);
        const double cx = isHDrag ? dragX_ : xForOffset(d.offsetMs);
        const bool active = (d.sceneID == activeSceneID_);
        painter.setPen(Qt::NoPen);
        painter.setBrush((active || isHDrag) ? activeColor : dotColor);
        painter.drawEllipse(QPointF(cx, cy), kDotRadius, kDotRadius);
        if (active || isHDrag) {
            painter.setPen(QPen(activeColor, 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPointF(cx, cy), kDotRadius + 3.0, kDotRadius + 3.0);
        }
        // Band-assignment ring (SP-081) in the assigned band's color.
        const QString bandID = sceneBands_.value(d.sceneID);
        if (!bandID.isEmpty()) {
            QColor ringColor = pal.color(QPalette::Text);
            for (const Band& b : bands_) {
                if (b.bandID == bandID) { ringColor = QColor(b.color); break; }
            }
            painter.setPen(QPen(ringColor, 2.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPointF(cx, cy), kDotRadius + 5.0, kDotRadius + 5.0);
        }
    }

    // Epoch label under the left edge of the baseline.
    painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
    painter.drawText(QPointF(kSideInset, cy + kDotRadius + 16.0), epochLabel_);
}

void TimelinePanel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        pressPos_   = event->position();
        pressedDot_ = -1;
        dragMode_   = DragMode::None;
        draggingBorder_ = -1;

        // Priority: a band border (T-0331) wins over a dot, since borders sit in the
        // band gutters where a stray dot could also be. Then a dot (mode decided by
        // the first drag direction). Background press → nothing (reserved for pan).
        const int border = borderIndexNearX(pressPos_.x());
        if (border >= 0) {
            draggingBorder_  = border;
            dragMode_        = DragMode::Border;
            dragProportions_ = effectiveProportions();
            event->accept();
            return;
        }
        pressedDot_ = dotIndexAt(event->pos());
        if (pressedDot_ >= 0) {
            dragX_ = xForOffset(dots_.at(pressedDot_).offsetMs);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void TimelinePanel::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    const QPointF p = event->position();

    // --- Border re-proportion drag (T-0331) -------------------------------
    if (dragMode_ == DragMode::Border && draggingBorder_ >= 0
        && draggingBorder_ + 1 < bands_.size()) {
        const double usable = std::max(1.0, width() - 2.0 * kSideInset);
        const int i = draggingBorder_;
        // Move proportion between band i and i+1: band i's new width runs from its left
        // edge to the pointer. Keep each above a 0.05 floor; their pair-sum is constant
        // so no other band shifts.
        const double pairSum = dragProportions_.at(i) + dragProportions_.at(i + 1);
        const double bandStartFrac = (bandLeftX(i) - kSideInset) / usable;
        const double leftProp = std::clamp((p.x() - kSideInset) / usable - bandStartFrac,
                                           0.05, pairSum - 0.05);
        dragProportions_[i]     = leftProp;
        dragProportions_[i + 1] = pairSum - leftProp;
        update();
        event->accept();
        return;
    }

    // --- Dot drag: decide mode on first movement past the threshold -------
    if (pressedDot_ >= 0) {
        const double dx = p.x() - pressPos_.x();
        const double dy = p.y() - pressPos_.y();
        if (dragMode_ == DragMode::None
            && (std::abs(dx) > kDragThreshold || std::abs(dy) > kDragThreshold)) {
            // Up into the label row (and a structure is present) → assignment; else a
            // horizontal story-time drag. "Mostly up" = dy dominant and into the row.
            const bool intoLabelRow = !bands_.isEmpty()
                                      && p.y() < bandLabelRowHeight() + 6.0;
            dragMode_ = (intoLabelRow && dy < 0 && std::abs(dy) >= std::abs(dx))
                            ? DragMode::DotToBand
                            : DragMode::DotHorizontal;
        }
        if (dragMode_ == DragMode::DotHorizontal) {
            dragX_ = std::clamp(p.x(), kSideInset, width() - kSideInset);
            update();
            event->accept();
            return;
        }
        if (dragMode_ == DragMode::DotToBand) {
            update();   // (a full drag-ghost is unnecessary; the drop resolves on release)
            event->accept();
            return;
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TimelinePanel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if (dragMode_ == DragMode::Border && draggingBorder_ >= 0) {
        emit bandProportionsChanged(dragProportions_);   // shell persists via update_band_layout
        draggingBorder_ = -1;
        dragMode_ = DragMode::None;
        update();
        event->accept();
        return;
    }

    if (pressedDot_ >= 0) {
        const QString sceneID = dots_.at(pressedDot_).sceneID;
        if (dragMode_ == DragMode::DotHorizontal) {
            emit dotDragged(sceneID, offsetForX(dragX_));   // → Time Delta Picker (SP-080)
        } else if (dragMode_ == DragMode::DotToBand) {
            const int bi = bandIndexAtX(event->position().x());
            if (bi >= 0) {
                emit sceneAssignedToBand(sceneID, bands_.at(bi).bandID);
            }
        } else {
            emit sceneClicked(sceneID);   // no drag — navigate (SP-079)
        }
        pressedDot_ = -1;
        dragMode_ = DragMode::None;
        update();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void TimelinePanel::contextMenuEvent(QContextMenuEvent* event)
{
    const int i = dotIndexAt(event->pos());
    if (i < 0) {
        QWidget::contextMenuEvent(event);
        return;
    }
    const QString sceneID = dots_.at(i).sceneID;
    QMenu menu(this);
    QAction* setDelta = menu.addAction(tr("Set Time Delta…"));   // SP-080

    // SP-081: band assignment entries when a structure is present. The shell owns the
    // band submenu (it knows the current bands), so we just signal intent.
    QAction* assign = nullptr;
    QAction* unassign = nullptr;
    if (!bands_.isEmpty()) {
        menu.addSeparator();
        assign = menu.addAction(tr("Assign to Act…"));
        if (!sceneBands_.value(sceneID).isEmpty()) {
            unassign = menu.addAction(tr("Unassign"));
        }
    }

    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == setDelta) {
        emit setTimeDeltaRequested(sceneID);
    } else if (chosen != nullptr && chosen == assign) {
        emit assignBandRequested(sceneID);
    } else if (chosen != nullptr && chosen == unassign) {
        emit unassignBandRequested(sceneID);
    }
    event->accept();
}

bool TimelinePanel::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
        auto* help = static_cast<QHelpEvent*>(event);
        const int i = dotIndexAt(help->pos());
        if (i >= 0) {
            const Dot& d = dots_.at(i);
            QString text = d.title;
            if (!d.chapterTitle.isEmpty()) {
                text += QStringLiteral("\n") + d.chapterTitle;
            }
            text += QStringLiteral("\n") + humanStoryTime(d.offsetMs);
            QToolTip::showText(help->globalPos(), text, this);
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QWidget::event(event);
}
