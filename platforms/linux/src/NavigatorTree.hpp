#pragma once

#include <QString>
#include <QTreeView>

// NavigatorTree — the scene navigator QTreeView with app-mediated scene
// drag-reorder (EP-023 / SP-067, T-0260, AC4).
//
// The navigator is a projection of the authoritative SceneDocument segments, never
// the source of truth. Qt's native InternalMove would reorder the MODEL rows but not
// the document / offset map / on-disk index — so this subclass intercepts the drop
// instead: it computes the drop target (which chapter, after which scene) from the
// row + drop-indicator position and emits sceneDropRequested. EditorShell then calls
// scrivi_reorder_scene, re-splices the document via SceneDocument::moveScene, and
// rebuilds the navigator from the segments. The base-class row auto-move is never
// invoked (dropEvent does not chain to QTreeView).
//
// Drag mechanics:
//   • SCENE rows drag (kSceneIDRole) and, since SP-073, CHAPTER rows drag too
//     (kChapterIDRole) — a chapter moves as a container with all its scenes. Which kind
//     of drag is in progress is latched at startDrag; the two resolve independently.
//   • setDropIndicatorShown(true) draws the horizontal insertion line so the landing
//     position (including chapter-boundary positions) is unambiguous.
//   • dragMoveEvent gates legal drops. A SCENE may land BETWEEN scenes (Above/Below a
//     scene row) or as a chapter's first scene (Above the chapter's first scene row);
//     a drop directly ONTO a scene, or onto a chapter heading in a meaningless way, is
//     rejected. A CHAPTER may land only at CHAPTER boundaries: Above a chapter heading
//     (→ before that chapter) or below the manuscript's last scene / on the empty
//     viewport area (→ after the last chapter); drops into a chapter's scene list are
//     rejected. Neither drop may be a no-op (landing exactly where the row already is).
class QStandardItemModel;

// Custom data roles on navigator items, shared by NavigatorTree (drop resolution) and
// EditorShell (row construction + activation). Scene rows carry kSceneIDRole; chapter
// rows carry kChapterIDRole. Each row carries exactly one of the two.
namespace navigator {
constexpr int kSceneIDRole   = Qt::UserRole + 1;
constexpr int kChapterIDRole = Qt::UserRole + 2;
} // namespace navigator

class NavigatorTree : public QTreeView
{
    Q_OBJECT

public:
    explicit NavigatorTree(QWidget* parent = nullptr);

signals:
    // A legal scene drop happened. `draggedSceneID` is the scene being moved;
    // `targetChapterID` is the chapter it should join; `afterSceneID` is the sibling it
    // should land right after, or empty to become the target chapter's first scene.
    // EditorShell performs the actual reorder + re-splice.
    void sceneDropRequested(const QString& draggedSceneID,
                            const QString& targetChapterID,
                            const QString& afterSceneID);

    // A legal CHAPTER drop happened (SP-073, AC5). `draggedChapterID` moves as a
    // container (with all its scenes) to sit right after `afterChapterID` — empty
    // means it becomes the manuscript's first chapter. EditorShell performs the
    // actual reorder + re-splice.
    void chapterDropRequested(const QString& draggedChapterID,
                              const QString& afterChapterID);

protected:
    // Runs the drag with CopyAction (never Move) and latches the source scene id. The
    // ROOT cause of the scene vanishing on drop (I-0068) and the reorder not persisting
    // (I-0067) is that a Move drop makes QAbstractItemView::startDrag remove the source
    // row from the model itself once the drop is accepted — regardless of whether our
    // handler did anything (so a backend-rejected reorder still deleted the row). Forcing
    // CopyAction here (and in drag/dropEvent) stops that: Qt never mutates the model; our
    // rebuildNavigator (via sceneDropRequested → EditorShell::onSceneDropped) is the only
    // thing that changes it. We also latch the dragged sceneID from the grabbed row here
    // (stable for the whole drag, unlike currentIndex() at drop time).
    void startDrag(Qt::DropActions supportedActions) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    // The sceneID being dragged, captured at startDrag from the grabbed row (empty if
    // the drag didn't originate on a single scene row). Stable for the whole drag,
    // unlike currentIndex()/selection.
    QString draggedSceneID() const;

    // The chapterID being dragged (SP-073), captured at startDrag from the grabbed
    // chapter row. Empty when the drag is a scene drag (exactly one of the two latches
    // is set per drag).
    QString draggedChapterID() const;

    // The scene row the current drag started from, captured in startDrag. Cleared when a
    // drag isn't in progress.
    QString dragSourceSceneID_;

    // The chapter row the current drag started from (SP-073). Cleared when a drag isn't
    // in progress.
    QString dragSourceChapterID_;

    // Resolve the drop at viewport point `pos` (with drop-indicator position `ipos`)
    // into (targetChapterID, afterSceneID). Returns false if the drop is illegal (onto
    // a scene, an undeterminable target, or a no-op given `draggedID`). On success the
    // out-params are filled and the drop is a legal landing.
    bool resolveDrop(const QString& draggedID,
                     const QModelIndex& dropIndex,
                     int ipos,
                     QString& targetChapterID,
                     QString& afterSceneID) const;

    // Resolve a CHAPTER drop (SP-073) into `afterChapterID` (empty = manuscript front).
    // Legal landings are chapter boundaries only: Above a top-level chapter row lands
    // before that chapter; BelowItem on the manuscript's LAST scene row, or a drop on
    // the empty viewport area (invalid dropIndex), lands after the last chapter.
    // Returns false for drops into a chapter's scene list, onto rows ambiguously, or
    // no-op landings (the chapter would stay exactly where it is).
    bool resolveChapterDrop(const QString& draggedChID,
                            const QModelIndex& dropIndex,
                            int ipos,
                            QString& afterChapterID) const;
};
