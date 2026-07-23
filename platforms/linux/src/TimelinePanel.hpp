#pragma once

#include <QHash>
#include <QList>
#include <QPointF>
#include <QString>
#include <QWidget>

// TimelinePanel — the Linux Timeline strip (EP-025 / SP-079, T-0322).
//
// The Linux mirror of Apple's TimelineStripView (EP-015/EP-016). A hideable
// HORIZONTAL strip that docks across the BOTTOM of the editor (below the
// navigator | viewport | inspector splitter) and paints one dot per scene,
// positioned by STORY-TIME.
//
// This sprint (SP-079) is the READ + LAYOUT + SELECT core:
//   • one dot per scene on a baseline, x-positioned by each scene's story-time
//     offset (computed from the default gap chain — mirrors Apple's
//     TimelineViewModel.recomputeAllOffsets);
//   • an empty-state message when the project has no scenes;
//   • click a dot → emit sceneClicked(sceneID) (the shell navigates);
//   • the active scene's dot is highlighted (setActiveScene);
//   • hover tooltip = title + human-readable story-time.
//
// SP-080 adds DOT DRAG + a right-click "Set Time Delta…": press-on-dot then move
// past a small threshold drags the dot horizontally (live preview); on release the
// shell opens the Time Delta Picker for the new offset. A press with no move stays a
// click (sceneClicked). Empty-background drag is left free for pan (SP-083). Bands /
// events / clustering / pan+zoom are SP-081–SP-083. No scrivi_* calls live here — the
// EditorShell feeds it dots and owns all persistence.
class TimelinePanel : public QWidget
{
    Q_OBJECT

public:
    explicit TimelinePanel(QWidget* parent = nullptr);

    // One scene's timeline dot. offsetMs/durationMs are already chain-resolved by
    // the shell (EditorShell::reloadTimeline); the panel only lays out + paints.
    struct Dot {
        QString sceneID;
        QString title;         // resolved ("Scene N" when untitled)
        QString chapterTitle;
        qint64  offsetMs   = 0;   // story-time start (from the gap chain)
        qint64  durationMs = 0;
    };

    // One story-structure band (EP-025 / SP-081, T-0330). Proportions across the
    // current bands sum to 1.0; the panel lays them across the full strip width.
    struct Band {
        QString bandID;
        QString label;
        QString color;        // hex "#RRGGBB"
        double  proportion = 0.0;
    };

    // Replace the panel's contents. `epochLabel` names the story-time origin
    // ("Story Open"); `dots` are in manuscript order. Triggers a repaint.
    void setTimeline(const QString& epochLabel, const QList<Dot>& dots);

    // Set the story-structure bands to paint behind the dots (empty = no structure →
    // no bands). `sceneBands` maps sceneID → its assigned bandID (for the colored ring).
    // Triggers a repaint.
    void setBands(const QList<Band>& bands,
                  const QHash<QString, QString>& sceneBands);

    // Highlight the dot for `sceneID` as the active scene (empty clears). Repaints.
    void setActiveScene(const QString& sceneID);

signals:
    // A scene dot was clicked (press with no drag) — the shell scrolls/selects it.
    void sceneClicked(const QString& sceneID);

    // A scene dot was dragged to a new story-time position (SP-080, T-0326). Emitted
    // on release after the pointer moved past the drag threshold. `newOffsetMs` is the
    // release x mapped back through the current story-time window (clamped ≥ 0). The
    // shell opens the Time Delta Picker seeded with this offset (T-0328).
    void dotDragged(const QString& sceneID, qint64 newOffsetMs);

    // Right-click ▸ "Set Time Delta…" on a dot (SP-080, T-0328) — opens the picker
    // without a drag gesture (Apple's context-menu entry). The shell shows the picker
    // seeded with the dot's current offset.
    void setTimeDeltaRequested(const QString& sceneID);

    // --- SP-081 story-structure signals -----------------------------------
    // Band borders were re-proportioned by a drag (T-0331): the shell persists the new
    // layout via update_band_layout. `proportions` is the full per-band list (same order
    // as the current bands), summing to 1.0.
    void bandProportionsChanged(const QList<double>& proportions);

    // A dot was dragged up onto a band label (T-0332): assign `sceneID` to `bandID`.
    void sceneAssignedToBand(const QString& sceneID, const QString& bandID);

    // Dot context-menu "Assign to Act…"/"Unassign" (T-0332): the shell shows a submenu
    // of the current bands (it knows them) + Unassign. Emitted on right-click of a dot
    // alongside setTimeDeltaRequested's menu — see contextMenuEvent.
    void assignBandRequested(const QString& sceneID);
    void unassignBandRequested(const QString& sceneID);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    // Hover tooltip (title + human-readable story-time) via help events.
    bool event(QEvent* event) override;

private:
    // X pixel for a story-time offset, given the current [minMs, maxMs] window and
    // the horizontal insets. Returns the panel-space x of a dot's centre.
    double xForOffset(qint64 offsetMs) const;
    // Story-time offset for a panel-space x (inverse of xForOffset), clamped ≥ 0.
    qint64 offsetForX(double x) const;
    // The dot under panel-space point `p` (within the hit radius), or -1.
    int dotIndexAt(const QPoint& p) const;
    // Human-readable story-time for `offsetMs` relative to the epoch, e.g.
    // "3 days, 4 hours after Story Open" (or "at Story Open" for 0).
    QString humanStoryTime(qint64 offsetMs) const;

    // --- SP-081 band geometry ---------------------------------------------
    // The label-row height at the top of the strip where band labels sit and where a
    // dot must be dragged up to for assignment (T-0332).
    double bandLabelRowHeight() const;
    // The band index whose proportional slice contains panel x, or -1.
    int    bandIndexAtX(double x) const;
    // The border index (between band i and i+1) within grab distance of x, or -1.
    int    borderIndexNearX(double x) const;
    // Left/right panel x of band i's slice, honouring dragProportions_ while a border
    // drag is live.
    double bandLeftX(int i) const;
    double bandRightX(int i) const;
    // The proportions used for layout right now (dragProportions_ mid-drag, else the
    // bands' own proportions).
    QList<double> effectiveProportions() const;

    QString      epochLabel_ = QStringLiteral("Story Open");
    QList<Dot>   dots_;
    QString      activeSceneID_;

    // Story structure (SP-081): the bands painted behind the dots + each scene's
    // assigned bandID (for the ring). Empty bands_ = no structure.
    QList<Band>             bands_;
    QHash<QString, QString> sceneBands_;

    // Cached story-time window [min, max] over the current dots (recomputed in
    // setTimeline) so paint + hit-testing agree on the x mapping.
    qint64 minMs_ = 0;
    qint64 maxMs_ = 1;

    // Drag state. A press selects at most ONE of these modes based on start zone:
    //   • DotHorizontal — press on a dot, moved (mostly) horizontally → story-time (SP-080)
    //   • DotToBand     — press on a dot, moved up into the label row → assignment (T-0332)
    //   • Border        — press on a band border → re-proportion (T-0331)
    // pressedDot_ is the dot under the press (-1 = none); pressPos_ is the press point.
    enum class DragMode { None, DotHorizontal, DotToBand, Border };
    DragMode dragMode_   = DragMode::None;
    int      pressedDot_ = -1;
    QPointF  pressPos_;
    double   dragX_      = 0.0;   // live pointer x during a dot horizontal drag

    // Border drag (T-0331): the border index being dragged + the live proportions.
    int           draggingBorder_ = -1;
    QList<double> dragProportions_;
};
