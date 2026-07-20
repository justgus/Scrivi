#include "NavigatorTree.hpp"

#include <QAbstractItemModel>
#include <QDrag>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
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
    // Capture the grabbed row NOW, from the drag's source index — this is the only
    // point where the row under the cursor is guaranteed to be the one the user picked
    // up. Reading currentIndex() later (in dragMoveEvent/dropEvent) is unreliable: a drag
    // can move the current/selection, so the dragged id must be latched here. A scene row
    // carries kSceneIDRole; a chapter row carries kChapterIDRole (SP-073) — exactly one
    // latch is set per drag, and the other resolver rejects the drop.
    const QModelIndex src = currentIndex();
    dragSourceSceneID_ = src.isValid() ? src.data(kSceneIDRole).toString() : QString();
    dragSourceChapterID_ =
        (dragSourceSceneID_.isEmpty() && src.isValid())
            ? src.data(kChapterIDRole).toString()
            : QString();

    if (!dragSourceChapterID_.isEmpty()) {
        // CHAPTER drag (SP-073). Chapter rows must be SELECTABLE for this to ever run:
        // QAbstractItemView::mouseMoveEvent enters DraggingState (and calls startDrag)
        // only when the pressed row is among the selected draggable indexes (I-0082 —
        // a non-selectable heading fell into rubber-band selection instead). Run the
        // drag loop manually rather than chaining to the base: same CopyAction
        // discipline (Qt never mutates the model; our rebuild is the only thing that
        // changes it), same dragMove/drop pipeline, and independent of whatever else
        // is in the selection. The mime payload is irrelevant — the drop uses the
        // latched id — but it must be the model's own format so dragEnterEvent
        // accepts it.
        QDrag drag(this);
        QMimeData* mime = model() != nullptr
                              ? model()->mimeData({src})
                              : nullptr;
        drag.setMimeData(mime != nullptr ? mime : new QMimeData);
        drag.exec(Qt::CopyAction);

        dragSourceChapterID_.clear();
        dragSourceSceneID_.clear();
        return;
    }

    // Force CopyAction (never Move), regardless of what the caller passed: a Move drop
    // lets the base class remove the source row from the model after acceptance, which is
    // the I-0067/I-0068 vanish. We own the move; Qt must not mutate the model. (startDrag
    // runs the modal drag loop and returns when the drag ends.)
    QTreeView::startDrag(Qt::CopyAction);

    // Drag finished — clear the latches so a stale id can't leak into a later drop.
    dragSourceSceneID_.clear();
    dragSourceChapterID_.clear();
}

QString NavigatorTree::draggedSceneID() const
{
    // The scene id latched at startDrag from the grabbed row — stable for the whole drag,
    // unlike currentIndex()/selection which a drag can move.
    return dragSourceSceneID_;
}

QString NavigatorTree::draggedChapterID() const
{
    // The chapter id latched at startDrag (SP-073) — set only for a chapter-row drag.
    return dragSourceChapterID_;
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

bool NavigatorTree::resolveChapterDrop(const QString& draggedChID,
                                       const QModelIndex& dropIndex,
                                       int ipos,
                                       QString& afterChapterID) const
{
    if (draggedChID.isEmpty() || model() == nullptr) {
        return false;
    }

    // Top-level chapter rows in manuscript order, and the dragged chapter's current
    // predecessor (for the no-op guard).
    const int chapterCount = model()->rowCount();
    if (chapterCount < 2) {
        return false;   // nothing to reorder against
    }
    auto chapterIDAtRow = [this](int row) {
        return model()->index(row, 0).data(kChapterIDRole).toString();
    };
    QString predOfDragged;   // chapter right before the dragged one (empty = it's first)
    {
        QString prev;
        for (int row = 0; row < chapterCount; ++row) {
            const QString id = chapterIDAtRow(row);
            if (id == draggedChID) {
                predOfDragged = prev;
                break;
            }
            prev = id;
        }
    }

    if (!dropIndex.isValid()) {
        // Empty viewport area below the tree → land after the LAST chapter.
        afterChapterID = chapterIDAtRow(chapterCount - 1);
    } else {
        const QString dropSceneID   = dropIndex.data(kSceneIDRole).toString();
        const QString dropChapterID = dropIndex.data(kChapterIDRole).toString();

        if (!dropChapterID.isEmpty()) {
            // Hovering a CHAPTER heading. Only AboveItem is an unambiguous chapter
            // boundary: the chapter lands BEFORE this heading, i.e. after this
            // chapter's predecessor (empty = manuscript front). OnItem has no meaning
            // for a container move, and BelowItem sits between the heading and its
            // first scene — inside the chapter — so both are rejected.
            if (ipos != QAbstractItemView::AboveItem) {
                return false;
            }
            const int row = dropIndex.row();
            afterChapterID = (row > 0) ? chapterIDAtRow(row - 1) : QString();
        } else if (!dropSceneID.isEmpty()) {
            // Hovering a SCENE row: inside a chapter's scene list — not a chapter
            // boundary — EXCEPT BelowItem on the manuscript's very last scene, which
            // is the "after the last chapter" landing (reachable even when the tree
            // fills the viewport and no empty area exists).
            const QModelIndex chapterIdx = dropIndex.parent();
            const bool lastChapter = chapterIdx.isValid()
                                     && chapterIdx.row() == chapterCount - 1;
            const bool lastScene = lastChapter
                                   && dropIndex.row()
                                          == model()->rowCount(chapterIdx) - 1;
            if (!(lastScene && ipos == QAbstractItemView::BelowItem)) {
                return false;
            }
            afterChapterID = chapterIDAtRow(chapterCount - 1);
        } else {
            return false;   // neither a chapter nor a scene row
        }
    }

    // No-op guards: a chapter can't land after itself, and landing after its current
    // predecessor leaves it exactly where it is.
    if (afterChapterID == draggedChID || afterChapterID == predOfDragged) {
        return false;
    }
    return true;
}

void NavigatorTree::dragMoveEvent(QDragMoveEvent* event)
{
    // Let the base class update the drop indicator + autoscroll, THEN veto illegal
    // landings so the insertion line only shows where a drop is actually accepted.
    QTreeView::dragMoveEvent(event);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QModelIndex dropIndex = indexAt(event->position().toPoint());
#else
    const QModelIndex dropIndex = indexAt(event->pos());
#endif

    bool legal = false;
    if (!draggedChapterID().isEmpty()) {
        // CHAPTER drag (SP-073): boundary-only landings.
        QString afterChapterID;
        legal = resolveChapterDrop(draggedChapterID(), dropIndex,
                                   dropIndicatorPosition(), afterChapterID);
    } else {
        QString targetChapterID;
        QString afterSceneID;
        legal = resolveDrop(draggedSceneID(), dropIndex, dropIndicatorPosition(),
                            targetChapterID, afterSceneID);
    }

    if (legal) {
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QModelIndex dropIndex = indexAt(event->position().toPoint());
#else
    const QModelIndex dropIndex = indexAt(event->pos());
#endif

    if (!draggedChapterID().isEmpty()) {
        // CHAPTER drop (SP-073): same CopyAction discipline as scenes — Qt never
        // touches the model; EditorShell::onChapterDropped + rebuildNavigator do.
        QString afterChapterID;
        if (resolveChapterDrop(draggedChapterID(), dropIndex,
                               dropIndicatorPosition(), afterChapterID)) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
            emit chapterDropRequested(draggedChapterID(), afterChapterID);
        } else {
            event->ignore();
        }
        return;
    }

    const QString draggedID = draggedSceneID();
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
