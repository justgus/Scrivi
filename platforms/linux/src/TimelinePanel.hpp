#pragma once

#include <QList>
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
// Drag / Time Delta Picker / bands / historical + imported events / clustering /
// pan+zoom are LATER sprints (SP-080–SP-083). No scrivi_* calls live here — the
// EditorShell feeds it dots built from ScriviBridge::getTimeline +
// getSceneStoryTime.
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

    // Replace the panel's contents. `epochLabel` names the story-time origin
    // ("Story Open"); `dots` are in manuscript order. Triggers a repaint.
    void setTimeline(const QString& epochLabel, const QList<Dot>& dots);

    // Highlight the dot for `sceneID` as the active scene (empty clears). Repaints.
    void setActiveScene(const QString& sceneID);

signals:
    // A scene dot was clicked — the shell scrolls/selects that scene.
    void sceneClicked(const QString& sceneID);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    // Hover tooltip (title + human-readable story-time) via help events.
    bool event(QEvent* event) override;

private:
    // X pixel for a story-time offset, given the current [minMs, maxMs] window and
    // the horizontal insets. Returns the panel-space x of a dot's centre.
    double xForOffset(qint64 offsetMs) const;
    // The dot under panel-space point `p` (within the hit radius), or -1.
    int dotIndexAt(const QPoint& p) const;
    // Human-readable story-time for `offsetMs` relative to the epoch, e.g.
    // "3 days, 4 hours after Story Open" (or "at Story Open" for 0).
    QString humanStoryTime(qint64 offsetMs) const;

    QString      epochLabel_ = QStringLiteral("Story Open");
    QList<Dot>   dots_;
    QString      activeSceneID_;

    // Cached story-time window [min, max] over the current dots (recomputed in
    // setTimeline) so paint + hit-testing agree on the x mapping.
    qint64 minMs_ = 0;
    qint64 maxMs_ = 1;
};
