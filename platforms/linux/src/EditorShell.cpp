#include "EditorShell.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QShowEvent>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextCursor>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariantList>

#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"

namespace {
// Custom data roles on navigator items so activation can recover the sceneID.
constexpr int kSceneIDRole = Qt::UserRole + 1;
// Chapter rows carry their chapterID so the context menu can target a whole-chapter
// delete (T-0251). Scene rows leave this empty; chapter rows leave kSceneIDRole empty.
constexpr int kChapterIDRole = Qt::UserRole + 2;
// Idle-save debounce: how long after the last keystroke an auto-save fires.
// Mirrors Apple's ~1s; the sprint spec is ~1.5s.
constexpr int kSaveDebounceMs = 1500;
} // namespace

EditorShell::EditorShell(QWidget* parent) : QWidget(parent)
{
    bridge_    = new ScriviBridge(this);
    navModel_  = new QStandardItemModel(this);

    // --- Toolbar: Close (back to landing) + title -------------------------
    auto* closeButton = new QPushButton(tr("‹ Close"), this);
    closeButton->setFlat(true);
    // Flush pending edits before leaving the editor (close leg of T-0239), then
    // signal the shell to return to landing.
    connect(closeButton, &QPushButton::clicked, this, [this] {
        saveDirtyScenes();
        emit closeRequested();
    });

    auto* titleLabel = new QLabel(this);
    titleLabel->setObjectName(QStringLiteral("editorTitle"));
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto* toolbar = new QHBoxLayout;
    toolbar->addWidget(closeButton);
    toolbar->addWidget(titleLabel, /*stretch=*/1);
    // Keep the title visually centered against the Close button.
    auto* spacer = new QWidget(this);
    spacer->setFixedWidth(72);
    toolbar->addWidget(spacer);

    // --- Navigator (left) -------------------------------------------------
    navigator_ = new QTreeView(this);
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

    // --- Viewport (right): editable continuous document -------------------
    viewport_ = new ManuscriptEditor(this);
    viewport_->document()->setUndoRedoEnabled(false);    // custom undo owns ⌘Z later
    viewport_->setFrameShape(QFrame::NoFrame);
    viewport_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    // Editable (SP-062, T-0238). Boundary integrity is enforced by ManuscriptEditor;
    // per-scene dirty tracking + map maintenance happen in onContentsChange.

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(navigator_);
    splitter->addWidget(viewport_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({240, 580});

    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet(QStringLiteral("color:#c0392b;"));
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addLayout(toolbar);
    root->addWidget(splitter, /*stretch=*/1);
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
    // NOTE: chapters created via scrivi_create_chapter carry a STORED "Chapter N" title,
    // so they don't auto-renumber when an earlier chapter is deleted. True
    // renumber-on-delete (rewriting later chapters' titles) is tracked as I-0063 — out of
    // SP-066 scope. Untitled chapters (empty stored title) do derive their ordinal from
    // order via chapterHeadingText, so those renumber for free on the navigator rebuild.
    loading_ = false;

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
            chapterItem->setSelectable(false);   // chapters group; scenes select
            chapterItem->setData(seg.chapterID, kChapterIDRole);   // for delete (T-0251)
            navModel_->appendRow(chapterItem);
            lastChapterID = seg.chapterID;
        }
        const QString sceneText =
            seg.title.isEmpty() ? tr("Untitled Scene") : seg.title;
        auto* sceneItem = new QStandardItem(sceneText);
        sceneItem->setEditable(false);
        sceneItem->setData(seg.sceneID, kSceneIDRole);
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
    int idx = sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position());
    if (idx < 0) {
        idx = activeSegment_;
    }
    if (idx < 0 || idx >= sceneDoc_.segments().size()) {
        return;
    }
    const SceneSegment host = sceneDoc_.segments().at(idx);   // copy (list mutates)

    // Save the current scene first so the create builds on persisted state (mirrors
    // Apple's ⌘↩: save-then-create).
    saveDirtyScenes();

    // Create the scene after the host, in the host's chapter.
    const QVariantMap result = bridge_->createScene(
        projectPath_, appSupportRoot_, projectID_, host.chapterID, host.sceneID);
    const QString newSceneID = result.value(QStringLiteral("sceneID")).toString();
    if (newSceneID.isEmpty()) {
        return;   // bridge already surfaced errorOccurred
    }

    // Splice the new empty scene into the live document + map (guarded so the
    // programmatic boundary insert doesn't churn dirty flags), refresh the
    // navigator, and drop the caret into the new body.
    loading_ = true;
    const int newIdx = sceneDoc_.insertSceneAfter(
        idx, newSceneID,
        result.value(QStringLiteral("chapterID")).toString(),
        /*title=*/QString(),          // new scenes are untitled until named (EP-023)
        host.chapterTitle,            // same chapter → same heading label
        /*slug=*/QString(),
        result.value(QStringLiteral("metadataPath")).toString(),
        result.value(QStringLiteral("contentPath")).toString(),
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

    // Save the current scene first (mirrors Apple's ⌘⇧↩: save-then-create).
    saveDirtyScenes();

    // ScriviCore appends the new chapter (with its first scene) to the END of the
    // manuscript, so it splices after the LAST segment — not after the caret.
    const QVariantMap result =
        bridge_->createChapter(projectPath_, appSupportRoot_, projectID_);
    const QString firstSceneID =
        result.value(QStringLiteral("firstSceneID")).toString();
    if (firstSceneID.isEmpty()) {
        return;   // bridge already surfaced errorOccurred
    }

    const QString newChapterID =
        result.value(QStringLiteral("chapterID")).toString();
    const int lastIdx = sceneDoc_.segments().size() - 1;
    loading_ = true;
    // I-0062 (T-0256): the new chapter is untitled, so insertSceneAfter derives its
    // heading as the ordinal "Chapter N" from the segment's position (SceneDocument
    // computes it — the app layer owns chapter numbering, matching macOS). So the live
    // heading shows the correct "Chapter N" immediately — no reload, no title round-trip.
    const int newIdx = sceneDoc_.insertSceneAfter(
        lastIdx, firstSceneID,
        newChapterID,
        /*title=*/QString(),          // untitled until named (EP-023)
        /*chapterTitle=*/QString(),   // untitled → derived ordinal heading
        /*slug=*/QString(),
        result.value(QStringLiteral("firstSceneMetadataPath")).toString(),
        result.value(QStringLiteral("firstSceneContentPath")).toString(),
        result.value(QStringLiteral("chapterMetadataPath")).toString(),
        /*newChapter=*/true);
    loading_ = false;

    if (newIdx < 0) {
        return;
    }
    rebuildNavigator();
    moveCaretToSegment(newIdx);
}
