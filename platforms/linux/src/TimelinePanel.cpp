#include "TimelinePanel.hpp"

#include <QContextMenuEvent>
#include <QCursor>
#include <QHelpEvent>
#include <QMenu>
#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSet>
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
constexpr double kImportedRowHeight = 26.0;   // height per imported-timeline row (T-0342)
// The bottom band occupied by the +/- zoom buttons + the horizontal scrollbar
// (layoutControls): kBtn(20) + kMargin(4). Content (imported rows, baseline) must stay
// ABOVE this so the lowest imported row isn't hidden behind the scrollbar (I-0090).
constexpr double kBottomControlsHeight = 24.0;

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
    recomputeWindow();
    update();
}

void TimelinePanel::setHistoricalEvents(const QList<HistDot>& events)
{
    histDots_ = events;
    recomputeWindow();   // a historical event may extend the window past the scenes
    update();
}

void TimelinePanel::setImportedTimelines(const QList<ImportedRow>& rows)
{
    importedRows_ = rows;
    // The panel's minimum height grows by one row per VISIBLE import (FR-064) so a
    // resized-small strip still shows every row ABOVE the bottom controls band (I-0090).
    // Base = the scene-row min (already includes room for the controls); each visible
    // imported row adds kImportedRowHeight.
    const int visibleCount = static_cast<int>(visibleImportedRowIndices().size());
    setMinimumHeight(kMinHeight + visibleCount * static_cast<int>(kImportedRowHeight));
    update();
}

QList<int> TimelinePanel::visibleImportedRowIndices() const
{
    QList<int> out;
    for (int i = 0; i < importedRows_.size(); ++i) {
        if (importedRows_.at(i).visible) {
            out.append(i);
        }
    }
    return out;
}

double TimelinePanel::projectRowY() const
{
    // Reserve the bottom of the strip for the visible imported rows AND the zoom
    // controls/scrollbar band; the project (scene/historical) row centres in the space
    // that remains above them. With no imported rows this is (height-controls)/2 — the
    // baseline sits a touch above centre, clear of the controls.
    const int visibleCount = static_cast<int>(visibleImportedRowIndices().size());
    const double reserved = visibleCount * kImportedRowHeight + kBottomControlsHeight;
    const double top = bandLabelRowHeight();
    return top + (height() - top - reserved) / 2.0;
}

double TimelinePanel::importedRowY(int slot) const
{
    // Visible imported rows stack upward from ABOVE the bottom controls band (so the
    // lowest row isn't hidden behind the scrollbar, I-0090); slot 0 is the topmost
    // imported row (directly under the project row).
    const double bottom = height() - kBottomControlsHeight;
    return bottom - (slot + 0.5) * kImportedRowHeight;
}

int TimelinePanel::importedRowAt(const QPoint& p) const
{
    const QList<int> vis = visibleImportedRowIndices();
    for (int slot = 0; slot < vis.size(); ++slot) {
        const double cy = importedRowY(slot);
        if (std::abs(p.y() - cy) <= kImportedRowHeight / 2.0) {
            return vis.at(slot);
        }
    }
    return -1;
}

bool TimelinePanel::importedEventTooltipAt(const QPoint& p, const QPoint& globalPos) const
{
    // An imported dot under `p` → show {title, source name, computed story-time}
    // (§7.8/FR-063). Only visible rows + window-clipped events are hit-testable (they're
    // the only ones drawn). Returns true if a tooltip was shown.
    const QList<int> vis = visibleImportedRowIndices();
    for (int slot = 0; slot < vis.size(); ++slot) {
        const ImportedRow& row = importedRows_.at(vis.at(slot));
        const double ry = importedRowY(slot);
        for (const ImportedEvent& ev : row.events) {
            if (ev.projectOffsetMs < minMs_ || ev.projectOffsetMs > maxMs_) {
                continue;
            }
            const double cx = xForOffset(ev.projectOffsetMs);
            const double dx = p.x() - cx;
            const double dy = p.y() - ry;
            if (dx * dx + dy * dy <= kHitRadius * kHitRadius) {
                QString text = ev.title;
                if (!row.sourceName.isEmpty()) {
                    text += QStringLiteral("\n") + row.sourceName;
                }
                text += QStringLiteral("\n") + humanStoryTime(ev.projectOffsetMs);
                QToolTip::showText(globalPos, text, const_cast<TimelinePanel*>(this));
                return true;
            }
        }
    }
    return false;
}

