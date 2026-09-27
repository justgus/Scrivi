#pragma once

#include <QHash>
#include <QSet>
#include <QString>

#include "SceneDocument.hpp"

class ScriviBridge;

// ProjectSession — ALL per-open-project state for the Linux app
// (EP-043 / SP-145, T-0551).
//
// ## ✅ WHAT THIS IS
//
// The Qt analogue of Apple's `ProjectSession` (`Scrivi/App/ProjectSession.swift`,
// EP-018 / T-0192). ⚠️ ONE INSTANCE PER OPEN PROJECT. It owns everything specific
// to a single open project; ⛔ it owns NO widgets and does NO layout.
//
// ## ⚠️ WHY IT EXISTS — the split this Sprint is for
//
// ⚠️ `EditorShell` is 2,783 lines and is BOTH the widget and the project's state
// holder: ONE `bridge_`, ONE `projectPath_`, ONE `sceneDoc_`. ⛔ A second project
// has nowhere to go, which is [I-0178].
//
// ✅ The split line is deliberately simple, and it is the one Apple drew:
//   • `ProjectSession` owns **state the PROJECT has** — identity, paths, the
//     assembled document, which scenes are dirty, where the caret is.
//   • `EditorShell` keeps **the widgets SHOWING it** — viewport, navigator,
//     splitters, inspector, timeline, the progress row.
//
// ⚠️ [SP-146] then SHOWS one session per window. ⛔ THIS Sprint does not: there is
// still exactly one session at a time, and `EditorShell` still holds it.
// ⚠️ NOTE THE OWNERSHIP DIRECTION, because it is the point of the Epic: in [SP-146]
// the sessions are owned by an app-level `AppEnvironment` (which Linux does not yet
// have) and a window is HANDED one — ⛔ the window does NOT own the project's
// identity. ✅ Apple does exactly this (`AppEnvironment.makeSession()`), and the
// current inversion is what makes a second project impossible.
// ✅ Scoped in `docs/Epics/Epic-EP-043.md` §The app-level owner.
//
// ## ⚠️ IDENTITY IS `projectID`, NOT PATH — [EP-043] [R-Q2]
//
// ✅ `projectID` is the key everything else hangs from: the registry keys by it,
// and [SP-147] will key persisted geometry by it. ⚠️ `projectPath` is an
// ATTRIBUTE — the thing that RESOLVES the id to disk — ⛔ not the identity.
// ⚠️ Keying by path is wrong the moment a project moves; keying by id means a
// moved project keeps its geometry and is merely skipped for one launch.
// ✅ Linux already had the habit: `timeline-view.ini` keys `timeline/<projectID>`.
//
// ## ⚠️ BEHAVIOUR-PRESERVING — what this class must NOT become
//
// ⛔ This class does NOT open projects, save scenes, talk to the core, or decide
// anything. ✅ It HOLDS state; `EditorShell` still drives every operation.
// ⚠️ That is what makes [SP-145] reviewable: the extraction can be read as "these
// members moved", with no behaviour to re-verify. ⛔ Adding logic here would
// forfeit that, and [EP-018]'s equivalent Sprint carried the same mandate
// ("behavior-preserving", T-0192).
//
// ⚠️ `bridge_` is held as a NON-OWNING pointer. ✅ `EditorShell` creates and
// parents it (Qt object ownership), exactly as before — ⛔ moving that ownership
// here would be a lifetime change, which is not behaviour-preserving.
class ProjectSession
{
public:
    ProjectSession() = default;

    // ⚠️ NON-COPYABLE, NON-MOVABLE, AND SAID SO EXPLICITLY.
    //
    // ✅ It is already non-copyable incidentally: `SceneDocument` holds a
    // `QTextDocument` BY VALUE, and a QObject cannot be copied. ⛔ Relying on that
    // would leave the guarantee resting on a detail of a DIFFERENT class — if
    // `SceneDocument` ever held its document by pointer, this type would silently
    // become copyable and a copied session would be a second holder of one
    // project's identity.
    // ⚠️ It ALSO matters for the [SP-145] seam: `EditorShell` binds references INTO
    // this object, so it must never be moved out from under them.
    ProjectSession(const ProjectSession&)            = delete;
    ProjectSession& operator=(const ProjectSession&) = delete;
    ProjectSession(ProjectSession&&)                 = delete;
    ProjectSession& operator=(ProjectSession&&)      = delete;

    // ---- Identity -------------------------------------------------------
    // ⚠️ The project's ID is its IDENTITY ([R-Q2]). Empty until a load succeeds.
    [[nodiscard]] QString projectID() const { return projectID_; }
    [[nodiscard]] QString projectPath() const { return projectPath_; }
    [[nodiscard]] QString appSupportRoot() const { return appSupportRoot_; }

    // Record identity after a successful open. ⚠️ `projectID` may be empty when a
    // load failed; callers already tolerate that (the save path checks it).
    void setIdentity(const QString& projectID,
                     const QString& projectPath,
                     const QString& appSupportRoot)
    {
        projectID_      = projectID;
        projectPath_    = projectPath;
        appSupportRoot_ = appSupportRoot;
    }

