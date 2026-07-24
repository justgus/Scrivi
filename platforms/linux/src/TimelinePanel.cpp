#include "TimelinePanel.hpp"

#include <QContextMenuEvent>
#include <QCursor>
#include <QHelpEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollBar>
#include <QToolButton>
#include <QToolTip>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

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

    // --- SP-083 zoom control (bottom-right): + on the left, − on the right ---
    // Plain-click buttons = the guaranteed VNC-safe zoom path (a Mac Magic
    // Mouse/trackpad may not emit a discrete wheel x11vnc forwards).
    zoomInBtn_  = new QToolButton(this);
    zoomInBtn_->setText(QStringLiteral("+"));
    zoomInBtn_->setToolTip(tr("Zoom in (Ctrl+wheel also zooms)"));
    zoomInBtn_->setFocusPolicy(Qt::NoFocus);
    connect(zoomInBtn_, &QToolButton::clicked, this, &TimelinePanel::zoomInStep);

    zoomOutBtn_ = new QToolButton(this);
    zoomOutBtn_->setText(QStringLiteral("−"));
    zoomOutBtn_->setToolTip(tr("Zoom out"));
    zoomOutBtn_->setFocusPolicy(Qt::NoFocus);
    connect(zoomOutBtn_, &QToolButton::clicked, this, &TimelinePanel::zoomOutStep);

    // Horizontal scrollbar to pan when zoomed (hidden at zoom 1).
    hScroll_ = new QScrollBar(Qt::Horizontal, this);
    hScroll_->setFocusPolicy(Qt::NoFocus);
    hScroll_->hide();
    connect(hScroll_, &QScrollBar::valueChanged, this, [this](int v) {
        // Map the bar value back to panFraction_ (0..maxPan scaled to the bar range).
        const double maxPan = std::max(0.0, 1.0 - 1.0 / zoom_);
        const int span = hScroll_->maximum() - hScroll_->minimum();
        panFraction_ = span > 0 ? maxPan * (double(v - hScroll_->minimum()) / span) : 0.0;
        clampPan();
        update();
        emit viewStateChanged(zoom_, panFraction_);   // persist (T-0338)
    });

    // Keep the control + scrollbar in step with zoom changes.
    connect(this, &TimelinePanel::zoomChanged, this, &TimelinePanel::syncScrollBar);
}

void TimelinePanel::layoutControls()
{
    constexpr int kBtn = 20;
    constexpr int kMargin = 4;
    const int y = height() - kBtn - kMargin;
    // + on the left, − on the right (user spec).
    zoomInBtn_->setGeometry(width() - 2 * kBtn - kMargin - 2, y, kBtn, kBtn);
    zoomOutBtn_->setGeometry(width() - kBtn - kMargin, y, kBtn, kBtn);
    // Scrollbar runs along the bottom, left of the buttons.
    hScroll_->setGeometry(kMargin, height() - kBtn - kMargin,
                          width() - 2 * kBtn - 3 * kMargin - 4, kBtn - 6);
}

void TimelinePanel::syncScrollBar()
{
    if (hScroll_ == nullptr) {
        return;
    }
    if (zoom_ <= 1.0) {
        hScroll_->hide();
        zoomOutBtn_->setEnabled(false);
        return;
    }
    zoomOutBtn_->setEnabled(true);
    // Model the bar as a 0..1000 range whose page = visible fraction (1/zoom_).
    const QSignalBlocker block(hScroll_);
    constexpr int kRange = 1000;
    const double maxPan = std::max(1e-6, 1.0 - 1.0 / zoom_);
    hScroll_->setRange(0, kRange);
    hScroll_->setPageStep(int(kRange / zoom_));
    hScroll_->setValue(int(kRange * (panFraction_ / maxPan)));
    hScroll_->show();
}

void TimelinePanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    layoutControls();
    syncScrollBar();
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

    // Story-structure bands wrap the MAIN storyline only: story-time [0, last-scene-end],
    // NOT the flashback region left of the epoch (user decision 2026-07-23). storyEndMs_
    // is the latest scene end at/after the epoch; guard against a zero-width span so the
    // band region never collapses. (maxMs_ is the overall window end incl. duration; when
    // every scene is a flashback, clamp the storyline span to a minimal positive width.)
    storyEndMs_ = std::max<qint64>(maxMs_, 1);

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

double TimelinePanel::bandRegionLeftX() const
{
    // The band region starts at the epoch (offset 0), mapped through zoom/pan.
    return xForOffset(0);
}

