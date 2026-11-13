#include "FormatMenu.hpp"
#include "AppEnvironment.hpp"
#include "ScriviWindow.hpp"

#include "ScriviBuildStamp.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QQuickWidget>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "AsyncCall.hpp"
#include "BufferService.hpp"
#include "DiagnosticLog.hpp"
#include "EditorShell.hpp"
#include "AuthorshipDialogs.hpp"
#include "ProjectSettingsDialog.hpp"
#include "ScriviBridge.hpp"

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
    // ⚠️ [SP-146] T-0561 — THE ENVIRONMENT OPENS THE WINDOW NOW, not this controller.
    //
    // ✅ `openProjectWindow()` enforces R3 (raise an already-open project) and
    // otherwise creates a NEW editor-only window ([R-Q3]). ⛔ It no longer swaps the
    // Landing window's own central widget, which is what made a second project
    // impossible.
    if (env_ != nullptr) {
        env_->openProjectWindow(projectPath, title, openedProject);
        return;
    }

    // ⚠️ FALLBACK — no app environment (tests). ✅ Behaves exactly as before.
    if (window_ != nullptr) {
        window_->showEditor(projectPath, title, openedProject);
    }
}

// ---- [SP-206] Import Archive -------------------------------------------------

void ShellController::importArchive(const QString& archivePath)
{
    QWidget* parent = QApplication::activeWindow();
    QString archive = archivePath;
    if (archive.isEmpty()) {
        archive = QFileDialog::getOpenFileName(parent, tr("Import Archive"), QDir::homePath(),
                                               tr("Scrivi Archive (*.scrivarchive)"));
        if (archive.isEmpty()) {
            return;
        }
    }
    // Apple's shape: the name first (prefilled with the archive's), then the folder.
    const QString stem = QFileInfo(archive).completeBaseName();
    bool ok = false;
    const QString name = QInputDialog::getText(parent, tr("Import “%1”").arg(stem),
                                               tr("Name the project, then choose the folder to unpack it into."),
                                               QLineEdit::Normal, stem, &ok);
    if (!ok) {
        return;
    }
    const QString folder = QFileDialog::getExistingDirectory(
        parent, tr("Choose the folder to unpack “%1” into").arg(name),
        QFileInfo(archive).absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folder.isEmpty()) {
        return;
    }
    runImport(archive, folder, {{QStringLiteral("projectFolderName"), name}});
}

void ShellController::runImport(const QString& archivePath, const QString& folder, const QVariantMap& options)
{
    QWidget* parent = QApplication::activeWindow();
    // AC4: off the GUI thread; the progress dialog only once it has run 400 ms.
    auto* progress = new QProgressDialog(tr("Importing archive…"), QString(), 0, 0, parent);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setAttribute(Qt::WA_DeleteOnClose);
    progress->hide();
    QTimer::singleShot(400, progress, [progress]() { progress->show(); });

    AsyncCall::run<QVariantMap>(
        this,
        [archivePath, folder, options]() { return ScriviBridge().importArchive(archivePath, folder, options); },
        [this, archivePath, folder, options, progress](QVariantMap r) {
            progress->close();
            QWidget* parent = QApplication::activeWindow();
            if (r.contains(QStringLiteral("errorMessage"))) {
                if (r.value(QStringLiteral("errorDetail")).toString() == QStringLiteral("projectOpen")) {
                    QMessageBox::warning(parent, tr("Close the project first"),
                        tr("The project in “%1” that Replace would delete is open. Close its window, then import again.")
                            .arg(QFileInfo(folder).fileName()));
                } else {
                    QMessageBox::warning(parent, tr("The archive could not be imported"),
                                         r.value(QStringLiteral("errorMessage")).toString());
                }
                return;
            }
            if (!r.value(QStringLiteral("imported")).toBool()) {
                const QVariantMap c = r.value(QStringLiteral("conflict")).toMap();
                const QString name = c.value(QStringLiteral("folderName")).toString();
                const QString stem = QFileInfo(name).completeBaseName();
                const bool world = c.value(QStringLiteral("kind")).toString() == QStringLiteral("world");
                QMessageBox box(QMessageBox::Question, tr("“%1” already exists").arg(name),
                    world ? tr("“%1” already holds a different world with this name. Rename the imported world?")
                                .arg(QFileInfo(folder).fileName())
                          : tr("“%1” already holds a project with this name. Rename the imported project, or replace "
                               "the one that is there?").arg(QFileInfo(folder).fileName()),
                    QMessageBox::NoButton, parent);
                QPushButton* rename  = box.addButton(tr("Rename…"), QMessageBox::AcceptRole);
                QPushButton* replace = world ? nullptr : box.addButton(tr("Replace"), QMessageBox::DestructiveRole);
                box.addButton(QMessageBox::Cancel);
                box.exec();
                QVariantMap next = options;
                if (box.clickedButton() == rename) {
                    bool ok = false;
                    const QString newName = QInputDialog::getText(parent, tr("Rename"), tr("Folder name:"),
                                                                  QLineEdit::Normal, stem + QStringLiteral(" 2"), &ok);
                    if (!ok) {
                        return;
                    }
                    if (world) {
                        QVariantMap names = next.value(QStringLiteral("worldFolderNames")).toMap();
                        names.insert(c.value(QStringLiteral("worldID")).toString(), newName);
                        next.insert(QStringLiteral("worldFolderNames"), names);
                    } else {
                        next.insert(QStringLiteral("projectFolderName"), newName);
                    }
                } else if (replace != nullptr && box.clickedButton() == replace) {
                    next.insert(QStringLiteral("replaceProject"), true);
                } else {
                    return;   // Cancel: nothing was extracted
                }
                runImport(archivePath, folder, next);
                return;
            }
            const QString projectRoot = r.value(QStringLiteral("projectRoot")).toString();
            // Q1: the open path would raise the window of the copy already open (R3) — say so instead.
            if (env_ != nullptr) {
                if (ScriviWindow* open = env_->existingWindowFor(r.value(QStringLiteral("projectID")).toString())) {
                    const QString openPath = open->editorShell() != nullptr ? open->editorShell()->currentProjectPath()
                                                                            : QString();
                    QMessageBox::information(parent, tr("A copy of this project is already open"),
                        tr("“%1” is open. Close it to open the one you imported, at “%2”.").arg(openPath, projectRoot));
                    return;
                }
                env_->showLanding();
            }
            emit openPathRequested(projectRoot);
        },
        [progress]() { progress->close(); },
        0);   // never abandoned: it writes
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
    // ⚠️ [SP-146] T-0561 — `landing` MAY BE NULL, and that is the new normal.
    // ✅ A window created by `AppEnvironment::openProjectWindow()` is EDITOR-ONLY
    // ([R-Q3]): Landing is its own window now, so a project window has no landing
    // page to stack. ⛔ The first window still receives one, so the single-window
    // path is unchanged until [SP-147] retires it.
    if (landing_ != nullptr) {
        landing_->setResizeMode(QQuickWidget::SizeRootObjectToView);
        stack_->addWidget(landing_);   // page 0 — landing
    }
    setCentralWidget(stack_);

    buildMenuBar(this, this, env_);
    updateMenuState(/*editorActive=*/false);   // start on the landing page
}

