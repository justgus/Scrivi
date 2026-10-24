#include "EditorShell.hpp"

#include <QElapsedTimer>

#include "AsyncCall.hpp"
#include "DiagnosticLog.hpp"
#include "ThemeColours.hpp"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QCursor>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QShowEvent>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantList>

#include <algorithm>
#include <chrono>
#include <cstdio>

#include "EpochOffsetDialog.hpp"
#include "BufferService.hpp"
#include "HistoryCapture.hpp"
#include "HistoricalEventDialog.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptEscapes.hpp"
#include "ManuscriptPresenter.hpp"
#include "NavigatorTree.hpp"
#include "SceneInspector.hpp"
#include "WorldsDialog.hpp"
#include "ScriviBridge.hpp"
#include "StoryStructures.hpp"
#include "TimeDeltaPicker.hpp"
#include "TimelinePanel.hpp"
#include "ManuscriptTypography.hpp"
#include "ProjectPreferences.hpp"

#include <utility>

// Short alias for the built-in story-structure presets (SP-081).
namespace story = scrivi::linux_app::story;

// ✅ A PERMANENT diagnostic (SP-173 Q2, user 2026-10-10: "keep it"); born measuring I-0285 (dumas took ~3½ min to open).
// Set SCRIVI_LOAD_LOG=1 to print each load phase as ms since launch; SCRIVI_NO_PRESENTER=1 skips the presenter, so one build
// compares with and without it. How to use it: docs/Scrivi_Linux_Rig_Setup_v0_1.md §5.1.
namespace {
const auto kLaunch = std::chrono::steady_clock::now();
void loadLog(const char* phase, int n = -1)
{
    static const bool on = qEnvironmentVariableIsSet("SCRIVI_LOAD_LOG");
    if (!on) { return; }
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - kLaunch).count();
    if (n >= 0) { std::fprintf(stderr, "[SCRIVI-LOAD] %8lld ms  %s (%d)\n", (long long)ms, phase, n); }
    else        { std::fprintf(stderr, "[SCRIVI-LOAD] %8lld ms  %s\n", (long long)ms, phase); }
}
}

namespace {

// ⚠️ T-0499/[I-0195] — the project-open budget now lives in `AsyncCall.hpp`,
// because SP-144 / [I-0232] gave `ScriviBridge::openProjectAsync` a SECOND call
// site for it. ⛔ Two copies of a tuned timeout drift apart silently.
using AsyncCall::kProjectOpenTimeoutMs;
using DiagnosticLog::Span;

// Custom data roles on navigator items (shared with NavigatorTree's drop resolution):
// scene rows carry kSceneIDRole; chapter rows carry kChapterIDRole.
constexpr int kSceneIDRole   = navigator::kSceneIDRole;
constexpr int kChapterIDRole = navigator::kChapterIDRole;
// Idle-save debounce: how long after the last keystroke an auto-save fires.
// Mirrors Apple's ~1s; the sprint spec is ~1.5s.
constexpr int kSaveDebounceMs = 1500;
} // namespace

EditorShell::EditorShell(QWidget* parent, AppEnvironment* env)
    : QWidget(parent), env_(env)
{
    bridge_    = new ScriviBridge(this);
    navModel_  = new QStandardItemModel(this);
    // SP-145 / T-0551 — the session holds the per-project state; the bridge is
    // still OWNED here (Qt parenting) and only borrowed by the session.
    session_.setBridge(bridge_);

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
    // SP-176 (EP-048 S4, Q4): the find bar sits at the top of the manuscript, hidden until Edit ▸ Find.
    findBar_ = new FindBar(viewport_, this);
    auto* manuscript = new QWidget(this);
    auto* manuscriptColumn = new QVBoxLayout(manuscript);
    manuscriptColumn->setContentsMargins(0, 0, 0, 0);
    manuscriptColumn->setSpacing(0);
    manuscriptColumn->addWidget(findBar_);
    manuscriptColumn->addWidget(viewport_, 1);

    // --- Inspector (right): EP-024 Scene Inspector panel ------------------
    // Docks as the THIRD pane of the same splitter (navigator | viewport |
    // inspector), the cleanest match for Apple's "between the manuscript surface
    // and the window edge". SP-125 (EP-035) replaced its stub
    // with the scene's real objects; it is given its project context in load()
    // and follows the active scene through selectNavigatorScene(). Defaults SHOWN
    // (Apple parity — user decision 2026-07-22). View ▸ Show Inspector (T-0320)
    // toggles it.
    //
    // ⚠️ SP-145 / T-0553: visibility is PER-PROJECT STATE on `session_` and the
    // inspector's value PERSISTS through the core ([I-0251]).
    // ⛔ THIS COMMENT PREVIOUSLY READ "Session-scoped visibility (inspectorVisible_)"
    // — ⚠️ and there was NO SUCH MEMBER; visibility was read off the widget. ✅ That
    // phantom member is what [EP-043]'s [R-Q5] was written against, which is why the
    // ruling had to be amended (see the Epic's §Rulings amendment 2026-09-27).
    // ⚠️ A comment naming state that does not exist is the defect class CLAUDE.md's
    // standing rule describes: it read correctly and was false.
    inspector_ = new SceneInspector(this);
    // T-0482: opening an object is requested here and OWNED by the shell — the
    // panel only reports the request. ⚠️ worldID travels with it and must be
    // threaded through every subsequent call (omitting it is how SP-104 blocked
    // object creation outright).
    connect(inspector_, &SceneInspector::openObjectRequested,
            this, &EditorShell::onOpenObjectRequested);

    // ⚠️ I-0193: keep a LOCAL copy of the bound worlds' names, so an error message
    // can name a world without calling the core on the UI thread. The inspector
    // has already read them on the worker thread; this is the same data, free.
    connect(inspector_, &SceneInspector::worldNamesResolved, this,
            [this](const QHash<QString, QString>& names) { worldNames_ = names; });

    splitter_ = new QSplitter(Qt::Horizontal, this);
    splitter_->addWidget(navigator_);
    splitter_->addWidget(manuscript);   // the find bar + the viewport
    splitter_->addWidget(inspector_);
    splitter_->setStretchFactor(0, 0);   // navigator — fixed-ish
    splitter_->setStretchFactor(1, 1);   // viewport — takes the slack
    splitter_->setStretchFactor(2, 0);   // inspector — fixed-ish
    // Keep the inspector from being dragged away entirely; hide it via the menu.
    splitter_->setCollapsible(2, false);
    // ⚠️ 400 = default inspector width (user ruling 2026-08-30, doubled from 200 so
    // the three tab labels fit untruncated). The window grew by the same amount
    // (ScriviWindow.cpp) so the writing surface is NOT squeezed to pay for it —
    // ✅ "if it bothers you that it takes up one third of the window you can make
    // the default window size larger as well. The screens can handle it."
    splitter_->setSizes({240, 580, 400});

    // --- Timeline (bottom): EP-025 Timeline strip -------------------------
    // Docks as a resizable BOTTOM strip below the panels (user decision 2026-07-22,
    // Apple EP-016 layout). An outer VERTICAL splitter holds the 3-pane row above and
    // the timeline below; the divider resizes the strip by its top edge. Non-collapsible
    // via drag — hide it via View ▸ Show Timeline (T-0323). Session-scoped, default shown.
    timeline_ = new TimelinePanel(this);
    connect(timeline_, &TimelinePanel::sceneClicked,
            this, [this](const QString& sceneID) {
                // Same navigate the navigator click uses: caret to the scene start +
                // highlight it in the navigator (moveCaretToSegment also sets active).
                const int idx = sceneDoc_.sceneIndexForScene(sceneID);
                if (idx >= 0) {
                    moveCaretToSegment(idx);
                    viewport_->setFocus();
                }
            });
    // SP-080: dot drag + context-menu "Set Time Delta…" → the picker flow (T-0328).
    connect(timeline_, &TimelinePanel::dotDragged,
            this, &EditorShell::onDotDragged);
    connect(timeline_, &TimelinePanel::setTimeDeltaRequested,
            this, &EditorShell::onSetTimeDeltaRequested);
    // SP-081: story-structure band signals (T-0331/0332).
    connect(timeline_, &TimelinePanel::bandProportionsChanged,
            this, &EditorShell::onBandProportionsChanged);
    connect(timeline_, &TimelinePanel::sceneAssignedToBand,
            this, &EditorShell::onSceneAssignedToBand);
    connect(timeline_, &TimelinePanel::assignBandRequested,
            this, &EditorShell::onAssignBandRequested);
    connect(timeline_, &TimelinePanel::unassignBandRequested,
            this, &EditorShell::onUnassignBandRequested);
    // SP-083 T-0338: persist the timeline zoom/pan per project so it reopens as left.
    connect(timeline_, &TimelinePanel::viewStateChanged,
            this, &EditorShell::onTimelineViewStateChanged);
    // SP-082: historical events + import/export (T-0341/0342/0343).
    connect(timeline_, &TimelinePanel::historicalEventDragged,
            this, &EditorShell::onHistoricalEventDragged);
    connect(timeline_, &TimelinePanel::newHistoricalEventRequested,
            this, &EditorShell::onNewHistoricalEventRequested);
    connect(timeline_, &TimelinePanel::editHistoricalEventRequested,
            this, &EditorShell::onEditHistoricalEventRequested);
    connect(timeline_, &TimelinePanel::deleteHistoricalEventRequested,
            this, &EditorShell::onDeleteHistoricalEventRequested);
    connect(timeline_, &TimelinePanel::importTimelineRequested,
            this, &EditorShell::onImportTimelineRequested);
    connect(timeline_, &TimelinePanel::exportTimelineRequested,
            this, &EditorShell::onExportTimelineRequested);
    connect(timeline_, &TimelinePanel::editImportedOffsetRequested,
            this, &EditorShell::onEditImportedOffsetRequested);
    connect(timeline_, &TimelinePanel::setImportedTimelineVisibleRequested,
            this, &EditorShell::onSetImportedTimelineVisible);
    connect(timeline_, &TimelinePanel::removeImportedTimelineRequested,
            this, &EditorShell::onRemoveImportedTimelineRequested);

    outerSplitter_ = new QSplitter(Qt::Vertical, this);
    outerSplitter_->addWidget(splitter_);
    outerSplitter_->addWidget(timeline_);
    outerSplitter_->setStretchFactor(0, 1);   // panels take the slack
    outerSplitter_->setStretchFactor(1, 0);   // timeline — fixed-ish
    outerSplitter_->setCollapsible(1, false);
    outerSplitter_->setSizes({620, 120});     // ~120px default timeline height

    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet(QStringLiteral("color:#c0392b;"));
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();

    // T-0500 ([I-0195]) — the DETERMINATE open-progress strip.
    //
    // ✅ `files read / files to read`, a REAL fraction (SP-128 §2a): the scene
    // count arrives with `openProject`, BEFORE the per-scene body reads that cost
    // the time. ⚠️ It is a COUNT, never an estimate -- filesystem calls are
    // deterministic and the package layout is OURS, not an unknown user tree.
    // ⚠️ A spinner here would be a REGRESSION against a settled ruling.
    progressRow_   = new QWidget(this);
    progressBar_   = new QProgressBar(progressRow_);
    progressLabel_ = new QLabel(progressRow_);
    progressBar_->setTextVisible(false);
    // ⚠️ Theme-derived, NOT a hardcoded colour. I-0186: `palette(mid)` measured
    // 1.07:1 on the rig's real dark theme, and a stylesheet colour overrides the
    // theme permanently and ignores a runtime light/dark switch.
    ThemeColours::applyTextColour(progressLabel_,
                                  ThemeColours::deemphasised(palette()));
    {
        auto* row = new QHBoxLayout(progressRow_);
        row->setContentsMargins(8, 4, 8, 4);
        row->addWidget(progressLabel_);
        row->addWidget(progressBar_, /*stretch=*/1);
    }
    progressRow_->hide();

    // ⚠️ A local project opens in well under a second. Flashing a bar for 200 ms
    // is noise, not information -- so the strip appears only if the load is still
    // running after this delay. ✅ The writer sees it exactly when it tells them
    // something they did not already know.
    progressDelay_ = new QTimer(this);
    progressDelay_->setSingleShot(true);
    progressDelay_->setInterval(400);
    connect(progressDelay_, &QTimer::timeout, this, [this]() {
        if (progressRow_ != nullptr) { progressRow_->show(); }
    });

    // ⚠️ Qt::QueuedConnection is LOAD-BEARING, not a style choice.
    //
    // ⚠️ `loadProgress` is emitted FROM THE WORKER THREAD (T-0499). Sender and
    // receiver are both `this`, so Qt's AUTO connection resolves to a DIRECT
    // call -- ✅ which would run `onLoadProgress` ON THE WORKER THREAD and touch
    // `progressBar_`/`progressLabel_` from off the UI thread.
    //
    // ⚠️ THAT IS EXACTLY WHAT SHIPPED, and it is why the progress bar never
    // appeared and why opening a project became unreliable (reported 2026-09-11:
    // a recent-projects click only reordered the list; the editor never showed).
    // ⚠️ `AsyncCall.hpp`'s own header states the rule this violated: the callable
    // may touch ONLY the C ABI and its own locals -- never a widget.
    //
    // ✅ QUEUED forces delivery on the receiver's thread, which is the UI thread.
    connect(this, &EditorShell::loadProgress, this, &EditorShell::onLoadProgress,
            Qt::QueuedConnection);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addLayout(toolbar);
    root->addWidget(outerSplitter_, /*stretch=*/1);
    root->addWidget(progressRow_);
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

    // EP-026 S2 (SP-178 / T-0605): Ctrl+Z / Ctrl+Shift+Z, and the paste / cut / multi-scene brackets the history needs
    // (Apple: `beginPasteOrCut` … `flush`, `recordGroupedEdit`).
    connect(viewport_, &ManuscriptEditor::undoRequested, this, &EditorShell::undo);
    connect(viewport_, &ManuscriptEditor::redoRequested, this, &EditorShell::redo);
    connect(viewport_, &ManuscriptEditor::pasteOrCutBegins, this, [this](const QString& kind, const QString& bufferID) {
        if (history_ != nullptr) { history_->beginPasteOrCut(kind, bufferID); }
    });
    connect(viewport_, &ManuscriptEditor::pasteOrCutEnded, this, [this](const QString& kind) {
        if (history_ != nullptr) { history_->flush(kind, kind); }
    });
    connect(viewport_, &ManuscriptEditor::crossSceneCutRequested, this, [this](int keeping, bool thenPaste) {
        cutAcrossScenes(keeping == 1 ? CutKeeping::Clipboard : CutKeeping::Nothing, {},
                        thenPaste ? std::function<void()>([this] { viewport_->pasteClipboard(); }) : nullptr);
    });
    // SP-181 (EP-048 S4b): the find bar's Replace All is the shell's — off the GUI thread (Q3).
    findBar_->setReplaceAllHandler([this](const QVector<MarkdownRange>& matches, const QString& typed,
                                          std::function<void(int)> done) { replaceAll(matches, typed, std::move(done)); });

    // EP-026 S5 (SP-179 / T-0606): the copy-buffer chords (Ctrl+N copy · Ctrl+Shift+N paste · Alt+N cut).
    connect(viewport_, &ManuscriptEditor::bufferCopyRequested, this, &EditorShell::copyIntoBuffer);
    connect(viewport_, &ManuscriptEditor::bufferPasteRequested, this, &EditorShell::pasteFromBuffer);
    connect(viewport_, &ManuscriptEditor::bufferCutRequested, this, &EditorShell::cutIntoBuffer);

    // EP-026 S4 (SP-180 / T-0607): structured copy / paste across scenes. ✅ [SP-191]: the editor builds the Range from the
    // scene list (no core call, no disk read); the paste is one range call off the GUI thread.
    connect(viewport_, &ManuscriptEditor::structuredPasteRequested, this, &EditorShell::pasteRange);

    // Surface bridge errors inline rather than silently failing.
    //
    // ⚠️ The core's message is written for a DEVELOPER and names raw identifiers.
    // A writer who double-clicked a character saw:
    //   "Error 1: world 'world_character_01a000fb-539a-…' is unavailable."
    // ⚠️ A 36-character UUID tells her nothing, and "Error 1" reads as a crash
    // rather than as a drive she can plug back in. (User-reported 2026-08-30.)
    //
    // ✅ The unavailable-world case is REPHRASED — it is common, expected, and
    // recoverable. ⚠️ Everything else is passed through UNCHANGED: inventing
    // friendly text for errors we have not seen would hide real faults, which is
    // worse than an ugly message.
    // ⚠️ QUEUED for the same reason as `loadProgress` above, and it is the SECOND
    // half of the same defect.
    //
    // ⚠️ `ScriviBridge::openProject`/`openScene` EMIT `errorOccurred` on failure
    // (`ScriviBridge.cpp:126`), and T-0499 moved those calls ONTO A WORKER
    // THREAD. Sender and receiver are both UI-thread objects, so AUTO resolves
    // to DIRECT -- ⚠️ this lambda would then call `errorLabel_->setText()` and
    // `->show()` FROM THE WORKER, which is undefined behaviour in Qt and can
    // wedge the open path outright.
    connect(bridge_, &ScriviBridge::errorOccurred, this,
            [this](int code, const QString& message) {
                errorLabel_->setText(writerFacingError(code, message));
                errorLabel_->show();
            }, Qt::QueuedConnection);
}

