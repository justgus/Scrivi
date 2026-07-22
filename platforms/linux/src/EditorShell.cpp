#include "EditorShell.hpp"

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QShowEvent>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantList>

#include "ManuscriptEditor.hpp"
#include "NavigatorTree.hpp"
#include "SceneInspector.hpp"
#include "ScriviBridge.hpp"

namespace {
// Custom data roles on navigator items (shared with NavigatorTree's drop resolution):
// scene rows carry kSceneIDRole; chapter rows carry kChapterIDRole.
constexpr int kSceneIDRole   = navigator::kSceneIDRole;
constexpr int kChapterIDRole = navigator::kChapterIDRole;
// Idle-save debounce: how long after the last keystroke an auto-save fires.
// Mirrors Apple's ~1s; the sprint spec is ~1.5s.
constexpr int kSaveDebounceMs = 1500;
} // namespace

EditorShell::EditorShell(QWidget* parent) : QWidget(parent)
{
    bridge_    = new ScriviBridge(this);
    navModel_  = new QStandardItemModel(this);

    // --- Title bar --------------------------------------------------------
    // The project title only. Leaving the editor (Close/New/Open) is now driven by the
    // native File menu (SP-077, T-0316) — which flushes edits first — so the old raw
    // "‹ Close" button was removed. The `closeRequested` signal is retained for a future
    // proper button bar. Menu ▸ File ▸ Close Project is the flush-safe path today.
    auto* titleLabel = new QLabel(this);
    titleLabel->setObjectName(QStringLiteral("editorTitle"));
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto* toolbar = new QHBoxLayout;
    toolbar->addWidget(titleLabel, /*stretch=*/1);

    // --- Navigator (left) -------------------------------------------------
    navigator_ = new NavigatorTree(this);
    navigator_->setModel(navModel_);
    navigator_->setHeaderHidden(true);
    navigator_->setEditTriggers(QAbstractItemView::NoEditTriggers);  // display-only
    navigator_->setSelectionBehavior(QAbstractItemView::SelectRows);
    navigator_->setSelectionMode(QAbstractItemView::SingleSelection);
    // Click / keyboard both select a scene → scroll the viewport (T-0236).
    connect(navigator_, &QTreeView::activated,
            this, &EditorShell::onNavigatorActivated);
    connect(navigator_, &QTreeView::clicked,
            this, &EditorShell::onNavigatorActivated);
    // Right-click → Delete context menu on scene + chapter rows (EP-023, T-0251).
    navigator_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(navigator_, &QTreeView::customContextMenuRequested,
            this, &EditorShell::onNavigatorContextMenu);
    // Scene drag-reorder (EP-023 / SP-067, T-0260, AC4): NavigatorTree resolves the
    // drop and hands us (scene, targetChapter, afterScene); we do the real move.
    connect(navigator_, &NavigatorTree::sceneDropRequested,
            this, &EditorShell::onSceneDropped);
    connect(navigator_, &NavigatorTree::chapterDropRequested,
            this, &EditorShell::onChapterDropped);

    // --- Viewport (right): editable continuous document -------------------
    viewport_ = new ManuscriptEditor(this);
    viewport_->document()->setUndoRedoEnabled(false);    // custom undo owns ⌘Z later
    viewport_->setFrameShape(QFrame::NoFrame);
    viewport_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    // Editable (SP-062, T-0238). Boundary integrity is enforced by ManuscriptEditor;
    // per-scene dirty tracking + map maintenance happen in onContentsChange.

    // --- Inspector (right): EP-024 Scene Inspector panel ------------------
    // Docks as the THIRD pane of the same splitter (navigator | viewport |
    // inspector), the cleanest match for Apple's "between the manuscript surface
    // and the window edge". A UI stub for now (no project data). Session-scoped
    // visibility (inspectorVisible_); defaults SHOWN (Apple parity — user decision
    // 2026-07-22). View ▸ Show Inspector (T-0320) toggles it.
    inspector_ = new SceneInspector(this);

    splitter_ = new QSplitter(Qt::Horizontal, this);
    splitter_->addWidget(navigator_);
    splitter_->addWidget(viewport_);
    splitter_->addWidget(inspector_);
    splitter_->setStretchFactor(0, 0);   // navigator — fixed-ish
    splitter_->setStretchFactor(1, 1);   // viewport — takes the slack
    splitter_->setStretchFactor(2, 0);   // inspector — fixed-ish
    // Keep the inspector from being dragged away entirely; hide it via the menu.
    splitter_->setCollapsible(2, false);
    splitter_->setSizes({240, 580, 200});   // 200 = default inspector width (user pref)

    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet(QStringLiteral("color:#c0392b;"));
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addLayout(toolbar);
    root->addWidget(splitter_, /*stretch=*/1);
    root->addWidget(errorLabel_);

    // Idle-save debounce (T-0239): (re)armed on each edit; fires one saveDirtyScenes.
    saveTimer_ = new QTimer(this);
    saveTimer_->setSingleShot(true);
    saveTimer_->setInterval(kSaveDebounceMs);
    connect(saveTimer_, &QTimer::timeout, this, &EditorShell::saveDirtyScenes);

    // Scene-switch save: watch the caret so leaving a scene flushes it.
    connect(viewport_, &QPlainTextEdit::cursorPositionChanged,
            this, &EditorShell::onCursorMoved);

    // Scroll-driven active scene (T-0243): the visible scene becomes active. The
    // vertical scrollbar's valueChanged fires on every scroll tick.
    connect(viewport_->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &EditorShell::onScrolled);

    // In-editor scene creation (T-0240): Ctrl+Return in the ManuscriptEditor.
    connect(viewport_, &ManuscriptEditor::createSceneRequested,
            this, &EditorShell::onCreateSceneRequested);
    // In-editor chapter creation (T-0241): Ctrl+Shift+Return.
    connect(viewport_, &ManuscriptEditor::createChapterRequested,
            this, &EditorShell::onCreateChapterRequested);

    // In-editor scene/chapter merging (EP-028 / T-0306): Ctrl+Backspace merges the
    // current scene into the previous one; Ctrl+Shift+Backspace merges the current
    // chapter into the previous one. The editor only signals; these slots enforce the
    // guards and call the merge endpoints (macOS parity, SP-075).
    connect(viewport_, &ManuscriptEditor::mergeSceneRequested,
            this, &EditorShell::onMergeSceneRequested);
    connect(viewport_, &ManuscriptEditor::mergeChapterRequested,
            this, &EditorShell::onMergeChapterRequested);

    // Surface bridge errors inline rather than silently failing.
    connect(bridge_, &ScriviBridge::errorOccurred, this,
            [this](int code, const QString& message) {
                errorLabel_->setText(tr("Error %1: %2").arg(code).arg(message));
                errorLabel_->show();
            });
}

