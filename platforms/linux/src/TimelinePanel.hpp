#pragma once

#include <QHash>
#include <QList>
#include <QPointF>
#include <QString>
#include <QWidget>

#include <algorithm>
#include <limits>

class QToolButton;
class QTimer;
class QScrollBar;

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

    // One historical-event dot (EP-025 / SP-082, T-0341). Worldbuilding moments
    // (not scenes) painted on the project row in a muted warm tone (#C8A97A, spec
    // §7.2/FR-052). Draggable in story time; no manuscript association, no band ring.
    struct HistDot {
        QString eventID;
        QString title;
        QString description;
        QStringList tags;
        qint64  offsetMs = 0;   // story-time position (absolute, from the epoch)
    };

    // Replace the panel's contents. `epochLabel` names the story-time origin
    // ("Story Open"); `dots` are in manuscript order. Triggers a repaint.
    void setTimeline(const QString& epochLabel, const QList<Dot>& dots);

    // Set the historical events painted on the project row (EP-025 / SP-082, T-0341).
    // Fed by the shell from listHistoricalEvents. Recomputes the story-time window so
    // an event before the first scene / after the last is not clipped. Triggers a repaint.
    void setHistoricalEvents(const QList<HistDot>& events);

    // One imported external-timeline event (a read-only grey dot, EP-025 / SP-082,
    // T-0342). `projectOffsetMs` is the event's own offset + the row's epochOffsetMs,
    // i.e. its position in THIS project's story time; clipped to the window at paint.
    struct ImportedEvent {
        QString title;
        qint64  projectOffsetMs = 0;
    };

    // One imported-timeline row below the project row (EP-025 / SP-082, T-0342). Grey,
    // read-only, per-source shade; hidden rows are kept but not drawn (FR-065).
    struct ImportedRow {
        QString timelineID;
        QString sourceName;
        QString greyShade;     // hex "#RRGGBB"; blank → a default grey
        bool    visible = true;
        QList<ImportedEvent> events;
    };

    // Set the imported-timeline rows (EP-025 / SP-082, T-0342). Fed by the shell from
    // listImportedTimelines + the stored files (events aren't in the list projection).
    // Only visible rows occupy layout height + paint. Triggers a repaint.
    void setImportedTimelines(const QList<ImportedRow>& rows);

    // --- SP-084 co-located dot clustering (T-0346) ------------------------
    // One member of an aggregate: a project-row dot, either a scene (`isScene`, index
    // into dots_) or a historical event (index into histDots_). `offsetMs` is its
    // story-time position (the cluster pass sorts + groups by screen-x of this).
    struct ClusterMember {
        bool   isScene = true;
        int    index   = -1;     // into dots_ (scene) or histDots_ (historical)
        qint64 offsetMs = 0;
    };

    // A co-located group of ≥2 project-row members rendered as one aggregate dot
    // (larger core + count + segmented arc, T-0347). `centerOffsetMs` is the centre of the
    // story-time slice the members share ([I-0338]: `computeClusters`). Members are in story order.
    struct Aggregate {
        qint64 centerOffsetMs = 0;
        QList<ClusterMember> members;
    };

    // Set the story-structure bands to paint behind the dots (empty = no structure →
    // no bands). `sceneBands` maps sceneID → its assigned bandID (for the colored ring).
    // Triggers a repaint.
    void setBands(const QList<Band>& bands,
                  const QHash<QString, QString>& sceneBands);

    // Highlight the dot for `sceneID` as the active scene (empty clears). Repaints.
    void setActiveScene(const QString& sceneID);

    // --- SP-083 zoom/pan (public control API for the +/- buttons, T-0334) --
    // Step the zoom in/out about the current pointer (or the strip center if the
    // pointer is outside), keeping the story-time under that point fixed.
    void zoomInStep();
    void zoomOutStep();
    // The current zoom factor (1 = full-fit). Lets the shell enable/disable the
    // scrollbar + the − button.
    double zoomFactor() const { return zoom_; }

    // T-0640 (Apple's `zoomToCurrentScene`): zoom until the nearest other project-row dot sits a twelfth of the window
    // away from the active scene, so the two draw apart, then centre on it. No-op with no active scene.
    void zoomToActiveScene();

    // T-0641 (Apple's `TimelineScale`): the ticks along the top. ⚠️ No calendar (Timeline design principle 5): offsets
    // from the epoch counted from 1 ("Day 1" starts at Story Open), month = 30 days, year = 365 (the picker's units).
    struct Tick { qint64 ms = 0; QString label; };
    static QList<Tick> scaleTicks(qint64 visibleMinMs, qint64 visibleSpanMs, double usable,
                                  double minSpacing = 90.0);
    static QString scaleLabel(qint64 ms, qint64 step);

    // The current story-time window [min, max] over all dots (T-0342). Used by the
    // epoch-offset dialog to preview which imported events fall inside vs clipped.
    void storyTimeWindow(qint64& minMs, qint64& maxMs) const { minMs = minMs_; maxMs = maxMs_; }

    // --- SP-084 clustering test hooks (T-0349) ----------------------------
    // Headless assertions on the clustering pass without a real mouse/paint: the number
    // of aggregates at the current zoom/width, and the largest aggregate's member count.
    // Used only by timeline_cluster_smoke (co-located dots → one aggregate; a zoom that
    // separates them → zero aggregates). Public because the smoke is a separate binary.
    int aggregateCountForTest() const { return static_cast<int>(computeClusters().size()); }
    bool aggregateHoldsSceneForTest(int aggregate, int sceneIndex) const {
        const QList<Aggregate> aggs = computeClusters();
        if (aggregate < 0 || aggregate >= aggs.size()) { return false; }
        return std::any_of(aggs.at(aggregate).members.cbegin(), aggs.at(aggregate).members.cend(),
                           [sceneIndex](const ClusterMember& m) { return m.isScene && m.index == sceneIndex; });
    }
    double aggregateXForTest(int aggregate) const {
        const QList<Aggregate> aggs = computeClusters();
        return aggregate >= 0 && aggregate < aggs.size() ? xForOffset(aggs.at(aggregate).centerOffsetMs) : -1.0;
    }
    double projectRowYForTest() const { return projectRowY(); }
    int largestAggregateSizeForTest() const {
        int m = 0;
        for (const Aggregate& a : computeClusters()) {
            m = std::max<int>(m, static_cast<int>(a.members.size()));
        }
        return m;
    }

    // Restore a previously-persisted zoom + pan (T-0338). `zoom` is clamped to [1, maxZoom_] ([I-0340]);
    // `panFraction` is clamped to the valid range for that zoom. No signal is emitted (the
    // shell is applying saved state, not producing new state). Repaints + syncs the
    // scrollbar. Call after setTimeline (which resets the window).
    void setViewState(double zoom, double panFraction);
    double panFraction() const { return panFraction_; }

