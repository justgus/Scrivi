#pragma once

#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QVariantMap>
#include <QtGlobal>

class ScriviWindow;
class LandingWindow;
class ShellController;

#include "OpenProjectRegistry.hpp"
#include "ProjectWindowManager.hpp"
#include "SessionStore.hpp"

// AppEnvironment — Linux's app-global state owner (EP-043 / SP-146, T-0558).
//
// ## ✅ WHAT THIS IS
//
// The Qt analogue of Apple's `AppEnvironment` (`Scrivi/App/AppEnvironment.swift`,
// EP-018 / T-0192, T-0194). ⚠️ ONE INSTANCE, constructed in `main()` and handed
// down. ⛔ It owns NO widgets and does NO layout.
//
// ## ⛔ LINUX HAD NO APP-LEVEL OWNER AT ALL — that is why this class exists
//
// ⚠️ Measured 2026-09-27 (Epic §The app-level owner): `platforms/linux/src/` had
// NO `AppEnvironment` equivalent — ⛔ no app singleton, no `Q_GLOBAL_STATIC`, no
// `qApp` property. ✅ App-global state was LOCAL VARIABLES IN `main()`:
// `appSupportRoot` was resolved at `main.cpp:87` and hand-threaded into the QML
// context, `ScriviWindow` and `ShellController` SEPARATELY.
//
// ⚠️ So this is not "a member moved." ✅ It is the first app-level owner the Linux
// app has had, and [SP-145]'s own note calling it "SP-146's first step"
// UNDERSTATED it: it is a real Task, not a preamble.
//
// ## ⛔ WHY THE REGISTRY CANNOT LIVE ON A WIDGET
//
// ⚠️ `OpenProjectRegistry` answers *"is this project already open?"* — ✅ a question
// asked BEFORE a window exists, about the app's siblings. ⛔ No single `EditorShell`
// can answer it about the others. ✅ [SP-145] put it on `EditorShell` because that
// Sprint changed no window code and one shell held one project; ⚠️ that was correct
// for one window and WRONG for N, and the user agreed 2026-09-27.
//
// ✅ APPLE'S SHAPE IS THE TARGET, and the load-bearing fact is this: ⚠️ EVERY reader
// of Apple's registry is inside `AppEnvironment`; ⛔ NOT ONE is in a view.
//
// ## ⚠️ NOT A SINGLETON — deliberately
//
// ⛔ No `instance()`, no `Q_GLOBAL_STATIC`. ✅ Apple constructs its `AppEnvironment`
// explicitly and hands it down, and this does the same: ⚠️ constructed once in
// `main()`, passed by pointer to the things that need it. ✅ A singleton would make
// the ownership graph implicit exactly where this Epic is trying to make it
// explicit — ⛔ and would make a second instance (tests) impossible.
//
// ## ⚠️ WHAT T-0558 PUTS HERE, AND WHAT COMES LATER
//
// ✅ T-0558 (this Task): `appSupportRoot` ONLY — ⚠️ mechanical, no behaviour change.
// ⚠️ T-0559: the registry and `ProjectSession` OWNERSHIP move here; `EditorShell`
//    takes a `ProjectSession*` instead of holding one.
// ⚠️ T-0560: `openProject()` gains the R3 check and window orchestration.
// ✅ [SP-147]: the open-session manifest and geometry. T-0566 records it and
//    adds the R6 guard; T-0567 restores from it.
//
// ✅ T-0559 DONE: the registry's owner. ✅ T-0560 DONE: the window manager and
// `openProject()`'s R3 check.
class AppEnvironment
{
public:
    // ⚠️ `appSupportRoot` is resolved by the caller (`scrivi::linux_app::appSupportRoot()`)
    // and injected, so this class does no environment lookup of its own — ✅ the same
    // discipline `ProjectSession` already follows.
    explicit AppEnvironment(QString appSupportRoot)
        : appSupportRoot_(appSupportRoot)
        , session_(std::move(appSupportRoot))
    {
    }

    // ⚠️ NON-COPYABLE, NON-MOVABLE. ✅ It is handed out by pointer and things bind to
    // it; ⛔ a copy would be a second app-global state owner, which is a contradiction.
    AppEnvironment(const AppEnvironment&)            = delete;
    AppEnvironment& operator=(const AppEnvironment&) = delete;
    AppEnvironment(AppEnvironment&&)                 = delete;
    AppEnvironment& operator=(AppEnvironment&&)      = delete;

    // The app-support root — ⚠️ app-global, resolved once, stable for the process.
    [[nodiscard]] QString appSupportRoot() const { return appSupportRoot_; }

