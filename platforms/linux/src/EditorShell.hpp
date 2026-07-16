#pragma once

#include <QSet>
#include <QString>
#include <QWidget>

#include "SceneDocument.hpp"

class NavigatorTree;
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

protected:
    // Give the writing surface keyboard focus when the editor page becomes visible
    // (T-0246) — the QStackedWidget swaps to the editor after load(), so focusing
    // here (not just in load()) reliably lands focus on the QPlainTextEdit.
    void showEvent(QShowEvent* event) override;

signals:
    // The user asked to leave the editor (‹ Close). The shell returns to landing.
    void closeRequested();

private slots:
    void onNavigatorActivated(const QModelIndex& index);
    // Right-click on a navigator row (T-0251): build a context menu with Delete for
    // the row under the cursor — scene rows and chapter rows both. `pos` is in the
    // navigator viewport's coordinates.
    void onNavigatorContextMenu(const QPoint& pos);
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
    // Ctrl+Shift+Return in the editor (T-0261, I-0064): SPLIT the current chapter at
    // the caret (macOS ⌘⇧↩ parity, disk-correct). Creates a new chapter K after the
    // current chapter C, reassigns the scenes that followed the caret's scene into K,
    // splits the caret's scene head/tail when mid-scene, renumbers subsequent chapters,
    // and confirms first when a renumber will happen. No longer appends an empty chapter
    // at the manuscript end.
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

    // --- EP-023 delete (T-0251/T-0252) ------------------------------------
    // Delete the scene with `sceneID` after a confirmation dialog: drop it from disk
    // (scrivi_delete_scene), splice it out of the document/map/navigator, and if it was
    // the active scene, promote the nearest remaining scene + refocus the editor.
    void deleteSceneByID(const QString& sceneID);
    // Delete `chapterID` and ALL its scenes after a confirmation that names the count:
    // scrivi_delete_chapter, splice every member out, then re-anchor the active scene.
    void deleteChapterByID(const QString& chapterID);
    // Shared post-delete fixup: rebuild the navigator, then (if the previously active
    // scene is gone) promote `fallbackSeg` clamped into range as active, move the caret
    // to its start, and refocus the editor. `previouslyActiveSceneID` is checked
    // against the surviving segments to decide whether re-anchoring is needed.
    void afterStructuralRemoval(const QString& previouslyActiveSceneID,
                                int fallbackSeg);

    // --- EP-023 rename (T-0255) -------------------------------------------
    // Rename the scene with `sceneID`: prompt (QInputDialog) pre-filled with the current
    // title, call the bridge (sidecar `title`), update the map + navigator label. A
    // blank/whitespace title clears the custom title (navigator then falls back to the
    // first prose line).
    void renameSceneByID(const QString& sceneID);
    // Rename `chapterID`: prompt pre-filled with the current chapter title, call the
    // bridge on the chapter's metadata path, update every member's `chapterTitle` +
    // rewrite the live heading in place. Blank/whitespace → falls back to "Chapter N".
    void renameChapterByID(const QString& chapterID);
    // Editor focus back to the writing surface after a modal rename dialog closes.
    void refocusEditor();

    // --- EP-023 I-0063 renumber created chapters (T-0262) -----------------
    // After any chapter structural change (delete / insert / split / reorder), walk the
    // chapters in manuscript order and, for each whose STORED title matches the auto
    // pattern `^Chapter \d+$`, renameChapter it (via the bridge, on its chapterMetadataPath)
    // to its correct 1-based ordinal — so a created chapter's stored "Chapter N" tracks
    // its position. Custom titles (anything not matching the pattern) are left alone;
    // untitled chapters already renumber for free via chapterHeadingText. Runs under the
    // loading_ guard. Returns true if any chapter was renamed (the caller should
    // re-derive labels + rebuild the navigator afterward).
    bool renumberCreatedChapters();
    // Re-fetch the backend-derived scene + chapter titles from scrivi_open_project and
    // apply them to the SceneDocument (scene labels via setSceneTitle; each chapter's
    // title via setChapterTitle, which also rewrites the live heading in place). Used
    // after a rename so the live navigator/heading match exactly what a reload shows —
    // including the fallback chain (blank custom title → first prose line / "Chapter N"),
    // which ScriviCore owns. Runs under the loading_ guard (no dirty churn).
    void applyDerivedLabels();

    // --- EP-023 scene reorder (T-0260 / SP-067, AC4) ----------------------
    // A navigator scene row was dropped: move the dragged scene into targetChapterID
    // right after afterSceneID (empty = first in the target chapter). Saves dirty
    // scenes, calls scrivi_reorder_scene, re-splices via SceneDocument::moveScene,
    // renumbers created chapters (I-0063), rebuilds the navigator, and restores the
    // caret/active scene to the moved scene.
    void onSceneDropped(const QString& draggedSceneID,
                        const QString& targetChapterID,
                        const QString& afterSceneID);

    ScriviBridge*       bridge_    = nullptr;   // owns its own bootstrapped bridge
    ManuscriptEditor*   viewport_  = nullptr;
    NavigatorTree*      navigator_ = nullptr;
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