void EditorShell::load(const QString& projectPath,
                       const QString& appSupportRoot,
                       const QString& title,
                       const QVariantMap& openedProject)
{
    loadLog(openedProject.isEmpty() ? "load: entered (will open the project)" : "load: entered (open handed over)");
    ++loads_;   // EP-053 S2a baseline
    DiagnosticLog::note(QStringLiteral("load: %1").arg(projectPath));   // [SP-185] T-0612
    QElapsedTimer loadClock;
    loadClock.start();
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
        hideLoadProgress();
        emit loadFinished(false);
        return;
    }

    // ⚠️ T-0499 ([I-0195]) — THE READS BELOW RUN ON A WORKER THREAD.
    //
    // ⚠️ WHAT THIS COSTS, and why it had to move: the load is `openProject` plus
    // ONE `openScene` PER SCENE, sequential and blocking. Every one of those ran
    // on the UI thread. On a `cache=none` mount the rig measured project open at
    // ~10x local, ⚠️ and the cost is UNBOUNDED -- a world accumulates objects over
    // a project's life, a project may bind SEVERAL worlds, and project and worlds
    // may BOTH sit on slow network storage.
    //
    // ✅ USER RULING (2026-09-08): waiting is ACCEPTABLE; waiting with a FROZEN,
    // SILENT UI is NOT. So the fix is to get off the thread and REPORT, not to
    // make the read faster.
    //
    // ⚠️ THIS IS NOT [I-0193]'s CASE AND MUST NOT BE TREATED AS ONE. That was the
    // UNREACHABLE volume, which needs a TIMEOUT. This is the REACHABLE-BUT-SLOW
    // volume, which needs PROGRESS -- ⚠️ and a timeout here would ABORT a
    // legitimate slow load, which is a WORSE defect than the freeze it replaced.
    // ✅ Hence the deliberately large budget below.
    // T-0500: arm the delayed reveal. ⚠️ Reset to a clean state first -- a second
    // open must not inherit the last one's fraction.
    if (progressBar_ != nullptr) {
        progressBar_->setRange(0, 0);
        progressBar_->setValue(0);
        progressLabel_->setText(tr("Opening this project…"));
    }
    if (progressRow_ != nullptr) { progressRow_->hide(); }
    if (progressDelay_ != nullptr) { progressDelay_->start(); }

    // ✅ [I-0309]: the worker builds its OWN bridge from this handle — closing the window mid-load freed `bridge_` under it.
    const WorkerBridge::Handle bridgeHandle = WorkerBridge::handleFor(bridge_);
    const QString path   = projectPath;
    const QString appSup = appSupportRoot;

    // ⚠️ SP-144 ([I-0232] AC5) — THE PROJECT IS OPENED ONCE, NOT TWICE.
    //
    // ⚠️ The landing has already called `openProject` to decide whether the
    // project is `ready`; it hands that envelope here rather than letting this
    // load repeat the work. MEASURED: a second open costs ~79% of the first
    // (196 → 155 filesystem calls) and nothing absorbed it — `ProjectIndex`
    // accelerates `openScene`, but `ProjectOpener` never consults it.
    //
    // ✅ This is APPLE'S shape (`ProjectSession.loadAsync` opens once and passes
    // `result.scenes` into the scene loop), not a new mechanism.
    //
    // ⚠️ An EMPTY map means "no envelope was handed over" — a reload after a
    // structural edit — and the worker opens the project itself. ⛔ Both paths
    // must stay behaviourally identical; only the OPENER differs.
    const QVariantMap handedOver = openedProject;

    // EP-026 S2 ([SP-178] Q1): the undo history opens IN THE WORKER on a project's FIRST load — ⛔ never on a structural
    // reload of the project already open here, which keeps its history (a second `scrivi_history_open` mints a new
    // session: a false "previous session" warning on the next undo, and the pending edit dropped).
    const bool openHistory = history_ == nullptr || !history_->isOpen() || history_->projectRootPath() != projectPath;

    // ⚠️ Progress is emitted FROM THE WORKER THREAD, so it is queued to the UI
    // thread rather than called directly. A direct call would touch widgets from
    // the wrong thread.
    // ✅ [I-0309]: through the worker's relay, never `this` — the window may be gone before the worker is.
    connect(bridgeHandle.relay.get(), &ScriviBridgeRelay::progress, this, &EditorShell::loadProgress,
            Qt::QueuedConnection);

    AsyncCall::run<LoadPayload>(
        this,
        [bridgeHandle, path, appSup, handedOver, openHistory]() -> LoadPayload {
            WorkerBridge bridge(bridgeHandle);
            ScriviBridgeRelay* const progress = bridge.relay();
            LoadPayload out;

            // ✅ Use the landing's envelope when it handed one over; open only
            // when it did not (the reload path).
            const QVariantMap opened = handedOver.isEmpty()
                ? bridge->openProject(path, appSup)
                : handedOver;
            loadLog("worker: openProject done");
            if (opened.value(QStringLiteral("mode")).toString()
                != QStringLiteral("ready")) {
                // repairRequired / cannotOpen were already handled by the landing
                // flow; reaching here in a non-ready state is unexpected.
                out.ok = false;
                out.failureMessage =
                    tr("This project could not be opened into the editor.");
                return out;
            }

            out.projectID = opened.value(QStringLiteral("projectID")).toString();
            out.projectTitle = opened.value(QStringLiteral("projectTitle")).toString();
            out.settings = bridge->getProjectSettings(path);   // [SP-182]: read here, off the GUI thread
            const QVariantList scenes = opened.value(QStringLiteral("scenes")).toList();
            const QVariantMap active  = opened.value(QStringLiteral("activeScene")).toMap();
            out.activeSceneID = active.value(QStringLiteral("sceneID")).toString();
            const QString activeMarkdown =
                active.value(QStringLiteral("markdown")).toString();

            // Restored surface state for the active scene (T-0247).
            const QVariantMap restored = opened.value(QStringLiteral("restored")).toMap();
            out.restoredAnchor = restored.value(QStringLiteral("anchor")).toInt();
            out.restoredFocus  = restored.value(QStringLiteral("focus")).toInt();
            out.restoredScroll = restored.value(QStringLiteral("scroll")).toDouble();

            // ✅ T-0500: THE TOTAL IS KNOWN HERE, BEFORE THE EXPENSIVE PART.
            // ⚠️ `openProject` has already returned the scene list, so this is a
            // COUNT, not an estimate -- which is exactly what makes the progress
            // bar DETERMINATE rather than a spinner (SP-128 §2a).
            // SP-173 / I-0285 (AC5): the timeline read is the load's last step, counted on the bar like each scene body.
            // ✅ [SP-178] Q1: opening the undo history is counted too — 1 for the log, 1 per scene validated against it.
            const int sceneCount = scenes.size();
            const int total = sceneCount + 1 + (openHistory ? sceneCount + 1 : 0);
            int done = 0;
            emit progress->progress(done, total, sceneCount);

            out.inputs.reserve(scenes.size());
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
                // The chapter's own sidecar path (needed by rename + I-0063
                // renumber). Every scene entry carries its chapterMetadataPath.
                in.chapterMetadataPath =
                    s.value(QStringLiteral("chapterMetadataPath")).toString();

                if (sceneID == out.activeSceneID) {
                    in.markdown = activeMarkdown;   // already have it — no round-trip
                }
                out.inputs.append(in);
            }

            // ✅ SP-173 / I-0285 (fix 3): the texts in CHUNKS through `scrivi_read_scene_texts`, which reads each chunk IN
            // PARALLEL inside the core. ⛔ One `openSceneForBulkLoad` per scene cost ~32 ms each over the share — one
            // network round trip after another (the project lock serialises our own calls, so the parallelism must be the
            // core's). ⚠️ Progress is emitted per CHUNK, so the fraction still MOVES on a slow mount.
            constexpr int kChunk = 64;
            for (int start = 0; start < out.inputs.size(); start += kChunk) {
                const int end = std::min<int>(start + kChunk, out.inputs.size());
                QStringList ids;
                for (int i = start; i < end; ++i) {
                    const QString& id = out.inputs.at(i).sceneID;
                    if (id != out.activeSceneID) { ids.append(id); }
                }
                const QHash<QString, QString> texts = ids.isEmpty() ? QHash<QString, QString>{} : bridge->readSceneTexts(path, ids);
                for (int i = start; i < end; ++i) {
                    SceneDocument::Input& in = out.inputs[i];
                    if (in.sceneID != out.activeSceneID) {
                        in.markdown = texts.value(in.sceneID);
                    }
                }
                done = end;
                emit progress->progress(done, total, sceneCount);
            }

            loadLog("worker: all scene bodies read", done);
            // EP-026 S2 ([SP-178] Q1): open the undo history and validate every scene's disk text against the head hash
            // its last close persisted (§6.b — a scene edited outside Scrivi gets an externalChange barrier). History is
            // best-effort: a failed open leaves it closed, and the project opens regardless.
            out.historyRequested = openHistory;
            if (openHistory) {
                out.historyOpened = !ScriviBridge::historyOpen(path).isEmpty();
                emit progress->progress(++done, total, sceneCount);
                constexpr int kValidateReport = 64;   // progress per chunk, as the bodies
                for (int i = 0; i < out.inputs.size(); ++i) {
                    if (out.historyOpened) {
                        ScriviBridge::historyValidateScene(path, out.inputs.at(i).sceneID, out.inputs.at(i).markdown);
                    }
                    ++done;
                    if ((i + 1) % kValidateReport == 0 || i + 1 == out.inputs.size()) {
                        emit progress->progress(done, total, sceneCount);
                    }
                }
                loadLog("worker: undo history opened", done);
            }
            // ⚠️ BEFORE the timeline read, not after: opening history is a project WRITE (it bumps the revision), and a
            // timeline read older than it is discarded and re-read on the UI thread ([I-0285] AC4 — measured, smoke).
            // SP-173 / I-0285 (AC4): the timeline's data, read HERE and not on the UI thread; applied there only if the
            // project has not changed since (its revision).
            out.timeline = EditorShell::fetchTimelineData(&*bridge, path);
            emit progress->progress(++done, total, sceneCount);
            loadLog("worker: timeline read");

            out.ok = true;
            return out;
        },
        [this, projectPath, appSupportRoot, loadClock](const LoadPayload& payload) {
            loadLog("main: payload received");
            hideLoadProgress();
            if (!payload.ok) {
                if (!errorLabel_->isVisible()) {
                    errorLabel_->setText(payload.failureMessage);
                    errorLabel_->show();
                }
                emit loadFinished(false);
                return;
            }
            applyLoadedProject(projectPath, appSupportRoot, payload);
            DiagnosticLog::note(QStringLiteral("load: finished in %1 ms (%2 scenes)").arg(loadClock.elapsed()).arg(sceneDoc_.segments().size()));
            emit loadFinished(true);
        },
        [this]() {
            hideLoadProgress();
            // ⚠️ HONEST, and NOT a claim about the data (I-0115). Reaching here
            // means the read exceeded even the generous budget below -- the
            // volume is likely unreachable rather than merely slow.
            errorLabel_->setText(
                tr("This project is taking longer than expected to open — it or "
                   "one of its worlds may be on a disconnected or unreachable "
                   "volume. Nothing has been lost."));
            errorLabel_->show();
            emit loadFinished(false);
        },
        // ⚠️ NOT AsyncCall::kDefaultTimeoutMs. That is 5 s, tuned to separate a
        // healthy call from a DEAD share ([I-0193]). ⚠️ Applying it here would
        // abort exactly the legitimate slow load [I-0195] exists to support.
        // ✅ This budget is a LAST-RESORT backstop for a hung volume, not a
        // latency policy -- progress, not a timeout, is the answer to slowness.
        kProjectOpenTimeoutMs);
}

EditorShell::~EditorShell()
{
    // ✅ [I-0310] (SP-185): the shell OWNS the document its editor shows (`sceneDoc_`), and its members are destroyed BEFORE
    // `QWidget`'s destructor runs. That destructor closes a shown shell; the focus leaving the editor then made
    // `QPlainTextEdit::focusOutEvent` read the freed document (a crash, found tearing down a shown, focused shell). Hiding
    // here moves the focus while everything still exists, so the close below finds nothing left to do.
    hide();
}

void EditorShell::hideLoadProgress()
{
    if (progressDelay_ != nullptr) { progressDelay_->stop(); }
    if (progressRow_ != nullptr)   { progressRow_->hide(); }
}

void EditorShell::onLoadProgress(int done, int total, int scenes)
{
    if (progressBar_ == nullptr) { return; }

    // SP-173 / I-0285 (AC5): `total` counts every scene body PLUS the timeline read. ✅ [SP-178] Q1: on a first load it
    // also counts opening the undo history (1) and validating each scene against it (`scenes`). The BAR counts all of it;
    // the LABEL names the phase, and never claims a scene that does not exist.

    // ⚠️ A scene count of 0 or 1 is not worth a bar: a single-scene project has one
    // read, and there is no meaningful fraction to show.
    if (scenes <= 1) { return; }

    progressBar_->setRange(0, total);
    progressBar_->setValue(done);
    // ✅ The NUMBERS are shown, not just a bar. A writer waiting on a slow mount
    // can see that it is MOVING and roughly how much is left -- which is the
    // difference between waiting and wondering whether it has hung.
    // Phases, in the worker's order: the scene bodies (`scenes`), then — first load only — the history log (1) and each
    // scene's validation (`scenes`), then the timeline (1). Each report arrives AFTER its step, so the label names the NEXT.
    const bool withHistory = total > scenes + 1;
    if (!withHistory || done < scenes) {
        progressLabel_->setText(tr("Opening this project… %1 of %2 scenes")
                                    .arg(std::min(done, scenes)).arg(scenes));
    } else if (done == scenes) {
        progressLabel_->setText(tr("Opening undo history…"));
    } else {
        progressLabel_->setText(tr("Checking undo history… %1 of %2 scenes")
                                    .arg(std::min(done - scenes - 1, scenes)).arg(scenes));
    }
}

void EditorShell::applyLoadedProject(const QString& projectPath,
                                     const QString& appSupportRoot,
                                     const LoadPayload& payload)
{
    const Span span("load: build the manuscript (main thread)", Span::Mode::Always);   // [SP-185] T-0612
    // ⚠️ UI THREAD ONLY. Everything below touches widgets and the document.
    // EP-026 S2 (SP-178): the caret this load places is not the writer's move — not reported to history. ⚠️ A report is a
    // project WRITE (it bumps the revision), and it would make the worker's timeline read stale ([I-0285] AC4, smoke).
    applyingLoad_ = true;
    const QString projectID     = payload.projectID;
    const QString activeSceneID = payload.activeSceneID;
    const int restoredAnchor    = payload.restoredAnchor;
    const int restoredFocus     = payload.restoredFocus;
    const double restoredScroll = payload.restoredScroll;
    const QList<SceneDocument::Input>& inputs = payload.inputs;

    // ⚠️ SP-145 / T-0552: captured BEFORE `projectID_` is overwritten, so the
    // registry can drop the id this shell was previously registered under.
    const QString previousProjectID = projectID_;

    // Stash identity for the save path (T-0239); reset save state for the new
    // project. ⚠️ Set HERE, on the UI thread, before anything reads it -- these
    // members belong to the UI thread and must never be written from the worker.
    // ⚠️ SP-145 / T-0551: these names are now REFERENCES into `session_`, so these
    // four assignments write the session's state. ✅ Deliberately unchanged
    // statements — the extraction moved WHERE the state lives, not what runs.
    projectID_      = projectID;
    projectPath_    = projectPath;
    appSupportRoot_ = appSupportRoot;
    activeSegment_  = -1;
    saveTimer_->stop();

    // ⚠️ SP-145 / T-0552 — REGISTER the session under its projectID ([R-Q2]).
    //
    // ⚠️ Registered HERE, not in load(), because this is the first point where the
    // project's IDENTITY is known: `load()` has only a path, and a path is not an
    // identity. ✅ A failed load never reaches this function, so a failed open
    // never registers — which is what the registry's empty-ID guard also enforces.
    //
    // ⚠️ THE REGISTRY HOLDS AT MOST ONE SESSION IN THIS SPRINT. ✅ Re-loading a
    // DIFFERENT project into this same shell must not leave the previous id behind
    // (this shell has exactly one session, so the old id would be a phantom "open"
    // project and R3 would refuse to reopen it). ⛔ So deregister the old id first.
    //
    // ⚠️ [SP-146] T-0559 — THE REGISTRY NOW LIVES ON `AppEnvironment`, not here.
    // ✅ Same two calls, same order, same meaning; ⛔ only the owner changed.
    // ⚠️ GUARDED: tests construct an `EditorShell` with no app environment.
    if (env_ != nullptr) {
        OpenProjectRegistry& registry = env_->openProjects();
        if (registry.session(previousProjectID) == &session_) {
            registry.deregister(previousProjectID);
        }
        registry.registerSession(&session_);
    }

    // The inspector reads objects through the same bridge, for the same project
    // (SP-125). Given here — after projectPath_ is set and before any scene is
    // promoted — so the panel never queries a stale root.
    if (inspector_ != nullptr) {
        inspector_->setContext(bridge_, projectPath_);

        // ⚠️ SP-145 / T-0553 ([I-0251]) — RESTORE the writer's pane choice for THIS
        // project. ✅ setContext() has just loaded the layout document, so the stored
        // value is available now and not before.
        //
        // ⚠️ THE GUARD IS NOT OPTIONAL: `setInspectorVisible` persists, so restoring
        // without it writes the file on every open with the value just read.
        restoringPaneVisibility_ = true;
        setInspectorVisible(!inspector_->storedInspectorHidden());
        // [I-0255] — the other two panes, same document, same guard.
        setTimelineVisible(!inspector_->storedTimelineHidden());
        setNavigatorVisible(!inspector_->storedNavigatorHidden());
        restoringPaneVisibility_ = false;
    }

    // Assemble the document under the loading_ guard so the programmatic inserts
    // in build() don't churn dirty flags / the offset map through onContentsChange.
    loading_ = true;
    dirtyScenes_.clear();
    // EP-048 (SP-166): detach the presenter while the text and the scene map are rebuilt — it would
    // highlight 1.8 MB against a stale map, then again — and re-attach once both are current.
    if (presenter_ == nullptr) {
        presenter_ = new ManuscriptPresenter(&ScriviBridge::analyzeMarkdown, this);
        presenter_->setSceneDocument(&sceneDoc_);
        viewport_->setPresenter(presenter_);
    }
    presenter_->attach(nullptr);
    loadLog("main: sceneDoc build start");
    sceneDoc_.build(inputs);
    loadLog("main: sceneDoc built");
    viewport_->setDocument(sceneDoc_.document());
    loadLog("main: setDocument done");
    // EP-048 S5a ([SP-182]): the project's settings — kept across a reload of the same project (an open dialog holds them),
    // and the type applied to EVERY new document, so a reload never resets the face.
    if (prefs_ == nullptr) {
        prefs_ = new ProjectPreferences(bridge_, projectPath, payload.projectTitle, payload.settings, this);
        connect(prefs_, &ProjectPreferences::typographyChanged, this, &EditorShell::applyTypography);
        connect(prefs_, &ProjectPreferences::markupHintsChanged, this, [this](bool on) {
            const QTextCursor c = viewport_->textCursor();
            presenter_->setHintsEnabled(on, c.selectionStart(), c.selectionEnd());
        });
        connect(prefs_, &ProjectPreferences::titleChanged, this, [this](const QString& t) {
            if (auto* l = findChild<QLabel*>(QStringLiteral("editorTitle"))) {
                l->setText(t.trimmed().isEmpty() ? tr("Project") : t);
            }
        });
    }
    // The title shown is `project.json`'s (a rename in the dialog is there; the recents' copy may be older).
    if (auto* l = findChild<QLabel*>(QStringLiteral("editorTitle")); l != nullptr && !prefs_->title().trimmed().isEmpty()) {
        l->setText(prefs_->title());
    }
    presenter_->setHintsEnabled(prefs_->markupHints(), 0, 0);   // detached: sets the switch, draws at attach
    applyTypography();
    // setDocument re-enables undo on the freshly-attached document; keep it off.
    viewport_->document()->setUndoRedoEnabled(false);
    // Point the boundary guard at the freshly-built map, and hook edits. The
    // connection is unique so re-loads don't stack duplicate handlers on the (new)
    // document; contentsChange belongs to the document, which setDocument replaced.
    viewport_->setSceneDocument(&sceneDoc_);
    connect(viewport_->document(), &QTextDocument::contentsChange,
            this, &EditorShell::onContentsChange, Qt::UniqueConnection);
    if (!qEnvironmentVariableIsSet("SCRIVI_NO_PRESENTER")) {
        presenter_->attach(sceneDoc_.document());   // highlights the whole manuscript once
    }
    loadLog("main: presenter attached (first highlight)");
    findBar_->attach(&sceneDoc_, presenter_);   // SP-176: the presented text is rebuilt at the next Find

    // Populate the navigator: chapter parents → scene children. ⛔ [I-0285] Still under `loading_`: `rebuildNavigator()` ends
    // in `reloadTimeline()`, which skips while loading — the load builds the timeline ONCE, at its end. (Clearing the flag first
    // built it twice: 27 s each on dumas over the share.)
    rebuildNavigator();

    // EP-026 S2 (SP-178): adopt the history the worker opened (first load only), and record each scene's text as history
    // now sees it — the "text before" of its first edit.
    if (payload.historyRequested) {
        if (history_ != nullptr) {
            history_->close();
            history_->deleteLater();
        }
        history_ = new HistoryCapture(projectPath, this);
        history_->adoptOpened(payload.historyOpened);
    }
    historyText_.clear();
    grouping_ = false;
    groupParts_.clear();
    for (int i = 0; i < sceneDoc_.segments().size(); ++i) {
        historyText_.insert(sceneDoc_.segments().at(i).sceneID, sceneDoc_.bodyText(i));
    }

    loading_ = false;
    loadLog("main: navigator rebuilt");

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

    // EP-025: build the timeline dots from the backend story-time now that the
    // segments + activeSegment are set. ✅ SP-173 / I-0285 (D4): from the worker's read, if the project has not changed since
    // (one cheap revision check, no file touched); otherwise read again — one call, on this thread.
    if (payload.timeline.fetched && payload.timeline.revision == bridge_->projectRevision(projectPath_)) {
        applyTimelineData(payload.timeline);
        loadLog("main: timeline applied from the worker's read — applyLoaded done");
    } else {
        reloadTimeline();
        loadLog("main: timeline RE-READ (project changed since the worker read it) — applyLoaded done");
    }
    // EP-026 S5 (SP-179): this project's copy buffers (a reload of the same project keeps them).
    // ⚠️ AFTER the timeline check above, never before: `scrivi_buffers_list` takes the core's project WRITE guard, which
    // moves `projectRevision` — so reading the slots first made the worker's timeline read look stale and re-read the
    // timeline on the UI thread ([I-0285]'s cost; `open_progress_smoke` caught it).
    if (buffers_ == nullptr || buffers_->projectRootPath() != projectPath) {
        if (buffers_ != nullptr) {
            buffers_->deleteLater();
        }
        buffers_ = new BufferService(projectPath, this);
    }
    {
        // SP-173 (Q1): what the exclusive project lock cost this process so far — waits of 1 ms or more, all threads.
        const ScriviBridge::LockWaitStats w = ScriviBridge::lockWaitStats();
        const QByteArray line = QStringLiteral("lock waits so far: %1 ms total over %2 waits, longest %3 ms")
                                    .arg(w.totalMs).arg(w.count).arg(w.maxMs).toUtf8();
        loadLog(line.constData());
    }
    QTimer::singleShot(0, this, []() { loadLog("main: event loop idle (layout + first paint queued before this)"); });
    applyingLoad_ = false;
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
    // Barrier BEFORE the delete (Apple, I-0066): undo cannot walk back into the deleted scene.
    recordBarrier(QStringLiteral("sceneDelete"), tr("Can't undo past deleting a scene"));

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

    viewport_->removePendingPair();   // SP-175: a structural edit drops the pending pair (Apple: a rebuild does)
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
    // Barrier BEFORE the delete (Apple, I-0066) — Apple's kind for a chapter delete is `sceneDelete` too.
    recordBarrier(QStringLiteral("sceneDelete"), tr("Can't undo past deleting a chapter"));

    const QVariantMap r = bridge_->deleteChapter(projectPath_, chapterID);
    if (r.isEmpty()) {
        return;
    }

    viewport_->removePendingPair();   // SP-175: a structural edit drops the pending pair (Apple: a rebuild does)
    loading_ = true;
    sceneDoc_.removeChapter(chapterID);
    loading_ = false;

    // Fallback active scene: the segment that now occupies the deleted chapter's first
    // slot (the following chapter's first scene), clamped into range.
    rehighlightAfterEdit(0, -1);
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
    viewport_->removePendingPair();   // SP-175: a structural edit drops the pending pair (Apple: a rebuild does)
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
    viewport_->removePendingPair();   // SP-175: a structural edit drops the pending pair (Apple: a rebuild does)
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

    rehighlightAfterEdit(0, -1);
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
    viewport_->removePendingPair();   // SP-175: a structural edit drops the pending pair (Apple: a rebuild does)
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
    const Span span("edit");
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
    if (idx < 0 || sceneDoc_.pendingEditInProgress()) {
        return;   // SP-175: the pending pair going in or out is not a scene edit — nothing to save
    }
    dirtyScenes_.insert(sceneDoc_.segments().at(idx).sceneID);
    saveTimer_->start();   // (re)arm the idle-save debounce (T-0239)
    reportEdit(idx, position, charsAdded);
}