bool EditorShell::load(const QString& projectPath,
                       const QString& appSupportRoot,
                       const QString& title)
{
    errorLabel_->hide();
    errorLabel_->clear();

    if (auto* t = findChild<QLabel*>(QStringLiteral("editorTitle"))) {
        t->setText(title.isEmpty() ? tr("Project") : title);
    }

    // Bootstrap identity for this bridge (idempotent; resolves the same identity as
    // the landing bridge because it points at the same appSupportRoot).
    bridge_->bootstrap(QStringLiteral("Scrivi"), appSupportRoot);
    if (!bridge_->ready()) {
        errorLabel_->setText(tr("Could not resolve identity to open the project."));
        errorLabel_->show();
        return false;
    }

    const QVariantMap opened = bridge_->openProject(projectPath, appSupportRoot);
    if (opened.value(QStringLiteral("mode")).toString() != QStringLiteral("ready")) {
        // repairRequired / cannotOpen were already handled by the landing flow;
        // reaching here in a non-ready state is unexpected — show and bail.
        if (!errorLabel_->isVisible()) {
            errorLabel_->setText(tr("This project could not be opened into the editor."));
            errorLabel_->show();
        }
        return false;
    }

    const QString projectID = opened.value(QStringLiteral("projectID")).toString();
    const QVariantList scenes = opened.value(QStringLiteral("scenes")).toList();
    const QVariantMap active = opened.value(QStringLiteral("activeScene")).toMap();
    const QString activeSceneID = active.value(QStringLiteral("sceneID")).toString();
    const QString activeMarkdown = active.value(QStringLiteral("markdown")).toString();

    // Restored surface state for the active scene (T-0247): scene-local cursor
    // offsets + a scroll fraction. Applied after the document is assembled.
    const QVariantMap restored = opened.value(QStringLiteral("restored")).toMap();
    const int restoredAnchor = restored.value(QStringLiteral("anchor")).toInt();
    const int restoredFocus  = restored.value(QStringLiteral("focus")).toInt();
    const double restoredScroll = restored.value(QStringLiteral("scroll")).toDouble();

    // Stash identity for the save path (T-0239); reset save state for the new project.
    projectID_      = projectID;
    projectPath_    = projectPath;
    appSupportRoot_ = appSupportRoot;
    activeSegment_  = -1;
    saveTimer_->stop();

    // Assemble the continuous document in manuscript (scenes[]) order. The active
    // scene's body already arrived in openProject; fetch every other body via
    // scrivi_open_scene (Open Question #2's resolved path — no payload extension).
    QList<SceneDocument::Input> inputs;
    inputs.reserve(scenes.size());
    for (const QVariant& v : scenes) {
        const QVariantMap s = v.toMap();
        const QString sceneID = s.value(QStringLiteral("sceneID")).toString();

        SceneDocument::Input in;
        in.sceneID      = sceneID;
        in.chapterID    = s.value(QStringLiteral("chapterID")).toString();
        in.title        = s.value(QStringLiteral("title")).toString();
        in.chapterTitle = s.value(QStringLiteral("chapterTitle")).toString();
        in.slug         = s.value(QStringLiteral("slug")).toString();
        in.metadataPath = s.value(QStringLiteral("metadataPath")).toString();
        in.contentPath  = s.value(QStringLiteral("contentPath")).toString();
        // The chapter's own sidecar path (needed by rename + I-0063 renumber). Every
        // scene entry in open_project carries its chapter's chapterMetadataPath.
        in.chapterMetadataPath =
            s.value(QStringLiteral("chapterMetadataPath")).toString();

        if (sceneID == activeSceneID) {
            in.markdown = activeMarkdown;   // already have it — skip the round-trip
        } else {
            const QVariantMap sc =
                bridge_->openScene(projectPath, appSupportRoot, projectID, sceneID);
            in.markdown = sc.value(QStringLiteral("markdown")).toString();
        }
        inputs.append(in);
    }

    // Assemble the document under the loading_ guard so the programmatic inserts
    // in build() don't churn dirty flags / the offset map through onContentsChange.
    loading_ = true;
    dirtyScenes_.clear();
    sceneDoc_.build(inputs);
    viewport_->setDocument(sceneDoc_.document());
    // setDocument re-enables undo on the freshly-attached document; keep it off.
    viewport_->document()->setUndoRedoEnabled(false);
    // Point the boundary guard at the freshly-built map, and hook edits. The
    // connection is unique so re-loads don't stack duplicate handlers on the (new)
    // document; contentsChange belongs to the document, which setDocument replaced.
    viewport_->setSceneDocument(&sceneDoc_);
    connect(viewport_->document(), &QTextDocument::contentsChange,
            this, &EditorShell::onContentsChange, Qt::UniqueConnection);
    loading_ = false;

    // Populate the navigator: chapter parents → scene children.
    rebuildNavigator();

    // Apply the initial active scene (T-0247): restore the *saved* caret + scroll
    // within it, rather than snapping to the scene start. ScriviCore returns the
    // caret offsets scene-local (I-0058); map them to global by adding the segment's
    // body start, clamp into the body, and apply the whole-document scroll fraction.
    // (A navigator click still goes to the scene start — restore only fires here, on
    // the reopen's active scene, once.)
    const int initialSeg = activeSceneID.isEmpty()
                               ? 0
                               : sceneDoc_.sceneIndexForScene(activeSceneID);
    if (initialSeg >= 0) {
        const SceneSegment& seg = sceneDoc_.segments().at(initialSeg);
        const int bodyEnd = seg.bodyStart + seg.bodyLength;
        const int globalAnchor =
            qBound(seg.bodyStart, seg.bodyStart + restoredAnchor, bodyEnd);
        const int globalFocus =
            qBound(seg.bodyStart, seg.bodyStart + restoredFocus, bodyEnd);

        programmaticViewportChange_ = true;
        QTextCursor cursor(sceneDoc_.document());
        cursor.setPosition(globalAnchor);
        if (globalFocus != globalAnchor) {
            cursor.setPosition(globalFocus, QTextCursor::KeepAnchor);
        }
        viewport_->setTextCursor(cursor);
        viewport_->centerCursor();
        // Apply the saved scroll fraction over the whole document (overrides the
        // centerCursor scroll when a real fraction was persisted).
        if (QScrollBar* vsb = viewport_->verticalScrollBar()) {
            const int range = vsb->maximum() - vsb->minimum();
            if (range > 0 && restoredScroll > 0.0) {
                vsb->setValue(vsb->minimum()
                              + int(qRound(restoredScroll * range)));
            }
        }
        programmaticViewportChange_ = false;
        activeSegment_ = initialSeg;

        selectNavigatorScene(seg.sceneID);
    }
    // Seed the active-segment tracker from the caret so the first real cursor move
    // only saves when the scene genuinely changes.
    activeSegment_ = sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position());

    // Focus the writing surface so the author can type immediately (T-0246). If the
    // editor page isn't visible yet (loaded before the stack swaps to it), showEvent
    // will focus it when it appears.
    viewport_->setFocus();

    return true;
}

void EditorShell::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // The QStackedWidget swaps to the editor after load() returns, so focus the
    // writing surface here to be sure it takes on the page swap.
    if (viewport_ != nullptr) {
        viewport_->setFocus();
    }
}

void EditorShell::onNavigatorActivated(const QModelIndex& index)
{
    const QString sceneID = index.data(kSceneIDRole).toString();
    if (sceneID.isEmpty()) {
        return;   // a chapter row (no sceneID) — ignore
    }
    // A navigator click takes the CARET to the start of the clicked scene (the user
    // is about to review/edit from the top) and scrolls it into view — the same
    // deliberate caret placement create-scene uses. Moving the caret fires
    // onCursorMoved → promoteActiveScene, which saves the departing scene and updates
    // the navigator highlight. No separate caret-free scroll.
    moveCaretToSegment(sceneDoc_.sceneIndexForScene(sceneID));
    // Ensure the highlight reflects the click even when the scene was already active
    // (promoteActiveScene no-ops then).
    selectNavigatorScene(sceneID);
}

void EditorShell::onNavigatorContextMenu(const QPoint& pos)
{
    const QModelIndex index = navigator_->indexAt(pos);
    if (!index.isValid()) {
        return;
    }
    const QString sceneID   = index.data(kSceneIDRole).toString();
    const QString chapterID = index.data(kChapterIDRole).toString();

    QMenu menu(this);
    if (!sceneID.isEmpty()) {
        QAction* rename = menu.addAction(tr("Rename Scene…"));
        connect(rename, &QAction::triggered, this,
                [this, sceneID] { renameSceneByID(sceneID); });
        QAction* del = menu.addAction(tr("Delete Scene"));
        connect(del, &QAction::triggered, this,
                [this, sceneID] { deleteSceneByID(sceneID); });
    } else if (!chapterID.isEmpty()) {
        QAction* rename = menu.addAction(tr("Rename Chapter…"));
        connect(rename, &QAction::triggered, this,
                [this, chapterID] { renameChapterByID(chapterID); });
        QAction* del = menu.addAction(tr("Delete Chapter"));
        connect(del, &QAction::triggered, this,
                [this, chapterID] { deleteChapterByID(chapterID); });
    } else {
        return;   // neither a scene nor a chapter row
    }
    menu.exec(navigator_->viewport()->mapToGlobal(pos));
}

