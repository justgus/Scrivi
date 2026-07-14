#include "ScriviWindow.hpp"

#include <QCloseEvent>
#include <QQuickWidget>
#include <QStackedWidget>

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
    resize(820, 560);

    stack_ = new QStackedWidget(this);
    // The QQuickWidget resizes with the view so the QML fills the window.
    landing_->setResizeMode(QQuickWidget::SizeRootObjectToView);
    stack_->addWidget(landing_);   // page 0 — landing
    setCentralWidget(stack_);
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
    }
    // On failure the editor shows its own inline error and we stay on landing.
}

void ScriviWindow::showLanding()
{
    stack_->setCurrentWidget(landing_);
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
