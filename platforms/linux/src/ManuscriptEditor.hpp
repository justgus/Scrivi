#pragma once

#include <QPlainTextEdit>

class ManuscriptPresenter;
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

    // EP-048 (SP-166): the presenter that draws this document (non-owning; nullptr = drawn as stored).
    // The editor reads its STOP RUNS to snap the caret (L5) and keep markers atomic (L9), and moves
    // its REVEAL with the selection (L6).
    void setPresenter(ManuscriptPresenter* presenter) { presenter_ = presenter; }

    // Edit ▸ Cut. ⛔ [I-0270] `QPlainTextEdit::cut()` is not virtual and never passes the
    // keyPressEvent edit guard, so a menu cut across a scene break deleted heading and
    // separator text and desynchronised the offset map. Routes through the guard instead.
    void cutSelection();

signals:
    // Ctrl+Return — the Linux analogue of Apple's ⌘↩ "new scene" (T-0240). The
    // editor does NOT insert a newline; EditorShell handles the create.
    void createSceneRequested();
    // Ctrl+Shift+Return — the analogue of ⌘⇧↩ "new chapter" (T-0241).
    void createChapterRequested();
    // Ctrl+Backspace — the Linux analogue of Apple's ⌘⌫ "merge scene into the
    // previous scene" (EP-028 / T-0304). The editor does NOT delete anything;
    // EditorShell resolves the caret, applies the start-of-scene / manuscript-start
    // guards, and calls the merge endpoint.
    void mergeSceneRequested();
    // Ctrl+Shift+Backspace — the analogue of ⇧⌘⌫ "merge chapter into the previous
    // chapter" (EP-028 / T-0304). Same division of labor: EditorShell decides.
    void mergeChapterRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void insertFromMimeData(const QMimeData* source) override;
    // EP-049 (SP-160): copy / cut / drag put the PRESENTED text on the clipboard (Apple: `writeSelection`),
    // and carry the stored source in a private format so Scrivi's own paste restores it exactly.
    QMimeData* createMimeDataFromSelection() const override;
    // EP-049 (SP-160, M2): composed input (dead keys, IME) commits HERE, never through keyPressEvent —
    // so it is escaped here too (Apple's `insertText` covers both).
    void inputMethodEvent(QInputMethodEvent* event) override;
    // Draw the faint between-scene separator rule (EP-028 SP-076, T-0308) — the Linux
    // analogue of Apple's DividerAttachmentCell. Runs AFTER the base class paints text,
    // then strokes a 1px inset line centered in each within-chapter scene-separator gap.
    // Purely visual: no document text or offset-map change (positions come from
    // SceneDocument::sceneSeparatorPositions()).
    void paintEvent(QPaintEvent* event) override;
    // EP-048 (SP-166): no snap and no reveal MID-DRAG (Apple: never while `stillSelecting`) — applied on release.
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    // Keep the caret out of protected boundary text (T-0246): when the cursor lands
    // in a heading/separator gap (via click or arrow navigation), snap it to the
    // nearest editable body position. Re-entrancy-guarded (setTextCursor re-fires
    // cursorPositionChanged). No-op when there's a selection (the user is selecting).
    void normalizeCaret();

private:
    // True if the given key event would modify document text (typed char, Enter,
    // Backspace, Delete, cut, paste-shortcut) as opposed to pure navigation/copy.
    static bool isModifyingKey(const QKeyEvent* event);

    // For a modifying key with no selection, the half-open range [start,end) it
    // would change. Backspace → [pos-1,pos); Delete → [pos,pos+1); insertion →
    // [pos,pos). With a selection every modifying key replaces the selection.
    // Returns false if the guard should simply block (e.g. nothing to check).
    bool modifiedRangeFor(const QKeyEvent* event, int& start, int& end) const;

    // ── EP-049 (SP-160, T-0586): the escape layer's WRITE half — a port of Apple's ──
    // True when document boundary `pos` sits between a hidden backslash and the mark it escapes.
    // ⚠️ Linux SHOWS the backslash, so the caret can rest there; an insertion must not (AC4b).
    bool isInsideEscapePair(int pos) const;
    // Widen [start, end) so it never splits an escape pair (Apple: `snapSelection`).
    void widenOverPairs(int& start, int& end) const;
    // One edit replacing [start, end) with `text` (already in STORED form), caret after it.
    void replaceRange(int start, int end, const QString& text);
    // The escaped-edit paths; each returns true when it handled the key.
    bool handleReturn(QKeyEvent* event);
    bool handleDeletion(QKeyEvent* event);
    bool handleTyping(QKeyEvent* event);
    // Apple's `paragraphJoin` (Q3): the range to replace with one space, or false.
    bool paragraphJoinRange(int loc, int& start) const;

    // [I-0270] Removes [start, end)'s text from each scene it spans, keeping the scenes.
    // `copyFirst` copies the selection to the clipboard first (a cut). Returns false —
    // and changes nothing — when the range does not span two or more scene bodies.
    bool deleteAcrossScenes(int start, int end, bool copyFirst);

    // ── EP-048 (SP-166): stop runs (Apple: `MarkdownEscapes.snapCaret` / `snapSelection`) ──
    // Where a caret proposed at `loc`, coming from `previous`, must land instead (or `loc`).
    int snapCaret(int loc, int previous) const;
    void logCaret(int previous, int qtPos, int editable, int snapped) const;   // SP-166 measurement (SCRIVI_CARET_LOG)
    int snapOnce(int loc, int previous) const;
    bool isUnreachable(int pos) const;
    // A selection never ends inside a stop run; the end moves back when the selection SHRANK.
    void snapSelection(int& start, int& end, int previousEnd) const;
    // ⌫ / ⌦ next to a marker act on what the writer SEES (L9 part); true when handled.
    bool handleAtomicDeletion(bool back, int loc);

    SceneDocument* sceneDoc_ = nullptr;   // non-owning
    ManuscriptPresenter* presenter_ = nullptr;   // non-owning
    bool mouseSelecting_ = false;
    int lastSelEnd_ = 0;
    bool normalizingCaret_ = false;       // re-entrancy guard for normalizeCaret()
    int  lastCaretPos_ = 0;               // previous caret pos → gives normalizeCaret()
                                          // the direction of travel across a boundary gap
};