void EditorShell::deleteSceneByID(const QString& sceneID)
{
    const int idx = sceneDoc_.sceneIndexForScene(sceneID);
    if (idx < 0) {
        return;
    }
    const int sceneCount = sceneDoc_.segments().size();

    // Guard the manuscript's last remaining scene: a project must keep at least one
    // scene (mirrors the Apple navigator, and there is nowhere to place the caret
    // otherwise). Offer a clear notice instead of a silent no-op.
    if (sceneCount <= 1) {
        QMessageBox::information(
            this, tr("Delete Scene"),
            tr("This is the only scene in the manuscript and can't be deleted."));
        return;
    }

    const SceneSegment seg = sceneDoc_.segments().at(idx);   // copy (list mutates)
    const QString sceneTitle =
        seg.title.isEmpty() ? tr("this scene") : QStringLiteral("“%1”").arg(seg.title);

    const QMessageBox::StandardButton choice = QMessageBox::question(
        this, tr("Delete Scene"),
        tr("Delete %1? This can't be undone.").arg(sceneTitle),
        QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Cancel);
    if (choice != QMessageBox::Yes) {
        return;
    }

    // Persist any OTHER pending edits first so a background flush can't later resurrect
    // or clobber a file, then drop the doomed scene from the dirty set so auto-save
    // won't rewrite the .md we're about to delete.
    dirtyScenes_.remove(sceneID);
    saveDirtyScenes();

    const QVariantMap r = bridge_->deleteScene(projectPath_, sceneID);
    if (r.isEmpty()) {
        return;   // bridge already surfaced errorOccurred; leave the UI untouched
    }

    // Remember which scene was active so afterStructuralRemoval can keep it active at
    // its new index if it survived (i.e. a *different* scene was deleted). Empty when
    // the active scene is the one being deleted.
    const QString activeSceneID =
        (activeSegment_ >= 0 && activeSegment_ < sceneCount)
            ? sceneDoc_.segments().at(activeSegment_).sceneID
            : QString();

    // Nearest remaining scene = the one that slides into this slot (next), else the
    // previous one when we deleted the last segment. Only used when the active scene
    // was the deletion target.
    const int fallbackSeg = (idx < sceneCount - 1) ? idx : idx - 1;

    loading_ = true;
    sceneDoc_.removeScene(idx);
    loading_ = false;

    afterStructuralRemoval(activeSceneID, fallbackSeg);
}

void EditorShell::deleteChapterByID(const QString& chapterID)
{
    // Count the chapter's scenes for the confirmation message and the last-chapter
    // guard.
    int memberCount = 0;
    QString firstMemberSceneID;
    for (const SceneSegment& s : sceneDoc_.segments()) {
        if (s.chapterID == chapterID) {
            if (firstMemberSceneID.isEmpty()) {
                firstMemberSceneID = s.sceneID;
            }
            ++memberCount;
        }
    }
    if (memberCount == 0) {
        return;   // unknown chapter
    }

    // Guard the manuscript's last chapter: deleting every scene would leave an empty
    // manuscript with no caret home. Refuse with a notice (parity with the last-scene
    // guard above).
    if (memberCount >= sceneDoc_.segments().size()) {
        QMessageBox::information(
            this, tr("Delete Chapter"),
            tr("This is the only chapter in the manuscript and can't be deleted."));
        return;
    }

    const QMessageBox::StandardButton choice = QMessageBox::question(
        this, tr("Delete Chapter"),
        tr("Delete this chapter and all %n scene(s) in it? This can't be undone.",
           nullptr, memberCount),
        QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Cancel);
    if (choice != QMessageBox::Yes) {
        return;
    }

    // Was the active scene inside the doomed chapter? If so we must re-anchor after.
    const QString activeSceneID =
        (activeSegment_ >= 0 && activeSegment_ < sceneDoc_.segments().size())
            ? sceneDoc_.segments().at(activeSegment_).sceneID
            : QString();
    const int firstMemberIdx = sceneDoc_.sceneIndexForScene(firstMemberSceneID);

    // Drop every doomed scene from the dirty set, then flush the survivors.
    for (const SceneSegment& s : sceneDoc_.segments()) {
        if (s.chapterID == chapterID) {
            dirtyScenes_.remove(s.sceneID);
        }
    }
    saveDirtyScenes();

    const QVariantMap r = bridge_->deleteChapter(projectPath_, chapterID);
    if (r.isEmpty()) {
        return;
    }

    loading_ = true;
    sceneDoc_.removeChapter(chapterID);
    loading_ = false;

    // I-0063 (T-0262): chapters created via scrivi_create_chapter carry a STORED
    // "Chapter N" title, so deleting an earlier chapter leaves later created chapters
    // with a stale ordinal. Rewrite each auto-pattern chapter's sidecar to its new
    // position, then re-derive the live labels/headings so the navigator + document
    // match disk. Untitled chapters already renumbered via chapterHeadingText on the
    // rebuild below; custom titles are untouched.
    if (renumberCreatedChapters()) {
        applyDerivedLabels();
    }

    // Fallback active scene: the segment that now occupies the deleted chapter's first
    // slot (the following chapter's first scene), clamped into range.
    afterStructuralRemoval(activeSceneID, firstMemberIdx);
}

void EditorShell::afterStructuralRemoval(const QString& previouslyActiveSceneID,
                                         int fallbackSeg)
{
    rebuildNavigator();

    if (sceneDoc_.segments().isEmpty()) {
        // Should not happen — the last-scene / last-chapter guards prevent it — but
        // keep the tracker consistent if it ever does.
        activeSegment_ = -1;
        return;
    }

    // Does the previously-active scene still exist? (Empty ID = it was the deletion
    // target, so it's gone by definition.)
    const int survivingActive =
        previouslyActiveSceneID.isEmpty()
            ? -1
            : sceneDoc_.sceneIndexForScene(previouslyActiveSceneID);

    if (survivingActive >= 0) {
        // The active scene survived; just keep it active at its new index and let the
        // navigator highlight follow. No caret move (the author's place is preserved).
        activeSegment_ = survivingActive;
        selectNavigatorScene(sceneDoc_.segments().at(survivingActive).sceneID);
        return;
    }

    // The active scene was deleted: promote the nearest remaining scene, drop the
    // caret at its start, and hand focus back to the editor (delete-of-active, AC2).
    const int target =
        qBound(0, fallbackSeg, sceneDoc_.segments().size() - 1);
    activeSegment_ = -1;               // force moveCaretToSegment/promote to register it
    moveCaretToSegment(target);
    selectNavigatorScene(sceneDoc_.segments().at(target).sceneID);
    viewport_->setFocus();
}