// EP-026 S2 (SP-178): one scene edit → history (Apple: `textDidChange` → `noteEdit`). ⛔ Not for text an undo/redo applied
// (`applyHistoryStep` keeps `historyText_` itself), and a multi-scene edit is collected and reported ONCE at its end.
void EditorShell::reportEdit(int idx, int position, int charsAdded)
{
    if (history_ == nullptr || history_->isApplying()) {
        return;
    }
    const SceneSegment& seg = sceneDoc_.segments().at(idx);
    if (grouping_) {
        const int caret = storedOffset(idx, position);
        for (GroupPart& part : groupParts_) {
            if (part.sceneID == seg.sceneID) {
                part.caret = std::min(part.caret, caret);
                return;
            }
        }
        groupParts_.append({seg.sceneID, historyText_.value(seg.sceneID), caret});
        return;
    }
    const QString after = sceneDoc_.bodyText(idx);
    const QString before = historyText_.value(seg.sceneID);
    if (after == before) {
        return;   // no text change (a re-highlight reports `contentsChange` with equal counts)
    }
    historyText_.insert(seg.sceneID, after);
    // ⚠️ The caret AFTER the edit, from the change itself: the editor moves its own cursor only once the edit block ends.
    history_->noteEdit(seg.sceneID, after, storedOffset(idx, position + charsAdded), before);
    editInFlight_ = true;
    QTimer::singleShot(0, this, [this]() { editInFlight_ = false; });
}

int EditorShell::storedOffset(int idx, int docPos) const
{
    const SceneSegment& seg = sceneDoc_.segments().at(idx);
    int local = std::clamp(docPos - seg.bodyStart, 0, seg.bodyLength);
    // The pending pair is document-only (`bodyText` leaves it out): a caret after it sits that much earlier in the text.
    if (const auto& p = sceneDoc_.pending(); p && p->start >= seg.bodyStart && p->start - seg.bodyStart < local
        && sceneDoc_.bodyText(idx).size() < seg.bodyLength) {
        local = std::max(p->start - seg.bodyStart, local - 2 * p->markerLength);
    }
    return local;
}

void EditorShell::onCursorMoved()
{
    const Span span("caret moved");
    if (loading_) {
        return;
    }
    // The caret owns *where you type*; scroll owns "which scene is active". But a
    // caret jump across scenes (e.g. a click or arrow into another scene) should
    // still flush the departing scene — route it through the single active-scene
    // writer, which promotes + saves only when the scene actually changes.
    const int seg = sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position());
    // EP-026 S2 (SP-178): another scene commits the pending edit; the same scene commits unless only whitespace changed
    // (the core decides). Apple: `textViewDidChangeSelection` → `caretMoved`.
    if (history_ != nullptr && seg >= 0 && !grouping_ && !applyingLoad_) {
        const Span historySpan("history: caret moved");
        history_->caretMoved(sceneDoc_.segments().at(seg).sceneID, editInFlight_);
    }
    promoteActiveScene(seg);
}

void EditorShell::onScrolled()
{
    const Span span("scrolled");
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
    const Span span("active scene changed");
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
    // Timeline follows the active scene too (EP-025, T-0324): highlight its dot. This
    // is the single hook both promoteActiveScene and moveCaretToSegment route through,
    // so the timeline stays in sync with the navigator without a separate wiring.
    if (timeline_ != nullptr) {
        const Span span("timeline: active scene");
        timeline_->setActiveScene(sceneID);
    }

    // Scene Inspector follows the active scene too (EP-035, T-0481). This is the
    // SAME single hook promoteActiveScene and moveCaretToSegment already route
    // through for the navigator highlight and the timeline dot, so the panel
    // stays in sync without a separate wiring. setScene() is a no-op when the
    // scene has not actually changed, so scroll traffic costs nothing.
    if (inspector_ != nullptr) {
        const Span span("inspector: set scene");
        inspector_->setScene(sceneID);
    }

    // Highlight-only: find the matching child row and select it. No scroll, no caret.
    const Span navigatorSpan("navigator: highlight");
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

    const QString body = sceneDoc_.bodyText(segmentIndex);
    const QVariantMap r = bridge_->saveScene(
        projectID_, projectPath_, appSupportRoot_,
        seg.sceneID, seg.metadataPath, seg.contentPath,
        body,
        selectionAnchor, selectionFocus, scroll);
    const bool saved = !r.isEmpty() && r.value(QStringLiteral("saved")).toBool();
    if (saved) {
        notePersisted(seg.sceneID, body);   // EP-026 S2: [I-0104] / [I-0259]
        dirtyScenes_.remove(seg.sceneID);
    }
    // ⛔ [I-0304] — A FAILED WRITE LEAVES THE SCENE DIRTY (Apple: [I-0259]). It was dropped "whatever the outcome", so
    // nothing retried it and a close never wrote it: the edit lived only on screen. The bridge has already surfaced
    // errorOccurred; the next save (the debounce after an edit, a scene change, a close) retries it.
    return saved;
}

QList<int> EditorShell::paneSizes() const
{
    return splitter_ != nullptr ? splitter_->sizes() : QList<int>{};
}

QList<int> EditorShell::outerSizes() const
{
    return outerSplitter_ != nullptr ? outerSplitter_->sizes() : QList<int>{};
}

namespace {
void applySizes(QSplitter* splitter, const QList<int>& sizes)
{
    if (splitter == nullptr || sizes.size() != splitter->count()) {
        return;
    }
    for (const int s : sizes) {
        if (s <= 0) {
            return;
        }
    }
    // ✅ `setSizes` keeps the PROPORTIONS when the splitter's total differs (e.g. a
    // window restored maximized on a different screen).
    splitter->setSizes(sizes);
}
}  // namespace

void EditorShell::setPaneSizes(const QList<int>& sizes)  { applySizes(splitter_, sizes); }
void EditorShell::setOuterSizes(const QList<int>& sizes) { applySizes(outerSplitter_, sizes); }

void EditorShell::releaseProject()
{
    // EP-026 S2 (SP-178): close the undo history — after `flushHistory()` and the saves (Apple's order: flush → save →
    // close). `scrivi_history_close` writes the final checkpoint; without it every repair made this session re-flags.
    if (history_ != nullptr) {
        history_->close();
        history_->deleteLater();
        history_ = nullptr;
    }
    historyText_.clear();
    oldCutRedo_.clear();
    // EP-048 S5a ([SP-182]): the settings go with the project.
    if (prefs_ != nullptr) {
        prefs_->deleteLater();
        prefs_ = nullptr;
    }
    // EP-026 S5 (SP-179): the copy buffers go with the project (the palette lets go of them as they are destroyed).
    if (buffers_ != nullptr) {
        buffers_->deleteLater();
        buffers_ = nullptr;
    }

    // EP-039 T-0512. ⚠️ Order matters: the core is keyed by the project ROOT, so this
    // must run while `projectPath_` still holds it.
    if (bridge_ != nullptr && !projectPath_.isEmpty()) {
        bridge_->closeProject(projectPath_);
    }

    // ⚠️ SP-145 / T-0552 — DEREGISTER. ✅ The project is no longer open, so R3 must
    // stop reporting it as open; otherwise reopening it would be refused forever.
    // ⛔ The session object itself is NOT destroyed (this shell outlives one
    // project), so this is a registry removal, not a lifetime change.
    // ⚠️ [SP-146] T-0559 — registry now on `AppEnvironment`; guarded for tests.
    if (env_ != nullptr && !projectID_.isEmpty()) {
        env_->openProjects().deregister(projectID_);
    }
}

void EditorShell::stampWritingSurface()
{
    // SP-144 — Apple's `stampWritingSurface` ([I-0058]/[I-0131]), which Linux
    // lacked. See the header for why removing `openScene`'s implicit write made
    // it necessary.
    //
    // ⚠️ THIS DOES NOT DELEGATE TO `saveScene(activeSegment_)`, AND THE FIRST CUT
    // OF IT DID — which the user caught on the rig (2026-09-20): scrolling to a
    // scene WITHOUT TYPING and reopening landed "minus one line".
    //
    // ⚠️ WHY. `saveScene` writes the caret AND the scroll fraction ONLY when the
    // CARET is inside the segment being saved; otherwise it deliberately writes
    // `0/0/0.0`, so a background flush cannot clobber a real cursor with a stale
    // one. ✅ Correct for a background flush. ⛔ WRONG HERE: the case this
    // function exists for is precisely the writer who SCROLLED somewhere and
    // never typed, so the caret is in the OLD scene and the scroll was being
    // written as `0.0` — restoring to the top of the scene instead of where they
    // were looking.
    //
    // ✅ Apple gets this right and this now mirrors it exactly
    // (`ViewportSceneLoader.swift:424`): stamp the SCROLLED-TO scene, carry the
    // REAL scroll fraction always, and carry the caret offset only when the caret
    // really is in that scene (0 otherwise, per §9.3 — "scene changed / cursor
    // not here" ⇒ restore the scene and place the cursor safely).
    //
    // ⛔ Deliberately NOT conditional on the scene being dirty: `saveScene`
    // returns early for a clean scene, so delegating would silently no-op — the
    // exact "clean-scene hole" [I-0131] closed on Apple.
    if (activeSegment_ < 0 || activeSegment_ >= sceneDoc_.segments().size()) {
        return;   // nothing loaded — no surface to record
    }
    if (bridge_ == nullptr || viewport_ == nullptr) { return; }

    const SceneSegment& seg = sceneDoc_.segments().at(activeSegment_);

    // ✅ The scroll fraction ALWAYS — this is the value being preserved.
    double scroll = 0.0;
    if (QScrollBar* vsb = viewport_->verticalScrollBar()) {
        const int range = vsb->maximum() - vsb->minimum();
        scroll = range > 0
                     ? double(vsb->value() - vsb->minimum()) / double(range)
                     : 0.0;
    }

    // ⚠️ The caret offset ONLY when the caret is genuinely in this scene. Sending
    // a caret from a DIFFERENT scene would place the cursor at a meaningless
    // offset in this one.
    long long selectionAnchor = 0;
    long long selectionFocus  = 0;
    const QTextCursor caret = viewport_->textCursor();
    if (sceneDoc_.sceneIndexForCaret(caret.position()) == activeSegment_) {
        selectionAnchor = qMax(0, caret.anchor()   - seg.bodyStart);
        selectionFocus  = qMax(0, caret.position() - seg.bodyStart);
    }

    // ⚠️ Writes the scene BYTES too, exactly as Apple's stamp does — the backend
    // stamps `lastWritingSurface` on every `saveScene`, and there is no
    // surface-only endpoint. ✅ Idempotent for unchanged bytes, and this is one
    // write on the teardown path, not one per scene on load.
    const QString body = sceneDoc_.bodyText(activeSegment_);
    const QVariantMap r = bridge_->saveScene(
        projectID_, projectPath_, appSupportRoot_,
        seg.sceneID, seg.metadataPath, seg.contentPath,
        body,
        selectionAnchor, selectionFocus, scroll);
    if (!r.isEmpty() && r.value(QStringLiteral("saved")).toBool()) {
        notePersisted(seg.sceneID, body);   // EP-026 S2: [I-0104] / [I-0259]
        dirtyScenes_.remove(seg.sceneID);   // on disk now; ⛔ [I-0304]: a failed write stays dirty
    }
}

void EditorShell::saveDirtyScenes()
{
    if (dirtyScenes_.isEmpty()) {
        return;
    }
    const Span span("save dirty scenes (main thread)");
    saveTimer_->stop();   // an explicit flush supersedes a pending debounce
    // EP-026 S2 (SP-178): an autosave never seals the typing session unless the writer is already idle (T-0396;
    // Apple: `flushThenSave`). Teardown commits first, in `flushHistory()`.
    if (history_ != nullptr) {
        history_->beforeSave();
    }

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

    // Keep the timeline strip in sync after any structural change (create / delete /
    // rename / reorder / merge / split all rebuild the navigator). Guarded internally
    // against the mid-load programmatic build (load() calls reloadTimeline once at end).
    reloadTimeline();
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

// ── [SP-191] Structural edits: ONE range call off the GUI thread, the delta applied — Apple's `runStructuralEdit` ──────

namespace {
QJsonObject rangeEntry(const SceneSegment& seg, qint64 start = 0, qint64 whole = 0)
{
    return QJsonObject{{QStringLiteral("sceneID"), seg.sceneID}, {QStringLiteral("chapterID"), seg.chapterID},
                       {QStringLiteral("chapterTitle"), QString()}, {QStringLiteral("sceneTitle"), QString()},
                       {QStringLiteral("start"), start}, {QStringLiteral("length"), 0},
                       {QStringLiteral("sceneLength"), whole}, {QStringLiteral("text"), QString()}};
}
QString rangeJsonOf(const QJsonArray& entries)
{
    return QString::fromUtf8(QJsonDocument(QJsonObject{{QStringLiteral("schema"), QStringLiteral("scrivi.range.v1")},
                                                       {QStringLiteral("scenes"), entries}})
                                 .toJson(QJsonDocument::Compact));
}
QString rangeJsonOf(const QVariantMap& result)
{
    return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(result.value(QStringLiteral("range")).toMap()))
                                 .toJson(QJsonDocument::Compact));
}
QVariantList rangeScenes(const QVariantMap& result)
{
    return result.value(QStringLiteral("range")).toMap().value(QStringLiteral("scenes")).toList();
}
qint64 byteLength(const QString& s) { return s.toUtf8().size(); }
}   // namespace

