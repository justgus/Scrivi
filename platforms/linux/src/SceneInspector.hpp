#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QWidget>

class QLabel;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
class ScriviBridge;

// SceneInspector — the Linux Scene Inspector panel.
//
// EP-024 / SP-078 (T-0318) built the panel and its tab structure as a UI SKELETON
// wired to NO project data. EP-035 / SP-125 (T-0481–T-0483) replaces that stub
// with the scene's real objects — the FIRST object surface the Linux app has
// ever had.
//
// A hideable, tabbed side panel docking to the RIGHT of the manuscript surface in
// EditorShell (the third pane of its QSplitter). Visibility is owned by
// EditorShell (setInspectorVisible), toggled from View ▸ Show Inspector (T-0320).
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

    QWidget* buildSceneEntitiesTab();

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
    QTreeWidget* tree_    = nullptr;
    QLabel*      title_   = nullptr;
    QLabel*      status_  = nullptr;   // the explained empty/pending/error line

    ScriviBridge* bridge_ = nullptr;
    QString projectRootPath_;
    QString sceneID_;

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
