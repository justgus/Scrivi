#include "AppEnvironment.hpp"

#include <QApplication>
#include <QScreen>

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
namespace {

// ⚠️ [SP-147] T-0567 — a saved frame whose display is GONE must not open the window
// off-screen. ✅ APPLE'S RULE, ported as-is (`ProjectWindowFrameStore.clampedOnscreen`):
// keep the frame if it overlaps some screen by at least 80×80; otherwise re-centre
// it on the primary screen, shrinking it only as far as needed to fit.
QRect clampedOnscreen(const QRect& frame)
{
    for (const QScreen* screen : QGuiApplication::screens()) {
        const QRect i = screen->availableGeometry().intersected(frame);
        if (i.width() >= 80 && i.height() >= 80) {
            return frame;
        }
    }
    const QScreen* primary = QGuiApplication::primaryScreen();
    if (primary == nullptr) {
        return frame;
    }
    const QRect avail = primary->availableGeometry();
    QRect f(0, 0, qMin(frame.width(), avail.width()), qMin(frame.height(), avail.height()));
    f.moveCenter(avail.center());
    return f;
}

}  // namespace

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
                                                const QVariantMap& openedProject,
                                                const QString& projectIDHint)
{
    // ⚠️ The envelope's identity when there is one; ✅ restore's hint otherwise.
    QString projectID = openedProject.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        projectID = projectIDHint;
    }

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

    // ⚠️ [SP-147] T-0567 — AC5: THIS PROJECT'S window state, keyed BY PROJECT.
    // ⛔ [EP-018] shipped one global autosave frame once and every restored window
    // stacked at the default. ✅ No record (a first-ever open, or New Project with
    // no identity yet) ⇒ Qt's default placement, as before.
    const SessionStore::Entry saved =
        projectID.isEmpty() ? SessionStore::Entry{} : session_.entry(projectID);
    if (saved.frame.isValid()) {
        window->setGeometry(clampedOnscreen(saved.frame));
    }

    // ✅ Hold the identity until the load settles — see `existingWindowFor()`.
    if (!projectID.isEmpty()) {
        pendingOpens_.insert(projectID, window);
    }

    // ⚠️ `setGeometry` FIRST, then `showMaximized` — ✅ the stored frame becomes the
    // NORMAL geometry the window returns to when un-maximized.
    if (saved.maximized) {
        window->showMaximized();
    } else {
        window->show();
    }
    window->showEditor(projectPath, title, openedProject);
    window->applySplitterSizes(saved.paneSizes, saved.outerSizes);
    return window;
}

void AppEnvironment::projectLoadSettled(ScriviWindow* window)
{
    for (const QString& key : pendingOpens_.keys(window)) {
        pendingOpens_.remove(key);
    }
}

// ⚠️ [SP-147] T-0567 — R4. ✅ THE GUARD IS INSIDE `projectsToRestore()`, reached
// through `resolvableProjectsToRestore()`: ⛔ a headless or `SCRIVI_NO_RESTORE`
// launch gets an EMPTY list here and opens nothing.
//
// ✅ NO ENVELOPE: each window's editor performs the project's one and only open
// (the reload path, `EditorShell::load` with an empty map) — ⛔ opening here first
// would reintroduce [I-0232]'s double open.
// ⚠️ A project that is no longer `ready` (needs repair) FAILS that load and its
// window closes back to Landing — ⛔ restore NEVER repairs without the writer's
// consent; the repair prompt lives on Landing, which she can open it from.
void AppEnvironment::restoreSession()
{
    for (const SessionStore::Entry& e : resolvableProjectsToRestore()) {
        openProjectWindow(e.path, e.title, /*openedProject=*/{}, e.projectID);
    }
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
    projectLoadSettled(window);   // ⚠️ closed before its load finished — T-0567

    // ⛔ [I-0257] — DO NOT RE-SHOW LANDING WHILE QUITTING. ⚠️ Re-showing a window
    // during a teardown is what aborted `closeAllWindows()`'s cascade and left
    // windows standing.
    if (quitting_) {
        return;
    }

    // ⚠️ THE CLOSING WINDOW IS ALREADY OUT OF THE MAP, so "empty" means this was
    // the last one.
    if (windows_.isEmpty()) {
        showLanding();
    }
}

// ⚠️ [SP-146] T-0562 ([I-0257]) — QUIT, DONE EXPLICITLY.
//
// ⛔ `File ▸ Quit` USED TO CALL `QApplication::quit()` DIRECTLY, and that is the
// defect the user found: ⚠️ *"sometimes only closes one or two of the open
// windows… I cannot discern a pattern."*
//
// ✅ TWO MEASURED CAUSES, and the fix addresses both:
//
//   ⛔ (1) `QApplication::quit()` DOES NOT CLOSE WINDOWS. ⚠️ It exits the event
//          loop. With `WA_DeleteOnClose` windows, nothing tears them down — so
//          they simply stayed on screen. MEASURED: landing + 2 projects → quit →
//          both project windows still visible.
//
//   ⛔ (2) `QApplication::closeAllWindows()` WALKS A SNAPSHOT, and our own
//          teardown MUTATES the window set inside that walk: a closing project
//          window calls `projectWindowClosing()`, which RE-SHOWS Landing, while
//          `LandingWindow::closeEvent` ignores its own close whenever a project
//          window remains. ⚠️ The cascade aborts partway. MEASURED: it closed ONE
//          of TWO project windows.
//
// ✅ THAT IS THE "NO PATTERN": ⚠️ the outcome depends on where the mutation lands
// in the snapshot's order — ⛔ neither "last clicked" nor "last opened", exactly as
// the user reported. ⚠️ A second Quit cleared the rest because fewer windows then
// remained to perturb the walk.
//
// ✅ SO: flush everything FIRST (R7 — never lose edits), then close OUR windows
// from OUR OWN map (⛔ not from Qt's snapshot), then quit.
void AppEnvironment::quitApplication()
{
    // ⚠️ Re-entrant Quit (the writer clicks twice) must not restart the teardown.
    if (quitting_) {
        return;
    }
    quitting_ = true;

    // ✅ R7 FIRST, AND UNCONDITIONALLY. ⚠️ Flushing before any window dies means a
    // teardown that goes wrong still cannot lose edits.
    flushAllWindows();

    // ✅ Close from OUR map, which we control, rather than Qt's snapshot.
    // ⚠️ `allWindows()` returns a VALUE list, so deregistration during the loop
    // cannot invalidate what we are iterating.
    const QList<ScriviWindow*> windows = windows_.allWindows();
    for (ScriviWindow* window : windows) {
        if (window != nullptr) {
            window->close();
        }
    }

    if (landing_ != nullptr) {
        landing_->close();
    }
    QApplication::quit();
}