bool EditorShell::runRangeEdit(const QString& label, RangeCall call, RangeNode node, RangeCaret caret,
                               std::function<void()> failed, std::function<void(const QVariantMap&)> then)
{
    if (loading_ || projectPath_.isEmpty()) {
        return false;   // ✅ Q5: a repeat (or anything else) while an edit is in flight is IGNORED, not queued
    }
    viewport_->removePendingPair();   // SP-175: a structural edit drops the pending pair
    // Every unsaved scene, written on the worker FIRST (the core refuses a caret scene that is not what is on disk) — and
    // what each scene holds now, for the node's `sceneTextsBefore` ([I-0293]).
    struct Job { QString sceneID; QString metadataPath; QString contentPath; QString text; };
    QVector<Job> jobs;
    QHash<QString, QString> before;
    for (int i = 0; i < sceneDoc_.segments().size(); ++i) {
        const SceneSegment& seg = sceneDoc_.segments().at(i);
        const QString text = sceneDoc_.bodyText(i);
        before.insert(seg.sceneID, text);
        if (dirtyScenes_.remove(seg.sceneID)) {
            jobs.append({seg.sceneID, seg.metadataPath, seg.contentPath, text});
        }
    }
    saveTimer_->stop();
    const QString historyRoot = node && history_ != nullptr && history_->isOpen() ? history_->projectRootPath() : QString();
    lockForWork(label);      // ✅ Q5: the cue — read-only at once, in the key press's own turn; the progress row after 400 ms
    beginBackgroundWork();   // [I-0305]: a close waits for it
    DiagnosticLog::note(QStringLiteral("structural \"%1\": worker started").arg(label));
    QElapsedTimer clock;
    clock.start();
    struct Outcome {
        QVariantMap result; bool failed = false; QVector<bool> saved; QVariantMap history;
        bool storyTimesRead = false; QHash<QString, TimelineData::StoryTime> storyTimes;
        QString phases;   // each phase's time, for the log
    };
    const WorkerBridge::Handle bridgeHandle = WorkerBridge::handleFor(bridge_);   // ✅ [I-0309]: never `bridge_` itself
    const QString projectID = projectID_, path = projectPath_, support = appSupportRoot_;
    AsyncCall::run<Outcome>(
        this,
        // ⚠️ The WORKER: the C ABI only, on copies. The saves, the pending typing committed, the ONE range call, its history
        // node and the timeline's read — so the GUI thread makes no core call for the edit.
        [jobs, before, historyRoot, call = std::move(call), node = std::move(node), bridgeHandle, projectID, path, support]() {
            WorkerBridge bridge(bridgeHandle);
            Outcome o;
            QElapsedTimer phase;
            phase.start();
            const auto lap = [&](const char* name) {
                o.phases += QStringLiteral(" %1 %2 ms").arg(QString::fromUtf8(name)).arg(phase.restart());
            };
            for (const Job& j : jobs) {
                const QVariantMap r = bridge->saveScene(projectID, path, support, j.sceneID, j.metadataPath, j.contentPath,
                                                        j.text, 0, 0, 0.0);
                o.saved.append(!r.isEmpty() && r.value(QStringLiteral("saved")).toBool());
            }
            lap("saves");
            if (!historyRoot.isEmpty()) {
                ScriviBridge::captureFlush(historyRoot, QStringLiteral("flush"), QStringLiteral("typing"));
            }
            lap("· flush");
            o.result = call(*bridge);
            o.failed = bridge->lastCallFailed();
            lap("· range call");
            if (o.failed) {
                return o;
            }
            if (!historyRoot.isEmpty()) {
                const QJsonObject payload = node(o.result);
                const RangeDelta d = RangeDelta::fromVariant(o.result.value(QStringLiteral("delta")).toMap());
                QHash<QString, QString> after;
                QHash<QString, QString> was;
                for (const RangeDelta::Scene& s : d.scenes) {
                    if (!s.hasText) { continue; }
                    after.insert(s.sceneID, s.text);
                    if (before.contains(s.sceneID)) { was.insert(s.sceneID, before.value(s.sceneID)); }
                }
                for (const QString& id : d.removedSceneIDs) { was.insert(id, before.value(id)); }
                o.history = ScriviBridge::captureRecordBarrier(
                    historyRoot, HistoryCapture::barrierParams(payload.value(QStringLiteral("op")).toString(), QString(),
                                                               after, payload, was));
            }
            lap("· history node");
            // ✅ [SP-191] live pass: the story times only — the rest of the timeline does not change with the manuscript.
            o.storyTimes = EditorShell::storyTimesOf(bridge->listStoryTimes(path));
            o.storyTimesRead = !bridge->lastCallFailed();
            lap("· story times");
            return o;
        },
        [this, jobs, caret = std::move(caret), failed, then = std::move(then), label, clock](const Outcome& o) {
            DiagnosticLog::note(QStringLiteral("structural \"%1\": worker finished in %2 ms%3 —%4")
                                    .arg(label).arg(clock.elapsed()).arg(o.failed ? QStringLiteral(" (FAILED)") : QString(),
                                                                         o.phases));
            for (qsizetype k = 0; k < jobs.size(); ++k) {
                if (o.saved.value(k)) {
                    notePersisted(jobs.at(k).sceneID, jobs.at(k).text);   // [I-0104] / [I-0259]: only after it succeeded
                } else if (sceneDoc_.sceneIndexForScene(jobs.at(k).sceneID) >= 0) {
                    dirtyScenes_.insert(jobs.at(k).sceneID);   // ⛔ [I-0304]: a failed write stays dirty
                }
            }
            if (o.failed) {
                unlockAfterWork();
                QApplication::beep();
                if (failed) { failed(); }
                endBackgroundWork();
                emit structuralEditFinished(false);
                return;
            }
            QElapsedTimer applying;
            applying.start();
            if (history_ != nullptr && !o.history.isEmpty()) {
                history_->adopt(o.history);
            }
            applyRangeResult(o.result);
            if (o.storyTimesRead && timelineData_.fetched) {
                TimelineData data = timelineData_;
                data.storyTimes = o.storyTimes;
                applyTimelineData(data);
            }
            unlockAfterWork();
            if (const auto [sceneID, byte] = caret(o.result); !sceneID.isEmpty()) {
                placeCaret(sceneID, byte);
            }
            DiagnosticLog::note(QStringLiteral("structural \"%1\": applied in %2 ms").arg(label).arg(applying.elapsed()));
            endBackgroundWork();
            emit structuralEditFinished(true);
            if (then) { then(o.result); }   // ✅ [SP-188] Q4 / Q6
        },
        [this, label, failed] {
            DiagnosticLog::note(QStringLiteral("structural \"%1\": TIMED OUT").arg(label));
            unlockAfterWork();
            errorLabel_->setText(tr("This change is taking longer than expected — the project may be on a disconnected or "
                                    "unreachable volume. Reopen it before editing further."));
            errorLabel_->show();
            if (failed) { failed(); }
            endBackgroundWork();
            emit structuralEditFinished(false);
        },
        kProjectOpenTimeoutMs);
    return true;
}

void EditorShell::applyRangeResult(const QVariantMap& result)
{
    const RangeDelta delta = RangeDelta::fromVariant(result.value(QStringLiteral("delta")).toMap());
    // The document edits are the core's, not the writer's: `loading_` (held by `lockForWork`) keeps them from being
    // reported or dirtied.
    const int lo = sceneDoc_.applyRangeDelta(delta);
    if (lo < 0) {
        return;
    }
    int hi = lo;   // the run's last scene, as it now stands
    for (const RangeDelta::Scene& s : delta.scenes) {
        hi = std::max(hi, sceneDoc_.sceneIndexForScene(s.sceneID));
    }
    rehighlightAfterEdit(lo, hi);
    for (const QString& id : delta.removedSceneIDs) {
        dirtyScenes_.remove(id);
        historyText_.remove(id);
        if (inspector_ != nullptr) {
            inspector_->forgetScene(id);   // ✅ [I-0315]: never write notes to a scene that is gone
        }
    }
    for (const RangeDelta::Scene& s : delta.scenes) {
        if (s.hasText) {
            dirtyScenes_.remove(s.sceneID);   // the core wrote it
            historyText_.insert(s.sceneID, s.text);
        }
    }
    // ✅ [SP-191] Q10: "Chapter N" is derived from position, never stored — the delta's reflow renumbered the headings.
    rebuildNavigator();
}

namespace {
// What the editor needs from the caret to anchor a range call: its scene and its scene-local UTF-8 byte offset.
struct CaretAt { int index = -1; SceneSegment seg; QString text; qint64 byte = 0; };
}   // namespace

void EditorShell::onCreateSceneRequested()
{
    // ⌃↩: a new scene at the caret. ✅ [SP-191] Q1: a PASTE OF A BREAK (an empty two-entry Range) — one range call off the
    // GUI thread, undone in one step (Q3: no "Can't undo past…"). ✅ Q7: at a scene's very start the new scene goes BEFORE
    // it (the scene keeps its text, title and sidecar) and the caret goes into the new scene; elsewhere the scene splits
    // and the caret starts the new one. Apple: `handleCreateScene`.
    insertBreak(/*newChapter=*/false, tr("Splitting the scene…"));
}

void EditorShell::rehighlightAfterEdit(int lo, int hi)
{
    // ✅ [SP-191] rig pass: the presenter draws each changed line INSIDE the document edit's `insertText` — the run's splice
    // and each heading `reflowBoundaryAt` renumbers — before the scene offsets after it have shifted. Judged against stale
    // positions, a heading was drawn as body text, and scene text as a heading. Once the edit has landed and every offset
    // is current: the run again, from its leading boundary through the follower's, then every chapter boundary after it
    // (a few lines per chapter).
    if (presenter_ == nullptr || presenter_->document() == nullptr) {
        return;
    }
    const QList<SceneSegment>& segs = sceneDoc_.segments();
    const auto boundaryStart = [&](int i) { return i == 0 ? 0 : segs.at(i - 1).bodyStart + segs.at(i - 1).bodyLength; };
    if (lo >= 0 && hi >= lo && hi < segs.size()) {
        presenter_->rehighlightRange(boundaryStart(lo),
                                     hi + 1 < segs.size() ? segs.at(hi + 1).bodyStart : segs.at(hi).bodyStart + segs.at(hi).bodyLength);
    }
    for (int i = std::max(hi + 1, 0); i < segs.size(); ++i) {
        if (i > 0 && segs.at(i).chapterID == segs.at(i - 1).chapterID) {
            continue;
        }
        presenter_->rehighlightRange(boundaryStart(i), segs.at(i).bodyStart);
    }
}

EditorShell::StructuralAnchor EditorShell::structuralAnchor(int pos) const
{
    StructuralAnchor at{pos, false};
    if (presenter_ == nullptr) {
        return at;
    }
    while (at.pos > 0) {
        const auto stop = presenter_->stopAt(at.pos - 1);
        if (!stop || !stop->homeAfter() || stop->end != at.pos || stop->start >= at.pos) {
            break;
        }
        if (stop->kind == ManuscriptPresenter::StopKind::Prefix || stop->kind == ManuscriptPresenter::StopKind::ListPrefix) {
            at.beforeLineMarker = true;
        }
        at.pos = stop->start;
    }
    return at;
}

void EditorShell::insertBreak(bool newChapter, const QString& label)
{
    const int caretPos = structuralAnchor(viewport_->textCursor().position()).pos;
    const int idx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (loading_ || idx < 0) {
        return;
    }
    const SceneSegment seg = sceneDoc_.segments().at(idx);
    const QString text = sceneDoc_.bodyText(idx);
    const int caretChar = std::clamp(caretPos - seg.bodyStart, 0, int(text.size()));
    const qint64 caretByte = byteLength(text.left(caretChar));
    QJsonObject first = rangeEntry(seg);
    first.insert(QStringLiteral("sceneID"), QString());          // ✅ Q1: an empty ID mints a fresh one
    QJsonObject opening = first;
    if (newChapter) {
        opening.insert(QStringLiteral("chapterID"), QString());  // another chapter: the paste opens one there
    }
    const QString rangeJson = rangeJsonOf(QJsonArray{first, opening});
    const QString root = projectPath_, name = bridge_->displayName(), caretScene = seg.sceneID;
    runRangeEdit(
        label,
        [=](ScriviBridge& bridge) { return bridge.rangePaste(root, rangeJson, caretScene, caretByte, text, name); },
        [=](const QVariantMap& r) {
            return QJsonObject{{QStringLiteral("op"), QStringLiteral("rangePaste")},
                               {QStringLiteral("caretSceneID"), caretScene}, {QStringLiteral("caretByte"), caretByte},
                               {QStringLiteral("rangeJSON"), rangeJsonOf(r)}};
        },
        [caretScene](const QVariantMap& r) -> std::pair<QString, qint64> {
            // The new scene — after the caret scene, or before it at a scene's start (Q7).
            for (const QVariant& v : rangeScenes(r)) {
                const QString id = v.toMap().value(QStringLiteral("sceneID")).toString();
                if (id != caretScene) { return {id, 0}; }
            }
            return {};
        });
}

void EditorShell::onCreateChapterRequested()
{
    // ⌃⇧↩: a new chapter at the caret — the same paste, its second entry in another chapter, so the core creates the
    // chapter IN PLACE and moves the rest of this chapter into it ([I-0311], [I-0074]). ✅ Q8: at a scene's very start it
    // is a RELABEL — the scene and the rest of its chapter move into a new chapter; no empty scene, no text moves. At a
    // chapter's first scene a chapter already starts there: nothing happens. No confirmation (Q9). Apple:
    // `handleCreateChapter`.
    const int caretPos = structuralAnchor(viewport_->textCursor().position()).pos;   // before a heading's `## `
    const int idx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (loading_ || idx < 0) {
        return;
    }
    const SceneSegment seg = sceneDoc_.segments().at(idx);
    if (caretPos > seg.bodyStart) {
        insertBreak(/*newChapter=*/true, tr("Splitting the chapter…"));
        return;
    }
    if (idx == 0 || sceneDoc_.segments().at(idx - 1).chapterID != seg.chapterID) {
        return;
    }
    QJsonArray run;
    for (int i = idx; i < sceneDoc_.segments().size() && sceneDoc_.segments().at(i).chapterID == seg.chapterID; ++i) {
        run.append(rangeEntry(sceneDoc_.segments().at(i)));
    }
    relabel(rangeJsonOf(run), QString(), seg.sceneID, tr("Splitting the chapter…"));
}

void EditorShell::onMergeSceneRequested()
{
    // ⌃⌫ at a scene's very start: merge it into the scene before (same chapter only). ✅ [SP-191] Q3e: a CUT of the
    // zero-length range across the break — the texts join DIRECTLY (Q3a: no blank line), the removed scene's sidecar
    // travels in the Range (Q3b), and Undo puts the break back exactly. Apple: `handleMergeScene`.
    const int caretPos = structuralAnchor(viewport_->textCursor().position()).pos;   // before a heading's `## `
    const int idx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (loading_ || idx <= 0 || idx >= sceneDoc_.segments().size()) {
        return;
    }
    const SceneSegment current = sceneDoc_.segments().at(idx);
    const SceneSegment previous = sceneDoc_.segments().at(idx - 1);
    if (caretPos != current.bodyStart || current.chapterID != previous.chapterID) {
        return;   // only at the very start, and a chapter's first scene is the chapter merge's
    }
    const qint64 prevBytes = byteLength(sceneDoc_.bodyText(idx - 1));
    const QString rangeJson = rangeJsonOf(QJsonArray{rangeEntry(previous, prevBytes, prevBytes),
                                                     rangeEntry(current, 0, byteLength(sceneDoc_.bodyText(idx)))});
    cutRange(rangeJson, tr("Merging the scenes…"));
}

void EditorShell::onMergeChapterRequested()
{
    // ⌃⇧⌫ at a chapter's first scene: merge the chapter into the one before. ✅ [SP-191] Q3e: a RELABEL of its scenes into
    // that chapter — on disk their files move; no text changes. Undo re-creates the chapter from its sidecar, which the
    // result carries (Q3b). Every scene of the chapter joins the earlier one (Q3d). Apple: `handleMergeChapter`.
    const int caretPos = structuralAnchor(viewport_->textCursor().position()).pos;   // before a heading's `## `
    const int idx = sceneDoc_.sceneIndexForCaret(caretPos);
    if (loading_ || idx <= 0 || idx >= sceneDoc_.segments().size()) {
        return;
    }
    const SceneSegment current = sceneDoc_.segments().at(idx);
    const QString previous = sceneDoc_.segments().at(idx - 1).chapterID;
    if (caretPos != current.bodyStart || current.chapterID == previous) {
        return;
    }
    QJsonArray run;
    for (int i = idx; i < sceneDoc_.segments().size() && sceneDoc_.segments().at(i).chapterID == current.chapterID; ++i) {
        run.append(rangeEntry(sceneDoc_.segments().at(i)));
    }
    relabel(rangeJsonOf(run), previous, current.sceneID, tr("Merging the chapters…"));
}

bool EditorShell::cutRange(const QString& rangeJson, const QString& label, std::function<void(const QVariantMap&)> then)
{
    const QString root = projectPath_, name = bridge_->displayName();
    return runRangeEdit(
        label,
        [=](ScriviBridge& bridge) { return bridge.rangeCut(root, rangeJson, name); },
        [](const QVariantMap& r) {
            return QJsonObject{{QStringLiteral("op"), QStringLiteral("rangeCut")},
                               {QStringLiteral("caretSceneID"), r.value(QStringLiteral("undoCaretSceneID")).toString()},
                               {QStringLiteral("caretByte"), r.value(QStringLiteral("undoCaretByte")).toLongLong()},
                               {QStringLiteral("rangeJSON"), rangeJsonOf(r)}};
        },
        [](const QVariantMap& r) -> std::pair<QString, qint64> {
            return {r.value(QStringLiteral("undoCaretSceneID")).toString(), r.value(QStringLiteral("undoCaretByte")).toLongLong()};
        },
        {}, std::move(then));
}

bool EditorShell::cutAcrossScenes(CutKeeping keeping, const QString& bufferID, std::function<void()> then)
{
    const auto range = viewport_->crossSceneCutRange();
    if (!range) {
        return false;
    }
    if (loading_ || projectPath_.isEmpty()) {
        return true;   // ✅ Q5: ignored, not queued
    }
    std::optional<ManuscriptEditor::CrossSceneCopy> copy;
    QString token;
    if (keeping != CutKeeping::Nothing) {
        copy = viewport_->crossSceneCopy();
    }
    if (keeping == CutKeeping::Clipboard && copy) {
        token = viewport_->putCutOnClipboard(*copy);
    }
    const QString json = QString::fromUtf8(QJsonDocument(*range).toJson(QJsonDocument::Compact));
    cutRange(json, keeping == CutKeeping::Nothing ? tr("Deleting…") : tr("Cutting…"),
             [this, keeping, bufferID, copy, token, then = std::move(then)](const QVariantMap& r) {
                 // Q4: the returned Range, the copy's balanced pieces in place of the stored ones (same scenes, same order).
                 QJsonObject held = QJsonObject::fromVariantMap(r.value(QStringLiteral("range")).toMap());
                 held.remove(QStringLiteral("join"));   // an undo record: the core refuses it anywhere but at its survivor
                 QJsonArray entries = held.value(QStringLiteral("scenes")).toArray();
                 const QJsonArray copied = copy ? copy->range.value(QStringLiteral("scenes")).toArray() : QJsonArray();
                 if (copied.size() == entries.size()) {
                     for (qsizetype k = 0; k < entries.size(); ++k) {
                         QJsonObject e = entries.at(k).toObject();
                         e.insert(QStringLiteral("text"), copied.at(k).toObject().value(QStringLiteral("text")));
                         entries.replace(k, e);
                     }
                     held.insert(QStringLiteral("scenes"), entries);
                 }
                 held = ManuscriptEditor::normalizedRange(held);
                 if (keeping == CutKeeping::Clipboard) {
                     viewport_->holdCutRange(token, held);
                 } else if (keeping == CutKeeping::Buffer && buffers_ != nullptr) {
                     buffers_->load(copy ? copy->source : QString(), bufferID, held);
                 }
                 if (then) { then(); }
             });
    return true;
}

void EditorShell::relabel(const QString& rangeJson, const QString& target, const QString& keepCaretIn, const QString& label)
{
    const QString root = projectPath_, name = bridge_->displayName();
    runRangeEdit(
        label,
        [=](ScriviBridge& bridge) { return bridge.rangeRelabel(root, rangeJson, target, name); },
        [](const QVariantMap& r) {
            const QVariantList scenes = rangeScenes(r);
            return QJsonObject{{QStringLiteral("op"), QStringLiteral("rangeRelabel")},
                               {QStringLiteral("rangeJSON"), rangeJsonOf(r)},
                               {QStringLiteral("fromChapterID"), r.value(QStringLiteral("fromChapterID")).toString()},
                               {QStringLiteral("targetChapterID"),
                                scenes.isEmpty() ? QString() : scenes.first().toMap().value(QStringLiteral("chapterID")).toString()}};
        },
        [keepCaretIn](const QVariantMap&) -> std::pair<QString, qint64> { return {keepCaretIn, 0}; });
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

void EditorShell::cutSelection()   { viewport_->cutSelection(); }   // [I-0270] through the guard
void EditorShell::copySelection()  { viewport_->copyToClipboard(); }   // [I-0307]
void EditorShell::applyFormat(ManuscriptEditor::Format format) { viewport_->applyFormat(format); }   // SP-175
void EditorShell::find(FindCommand command) { findBar_->run(command); }   // SP-176
void EditorShell::pasteClipboard() { viewport_->pasteClipboard(); }   // EP-026 S4: may be structured

// ── EP-026 S2 (SP-178 / T-0605): undo / redo ───────────────────────────────────────────────────────────────────────
// Apple: `ManuscriptTextView.Coordinator` — `performUndo`, `apply(step:)`, `applySceneChange`, the barrier and
// session-boundary notices.

bool EditorShell::canUndo() const { return history_ != nullptr && history_->canUndo(); }
bool EditorShell::undoAvailable() const { return canUndo() || (history_ != nullptr && history_->atBarrier()); }
bool EditorShell::canRedo() const { return history_ != nullptr && history_->canRedo(); }

void EditorShell::undo()
{
    const Span span("undo");
    if (history_ != nullptr && !loading_) {
        // ✅ [I-0319]: asked before an undo of an earlier session's change, never for this session's.
        const QString when = history_->undoEntersEarlierSession();
        if (!when.isEmpty() && !confirmEarlierSessionUndo(when)) {
            return;
        }
        applyHistoryStep(history_->undo(), /*isUndo=*/true);
    }
}

void EditorShell::redo()
{
    const Span span("redo");
    if (history_ != nullptr && !loading_) {
        applyHistoryStep(history_->redo(), /*isUndo=*/false);
    }
}

void EditorShell::flushHistory()
{
    if (history_ != nullptr) {
        history_->flush(QStringLiteral("flush"));
    }
}

void EditorShell::notePersisted(const QString& sceneID, const QString& text)
{
    if (history_ != nullptr) {
        history_->noteScenePersisted(sceneID, text);
    }
}

void EditorShell::recordBarrier(const QString& kind, const QString& note, const QHash<QString, QString>& sceneTexts)
{
    if (history_ != nullptr) {
        history_->recordBarrier(kind, note, sceneTexts);
    }
}

namespace {
// Apple's `NSAlert` (informational, one OK button) — a modal notice.
void historyNotice(QWidget* parent, const QString& text, const QString& detail = {})
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Information);
    box.setObjectName(QStringLiteral("historyNotice"));
    box.setText(text);
    box.setInformativeText(detail);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}
}   // namespace

