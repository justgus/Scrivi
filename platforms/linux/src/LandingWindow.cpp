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

void LandingWindow::showRestored()
{
    // ⚠️ [I-0265] — Apple remembers its Welcome window's frame (`WindowFrameAutosave`);
    // ⛔ Linux always reopened Landing at 820×560, however the writer had sized it.
    // ✅ `setGeometry` BEFORE showing, then `showMaximized` — so the stored frame becomes
    // the size un-maximizing returns to. ⚠️ On Wayland only the SIZE takes effect ([I-0264]).
    if (env_ != nullptr) {
        const QRect frame = env_->sessionStore().landingFrame();
        if (frame.isValid()) {
            setGeometry(AppEnvironment::clampedOnscreen(frame));
        }
        if (env_->sessionStore().landingMaximized()) {
            showMaximized();
            return;
        }
    }
    show();
}

void LandingWindow::recordGeometry()
{
    if (env_ == nullptr) {
        return;
    }
    const bool maximized = isMaximized();
    const QRect frame = AppEnvironment::recordableFrame(
        maximized ? normalGeometry() : geometry(),
        env_->sessionStore().landingFrame(),
        AppEnvironment::platformReportsWindowPosition());
    env_->sessionStore().recordLanding(frame, maximized);
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
    // ⚠️ [I-0265] — record FIRST, on every path: a hide (projects still open), a real
    // close, and the quit teardown (`quitApplication()` → `close()`).
    recordGeometry();

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
    // ⛔ [I-0257] — NEVER ignore the close while the app is QUITTING. ⚠️ That
    // ignore is what stopped Landing dying, ✅ and it is right for a writer
    // tidying up — ⛔ but during teardown it left the app half-closed.
    if (env_ != nullptr && !env_->isQuitting() && !env_->windows().isEmpty()) {
        hide();
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}
