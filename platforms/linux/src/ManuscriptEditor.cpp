#include "ManuscriptEditor.hpp"
#include "DiagnosticLog.hpp"

#include <QScopedValueRollback>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>

#include "ThemeColours.hpp"
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>
#include <cmath>

#include "ManuscriptEscapes.hpp"
#include "ManuscriptPresenter.hpp"
#include "SceneDocument.hpp"
#include "ScriviBridge.hpp"

#include <QApplication>
#include <QClipboard>

#include <QInputMethodEvent>
#include <QMimeData>

namespace {
// Horizontal inset (px) at each end of the rule, matching Apple's DividerAttachmentCell
// (which insets 20pt). Keeps the line from touching the text margins.
constexpr int kRuleInset = 20;
// EP-049: Scrivi's own clipboard data carries the STORED source here, so an in-app paste restores it
// exactly (Apple remembers its own pasteboard write — `ownCopy`; same result, Qt means).
const QString kSourceMime = QStringLiteral("application/x-scrivi-manuscript-source");
// EP-026 S4 ([SP-180] Q1): a cross-scene copy's clipboard data carries this editor's token for the Range it holds —
// ✅ never the Range itself (T2=A: the system clipboard carries flat text only).
const QString kFragmentTokenMime = QStringLiteral("application/x-scrivi-fragment-token");
}   // namespace

ManuscriptEditor::ManuscriptEditor(QWidget* parent) : QPlainTextEdit(parent)
{
    document()->setUndoRedoEnabled(false);   // Ctrl+Z is the custom history's (EP-026 S2) — see keyPressEvent

    // Keep the caret out of protected boundary text (T-0246).
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &ManuscriptEditor::normalizeCaret);
}

void ManuscriptEditor::normalizeCaret()
{
    if (sceneDoc_ == nullptr || normalizingCaret_ || mouseSelecting_) {
        return;
    }
    leavePendingPair();
    QTextCursor cursor = textCursor();
    // Don't fight an active selection across the scene boundaries (the edit guard already prevents
    // boundary-touching edits); only its ends are kept out of hidden runs (EP-048 L5).
    if (cursor.hasSelection()) {
        int s = cursor.selectionStart();
        int e = cursor.selectionEnd();
        snapSelection(s, e, lastSelEnd_);
        if (s != cursor.selectionStart() || e != cursor.selectionEnd()) {
            const bool forward = cursor.anchor() <= cursor.position();
            normalizingCaret_ = true;
            cursor.setPosition(forward ? s : e);
            cursor.setPosition(forward ? e : s, QTextCursor::KeepAnchor);
            setTextCursor(cursor);
            normalizingCaret_ = false;
        }
        lastCaretPos_ = cursor.position();
        lastSelEnd_ = e;
        if (presenter_ != nullptr) {
            presenter_->reveal(s, e);
        }
        return;
    }
    const int pos = cursor.position();
    // Snap in the DIRECTION the caret was travelling, so arrow keys cross a boundary into
    // the next/previous scene instead of getting stuck at it. Forward (Down/Right/typing)
    // when the caret advanced; backward (Up/Left) when it retreated. A same-position event
    // (no movement) keeps the previous direction bias as "forward" by default.
    const bool movingForward = (pos >= lastCaretPos_);
    int snapped = sceneDoc_->editablePositionInDirection(pos, movingForward);
    // EP-048 L5: then out of any hidden run, to its home (Apple: `snapCaret`).
    if (snapped == pos) {
        snapped = snapCaret(pos, lastCaretPos_);
    }
    if (snapped != pos) {
        normalizingCaret_ = true;
        cursor.setPosition(snapped);
        setTextCursor(cursor);
        normalizingCaret_ = false;
    }
    lastCaretPos_ = snapped;
    lastSelEnd_ = snapped;
    if (presenter_ != nullptr) {
        presenter_->reveal(snapped, snapped);
    }
}

void ManuscriptEditor::mousePressEvent(QMouseEvent* event)
{
    mouseSelecting_ = true;
    QPlainTextEdit::mousePressEvent(event);
}

void ManuscriptEditor::mouseReleaseEvent(QMouseEvent* event)
{
    QPlainTextEdit::mouseReleaseEvent(event);
    mouseSelecting_ = false;
    normalizeCaret();
}

// ── EP-048 L5 (SP-166): stop runs — a port of Apple's `MarkdownEscapes.snapCaret` / `snapSelection` ──

int ManuscriptEditor::snapCaret(int loc, int previous) const
{
    if (presenter_ == nullptr) {
        return loc;
    }
    int cur = loc;
    // Landing past one run can land beside the next (`**a** **b**`): settle, a few steps at most.
    for (int k = 0; k < 4; ++k) {
        const int next = snapOnce(cur, previous);
        if (next == cur) {
            break;
        }
        cur = next;
    }
    return cur;
}

int ManuscriptEditor::snapOnce(int loc, int previous) const
{
    const int length = document()->characterCount() - 1;
    std::optional<ManuscriptPresenter::Stop> run;
    if (loc > 0) {
        run = presenter_->stopAt(loc - 1);                                   // inside, or just past it
    }
    if (!run && loc < length) {
        if (auto r = presenter_->stopAt(loc); r && r->homeAfter()) {         // just before an opener
            run = r;
        }
    }
    if (!run) {
        return loc;
    }
    const int s = run->start, e = run->end;
    if (run->homeAfter()) {
        if (loc == e) {
            return loc;
        }
        // ← from home: out to the left, past the character before the run.
        if (previous == e && loc == e - 1) {
            return s > 0 ? s - 1 : e;
        }
        return e;
    }
    if (loc == s) {
        return loc;
    }
    // → from home: out to the right, past the character after the run.
    if (previous == s && loc == s + 1) {
        return std::min(e + 1, length);
    }
    return s;
}

bool ManuscriptEditor::isUnreachable(int pos) const
{
    const int length = document()->characterCount() - 1;
    return presenter_ != nullptr && pos > 0 && pos < length && presenter_->stopAt(pos - 1).has_value();
}

void ManuscriptEditor::snapSelection(int& start, int& end, int previousEnd) const
{
    if (presenter_ == nullptr) {
        return;
    }
    const int length = document()->characterCount() - 1;
    if (isUnreachable(start)) {
        while (start > 0 && presenter_->stopAt(start - 1)) {
            --start;
        }
    }
    if (isUnreachable(end)) {
        if (end < previousEnd) {
            while (end > 0 && presenter_->stopAt(end - 1)) {
                --end;
            }
        } else {
            while (end < length && presenter_->stopAt(end)) {
                ++end;
            }
            if (end < length && document()->characterAt(end - 1) == QLatin1Char('\\')) {
                ++end;   // an escape backslash keeps its mark with it
            }
        }
    }
}