    // ---- Mutable references — THE [SP-145] SEAM ONLY ---------------------
    //
    // ⚠️ THESE EXIST FOR THE EXTRACTION AND NOTHING ELSE. `EditorShell` binds its
    // long-standing member NAMES (`projectID_`, `projectPath_`, …) to these, so
    // ~300 existing statements keep compiling and provably keep their behaviour
    // while the STATE itself lives here.
    //
    // ⛔ DO NOT CALL THESE FROM NEW CODE. ✅ New code uses the value accessors
    // above and `setIdentity()`. ⚠️ Handing out a mutable reference to private
    // state is a real encapsulation cost, accepted DELIBERATELY and TEMPORARILY
    // because the alternative — rewriting ~300 call sites in the same Sprint that
    // promises to preserve behaviour — would make a genuine regression invisible
    // in the diff. ✅ [SP-146] removes the bindings and these can go with them.
    [[nodiscard]] QString& projectIDRef()      { return projectID_; }
    [[nodiscard]] QString& projectPathRef()    { return projectPath_; }
    [[nodiscard]] QString& appSupportRootRef() { return appSupportRoot_; }
    [[nodiscard]] int&     activeSegmentRef()  { return activeSegment_; }

    // ⚠️ `appSupportRoot` is app-global, not per-project — but it is injected and
    // stored here because every scrivi_* call needs it alongside the project path,
    // and threading it separately through the session's users would be noise.
    // ✅ Apple does the same (`ProjectSession.appSupportRoot`, injected, read-only).
    void setAppSupportRoot(const QString& root) { appSupportRoot_ = root; }

    [[nodiscard]] bool hasProject() const { return !projectPath_.isEmpty(); }

    // ---- The bridge (NOT owned) -----------------------------------------
    // ⚠️ Owned by `EditorShell` via Qt parenting; this is a borrowed pointer.
    [[nodiscard]] ScriviBridge* bridge() const { return bridge_; }
    void setBridge(ScriviBridge* bridge) { bridge_ = bridge; }

    // ---- The assembled manuscript ---------------------------------------
    // ⚠️ Non-const accessor: `SceneDocument` is mutated constantly (splices on
    // every structural edit, offset maintenance on every keystroke).
    [[nodiscard]] SceneDocument&       sceneDoc()       { return sceneDoc_; }
    [[nodiscard]] const SceneDocument& sceneDoc() const { return sceneDoc_; }

    // ---- Dirty-scene tracking (T-0238/T-0239) ---------------------------
    // sceneIDs whose body changed since the last save. Drained by the shell's
    // saveDirtyScenes().
    [[nodiscard]] QSet<QString>&       dirtyScenes()       { return dirtyScenes_; }
    [[nodiscard]] const QSet<QString>& dirtyScenes() const { return dirtyScenes_; }

    // ---- Caret anchoring -------------------------------------------------
    // The segment index the caret last sat in, so a scene switch can be detected
    // and the departing scene saved. -1 = none yet.
    [[nodiscard]] int  activeSegment() const { return activeSegment_; }
    void setActiveSegment(int seg) { activeSegment_ = seg; }

    // ---- Pane visibility — PER-PROJECT STATE (T-0553, [R-Q4]/[R-Q5]) ----
    //
    // ⚠️ THIS STATE IS NEW, AND THAT IS A CORRECTION WORTH READING.
    // ⛔ [R-Q5] was written believing `EditorShell` held `inspectorVisible_` and
    // `timelineVisible_` members that merely needed moving. ⛔ IT DID NOT — and
    // never had. ✅ Visibility was read STRAIGHT OFF THE WIDGETS
    // (`inspector_->isVisible()`), while `EditorShell.cpp`'s own comment NAMED a
    // member that was never added. ✅ So this state is INTRODUCED here.
    //
    // ⚠️ WHY IT BELONGS TO THE PROJECT, not the widget: a writer's choice to hide
    // the inspector is a property of how she is working on THAT manuscript.
    // ✅ Apple agrees for the inspector and persists it (Doc 2 AC4).
    //
    // ⚠️ THE TWO FLAGS ARE DELIBERATELY ASYMMETRIC — ⛔ do not "tidy" this:
    //   • `inspectorVisible` PERSISTS, through the core, in `inspector-layout.json`
    //     ([I-0251]). ✅ Apple does exactly this.
    //   • `timelineVisible` is SESSION-SCOPED. ✅ Apple does not persist it either
    //     (`ProjectSession.swift:98`), and SP-078/T-0320's ruling STANDS for it.
    // ✅ Both default to SHOWN (Apple parity — user decision 2026-07-22).
    [[nodiscard]] bool inspectorVisible() const { return inspectorVisible_; }
    void setInspectorVisible(bool visible) { inspectorVisible_ = visible; }

    [[nodiscard]] bool timelineVisible() const { return timelineVisible_; }
    void setTimelineVisible(bool visible) { timelineVisible_ = visible; }

    // ---- Reset ----------------------------------------------------------
    // Clear per-project state for a fresh load. ⚠️ Does NOT touch `bridge_` or
    // `appSupportRoot_`: both outlive a project change (the bridge is the shell's
    // and is re-bootstrapped against the same app-support root).
    // ⚠️ Pane visibility is deliberately NOT reset here — the next load's layout
    // read sets `inspectorVisible_`, and resetting it first would flash the pane.
    void resetForLoad()
    {
        projectID_.clear();
        projectPath_.clear();
        dirtyScenes_.clear();
        activeSegment_ = -1;
    }

private:
    QString       projectID_;
    QString       projectPath_;
    QString       appSupportRoot_;
    ScriviBridge* bridge_ = nullptr;   // NOT owned — EditorShell parents it
    SceneDocument sceneDoc_;
    QSet<QString> dirtyScenes_;
    int           activeSegment_ = -1;

    // Defaults SHOWN — Apple parity (user decision 2026-07-22).
    bool inspectorVisible_ = true;
    bool timelineVisible_  = true;
};
