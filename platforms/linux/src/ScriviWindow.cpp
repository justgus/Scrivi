#include "AppEnvironment.hpp"
#include "ScriviWindow.hpp"

#include "ScriviBuildStamp.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QQuickWidget>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "EditorShell.hpp"

// ---- ShellController --------------------------------------------------------

ShellController::ShellController(ScriviWindow* window, QString appSupportRoot,
                                 AppEnvironment* env)
    : QObject(window), window_(window), appSupportRoot_(std::move(appSupportRoot)),
      env_(env)
{
}

void ShellController::openEditor(const QString& projectPath, const QString& title,
                                 const QVariantMap& openedProject)
{
    // ⚠️ [SP-146] T-0560 — R3 IS CHECKED HERE, AND THIS IS THE RIGHT PLACE.
    //
    // ✅ This is the SINGLE FUNNEL through which a project becomes a window: the
    // landing's Open button, a recents click and the New Project flow ALL reach
    // `shell.openEditor(...)` (`Landing.qml:121`, `:413`). ⛔ Putting the check
    // deeper (in `showEditor`) would be too late — the window would already be the
    // one being reused; putting it in QML would put policy in the view.
    //
    // ⚠️ THE ANSWER COMES FROM THE REGISTRY, NOT THE WINDOW LIST — ✅ that is the
    // whole reason [EP-018] made an app-side registry authoritative: macOS 26's own
    // `WindowGroup(for:)` de-duplication was NOT race-safe (T-0191), and Qt offers
    // no de-duplication at all.
    //
    // ⛔ WE CANNOT ASK BY PATH. ⚠️ `projectID` is the identity ([R-Q2]) and the
    // landing's envelope is where it first appears — ✅ so the check uses the id
    // from `openedProject` when the caller has one. ⚠️ When it does not (an empty
    // envelope means "the editor opens it itself"), there is no identity to compare
    // yet and the open proceeds; ✅ the session registration in `EditorShell` still
    // keeps the registry honest.
    if (window_ == nullptr) {
        return;
    }

    if (env_ != nullptr) {
        const QString projectID = openedProject.value(QStringLiteral("projectID")).toString();
        if (ScriviWindow* existing = env_->existingWindowFor(projectID)) {
            // ✅ ALREADY OPEN — raise it instead of opening a second copy.
            existing->raiseToFront();
            return;
        }
    }

    window_->showEditor(projectPath, title, openedProject);
}

// ---- ScriviWindow -----------------------------------------------------------

ScriviWindow::ScriviWindow(QQuickWidget* landing, QString appSupportRoot,
                           AppEnvironment* env)
    : landing_(landing), appSupportRoot_(std::move(appSupportRoot)), env_(env)
{
    setWindowTitle(QStringLiteral("Scrivi — Linux (alpha)"));
    // ⚠️ 1220×760 (user ruling 2026-08-30): 240 navigator + 580 manuscript + 400
    // inspector. ⚠️ Widened from 1020 in the SAME step the inspector doubled, so
    // the extra panel width comes out of the WINDOW rather than out of the
    // writing surface.
    resize(1220, 760);

    stack_ = new QStackedWidget(this);
    // The QQuickWidget resizes with the view so the QML fills the window.
    landing_->setResizeMode(QQuickWidget::SizeRootObjectToView);
    stack_->addWidget(landing_);   // page 0 — landing
    setCentralWidget(stack_);

    buildMenuBar();
    updateMenuState(/*editorActive=*/false);   // start on the landing page
}

