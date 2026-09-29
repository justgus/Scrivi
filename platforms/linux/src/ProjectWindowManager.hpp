#pragma once

#include <QHash>
#include <QList>
#include <QString>

class ScriviWindow;

// ProjectWindowManager — projectID → the window showing that project
// (EP-043 / SP-146, T-0560).
//
// ## ✅ WHAT THIS IS
//
// The Qt analogue of Apple's `ProjectWindowManager`
// (`Scrivi/App/ProjectWindowManager.swift`, EP-018 / T-0194).
//
// ## ⚠️ IT IS A SEPARATE MAP FROM `OpenProjectRegistry`, AND THAT IS DELIBERATE
//
// ⛔ Apple keeps TWO maps and so does this:
//   • `OpenProjectRegistry`  projectID → **session**  (the project's STATE)
//   • `ProjectWindowManager` projectID → **window**   (the project's SURFACE)
//
// ⚠️ Collapsing them looks tempting and is wrong: ✅ the registry is the
// AUTHORITATIVE answer to "is this project open?" and is consulted BEFORE a window
// exists — ⛔ while a window is a `QWidget` with a Qt lifetime of its own. ✅ Keeping
// them apart is what lets R3 be answered without asking the window layer anything.
//
// ## ⚠️ OWNERSHIP — ⛔ this class does NOT own the windows
//
// ✅ A `ScriviWindow` is a top-level `QMainWindow` with NO parent; ⚠️ it owns itself
// and is destroyed by `Qt::WA_DeleteOnClose`. ⛔ This map holds BORROWED pointers.
// ⚠️ A window MUST deregister before it dies or this map holds a dangle — ✅ which
// is why `ScriviWindow` deregisters in its own destructor, not from outside.
class ProjectWindowManager
{
public:
    // The window showing a projectID, or nullptr when that project has no window.
    [[nodiscard]] ScriviWindow* window(const QString& projectID) const
    {
        return windows_.value(projectID, nullptr);
    }

    // ⚠️ Register a window under the project it shows. ✅ An empty projectID is NOT
    // registered — the same guard `OpenProjectRegistry` applies, for the same
    // reason: a failed load has no identity and must not look like an open project.
    void registerWindow(const QString& projectID, ScriviWindow* window)
    {
        if (projectID.isEmpty() || window == nullptr) {
            return;
        }
        windows_.insert(projectID, window);
    }

    // ✅ Safe when absent.
    void deregister(const QString& projectID) { windows_.remove(projectID); }

    // ⚠️ Remove by WINDOW, for teardown: a closing window knows itself but may not
    // still know which project it showed. ✅ Removes every key pointing at it.
    void deregisterWindow(ScriviWindow* window)
    {
        for (const QString& key : windows_.keys(window)) {
            windows_.remove(key);
        }
    }

    [[nodiscard]] bool isEmpty() const { return windows_.isEmpty(); }
    [[nodiscard]] int  count()   const { return windows_.size(); }

    // ⚠️ Order is NOT significant.
    [[nodiscard]] QList<ScriviWindow*> allWindows() const { return windows_.values(); }

private:
    // projectID → window (borrowed, never owned).
    QHash<QString, ScriviWindow*> windows_;
};
