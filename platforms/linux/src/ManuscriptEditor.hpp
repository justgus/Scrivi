#pragma once

#include <QPlainTextEdit>

#include <functional>
#include <utility>

#include "MarkdownAnalysis.hpp"

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
// Document-level undo stays DISABLED here (setUndoRedoEnabled(false)) — Ctrl+Z is the
// custom history's (EP-026 S2, SP-178): the editor signals undoRequested / redoRequested.
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

    // ── SP-175 / T-0602 (EP-048 S3b): the Format commands (Apple: `ManuscriptNSTextView.applyFormat`) ──
    // ✅ The edits are ScriviCore's (`scrivi_markdown_*`, SP-174) on the scene holding the selection; each is ONE
    // document edit, then the selection it returns. A command that cannot be done — across a boundary, nothing
    // visible to format, or one that could not be written balanced — beeps and changes nothing.
    enum class Format { Bold, Italic, Heading1, Heading2, Heading3, Body, BulletList, NumberedList };
    void applyFormat(Format format);

    // Take the PENDING pair out again (✅ [SP-174] Q5): document only, never scene text, never dirty. Called when the
    // caret leaves it, and by the shell before any structural edit (Apple: a storage rebuild drops it).
    void removePendingPair();

    // ── EP-026 S5 (SP-179 / T-0606): the copy buffers — Apple's `copyIntoBuffer` / `cutIntoBuffer` / `pasteFromBuffer`
    // (`ManuscriptTextView.swift`). The editor does the text; `EditorShell` talks to the slots (`BufferService`). ──
    // What a slot gets for the selection: the BALANCED stored source Ctrl+C keeps for Scrivi's own paste ([I-0289], Q2) —
    // across scenes, the flat text with its headings and separators ([SP-179] Q1). Empty when nothing is selected.
    [[nodiscard]] QString selectionSource() const;
    // Cut the selection into a slot: `store(source)` FIRST (a cut must never lose its text), then the balanced removal as
    // ONE `cut` step tagged with `bufferID` — across scenes, [I-0270]'s per-scene removal, one grouped `cut` step (Apple's
    // shape: the grouped step carries no slot). Changes nothing when nothing removable is selected or `store` fails.
    bool cutSelectionIntoBuffer(const QString& bufferID, const std::function<bool(const QString& source)>& store);
    // Paste a slot's STORED text verbatim (never re-escaped, EP-045 AC4) over the selection: ONE `paste` step. A slot is
    // always Scrivi's own copy, so it keeps ITS formatting where it lands ([I-0289], [SP-163]). False when refused.
    bool pasteStored(const QString& source);
    // The digit 1–9 the key event's PHYSICAL key carries, or 0. ⚠️ Ctrl+Shift+1 arrives as `!` (the shifted key), so the
    // scan code decides; the US-layout shifted marks are the fallback when there is none (offscreen tests).
    [[nodiscard]] static int bufferDigit(const QKeyEvent* event);

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

    // ── EP-026 S2 (SP-178 / T-0605): the undo history ──
    // Ctrl+Z / Ctrl+Shift+Z (✅ EP-026 Q3; ⛔ no Ctrl+Y). Caught HERE because the editor claims both through
    // ShortcutOverride before an Edit-menu shortcut can see them; EditorShell undoes / redoes.
    void undoRequested();
    void redoRequested();
    // A paste or an in-scene cut is about to change the text / has changed it (Apple: `beginPasteOrCut` … `flush`).
    // `kind` is "paste" | "cut".
    // `bufferID` tags a cut INTO a copy buffer (S5); empty otherwise.
    void pasteOrCutBegins(const QString& kind, const QString& bufferID);
    void pasteOrCutEnded(const QString& kind);
    // [I-0270] ONE edit across several scenes is about to run / has run, one `contentsChange` per scene in between —
    // EditorShell reports it as ONE grouped undo step. `kind` is "delete" | "cut".
    void groupedEditBegins();
    void groupedEditEnded(const QString& kind);

    // ── EP-026 S5 (SP-179): the buffer chords (✅ EP-026 Q3: Ctrl+N copy · Ctrl+Shift+N paste · Alt+N cut). Caught HERE,
    // each claimed in ShortcutOverride first; EditorShell acts on the slot. `bufferID` is "1"–"9". ──
    void bufferCopyRequested(const QString& bufferID);
    void bufferPasteRequested(const QString& bufferID);
    void bufferCutRequested(const QString& bufferID);

