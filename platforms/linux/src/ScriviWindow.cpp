#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
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

ShellController::ShellController(ScriviWindow* window, QString appSupportRoot)
    : QObject(window), window_(window), appSupportRoot_(std::move(appSupportRoot))
{
}

void ShellController::openEditor(const QString& projectPath, const QString& title)
{
    if (window_ != nullptr) {
        window_->showEditor(projectPath, title);
    }
}

// ---- ScriviWindow -----------------------------------------------------------

ScriviWindow::ScriviWindow(QQuickWidget* landing, QString appSupportRoot)
    : landing_(landing), appSupportRoot_(std::move(appSupportRoot))
{
    setWindowTitle(QStringLiteral("Scrivi — Linux (alpha)"));
    resize(1020, 760);   // 820×560 + 200 each (user pref 2026-07-22)

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
    // (flushEditor() == editor_->saveDirtyScenes(), safe when nothing is open). Quit is
    // already covered by the aboutToQuit → flushEditor hook in main().
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

void ScriviWindow::showEditor(const QString& projectPath, const QString& title)
{
    if (editor_ == nullptr) {
        editor_ = new EditorShell(this);
        connect(editor_, &EditorShell::closeRequested,
                this, &ScriviWindow::showLanding);
        stack_->addWidget(editor_);   // page 1 — editor
    }

    if (editor_->load(projectPath, appSupportRoot_, title)) {
        stack_->setCurrentWidget(editor_);
        updateMenuState(/*editorActive=*/true);
    }
    // On failure the editor shows its own inline error and we stay on landing.
}

void ScriviWindow::showLanding()
{
    stack_->setCurrentWidget(landing_);
    updateMenuState(/*editorActive=*/false);
}

void ScriviWindow::flushEditor()
{
    if (editor_ != nullptr) {
        editor_->saveDirtyScenes();
    }
}

void ScriviWindow::closeEvent(QCloseEvent* event)
{
    flushEditor();   // don't lose edits when the window (and app) closes
    QMainWindow::closeEvent(event);
}
