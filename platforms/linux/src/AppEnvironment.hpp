#pragma once

#include <QString>

#include "OpenProjectRegistry.hpp"

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
// ⛔ [SP-147]: the open-session manifest and geometry restore. ⛔ NOT HERE YET.
//
// ⚠️ THE REGISTRY IS DECLARED HERE FROM THE START, unused by this Task, because
// T-0559 moves its OWNERSHIP and a half-moved registry is worse than either end
// state. ✅ It is empty until T-0559 populates it.
class AppEnvironment
{
public:
    // ⚠️ `appSupportRoot` is resolved by the caller (`scrivi::linux_app::appSupportRoot()`)
    // and injected, so this class does no environment lookup of its own — ✅ the same
    // discipline `ProjectSession` already follows.
    explicit AppEnvironment(QString appSupportRoot)
        : appSupportRoot_(std::move(appSupportRoot))
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

private:
    QString            appSupportRoot_;
    OpenProjectRegistry openProjects_;
};
