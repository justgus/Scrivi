#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QWidget>

#include "InspectorLayoutStore.hpp"

class QLabel;
class QLineEdit;
class QTabWidget;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class ScriviBridge;

// SceneInspector — the Linux Scene Inspector panel.
//
// EP-024 / SP-078 (T-0318) built a UI SKELETON wired to no project data.
// EP-035 / SP-125 (T-0481–T-0483) gave it the scene's real objects — the first
// object surface the Linux app ever had. EP-035 / SP-126 (T-0485–T-0489) gives
// it the THREE-TAB SHELL it should always have had.
//
// A hideable, tabbed side panel docking to the RIGHT of the manuscript surface in
// EditorShell (the third pane of its QSplitter). Visibility is owned by
// EditorShell (setInspectorVisible), toggled from View ▸ Show Inspector (T-0320).
//
// ## ⚠️ "Scene Entities" was never a real tab
//
// SP-125 wired a tab named "Scene Entities". ⚠️ **That is the name of Apple's
// SP-090 PLACEHOLDER, which Apple itself deleted** — `SceneInspectorView.swift:6`
// records SP-090 as *"replaces the SP-090 placeholder (a single segmented tab
// over a stub 'Scene Entities' body)"*. ⚠️ **EP-024 copied the placeholder rather
// than its successor**, and SP-125 then wired the copy. It retires here.
//
// ## The three tabs, and two different orderings
//
// ⚠️ **DISPLAY order is Writing | Worldbuilding | Properties**, defaulting to
// Writing. That is Apple's `InspectorTab` DECLARATION order
// (InspectorCard.swift:19-22), which is what `allCases` — and therefore the tab
// bar (SceneInspectorView.swift:100) — iterates.
//
// ⚠️ **BUILD order was Writing → Properties → Worldbuilding** (user ruling): the
// cheap surfaces prove the shell before the expensive one sits on it. ⚠️ **These
// are different orderings and must not be conflated** — SP-126's own draft got
// the display order wrong by assuming they were the same.
//
// ⚠️ **The selected tab persists at PROJECT level and does NOT follow the scene.**
// Switching scenes reloads the current tab's content for the new scene; it never
// changes which tab is showing.
//
// ## The read path — ASK THE SCENE, never infer
//
// Mirrors Apple's ObjectCardModel.load() (Worldbuilding Object Model v0.2 §6)
// exactly, and for the same reason: nothing is inferred from scene text.
//
//   1. `listEdgesFor(sceneID)`  — the scene's edges, each carrying the far
//      endpoint already resolved (its ID, display name, kind, world and label).
//   2. `listObjects(root, "")`  — one listing per load, to confirm each far
//      endpoint and pick up its image path, rather than a lookup per edge.
//   3. Group by kind, with the kind partition DERIVED from ObjectKindScope
//      (T-0480) — never restated here.
//
// ⚠️ **The label travels on the edge ALREADY RESOLVED for the queried endpoint.**
// `listEdgesFor` does the direction projection in the core; recomputing it in Qt
// is the same defect class as restating the kind list, and it is explicitly
// called out as a known trap for this Epic.
//
// ⚠️ **A pending endpoint is absent from the object listing** — its world is
// away — so it CANNOT be confirmed against the index. Dropping unconfirmed edges
// would silently HIDE pending objects, which §7.2 forbids: show it, name it,
// refuse to modify it. A pending edge is admitted on its own `otherKind`, which
// is the binding's cached kind (Apple's I-0124: checking the CARD's kind instead
// of the far endpoint's put locations under Characters with a world away).
//
// ## Three distinct states, never one blank panel (T-0483)
//
// A scene with no objects, a world that is unavailable, an object that is
// pending, and a core that could not be reached are DIFFERENT conditions and are
// each explained in their own words. Absence is never deletion.
class SceneInspector : public QWidget
{
    Q_OBJECT

public:
    explicit SceneInspector(QWidget* parent = nullptr);

    // Hand the panel its project context. Called by EditorShell once the project
    // is open, before any scene is set. A null bridge or empty root leaves the
    // panel in its "no project" state rather than showing a bare empty list.
    void setContext(ScriviBridge* bridge, const QString& projectRootPath);

    // Follow the active scene. EditorShell routes BOTH the caret hook and the
    // scroll hook through selectNavigatorScene(), which is the single existing
    // active-scene notification point — the same hook the navigator highlight and
    // the timeline dot already use, so the inspector stays in sync without a
    // separate wiring.
    //
    // Re-entrant and cheap when the scene has not changed: a repeat of the
    // current sceneID is a no-op, so scroll traffic does not re-hit the backend.
    void setScene(const QString& sceneID);

    // Re-read the current scene's objects from disk. For callers that changed the
    // graph or the object set behind the panel's back.
    void reload();

private:
    // The tabs, in Apple's DISPLAY order. Index order here IS the on-screen
    // order, so it must stay Writing, Worldbuilding, Properties.
    enum Tab { TabWriting = 0, TabWorldbuilding = 1, TabProperties = 2 };

