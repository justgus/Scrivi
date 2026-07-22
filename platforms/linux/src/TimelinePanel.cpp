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

void TimelinePanel::setActiveScene(const QString& sceneID)
{
    if (activeSceneID_ == sceneID) {
        return;
    }
    activeSceneID_ = sceneID;
    update();
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

    // Baseline (a faint horizontal rule, theme-aware).
    painter.setPen(QPen(pal.color(QPalette::Mid), 1.0));
    painter.drawLine(QPointF(kSideInset, cy),
                     QPointF(width() - kSideInset, cy));

    // Dots. The active scene's dot is drawn filled with the highlight colour and a
    // ring; the rest use the text colour. While dragging, the dragged dot follows the
    // live pointer x (dragX_) instead of its stored offset — the SP-080 drag preview.
    const QColor dotColor    = pal.color(QPalette::Text);
    const QColor activeColor = pal.color(QPalette::Highlight);
    for (int i = 0; i < dots_.size(); ++i) {
        const Dot& d = dots_.at(i);
        const bool isDragged = (dragging_ && i == pressedDot_);
        const double cx = isDragged ? dragX_ : xForOffset(d.offsetMs);
        const bool active = (d.sceneID == activeSceneID_);
        painter.setPen(Qt::NoPen);
        painter.setBrush((active || isDragged) ? activeColor : dotColor);
        painter.drawEllipse(QPointF(cx, cy), kDotRadius, kDotRadius);
        if (active || isDragged) {
            painter.setPen(QPen(activeColor, 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPointF(cx, cy), kDotRadius + 3.0, kDotRadius + 3.0);
        }
    }

    // Epoch label under the left edge of the baseline.
    painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
    painter.drawText(QPointF(kSideInset, cy + kDotRadius + 16.0), epochLabel_);
}

void TimelinePanel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Remember which dot (if any) is under the press. A click vs a drag is decided
        // in mouseMoveEvent (movement past the threshold) / mouseReleaseEvent. A press
        // on the background leaves pressedDot_ = -1 (reserved for pan in SP-083).
        pressedDot_ = dotIndexAt(event->pos());
        dragging_   = false;
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
    if (pressedDot_ >= 0 && (event->buttons() & Qt::LeftButton)) {
        const double x = event->position().x();
        if (!dragging_
            && std::abs(x - xForOffset(dots_.at(pressedDot_).offsetMs)) > kDragThreshold) {
            dragging_ = true;   // crossed the threshold — this is a drag, not a click
        }
        if (dragging_) {
            dragX_ = std::clamp(x, kSideInset, width() - kSideInset);
            update();   // redraw the dragged dot at the live x
            event->accept();
            return;
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TimelinePanel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && pressedDot_ >= 0) {
        const QString sceneID = dots_.at(pressedDot_).sceneID;
        if (dragging_) {
            // Dragged: map the release x back to a story-time offset; the shell opens
            // the Time Delta Picker seeded with it (T-0328).
            const qint64 newOffsetMs = offsetForX(dragX_);
            emit dotDragged(sceneID, newOffsetMs);
        } else {
            // No movement past the threshold — a plain click → navigate (SP-079).
            emit sceneClicked(sceneID);
        }
        pressedDot_ = -1;
        dragging_   = false;
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
    // Right-click on a dot → "Set Time Delta…" (Apple's context-menu entry). The shell
    // opens the picker without a drag (T-0328).
    QMenu menu(this);
    QAction* setDelta = menu.addAction(tr("Set Time Delta…"));
    const QString sceneID = dots_.at(i).sceneID;
    if (menu.exec(event->globalPos()) == setDelta) {
        emit setTimeDeltaRequested(sceneID);
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