double TimelinePanel::bandRegionRightX() const
{
    // …and ends at the last scene's end. So bands wrap the main storyline and expand /
    // contract with zoom, instead of filling the whole strip (which would drag them out
    // into the flashback region left of the epoch).
    return xForOffset(storyEndMs_);
}

double TimelinePanel::bandLeftX(int i) const
{
    const double left  = bandRegionLeftX();
    const double span  = bandRegionRightX() - left;
    const QList<double> props = effectiveProportions();
    double acc = 0.0;
    for (int k = 0; k < i && k < props.size(); ++k) {
        acc += props.at(k);
    }
    return left + acc * span;
}

double TimelinePanel::bandRightX(int i) const
{
    const double left  = bandRegionLeftX();
    const double span  = bandRegionRightX() - left;
    const QList<double> props = effectiveProportions();
    double acc = 0.0;
    for (int k = 0; k <= i && k < props.size(); ++k) {
        acc += props.at(k);
    }
    return left + acc * span;
}

int TimelinePanel::bandIndexAtX(double x) const
{
    if (bands_.isEmpty()) {
        return -1;
    }
    for (int i = 0; i < bands_.size(); ++i) {
        if (x >= bandLeftX(i) && x < bandRightX(i)) {
            return i;
        }
    }
    // Outside the band region (bands now wrap only [0, last-end], so a flashback dot
    // sits LEFT of band 0): snap to the nearer end so a scene before Story Open can still
    // be assigned by dragging its dot straight up onto the label row.
    return x < bandRegionLeftX() ? 0 : bands_.size() - 1;
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
    // Story-time → x, through the zoom/pan window (SP-083). `frac` is the offset's
    // position in the FULL [minMs_, maxMs_] window (0..1). The VISIBLE window is a
    // slice of width 1/zoom_ starting at panFraction_, mapped across the usable width:
    //   visibleFrac = (frac - panFraction_) * zoom_.
    // zoom_ = 1, panFraction_ = 0 → the original full-fit mapping.
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const double frac =
        static_cast<double>(offsetMs - minMs_) / static_cast<double>(maxMs_ - minMs_);
    const double visibleFrac = (frac - panFraction_) * zoom_;
    return kSideInset + visibleFrac * usable;
}

qint64 TimelinePanel::offsetForX(double x) const
{
    // Inverse of xForOffset through the zoom/pan window. The offset MAY be negative: a
    // scene can start before the epoch (a flashback before Story Open). frac is NOT
    // clamped to [0,1] so a drag can carry a dot to an offset just off the visible edge.
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const double visibleFrac = (x - kSideInset) / usable;
    const double frac = panFraction_ + visibleFrac / zoom_;
    return minMs_ + static_cast<qint64>(frac * static_cast<double>(maxMs_ - minMs_));
}

void TimelinePanel::clampPan()
{
    // The visible window (width 1/zoom_) must stay within [0,1] of the full window.
    const double maxPan = std::max(0.0, 1.0 - 1.0 / zoom_);
    panFraction_ = std::clamp(panFraction_, 0.0, maxPan);
}

void TimelinePanel::zoomAbout(double factor, double anchorX)
{
    // Zoom by `factor` keeping the story-time under anchorX fixed on screen.
    const double usable = std::max(1.0, width() - 2.0 * kSideInset);
    const double anchorVisibleFrac = (anchorX - kSideInset) / usable;   // 0..1 on screen
    const double anchorFullFrac = panFraction_ + anchorVisibleFrac / zoom_;   // in full window

    const double newZoom = std::clamp(zoom_ * factor, 1.0, 500.0);
    if (newZoom == zoom_) {
        return;
    }
    zoom_ = newZoom;
    // Solve panFraction_ so anchorFullFrac still lands at anchorVisibleFrac on screen.
    panFraction_ = anchorFullFrac - anchorVisibleFrac / zoom_;
    clampPan();
    update();
    emit zoomChanged();
    emit viewStateChanged(zoom_, panFraction_);   // persist (T-0338)
}

void TimelinePanel::setViewState(double zoom, double panFraction)
{
    // Apply persisted state without re-emitting viewStateChanged (that would echo a save).
    zoom_ = std::clamp(zoom, 1.0, 500.0);
    panFraction_ = panFraction;
    clampPan();
    syncScrollBar();
    update();
    emit zoomChanged();   // let the +/- control + scrollbar reflect the restored zoom
}