void EditorShell::onSceneDropped(const QString& draggedSceneID,
                                 const QString& targetChapterID,
                                 const QString& afterSceneID)
{
    const int fromIdx = sceneDoc_.sceneIndexForScene(draggedSceneID);
    if (fromIdx < 0 || targetChapterID.isEmpty()) {
        return;
    }
    const SceneSegment moved = sceneDoc_.segments().at(fromIdx);   // copy (list mutates)
    const QString sourceChapterID = moved.chapterID;

    // The target chapter's display title: pull it from an existing member of that
    // chapter (a cross-chapter move joins a chapter that already has scenes; a
    // within-chapter move keeps its own title). Falls back to the moved scene's title
    // if the target chapter is currently only this scene (degenerate).
    QString targetChapterTitle = moved.chapterTitle;
    const int firstOfTarget = sceneDoc_.firstSegmentOfChapter(targetChapterID);
    if (firstOfTarget >= 0) {
        targetChapterTitle = sceneDoc_.segments().at(firstOfTarget).chapterTitle;
    }

    // Persist any pending edits first so the moved body (and its neighbours) are on disk
    // before the reorder rewrites the index. moveScene preserves the LIVE body, so a
    // stale save can't clobber it, but flushing keeps disk + document in lock-step.
    saveDirtyScenes();

    const QVariantMap r = bridge_->reorderScene(
        projectPath_, draggedSceneID, sourceChapterID, targetChapterID, afterSceneID);
    if (r.isEmpty()) {
        return;   // bridge already surfaced errorOccurred; leave the UI untouched
    }

    // Re-splice the live document + offset map to the new order (no full rebuild), then
    // rebuild the navigator projection and re-anchor the caret on the moved scene.
    loading_ = true;
    const int newIdx = sceneDoc_.moveScene(
        fromIdx, targetChapterID, targetChapterTitle, afterSceneID);
    loading_ = false;

    // Path refresh (SP-073 / I-0081): the reorder renamed the scene's files to their new
    // order-key stem (and relocated them cross-chapter), so the segment's captured paths
    // are stale — a rename or auto-save through them would target vanished files. The
    // envelope reports the post-move paths; apply them regardless of the splice outcome
    // (the reload fallback below reloads everything anyway, but a refreshed segment is
    // never wrong).
    sceneDoc_.refreshScenePaths(
        draggedSceneID,
        r.value(QStringLiteral("metadataPath")).toString(),
        r.value(QStringLiteral("contentPath")).toString(),
        r.value(QStringLiteral("chapterMetadataPath")).toString());

    if (newIdx < 0) {
        // The map move failed (unexpected — the bridge already reordered on disk). Fall
        // back to a full reload so the UI matches disk rather than drifting.
        load(projectPath_, appSupportRoot_,
             findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
                 ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
                 : QString());
        return;
    }

    rebuildNavigator();
    activeSegment_ = -1;                 // force promote to register the move
    moveCaretToSegment(newIdx);
    selectNavigatorScene(draggedSceneID);
    viewport_->setFocus();
}

void EditorShell::onChapterDropped(const QString& draggedChapterID,
                                   const QString& afterChapterID)
{
    if (draggedChapterID.isEmpty()
        || sceneDoc_.firstSegmentOfChapter(draggedChapterID) < 0) {
        return;
    }

    // Persist pending edits BEFORE the reorder: the reorder reslugs the chapter's
    // folder, so any dirty scene saved after it must use the refreshed paths — flush
    // now while every captured path is still valid.
    saveDirtyScenes();

    const QVariantMap r =
        bridge_->reorderChapter(projectPath_, draggedChapterID, afterChapterID);
    if (r.isEmpty()) {
        return;   // bridge already surfaced errorOccurred; leave the UI untouched
    }

    // Path refresh FIRST (I-0074/I-0079 class): scrivi_reorder_chapter renamed the
    // chapter's folder to its new order-key slug, so every captured metadataPath/
    // contentPath in that chapter is stale. The envelope reports the chapter's
    // post-reorder sidecar path; re-base the members' paths on it before anything
    // else can save — even the reload fallback below is then safe.
    sceneDoc_.refreshChapterPaths(
        draggedChapterID, r.value(QStringLiteral("metadataPath")).toString());

    // Re-splice the live document + offset map (chapter block move, no full rebuild).
    loading_ = true;
    const int newIdx = sceneDoc_.moveChapter(draggedChapterID, afterChapterID);
    loading_ = false;

    if (newIdx < 0) {
        // The map move failed (unexpected — the bridge already reordered on disk). Fall
        // back to a full reload so the UI matches disk rather than drifting.
        load(projectPath_, appSupportRoot_,
             findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
                 ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
                 : QString());
        return;
    }

    // I-0063: chapters created via scrivi_create_chapter carry a STORED "Chapter N"
    // title, so moving a chapter shuffles later created chapters' ordinals. Rewrite
    // the stale sidecars, then re-derive the live labels so navigator + document
    // match disk. (Untitled chapters already renumbered via reflowAllChapterHeadings
    // inside moveChapter; custom titles untouched.)
    if (renumberCreatedChapters()) {
        applyDerivedLabels();
    }

    rebuildNavigator();

    // Anchor the caret on the moved chapter's first scene — the container the writer
    // just placed — mirroring the scene-drag re-anchor.
    activeSegment_ = -1;                 // force promote to register the move
    moveCaretToSegment(newIdx);
    selectNavigatorScene(sceneDoc_.segments().at(newIdx).sceneID);
    viewport_->setFocus();
}

void EditorShell::refocusEditor()
{
    if (viewport_ != nullptr) {
        viewport_->setFocus();
    }
}

void EditorShell::applyDerivedLabels()
{
    // Pull the freshly-derived labels ScriviCore would show on a reload. This is the
    // authority for the fallback chain (custom title → first prose line / "Chapter N"),
    // so we never reimplement it UI-side.
    const QVariantMap opened = bridge_->openProject(projectPath_, appSupportRoot_);
    const QVariantList scenes = opened.value(QStringLiteral("scenes")).toList();
    if (scenes.isEmpty()) {
        return;
    }

    // Guard the document edits (setChapterTitle rewrites the live heading) so the
    // programmatic changes don't churn dirty flags / active-scene tracking.
    const bool wasLoading = loading_;
    loading_ = true;

    // Scene labels: match by sceneID, apply the derived title to that segment.
    for (const QVariant& v : scenes) {
        const QVariantMap s = v.toMap();
        const int i =
            sceneDoc_.sceneIndexForScene(s.value(QStringLiteral("sceneID")).toString());
        if (i >= 0) {
            sceneDoc_.setSceneTitle(i, s.value(QStringLiteral("title")).toString());
        }
    }

    // Chapter titles: apply once per chapter (first occurrence). setChapterTitle updates
    // every member's chapterTitle and rewrites the live heading in place.
    QSet<QString> seenChapters;
    for (const QVariant& v : scenes) {
        const QVariantMap s = v.toMap();
        const QString chapterID = s.value(QStringLiteral("chapterID")).toString();
        if (chapterID.isEmpty() || seenChapters.contains(chapterID)) {
            continue;
        }
        seenChapters.insert(chapterID);
        sceneDoc_.setChapterTitle(chapterID,
                                  s.value(QStringLiteral("chapterTitle")).toString());
    }

    loading_ = wasLoading;
}

bool EditorShell::renumberCreatedChapters()
{
    // I-0063 (Option A): created chapters carry a STORED "Chapter N" title (ScriviCore
    // stamps it at creation), so they don't renumber from order like untitled chapters
    // do. After a structural change, walk chapters in manuscript order and rewrite any
    // whose stored title is exactly the auto pattern to its new ordinal. A deliberately
    // user-typed title (which won't match the anchored pattern) is left untouched.
    //
    // The pattern is anchored (^Chapter \d+$) so a prose-y "Chapter of Secrets" is never
    // renumbered — only the literal auto-generated "Chapter <n>".
    static const QRegularExpression autoPattern(
        QStringLiteral("^Chapter \\d+$"));

    bool renamedAny = false;
    int ordinal = 0;
    QString lastChapterID;
    for (int i = 0; i < sceneDoc_.segments().size(); ++i) {
        const SceneSegment& seg = sceneDoc_.segments().at(i);
        if (seg.chapterID == lastChapterID) {
            continue;   // already handled this chapter (first segment did it)
        }
        lastChapterID = seg.chapterID;
        ++ordinal;

        const QString stored = seg.chapterTitle;   // the sidecar title from open_project
        if (!autoPattern.match(stored).hasMatch()) {
            continue;   // untitled (empty) or custom — no renumber
        }
        const QString desired = QStringLiteral("Chapter %1").arg(ordinal);
        if (stored == desired) {
            continue;   // already correct
        }
        if (seg.chapterMetadataPath.isEmpty()) {
            continue;   // can't target the sidecar — skip rather than guess
        }

        const QVariantMap r =
            bridge_->renameChapter(projectPath_, seg.chapterMetadataPath, desired);
        if (!r.isEmpty()) {
            renamedAny = true;
        }
    }
    return renamedAny;
}