void ScriviWindow::buildMenuBar()
{
    QMenuBar* bar = menuBar();

    // --- File -------------------------------------------------------------
    // New/Open/Close all return to the landing page, which hosts the full New/Open UI
    // and the ScriviBridge. Quit goes through the app so the flush-on-quit hook fires.
    //
    // DATA-SAFETY INVARIANT: edits must never be lost through any writer action. Every
    // path that LEAVES an open editor — Close, New, Open — flushes pending edits FIRST
    // (flushEditor(), safe when nothing is open). Quit is already covered by the
    // aboutToQuit → flushEditor hook in main().
    //
    // ⚠️ SP-144: flushEditor() is now saveDirtyScenes() + stampWritingSurface() —
    // edits AND the writer's place, since [I-0234] removed openScene's implicit
    // per-read surface stamp.
    QMenu* file = bar->addMenu(tr("&File"));
    QAction* newProj = file->addAction(tr("New Project…"));
    newProj->setShortcut(QKeySequence::New);
    connect(newProj, &QAction::triggered, this, [this]() {
        // Return to the landing page, then ask its QML to open the New Project panel.
        // showLanding() alone only shows the landing screen (== Close Project); the New
        // Project flow lives in the landing StackView, reachable only from QML.
        flushEditor();   // never lose edits when leaving the editor
        showLanding();
        if (shell_ != nullptr) {
            shell_->requestNewProject();
        }
    });

    QAction* openProj = file->addAction(tr("Open Project…"));
    openProj->setShortcut(QKeySequence::Open);
    connect(openProj, &QAction::triggered, this, [this]() {
        // Return to landing, then run its Open flow (folder picker + open) — the same as
        // the landing's Open Project button. showLanding() alone would just show the
        // landing screen (== Close Project) without the file dialog.
        flushEditor();   // never lose edits when leaving the editor
        showLanding();
        if (shell_ != nullptr) {
            shell_->requestOpenProject();
        }
    });

    QAction* closeProj = file->addAction(tr("Close Project"));
    // Ctrl+W (QKeySequence::Close on Linux) — the standard "close the current document"
    // gesture writers expect for returning to the landing page. The action is
    // editor-only (disabled on landing), so the shortcut is inert there too.
    closeProj->setShortcut(QKeySequence::Close);
    connect(closeProj, &QAction::triggered, this, [this]() {
        flushEditor();   // never lose edits when leaving the editor
        showLanding();
    });
    editorOnlyActions_.append(closeProj);

    // Timeline import/export (EP-025 / SP-082, T-0345). File operations — importing an
    // external .scrivi-timeline.json and exporting the project timeline — so they live
    // in File alongside Open/Close, not only in the timeline panel's right-click menu.
    // Editor-only (a project must be open); each forwards to the EditorShell trigger
    // (file dialog + bridge). No accelerators (no writer-standard key for them).
    file->addSeparator();
    QAction* importTimeline = file->addAction(tr("Import Timeline…"));
    connect(importTimeline, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->importTimeline(); } });
    editorOnlyActions_.append(importTimeline);

    QAction* exportTimeline = file->addAction(tr("Export Timeline…"));
    connect(exportTimeline, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->exportTimeline(); } });
    editorOnlyActions_.append(exportTimeline);

    file->addSeparator();
    QAction* quit = file->addAction(tr("Quit"));
    quit->setShortcut(QKeySequence::Quit);
    connect(quit, &QAction::triggered, qApp, &QApplication::quit);

    // --- Edit -------------------------------------------------------------
    // Cut/Copy/Paste forward to the focused writing surface (QPlainTextEdit slots).
    QMenu* edit = bar->addMenu(tr("&Edit"));
    QAction* cut = edit->addAction(tr("Cut"));
    cut->setShortcut(QKeySequence::Cut);
    connect(cut, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->cutSelection(); } });
    editorOnlyActions_.append(cut);

    QAction* copy = edit->addAction(tr("Copy"));
    copy->setShortcut(QKeySequence::Copy);
    connect(copy, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->copySelection(); } });
    editorOnlyActions_.append(copy);

    QAction* paste = edit->addAction(tr("Paste"));
    paste->setShortcut(QKeySequence::Paste);
    connect(paste, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->pasteClipboard(); } });
    editorOnlyActions_.append(paste);

    // --- View -------------------------------------------------------------
    // Show Inspector (EP-024 / SP-078, T-0320) — a checkable toggle for the Scene
    // Inspector panel, the Linux analogue of Apple's View ▸ Show Inspector at ⌘⌥I.
    // Ctrl+Alt+I is the Linux equivalent and (like SP-077's Ctrl+W) is not eaten by
    // the macOS→VNC input path. Editor-only; the check-state is synced to the
    // editor's real inspector visibility in updateMenuState().
    QMenu* view = bar->addMenu(tr("&View"));
    showInspectorAction_ = view->addAction(tr("Show Inspector"));
    showInspectorAction_->setCheckable(true);
    showInspectorAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I));
    connect(showInspectorAction_, &QAction::toggled, this, [this](bool on) {
        if (editor_ != nullptr) { editor_->setInspectorVisible(on); }
    });
    editorOnlyActions_.append(showInspectorAction_);

    // Show Timeline (EP-025 / SP-079, T-0323) — the bottom timeline strip. Ctrl+Alt+T,
    // the timeline analogue of the inspector's Ctrl+Alt+I; not macOS→VNC-intercepted.
    showTimelineAction_ = view->addAction(tr("Show Timeline"));
    showTimelineAction_->setCheckable(true);
    showTimelineAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_T));
    connect(showTimelineAction_, &QAction::toggled, this, [this](bool on) {
        if (editor_ != nullptr) { editor_->setTimelineVisible(on); }
    });
    editorOnlyActions_.append(showTimelineAction_);

    // Story Structure… (EP-025 / SP-081, T-0330) — pick a built-in structure (Three Act
    // / Five Act / Hero's Journey / …) or remove it; the timeline paints the bands.
    QAction* storyStructure = view->addAction(tr("Story Structure…"));
    connect(storyStructure, &QAction::triggered, this, [this]() {
        if (editor_ != nullptr) { editor_->pickStoryStructure(); }
    });
    editorOnlyActions_.append(storyStructure);

    // --- Scene ------------------------------------------------------------
    QMenu* scene = bar->addMenu(tr("&Scene"));
    QAction* splitScene = scene->addAction(tr("Split Scene"));
    splitScene->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    connect(splitScene, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->splitScene(); } });
    editorOnlyActions_.append(splitScene);

    QAction* mergeScene = scene->addAction(tr("Merge Scene"));
    mergeScene->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backspace));
    connect(mergeScene, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->mergeScene(); } });
    editorOnlyActions_.append(mergeScene);

    // --- Chapter ----------------------------------------------------------
    // Chapter ▸ Merge is the reason this menu exists: Ctrl+Shift+Backspace is swallowed
    // by the macOS→VNC input path, so the menu action is the reliable trigger.
    QMenu* chapter = bar->addMenu(tr("&Chapter"));
    QAction* splitChapter = chapter->addAction(tr("Split Chapter"));
    splitChapter->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return));
    connect(splitChapter, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->splitChapter(); } });
    editorOnlyActions_.append(splitChapter);

    QAction* mergeChapter = chapter->addAction(tr("Merge Chapter"));
    mergeChapter->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Backspace));
    connect(mergeChapter, &QAction::triggered, this,
            [this]() { if (editor_ != nullptr) { editor_->mergeChapter(); } });
    editorOnlyActions_.append(mergeChapter);

    // --- Project ----------------------------------------------------------
    QMenu* project = bar->addMenu(tr("&Project"));
    QAction* settings = project->addAction(tr("Project Settings…"));
    connect(settings, &QAction::triggered, this, [this]() {
        // T-0312 stub: no settings backend yet. A placeholder dialog so the menu item
        // is real and discoverable; real settings are a future task.
        QDialog dlg(this);
        dlg.setWindowTitle(tr("Project Settings"));
        auto* layout = new QVBoxLayout(&dlg);
        layout->addWidget(new QLabel(
            tr("Project settings are coming soon."), &dlg));
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
        connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        layout->addWidget(buttons);
        dlg.exec();
    });
    editorOnlyActions_.append(settings);

    // --- Project ▸ Manage Worlds… (EP-035 AC3, SP-127 / T-0494) ----------
    //
    // ⚠️ The surface that closes `capability_without_surface` for worlds:
    // addWorld / relinkWorld / getWorldStatus / getWorldBinding were all bridged
    // with ZERO callers, so a project whose world had MOVED could not be repaired
    // from the Linux app at all.
    //
    // ⚠️ Gated on a project being open (editorOnlyActions_) — unlike Help ▸ About,
    // worlds are a property OF a project and the action is meaningless without one.
    QAction* worldsAction = project->addAction(tr("Manage Worlds…"));
    connect(worldsAction, &QAction::triggered, this, [this]() {
        if (editor_ != nullptr) {
            editor_->manageWorlds();
        }
    });
    editorOnlyActions_.append(worldsAction);



    // --- Help ▸ About Scrivi ---------------------------------------------
    //
    // ⚠️ Exists to answer "WHICH BUILD am I running?" from inside the app.
    // ⚠️ On a remote rig that question previously had no answer short of
    // comparing file timestamps over SSH — and on 2026-08-30 the rig was found
    // running a DAY-OLD binary that predated an entire sprint.
    //
    // ⚠️ NOT gated on a project being open (deliberately absent from
    // `editorOnlyActions_`): the most likely moment to ask "is this the right
    // build?" is at the landing screen, before opening anything.
    QMenu* help = bar->addMenu(tr("&Help"));
    QAction* about = help->addAction(tr("About Scrivi"));
    connect(about, &QAction::triggered, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle(tr("About Scrivi"));
        auto* layout = new QVBoxLayout(&dlg);

        auto* name = new QLabel(tr("Scrivi"), &dlg);
        QFont nameFont = name->font();
        nameFont.setBold(true);
        nameFont.setPointSize(nameFont.pointSize() + 4);
        name->setFont(nameFont);
        layout->addWidget(name);

        // ⚠️ From the GENERATED stamp header, refreshed on EVERY build — not
        // __DATE__/__TIME__, which only move when this file recompiles.
        auto* build = new QLabel(
            tr("Build %1\n%2\nQt %3")
                .arg(QString::number(SCRIVI_BUILD_NUMBER),
                     QLatin1String(SCRIVI_BUILD_STAMP),
                     QLatin1String(qVersion())),
            &dlg);
        // Selectable so a tester can copy the stamp into a bug report.
        build->setTextInteractionFlags(Qt::TextSelectableByMouse);
        layout->addWidget(build);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
        connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        layout->addWidget(buttons);
        dlg.exec();
    });
}

