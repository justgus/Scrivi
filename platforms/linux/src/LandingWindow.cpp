#include "LandingWindow.hpp"

#include "AppEnvironment.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QQuickWidget>

LandingWindow::LandingWindow(QQuickWidget* landing, AppEnvironment* env)
    : landing_(landing), env_(env)
{
    setWindowTitle(QStringLiteral("Scrivi"));
    // ⚠️ 820×560 — the size Landing.qml's own root Item declares, so the window
    // matches the content it was designed at rather than inheriting the editor's
    // 1220×760 (which left the landing content floating in empty space).
    resize(820, 560);

    if (landing_ != nullptr) {
        landing_->setResizeMode(QQuickWidget::SizeRootObjectToView);
        setCentralWidget(landing_);
    }
}

void LandingWindow::raiseToFront()
{
    // ⚠️ THREE CALLS, ALL NEEDED — ✅ `show()` restores a hidden or minimised
    // window, `raise()` lifts it in the stacking order, `activateWindow()` gives it
    // focus. ⛔ Any one alone leaves a case where the writer asked for Landing and
    // nothing visibly happened.
    show();
    raise();
    activateWindow();
}

void LandingWindow::closeEvent(QCloseEvent* event)
{
    // ⛔ CLOSING LANDING MUST NOT QUIT THE APP WHILE A PROJECT IS OPEN.
    //
    // ⚠️ Qt quits when the LAST top-level window closes
    // (`quitOnLastWindowClosed`), ✅ which is right — ⛔ but only if this really is
    // the last one. ⚠️ A writer with two manuscripts open who closes the Landing
    // window is tidying up, NOT quitting, and losing both editors there would be
    // the sharpest kind of surprise.
    //
    // ✅ So: HIDE instead of closing whenever any project window is still open.
    // ⚠️ `File ▸ New` / `File ▸ Open` bring it straight back (`raiseToFront`).
    if (env_ != nullptr && !env_->windows().isEmpty()) {
        hide();
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}