// ── EP-026 S5 (SP-179 / T-0606): the copy buffers ─────────────────────────────────────────────────────────────────
// Apple: the coordinator's `copyIntoBuffer` / `cutIntoBuffer` / `pasteFromBuffer`. The editor does the text; the slots
// are `buffers_`'s.

void EditorShell::copyIntoBuffer(const QString& bufferID)
{
    if (buffers_ == nullptr) {
        return;
    }
    // Copy is not a text change: no history event (Trade T3), and the system clipboard is untouched.
    // ✅ EP-026 S4: across scenes the slot holds the Range too ([SP-191] Q4a), so its paste rebuilds the scene breaks.
    if (const auto cross = viewport_->crossSceneCopy()) {
        buffers_->load(cross->source, bufferID, cross->range);
        return;
    }
    const QString source = viewport_->selectionSource();
    if (!source.isEmpty()) {
        buffers_->load(source, bufferID);
    }
}

void EditorShell::cutIntoBuffer(const QString& bufferID)
{
    if (buffers_ == nullptr || loading_) {
        return;
    }
    viewport_->setFocus();   // a palette click must not leave the editor unfocused (Apple: `makeFirstResponder`)
    if (cutAcrossScenes(CutKeeping::Buffer, bufferID)) {
        return;   // ✅ [SP-188] across scenes: the structural cut, its Range into the slot when it lands (Q4)
    }
    viewport_->cutSelectionIntoBuffer(bufferID, [this, bufferID](const QString& source, const QJsonObject& range) {
        return buffers_->load(source, bufferID, range);
    });
}

void EditorShell::pasteFromBuffer(const QString& bufferID)
{
    if (buffers_ == nullptr || loading_) {
        return;
    }
    const QString text = buffers_->text(bufferID);
    if (text.isEmpty()) {
        return;   // an empty slot → silent no-op (Apple)
    }
    viewport_->setFocus();
    // ✅ EP-026 S4: a slot holding scenes — copied here or on the Mac — rebuilds them (Apple: `pasteStructuredRange`).
    if (const QJsonObject range = buffers_->range(bufferID); !range.isEmpty()) {
        pasteRange(range);
        return;
    }
    viewport_->pasteStored(text);
}

// ── EP-026 S4 (SP-180 / T-0607): structured paste — Apple's `pasteStructuredFragment` and `applyStructuralInverse` ───


void EditorShell::pasteRange(const QJsonObject& rangeIn)
{
    // ⌃V of a cross-scene copy, and ⌃⇧N of a slot holding one. Apple: `pasteStructuredRange`. ✅ [SP-191]: ONE
    // `rangePaste` off the GUI thread, the delta applied, no reload; Undo cuts exactly what it inserted. ✅ Q7: at a scene's
    // very start the pasted scenes go before it and the last part joins its start. The caret lands at the end of the block.
    const Span span("structured paste");
    if (loading_ || projectPath_.isEmpty() || rangeIn.value(QStringLiteral("scenes")).toArray().isEmpty()) {
        return;
    }
    viewport_->setFocus();
    viewport_->removePendingPair();
    // ✅ [SP-180] Q2 ([I-0295]): a selection is replaced — removed first, as its own step. ✅ [SP-188] Q6: across scenes
    // the removal is the structural cut, and the paste follows when it lands (the caret then at the join).
    if (cutAcrossScenes(CutKeeping::Nothing, {}, [this, rangeIn] { pasteRange(rangeIn); })) {
        return;
    }
    if (!viewport_->removeSelectionForPaste()) {
        QApplication::beep();   // the caret is in a heading or separator (Apple flashes and does not paste)
        return;
    }
    const StructuralAnchor anchor = structuralAnchor(viewport_->textCursor().position());
    const int pos = anchor.pos;
    const int idx = sceneDoc_.sceneIndexForEditablePosition(pos);
    if (idx < 0) {
        QApplication::beep();
        return;
    }
    const SceneSegment seg = sceneDoc_.segments().at(idx);
    const QString text = sceneDoc_.bodyText(idx);
    const int caretChar = std::clamp(pos - seg.bodyStart, 0, int(text.size()));
    const qint64 caretByte = byteLength(text.left(caretChar));

    // ✅ [T-0591]: pasting INSIDE a span — the first piece closes it, the last re-opens it for the text after the split;
    // the pasted text keeps its own formatting ([SP-163]).
    QJsonObject range = rangeIn;
    QJsonArray entries = range.value(QStringLiteral("scenes")).toArray();
    // ✅ [SP-191] live pass: anchored before a heading's (or list item's) marker, the last part keeps its own line — joined
    // onto `## Heading` it would turn the heading into body text. The line break is part of what the paste inserts, so its
    // undo takes it back too. Apple: `pasteStructuredRange`.
    if (anchor.beforeLineMarker && entries.size() > 1) {
        QJsonObject last = entries.last().toObject();
        const QString lastText = last.value(QStringLiteral("text")).toString();
        if (!lastText.isEmpty() && !lastText.endsWith(QLatin1Char('\n'))) {
            last.insert(QStringLiteral("text"), lastText + QLatin1Char('\n'));
            entries.replace(entries.size() - 1, last);
            range.insert(QStringLiteral("scenes"), entries);
            range = ManuscriptEditor::normalizedRange(range);
            entries = range.value(QStringLiteral("scenes")).toArray();
        }
    }
    QStringList texts;
    for (const QJsonValue& e : std::as_const(entries)) {
        texts.append(e.toObject().value(QStringLiteral("text")).toString());
    }
    const QStringList balanced =
        ScriviBridge::markdownBalancePastePieces(texts, ScriviBridge::markdownStyleAtCaret(text, caretChar));
    if (balanced != texts) {
        for (int k = 0; k < entries.size(); ++k) {
            QJsonObject e = entries.at(k).toObject();
            e.insert(QStringLiteral("text"), balanced.at(k));
            entries.replace(k, e);
        }
        range.insert(QStringLiteral("scenes"), entries);
        range = ManuscriptEditor::normalizedRange(range);
    }
    const QString rangeJson = QString::fromUtf8(QJsonDocument(range).toJson(QJsonDocument::Compact));
    pasteRangeAt(rangeJson, seg.sceneID, caretByte, text, tr("Pasting…"), /*records=*/true, {});
}

bool EditorShell::pasteRangeAt(const QString& rangeJson, const QString& caretScene, qint64 caretByte, const QString& caretText,
                               const QString& label, bool records, std::function<void()> failed,
                               std::function<void(const QVariantMap&)> inserted)
{
    const QString root = projectPath_, name = bridge_->displayName();
    return runRangeEdit(
        label,
        [=](ScriviBridge& bridge) { return bridge.rangePaste(root, rangeJson, caretScene, caretByte, caretText, name); },
        records ? RangeNode([=](const QVariantMap& r) {
                      return QJsonObject{{QStringLiteral("op"), QStringLiteral("rangePaste")},
                                         {QStringLiteral("caretSceneID"), caretScene}, {QStringLiteral("caretByte"), caretByte},
                                         {QStringLiteral("rangeJSON"), rangeJsonOf(r)}};
                  })
                : RangeNode(),
        [inserted](const QVariantMap& r) -> std::pair<QString, qint64> {
            if (inserted) { inserted(r); }
            const QVariantList scenes = rangeScenes(r);   // the end of what it put in
            if (scenes.isEmpty()) { return {}; }
            const QVariantMap last = scenes.last().toMap();
            return {last.value(QStringLiteral("sceneID")).toString(),
                    last.value(QStringLiteral("start")).toLongLong() + last.value(QStringLiteral("length")).toLongLong()};
        },
        std::move(failed));
}

void EditorShell::applyStructuralInverse(const QVariantMap& inverse, bool isUndo)
{
    // ✅ [SP-191] Undo and redo of a structural node: the inverse range call, through the same off-thread path as the edit
    // — the delta applied, nothing re-read. Each node holds the ONE Range both directions use (Apple:
    // `applyStructuralInverse`, field for field):
    //   rangePaste    undo = cut what it inserted                          redo = paste that again at its caret
    //   rangeCut      undo = paste what it removed at the survivor's seam   redo = cut that again
    //   rangeRelabel  undo = relabel the run back into the chapter it left  redo = into the chapter it went to
    // ✅ [SP-187] Q2: a node written before SP-191 (`structuredPaste` / `structuredCut`, a scrivi.fragment.v1) is CONVERTED
    // to a Range and takes the same path. ⚠️ An inverse the core refuses steps the history back, so the pointer never says
    // the manuscript is somewhere it is not.
    const QVariantMap p = inverse.value(QStringLiteral("payload")).toMap();
    const QString op = p.value(QStringLiteral("op")).toString();
    const bool undo = inverse.value(QStringLiteral("direction")).toString() == QStringLiteral("undo");
    const QString caretScene = p.value(QStringLiteral("caretSceneID")).toString();
    const qint64 caretByte = p.value(QStringLiteral("caretByte")).toLongLong();
    const QString label = isUndo ? tr("Undoing…") : tr("Redoing…");
    const auto stepBack = [this, undo] {
        if (history_ != nullptr) { undo ? history_->redo() : history_->undo(); }
        historyNotice(this, undo ? tr("This step could not be undone.") : tr("This step could not be redone."));
    };
    const auto textOf = [this](const QString& sceneID) {
        const int idx = sceneDoc_.sceneIndexForScene(sceneID);
        return idx < 0 ? QString() : sceneDoc_.bodyText(idx);
    };
    const QString root = projectPath_, name = bridge_->displayName();
    const auto cutInverse = [&](const QString& rangeJson) {
        if (!runRangeEdit(
                label, [=](ScriviBridge& bridge) { return bridge.rangeCut(root, rangeJson, name); }, RangeNode(),
                [](const QVariantMap& r) -> std::pair<QString, qint64> {
                    return {r.value(QStringLiteral("undoCaretSceneID")).toString(),
                            r.value(QStringLiteral("undoCaretByte")).toLongLong()};
                },
                stepBack)) {
            stepBack();
        }
    };
    const auto pasteInverse = [&](const QString& rangeJson, std::function<void(const QVariantMap&)> inserted = {}) {
        if (sceneDoc_.sceneIndexForScene(caretScene) < 0) { stepBack(); return; }
        if (!pasteRangeAt(rangeJson, caretScene, caretByte, textOf(caretScene), label, /*records=*/false, stepBack,
                          std::move(inserted))) {
            stepBack();
        }
    };

    if (op == QStringLiteral("rangePaste") || op == QStringLiteral("rangeCut")) {
        const QString rangeJson = p.value(QStringLiteral("rangeJSON")).toString();
        if ((op == QStringLiteral("rangePaste")) == undo) { cutInverse(rangeJson); } else { pasteInverse(rangeJson); }
        return;
    }
    if (op == QStringLiteral("rangeRelabel")) {
        const QString rangeJson = p.value(QStringLiteral("rangeJSON")).toString();
        const QString target = p.value(undo ? QStringLiteral("fromChapterID") : QStringLiteral("targetChapterID")).toString();
        const int caretIdx = sceneDoc_.sceneIndexForCaret(viewport_->textCursor().position());
        const QString keep = caretIdx >= 0 ? sceneDoc_.segments().at(caretIdx).sceneID : QString();
        if (!runRangeEdit(
                label, [=](ScriviBridge& bridge) { return bridge.rangeRelabel(root, rangeJson, target, name); }, RangeNode(),
                [keep](const QVariantMap&) -> std::pair<QString, qint64> { return {keep, 0}; }, stepBack)) {
            stepBack();
        }
        return;
    }
    const QJsonObject fragment = QJsonDocument::fromJson(p.value(QStringLiteral("fragmentJSON")).toString().toUtf8()).object();
    const QJsonArray pieces = fragment.value(QStringLiteral("pieces")).toArray();
    if (op == QStringLiteral("structuredPaste")) {
        // What it inserted: the fragment's pieces under the target and the scenes and chapters it created.
        const QStringList created = p.value(QStringLiteral("createdSceneIDs")).toStringList();
        const QStringList createdChapters = p.value(QStringLiteral("createdChapterIDs")).toStringList();
        if (pieces.isEmpty() || pieces.size() != created.size() + 1) { stepBack(); return; }
        const int caretIdx = sceneDoc_.sceneIndexForScene(caretScene);
        QString chapter = caretIdx >= 0 ? sceneDoc_.segments().at(caretIdx).chapterID : QString();
        qsizetype nextChapter = 0;
        QJsonArray entries;
        for (qsizetype i = 0; i < pieces.size(); ++i) {
            const QJsonObject piece = pieces.at(i).toObject();
            const QString id = i == 0 ? caretScene : created.at(i - 1);
            if (piece.value(QStringLiteral("opensWith")).toString() == QStringLiteral("chapter")) {
                chapter = createdChapters.value(nextChapter++);
            }
            const QString pieceText = piece.value(QStringLiteral("text")).toString();
            const qint64 start = i == 0 ? caretByte : 0;
            const int at = sceneDoc_.sceneIndexForScene(id);
            entries.append(QJsonObject{{QStringLiteral("sceneID"), id},
                                       {QStringLiteral("chapterID"), at >= 0 ? sceneDoc_.segments().at(at).chapterID : chapter},
                                       {QStringLiteral("chapterTitle"), piece.value(QStringLiteral("chapterTitle")).toString()},
                                       {QStringLiteral("sceneTitle"), piece.value(QStringLiteral("sceneTitle")).toString()},
                                       {QStringLiteral("start"), start}, {QStringLiteral("length"), byteLength(pieceText)},
                                       {QStringLiteral("sceneLength"), at >= 0 ? byteLength(textOf(id)) : start + byteLength(pieceText)},
                                       {QStringLiteral("text"), pieceText}});
        }
        if (undo) {
            cutInverse(rangeJsonOf(entries));
        } else {
            const QJsonObject normal = ManuscriptEditor::normalizedRange(
                QJsonObject{{QStringLiteral("schema"), QStringLiteral("scrivi.range.v1")}, {QStringLiteral("scenes"), entries}});
            pasteInverse(QString::fromUtf8(QJsonDocument(normal).toJson(QJsonDocument::Compact)));
        }
        return;
    }
    if (op == QStringLiteral("structuredCut")) {
        // The collapsing cut (retired by [I-0270]): its undo pastes the fragment back at the survivor; its redo cuts what
        // that paste inserted, remembered for this window's session.
        const QString key = QStringLiteral("%1|%2|%3").arg(caretScene, QString::number(caretByte),
                                                           p.value(QStringLiteral("fragmentJSON")).toString());
        if (undo) {
            if (pieces.isEmpty()) { stepBack(); return; }
            QJsonArray entries;
            int chapter = 0;
            for (const QJsonValue& v : pieces) {
                const QJsonObject piece = v.toObject();
                if (piece.value(QStringLiteral("opensWith")).toString() == QStringLiteral("chapter")) { ++chapter; }
                const QString pieceText = piece.value(QStringLiteral("text")).toString();
                entries.append(QJsonObject{{QStringLiteral("sceneID"), QString()},
                                           {QStringLiteral("chapterID"), QStringLiteral("~fragment-chapter-%1").arg(chapter)},
                                           {QStringLiteral("chapterTitle"), piece.value(QStringLiteral("chapterTitle")).toString()},
                                           {QStringLiteral("sceneTitle"), piece.value(QStringLiteral("sceneTitle")).toString()},
                                           {QStringLiteral("start"), 0}, {QStringLiteral("length"), byteLength(pieceText)},
                                           {QStringLiteral("sceneLength"), byteLength(pieceText)},
                                           {QStringLiteral("text"), pieceText}});
            }
            pasteInverse(rangeJsonOf(entries), [this, key](const QVariantMap& r) { oldCutRedo_.insert(key, rangeJsonOf(r)); });
        } else if (const QString inserted = oldCutRedo_.take(key); !inserted.isEmpty()) {
            cutInverse(inserted);
        } else {
            stepBack();
        }
        return;
    }
    stepBack();
}

void EditorShell::beginGroupedEdit()
{
    grouping_ = true;
    groupParts_.clear();
}

void EditorShell::endGroupedEdit(const QString& kind)
{
    grouping_ = false;
    QJsonArray edits;
    for (const GroupPart& part : std::as_const(groupParts_)) {
        const int idx = sceneDoc_.sceneIndexForScene(part.sceneID);
        if (idx < 0) { continue; }
        const QString after = sceneDoc_.bodyText(idx);
        historyText_.insert(part.sceneID, after);
        edits.append(QJsonObject{{QStringLiteral("sceneID"), part.sceneID},
                                 {QStringLiteral("textBefore"), part.before},
                                 {QStringLiteral("textAfter"), after},
                                 {QStringLiteral("caret"), part.caret}});
    }
    groupParts_.clear();
    if (history_ != nullptr && !edits.isEmpty()) {
        history_->groupedEdit(QString::fromUtf8(QJsonDocument(edits).toJson(QJsonDocument::Compact)), kind);
    }
}

void EditorShell::lockForWork(const QString& label)
{
    loading_ = true;                 // undo, redo and the buffer actions wait; nothing is reported to history meanwhile
    viewport_->setReadOnly(true);
    if (progressBar_ != nullptr) {
        progressBar_->setRange(0, 0);
        progressLabel_->setText(label);
    }
    if (progressDelay_ != nullptr) { progressDelay_->start(); }
}

void EditorShell::unlockAfterWork()
{
    hideLoadProgress();
    loading_ = false;
    viewport_->setReadOnly(false);
}

void EditorShell::applyTypography()
{
    const QFont font = prefs_ != nullptr ? prefs_->typography().bodyFont() : ManuscriptTypography().bodyFont();
    // ✅ Not an edit: Qt reports no text change for a font or a re-highlight, so nothing is dirtied or reported (measured:
    // `settings_smoke` §3, and a guard here caught no mutation).
    viewport_->setFont(font);
    viewport_->document()->setDefaultFont(font);
    if (presenter_ != nullptr && presenter_->document() != nullptr) {
        presenter_->rehighlight();   // headings, emphasis and chapter titles are sized from the default font
    }
}

bool EditorShell::markupHints() const
{
    return prefs_ == nullptr || prefs_->markupHints();
}

void EditorShell::setMarkupHints(bool on)
{
    if (prefs_ != nullptr) {
        prefs_->setMarkupHints(on);
    }
}

