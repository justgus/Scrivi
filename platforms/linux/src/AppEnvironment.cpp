#include "AppEnvironment.hpp"

#include "LandingWindow.hpp"
#include "ScriviWindow.hpp"

// ⚠️ [SP-146] T-0560 — R7: QUIT FLUSHES EVERY SESSION, NOT ONE.
//
// ⛔ THE OLD HOOK WAS SINGULAR BY CONSTRUCTION, and that is worth stating plainly:
// `main.cpp` constructed ONE `ScriviWindow` BY VALUE and connected
// `QCoreApplication::aboutToQuit` to THAT INSTANCE's `flushEditor`. ⚠️ With N
// windows it would have flushed whichever one `main()` happened to hold and
// SILENTLY DROPPED the edits in every other — ✅ the exact shape of data loss R7
// exists to prevent.
//
// ✅ Now quit asks the WINDOW MANAGER for every open window and flushes each.
// ⚠️ `flushEditor()` is a no-op when a window has no project, so a Landing-only
// window costs nothing.
void AppEnvironment::flushAllWindows()
{
    // ⚠️ Iterate a COPY: `flushEditor()` writes to disk and must not be affected by
    // anything that mutates the map mid-loop. ✅ `allWindows()` already returns a
    // value list, so this is a copy by construction.
    const QList<ScriviWindow*> windows = windows_.allWindows();
    for (ScriviWindow* window : windows) {
        if (window != nullptr) {
            window->flushEditor();
        }
    }
}

// ⚠️ [SP-146] T-0561 — [R-Q3]: `File ▸ New` / `File ▸ Open` in a PROJECT window
// RAISE Landing rather than swapping that window's own central widget.
//
// ⛔ BEFORE THIS, those actions called `showLanding()` ON THE PROJECT WINDOW, which
// replaced the manuscript the writer was looking at. ⚠️ With one window that read
// as "go back"; ✅ with N windows it would be destructive — the writer asked to open
// a DIFFERENT project, not to close this one.
void AppEnvironment::showLanding()
{
    if (landing_ != nullptr) {
        landing_->raiseToFront();
    }
}

// ⚠️ [SP-146] T-0561 — R3 ENFORCED, not merely answered.
//
// ✅ `existingWindowFor()` answers *"is this project already open?"* from the
// REGISTRY. ⛔ This acts on that answer: raise the existing window, or build a new
// editor-only one.
ScriviWindow* AppEnvironment::openProjectWindow(const QString& projectPath,
                                                const QString& title,
                                                const QVariantMap& openedProject)
{
    const QString projectID =
        openedProject.value(QStringLiteral("projectID")).toString();

    // ✅ R3 — ALREADY OPEN: raise, never duplicate.
    if (ScriviWindow* existing = existingWindowFor(projectID)) {
        existing->raiseToFront();
        return existing;
    }

    // ⚠️ A NEW TOP-LEVEL WINDOW, with NO PARENT. ⛔ Parenting it to Landing would
    // make it a child window that minimises and closes with its parent — ✅ these
    // are siblings, each showing its own manuscript.
    auto* window = new ScriviWindow(/*landing=*/nullptr, appSupportRoot_, this);

    // ⚠️ Qt deletes it when it closes, so nothing here owns it. ✅ Its destructor
    // deregisters it from the window map, which is why that map can hold borrowed
    // pointers safely.
    window->setAttribute(Qt::WA_DeleteOnClose);

    // ✅ Give it the shell controller so `File ▸ New` / `File ▸ Open` from THIS
    // window can drive the landing QML's flow ([R-Q3]).
    if (shell_ != nullptr) {
        window->setShellController(shell_);
    }
    window->show();
    window->showEditor(projectPath, title, openedProject);
    return window;
}

// ⚠️ [SP-146] T-0561 — [R-Q3]: closing the LAST project window SHOWS Landing and
// NEVER quits implicitly.
//
// ⛔ WITHOUT THIS THE APP WOULD QUIT: Qt's `quitOnLastWindowClosed` fires when the
// final top-level window closes, and a hidden Landing does not count. ⚠️ So a
// writer closing her only manuscript would lose the app rather than return to the
// project list — ✅ which is precisely what [R-Q3] ruled against, because R7 (quit
// flushes every session) becomes much harder to reason about if a window close can
// become a quit.
void AppEnvironment::projectWindowClosing(ScriviWindow* window)
{
    windows_.deregisterWindow(window);

    // ⚠️ THE CLOSING WINDOW IS ALREADY OUT OF THE MAP, so "empty" means this was
    // the last one.
    if (windows_.isEmpty()) {
        showLanding();
    }
}