void EditorShell::renameSceneByID(const QString& sceneID)
{
    const int idx = sceneDoc_.sceneIndexForScene(sceneID);
    if (idx < 0) {
        return;
    }
    const SceneSegment seg = sceneDoc_.segments().at(idx);   // copy (list mutates)

    bool ok = false;
    const QString newTitle = QInputDialog::getText(
        this, tr("Rename Scene"), tr("Scene title:"),
        QLineEdit::Normal, seg.title, &ok);
    if (!ok) {
        refocusEditor();
        return;   // cancelled
    }

    // Write the sidecar title (blank/whitespace clears the custom title). The scene has
    // no in-document heading, so nothing in the viewport text changes.
    const QVariantMap r =
        bridge_->renameScene(projectPath_, seg.metadataPath, newTitle.trimmed());
    if (r.isEmpty()) {
        refocusEditor();
        return;   // bridge already surfaced errorOccurred
    }

    // Re-derive the navigator label the same way a reload would — from
    // scrivi_open_project (custom title, else the backend's first-prose-line / "Scene N"
    // fallback). This keeps the live label identical to the reloaded one, including the
    // blank-clears-to-fallback case, without duplicating the fallback logic in the UI.
    applyDerivedLabels();
    rebuildNavigator();
    // Keep the renamed scene highlighted + active tracking intact.
    selectNavigatorScene(sceneID);
    refocusEditor();
}

void EditorShell::renameChapterByID(const QString& chapterID)
{
    const int first = sceneDoc_.firstSegmentOfChapter(chapterID);
    if (first < 0) {
        return;
    }
    const SceneSegment seg = sceneDoc_.segments().at(first);   // copy (list mutates)
    if (seg.chapterMetadataPath.isEmpty()) {
        // Should not happen once open_project's chapterMetadataPath is captured; guard
        // so a rename can't silently target the wrong path.
        errorLabel_->setText(tr("This chapter can't be renamed (missing metadata path)."));
        errorLabel_->show();
        return;
    }

    bool ok = false;
    const QString newTitle = QInputDialog::getText(
        this, tr("Rename Chapter"), tr("Chapter title:"),
        QLineEdit::Normal, seg.chapterTitle, &ok);
    if (!ok) {
        refocusEditor();
        return;   // cancelled
    }

    const QVariantMap r = bridge_->renameChapter(
        projectPath_, seg.chapterMetadataPath, newTitle.trimmed());
    if (r.isEmpty()) {
        refocusEditor();
        return;
    }

    // Re-derive labels (custom title, else "Chapter N") from a fresh open_project, apply
    // scene labels + the chapter's derived title (which also rewrites the live heading
    // in place via setChapterTitle → reflowBoundaryAt), then rebuild the navigator.
    applyDerivedLabels();
    rebuildNavigator();
    refocusEditor();
}

void EditorShell::onContentsChange(int position, int charsRemoved, int charsAdded)
{
    // Ignore the programmatic assembly in load(); only user edits are dirty.
    if (loading_) {
        return;
    }
    if (charsRemoved == 0 && charsAdded == 0) {
        return;
    }

    // Reconcile the offset map: the owning scene's body grows/shrinks by the delta
    // and later scenes shift. ManuscriptEditor has already ensured the edit stayed
    // inside one body, so this resolves to a real scene.
    const int idx = sceneDoc_.applyContentsChange(position, charsRemoved, charsAdded);
    if (idx < 0) {
        return;
    }
    dirtyScenes_.insert(sceneDoc_.segments().at(idx).sceneID);
    saveTimer_->start();   // (re)arm the idle-save debounce (T-0239)
}

void EditorShell::onCursorMoved()
{
    if (loading_) {
        return;
    }
    // The caret owns *where you type*; scroll owns "which scene is active". But a
    // caret jump across scenes (e.g. a click or arrow into another scene) should
    // still flush the departing scene — route it through the single active-scene
    // writer, which promotes + saves only when the scene actually changes.
    promoteActiveScene(sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position()));
}

void EditorShell::onScrolled()
{
    if (loading_ || programmaticViewportChange_) {
        return;   // ignore our own programmatic scrolls (navigator click, caret move)
    }
    // The visible scene becomes active (T-0243). promoteActiveScene saves the
    // departing dirty scene and updates the tracker only on a real boundary crossing.
    promoteActiveScene(visibleSceneIndex());
}

int EditorShell::visibleSceneIndex() const
{
    // Map the vertical MIDDLE of the viewport to a document position, then to its
    // scene (Apple's midpoint-of-viewport rule). cursorForPosition takes viewport
    // coordinates; the x is arbitrary (offsets are line-based).
    const QPoint mid(viewport_->viewport()->width() / 2,
                     viewport_->viewport()->height() / 2);
    const int pos = viewport_->cursorForPosition(mid).position();
    return sceneDoc_.sceneIndexForCaret(pos);
}

void EditorShell::promoteActiveScene(int newSeg)
{
    if (newSeg < 0 || newSeg == activeSegment_) {
        return;   // undeterminable, or still in the same scene — nothing to do
    }
    // The active scene changed. Flush everything dirty now so the departing scene is
    // on disk before we move on (scene-switch leg of the T-0239 cadence). Single
    // writer of activeSegment_ — both the caret and scroll hooks land here.
    if (!dirtyScenes_.isEmpty()) {
        saveDirtyScenes();
    }
    activeSegment_ = newSeg;

    // Navigator highlight follows the active scene (T-0244) — highlight only, so it
    // doesn't re-scroll the viewport or move the caret (no feedback loop).
    selectNavigatorScene(sceneDoc_.segments().at(newSeg).sceneID);
}

void EditorShell::selectNavigatorScene(const QString& sceneID)
{
    // Highlight-only: find the matching child row and select it. No scroll, no caret.
    for (int r = 0; r < navModel_->rowCount(); ++r) {
        QStandardItem* chapter = navModel_->item(r);
        for (int c = 0; c < chapter->rowCount(); ++c) {
            QStandardItem* scene = chapter->child(c);
            if (scene->data(kSceneIDRole).toString() == sceneID) {
                const QModelIndex idx = scene->index();
                navigator_->selectionModel()->setCurrentIndex(
                    idx, QItemSelectionModel::ClearAndSelect);
                return;
            }
        }
    }
}

bool EditorShell::saveScene(int segmentIndex)
{
    if (segmentIndex < 0 || segmentIndex >= sceneDoc_.segments().size()) {
        return false;
    }
    const SceneSegment& seg = sceneDoc_.segments().at(segmentIndex);

    // Surface state (T-0247): persist the scene-local caret + scroll *only* for the
    // scene the caret is actually in — ScriviCore restores whichever scene it deems
    // active on reopen, and a non-caret scene's offsets are meaningless. For the rest,
    // save 0/0/0.0 (body-only) so a background flush never clobbers a real cursor with
    // a stale one. The caret's global position maps to scene-local by subtracting the
    // body start; the scroll is the whole-document scrollbar fraction (matches the
    // single continuous viewport).
    long long selectionAnchor = 0;
    long long selectionFocus  = 0;
    double     scroll         = 0.0;
    const QTextCursor caret = viewport_->textCursor();
    if (sceneDoc_.sceneIndexForCaret(caret.position()) == segmentIndex) {
        selectionAnchor = qMax(0, caret.anchor()   - seg.bodyStart);
        selectionFocus  = qMax(0, caret.position() - seg.bodyStart);
        if (QScrollBar* vsb = viewport_->verticalScrollBar()) {
            const int range = vsb->maximum() - vsb->minimum();
            scroll = range > 0
                         ? double(vsb->value() - vsb->minimum()) / double(range)
                         : 0.0;
        }
    }

    const QVariantMap r = bridge_->saveScene(
        projectID_, projectPath_, appSupportRoot_,
        seg.sceneID, seg.metadataPath, seg.contentPath,
        sceneDoc_.bodyText(segmentIndex),
        selectionAnchor, selectionFocus, scroll);

    // Whatever the outcome, drop the scene from the dirty set: on success it's
    // clean; on failure the bridge already surfaced errorOccurred and re-queuing it
    // would just fail again every debounce. (A retry policy is out of scope here.)
    dirtyScenes_.remove(seg.sceneID);

    return !r.isEmpty() && r.value(QStringLiteral("saved")).toBool();
}