bool ManuscriptEditor::handleAtomicDeletion(bool back, int loc)
{
    if (presenter_ == nullptr) {
        return false;
    }
    using Kind = ManuscriptPresenter::StopKind;
    if (back) {
        const auto stop = loc > 0 ? presenter_->stopAt(loc - 1) : std::nullopt;
        if (!stop || stop->end != loc) {
            return false;
        }
        switch (stop->kind) {
        case Kind::Prefix:
        case Kind::ListPrefix:
            // ⌫ at a heading's (or list item's) visible start removes the WHOLE prefix — the line becomes body text.
            if (sceneDoc_->isEditableRange(stop->start, stop->end)) {
                replaceRange(stop->start, stop->end, QString());
            }
            return true;
        case Kind::Opener: {
            // ⌫ after an opening marker deletes what the writer SEES before the caret: the character before
            // the marker, or the paragraph join — never part of the marker.
            const int s = stop->start;
            int joinStart = 0;
            if (paragraphJoinRange(s, joinStart)) {
                if (sceneDoc_->isEditableRange(joinStart, s)) {
                    replaceRange(joinStart, s, QStringLiteral(" "));
                }
                return true;
            }
            int a = s - 1, b = s;
            if (a < 0) {
                return true;
            }
            widenOverPairs(a, b);
            if (sceneDoc_->isEditableRange(a, b)) {
                replaceRange(a, b, QString());
                QTextCursor c = textCursor();   // the caret stays at the marker's home
                c.setPosition(stop->end - (b - a));
                setTextCursor(c);
            }
            return true;
        }
        case Kind::Escape:
        case Kind::Closer:
            return false;
        }
        return false;
    }
    // ⌦ before a CLOSING marker deletes the character after it — never the marker.
    const auto stop = presenter_->stopAt(loc);
    if (!stop || stop->kind != Kind::Closer || stop->start != loc) {
        return false;
    }
    int a = stop->end, b = stop->end + 1;
    if (b > document()->characterCount() - 1) {
        return true;
    }
    widenOverPairs(a, b);
    if (sceneDoc_->isEditableRange(a, b)) {
        replaceRange(a, b, QString());
        QTextCursor c = textCursor();
        c.setPosition(loc);
        setTextCursor(c);
    }
    return true;
}

bool ManuscriptEditor::isModifyingKey(const QKeyEvent* event)
{
    // Deletion keys.
    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        return true;
    }
    // Paste / cut shortcuts modify text (copy does not).
    if (event->matches(QKeySequence::Paste) || event->matches(QKeySequence::Cut)) {
        return true;
    }
    // Any keystroke that carries insertable text (printable chars, Enter/Return
    // which insert a newline, Tab). Modifier-only or navigation keys produce empty
    // text and fall through as non-modifying.
    if (!event->text().isEmpty()) {
        // Control chars other than the ones we treat as text: Return/Enter/Tab do
        // insert; bare control combos (e.g. Ctrl+A select-all) carry non-printable
        // text but must NOT be treated as inserts. Gate on printability + the known
        // inserting keys.
        const QChar c = event->text().at(0);
        const bool inserts = c.isPrint()
                             || event->key() == Qt::Key_Return
                             || event->key() == Qt::Key_Enter
                             || event->key() == Qt::Key_Tab;
        return inserts;
    }
    return false;
}

bool ManuscriptEditor::modifiedRangeFor(const QKeyEvent* event,
                                        int& start, int& end) const
{
    const QTextCursor cursor = textCursor();

    if (cursor.hasSelection()) {
        // Every modifying key replaces the current selection.
        start = cursor.selectionStart();
        end   = cursor.selectionEnd();
        return true;
    }

    const int pos = cursor.position();
    if (event->key() == Qt::Key_Backspace) {
        start = pos - 1;
        end   = pos;
        return start >= 0;
    }
    if (event->key() == Qt::Key_Delete) {
        start = pos;
        end   = pos + 1;
        return true;
    }
    // Plain insertion at the caret.
    start = pos;
    end   = pos;
    return true;
}

int ManuscriptEditor::bufferDigit(const QKeyEvent* event)
{
    // ✅ The physical key first: xkb keycodes (X11 and Wayland alike) are evdev + 8, so the digit row 1–9 is 10–18 —
    // whatever the layout and Shift made of it (Ctrl+Shift+1 arrives as `!` on a US layout, EP-026 Q3's trap).
    const quint32 scan = event->nativeScanCode();
    if (scan >= 10 && scan <= 18) {
        return static_cast<int>(scan) - 9;
    }
    const int key = event->key();
    if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        return key - Qt::Key_0;
    }
    // No scan code (offscreen, synthesized input): the US-layout shifted digits.
    static const int shifted[] = {Qt::Key_Exclam, Qt::Key_At, Qt::Key_NumberSign, Qt::Key_Dollar, Qt::Key_Percent,
                                  Qt::Key_AsciiCircum, Qt::Key_Ampersand, Qt::Key_Asterisk, Qt::Key_ParenLeft};
    for (int i = 0; i < 9; ++i) {
        if (key == shifted[i]) {
            return i + 1;
        }
    }
    return 0;
}

std::pair<int, int> ManuscriptEditor::bufferChord(const QKeyEvent* event)
{
    const int digit = bufferDigit(event);
    if (digit == 0) {
        return {0, 0};
    }
    // Each chord names its modifiers EXACTLY, so they never overlap (Apple's rule; [I-0291]'s lesson).
    const Qt::KeyboardModifiers mods = event->modifiers() & ~Qt::KeypadModifier;
    if (mods == Qt::ControlModifier) {
        return {1, digit};   // copy
    }
    if (mods == (Qt::ControlModifier | Qt::ShiftModifier)) {
        return {2, digit};   // paste
    }
    if (mods == Qt::AltModifier) {
        return {3, digit};   // cut
    }
    return {0, 0};
}

bool ManuscriptEditor::event(QEvent* event)
{
    // EP-026 S5: the buffer chords are the editor's while it has focus — claimed before any window shortcut sees them.
    if (event->type() == QEvent::ShortcutOverride && bufferChord(static_cast<QKeyEvent*>(event)).first != 0) {
        event->accept();
        return true;
    }
    return QPlainTextEdit::event(event);
}