void ScriviWindow::buildMenuBar(QMainWindow* host, ScriviWindow* w, AppEnvironment* env)
{
    QMenuBar* bar = host->menuBar();
    // [SP-216] the start window (`w` nullptr) has no project: its project-only items are greyed for good, below.
    QList<QAction*> startWindowProjectOnly;
    QList<QAction*>& editorOnlyActions = w != nullptr ? w->editorOnlyActions_ : startWindowProjectOnly;
    const auto ed = [w]() -> EditorShell* { return w != nullptr ? w->editor_ : nullptr; };
    // Read when used: main() hands the shell over after both windows exist.
    const auto shell = [w, env]() -> ShellController* {
        return w != nullptr ? w->shell_ : env != nullptr ? env->shellController() : nullptr;
    };

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
    connect(newProj, &QAction::triggered, host, [=]() {
        // Return to the landing page, then ask its QML to open the New Project panel.
        // showLanding() alone only shows the landing screen (== Close Project); the New
        // Project flow lives in the landing StackView, reachable only from QML.
        // ⚠️ [SP-146] T-0561 ([R-Q3]) — RAISE LANDING; ⛔ do NOT replace the
        // manuscript in THIS window. ✅ The writer asked to create a DIFFERENT
        // project, not to close this one.
        if (w != nullptr) {
            w->flushEditor();   // never lose edits
        }
        if (env != nullptr) {
            env->showLanding();
        } else if (w != nullptr) {
            w->showLanding();   // single-window fallback (tests)
        }
        if (shell() != nullptr) {
            shell()->requestNewProject();
        }
    });

    QAction* openProj = file->addAction(tr("Open Project…"));
    openProj->setShortcut(QKeySequence::Open);
    connect(openProj, &QAction::triggered, host, [=]() {
        // Return to landing, then run its Open flow (folder picker + open) — the same as
        // the landing's Open Project button. showLanding() alone would just show the
        // landing screen (== Close Project) without the file dialog.
        // ⚠️ [SP-146] T-0561 ([R-Q3]) — RAISE LANDING, same reasoning as New.
        if (w != nullptr) {
            w->flushEditor();   // never lose edits
        }
        if (env != nullptr) {
            env->showLanding();
        } else if (w != nullptr) {
            w->showLanding();   // single-window fallback (tests)
        }
        if (shell() != nullptr) {
            shell()->requestOpenProject();
        }
    });

    QAction* closeProj = file->addAction(tr("Close Project"));
    // Ctrl+W (QKeySequence::Close on Linux) — the standard "close the current document"
    // gesture writers expect for returning to the landing page. The action is
    // editor-only (disabled on landing), so the shortcut is inert there too.
    closeProj->setShortcut(QKeySequence::Close);
    connect(closeProj, &QAction::triggered, host, [=]() {
        if (w != nullptr) {
            w->flushEditor();   // never lose edits when leaving the editor
            w->showLanding();
        }
    });
    editorOnlyActions.append(closeProj);

    file->addSeparator();
    // [SP-215] EP-054: every way in and out, by verb, as on the Mac (`ImportExportCommands`): Copy as HTML, Import ▸,
    // Export ▸. Export is the project in THIS window (SP-206 Q2); Import ▸ Archive… needs no project. The timeline (EP-025 / SP-082, T-0345) is also in the
    // Timeline panel's menu.
    QAction* copyAsHtml = file->addAction(tr("Copy as HTML"));
    connect(copyAsHtml, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { w->flushEditor(); ed()->copyManuscriptAsHTML(); } });
    editorOnlyActions.append(copyAsHtml);

    QMenu* importMenu = file->addMenu(tr("Import"));
    QAction* importTimeline = importMenu->addAction(tr("Timeline…"));
    connect(importTimeline, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->importTimeline(); } });
    editorOnlyActions.append(importTimeline);
    QAction* importArchive = importMenu->addAction(tr("Archive…"));
    connect(importArchive, &QAction::triggered, host,
            [=]() { if (shell() != nullptr) { shell()->importArchive(); } });

    QMenu* exportMenu = file->addMenu(tr("Export"));
    // AC7: the format is the item (the Mac's `ExportMenuItems`) — no format dialog; a chapter has no ePub (SP-213 Q7).
    const auto addFormats = [=](QMenu* menu, bool currentChapter) {
        const QList<QPair<QString, QString>> formats{{tr("Markdown…"), QStringLiteral("markdown")},
                                                     {tr("HTML…"), QStringLiteral("html")},
                                                     {tr("Word Document…"), QStringLiteral("docx")},
                                                     {tr("ePub…"), QStringLiteral("epub")},
                                                     {tr("PDF…"), QStringLiteral("pdf")}};
        for (const auto& [label, format] : formats) {
            if (currentChapter && format == QLatin1String("epub")) {
                continue;
            }
            connect(menu->addAction(label), &QAction::triggered, host, [=]() {
                if (ed() != nullptr) { w->flushEditor(); ed()->exportManuscript(format, currentChapter); }
            });
        }
    };
    QMenu* manuscriptMenu = exportMenu->addMenu(tr("Manuscript"));
    addFormats(manuscriptMenu, false);
    editorOnlyActions.append(manuscriptMenu->menuAction());
    // Q3: the caret's chapter — greyed when there is none, so it is set as the menu opens (not in editorOnlyActions).
    QMenu* chapterMenu = exportMenu->addMenu(tr("Current Chapter"));
    addFormats(chapterMenu, true);
    QAction* exportChapter = chapterMenu->menuAction();
    exportChapter->setObjectName(QStringLiteral("exportCurrentChapterAction"));
    exportChapter->setEnabled(false);
    connect(exportMenu, &QMenu::aboutToShow, host, [=]() {
        exportChapter->setEnabled(ed() != nullptr && w->stack_->currentWidget() == ed() && ed()->caretChapter());
    });
    QAction* exportTimeline = exportMenu->addAction(tr("Timeline…"));
    connect(exportTimeline, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->exportTimeline(); } });
    editorOnlyActions.append(exportTimeline);
    QAction* exportArchive = exportMenu->addAction(tr("Archive…"));
    connect(exportArchive, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { w->flushEditor(); ed()->exportArchive(); } });
    editorOnlyActions.append(exportArchive);

    file->addSeparator();
    QAction* quit = file->addAction(tr("Quit"));
    quit->setShortcut(QKeySequence::Quit);
    // ⛔ [I-0257] — NOT `qApp, &QApplication::quit` ANY MORE.
    // ⚠️ MEASURED: `quit()` exits the event loop and does NOT close windows, so
    // with `WA_DeleteOnClose` project windows they simply stayed on screen.
    // ✅ `quitApplication()` flushes everything, then tears down from the app's own
    // window map. ⚠️ The `env == nullptr` fallback keeps single-window tests working.
    connect(quit, &QAction::triggered, host, [=]() {
        if (env != nullptr) {
            env->quitApplication();
        } else {
            QApplication::quit();
        }
    });

    // --- Edit -------------------------------------------------------------
    // Cut/Copy/Paste forward to the focused writing surface (QPlainTextEdit slots).
    QMenu* edit = bar->addMenu(tr("&Edit"));

    // Edit ▸ Undo / Redo (EP-026 S2, SP-178 / T-0605) — the custom history. ✅ EP-026 Q3: Ctrl+Z / Ctrl+Shift+Z, ⛔ no
    // Ctrl+Y — so explicit sequences, not `QKeySequence::Undo/Redo` (whose X11 Redo includes Ctrl+Y). While the writing
    // surface has focus it claims both keys itself (ShortcutOverride) and signals the same `undo()` / `redo()`; these
    // shortcuts serve the rest of the window. Enabled from the history's can-state each time the menu opens (Apple's
    // `validateUserInterfaceItem`).
    QAction* undo = edit->addAction(tr("Undo"));
    undo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    connect(undo, &QAction::triggered, host, [=]() { if (ed() != nullptr) { ed()->undo(); } });
    editorOnlyActions.append(undo);
    QAction* redo = edit->addAction(tr("Redo"));
    redo->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    connect(redo, &QAction::triggered, host, [=]() { if (ed() != nullptr) { ed()->redo(); } });
    editorOnlyActions.append(redo);
    connect(edit, &QMenu::aboutToShow, host, [=]() {
        const bool editing = ed() != nullptr && w->stack_->currentWidget() == ed();
        undo->setEnabled(editing && ed()->undoAvailable());   // [I-0302]: at a barrier too, to show its notice
        redo->setEnabled(editing && ed()->canRedo());
    });
    // ⚠️ Re-enabled on close so the SHORTCUTS keep working: a disabled action's shortcut never fires, and the can-state
    // changes between menu openings. An undo with nothing to undo does nothing.
    connect(edit, &QMenu::aboutToHide, host, [=]() {
        const bool editing = ed() != nullptr && w->stack_->currentWidget() == ed();
        undo->setEnabled(editing);
        redo->setEnabled(editing);
    });
    edit->addSeparator();

    QAction* cut = edit->addAction(tr("Cut"));
    cut->setShortcut(QKeySequence::Cut);
    connect(cut, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->cutSelection(); } });
    editorOnlyActions.append(cut);

    QAction* copy = edit->addAction(tr("Copy"));
    copy->setShortcut(QKeySequence::Copy);
    connect(copy, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->copySelection(); } });
    editorOnlyActions.append(copy);

    QAction* paste = edit->addAction(tr("Paste"));
    paste->setShortcut(QKeySequence::Paste);
    connect(paste, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->pasteClipboard(); } });
    editorOnlyActions.append(paste);

    // Edit ▸ Copy To / Paste From / Cut To Buffer (EP-026 S5, SP-179 / T-0606) — Apple's three submenus, the discoverable
    // face of the chords. ⛔ No key equivalents on the items: the chords are the EDITOR's (it claims them first); each
    // submenu names its chord in a header, as Apple's do. A click acts on THIS window's project. ✅ Also the route that
    // always works if a remote session swallows a chord (`project_linux_vnc_input_constraints`).
    edit->addSeparator();
    {
        struct BufferMenu { const char* title; const char* header; void (EditorShell::*act)(const QString&); };
        static const BufferMenu menus[] = {
            {QT_TR_NOOP("Copy To Buffer"), QT_TR_NOOP("Ctrl+1–9 — copy the selection"), &EditorShell::copyIntoBuffer},
            {QT_TR_NOOP("Paste From Buffer"), QT_TR_NOOP("Ctrl+Shift+1–9 — paste at the cursor"), &EditorShell::pasteFromBuffer},
            {QT_TR_NOOP("Cut To Buffer"), QT_TR_NOOP("Alt+1–9 — cut the selection"), &EditorShell::cutIntoBuffer},
        };
        for (const BufferMenu& m : menus) {
            QMenu* sub = edit->addMenu(tr(m.title));
            sub->addAction(tr(m.header))->setEnabled(false);
            for (const QString& id : BufferService::slotIDs()) {
                const auto act = m.act;
                connect(sub->addAction(tr("Buffer %1").arg(id)), &QAction::triggered, host, [=]() {
                    if (ed() != nullptr) { (ed()->*act)(id); }
                });
            }
            editorOnlyActions.append(sub->menuAction());
        }
    }

    // Edit ▸ Find (SP-176 / T-0603, EP-048 S4) — Find over the presented text; ⛔ Replace waits for Linux undo (EP-026).
    edit->addSeparator();
    for (QAction* a : addFindMenu(edit, host, [=](FindCommand command) {
             if (ed() != nullptr) { ed()->find(command); }
         })) {
        editorOnlyActions.append(a);
    }

    // --- Format (SP-175 / T-0602, EP-048 S3b) ---------------------------------
    // ✅ [SP-175] Q4: between Edit and View, as Apple ([SP-163]).
    for (QAction* a : addFormatMenu(bar, host, [=](ManuscriptEditor::Format format) {
             if (ed() != nullptr) { ed()->applyFormat(format); }
         })) {
        editorOnlyActions.append(a);
    }

    edit->addSeparator();   // ✅ [SP-211] live pass: Settings… last, below a rule
    // EP-058 S3 ([SP-211] Q1): Edit ▸ Settings… (Ctrl+,) — the writer's settings (Apple: Scrivi ▸ Settings…, ⌘,). ⚠️ NOT in
    // `editorOnlyActions`: pen names belong to the writer, so it stays live on the start window too ([SP-216]).
    QAction* appSettings = edit->addAction(tr("Settings…"));
    appSettings->setObjectName(QStringLiteral("appSettingsAction"));
    appSettings->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    connect(appSettings, &QAction::triggered, host, [=]() {
        SettingsDialog dlg(host);
        dlg.exec();
    });

    // --- View -------------------------------------------------------------
    // Show Inspector (EP-024 / SP-078, T-0320) — a checkable toggle for the Scene
    // Inspector panel, the Linux analogue of Apple's View ▸ Show Inspector at ⌘⌥I.
    // Ctrl+Alt+I is the Linux equivalent and (like SP-077's Ctrl+W) is not eaten by
    // the macOS→VNC input path. Editor-only; the check-state is synced to the
    // editor's real inspector visibility in updateMenuState().

    QMenu* view = bar->addMenu(tr("&View"));

    // ⚠️ [I-0256] — Show Scene Navigator. ⛔ THERE WAS NO CONTROL FOR THIS AT ALL.
    // ✅ Found by the user's live pass 2026-09-29: *"there is no option to hide the
    // Scene Navigator."*
    // ⚠️ The Ctrl+Alt family is deliberate: `project_linux_vnc_input_constraints` records
    // that the macOS→VNC path EATS Ctrl+Shift combos, ⛔ so they cannot be used.
    // ✅ [SP-175] Q1: Ctrl+Alt+S (Apple's sidebar toggle is ⌃⌘S) — Ctrl+Alt+N is Format ▸ Numbered List
    // (Apple's ⌥⌘N, [SP-174] Q4). Measured on the rig: GNOME binds no Ctrl+Alt+S.
    QAction* showNavigator = view->addAction(tr("Show Scene Navigator"));
    showNavigator->setCheckable(true);
    showNavigator->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    connect(showNavigator, &QAction::toggled, host, [=](bool on) {
        if (ed() != nullptr) { ed()->setNavigatorVisible(on); }
    });
    editorOnlyActions.append(showNavigator);

    QAction* showInspector = view->addAction(tr("Show Inspector"));
    showInspector->setCheckable(true);
    showInspector->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I));
    connect(showInspector, &QAction::toggled, host, [=](bool on) {
        if (ed() != nullptr) { ed()->setInspectorVisible(on); }
    });
    editorOnlyActions.append(showInspector);

    // Show Timeline (EP-025 / SP-079, T-0323) — the bottom timeline strip.
    // ✅ [SP-175] Q3: Ctrl+Alt+M. ⛔ It was Ctrl+Alt+T, which GNOME binds to OPEN A TERMINAL (measured on the
    // rig, `gsettings` media-keys `terminal`), so the desktop took the key first.
    QAction* showTimeline = view->addAction(tr("Show Timeline"));
    showTimeline->setCheckable(true);
    showTimeline->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M));
    connect(showTimeline, &QAction::toggled, host, [=](bool on) {
        if (ed() != nullptr) { ed()->setTimelineVisible(on); }
    });
    editorOnlyActions.append(showTimeline);

    // Show Buffers (EP-026 S5, SP-179 / T-0606) — the ONE app-wide Copy Buffers palette, following the front project.
    // ✅ Ctrl+Alt+B (user, 2026-10-11: *"Ctrl-Alt-B is correct."*), Apple's ⌥⌘B. Every window's check follows the one
    // palette (`AppEnvironment::setBuffersPaletteVisible`).
    QAction* showBuffers = view->addAction(tr("Show Buffers"));
    showBuffers->setCheckable(true);
    showBuffers->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_B));
    connect(showBuffers, &QAction::toggled, host, [=](bool on) {
        if (env != nullptr) { env->setBuffersPaletteVisible(on); }
    });
    editorOnlyActions.append(showBuffers);

    // ✅ EP-048 S5a ([SP-182] Q4, EP-047 P11): View ▸ Show Markup Hints, Ctrl+Shift+H (Apple's ⇧⌘H). Per project window, and
    // it travels (`project-settings.json` `markupHints`, absent = ON). ⚠️ A Ctrl+Shift chord: the macOS→VNC path eats them
    // (`project_linux_vnc_input_constraints`), so it is measured on the rig (RDP), with the menu always available.
    view->addSeparator();
    QAction* showMarkupHints = view->addAction(tr("Show Markup Hints"));
    showMarkupHints->setCheckable(true);
    showMarkupHints->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    connect(showMarkupHints, &QAction::toggled, host, [=](bool on) {
        if (ed() != nullptr) { ed()->setMarkupHints(on); }
    });
    editorOnlyActions.append(showMarkupHints);

    // Story Structure… (EP-025 / SP-081, T-0330) — pick a built-in structure (Three Act
    // / Five Act / Hero's Journey / …) or remove it; the timeline paints the bands.
    QAction* storyStructure = view->addAction(tr("Story Structure…"));
    connect(storyStructure, &QAction::triggered, host, [=]() {
        if (ed() != nullptr) { ed()->pickStoryStructure(); }
    });
    editorOnlyActions.append(storyStructure);

    // --- Scene ------------------------------------------------------------
    QMenu* scene = bar->addMenu(tr("&Scene"));
    QAction* splitScene = scene->addAction(tr("Split Scene"));
    splitScene->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    connect(splitScene, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->splitScene(); } });
    editorOnlyActions.append(splitScene);

    QAction* mergeScene = scene->addAction(tr("Merge Scene"));
    mergeScene->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backspace));
    connect(mergeScene, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->mergeScene(); } });
    editorOnlyActions.append(mergeScene);

    // --- Chapter ----------------------------------------------------------
    // Chapter ▸ Merge is the reason this menu exists: Ctrl+Shift+Backspace is swallowed
    // by the macOS→VNC input path, so the menu action is the reliable trigger.
    QMenu* chapter = bar->addMenu(tr("&Chapter"));
    QAction* splitChapter = chapter->addAction(tr("Split Chapter"));
    splitChapter->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return));
    connect(splitChapter, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->splitChapter(); } });
    editorOnlyActions.append(splitChapter);

    QAction* mergeChapter = chapter->addAction(tr("Merge Chapter"));
    mergeChapter->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Backspace));
    connect(mergeChapter, &QAction::triggered, host,
            [=]() { if (ed() != nullptr) { ed()->mergeChapter(); } });
    editorOnlyActions.append(mergeChapter);

    // --- Project ----------------------------------------------------------
    QMenu* project = bar->addMenu(tr("&Project"));
    // ✅ EP-026 S3 ([SP-204]; Apple: EP-047 SP-167 Q1) — an ACTION, not a setting: its own dialog.
    QAction* purgeStale = project->addAction(tr("Purge Stale History Branches…"));
    purgeStale->setObjectName(QStringLiteral("purgeStaleBranchesAction"));
    connect(purgeStale, &QAction::triggered, host, [=]() {
        if (ed() != nullptr) {
            ed()->purgeStaleBranches();
        }
    });
    editorOnlyActions.append(purgeStale);

    // --- Project ▸ Manage Worlds… (EP-035 AC3, SP-127 / T-0494) ----------
    //
    // ⚠️ The surface that closes `capability_without_surface` for worlds:
    // addWorld / relinkWorld / getWorldStatus / getWorldBinding were all bridged
    // with ZERO callers, so a project whose world had MOVED could not be repaired
    // from the Linux app at all.
    //
    // ⚠️ Gated on a project being open (editorOnlyActions) — unlike Help ▸ About,
    // worlds are a property OF a project and the action is meaningless without one.
    QAction* worldsAction = project->addAction(tr("Manage Worlds…"));
    connect(worldsAction, &QAction::triggered, host, [=]() {
        if (ed() != nullptr) {
            ed()->manageWorlds();
        }
    });
    editorOnlyActions.append(worldsAction);

    project->addSeparator();   // ✅ [SP-211] live pass: Project Settings… last, below a rule
    QAction* settings = project->addAction(tr("Project Settings…"));
    // ✅ [SP-211] Q2 (Apple: ⌥⌘, since SP-210): Ctrl+, is the app's Settings.
    settings->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Comma));
    connect(settings, &QAction::triggered, host, [=]() {
        // ✅ EP-048 S5a ([SP-182]): the real dialog — it replaces the T-0312 "coming soon" stub. Changes apply live.
        if (ed() == nullptr || ed()->preferences() == nullptr) {
            return;
        }
        ProjectSettingsDialog dlg(ed()->preferences(), host);
        dlg.exec();
    });
    editorOnlyActions.append(settings);



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
    connect(about, &QAction::triggered, host, [=]() {
        QDialog dlg(host);
        dlg.setWindowTitle(tr("About Scrivi"));
        auto* layout = new QVBoxLayout(&dlg);

        auto* name = new QLabel(tr("Scrivi"), &dlg);
        QFont nameFont = name->font();
        nameFont.setBold(true);
        nameFont.setPointSize(nameFont.pointSize() + 4);
        name->setFont(nameFont);
        layout->addWidget(name);

        // ✅ [T-0622]: from the header generated on every build from git — the same version as the Apple build.
        auto* build = new QLabel(
            tr("Version %1 (%2)\nQt %3")
                .arg(QLatin1String(SCRIVI_VERSION), QLatin1String(SCRIVI_COMMIT), QLatin1String(qVersion())),
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

    // [SP-185] T-0612 — Help ▸ Show Log Folder: the diagnostic log, for a writer to send with a report. Not gated either.
    QAction* showLog = help->addAction(tr("Show Log Folder"));
    showLog->setObjectName(QStringLiteral("showLogFolderAction"));
    connect(showLog, &QAction::triggered, host, []() {
        QDir().mkpath(DiagnosticLog::folder());
        QDesktopServices::openUrl(QUrl::fromLocalFile(DiagnosticLog::folder()));
    });

    if (w != nullptr) {
        w->showNavigatorAction_ = showNavigator;
        w->showInspectorAction_ = showInspector;
        w->showTimelineAction_ = showTimeline;
        w->showBuffersAction_ = showBuffers;
        w->showMarkupHintsAction_ = showMarkupHints;
    } else {
        for (QAction* a : startWindowProjectOnly) {
            a->setEnabled(false);
        }
    }
}

void ScriviWindow::updateMenuState(bool editorActive)
{
    for (QAction* a : editorOnlyActions_) {
        a->setEnabled(editorActive);
    }

    // Reflect the editor's real inspector visibility in the View ▸ Show Inspector
    // check-state whenever a project is active (block signals so syncing the box
    // doesn't re-drive setInspectorVisible). No editor → leave it unchecked.
    // ⚠️ [I-0256] — same per-window check-state sync the other two panes get.
    if (showNavigatorAction_ != nullptr) {
        const bool shown = editorActive && editor_ != nullptr
                           && editor_->isNavigatorVisible();
        const QSignalBlocker block(showNavigatorAction_);
        showNavigatorAction_->setChecked(shown);
    }

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

    // [SP-182]: View ▸ Show Markup Hints follows THIS window's project.
    if (showMarkupHintsAction_ != nullptr) {
        const QSignalBlocker block(showMarkupHintsAction_);
        showMarkupHintsAction_->setChecked(editorActive && editor_ != nullptr && editor_->markupHints());
    }

    // EP-026 S5: View ▸ Show Buffers is app-wide — the same check in every window.
    setBuffersActionChecked(env_ != nullptr && env_->buffersPaletteVisible());
}

void ScriviWindow::setBuffersActionChecked(bool on)
{
    if (showBuffersAction_ != nullptr) {
        const QSignalBlocker block(showBuffersAction_);
        showBuffersAction_->setChecked(on);
    }
}

BufferService* ScriviWindow::bufferService() const
{
    return editor_ != nullptr ? editor_->buffers() : nullptr;
}

void ScriviWindow::changeEvent(QEvent* event)
{
    // EP-026 S5 (SP-179): the FRONT project is the one whose window was last activated (Apple: `frontmostSession`).
    if (event->type() == QEvent::ActivationChange && isActiveWindow() && env_ != nullptr) {
        env_->projectWindowActivated(this);
    }
    QMainWindow::changeEvent(event);
}

void ScriviWindow::showEditor(const QString& projectPath, const QString& title,
                              const QVariantMap& openedProject)
{
    title_ = title;
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
                    // ⚠️ [SP-147] T-0567 — the load has SETTLED either way, so the
                    // pending-open hold for R3 ends; the registry answers now.
                    if (env_ != nullptr) {
                        env_->projectLoadSettled(this);
                    }
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
                        // ⚠️ [SP-147] T-0566 — record it OPEN now, not only at
                        // quit: ✅ a SIGKILLed session still knows what was open.
                        env_->sessionStore().setOpen(shownProjectID(),
                                                     editor_->currentProjectPath(),
                                                     title_);
                        // EP-026 S5 (SP-179): its buffers now exist — the palette follows it if it is in front.
                        env_->projectWindowLoaded(this);
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
    // ✅ [I-0305]: Close Project releases the project below, so it waits for background work first — then saves what
    // landed dirty and closes. ⛔ [I-0335]: NOT `flushEditor()` — its resume-place stamp queues a save every time, so the
    // wait never ended (the dialog flashed forever). The stamp queued before this wait has landed; a dirty save converges.
    if (waitForBackgroundWork([this] {
            if (editor_ != nullptr) {
                editor_->saveDirtyScenes();
            }
            showLanding();
        })) {
        return;
    }
    // ⚠️ [SP-146] T-0561 — AN EDITOR-ONLY WINDOW HAS NO LANDING PAGE TO SHOW.
    //
    // ✅ [R-Q3]: Landing is its own window. ⚠️ So "go to landing" from a project
    // window means RAISE THAT WINDOW — ⛔ and this window CLOSES, because a project
    // window with no project is not a thing this app has.
    // ⚠️ `AppEnvironment::projectWindowClosing()` shows Landing when the last one
    // goes, so closing here is safe: ⛔ the app never quits implicitly.
    if (landing_ == nullptr) {
        // ⛔ [I-0336]: never release the project before `close()` — `closeEvent` asks about a held queue FIRST, and on
        // Cancel the window stays with its project; it releases the project itself when the close goes ahead.
        if (!close()) {
            return;
        }
        if (env_ != nullptr) {
            env_->showLanding();
        }
        return;
    }

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
        // EP-026 S2 (SP-178): commit the pending typing FIRST — an autosave never seals it (T-0396), so without this the
        // last sentence would reach disk but not history. Apple's order: flush → save → close (close is in
        // `releaseProject`).
        editor_->flushHistory();
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
    // ✅ [I-0305] — NOT WHILE THIS PROJECT IS BEING WRITTEN. The scenes a worker is writing are out of the dirty set, so
    // the flush below would not write them, and the worker's landing would never run (the window is gone). Wait, then
    // close as usual.
    if (waitForBackgroundWork([this] { close(); })) {
        event->ignore();
        return;
    }
    // ✅ [I-0322] (Apple: [T-0570]) — a close loses the edits a held write queue cannot land (the drive is gone), so the
    // writer is asked first. A quit asks once, at its start (`AppEnvironment::quitApplication`).
    if ((env_ == nullptr || !env_->isQuitting()) && !EditorShell::confirmDiscardingHeld(this, {editor_}, tr("Close"))) {
        event->ignore();
        return;
    }

    flushEditor();   // don't lose edits when the window (and app) closes

    // ⚠️ [SP-147] T-0566 — RECORD WHERE THIS WINDOW WAS, BEFORE THE RELEASE BELOW.
    // ✅ Every way a project window goes away arrives here: the X, `Close Project`
    // (`showLanding()` → `close()`), and Quit (`quitApplication()` → `close()`).
    // ⚠️ The environment decides open-vs-closed from whether the app is QUITTING.
    // ⚠️ A maximized window records its NORMAL geometry — ✅ what un-maximizing
    // after a restore should return to — ⛔ not the screen-sized maximized rect.
    if (env_ != nullptr && editor_ != nullptr && !shownProjectID().isEmpty()) {
        SessionStore::Entry state;
        state.projectID  = shownProjectID();
        state.path       = editor_->currentProjectPath();
        state.title      = title_;
        state.maximized  = isMaximized();
        state.frame      = state.maximized ? normalGeometry() : geometry();
        // ⚠️ [I-0264] — on Wayland the position above is a meaningless `0,0`; keep the
        // previously stored one and take only the new size.
        state.frame      = AppEnvironment::recordableFrame(
            state.frame, env_->sessionStore().entry(state.projectID).frame,
            AppEnvironment::platformReportsWindowPosition());
        state.paneSizes  = editor_->paneSizes();
        state.outerSizes = editor_->outerSizes();
        env_->projectWindowReleasing(state);
    }

    // ⚠️ [SP-146] T-0560 — R8: release the project as this window goes away.
    // ✅ `releaseProject()` calls `scrivi_close_project` for THIS project and
    // deregisters its session — ⛔ and only this one. ⚠️ Without it, closing a
    // window would leave the core's index resident and R3 reporting the project as
    // open forever, so it could never be reopened.
    if (editor_ != nullptr) {
        editor_->releaseProject();
    }

    // ⚠️ [SP-146] T-0561 — tell the app environment, so that when the LAST project
    // window closes Landing is shown rather than the app quitting ([R-Q3]).
    if (env_ != nullptr) {
        env_->projectWindowClosing(this);
    }
    QMainWindow::closeEvent(event);
}

bool ScriviWindow::waitForBackgroundWork(std::function<void()> then)
{
    if (editor_ == nullptr || !editor_->backgroundWorkInProgress()) {
        return false;
    }
    if (waitDialog_ != nullptr) {
        return true;   // already waiting: the first request finishes the close
    }
    waitDialog_ = showSavingUpdates(this);
    editor_->whenBackgroundWorkDone([this, then = std::move(then)] {
        if (waitDialog_ != nullptr) {
            waitDialog_->close();
            waitDialog_->deleteLater();
        }
        QTimer::singleShot(0, this, then);   // outside the worker's landing, which is still on the stack
    });
    return true;
}

QProgressDialog* ScriviWindow::showSavingUpdates(QWidget* parent)
{
    // ⛔ No Cancel: the work cannot be stopped, only waited for (the user's ruling).
    auto* dialog = new QProgressDialog(tr("Please wait — saving updates…"), QString(), 0, 0, parent);
    dialog->setObjectName(QStringLiteral("savingUpdatesDialog"));
    dialog->setWindowTitle(tr("Saving"));
    dialog->setWindowModality(parent != nullptr ? Qt::WindowModal : Qt::ApplicationModal);
    dialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    dialog->setCancelButton(nullptr);
    dialog->setMinimumDuration(0);
    dialog->setAutoClose(false);
    dialog->setAutoReset(false);
    dialog->show();
    return dialog;
}

// ⚠️ [SP-146] T-0560 — deregister from the window map BEFORE this window dies.
// ⛔ The manager holds BORROWED pointers, so a window that dies while still mapped
// leaves a dangle that the next R3 check would follow.
ScriviWindow::~ScriviWindow()
{
    if (env_ != nullptr) {
        env_->windows().deregisterWindow(this);
        env_->projectWindowDestroyed(this);   // EP-026 S5: the palette's front project
    }
}

void ScriviWindow::applySplitterSizes(const QList<int>& paneSizes,
                                      const QList<int>& outerSizes)
{
    if (editor_ != nullptr) {
        editor_->setPaneSizes(paneSizes);
        editor_->setOuterSizes(outerSizes);
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