void EditorShell::beginBackgroundWork()
{
    ++backgroundWork_;
}

void EditorShell::endBackgroundWork()
{
    if (backgroundWork_ <= 0 || --backgroundWork_ > 0) {
        return;
    }
    const QList<std::function<void()>> waiters = std::exchange(backgroundWaiters_, {});
    for (const auto& w : waiters) {
        w();
    }
}

void EditorShell::whenBackgroundWorkDone(std::function<void()> then)
{
    if (backgroundWork_ == 0) {
        then();
        return;
    }
    backgroundWaiters_.append(std::move(then));
}

// ✅ SP-181 (EP-048 S4b) — Replace All. Apple's shape (`replaceAll` + `applyReplacements`): every match escaped, snapped and
// balanced as a single Replace, back to front, ONE grouped `replace` step ([SP-164] Q2). ✅ Q3: nothing heavy on the GUI
// thread — the balanced edits are computed on a worker (the scene texts are plain strings there), the GUI thread only
// applies the finished texts (Qt's document is GUI-only), and the writes run on a worker again. Only scenes holding a match
// are edited or written.
void EditorShell::replaceAll(const QVector<MarkdownRange>& matches, const QString& typed, std::function<void(int)> done)
{
    if (loading_ || matches.isEmpty() || projectPath_.isEmpty()) {
        done(0);
        return;
    }
    viewport_->removePendingPair();   // not scene text ([SP-175] Q2): the offsets below are the scenes' own
    struct SceneEdits { QString sceneID; QString before; QVector<MarkdownRange> ranges; };   // ranges: scene-local
    QVector<SceneEdits> scenes;
    for (const MarkdownRange& m : matches) {
        int start = m.start;
        int end = m.end;
        viewport_->snapForTyping(start, end);
        const auto slice = sceneDoc_.sceneHolding(start, end);
        if (!slice) {
            continue;
        }
        const QString id = sceneDoc_.segments().at(slice->index).sceneID;
        if (scenes.isEmpty() || scenes.last().sceneID != id) {
            scenes.append({id, slice->text, {}});
        }
        scenes.last().ranges.append({start - slice->start, end - slice->start});
    }
    if (scenes.isEmpty()) {
        done(0);
        return;
    }
    const QString escaped = ManuscriptEscapes::escape(typed);
    lockForWork(tr("Replacing…"));
    beginBackgroundWork();   // [I-0305]: a close waits for it
    DiagnosticLog::note(QStringLiteral("replace all: %1 matches in %2 scenes, worker started").arg(matches.size()).arg(scenes.size()));
    QElapsedTimer replaceClock;
    replaceClock.start();
    AsyncCall::run<QVector<QString>>(
        this,
        // ⚠️ The WORKER: the C ABI only, on copies.
        [scenes, escaped]() {
            QVector<QString> after;
            after.reserve(scenes.size());
            for (const SceneEdits& sc : scenes) {
                QString text = sc.before;
                for (qsizetype k = sc.ranges.size() - 1; k >= 0; --k) {   // back to front: earlier offsets stay valid
                    const MarkdownRange r = sc.ranges.at(k);
                    if (const auto e = ScriviBridge::markdownBalancedEdit(text, r.start, r.end, escaped, false)) {
                        text.replace(e->range.start, e->range.end - e->range.start, e->replacement);
                    } else {
                        text.replace(r.start, r.end - r.start, escaped);
                    }
                }
                after.append(text);
            }
            return after;
        },
        [this, scenes, done, replaceClock](const QVector<QString>& after) {
            DiagnosticLog::note(QStringLiteral("replace all: worker finished in %1 ms").arg(replaceClock.elapsed()));
            // The GUI thread: the finished texts go in as the writer's own edit — reconciled, dirtied, ONE step.
            loading_ = false;
            beginGroupedEdit();
            QList<int> touched;
            int replaced = 0;
            for (qsizetype k = 0; k < scenes.size(); ++k) {
                const int idx = sceneDoc_.sceneIndexForScene(scenes.at(k).sceneID);
                if (idx < 0 || after.at(k) == scenes.at(k).before || sceneDoc_.bodyText(idx) != scenes.at(k).before) {
                    continue;
                }
                replaceSceneSpan(idx, scenes.at(k).before, after.at(k));
                touched.append(idx);
                replaced += int(scenes.at(k).ranges.size());
            }
            endGroupedEdit(QStringLiteral("replace"));
            loading_ = true;
            qInfo("[SCRIVI-FIND] replaceAll %d edits / %lld scenes", replaced, qlonglong(touched.size()));
            saveScenesOffMain(touched, [this, done, replaced] {
                unlockAfterWork();
                done(replaced);
                endBackgroundWork();
            });
        },
        [this, done] {
            unlockAfterWork();
            errorLabel_->setText(tr("Replace All is taking longer than expected — the project may be on a disconnected or "
                                    "unreachable volume. Reopen it before editing further."));
            errorLabel_->show();
            done(0);
            endBackgroundWork();
        },
        kProjectOpenTimeoutMs);
}

void EditorShell::replaceSceneSpan(int index, const QString& before, const QString& after)
{
    qsizetype head = 0;
    const qsizetype most = std::min(before.size(), after.size());
    while (head < most && before.at(head) == after.at(head)) {
        ++head;
    }
    qsizetype tail = 0;
    while (tail < most - head && before.at(before.size() - 1 - tail) == after.at(after.size() - 1 - tail)) {
        ++tail;
    }
    const int bodyStart = sceneDoc_.segments().at(index).bodyStart;
    QTextCursor c(sceneDoc_.document());
    c.beginEditBlock();
    c.setPosition(bodyStart + int(head));
    c.setPosition(bodyStart + int(before.size() - tail), QTextCursor::KeepAnchor);
    // ⛔ Raw — the text is already balanced; `replaceRange` would balance it again.
    c.insertText(after.mid(head, after.size() - tail - head), QTextCharFormat());
    c.endEditBlock();
}

void EditorShell::saveScenesOffMain(const QList<int>& indices, std::function<void()> landed)
{
    struct Job { QString sceneID; QString metadataPath; QString contentPath; QString text; };
    QVector<Job> jobs;
    for (const int idx : indices) {
        const SceneSegment& seg = sceneDoc_.segments().at(idx);
        if (!dirtyScenes_.contains(seg.sceneID)) {
            continue;
        }
        jobs.append({seg.sceneID, seg.metadataPath, seg.contentPath, sceneDoc_.bodyText(idx)});
        dirtyScenes_.remove(seg.sceneID);   // ⛔ never also written by the autosave on the GUI thread
    }
    if (dirtyScenes_.isEmpty()) {
        saveTimer_->stop();
    }
    if (jobs.isEmpty()) {
        landed();
        return;
    }
    if (history_ != nullptr) {
        history_->beforeSave();   // an autosave never seals the session unless the writer is idle (T-0396)
    }
    beginBackgroundWork();
    DiagnosticLog::note(QStringLiteral("save off main: %1 scene(s), worker started").arg(jobs.size()));
    QElapsedTimer saveClock;
    saveClock.start();
    if (progressLabel_ != nullptr) {
        progressLabel_->setText(jobs.size() == 1 ? tr("Saving 1 scene…") : tr("Saving %1 scenes…").arg(jobs.size()));
    }
    const WorkerBridge::Handle bridgeHandle = WorkerBridge::handleFor(bridge_);   // ✅ [I-0309]: never `bridge_` itself
    const QString projectID = projectID_;
    const QString path = projectPath_;
    const QString support = appSupportRoot_;
    AsyncCall::run<QVector<bool>>(
        this,
        // ⚠️ The WORKER: the C ABI only. Body-only saves (0/0/0.0): the writing surface is the caret scene's, kept by
        // the next ordinary save.
        [jobs, bridgeHandle, projectID, path, support]() {
            WorkerBridge bridge(bridgeHandle);
            QVector<bool> ok;
            ok.reserve(jobs.size());
            for (const Job& j : jobs) {
                const QVariantMap r = bridge->saveScene(projectID, path, support, j.sceneID, j.metadataPath, j.contentPath,
                                                        j.text, 0, 0, 0.0);
                ok.append(!r.isEmpty() && r.value(QStringLiteral("saved")).toBool());
            }
            return ok;
        },
        [this, jobs, landed, saveClock](const QVector<bool>& ok) {
            DiagnosticLog::note(QStringLiteral("save off main: %1 scene(s) landed in %2 ms (%3 failed)")
                                    .arg(jobs.size()).arg(saveClock.elapsed()).arg(ok.count(false)));
            for (qsizetype k = 0; k < jobs.size(); ++k) {
                const Job& j = jobs.at(k);
                const int idx = sceneDoc_.sceneIndexForScene(j.sceneID);
                if (!ok.at(k)) {
                    if (idx >= 0) {
                        dirtyScenes_.insert(j.sceneID);   // ⛔ [I-0304]: a failed write stays dirty; the next save retries
                    }
                    continue;
                }
                notePersisted(j.sceneID, j.text);   // [I-0104] / [I-0259]: only after a write that succeeded
                if (idx >= 0 && sceneDoc_.bodyText(idx) != j.text) {
                    dirtyScenes_.insert(j.sceneID);     // changed meanwhile: the next save writes it
                    saveTimer_->start();
                }
            }
            landed();
            endBackgroundWork();
        },
        [this, jobs, landed] {
            DiagnosticLog::note(QStringLiteral("save off main: TIMED OUT (%1 scene(s))").arg(jobs.size()));
            // ⛔ [I-0304]: the writes did not report back, so as far as anyone knows they failed — kept dirty.
            for (const Job& j : jobs) {
                if (sceneDoc_.sceneIndexForScene(j.sceneID) >= 0) {
                    dirtyScenes_.insert(j.sceneID);
                }
            }
            errorLabel_->setText(tr("Saving is taking longer than expected — the project may be on a disconnected or "
                                    "unreachable volume. Reopen it before editing further."));
            errorLabel_->show();
            landed();
            endBackgroundWork();
        },
        kProjectOpenTimeoutMs);
}

bool EditorShell::placeCaret(const QString& sceneID, qint64 byteOffset)
{
    const int idx = sceneDoc_.sceneIndexForScene(sceneID);
    if (idx < 0) {
        return false;
    }
    const QString text = sceneDoc_.bodyText(idx);
    const int chars = int(QString::fromUtf8(text.toUtf8().left(int(byteOffset))).size());
    activeSegment_ = -1;
    QTextCursor c = viewport_->textCursor();
    c.setPosition(sceneDoc_.segments().at(idx).bodyStart + std::min(chars, int(text.size())));
    viewport_->setTextCursor(c);
    viewport_->ensureCursorVisible();
    selectNavigatorScene(sceneID);
    viewport_->setFocus();
    return true;
}

int EditorShell::replaceSceneBody(int idx, const QString& text, int caretUtf16)
{
    const SceneSegment seg = sceneDoc_.segments().at(idx);   // copy: the edit below moves the map
    QTextCursor c(sceneDoc_.document());
    c.beginEditBlock();
    c.setPosition(seg.bodyStart);
    c.setPosition(seg.bodyStart + seg.bodyLength, QTextCursor::KeepAnchor);
    // ⛔ Raw — never the editor's `replaceRange`, which would re-balance the restored text into something else.
    c.insertText(text, QTextCharFormat());   // a '\n' becomes a block break, as typing's does
    c.endEditBlock();
    const SceneSegment& now = sceneDoc_.segments().at(idx);
    return now.bodyStart + std::clamp(caretUtf16, 0, now.bodyLength);
}

void EditorShell::applyHistoryStep(const QVariantMap& step, bool isUndo)
{
    const Span span("apply history step");
    if (history_ == nullptr || step.isEmpty()) {
        return;
    }

    // A structural barrier (§4.5): the notice, and no text change.
    const QVariantMap barrier = step.value(QStringLiteral("stoppedAtBarrier")).toMap();
    if (!barrier.isEmpty()) {
        const QString note = barrier.value(QStringLiteral("note")).toString();
        historyNotice(this, note.isEmpty() ? tr("Can't undo past this point.") : note);
        return;
    }

    // ✅ A reversible structural node — ✅ [SP-191] every split, merge and cross-scene paste, and the old nodes: its inverse
    // range call runs off the GUI thread and the delta is applied (no reload).
    const QVariantMap inverse = step.value(QStringLiteral("structuralInverse")).toMap();
    if (!inverse.isEmpty()) {
        applyStructuralInverse(inverse, isUndo);
        return;
    }

    // ✅ [SP-178] Q3: `forkAhead` (the branch chooser) is S3's — redo follows the primary branch meanwhile.
    const QVariantList changes = step.value(QStringLiteral("changes")).toList();
    if (!step.value(QStringLiteral("moved")).toBool() || changes.isEmpty()) {
        return;
    }

    // ⚠️ [I-0270] A step can carry SEVERAL scene changes (an edit across scene breaks) — apply every one.
    viewport_->removePendingPair();   // an undo rewrites scene text; the document-only pair goes first
    struct Placed { int seg; int pos; QString sceneID; QString text; };
    QList<Placed> placed;
    history_->setApplying(true);
    for (const QVariant& v : changes) {
        const QVariantMap change = v.toMap();
        const QString sceneID = change.value(QStringLiteral("sceneID")).toString();
        const int idx = sceneDoc_.sceneIndexForScene(sceneID);
        if (idx < 0) {
            continue;   // the scene is not in this manuscript
        }
        const QString text = change.value(QStringLiteral("newText")).toString();
        const int caret = change.value(QStringLiteral("cursorAfterUtf16"), text.size()).toInt();
        placed.append({idx, replaceSceneBody(idx, text, caret), sceneID, text});
        historyText_.insert(sceneID, text);
    }
    // Saved at once, as Apple's `applySceneChange` (each write reported to history by `saveScene`).
    for (const Placed& p : std::as_const(placed)) {
        saveScene(p.seg);
    }
    // The caret: one change places its own; several place it ONCE, in the EARLIEST scene ([SP-164]).
    if (!placed.isEmpty()) {
        const Placed first = *std::min_element(placed.cbegin(), placed.cend(),
                                               [](const Placed& a, const Placed& b) { return a.seg < b.seg; });
        QTextCursor c = viewport_->textCursor();
        c.setPosition(first.pos);
        viewport_->setTextCursor(c);
        viewport_->ensureCursorVisible();
        history_->setApplying(false);
        // The restored text is now the whitespace reference (SP-177: told to the core).
        history_->noteApplied(first.sceneID, first.text);
    }
    history_->setApplying(false);
}

bool EditorShell::confirmEarlierSessionUndo(const QString& timestamp)
{
    const QString when = HistoryCapture::friendlyTimestamp(timestamp);
    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setObjectName(QStringLiteral("historyNotice"));
    box.setText(tr("Undo a change from a previous session?"));
    box.setInformativeText(when.isEmpty() ? tr("This undo changes text you wrote before this session.")
                                          : tr("This undo changes text you wrote %1.").arg(when));
    QPushButton* undoButton = box.addButton(tr("Undo"), QMessageBox::AcceptRole);
    QPushButton* cancel = box.addButton(QMessageBox::Cancel);
    box.setDefaultButton(undoButton);
    box.setEscapeButton(cancel);
    box.exec();
    return box.clickedButton() != cancel;   // a programmatic accept() (the smokes' catcher) clicks nothing: Undo
}

// --- EP-024 Scene Inspector visibility (SP-078, T-0319) -------------------

void EditorShell::manageWorlds()
{
    // ⚠️ Guarded rather than assumed: the menu action is in editorOnlyActions_ and
    // so is disabled without a project, but a public method must not rely on its
    // only current caller's discipline.
    if (bridge_ == nullptr || projectPath_.isEmpty()) {
        return;
    }

    WorldsDialog dlg(bridge_, projectPath_, this);

    // ⚠️ THE I-0130 WIRING, and it is the reason this lives on the shell.
    //
    // Apple shipped the defect this prevents: a relink repaired the world while
    // the project-wide warning kept insisting it was missing until the writer
    // changed scenes. The dialog reloading its own list is NOT enough — the Scene
    // Inspector is a different widget reading the same core state.
    //
    // ⚠️ inspector_->reload(), NOT setScene(): setScene() early-returns when the
    // sceneID has not changed (SceneInspector.cpp:319), which after a relink is
    // exactly the case — the scene is the same, only the world's status moved.
    // Routing through setScene here would be a silent no-op and would look like
    // the relink had failed.
    connect(&dlg, &WorldsDialog::worldsChanged, this, [this]() {
        if (inspector_ != nullptr) {
            inspector_->reload();
        }
    });

    dlg.exec();
}

// ⚠️ [I-0256] — the Scene Navigator's show/hide. ✅ USER-RULED MECHANISM.
//
// ⚠️ The user, 2026-09-29: *"You cannot hide the navigator, but you can set its
// slider width to 0. That will be adequate. On macOS the mechanism is similar, you
// are just sliding the navigator away so it doesn't show by operating its button."*
//
// ✅ AND THE SPLITTER ALREADY ALLOWS IT, which is why this is a menu item rather
// than a mechanism: ⚠️ `setCollapsible(2, false)` is applied to the INSPECTOR ONLY
// (index 2), ⛔ so the navigator at index 0 has been collapsible all along.
// ✅ `setVisible(false)` collapses the pane to zero width inside the splitter and
// the viewport (stretch=1) reclaims it — ⚠️ the SAME call `setInspectorVisible`
// already makes, so the two panes behave identically.
//
// ✅ PERSISTED ([I-0255], 2026-10-03, user-directed): the writer's choice is stored
// in `inspector-layout.json` as `navigatorHidden`, the same key Apple writes. ⛔ This
// comment used to say the navigator was deliberately session-scoped because Apple's
// `columnVisibility` was in-memory — ✅ Apple now persists it too.
// ⚠️ Only the MENU toggle is recorded here. A splitter drag to zero width is a
// SIZE, which [EP-043]'s session already restores with the splitter geometry.
void EditorShell::setNavigatorVisible(bool visible)
{
    if (navigator_ == nullptr) {
        return;
    }
    navigator_->setVisible(visible);
    session_.setNavigatorVisible(visible);
    if (!restoringPaneVisibility_ && inspector_ != nullptr) {
        inspector_->setStoredNavigatorHidden(!visible);
    }
}

bool EditorShell::isNavigatorVisible() const
{
    return navigator_ != nullptr && navigator_->isVisible();
}

void EditorShell::setInspectorVisible(bool visible)
{
    if (inspector_ == nullptr) {
        return;
    }
    // setVisible() on the pane collapses/restores it inside the splitter; the
    // viewport (stretch=1) reclaims the freed width, so there is no dead space
    // when hidden.
    inspector_->setVisible(visible);

    // ⚠️ SP-145 / T-0553 ([I-0251]) — PER-PROJECT STATE, AND IT PERSISTS.
    //
    // ⛔ THIS USED TO BE SESSION-SCOPED with the comment "no on-disk persistence
    // (Apple parity)" — ⚠️ and that parity claim was WRONG. Apple's
    // `ProjectSession.inspectorVisible` writes straight through to
    // `inspector-layout.json` to satisfy Doc 2 AC4; ⛔ Linux read the key's default
    // and never wrote it back, so the writer's choice died at every quit.
    //
    // ✅ The value is recorded on the session (the project owns it) and written
    // THROUGH THE CORE by the panel that owns the document.
    session_.setInspectorVisible(visible);
    if (!restoringPaneVisibility_) {
        // ⚠️ Guarded: while RESTORING at load we must not write back the value we
        // just read. ⛔ Without this, opening a project rewrites its layout file on
        // every open — the same trap `setSelectedTab` avoids with a signal blocker.
        inspector_->setStoredInspectorHidden(!visible);
    }
}