    // ✅ THE AUTHORITATIVE RECORD of which projects are open ([R-Q2], R3).
    // ⚠️ Empty until T-0559 moves ownership here. ⛔ Ask THIS, never the window list.
    [[nodiscard]] OpenProjectRegistry&       openProjects()       { return openProjects_; }
    [[nodiscard]] const OpenProjectRegistry& openProjects() const { return openProjects_; }

    // ✅ projectID → the WINDOW showing it (T-0560). ⚠️ A SEPARATE map from the
    // registry, as Apple keeps them: ⛔ state and surface are different questions.
    [[nodiscard]] ProjectWindowManager&       windows()       { return windows_; }
    [[nodiscard]] const ProjectWindowManager& windows() const { return windows_; }

    // ✅ `<appSupportRoot>/session.ini` ([SP-147] AC1) — ⛔ the ONLY owner of that
    // file. ⚠️ Written by project windows as they load and close (T-0566).
    [[nodiscard]] SessionStore&       sessionStore()       { return session_; }
    [[nodiscard]] const SessionStore& sessionStore() const { return session_; }

    // ---- R6 — THE RESTORE GUARD ([SP-147] T-0566) -----------------------
    //
    // ⚠️ APPLE'S GUARD IS THE PRECEDENT (`AppEnvironment.swift:353-375`) — ✅ same
    // two questions, ⛔ different signals. ⚠️ [I-0150] is what it pays for: on
    // Apple, `xcodebuild test` LAUNCHED the app and rewrote a real project.
    //
    // ✅ Both are evaluated ONCE (function-local `static const`), as Apple's are
    // stored `let`s: ⚠️ *"the decision must not change underneath the app mid-run."*

    // True for a headless run — ✅ every smoke wrapper already exports
    // `QT_QPA_PLATFORM=offscreen`, so a test run identifies itself with NO new
    // plumbing. ⛔ `XDG_DATA_HOME` redirection was REJECTED: it depends on the
    // harness remembering to redirect, and one that forgets touches the real file.
    // ⚠️ Reads the env var, not `QGuiApplication::platformName()`, so it answers
    // without a display — ⛔ a `-platform offscreen` ARGUMENT is therefore not seen.
    [[nodiscard]] static bool isHeadlessRun()
    {
        static const bool headless =
            qEnvironmentVariable("QT_QPA_PLATFORM").startsWith(QLatin1String("offscreen"));
        return headless;
    }

    // True when the operator asked for a launch with NO project restored —
    // `SCRIVI_NO_RESTORE=1`. ✅ An ENV VAR, like Apple's `SCRIVI_NO_PROJECT_LOAD`;
    // ⚠️ presence is what counts, as on Apple.
    [[nodiscard]] static bool suppressRestore()
    {
        static const bool suppress = qEnvironmentVariableIsSet("SCRIVI_NO_RESTORE");
        return suppress;
    }

    // ✅ THE ONE ROUTE FROM `session.ini` TO A REOPENED WINDOW. ⚠️ T-0567's restore
    // MUST start here, ⛔ never at `sessionStore().openProjectIDs()` — the guard lives
    // in this function and nowhere else.
    //
    // ⛔ THE GUARD SUPPRESSES THE RESTORE ONLY. ⚠️ It returns nothing and WRITES
    // NOTHING: `session.ini` is left INTACT ([SP-147] AC6), so a suppressed launch
    // cannot lose the writer's windows — ⛔ a guard that CLEARED the file would erase
    // her session on the first test run. ✅ Apple says the same, explicitly.
    //
    // ⚠️ Returns every entry marked open, UNFILTERED by path — ✅ see
    // `resolvableProjectsToRestore()` for the AC4 filter.
    [[nodiscard]] QList<SessionStore::Entry> projectsToRestore() const
    {
        if (isHeadlessRun() || suppressRestore()) {
            qInfo("[Scrivi] %s — session restore suppressed; session.ini intact (R6).",
                  isHeadlessRun() ? "Headless run" : "SCRIVI_NO_RESTORE");
            return {};
        }
        QList<SessionStore::Entry> out;
        for (const QString& projectID : session_.openProjectIDs()) {
            out.append(session_.entry(projectID));
        }
        return out;
    }

