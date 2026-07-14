#include "EditorShell.hpp"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
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

    // Apply the initial active scene: select it in the navigator and scroll to it.
    if (!activeSceneID.isEmpty()) {
        scrollToScene(activeSceneID);
    }
    // Seed the active-segment tracker from the caret so the first real cursor move
    // only saves when the scene genuinely changes.
    activeSegment_ = sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position());

    return true;
}

void EditorShell::onNavigatorActivated(const QModelIndex& index)
{
    const QString sceneID = index.data(kSceneIDRole).toString();
    if (sceneID.isEmpty()) {
        return;   // a chapter row (no sceneID) — ignore
    }
    scrollToScene(sceneID);
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
    const int seg = sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position());
    if (seg == activeSegment_) {
        return;   // still in the same scene — nothing to flush
    }
    // The caret left activeSegment_. Flush everything dirty now (scene-switch leg of
    // the T-0239 cadence), so the departing scene is on disk before we move on.
    if (!dirtyScenes_.isEmpty()) {
        saveDirtyScenes();
    }
    activeSegment_ = seg;
}

void EditorShell::scrollToScene(const QString& sceneID)
{
    const int start = sceneDoc_.bodyStartForScene(sceneID);
    if (start < 0) {
        return;
    }

    // Move the caret to the scene's body start and center it in the viewport.
    // (Scroll-without-moving-caret on navigator click is SP-063's refinement.)
    QTextCursor cursor(sceneDoc_.document());
    cursor.setPosition(start);
    viewport_->setTextCursor(cursor);
    viewport_->centerCursor();

    // Reflect the selection in the navigator (find the matching child row).
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

    // Persist the scene's current body. Selection/scroll restore is SP-064; pass
    // 0/0/0.0 for now (the args exist so the loop is stable when SP-064 fills them).
    const QVariantMap r = bridge_->saveScene(
        projectID_, projectPath_, appSupportRoot_,
        seg.sceneID, seg.metadataPath, seg.contentPath,
        sceneDoc_.bodyText(segmentIndex),
        /*selectionAnchor=*/0, /*selectionFocus=*/0, /*scroll=*/0.0);

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
    for (const SceneSegment& seg : sceneDoc_.segments()) {
        if (chapterItem == nullptr || seg.chapterID != lastChapterID) {
            const QString chapterText = seg.chapterTitle.isEmpty()
                                            ? tr("Chapter")
                                            : seg.chapterTitle;
            chapterItem = new QStandardItem(chapterText);
            chapterItem->setEditable(false);
            chapterItem->setSelectable(false);   // chapters group; scenes select
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
    QTextCursor cursor(sceneDoc_.document());
    cursor.setPosition(seg.bodyStart);
    viewport_->setTextCursor(cursor);
    viewport_->centerCursor();
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

    const int lastIdx = sceneDoc_.segments().size() - 1;
    loading_ = true;
    const int newIdx = sceneDoc_.insertSceneAfter(
        lastIdx, firstSceneID,
        result.value(QStringLiteral("chapterID")).toString(),
        /*title=*/QString(),          // untitled until named (EP-023)
        /*chapterTitle=*/QString(),   // untitled chapter → default "Chapter" heading
        /*slug=*/QString(),
        result.value(QStringLiteral("firstSceneMetadataPath")).toString(),
        result.value(QStringLiteral("firstSceneContentPath")).toString(),
        /*newChapter=*/true);
    loading_ = false;

    if (newIdx < 0) {
        return;
    }
    rebuildNavigator();
    moveCaretToSegment(newIdx);
}