void EditorShell::saveDirtyScenes()
{
    if (dirtyScenes_.isEmpty()) {
        return;
    }
    saveTimer_->stop();   // an explicit flush supersedes a pending debounce

    // Snapshot the dirty sceneIDs, then save each by its current segment index (the
    // offset map is authoritative for the body text). saveScene() drains the set.
    const QSet<QString> pending = dirtyScenes_;
    for (int i = 0; i < sceneDoc_.segments().size(); ++i) {
        if (pending.contains(sceneDoc_.segments().at(i).sceneID)) {
            saveScene(i);
        }
    }
}

void EditorShell::rebuildNavigator()
{
    navModel_->clear();
    QStandardItem* chapterItem = nullptr;
    QString lastChapterID;
    for (int i = 0; i < sceneDoc_.segments().size(); ++i) {
        const SceneSegment& seg = sceneDoc_.segments().at(i);
        if (chapterItem == nullptr || seg.chapterID != lastChapterID) {
            // Derived heading (custom title, else the ordinal "Chapter N") — the same
            // authority the in-document heading uses, so the navigator + heading agree.
            chapterItem = new QStandardItem(sceneDoc_.chapterHeadingText(i));
            chapterItem->setEditable(false);
            // Chapter rows MUST be selectable for the chapter drag to start (SP-073 /
            // I-0082): QAbstractItemView enters DraggingState only when the pressed row
            // is among the SELECTED draggable indexes — a non-selectable row can never
            // be selected, so a chapter-heading drag silently fell into rubber-band
            // selection (selecting the nearest scene) instead of dragging. Clicking a
            // chapter row is harmless (onNavigatorActivated ignores rows without a
            // sceneID).
            chapterItem->setData(seg.chapterID, kChapterIDRole);   // for delete (T-0251)
            // Chapter rows drag as containers (SP-073, AC5) and accept drops so a
            // scene can be dropped on a heading to become that chapter's first scene
            // (T-0260). NavigatorTree runs the chapter drag manually (the row is
            // non-selectable) and resolves chapter drops to chapter boundaries only.
            chapterItem->setFlags(chapterItem->flags()
                                  | Qt::ItemIsDragEnabled
                                  | Qt::ItemIsDropEnabled);
            navModel_->appendRow(chapterItem);
            lastChapterID = seg.chapterID;
        }
        const QString sceneText =
            seg.title.isEmpty() ? tr("Untitled Scene") : seg.title;
        auto* sceneItem = new QStandardItem(sceneText);
        sceneItem->setEditable(false);
        sceneItem->setData(seg.sceneID, kSceneIDRole);
        // Scene rows are draggable for reorder (T-0260); nothing accepts drops (the
        // navigator handles them itself).
        sceneItem->setFlags((sceneItem->flags() | Qt::ItemIsDragEnabled)
                            & ~Qt::ItemIsDropEnabled);
        chapterItem->appendRow(sceneItem);
    }
    navigator_->expandAll();
}

void EditorShell::moveCaretToSegment(int index)
{
    if (index < 0 || index >= sceneDoc_.segments().size()) {
        return;
    }
    const SceneSegment& seg = sceneDoc_.segments().at(index);
    // This is a deliberate caret placement (create-scene/chapter, initial load); the
    // resulting scroll must not make onScrolled re-promote off a mid-move midpoint.
    // Guard it and set the active scene explicitly.
    programmaticViewportChange_ = true;
    QTextCursor cursor(sceneDoc_.document());
    cursor.setPosition(seg.bodyStart);
    viewport_->setTextCursor(cursor);
    viewport_->centerCursor();
    programmaticViewportChange_ = false;
    activeSegment_ = index;
}

void EditorShell::onCreateSceneRequested()
{
    // Which scene are we in? The caret's owning segment (fall back to the tracked
    // active segment). A create needs a valid host scene + chapter.
    const int caretPos = viewport_->textCursor().position();
    int idx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (idx < 0) {
        idx = activeSegment_;
    }
    if (idx < 0 || idx >= sceneDoc_.segments().size()) {
        return;
    }
    const SceneSegment host = sceneDoc_.segments().at(idx);   // copy (list mutates)

    // macOS ⌘↩ parity (ManuscriptTextView.swift:644-679): SPLIT the current scene at the
    // caret. Mid-body → the head stays in the current scene and the tail moves to the new
    // scene (caret follows the tail). At end-of-body → append an empty scene (nothing to
    // move). Previously this ALWAYS appended empty regardless of caret position — so a
    // mid-scene ⌃↩ left the text in place and dropped an empty scene below (the SP-076 VNC
    // "splits at the bottom, not the middle" report).
    const int bodyEnd = host.bodyStart + host.bodyLength;
    const int splitOffset = qBound(0, caretPos - host.bodyStart, host.bodyLength);
    const bool isAtEnd = (caretPos >= bodyEnd);
    const QString fullBody = sceneDoc_.bodyText(idx);
    const QString headText = fullBody.left(splitOffset);
    const QString tailText = fullBody.mid(splitOffset);

    // Save the current scene first so the create builds on persisted state (mirrors
    // Apple's ⌘↩: save-then-create).
    saveDirtyScenes();

    // Create the (initially empty) scene after the host, in the host's chapter.
    const QVariantMap result = bridge_->createScene(
        projectPath_, appSupportRoot_, projectID_, host.chapterID, host.sceneID);
    const QString newSceneID = result.value(QStringLiteral("sceneID")).toString();
    if (newSceneID.isEmpty()) {
        return;   // bridge already surfaced errorOccurred
    }
    const QString newMeta    = result.value(QStringLiteral("metadataPath")).toString();
    const QString newContent = result.value(QStringLiteral("contentPath")).toString();

    if (!isAtEnd) {
        // Mid-scene split: persist head → current scene, tail → new scene, then reload from
        // disk so the document/map reflect the moved text (disk is authoritative; a split is
        // a rare structural op, mirroring the chapter-split path).
        const QVariantMap headR =
            bridge_->saveScene(projectID_, projectPath_, appSupportRoot_,
                               host.sceneID, host.metadataPath, host.contentPath,
                               headText, 0, 0, 0.0);
        const QVariantMap tailR =
            bridge_->saveScene(projectID_, projectPath_, appSupportRoot_,
                               newSceneID, newMeta, newContent,
                               tailText, 0, 0, 0.0);
        if (headR.isEmpty() || tailR.isEmpty()) {
            // A save failed (bridge surfaced errorOccurred). Reload to whatever persisted so
            // the UI matches disk, then bail.
            load(projectPath_, appSupportRoot_,
                 findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
                     ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
                     : QString());
            return;
        }
        load(projectPath_, appSupportRoot_,
             findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
                 ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
                 : QString());
        // Caret to the start of the new (tail) scene — where the split landed.
        const int newSeg = sceneDoc_.sceneIndexForScene(newSceneID);
        if (newSeg >= 0) {
            activeSegment_ = -1;
            moveCaretToSegment(newSeg);
            selectNavigatorScene(newSceneID);
        }
        return;
    }

    // End-of-scene: splice the new EMPTY scene into the live document + map (guarded so the
    // programmatic boundary insert doesn't churn dirty flags), refresh the navigator, and
    // drop the caret into the new body — the original append behaviour.
    loading_ = true;
    const int newIdx = sceneDoc_.insertSceneAfter(
        idx, newSceneID,
        result.value(QStringLiteral("chapterID")).toString(),
        /*title=*/QString(),          // new scenes are untitled until named (EP-023)
        host.chapterTitle,            // same chapter → same heading label
        /*slug=*/QString(),
        newMeta,
        newContent,
        host.chapterMetadataPath,     // same chapter → same chapter metadata path
        /*newChapter=*/false);
    loading_ = false;

    if (newIdx < 0) {
        return;
    }
    rebuildNavigator();
    moveCaretToSegment(newIdx);
}

