#include "ManuscriptEditor.hpp"

#include <QKeyEvent>
#include <QTextCursor>

#include "SceneDocument.hpp"

ManuscriptEditor::ManuscriptEditor(QWidget* parent) : QPlainTextEdit(parent)
{
    document()->setUndoRedoEnabled(false);   // ⌘Z reserved for EP-026 custom history

    // Keep the caret out of protected boundary text (T-0246).
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &ManuscriptEditor::normalizeCaret);
}

void ManuscriptEditor::normalizeCaret()
{
    if (sceneDoc_ == nullptr || normalizingCaret_) {
        return;
    }
    QTextCursor cursor = textCursor();
    // Don't fight an active selection (the user may be selecting across a body); the
    // edit guard already prevents boundary-touching edits. Only normalize a plain
    // caret that has come to rest inside a heading/separator gap.
    if (cursor.hasSelection()) {
        lastCaretPos_ = cursor.position();
        return;
    }
    const int pos = cursor.position();
    // Snap in the DIRECTION the caret was travelling, so arrow keys cross a boundary into
    // the next/previous scene instead of getting stuck at it. Forward (Down/Right/typing)
    // when the caret advanced; backward (Up/Left) when it retreated. A same-position event
    // (no movement) keeps the previous direction bias as "forward" by default.
    const bool movingForward = (pos >= lastCaretPos_);
    const int snapped = sceneDoc_->editablePositionInDirection(pos, movingForward);
    if (snapped == pos) {
        lastCaretPos_ = pos;
        return;
    }
    normalizingCaret_ = true;
    cursor.setPosition(snapped);
    setTextCursor(cursor);
    normalizingCaret_ = false;
    lastCaretPos_ = snapped;
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

void ManuscriptEditor::keyPressEvent(QKeyEvent* event)
{
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

    // No document loaded, or a non-modifying key (navigation, copy, modifiers):
    // let the base class handle it.
    if (sceneDoc_ == nullptr || !isModifyingKey(event)) {
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    int start = 0;
    int end = 0;
    if (!modifiedRangeFor(event, start, end)) {
        return;   // e.g. Backspace at document start — nothing to do; swallow.
    }

    // Allow the edit only if the touched range stays entirely inside one scene's
    // editable body. Otherwise swallow the keystroke (boundary stays intact).
    if (!sceneDoc_->isEditableRange(start, end)) {
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}

void ManuscriptEditor::insertFromMimeData(const QMimeData* source)
{
    // Paste / drop: gate the same way. The insertion replaces the selection (or
    // lands at the caret) and must stay inside one body.
    if (sceneDoc_ != nullptr) {
        const QTextCursor cursor = textCursor();
        const int start = cursor.selectionStart();
        const int end   = cursor.selectionEnd();
        if (!sceneDoc_->isEditableRange(start, end)) {
            return;   // paste target touches a boundary — reject
        }
    }
    QPlainTextEdit::insertFromMimeData(source);
}