bool EditorShell::isInspectorVisible() const
{
    // ⚠️ Still answered by the WIDGET, deliberately. ✅ The widget is the truth about
    // what the writer can see; the session records the INTENT. ⛔ They can differ
    // legitimately for one moment — before a project is loaded there is no pane to
    // show — and the menu check-state must follow what is actually on screen.
    return inspector_ != nullptr && inspector_->isVisible();
}

// --- EP-025 Timeline visibility + reload (SP-079) -------------------------

void EditorShell::setTimelineVisible(bool visible)
{
    if (timeline_ == nullptr) {
        return;
    }
    // Collapse/restore the bottom pane; the panels above (stretch=1) reclaim the
    // height when hidden.
    timeline_->setVisible(visible);

    // ✅ [I-0255] — PERSISTED, exactly as the inspector is: ⛔ the [R-Q4] "session-
    // scoped, Apple does not persist it either" asymmetry is SUPERSEDED (user ruling
    // 2026-09-29; Apple persists `timelineHidden` as of 2026-10-03).
    session_.setTimelineVisible(visible);
    if (!restoringPaneVisibility_ && inspector_ != nullptr) {
        // ⚠️ Guarded for the same reason as the inspector: no write-back while restoring.
        inspector_->setStoredTimelineHidden(!visible);
    }
}

bool EditorShell::isTimelineVisible() const
{
    return timeline_ != nullptr && timeline_->isVisible();
}

void EditorShell::pickStoryStructure()
{
    // Ensure the timeline is visible so the writer sees the bands they just chose.
    if (timeline_ != nullptr && !timeline_->isVisible()) {
        setTimelineVisible(true);
    }
    chooseStoryStructure(QCursor::pos());
}

void EditorShell::reloadTimeline()
{
    if (timeline_ == nullptr || loading_) {
        return;   // load() calls this once at the end, past the loading_ guard
    }
    ++timelineUiReads_;
    applyTimelineData(fetchTimelineData(bridge_, projectPath_));
}

// SP-173 / I-0285 (D3): ONE core call for everything the timeline reads, safe on any thread (the bridge keeps only per-thread
// state across a call). ⚠️ The empty-array traps: `storyTimes` is OMITTED when no scene has an explicit story time (the
// common case) — absence means "every scene on the default chain", never failure.
EditorShell::TimelineData EditorShell::fetchTimelineData(ScriviBridge* bridge, const QString& projectRootPath)
{
    TimelineData d;
    const QVariantMap all = bridge->loadTimeline(projectRootPath);
    if (bridge->lastCallFailed()) {
        return d;   // fetched = false: the caller draws nothing new
    }
    d.fetched  = true;
    d.revision = bridge->lastRevision();
    d.epochLabel = all.value(QStringLiteral("timeline")).toMap().value(QStringLiteral("epochLabel")).toString();
    d.storyTimes = storyTimesOf(all.value(QStringLiteral("storyTimes")).toMap());
    d.eventsJSON = all.value(QStringLiteral("historicalEvents")).toMap().value(QStringLiteral("eventsJSON")).toString();
    const QVariantMap ss = all.value(QStringLiteral("storyStructure")).toMap();
    d.hasStructure   = ss.value(QStringLiteral("hasStructure")).toBool();
    d.structureID    = ss.value(QStringLiteral("structureID")).toString();
    d.bandLayoutJSON = ss.value(QStringLiteral("bandLayoutJSON")).toString();
    d.imported = all.value(QStringLiteral("importedTimelines")).toMap();
    return d;
}

QHash<QString, EditorShell::TimelineData::StoryTime> EditorShell::storyTimesOf(const QVariantMap& part)
{
    QHash<QString, TimelineData::StoryTime> out;
    for (const QVariant& v : part.value(QStringLiteral("storyTimes")).toList()) {
        const QVariantMap st = v.toMap();
        TimelineData::StoryTime t;
        t.gapMs      = st.value(QStringLiteral("gapMs")).toLongLong();
        t.durationMs = st.value(QStringLiteral("durationMs")).toLongLong();
        t.bandID     = st.value(QStringLiteral("bandID")).toString();
        out.insert(st.value(QStringLiteral("sceneID")).toString(), t);
    }
    return out;
}

// ⚠️ UI THREAD ONLY. Builds the panel from data already read — no core call.
void EditorShell::applyTimelineData(const TimelineData& data)
{
    if (timeline_ == nullptr || !data.fetched) {
        return;
    }
    ++timelineBuilds_;
    timelineData_ = data;

    // Epoch label from the timeline meta (falls back to "Story Open" inside the panel).
    const QString epochLabel = data.epochLabel;

    // Build the dots in manuscript order, computing each dot's story-time offset via
    // the default gap chain (mirrors Apple's TimelineViewModel.recomputeAllOffsets):
    //   offset[0] = gap[0]; offset[i] = (offset[i-1] + duration[i-1]) + gap[i].
    // gapMs + durationMs come from the explicit story times (SP-173: one call for all scenes); unset scenes
    // fall back to gap 0 + the 1-hour default duration.
    constexpr qint64 kDefaultDurationMs = 3'600'000;   // 1 hour (project default)
    QList<TimelinePanel::Dot> dots;
    timelineOffsets_.clear();
    timelineDurations_.clear();
    qint64 prevEnd = 0;
    const QList<SceneSegment>& segs = sceneDoc_.segments();
    for (int i = 0; i < segs.size(); ++i) {
        const SceneSegment& seg = segs.at(i);
        const TimelineData::StoryTime st = data.storyTimes.value(seg.sceneID);
        const qint64 gapMs = st.gapMs;
        qint64 durationMs = st.durationMs;
        if (durationMs <= 0) {
            durationMs = kDefaultDurationMs;
        }
        const qint64 offsetMs = (i == 0) ? gapMs : (prevEnd + gapMs);
        prevEnd = offsetMs + durationMs;

        // Cache for the Time Delta Picker (SP-080): previous-scene-end anchor + current
        // duration, without a second backend round-trip.
        timelineOffsets_.insert(seg.sceneID, offsetMs);
        timelineDurations_.insert(seg.sceneID, durationMs);

        TimelinePanel::Dot dot;
        dot.sceneID    = seg.sceneID;
        dot.title      = seg.title.isEmpty() ? tr("Scene %1").arg(i + 1) : seg.title;
        dot.chapterTitle = sceneDoc_.chapterHeadingText(i);
        dot.offsetMs   = offsetMs;
        dot.durationMs = durationMs;
        dots.append(dot);
    }

    timeline_->setTimeline(epochLabel, dots);

    // Historical events (SP-082, T-0341): worldbuilding moments on the project row. The
    // list endpoint returns {count, eventsJSON} where eventsJSON is a STRING wrapping
    // {"events":[…]} (confirmed against the C ABI at planning). Parse it into HistDots.
    // Fed after setTimeline so the panel's window includes them (setHistoricalEvents
    // recomputes the window). Empty on failure — the strip degrades to scenes only.
    QList<TimelinePanel::HistDot> histDots;
    histEvents_.clear();
    const QString eventsJSON = data.eventsJSON;
    if (!eventsJSON.isEmpty()) {
        const QJsonDocument doc = QJsonDocument::fromJson(eventsJSON.toUtf8());
        const QJsonArray arr = doc.object().value(QStringLiteral("events")).toArray();
        for (const QJsonValue& v : arr) {
            const QJsonObject o = v.toObject();
            TimelinePanel::HistDot h;
            h.eventID     = o.value(QStringLiteral("eventID")).toString();
            h.title       = o.value(QStringLiteral("title")).toString();
            h.description = o.value(QStringLiteral("description")).toString();
            h.offsetMs    = static_cast<qint64>(
                o.value(QStringLiteral("offsetMs")).toDouble());
            // ✅ T-0542 / [I-0241]: tags come FROM THE PROJECTION now (the core gained
            // them in the same Task). ⛔ They were previously re-read off disk per
            // lookup — see the tombstone where that function used to live.
            QStringList tags;
            for (const QJsonValue& t : o.value(QStringLiteral("tags")).toArray()) {
                tags.append(t.toString());
            }
            histDots.append(h);
            histEvents_.insert(h.eventID,
                               {h.title, h.description, h.offsetMs, tags});
        }
    }
    timeline_->setHistoricalEvents(histDots);

    // Imported timelines (SP-082, T-0342): grey rows below the project row.
    reloadImportedTimelines(data.imported);

    // SP-083 T-0338: restore the persisted zoom/pan for this project (default = full-fit).
    // Read from the app-support INI (same path onTimelineViewStateChanged writes). Applied
    // after setTimeline (which reset the window) via setViewState, which does NOT re-emit
    // viewStateChanged, so restoring never triggers a redundant save.
    const QString viewStatePath = timelineViewStatePath();
    if (!projectID_.isEmpty() && !viewStatePath.isEmpty()) {
        QSettings settings(viewStatePath, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("timeline/") + projectID_);
        const double savedZoom = settings.value(QStringLiteral("zoom"), 1.0).toDouble();
        const double savedPan  = settings.value(QStringLiteral("pan"),  0.0).toDouble();
        settings.endGroup();
        timeline_->setViewState(savedZoom, savedPan);
    }

    // Story structure (SP-081): load the current structure + band assignments and push
    // them to the panel. Empty structure → no bands. bandID per scene comes from the same
    // explicit story times the dots used (SP-173: it was a second per-scene call).
    QList<TimelinePanel::Band> panelBands;
    if (data.hasStructure) {
        currentStructureID_    = data.structureID;
        currentBandLayoutJSON_ = data.bandLayoutJSON;
        for (const story::Band& b : story::parseBandLayout(currentBandLayoutJSON_)) {
            panelBands.append({b.bandID, b.label, b.color, b.proportion});
        }
    } else {
        currentStructureID_.clear();
        currentBandLayoutJSON_.clear();
    }
    QHash<QString, QString> sceneBands;
    if (!panelBands.isEmpty()) {
        for (const SceneSegment& seg : segs) {
            const QString bandID = data.storyTimes.value(seg.sceneID).bandID;
            if (!bandID.isEmpty()) {
                sceneBands.insert(seg.sceneID, bandID);
            }
        }
    }
    timeline_->setBands(panelBands, sceneBands);

    // Re-highlight the active scene's dot after the rebuild.
    if (activeSegment_ >= 0 && activeSegment_ < segs.size()) {
        timeline_->setActiveScene(segs.at(activeSegment_).sceneID);
    }
}

// --- EP-025 Timeline drag/picker (SP-080, T-0328) -------------------------

void EditorShell::onDotDragged(const QString& sceneID, qint64 newOffsetMs)
{
    // The drag proposes a new absolute offset; the picker opens seeded with it.
    showTimeDeltaPicker(sceneID, newOffsetMs);
}

QString EditorShell::writerFacingError(int code, const QString& message) const
{
    // ⚠️ ONLY the unavailable-world case is rewritten. It is the one a writer
    // meets routinely (a world on a drive that is not plugged in), and the core's
    // phrasing names a UUID she has no way to interpret.
    //
    // ⚠️ Matched on the message TEXT rather than the code, because the code is
    // generic. That is fragile if the core rewords, and it is deliberately
    // fail-safe: an unmatched message falls through to the raw text, so a
    // reworded core loses the friendly phrasing but never HIDES the error.
    // ⚠️ T-0478 widened this. `WorldStatus::offline` had never been PRODUCED by
    // anything until SP-124, so matching only "is unavailable" was complete when
    // written. It stopped being complete the moment the core learned to say
    // "offline" — a world on a dead network share would have fallen through to
    // the raw "world 'world_character_01a0…' is offline", which is exactly the
    // UUID-shaped message this function exists to prevent.
    //
    // ⚠️ The message is built from `worldStatusName`, so these two strings are
    // the core's vocabulary for "not usable but not deleted". `missing` is
    // deliberately NOT here: it means the package was positively established
    // absent, which is a different thing to tell a writer.
    const bool unusableWorld = message.contains(QLatin1String("is unavailable"))
                            || message.contains(QLatin1String("is offline"));
    if (unusableWorld && message.contains(QLatin1String("world"))) {
        // Recover the world's display name from the binding cache when we can —
        // ⚠️ §7.2 requires the world to be NAMED, not anonymously warned about.
        //
        // ⚠️ I-0193: this used to call `bridge_->listWorlds(projectPath_)` HERE,
        // synchronously, on the UI thread. ⚠️ That is the single worst place in
        // the app to do it: this branch runs ONLY when a world is already known
        // unusable, so the call was GUARANTEED to hit the dead volume every time
        // it executed -- blocking ~102 s (measured on the rig, 2026-09-08) while
        // composing the sentence that explains that very failure. ⚠️ The error
        // handler froze the app to explain the error.
        //
        // ✅ The name never needed the volume. It is cached from every successful
        // listWorlds and kept in `worldNames_`, which is local memory. ⚠️ A miss
        // is HARMLESS by construction -- every branch below already has a
        // `name.isEmpty()` form, so an unknown world degrades to the anonymous
        // phrasing instead of blocking.
        QString name;
        for (auto it = worldNames_.cbegin(); it != worldNames_.cend(); ++it) {
            if (message.contains(it.key())) {
                name = it.value();
                break;
            }
        }
        // ⚠️ T-0478: `offline` is SHARPER than `unavailable` and the writer is
        // told which. "Offline" points at the network; "unavailable" stays
        // deliberately vague because the core genuinely does not know why.
        // ⚠️ Saying "offline" for an unplugged USB drive would be a new wrong
        // answer, which is why the core only emits it on a positive errno.
        const bool offline = message.contains(QLatin1String("is offline"));

        // ⚠️ Never implies the links are gone — they are HELD, which is the
        // guarantee the writer most needs to hear.
        if (offline) {
            return name.isEmpty()
                       ? tr("That object's world is offline — the server sharing "
                            "it can't be reached. Its links are held, and nothing "
                            "has been lost.")
                       : tr("World “%1” is offline — the server sharing it can't "
                            "be reached. Its links are held, and nothing has been "
                            "lost.").arg(name);
        }
        return name.isEmpty()
                   ? tr("That object's world is unavailable — its links are held, "
                        "and nothing has been lost.")
                   : tr("World “%1” is unavailable — its links are held, and "
                        "nothing has been lost.").arg(name);
    }
    // ⚠️ Everything else passes through verbatim. An unfamiliar error must stay
    // legible to whoever has to diagnose it.
    return tr("Error %1: %2").arg(code).arg(message);
}

void EditorShell::onOpenObjectRequested(const QString& objectKind,
                                       const QString& objectID,
                                       const QString& worldID)
{
    if (objectID.isEmpty()) {
        return;
    }

    // ⚠️ Confirm the object is actually READABLE before claiming anything about
    // it. openObject threads worldID through unchanged — EMPTY for a
    // project-scoped object, the world's ID for a world-scoped one. Omitting it
    // is what blocked object creation outright in SP-104, and this is the first
    // Linux call site that has to get it right.
    const QVariantMap opened =
        bridge_->openObject(projectPath_, objectKind, objectID, worldID);
    if (bridge_->lastCallFailed()) {
        // The bridge already surfaced the reason through errorOccurred; do not
        // paper over it with a second, vaguer message.
        return;
    }

    // ⚠️ The object DETAIL SHEET is EP-036's deliverable, not this sprint's. The
    // affordance is wired end-to-end and the object is genuinely read here — what
    // is missing is the surface to show it in. Say exactly that: a dead
    // double-click reads to a writer as a broken app, and inventing a partial
    // sheet here would be building EP-036 badly and early.
    // ⚠️ The open envelope carries {objectJson, path} only — there is no
    // displayName key at this boundary. The name lives INSIDE the object's own
    // JSON, which is also the form EP-036 will have to patch rather than
    // reconstruct (T-0436/T-0437).
    const QString objectJson = opened.value(QStringLiteral("objectJson")).toString();
    const QString displayName =
        QJsonDocument::fromJson(objectJson.toUtf8())
            .object()
            .value(QStringLiteral("displayName"))
            .toString();
    QMessageBox::information(
        this,
        tr("Open Object"),
        tr("“%1” opened successfully (%2).\n\n"
           "The object detail view is not built yet — it is the next Epic's work. "
           "Nothing about this object has been changed.")
            .arg(displayName.isEmpty() ? objectID : displayName, objectKind));
}

void EditorShell::onSetTimeDeltaRequested(const QString& sceneID)
{
    // Context menu: no drag — seed the picker with the scene's current offset.
    showTimeDeltaPicker(sceneID, timelineOffsets_.value(sceneID, 0));
}

void EditorShell::showTimeDeltaPicker(const QString& sceneID, qint64 seedOffsetMs)
{
    const int idx = sceneDoc_.sceneIndexForScene(sceneID);
    if (idx < 0) {
        return;
    }
    const QList<SceneSegment>& segs = sceneDoc_.segments();
    constexpr qint64 kDefaultDurationMs = 3'600'000;   // 1 hour (project default)

    // Anchor = the previous scene's END (offset + duration); 0 for the first scene, so
    // the delta is measured from the epoch. Uses the cache built in reloadTimeline.
    qint64 previousSceneEndMs = 0;
    QString referenceName;
    if (idx > 0) {
        const QString prevID = segs.at(idx - 1).sceneID;
        previousSceneEndMs = timelineOffsets_.value(prevID, 0)
                             + timelineDurations_.value(prevID, kDefaultDurationMs);
        referenceName = segs.at(idx - 1).title;
    }
    const qint64 currentDurationMs =
        timelineDurations_.value(sceneID, kDefaultDurationMs);

    // Every OTHER scene the writer may anchor to (by its END = offset + duration), in
    // manuscript order, excluding this scene and the immediate predecessor (already
    // offered as the "previous" anchor). Uses the reloadTimeline caches — no backend
    // round-trip. Choosing one is resolved to an absolute offset in the picker (no link).
    QList<TimeDeltaPicker::AnchorScene> otherScenes;
    for (int i = 0; i < segs.size(); ++i) {
        if (i == idx || i == idx - 1) {
            continue;
        }
        const QString sid = segs.at(i).sceneID;
        const qint64 endMs = timelineOffsets_.value(sid, 0)
                             + timelineDurations_.value(sid, kDefaultDurationMs);
        const QString title = segs.at(i).title.isEmpty()
            ? tr("Scene %1").arg(i + 1) : segs.at(i).title;
        otherScenes.append({sid, title, endMs});
    }

    TimeDeltaPicker picker(referenceName, seedOffsetMs, previousSceneEndMs,
                           currentDurationMs, kDefaultDurationMs,
                           QStringLiteral("Story Open"), otherScenes, this);
    if (picker.exec() != QDialog::Accepted) {
        return;   // cancelled — no change
    }

    if (picker.outcome() == TimeDeltaPicker::Outcome::SetOffset) {
        // Manual placement: the canonical stored value is the GAP from the previous
        // scene's end to this scene's start (Apple applyPickerResult).
        const qint64 offsetMs = picker.resultOffsetMs();
        const qint64 gapMs    = offsetMs - previousSceneEndMs;
        bridge_->setSceneStoryTime(projectPath_, sceneID, offsetMs,
                                   QStringLiteral("manual"), gapMs,
                                   picker.resultDurationMs(),
                                   QStringLiteral("manual"));
    } else {   // ResetDefault — return the scene to the gap chain
        bridge_->setSceneStoryTime(projectPath_, sceneID, previousSceneEndMs,
                                   QStringLiteral("default"), /*gapMs=*/0,
                                   picker.resultDurationMs(),
                                   QStringLiteral("default"));
    }

    // Chain propagation (Apple recomputeAndPersistFrom): from the changed scene
    // onward, recompute each scene's absolute offset from the running chain and
    // re-persist it, keeping each scene's OWN stored gapMs. A manual scene's gap is
    // preserved (it just shifts if an earlier scene moved); default scenes keep gap 0.
    qint64 prevEnd = 0;
    for (int i = 0; i < segs.size(); ++i) {
        const QString sid = segs.at(i).sceneID;
        const QVariantMap st = bridge_->getSceneStoryTime(projectPath_, sid);
        const qint64 gapMs = st.value(QStringLiteral("gapMs")).toLongLong();
        qint64 durationMs = st.value(QStringLiteral("durationMs")).toLongLong();
        if (durationMs <= 0) {
            durationMs = kDefaultDurationMs;
        }
        const QString source = st.value(QStringLiteral("offsetSource"),
                                        QStringLiteral("default")).toString();
        const QString durSource = st.value(QStringLiteral("durationSource"),
                                           QStringLiteral("default")).toString();
        const qint64 offsetMs = (i == 0) ? gapMs : (prevEnd + gapMs);
        prevEnd = offsetMs + durationMs;

        // Re-persist only from the changed scene onward (earlier scenes are unaffected).
        if (i >= idx) {
            bridge_->setSceneStoryTime(projectPath_, sid, offsetMs, source,
                                       gapMs, durationMs, durSource);
        }
    }

    reloadTimeline();   // rebuild the strip from the persisted story-time
}