void ScriviWindow::updateMenuState(bool editorActive)
{
    for (QAction* a : editorOnlyActions_) {
        a->setEnabled(editorActive);
    }

    // Reflect the editor's real inspector visibility in the View ▸ Show Inspector
    // check-state whenever a project is active (block signals so syncing the box
    // doesn't re-drive setInspectorVisible). No editor → leave it unchecked.
    if (showInspectorAction_ != nullptr) {
        const bool shown = editorActive && editor_ != nullptr
                           && editor_->isInspectorVisible();
        const QSignalBlocker block(showInspectorAction_);
        showInspectorAction_->setChecked(shown);
    }

    // Same for View ▸ Show Timeline (EP-025, T-0323).
    if (showTimelineAction_ != nullptr) {
        const bool shown = editorActive && editor_ != nullptr
                           && editor_->isTimelineVisible();
        const QSignalBlocker block(showTimelineAction_);
        showTimelineAction_->setChecked(shown);
    }
}

void ScriviWindow::showEditor(const QString& projectPath, const QString& title,
                              const QVariantMap& openedProject)
{
    if (editor_ == nullptr) {
        // ⚠️ [SP-146] T-0559 — hand the shell the app-global owner so the
        // registry it registers into is the APP's, not its own.
        editor_ = new EditorShell(this, env_);
        connect(editor_, &EditorShell::closeRequested,
                this, &ScriviWindow::showLanding);
        // ⚠️ T-0499 ([I-0195]): `load()` is ASYNCHRONOUS and no longer returns
        // whether it worked -- the answer is not known when it returns. The view
        // stack switches from HERE instead.
        //
        // ⚠️ NO Qt::UniqueConnection HERE, AND THAT IS THE FIX FOR [I-0198].
        //
        // ⚠️ `UniqueConnection` IS NOT SUPPORTED WITH A LAMBDA. Qt REJECTS the
        // whole connection and says so at runtime:
        //     QObject::connect(EditorShell, ScriviWindow): unique connections
        //     require a pointer to member function of a QObject subclass
        // ⚠️ So the handler was NEVER CONNECTED. The load succeeded, `onDone`
        // fired with ok=1, `loadFinished` was emitted -- and NOTHING WAS
        // LISTENING, so the view stack never switched. ✅ THAT is why a recents
        // click only reordered the list and the editor never appeared.
        //
        // ⚠️ My original comment claimed UniqueConnection prevented stacked
        // handlers. ✅ It is not needed at all: this connect is INSIDE
        // `if (editor_ == nullptr)`, which runs exactly once per window.
        connect(editor_, &EditorShell::loadFinished, this,
                [this](bool ok) {
                    if (!ok) {
                        // ⚠️ [I-0199]: the editor is ALREADY showing (we switch
                        // before loading, so the progress bar is visible), so a
                        // failure must actively RETURN to landing rather than
                        // "stay" there. ⚠️ The editor surfaces its own inline
                        // error before this runs.
                        showLanding();
                        return;
                    }
                    // Already on the editor page; nothing to switch.
                    updateMenuState(/*editorActive=*/true);

                    // ⚠️ [SP-146] T-0560 — REGISTER THE WINDOW under the project it
                    // now shows. ✅ Done HERE, on a SUCCESSFUL load, because this is
                    // the first point where the project's IDENTITY is known —
                    // ⛔ `showEditor()` has only a path, and a path is not an
                    // identity. ⚠️ Same reasoning as the session registration in
                    // `EditorShell` (T-0552), and the same empty-ID guard applies.
                    if (env_ != nullptr) {
                        env_->windows().registerWindow(shownProjectID(), this);
                    }
                });
        stack_->addWidget(editor_);   // page 1 — editor
    }

    // ⚠️ [I-0199] SWITCH TO THE EDITOR **BEFORE** LOADING, NOT AFTER.
    //
    // ⚠️ The progress bar lives INSIDE EditorShell. Switching the stack only in
    // the `loadFinished` handler meant the editor page was HIDDEN for the entire
    // load -- ✅ so the bar dutifully showed itself on a page nobody could see,
    // and was hidden again the instant the page finally appeared.
    // ⚠️ It could never be seen AT ANY PROJECT SIZE.
    //
    // ✅ Showing the editor first is also what the writer expects: the window
    // they asked for appears immediately, with an honest "opening…" instead of a
    // dead landing page. ⚠️ On FAILURE the handler returns to landing (below),
    // so a failed open still lands somewhere sensible.
    stack_->setCurrentWidget(editor_);
    updateMenuState(/*editorActive=*/true);

    // ⚠️ SP-144 ([I-0232] AC5): pass the landing's envelope through so the editor
    // does NOT re-open the project. Empty ⇒ the editor opens it itself (reload).
    editor_->load(projectPath, appSupportRoot_, title, openedProject);
}

