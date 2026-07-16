#include "NavigatorTree.hpp"

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QModelIndex>

using navigator::kChapterIDRole;
using navigator::kSceneIDRole;

NavigatorTree::NavigatorTree(QWidget* parent) : QTreeView(parent)
{
    // App-mediated scene drag-reorder (T-0260). We accept drops and draw the insertion
    // line, but resolve the drop ourselves (dropEvent does NOT chain to the base class),
    // so the model's rows are never auto-moved — the SceneDocument segments stay the
    // source of truth. DragDrop (not InternalMove) because we handle the move manually.
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    // CopyAction, NOT MoveAction (I-0067/I-0068): with a Move drop, QAbstractItemView::
    // startDrag removes the dragged SOURCE row from the model itself once the drop is
    // accepted — independent of our handler. That row-removal is what made a scene vanish
    // even when the backend reorder failed/aborted. We perform the real move ourselves
    // (sceneDropRequested → EditorShell::onSceneDropped → rebuildNavigator), so Qt must
    // never touch the model: a Copy drop leaves the source row in place, and our rebuild
    // is the ONLY thing that changes the navigator.
    setDefaultDropAction(Qt::CopyAction);
}

void NavigatorTree::startDrag(Qt::DropActions /*supportedActions*/)
{
    // Capture the grabbed scene row NOW, from the drag's source index — this is the only
    // point where the row under the cursor is guaranteed to be the one the user picked
    // up. Reading currentIndex() later (in dragMoveEvent/dropEvent) is unreliable: a drag
    // can move the current/selection, so the dragged id must be latched here. Only scene
    // rows carry kSceneIDRole (chapter rows aren't draggable), so a non-scene source
    // yields an empty id and resolveDrop rejects the drop.
    const QModelIndex src = currentIndex();
    dragSourceSceneID_ = src.isValid() ? src.data(kSceneIDRole).toString() : QString();

    // Force CopyAction (never Move), regardless of what the caller passed: a Move drop
    // lets the base class remove the source row from the model after acceptance, which is
    // the I-0067/I-0068 vanish. We own the move; Qt must not mutate the model. (startDrag
    // runs the modal drag loop and returns when the drag ends.)
    QTreeView::startDrag(Qt::CopyAction);

    // Drag finished — clear the latch so a stale id can't leak into a later drop.
    dragSourceSceneID_.clear();
}

QString NavigatorTree::draggedSceneID() const
{
    // The scene id latched at startDrag from the grabbed row — stable for the whole drag,
    // unlike currentIndex()/selection which a drag can move.
    return dragSourceSceneID_;
}

bool NavigatorTree::resolveDrop(const QString& draggedID,
                                const QModelIndex& dropIndex,
                                int ipos,
                                QString& targetChapterID,
                                QString& afterSceneID) const
{
    if (draggedID.isEmpty() || !dropIndex.isValid()) {
        return false;
    }

    const QString dropSceneID   = dropIndex.data(kSceneIDRole).toString();
    const QString dropChapterID = dropIndex.data(kChapterIDRole).toString();

    if (!dropSceneID.isEmpty()) {
        // Dropping relative to a SCENE row. Its parent is the owning chapter row.
        const QModelIndex chapterIdx = dropIndex.parent();
        targetChapterID = chapterIdx.data(kChapterIDRole).toString();
        if (targetChapterID.isEmpty()) {
            return false;   // orphan scene row — shouldn't happen
        }

        if (ipos == QAbstractItemView::OnItem) {
            return false;   // dropping ONTO a scene has no meaning — reject
        }
        if (ipos == QAbstractItemView::BelowItem) {
            // Land right after this scene.
            afterSceneID = dropSceneID;
        } else {
            // AboveItem (or OnViewport near this row): land before this scene, i.e.
            // after its previous sibling — or first in the chapter if it has none.
            const int row = dropIndex.row();
            if (row > 0) {
                const QModelIndex prev = dropIndex.sibling(row - 1, 0);
                afterSceneID = prev.data(kSceneIDRole).toString();
            } else {
                afterSceneID.clear();   // becomes the chapter's first scene
            }
        }
    } else if (!dropChapterID.isEmpty()) {
        // Dropping on/above a CHAPTER heading → make it that chapter's first scene.
        // (BelowItem on a chapter row is ambiguous — a chapter's first child covers it,
        // so treat any chapter-row drop as "first scene of this chapter".)
        targetChapterID = dropChapterID;
        afterSceneID.clear();
    } else {
        return false;   // neither a scene nor a chapter row
    }

    // No-op guards: can't land after itself, and dropping directly above/below the
    // dragged scene's own row is a landing back onto itself.
    if (afterSceneID == draggedID) {
        return false;
    }
    if (!dropSceneID.isEmpty() && dropSceneID == draggedID) {
        return false;
    }
    return true;
}

void NavigatorTree::dragMoveEvent(QDragMoveEvent* event)
{
    // Let the base class update the drop indicator + autoscroll, THEN veto illegal
    // landings so the insertion line only shows where a drop is actually accepted.
    QTreeView::dragMoveEvent(event);

    const QString draggedID = draggedSceneID();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QModelIndex dropIndex = indexAt(event->position().toPoint());
#else
    const QModelIndex dropIndex = indexAt(event->pos());
#endif
    QString targetChapterID;
    QString afterSceneID;
    if (resolveDrop(draggedID, dropIndex, dropIndicatorPosition(),
                    targetChapterID, afterSceneID)) {
        // Accept as a Copy (never Move) so the drag loop never asks the model to remove
        // the source row (I-0067/I-0068). The insertion line still shows here.
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void NavigatorTree::dropEvent(QDropEvent* event)
{
    // Resolve and emit; deliberately do NOT chain to QTreeView::dropEvent (which would
    // auto-move the model rows). EditorShell owns the real move + navigator rebuild.
    const QString draggedID = draggedSceneID();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QModelIndex dropIndex = indexAt(event->position().toPoint());
#else
    const QModelIndex dropIndex = indexAt(event->pos());
#endif
    QString targetChapterID;
    QString afterSceneID;
    if (resolveDrop(draggedID, dropIndex, dropIndicatorPosition(),
                    targetChapterID, afterSceneID)) {
        // Accept as CopyAction (never Move) so the source row is not auto-removed by the
        // drag machinery — our rebuildNavigator is the only thing that changes the model
        // (I-0067/I-0068). Then hand the resolved move to EditorShell.
        event->setDropAction(Qt::CopyAction);
        event->accept();
        emit sceneDropRequested(draggedID, targetChapterID, afterSceneID);
    } else {
        event->ignore();
    }
}