void TimelinePanel::recomputeWindow()
{
    // Window over story-time: from the earliest event's offset to the latest event's
    // end, so the whole span maps across the strip. Both scene dots (offset+duration)
    // and historical dots (a point, offset) participate — a historical event before the
    // first scene or after the last must not be clipped. Guard against a zero-width
    // window (single scene / all-coincident / empty) so xForOffset never /0.
    bool     any = false;
    qint64   lo  = 0;
    qint64   hi  = 1;
    for (const Dot& d : dots_) {
        const qint64 end = d.offsetMs + std::max<qint64>(d.durationMs, 1);
        if (!any) { lo = d.offsetMs; hi = end; any = true; }
        else      { lo = std::min(lo, d.offsetMs); hi = std::max(hi, end); }
    }
    for (const HistDot& h : histDots_) {
        if (!any) { lo = h.offsetMs; hi = h.offsetMs + 1; any = true; }
        else      { lo = std::min(lo, h.offsetMs); hi = std::max(hi, h.offsetMs + 1); }
    }
    if (!any) { lo = 0; hi = 1; }
    if (hi <= lo) { hi = lo + 1; }
    minMs_ = lo;
    maxMs_ = hi;

    // Story-structure bands wrap the MAIN storyline only: story-time [0, last-scene-end],
    // NOT the flashback region left of the epoch (user decision 2026-07-23). Use the
    // scene span (not historical) for the storyline end so a distant historical event
    // doesn't stretch the acts. Guard against a zero-width span.
    qint64 sceneEnd = 1;
    for (const Dot& d : dots_) {
        sceneEnd = std::max<qint64>(sceneEnd, d.offsetMs + std::max<qint64>(d.durationMs, 1));
    }
    storyEndMs_ = std::max<qint64>(sceneEnd, 1);
}

double TimelinePanel::memberX(const ClusterMember& m) const
{
    return xForOffset(m.offsetMs);
}

QList<TimelinePanel::Aggregate> TimelinePanel::computeClusters() const
{
    // Gather all project-row members (scene + historical dots) as a flat list, sorted by
    // story-time. Then greedily group runs whose screen-x are within one dot-diameter of
    // the run's first member — a co-located cluster (FR-032). A run of ≥2 becomes an
    // Aggregate; singletons are left to render normally. Because the threshold is a
    // SCREEN distance through xForOffset, zooming in widens the pixel gaps and a cluster
    // naturally resolves into singletons (zoom-resolve, FR-032).
    QList<ClusterMember> members;
    members.reserve(dots_.size() + histDots_.size());
    for (int i = 0; i < dots_.size(); ++i) {
        members.append({true, i, dots_.at(i).offsetMs});
    }
    for (int i = 0; i < histDots_.size(); ++i) {
        members.append({false, i, histDots_.at(i).offsetMs});
    }
    std::sort(members.begin(), members.end(),
              [](const ClusterMember& a, const ClusterMember& b) {
                  return a.offsetMs < b.offsetMs;
              });

    QList<Aggregate> aggregates;
    // A run starts co-locating within one dot-diameter (FR-032). But once ≥2 members
    // form an aggregate it renders LARGER (core + arc ring, ~kDotRadius+6 radius), so a
    // nearby member/aggregate that would visually overlap the bigger mark must still be
    // absorbed — otherwise two aggregates draw on top of each other (finding, T-0346).
    // So once a run is a pair, the gap is measured against the LAST absorbed member with
    // the wider AGGREGATE reach — a chain of co-located dots (and two aggregates that end
    // up within an aggregate-diameter) all merge into one.
    const double dotThreshold = 2.0 * kDotRadius;                 // singleton co-location
    const double aggThreshold = (kDotRadius + 6.0) + kDotRadius;  // aggregate reach ∪ next dot
    int i = 0;
    while (i < members.size()) {
        int j = i + 1;
        // Grow the run: each candidate is measured against the PREVIOUS member's x, using
        // the dot diameter until the run is a pair, then the aggregate reach.
        while (j < members.size()) {
            const double prevX = memberX(members.at(j - 1));
            const double threshold = (j - i >= 2) ? aggThreshold : dotThreshold;
            if (std::abs(memberX(members.at(j)) - prevX) > threshold) {
                break;
            }
            ++j;
        }
        if (j - i >= 2) {
            Aggregate agg;
            // Centre the aggregate on the run's MIDDLE member's x so the mark sits over the
            // cluster's visual centre rather than its left edge.
            agg.centerOffsetMs = members.at(i + (j - i) / 2).offsetMs;
            for (int k = i; k < j; ++k) {
                agg.members.append(members.at(k));
            }
            aggregates.append(agg);
        }
        i = j;
    }
    return aggregates;
}