    // Tab identifier as the layout file spells it, and the inverse. Kept next to
    // the enum so the two can never drift.
    static QString tabID(int index);
    static int     tabIndex(const QString& id);

public:

protected:
    // ⚠️ QTextEdit has NO editingFinished signal — unlike QLineEdit — so the
    // outline and todo fields commit on FOCUS-OUT, caught here. Without this
    // they only ever save when the scene changes, and a writer who types a note
    // and then clicks into the manuscript loses it. (Found by typing into the
    // real panel; the build was green and every smoke passed.)
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    // A writer asked to open an object — by double-click or by the context menu
    // (T-0482: both affordances, no gesture-only path). EditorShell owns what
    // "open" means; this panel only reports the request.
    //
    // ⚠️ `worldID` is EMPTY for a project-scoped object and MUST be threaded
    // through every subsequent call — omitting it is how SP-104 blocked object
    // creation outright.
    void openObjectRequested(const QString& objectKind,
                             const QString& objectID,
                             const QString& worldID);

private slots:
    void onItemActivated(QTreeWidgetItem* item, int column);
    void onContextMenuRequested(const QPoint& pos);

    // Tab changed by the writer. Persists the choice at PROJECT level and loads
    // the newly-shown tab's content for the current scene.
    void onTabChanged(int index);

    // --- Writing tab (T-0487) -------------------------------------------
    // Each commits on focus-out / editing-finished rather than per keystroke:
    // a scene note is not worth a disk write per character, and the scene-switch
    // flush below is what guarantees nothing is lost.
    void onTagsEdited();
    void onOutlineEdited();
    void onTodoEdited();

private:
    // One row as the panel renders it — the far endpoint of one of the scene's
    // edges, already resolved by the core.
    struct Entry {
        QString edgeID;
        QString objectID;
        QString kind;
        QString displayName;
        QString label;        // ⚠️ resolved BY THE CORE for this scene; never recomputed
        double  sortIndex = 0.0;
        QString worldID;      // "" = project-scoped
        bool    pending = false;
        QString pendingStatus;  // core's WorldStatus name when pending
    };

    // Tab bodies, built once in the constructor.
    QWidget* buildWritingTab();
    QWidget* buildWorldbuildingTab();
    QWidget* buildPropertiesTab();

    // Load the scene's notes once and populate BOTH the Writing and Properties
    // tabs from that single read — they are two views of one `getSceneNotes`
    // result, so reading twice would be pure waste.
    void loadSceneNotes();

    // Enable/disable the Writing fields. ⚠️ Disabled IN FACT when the scene's
    // notes could not be read — typing into a field we failed to load would
    // save over notes we never saw.
    void setWritingEnabled(bool on);

    // ⚠️ Flush any pending Writing-tab edit to disk NOW. Called before the scene
    // changes, because the commit-on-focus-out model otherwise loses an edit
    // when the writer clicks straight from a note into another scene.
    void flushPendingEdits();

    // Rebuild the tree from `entries_`, grouped by kind in the core's kind order.
    void rebuildTree();

    // Set the explanatory line beneath the tree. Empty text hides it.
    void setStatusLine(const QString& text, bool warning);

    // Writer-facing phrasing for a core WorldStatus name. Mirrors Apple's
    // WorldStatus.writerDescription (ScriviError.swift:115) word for word, so the
    // two platforms describe the same condition identically.
    //
    // ⚠️ Never says "missing" unless the core positively established it: a wrong
    // "missing" is materially worse than an honest "unavailable", because only
    // "missing" invites destructive remedies.
    static QString writerDescription(const QString& statusName);

    // Resolve a worldID to its display name for the pending line, falling back to
    // the ID. ⚠️ An unnamed world is still better than an unattributed warning.
    QString worldDisplayName(const QString& worldID);

    QTabWidget*  tabs_    = nullptr;
    QTreeWidget* tree_    = nullptr;   // Worldbuilding: the scene's objects
    QLabel*      title_   = nullptr;
    QLabel*      status_  = nullptr;   // the explained empty/pending/error line

    // --- Writing tab widgets (T-0487) ------------------------------------
    QLineEdit*  tagsEdit_    = nullptr;   // comma-separated; the core owns the array
    QTextEdit*  outlineEdit_ = nullptr;
    QTextEdit*  todoEdit_    = nullptr;   // one item per line, "[x] " = done
    QLabel*     writingStatus_ = nullptr;

    // --- Properties tab widgets (T-0488) ---------------------------------
    // ⚠️ Every one is READ-ONLY IN FACT, not merely styled — see the .cpp.
    QLabel* propTitle_    = nullptr;
    QLabel* propWords_    = nullptr;
    QLabel* propChars_    = nullptr;
    QLabel* propCreated_  = nullptr;
    QLabel* propModified_ = nullptr;
    QLabel* propStatus_   = nullptr;

    // Guards the programmatic population of the Writing tab, so filling the
    // widgets from disk does not immediately look like a writer edit and
    // schedule a save.
    bool loadingNotes_ = false;

    ScriviBridge* bridge_ = nullptr;
    QString projectRootPath_;
    QString sceneID_;

    // Tab selection persists HERE, at project level (T-0486).
    InspectorLayoutStore layout_;

    QList<Entry> entries_;
    // worldID → display name, refreshed per load from listWorlds.
    QHash<QString, QString> worldNames_;
    // Set when the last load failed outright, so an empty tree is never reported
    // as "no objects" when it is really "we could not read them".
    QString loadError_;
    // The edges read, but the object index did not. Rows are drawn from the graph
    // alone and are UNCONFIRMED — a distinct state from both "empty" and
    // "unreadable", and reported as its own sentence.
    bool indexUnreadable_ = false;
};
