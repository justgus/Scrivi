#pragma once

#include <QSet>
#include <QString>
#include <QWidget>

#include "SceneDocument.hpp"

class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QLabel;
class QTimer;
class ScriviBridge;
class ManuscriptEditor;

// EditorShell — the Linux editor screen (SP-061 T-0235/0236, SP-062 T-0238…).
//
// Replaces EP-021's placeholder ProjectWindow. A native Qt Widgets screen with:
//   • a scene NAVIGATOR (QTreeView: chapter parent rows → scene child rows, in
//     manuscript order, live titles) — display + selection only (structure editing
//     is EP-023);
//   • a continuous VIEWPORT — every scene body concatenated into ONE
//     ManuscriptEditor / QTextDocument (undo disabled), with per-scene start offsets
//     kept in SceneDocument (the sceneStartMap seed).
//
// On load() it re-opens the project through its own ScriviBridge (the same
// endpoints EP-021's landing already used), reads the ordered scenes[] + the
// activeScene body from scrivi_open_project, then loops scrivi_open_scene for the
// remaining bodies, assembles the document, populates the navigator, and scrolls
// to the initial active scene.
//
// SP-062 (T-0238) makes the viewport EDITABLE: the ManuscriptEditor gates edits to
// stay inside scene bodies (boundary integrity), and this shell keeps the offset
// map in sync on every edit (contentsChange) and flags the touched scene dirty.
// Persisting those dirty scenes (scrivi_save_scene) is T-0239.
class EditorShell : public QWidget
{
    Q_OBJECT

public:
    explicit EditorShell(QWidget* parent = nullptr);

    // Open `projectPath` into the editor. `appSupportRoot` is the injected stable
    // path; `title` is the display title (from recents). Returns true if the
    // project opened "ready" and the viewport was populated; false otherwise (an
    // error label is shown and closeRequested is NOT emitted — the caller decides).
    bool load(const QString& projectPath,
              const QString& appSupportRoot,
              const QString& title);

    // Flush any pending edits to disk immediately (T-0239). Called by the shell on
    // Close and by the host on app-quit so no edit is lost on the way out — the
    // Docker/VNC quit path in particular (the app is the container's foreground
    // process). Safe to call with nothing dirty (no-op).
    void saveDirtyScenes();

signals:
    // The user asked to leave the editor (‹ Close). The shell returns to landing.
    void closeRequested();

private slots:
    void onNavigatorActivated(const QModelIndex& index);
    // Document edit hook: reconcile the offset map, mark the touched scene dirty
    // (T-0238), and (re)arm the idle-save debounce (T-0239). Guarded against the
    // programmatic build in load().
    void onContentsChange(int position, int charsRemoved, int charsAdded);
    // Caret moved: if it crossed into a different scene, promote it active (which
    // saves the departing scene — the scene-switch leg of the T-0239 cadence).
    void onCursorMoved();
    // Viewport scrolled (T-0243): the visible scene becomes the active scene. Scroll
    // is the authority for "which scene is active"; crossing a boundary promotes the
    // newly-visible scene (and saves the departing one). Guarded against programmatic
    // scrolls (navigator click, promotion-driven moves).
    void onScrolled();
    // Ctrl+Return in the editor (T-0240): save the current scene, create a new one
    // after it in the same chapter, splice it into the document/map/navigator, and
    // drop the caret into its empty body.
    void onCreateSceneRequested();
    // Ctrl+Shift+Return in the editor (T-0241): save the current scene, create a new
    // chapter (appended to the end, with its first scene), splice a chapter-heading
    // boundary + that first scene onto the end of the document/map/navigator, and
    // drop the caret into it.
    void onCreateChapterRequested();

private:
    // Select the navigator row for `sceneID` (highlight only; no scroll, no caret).
    // Used both by a click and by the scroll-driven active-scene follow.
    void selectNavigatorScene(const QString& sceneID);
    // Save one scene's current body via the bridge; returns true on success. Clears
    // the scene from dirtyScenes_ whether or not it was present.
    bool saveScene(int segmentIndex);
    // (Re)populate the navigator tree from the current SceneDocument segments.
    void rebuildNavigator();
    // Move the caret to the start of segment `index`'s body and center it; also
    // reflects the selection in the navigator. Updates activeSegment_.
    void moveCaretToSegment(int index);
    // The segment whose body occupies the vertical MIDDLE of the viewport — the
    // "visible scene" (Apple's midpoint-of-viewport rule). -1 if none/undeterminable.
    int visibleSceneIndex() const;
    // Single writer for the active scene (T-0243): if `newSeg` differs from
    // activeSegment_, save the departing dirty scene(s) and set activeSegment_.
    // Both the caret hook and the scroll hook route through here so they never
    // double-drive or double-save. No-op if newSeg is invalid or unchanged.
    void promoteActiveScene(int newSeg);

    ScriviBridge*       bridge_    = nullptr;   // owns its own bootstrapped bridge
    ManuscriptEditor*   viewport_  = nullptr;
    QTreeView*          navigator_ = nullptr;
    QStandardItemModel* navModel_  = nullptr;
    QLabel*             errorLabel_ = nullptr;
    QTimer*             saveTimer_ = nullptr;   // idle-save debounce (~1.5s)
    SceneDocument       sceneDoc_;

    // Identity of the open project, stashed on load() for the save path.
    QString             projectID_;
    QString             projectPath_;
    QString             appSupportRoot_;

    // Dirty-scene tracking (T-0238): sceneIDs whose body changed since the last
    // save. saveDirtyScenes() (T-0239) drains this set. Populated by onContentsChange.
    QSet<QString>       dirtyScenes_;

    // The segment index the caret last sat in — so onCursorMoved can detect a
    // scene switch and save the departing scene. -1 = none yet.
    int                 activeSegment_ = -1;

    // True while load() is programmatically assembling the document, so the
    // contentsChange / cursor hooks ignore those (non-user) events.
    bool                loading_ = false;

    // Guard against a scroll↔selection feedback loop (T-0243/T-0244): set while the
    // shell is programmatically scrolling/selecting the viewport (navigator click,
    // caret move) so onScrolled doesn't re-promote/re-scroll off its own change.
    bool                programmaticViewportChange_ = false;
};
