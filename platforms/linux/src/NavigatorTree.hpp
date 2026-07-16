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
//   • Only SCENE rows drag (chapter rows carry no kSceneIDRole and are not draggable —
//     chapter drag-reorder is SP-068). The model marks scene items ItemIsDragEnabled.
//   • setDropIndicatorShown(true) draws the horizontal insertion line so the landing
//     position (including chapter-boundary positions) is unambiguous.
//   • dragMoveEvent gates legal drops: a scene may land BETWEEN scenes (Above/Below a
//     scene row) or as a chapter's first scene (Above the chapter's first scene row);
//     a drop directly ONTO a scene, or onto a chapter heading in a meaningless way, is
//     rejected. The drop must not be a no-op (landing exactly where the scene already
//     is).
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

    // The scene row the current drag started from, captured in startDrag. Cleared when a
    // drag isn't in progress.
    QString dragSourceSceneID_;

    // Resolve the drop at viewport point `pos` (with drop-indicator position `ipos`)
    // into (targetChapterID, afterSceneID). Returns false if the drop is illegal (onto
    // a scene, an undeterminable target, or a no-op given `draggedID`). On success the
    // out-params are filled and the drop is a legal landing.
    bool resolveDrop(const QString& draggedID,
                     const QModelIndex& dropIndex,
                     int ipos,
                     QString& targetChapterID,
                     QString& afterSceneID) const;
};