    // ⚠️ [SP-147] AC4 / T-0567 — the guarded set, minus any project whose path no
    // longer resolves. ⛔ A SKIP WRITES NOTHING: [R-Q2] — a project moved or on an
    // unplugged drive is skipped for ONE launch and keeps its record and geometry.
    // ⚠️ `QFileInfo::exists` is a synchronous stat on the UI thread; ✅ an absent
    // path or unplugged drive answers at once, ⛔ but a HUNG network mount would
    // block launch here ([I-0193]'s class). Unmeasured on the rig.
    [[nodiscard]] QList<SessionStore::Entry> resolvableProjectsToRestore() const
    {
        QList<SessionStore::Entry> out;
        for (const SessionStore::Entry& e : projectsToRestore()) {
            if (!e.path.isEmpty() && QFileInfo::exists(e.path)) {
                out.append(e);
            } else {
                qInfo("[Scrivi] Restore: skipping %s — path does not resolve (%s); record kept.",
                      qPrintable(e.projectID), qPrintable(e.path));
            }
        }
        return out;
    }

    // ✅ R4 — reopen every project that was open at the last quit (T-0567).
    // ⚠️ Called ONCE by `main()`, after Landing and the shell controller exist.
    // ✅ Each project goes through `openProjectWindow()`, the same funnel as a
    // Landing open — ⛔ no second open path.
    void restoreSession();

    // ---- [I-0264] WAYLAND: POSITION IS NOT THE APP'S TO KNOW ------------
    //
    // ⚠️ Under Wayland a client is never told where its window is, and cannot place it;
    // the compositor decides (GNOME centres new windows). ✅ Established on the rig
    // 2026-09-30: every saved frame read `0,0,W,H`. ✅ RULED (user): ACCEPT it — size and
    // maximized state still restore — ⛔ but stop RECORDING the meaningless `0,0`, which
    // would overwrite a real position saved from an X11 session.
    //
    // ✅ Pure, so it can be tested with no display: when the position is not known, keep
    // the PREVIOUSLY stored position and take only the new SIZE.
    [[nodiscard]] static QRect recordableFrame(const QRect& current, const QRect& previous,
                                               bool positionKnown)
    {
        if (positionKnown || !current.isValid()) {
            return current;
        }
        return QRect(previous.isValid() ? previous.topLeft() : QPoint(0, 0), current.size());
    }

    // False under Wayland. Defined out-of-line (needs QGuiApplication).
    [[nodiscard]] static bool platformReportsWindowPosition();

    // A saved frame whose display is gone is re-centred on the primary screen
    // (Apple's `clampedOnscreen`). Out-of-line (needs QScreen).
    [[nodiscard]] static QRect clampedOnscreen(const QRect& frame);

    // ⚠️ Called by a project window just BEFORE it releases its project (T-0566),
    // with the window's state captured while it still has a project to describe.
    // ✅ Records geometry and splitters; ⚠️ marks the project CLOSED unless the app
    // is QUITTING — ⛔ a quit closes every window, and a project open at quit must
    // stay open in the manifest or R4 has nothing to restore. ✅ Apple's
    // `isTerminating` freeze is the same rule.
    void projectWindowReleasing(const SessionStore::Entry& state)
    {
        session_.record(state);
        if (!quitting_) {
            session_.setClosed(state.projectID);
        }
    }

    // ---- R3 — THE NON-REENTRANCY CHECK (T-0560) -------------------------
    //
    // ✅ *"Is this project already open?"* — ⚠️ answered from the REGISTRY, never
    // from the window list. ⛔ [EP-018] proved on evidence that the platform's own
    // de-duplication could not be trusted (macOS 26's `WindowGroup(for:)` was not
    // race-safe, T-0191); ⚠️ Qt has no de-duplication to trust at all.
    //
    // ⚠️ RETURNS THE EXISTING WINDOW, or nullptr when the project is not open.
    // ✅ The caller RAISES what it gets back instead of opening a second copy.
    //
    // ⚠️ [SP-147] T-0567 — OR A WINDOW STILL LOADING IT. ⛔ The registry learns a
    // project only when its load FINISHES, so a writer clicking a recent while
    // restore is still loading that same project would get a SECOND window on it
    // — two editors writing one project. ✅ Restore makes that likely at launch
    // (Landing is up, loads are async), so an open whose identity is KNOWN up
    // front is held in `pendingOpens_` until it settles.
    [[nodiscard]] ScriviWindow* existingWindowFor(const QString& projectID) const
    {
        if (projectID.isEmpty()) {
            return nullptr;
        }
        if (openProjects_.isOpen(projectID)) {
            return windows_.window(projectID);
        }
        return pendingOpens_.value(projectID, nullptr);
    }