void EditorShell::onCreateChapterRequested()
{
    if (sceneDoc_.segments().isEmpty()) {
        return;
    }

    // I-0064: Ctrl+Shift+Return SPLITS the current chapter at the caret (macOS parity
    // with ⌘⇧↩), disk-correct. Caret in scene S of chapter C:
    //   1. createChapter(afterChapterID = C) → the new chapter K is born IN PLACE right
    //      after C, with a blank first scene K0. No reorder/rename step (EP-027 P6): K's
    //      folder gets its final order-key name at creation, so K0's returned paths stay
    //      valid — this is what fixes the lost-tail bug (a post-create rename used to
    //      invalidate them).
    //   2. mid-scene → save head into S, tail into K0.
    //      end-of-scene → K0 stays blank; the caret lands in it (a genuinely-new chapter).
    //   3. Every scene that FOLLOWED S within C moves into K, in order (after K0). If S was
    //      at the end and had followers, the caret follows the first of them.

    // Resolve the caret's scene S and the split offset within its body.
    const int caretPos = viewport_->textCursor().position();
    int segS = sceneDoc_.sceneIndexForCaret(caretPos);
    if (segS < 0) {
        segS = activeSegment_;
    }
    if (segS < 0 || segS >= sceneDoc_.segments().size()) {
        return;
    }
    const SceneSegment sceneS = sceneDoc_.segments().at(segS);   // copy (list mutates)
    const QString chapterC = sceneS.chapterID;

    const int bodyEnd = sceneS.bodyStart + sceneS.bodyLength;
    const int splitOffset = qBound(0, caretPos - sceneS.bodyStart, sceneS.bodyLength);
    const bool isAtEnd = (caretPos >= bodyEnd);
    const QString fullBody = sceneDoc_.bodyText(segS);
    const QString headText = fullBody.left(splitOffset);
    const QString tailText = fullBody.mid(splitOffset);

    // The scenes that follow S within C, in order (they move into the new chapter).
    QStringList followers;
    for (int i = segS + 1; i < sceneDoc_.segments().size(); ++i) {
        const SceneSegment& s = sceneDoc_.segments().at(i);
        if (s.chapterID != chapterC) {
            break;   // left chapter C — the rest belong to later chapters
        }
        followers.append(s.sceneID);
    }

    // No confirmation dialog: Ctrl+Shift+Return IS the approval. Chapter-splitting is a
    // frequent, flow-critical move during "seat of the pants" drafting, and a modal prompt
    // on every break breaks that flow. (The split is also non-destructive — it only creates
    // a chapter and moves scenes; nothing is deleted — so a confirm buys little.)

    // Save the current scene first (mirrors Apple's ⌘⇧↩: save-then-create), so the
    // head/tail split below writes over persisted state.
    saveDirtyScenes();

    // 1. Create the new chapter K in place, right after C (born with its final folder
    //    name — no reorder). K0 is its blank first scene; its returned paths stay valid.
    const QVariantMap result =
        bridge_->createChapter(projectPath_, appSupportRoot_, projectID_, chapterC);
    const QString firstSceneID =
        result.value(QStringLiteral("firstSceneID")).toString();
    const QString newChapterID =
        result.value(QStringLiteral("chapterID")).toString();
    const QString k0MetadataPath =
        result.value(QStringLiteral("firstSceneMetadataPath")).toString();
    const QString k0ContentPath =
        result.value(QStringLiteral("firstSceneContentPath")).toString();
    if (firstSceneID.isEmpty() || newChapterID.isEmpty()) {
        // The chapter wasn't created (the bridge emitted errorOccurred). Surface it rather
        // than silently doing nothing, and abort before we mutate anything else.
        QMessageBox::warning(this, tr("Split Failed"),
            tr("The new chapter could not be created; the manuscript was left unchanged."));
        return;
    }

    // The scene that will carry the caret afterward: mid-scene → K0 (holds the tail);
    // end-of-scene with followers → the first follower; end-of-scene without followers
    // → K0 (the new empty chapter).
    QString caretSceneID = firstSceneID;

    // A split step failed → stop and report rather than silently continuing into a
    // half-applied, corrupt on-disk state (the class of failure behind I-0074).
    auto failSplit = [&](const QString& step) {
        qWarning().noquote() << "Chapter split aborted at step:" << step;
        QMessageBox::warning(this, tr("Split Failed"),
            tr("The chapter split could not be completed (%1). The manuscript may be in a "
               "partially-changed state; reopen the project to let it self-repair.").arg(step));
    };

    // 2. Mid-scene split: head stays in S, tail becomes K's first scene (K0). (End-of-scene
    //    leaves K0 blank for the caret; no save needed.)
    if (!isAtEnd) {
        const SceneSegment& sHead = sceneDoc_.segments().at(segS);
        const QVariantMap headR =
            bridge_->saveScene(projectID_, projectPath_, appSupportRoot_,
                               sceneS.sceneID, sHead.metadataPath, sHead.contentPath,
                               headText, 0, 0, 0.0);
        if (headR.isEmpty()) { failSplit(tr("saving the current scene")); return; }
        const QVariantMap tailR =
            bridge_->saveScene(projectID_, projectPath_, appSupportRoot_,
                               firstSceneID, k0MetadataPath, k0ContentPath,
                               tailText, 0, 0, 0.0);
        if (tailR.isEmpty()) { failSplit(tr("writing the new chapter's first scene")); return; }
    }

    // 3. Followers move into K after K0, in order (runs in every case — mid-scene AND
    //    end-of-scene). reorderScene resolves chapters/scenes BY ID, so it is immune to any
    //    folder naming. K0 is NEVER deleted — at end-of-scene with no followers it is the
    //    blank scene the writer types into.
    QString afterID = firstSceneID;
    for (const QString& f : followers) {
        const QVariantMap rr =
            bridge_->reorderScene(projectPath_, f, chapterC, newChapterID, afterID);
        if (rr.isEmpty() || !rr.value(QStringLiteral("reordered")).toBool()) {
            failSplit(tr("moving a following scene into the new chapter"));
            // Still reload so the UI reflects whatever DID persist, then bail.
            const QString t = findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
                ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text() : QString();
            load(projectPath_, appSupportRoot_, t);
            return;
        }
        afterID = f;
    }
    if (isAtEnd && !followers.isEmpty()) {
        caretSceneID = followers.first();
    }

    // 4. Re-read authoritative disk state (the split touched multiple scenes + chapters)
    //    so sceneDoc_ matches the new manuscript order. A full reload keeps disk as the
    //    source of truth; the split is a rare, heavy structural op.
    const QString title =
        findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
            ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
            : QString();
    load(projectPath_, appSupportRoot_, title);

    // 5. Renumber created ("Chapter N") chapters whose ordinal shifted (I-0063), NOW that
    //    sceneDoc_ reflects the post-split order. If any sidecar changed, re-derive the
    //    live labels/headings so the navigator matches disk without another full reload.
    if (renumberCreatedChapters()) {
        applyDerivedLabels();
        rebuildNavigator();
    }

    const int caretSeg = sceneDoc_.sceneIndexForScene(caretSceneID);
    if (caretSeg >= 0) {
        activeSegment_ = -1;
        moveCaretToSegment(caretSeg);
        selectNavigatorScene(caretSceneID);
    }
    viewport_->setFocus();
}