void ScriviWindow::showLanding()
{
    // ⚠️ This IS "Close Project" on Linux — the editor page is left behind and a
    // DIFFERENT project may be opened next, in the same process.
    // ✅ EP-039 T-0512: release the core's in-memory index for the outgoing project,
    // or every project opened in a session stays resident. No-op when none is open.
    if (editor_ != nullptr) {
        editor_->releaseProject();
    }
    stack_->setCurrentWidget(landing_);
    updateMenuState(/*editorActive=*/false);
}

void ScriviWindow::flushEditor()
{
    if (editor_ != nullptr) {
        editor_->saveDirtyScenes();
        // ⚠️ SP-144 — then record WHERE THE WRITER WAS, even if nothing was dirty.
        //
        // ✅ Mirrors Apple's `saveAllDirtyBlocking`, which saves the dirty scenes
        // and THEN calls `stampWritingSurface` ([I-0058]/[I-0131]) so a scene the
        // writer scrolled to but never edited still resumes correctly.
        //
        // ⚠️ Linux needs this now because [I-0234] removed `openScene`'s implicit
        // per-read stamp, and Linux's save path only writes DIRTY scenes — so
        // without it, navigating without typing would leave nothing recording the
        // writer's place. ✅ ONE write on teardown, not one per scene on load.
        editor_->stampWritingSurface();
    }
}