// --- EP-025 story structure (SP-081, T-0330/0331/0332) --------------------

void EditorShell::chooseStoryStructure(const QPoint& globalPos)
{
    QMenu menu(this);
    // One entry per built-in structure (checked = current), plus "None (Remove)".
    QHash<QAction*, QString> actionToID;
    for (const story::Structure& s : story::builtInStructures()) {
        QAction* a = menu.addAction(s.name);
        a->setCheckable(true);
        a->setChecked(s.structureID == currentStructureID_);
        actionToID.insert(a, s.structureID);
    }
    menu.addSeparator();
    QAction* none = menu.addAction(tr("None (Remove Structure)"));
    none->setCheckable(true);
    none->setChecked(currentStructureID_.isEmpty());

    QAction* chosen = menu.exec(globalPos);
    if (chosen == nullptr) {
        return;
    }
    if (chosen == none) {
        // Remove — offsets + assignments are preserved by the endpoint (AC4).
        bridge_->removeStoryStructure(projectPath_);
    } else if (actionToID.contains(chosen)) {
        const QString structureID = actionToID.value(chosen);
        const story::Structure* s = story::structureForID(structureID);
        if (s != nullptr) {
            bridge_->setStoryStructure(projectPath_, structureID,
                                       story::bandLayoutJSON(s->bands));
        }
    }
    reloadTimeline();
}

void EditorShell::onBandProportionsChanged(const QList<double>& proportions)
{
    // Apply the dragged proportions to the current band layout + persist. The bandIDs/
    // labels/colors are unchanged; only proportions move (they sum to 1.0).
    QList<story::Band> bands = story::parseBandLayout(currentBandLayoutJSON_);
    if (bands.size() != proportions.size()) {
        return;   // stale — a reload will re-sync
    }
    for (int i = 0; i < bands.size(); ++i) {
        bands[i].proportion = proportions.at(i);
    }
    currentBandLayoutJSON_ = story::bandLayoutJSON(bands);
    bridge_->updateBandLayout(projectPath_, currentBandLayoutJSON_);
    reloadTimeline();
}

void EditorShell::onSceneAssignedToBand(const QString& sceneID, const QString& bandID)
{
    bridge_->assignSceneToBand(projectPath_, sceneID, bandID);
    reloadTimeline();
}

void EditorShell::onAssignBandRequested(const QString& sceneID)
{
    // Pop a submenu of the current bands (from the cached layout) at the cursor.
    const QList<story::Band> bands = story::parseBandLayout(currentBandLayoutJSON_);
    if (bands.isEmpty()) {
        return;
    }
    QMenu menu(this);
    QHash<QAction*, QString> actionToBand;
    for (const story::Band& b : bands) {
        QAction* a = menu.addAction(b.label);
        actionToBand.insert(a, b.bandID);
    }
    QAction* chosen = menu.exec(QCursor::pos());
    if (chosen != nullptr && actionToBand.contains(chosen)) {
        bridge_->assignSceneToBand(projectPath_, sceneID, actionToBand.value(chosen));
        reloadTimeline();
    }
}

void EditorShell::onUnassignBandRequested(const QString& sceneID)
{
    bridge_->unassignSceneFromBand(projectPath_, sceneID);
    reloadTimeline();
}

// --- EP-025 historical events (SP-082, T-0341) ----------------------------

namespace {
// Serialize tags to the C ABI's expected wrapper: {"tags":["a","b"]} (the same
// object-wrapping-an-array shape the story-structure layout uses — SP-081's lesson).
// Empty tags → "" (the C ABI treats an unparseable/empty tagsJSON as no tags).
QString tagsToJson(const QStringList& tags)
{
    if (tags.isEmpty()) {
        return {};
    }
    QJsonArray arr;
    for (const QString& t : tags) {
        arr.append(t);
    }
    QJsonObject o;
    o.insert(QStringLiteral("tags"), arr);
    return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
}

// ⛔ T-0542 / [I-0241]: `readHistoricalEventTagsFromDisk` WAS HERE and is DELETED.
//
// It listed and parsed EVERY file in `objects/historical-events/` to recover ONE
// field, because `scrivi_list_historical_events` did not project `tags`. ⚠️ Its own
// comment justified itself as "the same read-the-file pattern Apple uses for
// imported-timeline events" — ⛔ and Apple had ALREADY RETIRED that pattern in
// SP-129/T-0502 by extending the core, which is exactly what T-0542 did here.
//
// ⚠️ IT WAS ALSO A DATA-LOSS PATH. `onHistoricalEventDragged` re-read tags only to
// re-send them (updateHistoricalEvent overwrites all fields), and the function
// returned an EMPTY list on every failure — so a read that failed for any reason
// silently erased that event's tags.
//
// ⛔ Do not reintroduce a direct read of objects/historical-events/. Tags arrive in
// the list projection and live in `histEvents_`.
// ⚠️ ENFORCED: `scripts/check-package-boundary.sh` (EP-041 / T-0541) fails CI on a
// `QFile`/`QSaveFile` added here. ⚠️ The two allow-listed sites in this file are the
// QFileDialog export/import ONLY, and they are listed BY LINE, not by file — so a
// package read added anywhere else here is caught.

} // namespace

void EditorShell::onHistoricalEventDragged(const QString& eventID, qint64 newOffsetMs)
{
    // A drag re-times the event. updateHistoricalEvent overwrites ALL fields, so re-send
    // the cached title/description + the disk tags with the new offset (a partial update
    // would blank them). Cache is from reloadTimeline; tags come from disk (list drops them).
    if (!histEvents_.contains(eventID)) {
        return;   // unknown event (stale strip) — ignore rather than blank-write
    }
    const HistEventCache h = histEvents_.value(eventID);
    // ✅ T-0542: tags come from the cache, which the list projection filled.
    // ⚠️ They are STILL re-sent because updateHistoricalEvent overwrites all fields —
    // ⛔ but a cache miss is now impossible for an event that is on the strip, whereas
    // the old disk read returned empty on any failure and blanked them.
    bridge_->updateHistoricalEvent(projectPath_, eventID, h.title, newOffsetMs,
                                   h.description, tagsToJson(h.tags));
    reloadTimeline();
}

void EditorShell::onNewHistoricalEventRequested(qint64 atOffsetMs)
{
    HistoricalEventDialog dlg(tr("New Historical Event"), {}, {}, {}, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    bridge_->createHistoricalEvent(projectPath_, dlg.title(), atOffsetMs,
                                   dlg.description(), tagsToJson(dlg.tags()));
    reloadTimeline();
}

void EditorShell::onEditHistoricalEventRequested(const QString& eventID)
{
    if (!histEvents_.contains(eventID)) {
        return;
    }
    const HistEventCache h = histEvents_.value(eventID);
    // ✅ T-0542: prefilled from the cache (list projection), not from a disk walk.
    HistoricalEventDialog dlg(tr("Edit Historical Event"),
                              h.title, h.description, h.tags, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    // Keep the event's current offset (Edit changes content, not position — position is
    // moved by dragging the dot, §7.7). Re-send the (possibly changed) title/desc/tags.
    bridge_->updateHistoricalEvent(projectPath_, eventID, dlg.title(), h.offsetMs,
                                   dlg.description(), tagsToJson(dlg.tags()));
    reloadTimeline();
}

void EditorShell::onDeleteHistoricalEventRequested(const QString& eventID)
{
    bridge_->deleteHistoricalEvent(projectPath_, eventID);
    reloadTimeline();
}

// --- EP-025 export timeline (SP-082, T-0343) ------------------------------

void EditorShell::onExportTimelineRequested()
{
    // exportProjectTimeline returns {timelineJSON} — the full .scrivi-timeline.json body
    // (scene + historical events; no prose/identity, §6.6/FR-069). Ask for a save
    // location (QFileDialog — proven over VNC, EP-021) and write it.
    const QVariantMap r = bridge_->exportProjectTimeline(projectPath_);
    const QString timelineJSON = r.value(QStringLiteral("timelineJSON")).toString();
    if (timelineJSON.isEmpty()) {
        return;   // export failed (errorOccurred already emitted by the bridge)
    }
    // Default the save location to the folder the project lives in (its .scrivi
    // package's parent), not /root — the writer's projects live together (I-0090).
    const QString startDir = QFileInfo(projectPath_).absolutePath();
    const QString suggested = QDir(startDir)
        .filePath(QStringLiteral("timeline.scrivi-timeline.json"));
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Export Timeline"), suggested,
        tr("Scrivi Timeline (*.scrivi-timeline.json);;All Files (*)"));
    if (path.isEmpty()) {
        return;   // cancelled
    }
    QFile file(path);   // boundary-ok: QFileDialog export, writer-chosen path
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("Export Timeline"),
                             tr("Could not write to:\n%1").arg(path));
        return;
    }
    file.write(timelineJSON.toUtf8());
}

// --- EP-025 imported timelines (SP-082, T-0342) ---------------------------

// SP-129/T-0502: `readImportedTimelineFile` WAS HERE and is DELETED. It existed only
// because the list projection omitted events, and its own comment justified itself as
// "Apple's pattern" — ⚠️ Apple's pattern was a standing-rule violation, now also gone.
// The core projection carries events with `projectOffsetMs` pre-resolved; ⛔ do not
// reintroduce a direct read of objects/imported-timelines/.

void EditorShell::reloadImportedTimelines(const QVariantMap& li)
{
    QList<TimelinePanel::ImportedRow> rows;

    // SP-129/T-0502: metadata AND per-event dots both come from the list endpoint.
    // The projection used to omit events, so this re-opened and re-parsed every stored
    // file the core had already parsed; it now carries `events` with `projectOffsetMs`
    // pre-resolved, so the epoch arithmetic lives in the core instead of here.
    // ✅ SP-173: `li` is the importedTimelines part of the one timeline read (no call of its own).

    // [I-0214] Files the core could not read or parse. ⚠️ They were previously discarded
    // in silence, so an empty panel was indistinguishable from "nothing imported".
    // ⚠️ LOGGED ONLY, DELIBERATELY: Linux has no passive warning strip (Apple uses one,
    // mirroring WorldWarningView), and a QMessageBox here would be MODAL on every single
    // load — worse than the silence it replaces. ⛔ The Linux surface is a KNOWN GAP,
    // recorded in [I-0214]; do not close it with a modal.
    const int rejectedCount = li.value(QStringLiteral("rejectedCount")).toInt();
    if (rejectedCount > 0) {
        // ⚠️ `rejected` is a NESTED ARRAY in the envelope; parseEnvelope's toVariantMap()
        // preserves it as a QVariantList of QVariantMap — NOT as a JSON string.
        qWarning("[I-0214] %d imported timeline file(s) could not be loaded:", rejectedCount);
        for (const QVariant& v : li.value(QStringLiteral("rejected")).toList()) {
            const QVariantMap rm = v.toMap();
            qWarning("  %s — %s",
                     qUtf8Printable(rm.value(QStringLiteral("path")).toString()),
                     qUtf8Printable(rm.value(QStringLiteral("reason")).toString()));
        }
    }

    const QString timelinesJSON = li.value(QStringLiteral("timelinesJSON")).toString();
    if (!timelinesJSON.isEmpty()) {
        const QJsonArray arr = QJsonDocument::fromJson(timelinesJSON.toUtf8())
                                   .object().value(QStringLiteral("timelines")).toArray();
        for (const QJsonValue& v : arr) {
            const QJsonObject meta = v.toObject();
            TimelinePanel::ImportedRow row;
            row.timelineID = meta.value(QStringLiteral("timelineID")).toString();
            row.sourceName = meta.value(QStringLiteral("sourceProjectTitle")).toString();
            row.greyShade  = meta.value(QStringLiteral("assignedGreyShade")).toString();
            row.visible    = meta.value(QStringLiteral("visible")).toBool(true);

            // Per-event dots, straight from the projection. `projectOffsetMs` is
            // already offset + epochOffset — NOT recomputed here.
            for (const QJsonValue& ev : meta.value(QStringLiteral("events")).toArray()) {
                const QJsonObject eo = ev.toObject();
                TimelinePanel::ImportedEvent e;
                e.title = eo.value(QStringLiteral("title")).toString();
                e.projectOffsetMs = static_cast<qint64>(
                    eo.value(QStringLiteral("projectOffsetMs")).toDouble());
                row.events.append(e);
            }
            rows.append(row);
        }
    }
    timeline_->setImportedTimelines(rows);
}

void EditorShell::importTimeline()
{
    // Public trigger for File ▸ Import Timeline… (T-0345). Same flow the panel's
    // empty-area menu offers. Guard: no project → no-op (the menu action is editor-only,
    // but belt-and-suspenders since a shortcut could reach it).
    if (projectPath_.isEmpty() || timeline_ == nullptr) {
        return;
    }
    onImportTimelineRequested();
}

void EditorShell::exportTimeline()
{
    // Public trigger for File ▸ Export Timeline… (T-0345).
    if (projectPath_.isEmpty()) {
        return;
    }
    onExportTimelineRequested();
}

void EditorShell::onImportTimelineRequested()
{
    // Pick a .scrivi-timeline.json (QFileDialog — VNC-proven, EP-021). Start in the
    // folder the project lives in (its .scrivi package's parent), not /root (I-0090).
    const QString startDir = QFileInfo(projectPath_).absolutePath();
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Import Timeline"), startDir,
        tr("Scrivi Timeline (*.scrivi-timeline.json);;All Files (*)"));
    if (path.isEmpty()) {
        return;   // cancelled
    }
    QFile file(path);   // boundary-ok: QFileDialog import, writer-chosen path
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Import Timeline"),
                             tr("Could not read:\n%1").arg(path));
        return;
    }
    const QByteArray bytes = file.readAll();
    const QJsonObject obj = QJsonDocument::fromJson(bytes).object();

    // Feed the epoch-offset dialog the source name/label + the events' own offsets so it
    // can preview the in/out-of-window split against this project's current window.
    const QString sourceName = obj.value(QStringLiteral("sourceProjectTitle")).toString();
    const QString epochLabel = obj.value(QStringLiteral("epochLabel")).toString();
    QList<qint64> eventOffsets;
    for (const QJsonValue& ev : obj.value(QStringLiteral("events")).toArray()) {
        eventOffsets.append(static_cast<qint64>(
            ev.toObject().value(QStringLiteral("offsetMs")).toDouble()));
    }
    qint64 winMin = 0;
    qint64 winMax = 1;
    timeline_->storyTimeWindow(winMin, winMax);

    EpochOffsetDialog dlg(sourceName, epochLabel, eventOffsets, winMin, winMax, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    // Assign a distinct grey shade per source so multiple imports read apart (FR-059).
    // Cycle a small palette by the current import count.
    static const char* kGreyShades[] = {"#8A8A8A", "#A6A6A6", "#6F6F6F", "#BDBDBD"};
    const QVariantMap li = bridge_->listImportedTimelines(projectPath_);
    const int existing = li.value(QStringLiteral("count")).toInt();
    const QString shade = QString::fromLatin1(
        kGreyShades[existing % (sizeof(kGreyShades) / sizeof(kGreyShades[0]))]);

    bridge_->importExternalTimeline(projectPath_, QString::fromUtf8(bytes),
                                    dlg.epochOffsetMs(), shade);
    reloadTimeline();
}

void EditorShell::onEditImportedOffsetRequested(const QString& timelineID)
{
    // SP-129/T-0502: from the list projection, not the stored file. ⚠️ This wants the
    // SOURCE offsets (`offsetMs`), not `projectOffsetMs` — the dialog's whole job is to
    // choose the epoch offset that maps source time onto story time.
    const QVariantMap li = bridge_->listImportedTimelines(projectPath_);
    const QJsonArray all = QJsonDocument::fromJson(
        li.value(QStringLiteral("timelinesJSON")).toString().toUtf8())
        .object().value(QStringLiteral("timelines")).toArray();
    QJsonObject file;
    for (const QJsonValue& v : all) {
        if (v.toObject().value(QStringLiteral("timelineID")).toString() == timelineID) {
            file = v.toObject();
            break;
        }
    }
    if (file.isEmpty()) {
        return;
    }
    const QString sourceName = file.value(QStringLiteral("sourceProjectTitle")).toString();
    const QString epochLabel = file.value(QStringLiteral("epochLabel")).toString();
    QList<qint64> eventOffsets;
    for (const QJsonValue& ev : file.value(QStringLiteral("events")).toArray()) {
        eventOffsets.append(static_cast<qint64>(
            ev.toObject().value(QStringLiteral("offsetMs")).toDouble()));
    }
    qint64 winMin = 0;
    qint64 winMax = 1;
    timeline_->storyTimeWindow(winMin, winMax);

    EpochOffsetDialog dlg(sourceName, epochLabel, eventOffsets, winMin, winMax, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    bridge_->updateImportedTimelineOffset(projectPath_, timelineID, dlg.epochOffsetMs());
    reloadTimeline();
}

void EditorShell::onSetImportedTimelineVisible(const QString& timelineID, bool visible)
{
    bridge_->setImportedTimelineVisible(projectPath_, timelineID, visible);
    reloadTimeline();
}

void EditorShell::onRemoveImportedTimelineRequested(const QString& timelineID)
{
    bridge_->removeImportedTimeline(projectPath_, timelineID);
    reloadTimeline();
}

QString EditorShell::timelineViewStatePath() const
{
    // Store the view state INSIDE the app-support root, not Qt's default ~/.config —
    // app-support is the dir the Docker/VNC harness bind-mounts, so the file survives a
    // `docker run --rm` restart (the earlier default-QSettings path lived in ~/.config,
    // which the container wiped on exit → zoom/pan "not saved"). One INI, keyed per
    // project inside.
    if (appSupportRoot_.isEmpty()) {
        return {};
    }
    return appSupportRoot_ + QStringLiteral("/timeline-view.ini");
}

void EditorShell::onTimelineViewStateChanged(double zoom, double panFraction)
{
    // SP-083 T-0338: persist the timeline zoom/pan per project (Linux-only view state —
    // an INI under app-support, not the backend/.scrivi package). Restored in
    // reloadTimeline. No-op before a project is open / without an app-support root.
    const QString path = timelineViewStatePath();
    if (projectID_.isEmpty() || path.isEmpty()) {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("timeline/") + projectID_);
    settings.setValue(QStringLiteral("zoom"), zoom);
    settings.setValue(QStringLiteral("pan"),  panFraction);
    settings.endGroup();
    settings.sync();   // flush now — a --rm container may SIGKILL before Qt's lazy flush
}
