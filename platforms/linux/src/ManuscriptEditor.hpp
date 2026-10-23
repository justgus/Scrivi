#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QPlainTextEdit>

#include <functional>
#include <optional>
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
    // Edit ▸ Copy and Ctrl+C. ✅ [I-0307]: the ONLY copy that builds the structured (cross-scene) Range — see
    // `clipboardCopy_`.
    void copyToClipboard();
    // For smokes ([I-0307]): what Qt fills the X11 / Wayland PRIMARY selection with on a mouse selection. The offscreen
    // platform has no primary selection for Qt to fill, so the smoke asks for it directly.
    QMimeData* primarySelectionMime() const { return createMimeDataFromSelection(); }
    // Edit ▸ Paste and Ctrl+V (EP-026 S4, SP-180). A Range this editor copied, while the clipboard still holds THAT
    // copy ([SP-180] Q1), is handed to the shell to rebuild (`structuredPasteRequested`); anything else is the flat paste,
    // through `insertFromMimeData`. ⚠️ A drop is always flat, as on the Mac (its `paste:` override is the only way in).
    void pasteClipboard();

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
    // What a slot gets for a selection inside ONE scene: the BALANCED stored source Ctrl+C keeps for Scrivi's own paste
    // ([I-0289], Q2). Across scenes the shell stores `crossSceneCopy()` instead (EP-026 S4). Empty when nothing is selected.
    [[nodiscard]] QString selectionSource() const;
    // Cut a selection inside ONE scene into a slot: `store(source, range)` FIRST (a cut must never lose its text), then the
    // balanced removal as ONE `cut` step tagged with `bufferID`. Changes nothing when the selection is not inside one scene
    // (✅ [SP-188]: across scenes the shell's structural cut runs instead) or `store` fails.
    bool cutSelectionIntoBuffer(const QString& bufferID,
                                const std::function<bool(const QString& source, const QJsonObject& range)>& store);
    // Paste a slot's STORED text verbatim (never re-escaped, EP-045 AC4) over the selection: ONE `paste` step. A slot is
    // always Scrivi's own copy, so it keeps ITS formatting where it lands ([I-0289], [SP-163]). False when refused.
    bool pasteStored(const QString& source);
    // SP-181 (EP-048 S4b): Replace — `typed` over the selection, exactly as typing it would go in (escaped, never half an
    // escape pair, balanced; the first character's style, [SP-164] Q3). Apple: `replaceFound` → `insertText`. False when
    // the selection touches a boundary.
    bool typeOver(const QString& typed);
    // SP-181: a document range snapped as the editor snaps a selection (L5) and widened over escape pairs — the range a
    // Replace All replaces for one match (Apple: `snapSelection` in `applyReplacements`).
    void snapForTyping(int& start, int& end) const;
    // The digit 1–9 the key event's PHYSICAL key carries, or 0. ⚠️ Ctrl+Shift+1 arrives as `!` (the shifted key), so the
    // scan code decides; the US-layout shifted marks are the fallback when there is none (offscreen tests).
    [[nodiscard]] static int bufferDigit(const QKeyEvent* event);

    // ── EP-026 S4 (SP-180 / T-0607): structured cut / copy / paste across scenes — Apple's `structuredCopyIfCrossBoundary`,
    // `copiedRange` and `pasteStructuredRange` (`ManuscriptTextView.swift`). ──
    // A selection across scene breaks: its `scrivi.range.v1`, each scene's piece BALANCED ([T-0591]); `source`, the stored
    // pieces joined by a blank line (a buffer slot's flat text); and what other apps get — the presented pieces joined the
    // same way, no headings ([SP-180] Q4). nullopt for a selection inside one scene. ✅ [SP-191] (§4.2b): built from the
    // scene list — ⛔ it asked the core to extract the fragment, which read every selected scene from disk.
    struct CrossSceneCopy { QJsonObject range; QString source; QString presented; };
    [[nodiscard]] std::optional<CrossSceneCopy> crossSceneCopy() const;
    // A Range's offsets made consistent with its entries' texts, as `scrivi.range.v1` requires (only the first entry may
    // start mid-scene, only the last may end mid-scene) — for a Range whose texts were rebuilt (balanced), which only a
    // PASTE reads: a paste takes the texts, never the offsets. Apple: `ScriviRange.normalized()`.
    [[nodiscard]] static QJsonObject normalizedRange(QJsonObject range);
    // Before a structured paste ([SP-180] Q2, Apple [I-0295]): a selection inside one scene is REMOVED, as its own step.
    // False (nothing changed) when the caret or selection is only in a heading or separator, or crosses scenes
    // (✅ [SP-188] Q6: the shell's structural cut removes that one first).
    bool removeSelectionForPaste();

    // ── [SP-188] (design R4, §4.3): a cut across scenes removes the structure — Apple's `selectionRun` / `cutAcrossScenes`.
    // The scenes a selection [start, end) takes, in reading order, each with the part of it selected (document positions):
    // a scene whose body it overlaps, and
    //  - Q5 / Q2: across a SCENE separator it covers, the scenes on both sides — the one before at its end, the one after
    //    at its start (empty parts) — so whole scenes go whole, and a selection of exactly one separator is the merge;
    //  - Q1: a chapter heading is skipped (presentation, in no scene's text) unless the selection covers the whole chapter,
    //    which then takes the break after it (or, at the manuscript's end, the one before), so the cut removes the chapter.
    struct RunPart { int index; int start; int end; };
    [[nodiscard]] QList<RunPart> selectionRun(int start, int end) const;
    // True when the selection takes more than one scene (`selectionRun`).
    [[nodiscard]] bool selectionCrossesScenes() const;
    // The selection's Range with each part's STORED text (the core checks it against the disk — ⛔ never the balanced
    // copy), or nullopt when it takes one scene.
    [[nodiscard]] std::optional<QJsonObject> crossSceneCutRange() const;
    // A cut's clipboard (Q4): other apps get `copy`'s text NOW, with a fresh token; `holdCutRange` gives Scrivi's own paste
    // the cut's Range once it lands — only while the clipboard still carries that token.
    QString putCutOnClipboard(const CrossSceneCopy& copy);
    void holdCutRange(const QString& token, const QJsonObject& range);

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
    // ✅ [SP-188] Backspace / Delete (keeping 0) or Ctrl+X (keeping 1: the clipboard) over a selection that takes more than
    // one scene — the shell's structural cut. `thenPaste`: a flat paste over it, done once the cut lands (Q6).
    void crossSceneCutRequested(int keeping, bool thenPaste);

    // ── EP-026 S5 (SP-179): the buffer chords (✅ EP-026 Q3: Ctrl+N copy · Ctrl+Shift+N paste · Alt+N cut). Caught HERE,
    // each claimed in ShortcutOverride first; EditorShell acts on the slot. `bufferID` is "1"–"9". ──
    void bufferCopyRequested(const QString& bufferID);
    void bufferPasteRequested(const QString& bufferID);
    void bufferCutRequested(const QString& bufferID);
    // EP-026 S4: paste `range` at the caret, rebuilding its scenes and chapters — the shell's. Emitted by `pasteClipboard`.
    void structuredPasteRequested(const QJsonObject& range);

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

    // [SP-180] Q1: the last cross-scene copy's Range and the token its clipboard data carries — valid only while the
    // clipboard still holds that token (Apple: tied to the pasteboard's change count, [I-0279]).
    mutable QJsonObject heldRange_;
    mutable QString heldToken_;
    mutable quint64 copySerial_ = 0;
    // ✅ [I-0307] (SP-185): true only while copying to the CLIPBOARD. Qt also calls `createMimeDataFromSelection` to fill
    // the X11 / Wayland PRIMARY selection on every mouse selection (and for a drag, whose drop is flat anyway); building
    // the structured copy there asked ScriviCore for an extract on the GUI thread — the rig's freeze "after select across
    // scene" — and replaced the held copy and token, so the clipboard's copy no longer matched them. (⚠️ [SP-191]: the copy
    // no longer calls the core for its structure, but it still balances each piece, and would still replace the token.)
    bool clipboardCopy_ = false;

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