signals:
    // A scene dot was clicked (press with no drag) — the shell scrolls/selects it.
    void sceneClicked(const QString& sceneID);

    // A scene dot was dragged to a new story-time position (SP-080, T-0326). Emitted
    // on release after the pointer moved past the drag threshold. `newOffsetMs` is the
    // release x mapped back through the current story-time window; it MAY be negative
    // (dragged left of the epoch — a scene before Story Open). The shell opens the Time
    // Delta Picker seeded with this offset (T-0328).
    void dotDragged(const QString& sceneID, qint64 newOffsetMs);

    // Right-click ▸ "Set Time Delta…" on a dot (SP-080, T-0328) — opens the picker
    // without a drag gesture (Apple's context-menu entry). The shell shows the picker
    // seeded with the dot's current offset.
    void setTimeDeltaRequested(const QString& sceneID);

    // --- SP-082 historical-event signals (T-0341) -------------------------
    // A historical-event dot was dragged to a new story-time position. Emitted on
    // release after the pointer moved past the threshold; `newOffsetMs` may be negative
    // (before Story Open). The shell persists it via updateHistoricalEvent.
    void historicalEventDragged(const QString& eventID, qint64 newOffsetMs);

    // Empty-area right-click ▸ "New Historical Event Here" (§7.9). `atOffsetMs` is the
    // story-time under the click, so the new event lands where the writer pointed.
    void newHistoricalEventRequested(qint64 atOffsetMs);

    // Historical-dot context menu (§7.7): the shell opens the editor dialog / clears it.
    void editHistoricalEventRequested(const QString& eventID);
    void deleteHistoricalEventRequested(const QString& eventID);

    // Empty-area right-click ▸ Import / Export (§7.9). The shell runs the file dialogs
    // (T-0342 / T-0343). Kept here so the empty-area menu has a single home.
    void importTimelineRequested();
    void exportTimelineRequested();

    // --- SP-082 imported-timeline row signals (T-0342, §7.8) --------------
    // Right-click on an imported row: the shell runs the epoch-offset dialog / toggles
    // visibility / removes the stored file. `setVisible` carries the desired state.
    void editImportedOffsetRequested(const QString& timelineID);
    void setImportedTimelineVisibleRequested(const QString& timelineID, bool visible);
    void removeImportedTimelineRequested(const QString& timelineID);

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

    // Zoom changed (SP-083) — the shell may update the +/- control / scrollbar.
    void zoomChanged();

    // Zoom OR pan settled to a new user-produced value (T-0338) — the shell persists it
    // per-project (QSettings) so the timeline reopens at the same magnification/position.
    // NOT emitted by setViewState (restoring saved state must not re-trigger a save).
    void viewStateChanged(double zoom, double panFraction);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;   // [I-0286]: starts the members' window closing
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;   // Ctrl+wheel zoom (SP-083, T-0333)
    void resizeEvent(QResizeEvent* event) override; // reposition the +/- control + scrollbar
    // Hover tooltip (title + human-readable story-time) via help events.
    bool event(QEvent* event) override;

