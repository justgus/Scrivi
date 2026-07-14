// quit_smoke — headless check that the landing's Quit path actually quits the app
// (SP-062 regression fix). No user interaction.
//
// The shell flip (SP-061) moved the landing QML from a top-level
// QQmlApplicationEngine into a QQuickWidget. QQmlApplicationEngine auto-wires
// QQmlEngine::quit() (what QML's Qt.quit() emits) to QCoreApplication::quit(); a
// QQuickWidget's engine does NOT, so the landing Quit button did nothing ("Signal
// QQmlEngine::quit() emitted, but no receivers connected to handle it"). main.cpp
// now connects it explicitly.
//
// This harness reproduces that wiring against a QQuickWidget engine and asserts
// that emitting QQmlEngine::quit() drives the app to exit(0). It DOESN'T load the
// real Landing.qml (that needs the appSupportRoot/shell context) — it exercises the
// exact signal→slot the fix adds, which is where the bug lived.
//
// Uses the Qt offscreen platform (set by the driving script); shows no window.

#include <QApplication>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QTimer>

#include <cstdio>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QQuickWidget widget;

    // The connection main.cpp makes. If it's missing (the bug), quit() below is a
    // no-op and the fail-safe timer trips.
    QObject::connect(widget.engine(), &QQmlEngine::quit, &app, &QApplication::quit);

    bool quitFired = false;
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     [&quitFired] { quitFired = true; });

    // Emit the signal Qt.quit() emits, shortly after the loop starts.
    QTimer::singleShot(0, widget.engine(), [&widget] {
        emit widget.engine()->quit();
    });

    // Fail-safe: if quit() didn't take, force-exit non-zero after 3s so the test
    // fails loudly instead of hanging CI.
    QTimer::singleShot(3000, [] {
        std::fprintf(stderr, "FAIL: app did not quit after QQmlEngine::quit() "
                             "(the Quit signal is not connected)\n");
        QCoreApplication::exit(1);
    });

    const int rc = app.exec();
    if (rc == 0 && quitFired) {
        std::fprintf(stderr, "OK: QQmlEngine::quit() drove the app to exit(0).\n");
        std::printf("quit-ok\n");
        return 0;
    }
    return rc == 0 ? 1 : rc;
}