protected:
    // Claims the buffer chords in ShortcutOverride, so no window shortcut takes them while the editor has focus.
    bool event(QEvent* event) override;
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
    // ✅ SP-175 (EP-046 AC12, Apple's `shouldChangeText`): an edit that would leave emphasis unbalanced is re-issued as
    // the core's BALANCED edit of the stretch it touches — still ONE edit. Never for a computed edit (`computed_`).
    void replaceRange(int start, int end, const QString& text);
    // Apply an edit the core computed for the scene starting at document position `base`, then its selection.
    void applyComputed(int base, const MarkdownEdit& edit);
    // The pending pair (Q1 of SP-163): `markers` twice at `pos`, the caret between them.
    void insertPendingPair(int pos, const QString& markers);
    // The caret LEFT the pending pair: remove it, keeping the caret on the same character.
    void leavePendingPair();
    // The document text of [start, end), newlines as '\n'.
    QString textOf(int start, int end) const;
    // The escaped-edit paths; each returns true when it handled the key.
    bool handleReturn(QKeyEvent* event);
    bool handleDeletion(QKeyEvent* event);
    bool handleTyping(QKeyEvent* event);
    // Apple's `paragraphJoin` (Q3): the range to replace with one space, or false.
    bool paragraphJoinRange(int loc, int& start) const;

    // [I-0270] Removes [start, end)'s text from each scene it spans, keeping the scenes.
    // `copyFirst` copies the selection to the clipboard first (a cut). Returns false —
    // and changes nothing — when the range does not span two or more scene bodies.
    // `kind` names the grouped step; empty = "cut" when copying first, else "delete" (S5: a cut into a slot is a "cut").
    bool deleteAcrossScenes(int start, int end, bool copyFirst, const QString& kind = {});
    // The balanced {source, presented} of [start, end) — scene by scene, headings and separators as they are (SP-175).
    [[nodiscard]] std::pair<QString, QString> balancedCopyOf(int start, int end) const;
    // The buffer chord `event` is ({action, digit}), or {0, 0}: action 1 copy · 2 paste · 3 cut.
    [[nodiscard]] static std::pair<int, int> bufferChord(const QKeyEvent* event);

    // ── EP-048 (SP-166): stop runs (Apple: `MarkdownEscapes.snapCaret` / `snapSelection`) ──
    // Where a caret proposed at `loc`, coming from `previous`, must land instead (or `loc`).
    int snapCaret(int loc, int previous) const;
    int snapOnce(int loc, int previous) const;
    bool isUnreachable(int pos) const;
    // A selection never ends inside a stop run; the end moves back when the selection SHRANK.
    void snapSelection(int& start, int& end, int previousEnd) const;
    // ⌫ / ⌦ next to a marker act on what the writer SEES (L9 part); true when handled.
    bool handleAtomicDeletion(bool back, int loc);

    bool computed_ = false;        // applying an edit the core computed — never re-balanced
    bool pastingOwnCopy_ = false;  // Scrivi's own copy is being pasted: it keeps ITS formatting
    bool keepingPending_ = false;  // the editor itself is placing the caret inside the pending pair

    SceneDocument* sceneDoc_ = nullptr;   // non-owning
    ManuscriptPresenter* presenter_ = nullptr;   // non-owning
    bool mouseSelecting_ = false;
    int lastSelEnd_ = 0;
    bool normalizingCaret_ = false;       // re-entrancy guard for normalizeCaret()
    int  lastCaretPos_ = 0;               // previous caret pos → gives normalizeCaret()
                                          // the direction of travel across a boundary gap
};