void EditorShell::onMergeSceneRequested()
{
    // Resolve the caret's scene. A merge needs a valid host with a predecessor.
    const int caretPos = viewport_->textCursor().position();
    int segIdx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (segIdx < 0) {
        segIdx = activeSegment_;
    }
    if (segIdx <= 0 || segIdx >= sceneDoc_.segments().size()) {
        return;   // unknown, or the very first scene (nothing before it) — no-op
    }

    const SceneSegment current = sceneDoc_.segments().at(segIdx);        // copy
    const SceneSegment predecessor = sceneDoc_.segments().at(segIdx - 1);

    // Only fire at the VERY START of the scene's body (macOS parity: ⌘⌫ at position 0).
    if (caretPos != current.bodyStart) {
        return;
    }
    // Only a SAME-CHAPTER scene merge here; a chapter's first scene is the chapter-merge
    // gesture (Ctrl+Shift+Backspace), not this one.
    if (current.chapterID != predecessor.chapterID) {
        return;
    }

    // Flush pending edits first — the endpoint joins the ON-DISK bodies, so both the
    // current scene and the predecessor must be persisted before the merge.
    saveDirtyScenes();

    // Atomic same-chapter merge: current joins into predecessor (survivor keeps its
    // files); current's files are removed. Replaces any saveScene(join)+deleteScene
    // composition (EP-028 / SP-076).
    const QVariantMap r = bridge_->mergeScene(projectPath_, current.sceneID);
    if (r.isEmpty()) {
        return;   // bridge already surfaced errorOccurred; leave the UI untouched
    }

    // The merge rewrote disk (predecessor body grew, current scene's files gone). Disk
    // is authoritative, so reload the document/map/navigator from it rather than
    // hand-splicing — a merge is a rare structural op, so the reload cost is fine and it
    // is provably in sync with disk (mirrors the reorder handlers' reload-on-drift path).
    load(projectPath_, appSupportRoot_,
         findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
             ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
             : QString());

    // Re-anchor on the survivor (its sceneID persists across the merge) and drop the caret
    // at the JOIN SEAM — where the absorbed scene's text now begins inside the survivor
    // (macOS parity: ⌘⌫ lands the caret at predecessor.length). The seam offset is the
    // predecessor's ORIGINAL body length, captured before the merge (bodyLength), measured
    // from the survivor's post-reload body start. If either side was empty the blank-line
    // join is elided, but the seam still sits at the predecessor's original length, so this
    // holds. Clamp to the survivor's new body just in case.
    const int survivorSeg = sceneDoc_.sceneIndexForScene(predecessor.sceneID);
    if (survivorSeg >= 0) {
        const SceneSegment& survivor = sceneDoc_.segments().at(survivorSeg);
        const int seam = qBound(survivor.bodyStart,
                                survivor.bodyStart + predecessor.bodyLength,
                                survivor.bodyStart + survivor.bodyLength);
        // Place the caret at the seam WITHOUT re-centering the viewport. The merge happens
        // at the boundary the writer was already looking at, so the seam is already on
        // screen — do NOT call moveCaretToSegment (it centerCursor()s, which yanks the
        // scroll and reads as a distracting "jump to the previous scene"). Just set the
        // caret and let ensureCursorVisible scroll only if it actually fell off-screen.
        activeSegment_ = survivorSeg;   // register the active scene without a scroll
        programmaticViewportChange_ = true;
        QTextCursor cursor(sceneDoc_.document());
        cursor.setPosition(seam);
        viewport_->setTextCursor(cursor);
        viewport_->ensureCursorVisible();
        programmaticViewportChange_ = false;
        selectNavigatorScene(predecessor.sceneID);
    }
    viewport_->setFocus();
}

void EditorShell::onMergeChapterRequested()
{
    // Resolve the caret's scene. A chapter merge needs a chapter with a predecessor.
    const int caretPos = viewport_->textCursor().position();
    int segIdx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (segIdx < 0) {
        segIdx = activeSegment_;
    }
    if (segIdx <= 0 || segIdx >= sceneDoc_.segments().size()) {
        return;   // unknown, or the first scene of the manuscript — no-op
    }

    const SceneSegment current = sceneDoc_.segments().at(segIdx);        // copy
    const SceneSegment predecessor = sceneDoc_.segments().at(segIdx - 1);

    // Only fire at the VERY START of the scene's body (macOS parity: ⇧⌘⌫ at position 0).
    if (caretPos != current.bodyStart) {
        return;
    }
    // Must be a chapter boundary (the predecessor is in a different chapter)...
    if (current.chapterID == predecessor.chapterID) {
        return;
    }
    // ...and the caret must be at the chapter's FIRST scene (segIdx is the first segment
    // whose chapterID matches — guaranteed here because the predecessor differs, but keep
    // the explicit check to mirror macOS and guard against map anomalies).
    if (sceneDoc_.firstSegmentOfChapter(current.chapterID) != segIdx) {
        return;
    }

    // Flush pending edits before the relocation rewrites the chapter's files.
    saveDirtyScenes();

    // Atomic whole-chapter merge: RELOCATE every scene of the current chapter into the
    // predecessor's folder, then remove the emptied chapter (the I-0083 fix — no scene
    // loss on reopen). Not composed from deleteChapter.
    const QVariantMap r = bridge_->mergeChapter(projectPath_, current.chapterID);
    if (r.isEmpty()) {
        return;   // bridge already surfaced errorOccurred; leave the UI untouched
    }

    // The relocation minted new order-key filenames for every moved scene and removed the
    // old chapter folder — disk is authoritative and every captured path is stale (the
    // I-0081 class). Reload from disk so the document/map/navigator match exactly.
    load(projectPath_, appSupportRoot_,
         findChild<QLabel*>(QStringLiteral("editorTitle")) != nullptr
             ? findChild<QLabel*>(QStringLiteral("editorTitle"))->text()
             : QString());

    // Re-anchor on the merged chapter's (former) first scene — its sceneID survives the
    // relocation — now sitting inside the predecessor chapter.
    const int caretSeg = sceneDoc_.sceneIndexForScene(current.sceneID);
    if (caretSeg >= 0) {
        activeSegment_ = -1;
        moveCaretToSegment(caretSeg);
        selectNavigatorScene(current.sceneID);
    }
    viewport_->setFocus();
}

// --- SP-077 menu-bar triggers (T-0310/T-0311) --------------------------------
//
// Thin forwarders so the ScriviWindow menu bar drives the same operations as the
// keyboard shortcuts. The Scene/Chapter triggers give the caret focus first so the
// handlers (which resolve the caret's scene) see the writing surface's real caret,
// not a stale one — a menu click moves focus to the menu, so without this the caret
// position could be ambiguous. The operations themselves are unchanged.
void EditorShell::splitScene()
{
    viewport_->setFocus();
    onCreateSceneRequested();
}

void EditorShell::mergeScene()
{
    viewport_->setFocus();
    onMergeSceneRequested();
}

void EditorShell::splitChapter()
{
    viewport_->setFocus();
    onCreateChapterRequested();
}

void EditorShell::mergeChapter()
{
    viewport_->setFocus();
    onMergeChapterRequested();
}

void EditorShell::cutSelection()   { viewport_->cut(); }
void EditorShell::copySelection()  { viewport_->copy(); }
void EditorShell::pasteClipboard() { viewport_->paste(); }

// --- EP-024 Scene Inspector visibility (SP-078, T-0319) -------------------

void EditorShell::setInspectorVisible(bool visible)
{
    if (inspector_ == nullptr) {
        return;
    }
    // setVisible() on the pane collapses/restores it inside the splitter; the
    // viewport (stretch=1) reclaims the freed width, so there is no dead space
    // when hidden. Session-scoped: no on-disk persistence (Apple parity).
    inspector_->setVisible(visible);
}

bool EditorShell::isInspectorVisible() const
{
    return inspector_ != nullptr && inspector_->isVisible();
}
