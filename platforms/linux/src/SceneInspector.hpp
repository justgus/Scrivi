#pragma once

#include <QWidget>

class QTabWidget;

// SceneInspector — the Linux Scene Inspector panel (EP-024 / SP-078, T-0318).
//
// The Linux mirror of Apple's SceneInspectorView (EP-014). A hideable, tabbed
// side panel that docks to the RIGHT of the manuscript surface in EditorShell
// (the third pane of its QSplitter). This sprint is a UI SKELETON — the panel
// and its tab structure are in place but wired to NO project data:
//
//   • a QTabWidget with one tab, "Scene Entities";
//   • that tab's body is a stub — a title, a muted "No entities yet." empty
//     state, and a DISABLED "Add Entity" button (all placeholder UI).
//
// The tab container is a QTabWidget precisely so future tabs (relationships,
// notes, …) add with a single addTab() call and no structural refactor — the
// same extensibility Apple's InspectorTab enum gives the SwiftUI side.
//
// Width parity with Apple: 240px minimum, 280px default. No scrivi_* calls, no
// SceneDocument coupling — the panel holds no writer data yet. Visibility is
// owned by EditorShell (setInspectorVisible), toggled from View ▸ Show Inspector
// (SP-078, T-0320).
class SceneInspector : public QWidget
{
    Q_OBJECT

public:
    explicit SceneInspector(QWidget* parent = nullptr);

private:
    QTabWidget* tabs_ = nullptr;

    // Build the stub "Scene Entities" tab body (title + empty state + disabled
    // Add Entity button). Its own method so a future data-backed version swaps in
    // cleanly.
    QWidget* buildSceneEntitiesTab();
};
