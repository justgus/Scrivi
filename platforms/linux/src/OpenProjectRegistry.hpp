#pragma once

#include <QHash>
#include <QList>
#include <QString>

class ProjectSession;

// OpenProjectRegistry — the authoritative record of which projects are open
// (EP-043 / SP-145, T-0552).
//
// The Qt analogue of Apple's `OpenProjectRegistry`
// (`Scrivi/App/OpenProjectRegistry.swift`, EP-018 / T-0193).
//
// ## ⚠️ KEYED BY `projectID` — [EP-043] [R-Q2]
//
// ✅ `projectID` → the live `ProjectSession` for that project. ⛔ NOT keyed by
// path: two copies of one project at different paths are the SAME project and must
// resolve to ONE window, and a moved project must not become a stranger.
//
// ## ⚠️ WHY AN APP-SIDE REGISTRY IS AUTHORITATIVE
//
// ✅ This is the **R3 non-reentrancy** guard: opening an already-open project must
// RAISE its existing window, never open a second copy. ⚠️ [EP-018] proved on
// evidence that the platform's own de-duplication could not be trusted for this —
// macOS 26's `WindowGroup(for:)` de-dup was NOT race-safe (two rapid same-value
// opens both created windows, T-0191) — ⛔ so Apple abandoned native de-dup and
// made its registry authoritative. ✅ Restore-all opens windows CONCURRENTLY
// ([SP-147]), which is precisely the race that needs an authoritative answer.
//
// ⚠️ Qt has no window de-duplication to lean on at all, so the lesson ports
// directly: ✅ ASK THIS CLASS, never the window list.
//
// ## ⛔ WHAT THIS CLASS DOES NOT DO — and must not grow into
//
// ⛔ No windows. ⛔ No persistence. ⛔ No opening or closing of projects.
// ✅ It is a map with an opinion about its key.
//
// ⚠️ IT HOLDS AT MOST ONE SESSION IN [SP-145], AND THAT IS DELIBERATE — not an
// unfinished state. ✅ [EP-018] shipped its registry (T-0193) one Sprint BEFORE
// per-window sessions (T-0194) for the same reason: the registry is the thing the
// window layer is built AGAINST, so it exists and is correct first.
// ⚠️ `project_capability_without_surface` narrows the "dangling read" concern to a
// capability with NO reader; ⛔ this has a named reader arriving in [SP-146], which
// `feedback_design_to_capability_not_lcd` says is the CORRECT sequencing.
//
// ## ⚠️ OWNERSHIP
//
// ⛔ The registry does NOT own its sessions — it holds borrowed pointers. ✅ In
// [SP-145] `EditorShell` owns the single session; in [SP-146] each project window
// will own its own. ⚠️ A registered session MUST be deregistered before it dies,
// or this map holds a dangling pointer. ✅ That is [SP-146]'s contract to honour
// per-window, alongside R8's `scrivi_close_project`.
class OpenProjectRegistry
{
public:
    // The live session for a projectID, or nullptr when that project is not open.
    [[nodiscard]] ProjectSession* session(const QString& projectID) const
    {
        return sessions_.value(projectID, nullptr);
    }

    // ✅ THE R3 CHECK. True when the project is already open — the caller must then
    // raise/focus that window instead of opening a second copy.
    [[nodiscard]] bool isOpen(const QString& projectID) const
    {
        return !projectID.isEmpty() && sessions_.contains(projectID);
    }

    [[nodiscard]] bool isEmpty() const { return sessions_.isEmpty(); }
    [[nodiscard]] int  count()   const { return sessions_.size(); }

    // Every open projectID. ⚠️ Order is NOT significant — ⛔ do not let [SP-147]'s
    // restore order depend on it; the manifest owns order if order matters.
    [[nodiscard]] QList<QString> openProjectIDs() const { return sessions_.keys(); }

    // Register a loaded session under its projectID.
    //
    // ⚠️ A session with an EMPTY projectID is NOT registered: a failed load has no
    // identity, and inserting it under "" would make the next failed load appear
    // to be the same open project. ✅ Returns false in that case.
    bool registerSession(ProjectSession* session);

    // Remove a session by projectID. ✅ Safe when absent.
    void deregister(const QString& projectID)
    {
        sessions_.remove(projectID);
    }

private:
    // projectID → live session (borrowed, never owned).
    QHash<QString, ProjectSession*> sessions_;
};
