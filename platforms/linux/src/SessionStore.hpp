#pragma once

#include <QList>
#include <QRect>
#include <QString>

// SessionStore — what was open, where it was, how it was arranged
// (EP-043 / SP-147, T-0565).
//
// ## ✅ WHAT THIS IS
//
// The Qt counterpart of Apple's `OpenSessionManifest` + `ProjectWindowFrameStore`
// (EP-018). ⚠️ Apple keeps them as two types; ✅ Linux keeps ONE, because [R-Q1]
// ruled ONE FILE — `<appSupportRoot>/session.ini` via `QSettings` — and splitting
// the reader would put two owners on one file, which is the [I-0215] class.
//
// ## ⚠️ KEYED BY `projectID`, WITH `path` AS AN ATTRIBUTE — [R-Q2]
//
// ✅ `[project/<projectID>]` holds `path`, `frame`, `maximized`, `splitters`.
// ⛔ NOT keyed by path: ⚠️ a moved project must keep its geometry and merely be
// SKIPPED for one launch, and two copies at different paths are the same project.
// ✅ Linux already had the habit — `timeline-view.ini` keys `timeline/<projectID>`.
//
// ## ⛔ WHY `<appSupportRoot>` AND NOT Qt's DEFAULT
//
// ⚠️ `QSettings`'s default path is `~/.config`, ⛔ which the Docker/VNC harness WIPES
// on `docker run --rm` — ✅ app-support is the directory it bind-mounts.
// ⚠️ `EditorShell::timelineViewStatePath()` (`:2880`) already records this, having
// been caught by it once: *"zoom/pan not saved"*.
//
// ## ⚠️ EVERY WRITE CALLS `sync()`
//
// ⛔ A `--rm` container may SIGKILL before Qt's lazy flush. ✅ Same habit, same
// reason, as `onTimelineViewStateChanged` (`EditorShell.cpp:2906`).
//
// ## ⛔ THIS CLASS OWNS NO WINDOWS AND DOES NO RESTORING
//
// ✅ It reads and writes state. ⚠️ Deciding what to reopen is the app environment's
// ([T-0567]); ⛔ keeping those apart is what makes this testable without a display.
class SessionStore
{
public:
    // One recorded project. ⚠️ `frame` is invalid when nothing was stored — ✅ the
    // caller then leaves the window at its default size rather than forcing 0×0.
    struct Entry {
        QString     projectID;
        QString     path;
        // ⚠️ [SP-147] T-0567 — the title the window showed, so a RESTORED window can
        // show it too. ✅ Restore has no Landing envelope to take it from.
        QString     title;
        QRect       frame;
        bool        maximized = false;
        // ⚠️ Splitter proportions, as the INTEGER SIZES `QSplitter::sizes()` returns.
        // ✅ Stored per splitter: `panes` is the 3-pane row, `outer` is panes-over-
        // timeline. ⛔ Empty means "never stored" — the caller keeps the defaults.
        QList<int>  paneSizes;
        QList<int>  outerSizes;
    };

    explicit SessionStore(QString appSupportRoot)
        : appSupportRoot_(std::move(appSupportRoot))
    {
    }

    // ---- Reading ---------------------------------------------------------

    // Every recorded project, in no significant order.
    // ⚠️ [SP-147] AC4's caller filters by whether `path` still resolves; ⛔ this does
    // NOT filter, because a store that silently drops rows cannot be reasoned about.
    [[nodiscard]] QList<Entry> entries() const;

    // One project's record, or a default-constructed Entry when absent.
    [[nodiscard]] Entry entry(const QString& projectID) const;

    // ---- Writing ---------------------------------------------------------

    // Record (or update) a project as OPEN, with its window state.
    // ⚠️ An empty `projectID` is IGNORED — ✅ the same guard `OpenProjectRegistry`
    // applies: a failed load has no identity and must not become a phantom row.
    void record(const Entry& entry);

    // Mark a project as OPEN at `path`, touching NOTHING else (T-0566).
    // ⚠️ Called when a project finishes loading. ⛔ `record()` is wrong there: it
    // writes `maximized` unconditionally, so recording a fresh window's DEFAULT
    // state would overwrite the geometry this project was last closed with.
    // ✅ Writing `open` at load time — not only at quit — means a SIGKILLed
    // session still knows what was open; only its geometry is lost.
    void setOpen(const QString& projectID, const QString& path, const QString& title);

    // Mark a project as no longer open. ⛔ DOES NOT DELETE ITS GEOMETRY — ⚠️ [R-Q2]:
    // a project the writer closes today should reopen where she left it tomorrow.
    void setClosed(const QString& projectID);

    // The projectIDs that were open at the last quit, for the restore pass.
    [[nodiscard]] QList<QString> openProjectIDs() const;

    // ⛔ TEST SUPPORT ONLY — wipes the file. ⚠️ Never called by the app.
    void clearAll();

    [[nodiscard]] QString filePath() const;

private:
    QString appSupportRoot_;
};
