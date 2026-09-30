#pragma once

#include <QMainWindow>

class AppEnvironment;
class QQuickWidget;

// LandingWindow — the Landing screen as its OWN top-level window
// (EP-043 / SP-146, T-0561, [R-Q3]).
//
// ## ⚠️ WHY LANDING NEEDED ITS OWN WINDOW
//
// ⛔ BEFORE THIS, Landing was PAGE 0 of a `QStackedWidget` inside the one
// `ScriviWindow`, and a project was page 1. ✅ That is why a second project had
// nowhere to go ([I-0178]): the app had exactly one window and it was already
// showing something.
//
// ⚠️ [R-Q3] RULED THE SHAPE: ✅ a SEPARATE Landing window; ⛔ project windows are
// EDITOR-ONLY. ✅ `File ▸ New` / `File ▸ Open` in a project window RAISE this
// window and trigger its EXISTING flow — ⚠️ reusing the plumbing, which
// `ScriviWindow.hpp:88` already argued for, ⛔ rather than reimplementing an open
// UI per window.
//
// ## ⛔ "BOTH PAGES IN EVERY WINDOW" WAS REJECTED, ON MEASUREMENT
//
// ⚠️ The obvious alternative — give every window its own landing page — means
// N QML engines and N `ScriviBridge` instances. ⛔ The app already paid ~79% of an
// open for ONE duplicate ([I-0232]), ✅ and "which window shows landing" would be
// ambiguous. ⚠️ So there is exactly ONE Landing window, and it is this.
//
// ## ⚠️ IT OWNS THE QML, AND THE CONTEXT PROPERTIES GO WITH IT
//
// ✅ `appSupportRoot`, `defaultProjectsFolder` and `shell` are context properties on
// the landing `QQuickWidget`. ⚠️ They moved here with it — ⛔ `main()` no longer
// sets them on a widget it also hands to a project window.
//
// ## ⛔ CLOSING IT DOES NOT QUIT
//
// ⚠️ [R-Q3]'s sibling ruling: ✅ closing the LAST project window SHOWS Landing and
// NEVER quits implicitly. ⛔ So this window closing must not end the app while a
// project is still open — ✅ it hides instead, and the app quits only when the
// writer actually asks.
class LandingWindow : public QMainWindow
{
    Q_OBJECT

public:
    // `landing` is the QQuickWidget hosting Landing.qml. ⚠️ Taken as the central
    // widget, so this window owns it.
    LandingWindow(QQuickWidget* landing, AppEnvironment* env = nullptr);

    // ✅ Bring Landing to the front (File ▸ New / File ▸ Open from a project window).
    void raiseToFront();

    // ⚠️ [I-0265] — the first show at launch: at the remembered size (and maximized
    // state), or the designed 820×560 when nothing is stored.
    void showRestored();

protected:
    // ⛔ Closing Landing must NOT quit the app while projects are open.
    void closeEvent(QCloseEvent* event) override;

private:
    // ⚠️ [I-0265] — record this window's geometry in `session.ini`.
    void recordGeometry();

    QQuickWidget*   landing_ = nullptr;
    AppEnvironment* env_     = nullptr;   // NOT owned
};