    // ⚠️ A project window's load has finished, SUCCESS OR FAILURE — ✅ it leaves the
    // pending set (the registry answers from here on, or the window closes).
    void projectLoadSettled(ScriviWindow* window);

    // ---- R7 — quit must flush EVERY session, not one --------------------
    //
    // ⛔ `main()`'s `aboutToQuit → ScriviWindow::flushEditor` was SINGULAR BY
    // CONSTRUCTION: `main.cpp` built ONE window by value and bound quit to it.
    // ⚠️ With N windows that hook would flush one and silently drop the rest.
    // ✅ Defined out-of-line (AppEnvironment.cpp) because it calls into
    // `ScriviWindow`, which is only forward-declared here.
    void flushAllWindows();

    // ---- The Landing window ([R-Q3], T-0561) ----------------------------
    //
    // ⚠️ EXACTLY ONE, app-wide. ⛔ "Both pages in every window" was rejected on
    // measurement (N QML engines, N bridges; [I-0232] cost ~79% of an open for one
    // duplicate). ✅ Set once by `main()`; ⛔ NOT owned here.
    void setLandingWindow(LandingWindow* landing) { landing_ = landing; }
    [[nodiscard]] LandingWindow* landingWindow() const { return landing_; }

    // ✅ Bring Landing to the front — ⚠️ what `File ▸ New` / `File ▸ Open` in a
    // PROJECT window do, instead of swapping their own central widget.
    void showLanding();

    // ---- Project windows (T-0561) ---------------------------------------
    //
    // ✅ Create a NEW editor-only window for a project, or RAISE the existing one.
    // ⚠️ THIS IS WHERE R3 IS ENFORCED for real: ⛔ `existingWindowFor()` only
    // answers the question; this acts on the answer.
    // ⚠️ Returns the window showing that project — ✅ new or existing.
    // ⚠️ [SP-147] T-0567 — `projectIDHint` is the identity when the caller knows it
    // WITHOUT an envelope (restore). ✅ With an identity, the window takes that
    // project's saved geometry and splitters — ON EVERY OPEN, as Apple's
    // `ProjectWindowFrameStore` does (I-0051), ⛔ not only during restore.
    ScriviWindow* openProjectWindow(const QString& projectPath,
                                    const QString& title,
                                    const QVariantMap& openedProject,
                                    const QString& projectIDHint = {});

    // ⚠️ Called by a project window as it closes. ✅ When the LAST one goes, Landing
    // is shown — ⛔ the app never quits implicitly on a window close ([R-Q3]).
    void projectWindowClosing(ScriviWindow* window);

    // ---- QUIT ([I-0257], T-0562) ----------------------------------------
    //
    // ⛔ `File ▸ Quit` MUST NOT go straight to `QApplication::quit()`.
    // ⚠️ MEASURED 2026-09-29: `quit()` exits the event loop and DOES NOT CLOSE
    // WINDOWS — with `WA_DeleteOnClose` windows, nothing tears them down.
    // ⛔ AND `closeAllWindows()` ALONE IS NOT ENOUGH EITHER: it walks a SNAPSHOT,
    // and this class's own teardown MUTATES the window set mid-walk (a closing
    // project window re-shows Landing), which ABORTS the cascade.
    // ✅ So quitting is explicit: flush everything, tear down OUR windows from OUR
    // OWN map, then quit. ⚠️ `quitting_` suppresses the re-show while it runs.
    void quitApplication();

    // ✅ True while `quitApplication()` is tearing down — ⚠️ read by
    // `projectWindowClosing()` and `LandingWindow::closeEvent` so neither fights
    // the teardown.
    [[nodiscard]] bool isQuitting() const { return quitting_; }

    // ⚠️ The QML↔C++ boundary controller, owned by the Landing window.
    // ✅ Handed to every project window it creates, so `File ▸ New` / `File ▸ Open`
    // from ANY window can drive the landing QML's existing flow ([R-Q3]) —
    // ⛔ rather than each window reimplementing an open UI.
    void setShellController(ShellController* shell) { shell_ = shell; }

private:
    ShellController* shell_ = nullptr;   // NOT owned — the Landing window parents it

public:

private:
    QString             appSupportRoot_;
    LandingWindow*      landing_ = nullptr;   // NOT owned — main() owns it
    bool                quitting_ = false;   // ⚠️ [I-0257] — teardown in progress
    OpenProjectRegistry openProjects_;
    ProjectWindowManager windows_;
    SessionStore        session_;
    // ⚠️ projectID → a window whose load of it has not finished (T-0567). BORROWED.
    QHash<QString, ScriviWindow*> pendingOpens_;
};