private:
    // X pixel for a story-time offset, given the current [minMs, maxMs] window and
    // the horizontal insets. Returns the panel-space x of a dot's centre.
    double xForOffset(qint64 offsetMs) const;
    // Story-time offset for a panel-space x (inverse of xForOffset). May be negative
    // (left of the epoch = a scene before Story Open).
    qint64 offsetForX(double x) const;
    // The dot under panel-space point `p` (within the hit radius), or -1.
    int dotIndexAt(const QPoint& p) const;
    // The historical-event dot under `p` (within the hit radius), or -1 (T-0341).
    int histDotIndexAt(const QPoint& p) const;
    // Recompute the story-time window [minMs_, maxMs_] + storyEndMs_ over the current
    // scene + historical dots. Called by setTimeline / setHistoricalEvents (T-0341).
    void recomputeWindow();

    // --- SP-084 clustering (T-0346) ---------------------------------------
    // Compute the co-located aggregates for the CURRENT frame (zoom/pan/width). Two
    // project-row members co-locate if their screen-x are within one dot-diameter
    // (FR-032), so zooming in separates them. Returns the aggregates (≥2 members each,
    // members in story order) and — via `clustered` — the set of member keys that are
    // in some aggregate, so paint/hit-test can skip them as singletons. Recomputed each
    // paint (cheap, O(n log n)); no cached invalidation needed since it reads live
    // geometry. A scene member key is +index, a historical key is −(index+1).
    QList<Aggregate> computeClusters() const;
    // Screen-x of a member's dot (scene or historical) — the value clustering groups by.
    double memberX(const ClusterMember& m) const;
    // Paint a collapsed aggregate at (ax, cy): larger core + count + segmented arc ring
    // (T-0347). Not interactive (the arc is display-only).
    void paintAggregate(QPainter& painter, const Aggregate& agg,
                        double ax, double cy) const;
    // ✅ [I-0286] Open (or keep) the window listing `agg`'s members, above it — never painted inside the strip.
    void showMemberPopup(const Aggregate& agg);
    // The aggregate index (into `aggs`) whose collapsed core contains `p`, or -1.
    int aggregateAtPoint(const QList<Aggregate>& aggs, const QPoint& p) const;

    // --- SP-082 imported-row geometry (T-0342) ----------------------------
    // The visible imported rows, in order (hidden rows are excluded from layout, FR-065).
    QList<int> visibleImportedRowIndices() const;
    // The baseline y of the project (scene/historical) row — pushed UP when imported rows
    // occupy the lower part of the strip so both fit. Replaces the bare height()/2 the
    // dots used before imported rows existed.
    double projectRowY() const;
    // The y-center of visible-imported-row slot `slot` (0-based among visible rows).
    double importedRowY(int slot) const;
    // The imported row index whose row band contains panel-space `p`, or -1 (for the
    // row context menu). Matches on the whole row strip, not just a dot.
    int importedRowAt(const QPoint& p) const;
    // If an imported dot is under `p`, show its tooltip (title + source + story-time) at
    // `globalPos` and return true; else return false (T-0342).
    bool importedEventTooltipAt(const QPoint& p, const QPoint& globalPos) const;
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
    // The band region's screen extent: bands wrap the MAIN storyline only — story-time
    // [0, storyEndMs_] mapped through zoom/pan — NOT the whole strip (so a flashback left
    // of the epoch has no band behind it, and bands expand/contract with zoom).
    double bandRegionLeftX() const;
    double bandRegionRightX() const;
    // Left/right panel x of band i's slice within the band region, honouring
    // dragProportions_ while a border drag is live.
    double bandLeftX(int i) const;
    double bandRightX(int i) const;
    // The proportions used for layout right now (dragProportions_ mid-drag, else the
    // bands' own proportions).
    QList<double> effectiveProportions() const;

    // --- SP-083 zoom/pan helpers ------------------------------------------
    // Zoom by `factor` about panel-x `anchorX`, keeping the story-time under it fixed.
    void zoomAbout(double factor, double anchorX);
    // Keep the visible window (width 1/zoom_) inside [0,1] of the full window.
    void clampPan();

    QString        epochLabel_ = QStringLiteral("Story Open");
    QList<Dot>     dots_;
    QList<HistDot> histDots_;   // historical events on the project row (SP-082, T-0341)
    QList<ImportedRow> importedRows_;   // imported grey rows below the project row (T-0342)
    QString        activeSceneID_;

    // Story structure (SP-081): the bands painted behind the dots + each scene's
    // assigned bandID (for the ring). Empty bands_ = no structure.
    QList<Band>             bands_;
    QHash<QString, QString> sceneBands_;

    // Cached story-time window [min, max] over the current dots (recomputed in
    // setTimeline) so paint + hit-testing agree on the x mapping.
    qint64 minMs_ = 0;
    qint64 maxMs_ = 1;

    // The main storyline's end (last scene end at/after the epoch), used for the band
    // region — bands wrap [0, storyEndMs_], not the negative flashback region. Recomputed
    // in setTimeline.
    qint64 storyEndMs_ = 1;

    // Drag state. A press selects at most ONE of these modes based on start zone:
    //   • DotHorizontal — press on a dot, moved (mostly) horizontally → story-time (SP-080)
    //   • DotToBand     — press on a dot, moved up into the label row → assignment (T-0332)
    //   • Border        — press on a band border → re-proportion (T-0331)
    //   • Pan           — press on empty area above/below the dots → pan (SP-083, T-0335)
    // pressedDot_ is the dot under the press (-1 = none); pressPos_ is the press point.
    //   • HistHorizontal — press on a historical-event dot, dragged → story-time (T-0341)
    enum class DragMode { None, DotHorizontal, DotToBand, Border, Pan, HistHorizontal };
    DragMode dragMode_    = DragMode::None;
    int      pressedDot_  = -1;
    int      pressedHist_ = -1;   // historical-event dot under the press (-1 = none), T-0341
    QPointF  pressPos_;
    double   dragX_      = 0.0;   // live pointer x during a dot horizontal drag
    QPointF  dragPos_;            // live pointer during a DotToBand drag (for the cue)
    int      dragBandTarget_ = -1;   // band the DotToBand drag currently targets (paint cue)

    // Border drag (T-0331): the border index being dragged + the live proportions.
    int           draggingBorder_ = -1;
    QList<double> dragProportions_;

    // [I-0286] The members' window, the aggregate it shows (by its slice centre: indices renumber as the clusters change)
    // and the timer that closes it once the pointer has left both.
    QWidget* popup_ = nullptr;
    QTimer*  popupCloseTimer_ = nullptr;
    qint64   popupCentreMs_ = std::numeric_limits<qint64>::min();

    // Zoom/pan (SP-083). zoom_ ≥ 1 (1 = full-fit); panFraction_ is the left edge of the
    // visible window as a fraction of the full [minMs_, maxMs_] window.
    double zoom_        = 1.0;
    double maxZoom_     = 500.0;   // [I-0340] set by recomputeWindow: span ÷ the closest two dots (floor 1 h)
    double panFraction_ = 0.0;
    double panStartFraction_ = 0.0;   // panFraction_ at the start of a pan drag

    // +/- zoom control (T-0334, bottom-right) + a horizontal scrollbar shown when
    // zoomed. Lay out in resizeEvent; syncScrollBar() keeps the bar's range/value in
    // step with zoom_/panFraction_.
    QToolButton* zoomSceneBtn_ = nullptr;   // T-0640
    QToolButton* zoomInBtn_  = nullptr;
    QToolButton* zoomOutBtn_ = nullptr;
    QScrollBar*  hScroll_    = nullptr;
    void layoutControls();
    void syncScrollBar();
};