void TimelinePanel::paintAggregate(QPainter& painter, const Aggregate& agg,
                                   double ax, double cy) const
{
    const QPalette& pal = palette();
    const int n = static_cast<int>(agg.members.size());
    const double core = kDotRadius + 2.0;    // slightly larger than a single dot (FR-031)
    const double ringR = core + 4.0;         // the segmented arc sits just outside the core

    // Segmented arc ring: one 360°/N wedge per member, in story order (the members are
    // already sorted). Qt angles are in 1/16-degree units, 0° at 3 o'clock, CCW+. We lay
    // segments clockwise from 12 o'clock to match the fan-out order. A small gap between
    // wedges keeps them legible.
    const QColor sceneTint = pal.color(QPalette::Text);
    const QColor histTint(0xC8, 0xA9, 0x7A);
    const QColor selColor  = pal.color(QPalette::Highlight);
    const double perSeg = 360.0 / n;
    const QRectF ringRect(ax - ringR, cy - ringR, 2.0 * ringR, 2.0 * ringR);
    for (int s = 0; s < n; ++s) {
        const ClusterMember& m = agg.members.at(s);
        const bool activeMember =
            m.isScene && dots_.at(m.index).sceneID == activeSceneID_;
        QColor c = m.isScene ? sceneTint : histTint;
        if (activeMember) { c = selColor; }             // selection arc (FR-031a)
        // Clockwise-from-12: startAngle in Qt units. 90° is 12 o'clock; subtract to go CW.
        const double startDeg = 90.0 - (s + 1) * perSeg + 1.0;   // +1° gap
        const double spanDeg  = perSeg - 2.0;                    // 2° total gap
        painter.setPen(QPen(c, 2.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(ringRect,
                        static_cast<int>(startDeg * 16.0),
                        static_cast<int>(spanDeg * 16.0));
    }

    // Larger core.
    painter.setPen(Qt::NoPen);
    painter.setBrush(pal.color(QPalette::Mid));
    painter.drawEllipse(QPointF(ax, cy), core, core);

    // Member count centred on the core (the only text allowed on the line, FR-031).
    painter.setPen(pal.color(QPalette::Text));
    QFont f = painter.font();
    f.setPointSizeF(std::max(6.0, f.pointSizeF() - 1.0));
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(QRectF(ax - core, cy - core, 2.0 * core, 2.0 * core),
                     Qt::AlignCenter, QString::number(n));
}

QPointF TimelinePanel::fanOutMemberPos(int slot, double ax, double cy) const
{
    // Hexagonal ring positions (FR-035b): slot 0 at the centre; ring r (r≥1) holds 6r
    // positions at 360°/(6r) increments, clockwise from 12 o'clock. Rings expand upward
    // from the baseline. Radius grows one dot-spacing per ring.
    if (slot <= 0) {
        return QPointF(ax, cy);
    }
    int idx = slot - 1;   // 0-based position among the ring slots
    int ring = 1;
    while (idx >= 6 * ring) {
        idx -= 6 * ring;
        ++ring;
    }
    const int count = 6 * ring;
    const double radius = ring * (2.0 * kDotRadius + 3.0);
    // Clockwise from 12 o'clock: angle measured CW from straight up.
    constexpr double kTwoPi = 6.283185307179586;
    const double angle = kTwoPi * (static_cast<double>(idx) / count);
    const double dx = radius * std::sin(angle);
    const double dy = -radius * std::cos(angle);   // up = negative y
    return QPointF(ax + dx, cy + dy);
}

double TimelinePanel::fanRadius(int memberCount) const
{
    // The outermost ring index used by `memberCount` members (slot 0 = centre, ring r
    // holds 6r slots), then that ring's radius + a dot + padding — the grey backing disc
    // must enclose every fanned dot.
    int remaining = memberCount - 1;   // slots beyond the centre
    int ring = 0;
    while (remaining > 0) {
        ++ring;
        remaining -= 6 * ring;
    }
    const double outer = ring * (2.0 * kDotRadius + 3.0);
    return outer + kDotRadius + 6.0;   // + dot + padding
}

void TimelinePanel::paintFanOut(QPainter& painter, const Aggregate& agg,
                                double ax, double cy) const
{
    const QPalette& pal = palette();
    const QColor sceneColor  = pal.color(QPalette::Text);
    const QColor activeColor = pal.color(QPalette::Highlight);
    const QColor histColor(0xC8, 0xA9, 0x7A);

    // Grey backing disc (finding, T-0348): the fan is an OVERLAY drawn above every other
    // timeline entity (it's painted last, after all dots), and a filled grey circle
    // behind the fanned dots makes it immediately readable over a busy background (bands,
    // neighbouring dots). A darker rim outlines the overlay.
    const double r = fanRadius(static_cast<int>(agg.members.size()));
    QColor backing = pal.color(QPalette::Window);
    // Nudge toward a neutral grey with high opacity so underlying marks don't bleed through.
    backing = QColor(0x88, 0x88, 0x88);
    backing.setAlpha(232);
    painter.setPen(QPen(pal.color(QPalette::Mid), 1.0));
    painter.setBrush(backing);
    painter.drawEllipse(QPointF(ax, cy), r, r);

    for (int s = 0; s < agg.members.size(); ++s) {
        const ClusterMember& m = agg.members.at(s);
        const QPointF p = fanOutMemberPos(s, ax, cy);
        const bool active =
            m.isScene && dots_.at(m.index).sceneID == activeSceneID_;
        painter.setPen(Qt::NoPen);
        painter.setBrush(m.isScene ? (active ? activeColor : sceneColor) : histColor);
        painter.drawEllipse(p, kDotRadius, kDotRadius);
        if (active) {
            painter.setPen(QPen(activeColor, 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(p, kDotRadius + 3.0, kDotRadius + 3.0);
        }
    }
}

int TimelinePanel::aggregateAtPoint(const QList<Aggregate>& aggs, const QPoint& p) const
{
    const double cy = projectRowY();
    const double core = kDotRadius + 2.0;
    const double hit = core + 4.0;   // core + arc ring, generous
    for (int a = 0; a < aggs.size(); ++a) {
        const double ax = xForOffset(aggs.at(a).centerOffsetMs);
        const double dx = p.x() - ax;
        const double dy = p.y() - cy;
        if (dx * dx + dy * dy <= hit * hit) {
            return a;
        }
    }
    return -1;
}

bool TimelinePanel::fanMemberAt(const QList<Aggregate>& aggs, int aggIndex,
                                const QPoint& p, ClusterMember& out) const
{
    if (aggIndex < 0 || aggIndex >= aggs.size()) {
        return false;
    }
    const double cy = projectRowY();
    const double ax = xForOffset(aggs.at(aggIndex).centerOffsetMs);
    const Aggregate& agg = aggs.at(aggIndex);
    for (int s = 0; s < agg.members.size(); ++s) {
        const QPointF fp = fanOutMemberPos(s, ax, cy);
        const double dx = p.x() - fp.x();
        const double dy = p.y() - fp.y();
        if (dx * dx + dy * dy <= kHitRadius * kHitRadius) {
            out = agg.members.at(s);
            return true;
        }
    }
    return false;
}

void TimelinePanel::updateHoverFan(const QPoint& p)
{
    const QList<Aggregate> aggs = computeClusters();
    int newFan = -1;

    // Keep the current fan while the pointer stays inside its grey backing disc — the
    // same radius the overlay is drawn at (finding, T-0348), so the fan dismisses as soon
    // as the pointer leaves the visible disc, no sooner and no later.
    if (fannedAggregate_ >= 0 && fannedAggregate_ < aggs.size()) {
        const double cy = projectRowY();
        const double ax = xForOffset(aggs.at(fannedAggregate_).centerOffsetMs);
        const double dx = p.x() - ax;
        const double dy = p.y() - cy;
        const double keep = fanRadius(
            static_cast<int>(aggs.at(fannedAggregate_).members.size()));
        if (dx * dx + dy * dy <= keep * keep) {
            newFan = fannedAggregate_;
        }
    }
    // Otherwise, fan out whichever aggregate's collapsed core the pointer is over.
    if (newFan < 0) {
        newFan = aggregateAtPoint(aggs, p);
    }

    if (newFan != fannedAggregate_) {
        fannedAggregate_ = newFan;
        update();
    }
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
    const double cy = projectRowY();
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

int TimelinePanel::histDotIndexAt(const QPoint& p) const
{
    const double cy = projectRowY();
    for (int i = 0; i < histDots_.size(); ++i) {
        const double cx = xForOffset(histDots_.at(i).offsetMs);
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

    // Empty-state: no scenes AND no historical events → centred hint, no baseline.
    // (A project with only historical events still draws the baseline + those dots.)
    if (dots_.isEmpty() && histDots_.isEmpty()) {
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("No scenes yet — the timeline is empty."));
        return;
    }

    const double cy = projectRowY();

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

    // --- SP-084 clustering (T-0346/0347): collapse co-located members ------
    // Every aggregate member is painted by the aggregate/fan-out code, NEVER by the
    // per-dot baseline loops — so ALL members go in the skip-sets, INCLUDING the fanned
    // aggregate's (paintFanOut draws those in the ring). Skipping the fanned aggregate
    // here was a bug: its members then drew twice — once in the ring and once at their
    // real baseline x — so members at the wide aggregate's screen extremes showed as
    // "phantom" dots outside the ring (and shared selection with their ring twin). The
    // ring is the ONLY place a fanned member is drawn.
    const QList<Aggregate> aggregates = computeClusters();
    QSet<int> skipScene;
    QSet<int> skipHist;
    for (const Aggregate& agg : aggregates) {
        for (const ClusterMember& m : agg.members) {
            if (m.isScene) { skipScene.insert(m.index); }
            else           { skipHist.insert(m.index); }
        }
    }

    // Dots. The active scene's dot is drawn filled with the highlight colour and a
    // ring; the rest use the text colour. An ASSIGNED dot (SP-081) shows a ring in its
    // band's color. While dragging, the dragged dot follows the live pointer x (dragX_)
    // for a horizontal drag, or the pointer for a drag-to-band.
    const QColor dotColor    = pal.color(QPalette::Text);
    const QColor activeColor = pal.color(QPalette::Highlight);
    for (int i = 0; i < dots_.size(); ++i) {
        if (skipScene.contains(i)) { continue; }   // collapsed into an aggregate (T-0347)
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

    // Historical-event dots (SP-082, T-0341): muted warm tone (#C8A97A, spec §7.2),
    // on the same baseline, visually distinct from scene (accent) + imported (grey)
    // dots. Draggable in story time; the dragged one follows the live pointer x.
    const QColor histColor(0xC8, 0xA9, 0x7A);
    for (int i = 0; i < histDots_.size(); ++i) {
        if (skipHist.contains(i)) { continue; }   // collapsed into an aggregate (T-0347)
        const HistDot& h = histDots_.at(i);
        const bool isDrag = (dragMode_ == DragMode::HistHorizontal && i == pressedHist_);
        const double cx = isDrag ? dragX_ : xForOffset(h.offsetMs);
        painter.setPen(Qt::NoPen);
        painter.setBrush(histColor);
        painter.drawEllipse(QPointF(cx, cy), kDotRadius, kDotRadius);
        if (isDrag) {
            painter.setPen(QPen(histColor, 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPointF(cx, cy), kDotRadius + 3.0, kDotRadius + 3.0);
        }
    }

    // --- SP-084 aggregate dots (T-0347) + fan-out (T-0348) ----------------
    // Each aggregate collapses ≥2 co-located members into one compact mark: a slightly
    // larger core (FR-031) with the member count centred, and a segmented arc ring —
    // one 360°/N wedge per member in story order, tinted by member type (scene =
    // accent-ish text tone, historical = warm), the active scene's wedge lit in the
    // selection colour (FR-031a). The arc is display-only. The fanned-out aggregate
    // (hover, T-0348) is drawn LAST (below), as an overlay above every other entity.
    for (int a = 0; a < aggregates.size(); ++a) {
        if (a == fannedAggregate_) { continue; }   // fanned one painted last (on top)
        const Aggregate& agg = aggregates.at(a);
        paintAggregate(painter, agg, xForOffset(agg.centerOffsetMs), cy);
    }
    // Fan-out overlay, drawn after ALL collapsed aggregates + dots so it's the topmost
    // project-row element (finding, T-0348): a grey backing disc + the member dots.
    if (fannedAggregate_ >= 0 && fannedAggregate_ < aggregates.size()) {
        const Aggregate& agg = aggregates.at(fannedAggregate_);
        paintFanOut(painter, agg, xForOffset(agg.centerOffsetMs), cy);
    }

    // --- Imported-timeline rows (SP-082, T-0342) --------------------------
    // Each VISIBLE imported timeline is a grey row below the project row: a faint
    // baseline, the source name at the left, and read-only grey dots. Only events whose
    // computed project story-time falls within the window are drawn (window-clipped,
    // §6.7/FR-061) — the rest are silently omitted. Each source uses its own grey shade.
    {
        const QList<int> vis = visibleImportedRowIndices();
        for (int slot = 0; slot < vis.size(); ++slot) {
            const ImportedRow& row = importedRows_.at(vis.at(slot));
            const double ry = importedRowY(slot);

            // Row baseline (fainter than the project baseline).
            painter.setPen(QPen(pal.color(QPalette::Mid), 1.0, Qt::DotLine));
            painter.drawLine(QPointF(kSideInset, ry), QPointF(width() - kSideInset, ry));

            // Source-name label at the left, elided so it never overruns the first dot.
            painter.setPen(pal.color(QPalette::Disabled, QPalette::Text));
            const QString label = painter.fontMetrics().elidedText(
                row.sourceName, Qt::ElideRight, 120);
            painter.drawText(QPointF(kSideInset, ry - kDotRadius - 3.0), label);

            // Grey dots, window-clipped.
            QColor grey(row.greyShade);
            if (!grey.isValid()) { grey = QColor(0x8A, 0x8A, 0x8A); }
            painter.setPen(Qt::NoPen);
            painter.setBrush(grey);
            for (const ImportedEvent& ev : row.events) {
                if (ev.projectOffsetMs < minMs_ || ev.projectOffsetMs > maxMs_) {
                    continue;   // outside the current window → clipped
                }
                painter.drawEllipse(QPointF(xForOffset(ev.projectOffsetMs), ry),
                                    kDotRadius - 1.0, kDotRadius - 1.0);
            }
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
        pressPos_    = event->position();
        pressedDot_  = -1;
        pressedHist_ = -1;
        dragMode_    = DragMode::None;
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

        // A fanned-out aggregate member (SP-084, T-0348): if the pointer is on a member
        // of the currently fanned aggregate, route the press to that member as if it were
        // a normal dot — scene → click-navigate / horizontal drag, historical → drag.
        // Checked before the collapsed dot hit-tests (the collapsed members are skipped).
        if (fannedAggregate_ >= 0) {
            const QList<Aggregate> aggs = computeClusters();
            ClusterMember m;
            if (fanMemberAt(aggs, fannedAggregate_, event->pos(), m)) {
                if (m.isScene) {
                    pressedDot_ = m.index;
                    dragX_ = xForOffset(dots_.at(pressedDot_).offsetMs);
                } else {
                    pressedHist_ = m.index;
                    dragX_ = xForOffset(histDots_.at(pressedHist_).offsetMs);
                }
                event->accept();
                return;
            }
        }

        pressedDot_ = dotIndexAt(event->pos());
        if (pressedDot_ >= 0) {
            dragX_ = xForOffset(dots_.at(pressedDot_).offsetMs);
            event->accept();
            return;
        }
        // Historical-event dot (T-0341): press it → a horizontal story-time drag on
        // release (no band assignment, no navigate). Same dragX_ live-preview path.
        pressedHist_ = histDotIndexAt(event->pos());
        if (pressedHist_ >= 0) {
            dragX_ = xForOffset(histDots_.at(pressedHist_).offsetMs);
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
        // --- SP-084 hover fan-out (T-0348) --------------------------------
        // With no button held, hovering an aggregate fans its members out; leaving the
        // fan collapses it. `fannedAggregate_` drives the paint + the fan-aware hit-tests.
        updateHoverFan(event->pos());
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

    // --- Historical-event dot drag (T-0341): horizontal only ---------------
    if (pressedHist_ >= 0) {
        const double dx = p.x() - pressPos_.x();
        const double dy = p.y() - pressPos_.y();
        if (std::abs(dx) > kDragThreshold || std::abs(dy) > kDragThreshold) {
            dragMode_ = DragMode::HistHorizontal;
        }
        if (dragMode_ == DragMode::HistHorizontal) {
            dragX_ = std::clamp(p.x(), kSideInset, width() - kSideInset);
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

    // Historical-event dot release (T-0341): a horizontal drag re-times it; a plain
    // press does nothing (historical events have no manuscript to navigate to — edit is
    // via the context menu, §7.7).
    if (pressedHist_ >= 0) {
        if (dragMode_ == DragMode::HistHorizontal) {
            emit historicalEventDragged(histDots_.at(pressedHist_).eventID,
                                        offsetForX(dragX_));
        }
        pressedHist_ = -1;
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
    if (i >= 0) {
        // --- Scene-dot menu (SP-080/SP-081) ---
        const QString sceneID = dots_.at(i).sceneID;
        QMenu menu(this);
        QAction* setDelta = menu.addAction(tr("Set Time Delta…"));   // SP-080

        // SP-081: band assignment entries when a structure is present. The shell owns
        // the band submenu (it knows the current bands), so we just signal intent.
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
        return;
    }

    const int hi = histDotIndexAt(event->pos());
    if (hi >= 0) {
        // --- Historical-event menu (SP-082, §7.7) ---
        const QString eventID = histDots_.at(hi).eventID;
        QMenu menu(this);
        QAction* edit = menu.addAction(tr("Edit Historical Event…"));
        menu.addSeparator();
        QAction* del = menu.addAction(tr("Delete Historical Event"));
        QAction* chosen = menu.exec(event->globalPos());
        if (chosen == edit) {
            emit editHistoricalEventRequested(eventID);
        } else if (chosen == del) {
            emit deleteHistoricalEventRequested(eventID);
        }
        event->accept();
        return;
    }

    const int ri = importedRowAt(event->pos());
    if (ri >= 0) {
        // --- Imported-row menu (SP-082, §7.8) ---
        const ImportedRow& row = importedRows_.at(ri);
        QMenu menu(this);
        QAction* editOffset = menu.addAction(tr("Edit Epoch Offset…"));
        QAction* hide = menu.addAction(tr("Hide This Timeline"));
        menu.addSeparator();
        QAction* remove = menu.addAction(tr("Remove Imported Timeline"));
        QAction* chosen = menu.exec(event->globalPos());
        if (chosen == editOffset) {
            emit editImportedOffsetRequested(row.timelineID);
        } else if (chosen == hide) {
            emit setImportedTimelineVisibleRequested(row.timelineID, false);
        } else if (chosen == remove) {
            emit removeImportedTimelineRequested(row.timelineID);
        }
        event->accept();
        return;
    }

    // --- Empty-area menu (SP-082, §7.9) ---
    // "New Historical Event Here" seeds the new event at the story-time under the click;
    // Import/Export are the file-dialog flows (T-0342/T-0343). A hidden imported row can
    // no longer be right-clicked (it isn't drawn), so a "Show Hidden Timelines" submenu
    // here is the un-hide path (FR-065's "via the panel menu"). The shell owns all flows.
    QMenu menu(this);
    QAction* newHist = menu.addAction(tr("New Historical Event Here"));
    menu.addSeparator();
    QAction* import = menu.addAction(tr("Import Timeline…"));
    QAction* exportT = menu.addAction(tr("Export Timeline…"));

    QHash<QAction*, QString> showActions;   // action → hidden timelineID
    QList<int> hidden;
    for (int i = 0; i < importedRows_.size(); ++i) {
        if (!importedRows_.at(i).visible) { hidden.append(i); }
    }
    if (!hidden.isEmpty()) {
        menu.addSeparator();
        QMenu* showMenu = menu.addMenu(tr("Show Hidden Timelines"));
        for (int i : hidden) {
            const ImportedRow& row = importedRows_.at(i);
            QAction* a = showMenu->addAction(
                row.sourceName.isEmpty() ? tr("(untitled timeline)") : row.sourceName);
            showActions.insert(a, row.timelineID);
        }
    }

    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == newHist) {
        emit newHistoricalEventRequested(offsetForX(event->pos().x()));
    } else if (chosen == import) {
        emit importTimelineRequested();
    } else if (chosen == exportT) {
        emit exportTimelineRequested();
    } else if (chosen != nullptr && showActions.contains(chosen)) {
        emit setImportedTimelineVisibleRequested(showActions.value(chosen), true);
    }
    event->accept();
}

bool TimelinePanel::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
        auto* help = static_cast<QHelpEvent*>(event);

        // SP-084 clustering (T-0348): a fanned-out member's own tooltip wins; over a
        // collapsed aggregate core, show a "N events" summary. Checked before the plain
        // dot hit-tests (which would otherwise tooltip an underlying clustered dot).
        const QList<Aggregate> aggs = computeClusters();
        if (fannedAggregate_ >= 0) {
            ClusterMember m;
            if (fanMemberAt(aggs, fannedAggregate_, help->pos(), m)) {
                QString text;
                if (m.isScene) {
                    const Dot& d = dots_.at(m.index);
                    text = d.title;
                    if (!d.chapterTitle.isEmpty()) {
                        text += QStringLiteral("\n") + d.chapterTitle;
                    }
                    text += QStringLiteral("\n") + humanStoryTime(d.offsetMs);
                } else {
                    const HistDot& h = histDots_.at(m.index);
                    text = h.title + QStringLiteral("\n") + tr("Historical event")
                           + QStringLiteral("\n") + humanStoryTime(h.offsetMs);
                }
                QToolTip::showText(help->globalPos(), text, this);
                return true;
            }
        }
        const int aggIdx = aggregateAtPoint(aggs, help->pos());
        if (aggIdx >= 0) {
            const int n = static_cast<int>(aggs.at(aggIdx).members.size());
            QToolTip::showText(help->globalPos(),
                               tr("%n co-located event(s) — hover to fan out", "", n),
                               this);
            return true;
        }

        const int i = dotIndexAt(help->pos());
        const int hi = (i < 0) ? histDotIndexAt(help->pos()) : -1;
        if (i >= 0) {
            const Dot& d = dots_.at(i);
            QString text = d.title;
            if (!d.chapterTitle.isEmpty()) {
                text += QStringLiteral("\n") + d.chapterTitle;
            }
            text += QStringLiteral("\n") + humanStoryTime(d.offsetMs);
            QToolTip::showText(help->globalPos(), text, this);
        } else if (hi >= 0) {
            const HistDot& h = histDots_.at(hi);
            QString text = h.title;
            text += QStringLiteral("\n") + tr("Historical event");
            text += QStringLiteral("\n") + humanStoryTime(h.offsetMs);
            QToolTip::showText(help->globalPos(), text, this);
        } else if (importedEventTooltipAt(help->pos(), help->globalPos())) {
            // handled inside the helper (imported dot: title + source + story-time)
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QWidget::event(event);
}