void ScriviWindow::closeEvent(QCloseEvent* event)
{
    flushEditor();   // don't lose edits when the window (and app) closes

    // ⚠️ [SP-146] T-0560 — R8: release the project as this window goes away.
    // ✅ `releaseProject()` calls `scrivi_close_project` for THIS project and
    // deregisters its session — ⛔ and only this one. ⚠️ Without it, closing a
    // window would leave the core's index resident and R3 reporting the project as
    // open forever, so it could never be reopened.
    if (editor_ != nullptr) {
        editor_->releaseProject();
    }
    QMainWindow::closeEvent(event);
}

// ⚠️ [SP-146] T-0560 — deregister from the window map BEFORE this window dies.
// ⛔ The manager holds BORROWED pointers, so a window that dies while still mapped
// leaves a dangle that the next R3 check would follow.
ScriviWindow::~ScriviWindow()
{
    if (env_ != nullptr) {
        env_->windows().deregisterWindow(this);
    }
}

QString ScriviWindow::shownProjectID() const
{
    return editor_ != nullptr ? editor_->currentProjectID() : QString();
}

void ScriviWindow::raiseToFront()
{
    // ⚠️ THREE CALLS, AND ALL THREE ARE NEEDED. ✅ `show()` restores a minimised
    // window, `raise()` lifts it in the stacking order, and `activateWindow()`
    // gives it keyboard focus. ⛔ Any one alone leaves a case where the writer
    // asked for a project and nothing visibly happened.
    show();
    raise();
    activateWindow();
}