void TimelinePanel::wheelEvent(QWheelEvent* event)
{
    // Ctrl+wheel = zoom about the pointer (the universal X11/Linux zoom idiom). Plain
    // wheel is left for the enclosing scroll area / future pan. angleDelta().y() > 0 =
    // wheel up = zoom in.
    if (event->modifiers() & Qt::ControlModifier) {
        const double steps = event->angleDelta().y() / 120.0;   // one notch = 120
        if (steps != 0.0) {
            const double factor = std::pow(1.2, steps);
            zoomAbout(factor, event->position().x());
        }
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

void TimelinePanel::zoomInStep()
{
    // The +/- buttons zoom about the current pointer, or the strip center if the pointer
    // is outside the panel (user spec).
    const QPoint g = mapFromGlobal(QCursor::pos());
    const double anchor = rect().contains(g) ? g.x() : width() / 2.0;
    zoomAbout(1.25, anchor);
}

void TimelinePanel::zoomOutStep()
{
    const QPoint g = mapFromGlobal(QCursor::pos());
    const double anchor = rect().contains(g) ? g.x() : width() / 2.0;
    zoomAbout(1.0 / 1.25, anchor);
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
    // Measured from the EPOCH (offset 0 = Story Open), not from the earliest dot: a
    // flashback reads "2 years before Story Open", and the Story Open scene reads
    // "at Story Open" — regardless of what else is on the strip.
    const qint64 rel = offsetMs;   // signed: negative = before the epoch
    if (rel == 0) {
        return tr("at %1").arg(epochLabel_);
    }
    const bool before = rel < 0;

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
    qint64 remaining = before ? -rel : rel;
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
    return before
        ? tr("%1 before %2").arg(parts.join(QStringLiteral(", ")), epochLabel_)
        : tr("%1 after %2").arg(parts.join(QStringLiteral(", ")), epochLabel_);
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

        // Assignment cue (T-0332 / I-0089): during a drag-up-to-band, outline the target
        // band and draw a leader line from the dragged dot to it, so the writer can see
        // they're in assignment mode and which act they'll drop into.
        if (dragMode_ == DragMode::DotToBand && dragBandTarget_ >= 0
            && dragBandTarget_ < bands_.size() && pressedDot_ >= 0) {
            const double bl = bandLeftX(dragBandTarget_);
            const double br = bandRightX(dragBandTarget_);
            const QColor hi = pal.color(QPalette::Highlight);
            painter.setPen(QPen(hi, 2.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(bl, top, br - bl, bottom - top));
            // Leader from the dragged dot's baseline position up to the band label row.
            const double dotX = xForOffset(dots_.at(pressedDot_).offsetMs);
            painter.setPen(QPen(hi, 1.5, Qt::DashLine));
            painter.drawLine(QPointF(dotX, cy),
                             QPointF(dragPos_.x(), bandLabelRowHeight() + 2.0));
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

    // Epoch marker: the origin (offset 0 = Story Open) is anchored to its REAL story-time
    // position through the zoom/pan window, so it tracks scroll/zoom and sits where the
    // epoch actually is — which is no longer the left edge once scenes exist before it
    // (flashbacks at negative offsets). A short vertical tick marks the origin; the label
    // sits just right of it. When the origin scrolls off-screen, pin the label to the
    // nearer edge with a ‹/› hint so the writer still knows which way Story Open lies.
    const double originX = xForOffset(0);
    const double leftEdge  = kSideInset;
    const double rightEdge = width() - kSideInset;
    painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
    if (originX >= leftEdge && originX <= rightEdge) {
        // Origin tick across the strip's dot band.
        painter.setPen(QPen(pal.color(QPalette::Mid), 1.0, Qt::DashLine));
        painter.drawLine(QPointF(originX, cy - kDotRadius - 6.0),
                         QPointF(originX, cy + kDotRadius + 6.0));
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
        painter.drawText(QPointF(originX + 4.0, cy + kDotRadius + 16.0), epochLabel_);
    } else if (originX < leftEdge) {
        // Origin is off the left → Story Open lies to the left.
        painter.drawText(QPointF(leftEdge, cy + kDotRadius + 16.0),
                         tr("‹ %1").arg(epochLabel_));
    } else {
        // Origin is off the right → Story Open lies to the right.
        const QString txt = tr("%1 ›").arg(epochLabel_);
        const double w = painter.fontMetrics().horizontalAdvance(txt);
        painter.drawText(QPointF(rightEdge - w, cy + kDotRadius + 16.0), txt);
    }
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
        // Empty area (no dot, no border): pan when zoomed in (SP-083, T-0335). At
        // zoom 1 there's nothing to pan, so leave it to the base class.
        if (zoom_ > 1.0) {
            dragMode_ = DragMode::Pan;
            panStartFraction_ = panFraction_;
            setCursor(Qt::ClosedHandCursor);
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

    // --- Pan drag (SP-083, T-0335) ----------------------------------------
    if (dragMode_ == DragMode::Pan) {
        const double usable = std::max(1.0, width() - 2.0 * kSideInset);
        // Dragging right moves the content right → the visible window moves left (pan
        // decreases). Convert the pixel delta to a full-window-fraction delta (÷ zoom_).
        const double deltaFrac = (p.x() - pressPos_.x()) / usable / zoom_;
        panFraction_ = panStartFraction_ - deltaFrac;
        clampPan();
        update();
        event->accept();
        return;
    }

    // --- Border re-proportion drag (T-0331) -------------------------------
    if (dragMode_ == DragMode::Border && draggingBorder_ >= 0
        && draggingBorder_ + 1 < bands_.size()) {
        // Proportions are fractions of the BAND REGION ([0, last-scene-end] on screen),
        // not the whole strip — so re-proportioning stays correct now that bands wrap
        // only the main storyline and move with zoom/pan.
        const double regionLeft = bandRegionLeftX();
        const double regionSpan = std::max(1.0, bandRegionRightX() - regionLeft);
        const int i = draggingBorder_;
        // Move proportion between band i and i+1: band i's new width runs from its left
        // edge to the pointer. Keep each above a 0.05 floor; their pair-sum is constant
        // so no other band shifts.
        const double pairSum = dragProportions_.at(i) + dragProportions_.at(i + 1);
        const double bandStartFrac = (bandLeftX(i) - regionLeft) / regionSpan;
        const double leftProp = std::clamp((p.x() - regionLeft) / regionSpan - bandStartFrac,
                                           0.05, pairSum - 0.05);
        dragProportions_[i]     = leftProp;
        dragProportions_[i + 1] = pairSum - leftProp;
        update();
        event->accept();
        return;
    }

    // --- Dot drag: classify by DOMINANT DIRECTION, re-evaluated while still
    // ambiguous (I-0089) -------------------------------------------------------
    // The old code decided the mode on the FIRST 4px move and latched it. But the dots sit
    // at the strip's vertical centre (~60px) while the band label row is the top ~22px, so
    // on that first micro-move the pointer is nowhere near the row → it always latched to
    // DotHorizontal and the drag-up assignment could NEVER trigger. Instead: an UPWARD
    // drag (dy dominant, moving up) with a structure present → DotToBand; a sideways drag →
    // DotHorizontal. Keep re-evaluating until one axis clearly wins, so a drag that starts
    // slightly sideways but then heads up still becomes an assignment.
    if (pressedDot_ >= 0) {
        const double dx = p.x() - pressPos_.x();
        const double dy = p.y() - pressPos_.y();
        const bool pastThreshold =
            std::abs(dx) > kDragThreshold || std::abs(dy) > kDragThreshold;
        // (Re)classify while the mode isn't locked to a horizontal story-time drag. Once
        // DotHorizontal is chosen we keep it (the picker preview is live); DotToBand may be
        // revised back to horizontal only if the drag turns clearly sideways.
        if (pastThreshold && dragMode_ != DragMode::DotHorizontal) {
            const bool upward = dy < 0 && std::abs(dy) > std::abs(dx);
            if (!bands_.isEmpty() && upward) {
                dragMode_ = DragMode::DotToBand;
            } else if (std::abs(dx) >= std::abs(dy)) {
                dragMode_ = DragMode::DotHorizontal;
            }
            // else: still ambiguous (small, diagonal) — leave as-is until it resolves.
        }
        if (dragMode_ == DragMode::DotHorizontal) {
            dragX_ = std::clamp(p.x(), kSideInset, width() - kSideInset);
            update();
            event->accept();
            return;
        }
        if (dragMode_ == DragMode::DotToBand) {
            // Track the pointer + the band it currently targets so paint can show a cue
            // (a highlighted band + a leader line from the dot), making "assignment mode"
            // legible — the drop resolves on release via bandIndexAtX(release x).
            dragPos_         = p;
            dragBandTarget_  = bandIndexAtX(p.x());
            update();
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

    if (dragMode_ == DragMode::Pan) {
        dragMode_ = DragMode::None;
        unsetCursor();
        emit viewStateChanged(zoom_, panFraction_);   // persist the panned position (T-0338)
        event->accept();
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
        dragBandTarget_ = -1;
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