void ManuscriptEditor::keyPressEvent(QKeyEvent* event)
{
    const DiagnosticLog::Span span("key press in the manuscript");
    if (event->matches(QKeySequence::Copy)) {   // ✅ [I-0307]: the clipboard copy, marked as one
        copyToClipboard();
        event->accept();
        return;
    }
    // EP-026 S5 (SP-179 / T-0606): the copy-buffer chords (Apple: `keyDown`'s ⌘ / ⌃⌘ / ⌥ + digit).
    if (const auto [action, digit] = bufferChord(event); action != 0) {
        const QString bufferID = QString::number(digit);
        if (action == 1) {
            emit bufferCopyRequested(bufferID);
        } else if (action == 2) {
            emit bufferPasteRequested(bufferID);
        } else {
            emit bufferCutRequested(bufferID);
        }
        event->accept();
        return;
    }

    // EP-026 S2 (SP-178): Ctrl+Z undo, Ctrl+Shift+Z redo — matched EXPLICITLY. ⛔ Not `QKeySequence::Undo/Redo`: on X11
    // `Redo` also matches Ctrl+Y, which EP-026 Q3 ruled out (Ctrl+Y falls through to the disabled document undo — nothing).
    // The editor accepts these in ShortcutOverride, so the Edit-menu shortcut only fires when the editor lacks focus.
    if (event->key() == Qt::Key_Z) {
        const Qt::KeyboardModifiers mods = event->modifiers() & ~Qt::KeypadModifier;
        if (mods == Qt::ControlModifier) {
            emit undoRequested();
            event->accept();
            return;
        }
        if (mods == (Qt::ControlModifier | Qt::ShiftModifier)) {
            emit redoRequested();
            event->accept();
            return;
        }
    }

    // In-editor structure creation (T-0240 / T-0241): Ctrl+Return = new scene,
    // Ctrl+Shift+Return = new chapter (the Linux analogues of ⌘↩ / ⌘⇧↩). Catch
    // these before the modifying-key path so they never insert a newline; the
    // create itself is EditorShell's job.
    if (sceneDoc_ != nullptr
        && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        && (event->modifiers() & Qt::ControlModifier)) {
        if (event->modifiers() & Qt::ShiftModifier) {
            emit createChapterRequested();
        } else {
            emit createSceneRequested();
        }
        event->accept();
        return;
    }

    // In-editor structure merging (EP-028 / T-0304): Ctrl+Backspace = merge this
    // scene into the previous one, Ctrl+Shift+Backspace = merge this chapter into
    // the previous one (the Linux analogues of ⌘⌫ / ⇧⌘⌫). Catch these before the
    // modifying-key path so a bare Backspace never runs the edit guard and deletes a
    // character. The editor only signals intent; EditorShell resolves the caret,
    // enforces the start-of-scene / manuscript-start no-op, and calls the endpoint.
    // NOTE: accept BOTH Backspace and Delete for the Shift variant — over VNC/X11,
    // Shift+Backspace is frequently delivered as Qt::Key_Delete, which would otherwise
    // make chapter-merge silently miss everywhere.
    if (sceneDoc_ != nullptr
        && (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
        && (event->modifiers() & Qt::ControlModifier)) {
        if (event->modifiers() & Qt::ShiftModifier) {
            emit mergeChapterRequested();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Backspace) {
            // Plain Ctrl+Backspace = scene merge.
            emit mergeSceneRequested();
            event->accept();
            return;
        }
        // Ctrl+Delete without Shift is not a merge: it is Qt's delete-to-next-word, and it takes
        // the EDIT path below. ⛔ EP-049 (SP-160, M3): it used to go straight to QPlainTextEdit,
        // skipping the boundary guard AND the escape-pair rule (it orphaned a `\`).
    }

    // No document loaded, or a non-modifying key (navigation, copy, modifiers):
    // let the base class handle it.
    if (sceneDoc_ == nullptr || !isModifyingKey(event)) {
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    // SP-175: a cut (also Shift+Del) is the BALANCED cut, inside one scene or across scenes.
    if (event->matches(QKeySequence::Cut)) {
        cutSelection();
        return;
    }
    // EP-026 S4: a paste may be structured — and may replace a selection across scenes ([SP-180] Q2), which the guard
    // below would swallow.
    if (event->matches(QKeySequence::Paste)) {
        pasteClipboard();
        return;
    }

    int start = 0;
    int end = 0;
    if (!modifiedRangeFor(event, start, end)) {
        return;   // e.g. Backspace at document start — nothing to do; swallow.
    }

    // [I-0270] (user ruling 2026-10-02, Apple shape): Backspace / Delete / Cut on a
    // selection that spans scene breaks removes the selected text from EACH scene and
    // keeps every scene. Anything else that crosses a boundary is still refused below.
    const bool deletes = event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete
                         || event->matches(QKeySequence::Cut);
    if (deletes && textCursor().hasSelection() && !sceneDoc_->isEditableRange(start, end)) {
        widenOverPairs(start, end);   // EP-049: never leave half an escape pair at either end
        deleteAcrossScenes(start, end, event->matches(QKeySequence::Cut));
        return;
    }

    // Allow the edit only if the touched range stays entirely inside one scene's
    // editable body. Otherwise swallow the keystroke (boundary stays intact).
    if (!sceneDoc_->isEditableRange(start, end)) {
        return;
    }

    // ✅ EP-049 (SP-160, T-0586): Apple's escape layer, behind the same guard Apple's overrides sit
    // behind (`shouldChangeText` / `isSeparatorPosition`). Each handled gesture is ONE document edit.
    if (handleReturn(event) || handleDeletion(event) || handleTyping(event)) {
        return;
    }
    // Paste / cut keys: widen a selection that would split an escape pair, then let Qt run them —
    // paste lands in insertFromMimeData, cut builds its data in createMimeDataFromSelection.
    QTextCursor c = textCursor();
    if (c.hasSelection()) {
        int s = c.selectionStart();
        int e = c.selectionEnd();
        widenOverPairs(s, e);
        c.setPosition(s);
        c.setPosition(e, QTextCursor::KeepAnchor);
        setTextCursor(c);
    }
    QPlainTextEdit::keyPressEvent(event);
}

// ── EP-049 (SP-160, T-0586): the escape layer's WRITE half — a port of Apple's ──────────────────
// Apple: `ManuscriptNSTextView` in `Scrivi/Views/ManuscriptTextView.swift` + `ManuscriptEscapes.swift`.

bool ManuscriptEditor::isInsideEscapePair(int pos) const
{
    if (pos <= 0) {
        return false;
    }
    const QTextBlock block = document()->findBlock(pos - 1);
    const QString text = block.text();
    const int off = pos - 1 - block.position();
    if (off < 0 || off >= text.size() || text.at(off) != QLatin1Char('\\')) {
        return false;
    }
    // Escapes are decided left to right within the line (`\\*` is an escaped backslash, then a
    // bare mark), so ask the rules about the whole line — as Apple's styler maps one paragraph.
    QString source = text;
    bool continues = false;
    const QTextBlock next = block.next();
    if (next.isValid()) {
        source += QLatin1Char('\n');
        // Q1 = (a): a hard break needs a non-blank line of the SAME scene after it.
        const bool sameScene = sceneDoc_ == nullptr
            || (sceneDoc_->sceneIndexForEditablePosition(next.position()) >= 0
                && sceneDoc_->sceneIndexForEditablePosition(next.position())
                       == sceneDoc_->sceneIndexForEditablePosition(block.position()));
        continues = sameScene && !ManuscriptEscapes::isBlankLine(next.text());
    }
    return ManuscriptEscapes::hiddenBackslashes(source, continues).contains(off);
}

void ManuscriptEditor::widenOverPairs(int& start, int& end) const
{
    if (start == end) {
        // AC4b — an INSERTION point inside a pair moves before the backslash (Apple's caret never
        // rests there: `presentedToSource`). ⛔ Never widened, which would delete the pair.
        if (isInsideEscapePair(start)) {
            start = end = start - 1;
        }
        return;
    }
    if (isInsideEscapePair(start)) {
        --start;
    }
    if (isInsideEscapePair(end)) {
        ++end;
    }
}

void ManuscriptEditor::replaceRange(int start, int end, const QString& text)
{
    if (!computed_ && sceneDoc_ != nullptr) {
        if (const auto scene = sceneDoc_->sceneHolding(start, end)) {
            if (const auto e = ScriviBridge::markdownBalancedEdit(scene->text, start - scene->start, end - scene->start,
                                                                  text, pastingOwnCopy_)) {
                applyComputed(scene->start, *e);
                return;
            }
        }
    }
    QTextCursor c(document());
    c.beginEditBlock();
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    c.insertText(text);   // a '\n' becomes a block break, as Return's does
    c.endEditBlock();
    setTextCursor(c);
    ensureCursorVisible();
}

bool ManuscriptEditor::handleReturn(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Return && event->key() != Qt::Key_Enter) {
        return false;
    }
    const QTextCursor cursor = textCursor();
    int selStart = cursor.selectionStart();
    int selEnd = cursor.selectionEnd();
    widenOverPairs(selStart, selEnd);
    // ✅ [T-0584] (Q-E2-4 = (b), ruled 2026-10-05): Apple's Option-Return (`insertNewlineIgnoringFieldEditor:`)
    // stores a deliberate HARD LINE BREAK — `\` + `\n`, the form EP-045 AC6 writes. Changed on both platforms in
    // the same work ([SP-163]); the shared corpus pins it. (M1: Qt's own Alt-Return inserted nothing.)
    if (event->modifiers() & Qt::AltModifier) {
        if (sceneDoc_->isEditableRange(selStart, selEnd)) {
            replaceRange(selStart, selEnd, QStringLiteral("\\\n"));
        }
        return true;
    }
    // ✅ SP-175 ([SP-163] Q4): Return in a LIST ITEM starts the next item (an empty item ends the list), renumbering
    // what follows — one edit (Apple: `listReturnEdit` first in `insertNewline`).
    if (!cursor.hasSelection()) {
        if (const auto scene = sceneDoc_->sceneHolding(selStart, selStart)) {
            if (const auto e = ScriviBridge::markdownListReturnEdit(scene->text, selStart - scene->start)) {
                applyComputed(scene->start, *e);
                return true;
            }
        }
    }
    // ✅ Apple's `insertNewline` (EP-045 AC5/AC6): Return, keypad Enter AND Shift-Return store `\n\n`.
    // ⛔ M1: Qt's Shift-Return inserted U+2028, which `bodyText` wrote into the scene file.
    const QTextDocument* d = document();
    int start = selStart;
    while (start > 0 && d->characterAt(start - 1) == QLatin1Char(' ')) {
        --start;
    }
    QString tail = start < selStart ? QStringLiteral(" ") : QString();
    if (tail.isEmpty()) {
        // Only an escape PAIR collapses: an even run of backslashes is all `\\` pairs.
        int run = 0;
        while (run < start && d->characterAt(start - 1 - run) == QLatin1Char('\\')) {
            ++run;
        }
        if (run >= 2 && run % 2 == 0) {
            start -= 2;
            tail = QStringLiteral("\\");
        }
    }
    if (sceneDoc_->isEditableRange(start, selEnd)) {
        replaceRange(start, selEnd, tail + QStringLiteral("\n\n"));
    }
    return true;
}

bool ManuscriptEditor::paragraphJoinRange(int loc, int& start) const
{
    // Apple's `paragraphJoin` (Q3): ⌫ at a paragraph start JOINS it to the one above with ONE space.
    // ⚠️ Qt stores each paragraph break as U+2029.
    const QTextDocument* d = document();
    const QChar ps(QChar::ParagraphSeparator);
    const int nl = loc - 2;
    if (nl <= 0 || d->characterAt(loc - 1) != ps || d->characterAt(nl) != ps) {
        return false;
    }
    if (loc >= d->characterCount() || d->characterAt(loc) == ps) {
        return false;   // the paragraph being joined is empty — remove one break instead
    }
    int s = nl;
    while (s > 0 && d->characterAt(s - 1) == QLatin1Char(' ')) {
        --s;
    }
    if (s <= 0 || d->characterAt(s - 1) == ps) {
        return false;   // the paragraph above is empty
    }
    if (s == nl) {
        int run = 0;
        while (run < s && d->characterAt(s - 1 - run) == QLatin1Char('\\')) {
            ++run;
        }
        if (run % 2 == 1) {
            return false;   // a BARE trailing `\` — the writer's deliberate hard break survives
        }
    }
    start = s;
    return true;
}

bool ManuscriptEditor::handleDeletion(QKeyEvent* event)
{
    const bool back = event->key() == Qt::Key_Backspace;
    const bool forward = event->key() == Qt::Key_Delete;
    if ((!back && !forward) || event->matches(QKeySequence::Cut)) {
        return false;   // Shift+Del is Cut (M3) — the cut path
    }
    const QTextCursor cursor = textCursor();
    // ✅ [SP-163] Q1: ⌫ or ⌦ inside an EMPTY pending pair takes the pair away.
    if (!cursor.hasSelection() && sceneDoc_->pending() && cursor.position() == sceneDoc_->pending()->caret()) {
        const int at = sceneDoc_->pending()->start;
        removePendingPair();
        QTextCursor c = textCursor();
        c.setPosition(at);
        setTextCursor(c);
        return true;
    }
    int start = 0;
    int end = 0;
    if (cursor.hasSelection()) {
        start = cursor.selectionStart();
        end = cursor.selectionEnd();
    } else if (handleAtomicDeletion(back, cursor.position())) {
        return true;   // EP-048 L9: a marker is never deleted alone
    } else if (back) {
        const int loc = cursor.position();
        int joinStart = 0;
        if (paragraphJoinRange(loc, joinStart)) {
            if (sceneDoc_->isEditableRange(joinStart, loc)) {
                replaceRange(joinStart, loc, QStringLiteral(" "));
            }
            return true;
        }
        start = loc - 1;
        end = loc;
    } else if (event->modifiers() & Qt::ControlModifier) {
        // M3: Ctrl+Delete deletes to the next word INSIDE Qt — compute its real range here so the
        // pair rule (and the boundary guard) see it, not the one character `modifiedRangeFor` assumed.
        QTextCursor word = cursor;
        word.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        start = word.selectionStart();
        end = word.selectionEnd();
    } else {
        start = cursor.position();
        end = start + 1;
    }
    if (start == end) {
        return true;
    }
    widenOverPairs(start, end);   // Apple: `snapSelection` in `shouldChangeText`
    if (sceneDoc_->isEditableRange(start, end)) {
        replaceRange(start, end, QString());
    }
    return true;
}

bool ManuscriptEditor::handleTyping(QKeyEvent* event)
{
    const QString text = event->text();
    if (text.isEmpty() || event->matches(QKeySequence::Paste) || event->matches(QKeySequence::Cut)) {
        return false;
    }
    const QTextCursor cursor = textCursor();
    // ✅ [SP-163] Q1: whitespace typed into an EMPTY pending pair cannot start a span (`** x**` is not emphasis): it
    // goes BEFORE the pair, which stays pending with the caret inside it.
    if (const auto& p = sceneDoc_->pending(); p && !cursor.hasSelection() && cursor.position() == p->caret()
        && std::all_of(text.begin(), text.end(), [](QChar c) { return c == QLatin1Char(' ') || c == QLatin1Char('\t'); })) {
        const QString markers(p->markerLength, QLatin1Char('*'));
        const int at = p->start;
        removePendingPair();
        replaceRange(at, at, ManuscriptEscapes::escape(text));
        insertPendingPair(textCursor().position(), markers);
        return true;
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    if (sceneDoc_->isEditableRange(start, end)) {
        replaceRange(start, end, ManuscriptEscapes::escape(text));   // Apple: `insertText`
    }
    return true;
}

void ManuscriptEditor::inputMethodEvent(QInputMethodEvent* event)
{
    if (sceneDoc_ == nullptr || event->commitString().isEmpty()) {
        QPlainTextEdit::inputMethodEvent(event);
        return;
    }
    // M2: a composed character commits here only. Escape it exactly as typing does.
    QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    if (!sceneDoc_->isEditableRange(start, end)) {
        event->accept();
        return;
    }
    // SP-175: a commit that REPLACES a selection is one edit through `replaceRange`, so it is kept balanced.
    if (start != end && event->preeditString().isEmpty()) {
        replaceRange(start, end, ManuscriptEscapes::escape(event->commitString()));
        event->accept();
        return;
    }
    if (start != cursor.selectionStart() || end != cursor.selectionEnd()) {
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
    QInputMethodEvent escaped(event->preeditString(), event->attributes());
    escaped.setCommitString(ManuscriptEscapes::escape(event->commitString()),
                            event->replacementStart(), event->replacementLength());
    QPlainTextEdit::inputMethodEvent(&escaped);
}

std::pair<QString, QString> ManuscriptEditor::balancedCopyOf(int start, int end) const
{
    // ✅ SP-175 (EP-046 AC12, Apple's `balancedCopy`): Scrivi's own clipboard gets the BALANCED source — a selection
    // that starts or ends inside a span carries its own markers; other apps get the presented text. Scene by scene;
    // the headings and separators between scenes are copied as they are.
    QString source;
    QString presented;
    int p = start;
    if (sceneDoc_ != nullptr) {
        for (const SceneSegment& seg : sceneDoc_->segments()) {
            const int a = std::max(start, seg.bodyStart);
            const int b = std::min(end, seg.bodyStart + seg.bodyLength);
            if (b <= a) {
                continue;
            }
            if (a > p) {
                const QString gap = textOf(p, a);
                source += gap;
                presented += gap;
            }
            const auto scene = sceneDoc_->sceneHolding(a, b);
            const auto copy = scene ? ScriviBridge::markdownBalancedCopy(scene->text, a - scene->start, b - scene->start)
                                    : std::pair<QString, QString>{textOf(a, b), ManuscriptEscapes::map(textOf(a, b)).presented};
            source += copy.first;
            presented += copy.second;
            p = b;
        }
    }
    if (p < end) {
        const QString rest = textOf(p, end);
        source += rest;
        presented += ManuscriptEscapes::map(rest).presented;
    }
    return {source, presented};
}

QMimeData* ManuscriptEditor::createMimeDataFromSelection() const
{
    const DiagnosticLog::Span span("copy: clipboard data from the selection");
    const QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        return QPlainTextEdit::createMimeDataFromSelection();
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    auto* mime = new QMimeData;
    // ✅ EP-026 S4: across scenes, the Range is held here for Scrivi's own paste and its token rides on the clipboard;
    // the clipboard's text is the pieces joined by a blank line, no headings ([SP-180] Q4).
    if (const auto cross = clipboardCopy_ ? crossSceneCopy() : std::nullopt) {   // [I-0307]: the clipboard's only
        heldRange_ = cross->range;
        heldToken_ = QStringLiteral("%1:%2").arg(quintptr(this)).arg(++copySerial_);
        mime->setText(cross->presented);
        mime->setData(kSourceMime, cross->source.toUtf8());
        mime->setData(kFragmentTokenMime, heldToken_.toUtf8());
        return mime;
    }
    const auto [source, presented] = balancedCopyOf(start, end);
    // Apple: `writeSelection` — other apps get the writer's text, not backslashes.
    mime->setText(presented);
    mime->setData(kSourceMime, source.toUtf8());
    return mime;
}

void ManuscriptEditor::pasteClipboard()
{
    const DiagnosticLog::Span span("paste");
    const QMimeData* clip = QApplication::clipboard()->mimeData();
    // [SP-180] Q1: only THIS editor's last cross-scene copy, and only while the clipboard still holds it — anything copied
    // since, here or in any other app, replaced the token. A copy from another project's window carries its own token.
    if (sceneDoc_ != nullptr && clip != nullptr && !heldRange_.isEmpty() && clip->hasFormat(kFragmentTokenMime)
        && QString::fromUtf8(clip->data(kFragmentTokenMime)) == heldToken_) {
        emit structuredPasteRequested(heldRange_);
        return;
    }
    // The flat paste: widen a selection that would split an escape pair, then Qt's paste → `insertFromMimeData`.
    QTextCursor c = textCursor();
    if (c.hasSelection()) {
        int s = c.selectionStart();
        int e = c.selectionEnd();
        widenOverPairs(s, e);
        c.setPosition(s);
        c.setPosition(e, QTextCursor::KeepAnchor);
        setTextCursor(c);
    }
    paste();
}

std::optional<ManuscriptEditor::CrossSceneCopy> ManuscriptEditor::crossSceneCopy() const
{
    const QTextCursor cursor = textCursor();
    if (sceneDoc_ == nullptr || !cursor.hasSelection()) {
        return std::nullopt;
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    // One entry per scene the selection touches, in reading order (Apple: `copiedRange`): the selected part of each body —
    // and an EMPTY entry for a scene whose start lies inside the selection with none of its text selected (a boundary
    // crossed). Crossing needs two scenes with text selected (`selectionCrossesBoundary`).
    QJsonArray entries;
    QStringList sources;
    QStringList presented;
    int withText = 0;
    const QList<SceneSegment>& segs = sceneDoc_->segments();
    for (int i = 0; i < segs.size(); ++i) {
        const SceneSegment& seg = segs.at(i);
        const int a = std::max(start, seg.bodyStart);
        const int b = std::min(end, seg.bodyStart + seg.bodyLength);
        if (b > a) {
            ++withText;
        } else if (!(start <= seg.bodyStart && seg.bodyStart < end)) {
            continue;
        }
        const int from = b > a ? a - seg.bodyStart : 0;
        const int to = b > a ? b - seg.bodyStart : 0;
        const QString body = sceneDoc_->bodyText(i);
        // ✅ [T-0591]: each scene's piece is BALANCED (EP-046 AC12) — the stateless markdown call, on the piece's text.
        const auto copy = ScriviBridge::markdownBalancedCopy(body, from, to);
        entries.append(QJsonObject{{QStringLiteral("sceneID"), seg.sceneID},
                                   {QStringLiteral("chapterID"), seg.chapterID},
                                   {QStringLiteral("chapterTitle"), seg.chapterTitle},
                                   {QStringLiteral("sceneTitle"), seg.title},
                                   {QStringLiteral("start"), qint64(body.left(from).toUtf8().size())},
                                   {QStringLiteral("length"), 0},
                                   {QStringLiteral("sceneLength"), qint64(body.toUtf8().size())},
                                   {QStringLiteral("text"), copy.first}});
        sources.append(copy.first);
        presented.append(copy.second);
    }
    if (withText < 2) {
        return std::nullopt;
    }
    const QJsonObject range{{QStringLiteral("schema"), QStringLiteral("scrivi.range.v1")}, {QStringLiteral("scenes"), entries}};
    return CrossSceneCopy{normalizedRange(range), sources.join(QStringLiteral("\n\n")), presented.join(QStringLiteral("\n\n"))};
}

QJsonObject ManuscriptEditor::normalizedRange(QJsonObject range)
{
    QJsonArray entries = range.value(QStringLiteral("scenes")).toArray();
    const qsizetype n = entries.size();
    for (qsizetype i = 0; i < n; ++i) {
        QJsonObject e = entries.at(i).toObject();
        const qint64 length = e.value(QStringLiteral("text")).toString().toUtf8().size();
        const qint64 start = i > 0 ? 0 : e.value(QStringLiteral("start")).toInteger();
        const qint64 whole = e.value(QStringLiteral("sceneLength")).toInteger();
        e.insert(QStringLiteral("start"), start);
        e.insert(QStringLiteral("length"), length);
        e.insert(QStringLiteral("sceneLength"), i + 1 < n ? start + length : std::max(whole, start + length));
        entries.replace(i, e);
    }
    range.insert(QStringLiteral("scenes"), entries);
    return range;
}

bool ManuscriptEditor::removeSelectionForPaste()
{
    if (sceneDoc_ == nullptr) {
        return false;
    }
    const QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        return sceneDoc_->isEditablePosition(cursor.position());
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);   // EP-049: never leave half an escape pair
    if (sceneDoc_->isEditableRange(start, end)) {
        replaceRange(start, end, QString());   // the balanced removal — a typing step the shell commits before the paste
        return true;
    }
    return deleteAcrossScenes(start, end, /*copyFirst=*/false, QStringLiteral("delete"));   // [I-0270]
}

void ManuscriptEditor::insertFromMimeData(const QMimeData* source)
{
    const DiagnosticLog::Span span("paste: insert from the clipboard");
    // Paste / drop: gate the same way. The insertion replaces the selection (or
    // lands at the caret) and must stay inside one body.
    if (sceneDoc_ != nullptr) {
        const QTextCursor cursor = textCursor();
        int start = cursor.selectionStart();
        int end   = cursor.selectionEnd();
        widenOverPairs(start, end);   // EP-049: never split an escape pair (AC4 / AC4b)
        if (!sceneDoc_->isEditableRange(start, end)) {
            return;   // paste target touches a boundary — reject
        }
        // ✅ EP-049 (Apple: `readSelection`): Scrivi's OWN copy is restored EXACTLY — re-escaping it
        // would turn intended markup (R2, e.g. a `##` heading) literal; foreign text is escaped (R1).
        QString text;
        if (source->hasFormat(kSourceMime)) {
            text = QString::fromUtf8(source->data(kSourceMime));
        } else if (source->hasText()) {
            text = ManuscriptEscapes::escape(source->text());
        } else {
            return;
        }
        pastingOwnCopy_ = source->hasFormat(kSourceMime);
        emit pasteOrCutBegins(QStringLiteral("paste"), QString());   // EP-026 S2: a paste is its own undo step
        replaceRange(start, end, text);
        emit pasteOrCutEnded(QStringLiteral("paste"));
        pastingOwnCopy_ = false;
        return;
    }
    QPlainTextEdit::insertFromMimeData(source);
}

void ManuscriptEditor::paintEvent(QPaintEvent* event)
{
    // Text first — then overlay the separator rules on top of the (blank) gap blocks.
    QPlainTextEdit::paintEvent(event);

    if (sceneDoc_ == nullptr) {
        return;
    }
    const QList<int> separators = sceneDoc_->sceneSeparatorPositions();
    if (separators.isEmpty()) {
        return;
    }

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing, false);   // crisp 1px hairline
    // ⛔ [I-0252] Linux half (2026-09-29) — THIS WAS `palette().color(QPalette::Mid)`,
    // and that is the defect `ThemeColours` exists to prevent.
    //
    // ⚠️ USER: *"on Linux it is only barely visible in dark mode."*
    // ⛔ `Mid` is a STRUCTURAL role with NO contrast guarantee — ✅ [I-0186] already
    // MEASURED it at **1.07:1** on Yaru-dark (`ThemeColours.hpp`), which is the same
    // invisibility, in the same theme, for the same reason.
    // ⚠️ The old comment called `Mid` *"the Qt analogue of NSColor.separatorColor"* —
    // ⛔ and `separatorColor` is exactly what [I-0252] removed on macOS.
    //
    // ✅ DERIVED from `WindowText` on `Base`, so it is correct in a light theme, a
    // dark theme, and a theme nobody has written yet.
    // ⚠️ Deliberately fainter than `deemphasised()`: a divider is a MARK, not text,
    // and must not compete with prose (macOS settled the same band by measurement).
    const QColor ruleColor = ThemeColours::rule(palette());
    painter.setPen(QPen(ruleColor, 1));

    const QRectF vp = viewport()->rect();
    QTextDocument* doc = document();

    for (const int pos : separators) {
        // The separator's blank block is the one immediately BEFORE the following scene's
        // first block (that scene begins at `pos`). Take the block just above it.
        const QTextBlock sceneBlock = doc->findBlock(pos);
        if (!sceneBlock.isValid()) {
            continue;
        }
        const QTextBlock gapBlock = sceneBlock.previous();
        if (!gapBlock.isValid()) {
            continue;
        }
        // Block geometry is document-relative; translate by contentOffset() to viewport
        // coordinates (the standard QPlainTextEdit mapping used by line-number gutters).
        const QRectF r =
            blockBoundingGeometry(gapBlock).translated(contentOffset());
        if (r.bottom() < vp.top() || r.top() > vp.bottom()) {
            continue;   // off-screen — skip (viewport-culled)
        }
        const qreal y = std::round(r.center().y()) + 0.5;   // pixel-center the hairline
        painter.drawLine(QPointF(r.left() + kRuleInset, y),
                         QPointF(r.right() - kRuleInset, y));
    }
}

void ManuscriptEditor::copyToClipboard()
{
    const QScopedValueRollback<bool> clipboard(clipboardCopy_, true);
    copy();
}

void ManuscriptEditor::cutSelection()
{
    const DiagnosticLog::Span span("cut");
    const QTextCursor cursor = textCursor();
    if (sceneDoc_ == nullptr || !cursor.hasSelection()) {
        cut();
        return;
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);   // EP-049: a cut never leaves half an escape pair
    if (sceneDoc_->isEditableRange(start, end)) {
        QTextCursor widened(document());
        widened.setPosition(start);
        widened.setPosition(end, QTextCursor::KeepAnchor);
        setTextCursor(widened);
        copyToClipboard();                   // the balanced copy (createMimeDataFromSelection)
        emit pasteOrCutBegins(QStringLiteral("cut"), QString());   // EP-026 S2: a cut is its own undo step
        replaceRange(start, end, QString());  // ✅ SP-175: the balanced removal — a span it cuts into is closed and re-opened
        emit pasteOrCutEnded(QStringLiteral("cut"));
        return;
    }
    // Across scenes → per-scene removal; touching only a boundary → refused (no-op).
    deleteAcrossScenes(start, end, /*copyFirst=*/true);
}

// ── EP-026 S5 (SP-179 / T-0606): the copy buffers ─────────────────────────────────────────────────────────────────
// Apple: the coordinator's `copyIntoBuffer` / `cutIntoBuffer` / `pasteFromBuffer` (Scrivi/Views/ManuscriptTextView.swift).

QString ManuscriptEditor::selectionSource() const
{
    const QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        return {};
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    return balancedCopyOf(start, end).first;
}

bool ManuscriptEditor::cutSelectionIntoBuffer(const QString& bufferID,
                                              const std::function<bool(const QString& source, const QJsonObject& range)>& store)
{
    const QTextCursor cursor = textCursor();
    if (sceneDoc_ == nullptr || !cursor.hasSelection()) {
        return false;
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);   // EP-049: a cut never leaves half an escape pair
    const bool inOneScene = sceneDoc_->isEditableRange(start, end);
    if (!inOneScene && sceneDoc_->crossSceneCuts(start, end).isEmpty()) {
        return false;   // it touches only a boundary — refused, as Ctrl+X refuses it
    }
    // The slot FIRST: the text must survive a store failure and the delete being undone. Across scenes, the Range.
    const auto cross = inOneScene ? std::nullopt : crossSceneCopy();
    const bool stored = cross ? store(cross->source, cross->range)
                              : store(balancedCopyOf(start, end).first, QJsonObject());
    if (!stored) {
        return false;
    }
    if (!inOneScene) {
        return deleteAcrossScenes(start, end, /*copyFirst=*/false, QStringLiteral("cut"));   // [I-0270]
    }
    emit pasteOrCutBegins(QStringLiteral("cut"), bufferID);   // ONE `cut` step, tagged with the slot (Trade T3)
    replaceRange(start, end, QString());                       // ✅ SP-175: the balanced removal
    emit pasteOrCutEnded(QStringLiteral("cut"));
    return true;
}

bool ManuscriptEditor::pasteStored(const QString& source)
{
    if (sceneDoc_ == nullptr || source.isEmpty()) {
        return false;
    }
    const QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    if (!sceneDoc_->isEditableRange(start, end)) {
        return false;   // the target touches a boundary — refused, as any paste
    }
    pastingOwnCopy_ = true;   // ✅ [I-0289]: a slot is Scrivi's own copy — it keeps ITS formatting
    emit pasteOrCutBegins(QStringLiteral("paste"), QString());
    replaceRange(start, end, source);
    emit pasteOrCutEnded(QStringLiteral("paste"));
    pastingOwnCopy_ = false;
    return true;
}

bool ManuscriptEditor::typeOver(const QString& typed)
{
    if (sceneDoc_ == nullptr || isReadOnly()) {
        return false;
    }
    const QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    if (!sceneDoc_->isEditableRange(start, end)) {
        return false;
    }
    replaceRange(start, end, ManuscriptEscapes::escape(typed));   // as `handleTyping`
    return true;
}

void ManuscriptEditor::snapForTyping(int& start, int& end) const
{
    if (start < end) {
        snapSelection(start, end, end);
    }
    widenOverPairs(start, end);
}

bool ManuscriptEditor::deleteAcrossScenes(int start, int end, bool copyFirst, const QString& kind)
{
    if (sceneDoc_ == nullptr) {
        return false;
    }
    const QList<QPair<int, int>> cuts = sceneDoc_->crossSceneCuts(start, end);
    if (cuts.isEmpty()) {
        return false;
    }
    if (copyFirst) {
        copyToClipboard();
    }
    // Back to front, ONE edit per body, so each contentsChange lands inside a single
    // body and SceneDocument::applyContentsChange reconciles it (an edit spanning a
    // boundary would be charged entirely to one scene).
    // ✅ SP-175: each part through `replaceRange`, so every scene stays balanced (Apple: T-0591's per-scene cut).
    // EP-026 S2: the parts are ONE undo step — the shell reports them together.
    emit groupedEditBegins();
    for (int k = cuts.size() - 1; k >= 0; --k) {
        replaceRange(cuts.at(k).first, cuts.at(k).second, QString());
    }
    emit groupedEditEnded(!kind.isEmpty() ? kind : copyFirst ? QStringLiteral("cut") : QStringLiteral("delete"));
    QTextCursor caret = textCursor();
    caret.setPosition(cuts.first().first);
    setTextCursor(caret);
    return true;
}

// ── SP-175 / T-0602 (EP-048 S3b): the Format commands and the pending pair ──────────────────────────────────────
// Apple: `ManuscriptNSTextView.applyFormat`, `insertPendingPair`, `removePendingPair` (Scrivi/Views/ManuscriptTextView.swift).

QString ManuscriptEditor::textOf(int start, int end) const
{
    QTextCursor c(document());
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    return c.selectedText().replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
}

void ManuscriptEditor::applyComputed(int base, const MarkdownEdit& edit)
{
    computed_ = true;
    replaceRange(base + edit.range.start, base + edit.range.end, edit.replacement);
    computed_ = false;
    QTextCursor c = textCursor();
    c.setPosition(base + edit.selection.start);
    c.setPosition(base + edit.selection.end, QTextCursor::KeepAnchor);
    setTextCursor(c);
}

void ManuscriptEditor::applyFormat(Format format)
{
    if (sceneDoc_ == nullptr) {
        return;
    }
    const QTextCursor cursor = textCursor();
    const int start = cursor.selectionStart();
    const int end = cursor.selectionEnd();
    const auto scene = sceneDoc_->sceneHolding(start, end);
    std::optional<MarkdownEdit> edit;
    if (format == Format::Bold || format == Format::Italic) {
        const int bit = format == Format::Bold ? MarkdownAnalysis::kBold : MarkdownAnalysis::kItalic;
        const QString markers = format == Format::Bold ? QStringLiteral("**") : QStringLiteral("*");
        if (start == end) {
            // ✅ Q1: the same command on a pending pair takes it away again.
            if (const auto& p = sceneDoc_->pending(); p && start == p->caret()) {
                const int at = p->start;
                removePendingPair();
                QTextCursor c = textCursor();
                c.setPosition(at);
                setTextCursor(c);
                return;
            }
            if (!scene) {
                QApplication::beep();
                return;
            }
            // ✅ Q1: IN a word → format the word, the caret stays where it was; BETWEEN words → a pending pair.
            const int local = start - scene->start;
            const auto word = ScriviBridge::markdownWordRange(scene->text, local);
            if (!word) {
                insertPendingPair(start, markers);
                return;
            }
            edit = ScriviBridge::markdownEmphasisEdit(scene->text, word->start, word->end, bit, local);
        } else if (scene) {
            edit = ScriviBridge::markdownEmphasisEdit(scene->text, start - scene->start, end - scene->start, bit);
        }
    } else if (scene) {
        QString name;
        switch (format) {
        case Format::Heading1: name = QStringLiteral("heading1"); break;
        case Format::Heading2: name = QStringLiteral("heading2"); break;
        case Format::Heading3: name = QStringLiteral("heading3"); break;
        case Format::Body: name = QStringLiteral("body"); break;
        case Format::BulletList: name = QStringLiteral("bulletList"); break;
        case Format::NumberedList: name = QStringLiteral("numberedList"); break;
        case Format::Bold:
        case Format::Italic: break;
        }
        edit = ScriviBridge::markdownParagraphEdit(scene->text, start - scene->start, end - scene->start, name);
    }
    if (!scene || !edit) {
        QApplication::beep();   // across a boundary, nothing to format, or not writable balanced
        return;
    }
    applyComputed(scene->start, *edit);
}

void ManuscriptEditor::insertPendingPair(int pos, const QString& markers)
{
    // ✅ Document only: SceneDocument knows the pair FIRST (so the presenter dims it as it is drawn), and the insert runs
    // under `pendingEditInProgress`, so the shell does not mark the scene dirty — it is not scene text.
    // ⚠️ The insert moves the editor's own caret, and that must not count as LEAVING the pair (`keepingPending_`).
    keepingPending_ = true;
    sceneDoc_->setPending(SceneDocument::PendingPair{pos, static_cast<int>(markers.size())});
    sceneDoc_->setPendingEditInProgress(true);
    QTextCursor c(document());
    c.setPosition(pos);
    c.insertText(markers + markers);
    sceneDoc_->setPendingEditInProgress(false);
    QTextCursor caret = textCursor();
    caret.setPosition(pos + markers.size());
    setTextCursor(caret);
    keepingPending_ = false;
}

void ManuscriptEditor::removePendingPair()
{
    if (sceneDoc_ == nullptr || !sceneDoc_->pending()) {
        return;
    }
    const SceneDocument::PendingPair p = *sceneDoc_->pending();
    sceneDoc_->setPending(std::nullopt);
    const int len = p.end() - p.start;
    // Only the markers the pair put there (Apple makes the same check).
    if (p.end() > document()->characterCount() - 1 || textOf(p.start, p.end()) != QString(len, QLatin1Char('*'))) {
        return;
    }
    sceneDoc_->setPendingEditInProgress(true);
    QTextCursor c(document());
    c.setPosition(p.start);
    c.setPosition(p.end(), QTextCursor::KeepAnchor);
    c.removeSelectedText();
    sceneDoc_->setPendingEditInProgress(false);
}

void ManuscriptEditor::leavePendingPair()
{
    if (keepingPending_ || !sceneDoc_->pending()) {
        return;
    }
    const SceneDocument::PendingPair p = *sceneDoc_->pending();
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection() && cursor.position() == p.caret()) {
        return;
    }
    // ✅ [SP-163] Q1: the caret LEFT an empty pending pair — remove it, and keep the caret on the same character.
    const int len = p.end() - p.start;
    auto shifted = [&](int x) { return x >= p.end() ? x - len : (x > p.start ? p.start : x); };
    const int anchor = shifted(cursor.anchor());
    const int position = shifted(cursor.position());
    removePendingPair();
    normalizingCaret_ = true;
    cursor = textCursor();
    cursor.setPosition(anchor);
    cursor.setPosition(position, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    normalizingCaret_ = false;
}
