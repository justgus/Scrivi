#pragma once

#include <QPlainTextEdit>

class SceneDocument;

// ManuscriptEditor — the editable continuous writing surface (SP-062, T-0238).
//
// A QPlainTextEdit that edits the one SceneDocument document but PROTECTS the
// scene-boundary regions (chapter headings + separators): those characters are
// presentation, and the offset map is the scene-ownership authority, so the author
// must never be able to type into, delete, or select-across a boundary. This is
// the Qt analogue of the non-editable/non-deletable virtual separator in Apple's
// ManuscriptTextView.
//
// Enforcement is a keyPressEvent guard: any keystroke that would modify text is
// allowed only if the range it touches lies entirely inside one scene's editable
// body (SceneDocument::isEditableRange / isEditablePosition). Navigation,
// selection, and copy pass through untouched. Paste and drops route through
// insertFromMimeData, which applies the same gate.
//
// Document-level undo stays DISABLED here (setUndoRedoEnabled(false)) — ⌘Z is
// reserved for the future custom history (EP-026).
//
// The editor holds a non-owning pointer to the SceneDocument (owned by EditorShell)
// and consults it read-only; it never rebuilds the map. Offset-map maintenance on
// accepted edits happens in EditorShell via the document's contentsChange signal.
class ManuscriptEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ManuscriptEditor(QWidget* parent = nullptr);

    // Point the guard at the active SceneDocument (call after each load()). Passing
    // nullptr disables the guard (nothing is editable / everything passes through —
    // used only in the read-only state before a project is loaded).
    void setSceneDocument(SceneDocument* doc) { sceneDoc_ = doc; }

signals:
    // Ctrl+Return — the Linux analogue of Apple's ⌘↩ "new scene" (T-0240). The
    // editor does NOT insert a newline; EditorShell handles the create.
    void createSceneRequested();
    // Ctrl+Shift+Return — the analogue of ⌘⇧↩ "new chapter" (T-0241).
    void createChapterRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void insertFromMimeData(const QMimeData* source) override;

private:
    // True if the given key event would modify document text (typed char, Enter,
    // Backspace, Delete, cut, paste-shortcut) as opposed to pure navigation/copy.
    static bool isModifyingKey(const QKeyEvent* event);

    // For a modifying key with no selection, the half-open range [start,end) it
    // would change. Backspace → [pos-1,pos); Delete → [pos,pos+1); insertion →
    // [pos,pos). With a selection every modifying key replaces the selection.
    // Returns false if the guard should simply block (e.g. nothing to check).
    bool modifiedRangeFor(const QKeyEvent* event, int& start, int& end) const;

    SceneDocument* sceneDoc_ = nullptr;   // non-owning
};
