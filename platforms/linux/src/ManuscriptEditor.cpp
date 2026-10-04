#include "ManuscriptEditor.hpp"

#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>

#include "ThemeColours.hpp"
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include <cmath>

#include "ManuscriptEscapes.hpp"
#include "SceneDocument.hpp"

#include <QInputMethodEvent>
#include <QMimeData>

namespace {
// Horizontal inset (px) at each end of the rule, matching Apple's DividerAttachmentCell
// (which insets 20pt). Keeps the line from touching the text margins.
constexpr int kRuleInset = 20;
// EP-049: Scrivi's own clipboard data carries the STORED source here, so an in-app paste restores it
// exactly (Apple remembers its own pasteboard write — `ownCopy`; same result, Qt means).
const QString kSourceMime = QStringLiteral("application/x-scrivi-manuscript-source");
}   // namespace

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
    // Apple's Option-Return (`insertNewlineIgnoringFieldEditor:`) stores ONE `\n`. ⚠️ [T-0584] will
    // rule what it SHOULD store; until then, Apple's behaviour today. (M1: Qt's own Alt-Return inserted
    // nothing.)
    if (event->modifiers() & Qt::AltModifier) {
        if (sceneDoc_->isEditableRange(selStart, selEnd)) {
            replaceRange(selStart, selEnd, QStringLiteral("\n"));
        }
        return true;
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
    int start = 0;
    int end = 0;
    if (cursor.hasSelection()) {
        start = cursor.selectionStart();
        end = cursor.selectionEnd();
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

QMimeData* ManuscriptEditor::createMimeDataFromSelection() const
{
    const QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        return QPlainTextEdit::createMimeDataFromSelection();
    }
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    widenOverPairs(start, end);
    QTextCursor sel(document());
    sel.setPosition(start);
    sel.setPosition(end, QTextCursor::KeepAnchor);
    QString source = sel.selectedText();
    source.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
    auto* mime = new QMimeData;
    // Apple: `writeSelection` — other apps get the writer's text, not backslashes.
    mime->setText(ManuscriptEscapes::map(source).presented);
    mime->setData(kSourceMime, source.toUtf8());
    return mime;
}

void ManuscriptEditor::insertFromMimeData(const QMimeData* source)
{
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
        replaceRange(start, end, text);
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

void ManuscriptEditor::cutSelection()
{
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
        cut();   // inside one scene — the ordinary cut
        return;
    }
    // Across scenes → per-scene removal; touching only a boundary → refused (no-op).
    deleteAcrossScenes(start, end, /*copyFirst=*/true);
}

bool ManuscriptEditor::deleteAcrossScenes(int start, int end, bool copyFirst)
{
    if (sceneDoc_ == nullptr) {
        return false;
    }
    const QList<QPair<int, int>> cuts = sceneDoc_->crossSceneCuts(start, end);
    if (cuts.isEmpty()) {
        return false;
    }
    if (copyFirst) {
        copy();
    }
    // Back to front, ONE edit per body, so each contentsChange lands inside a single
    // body and SceneDocument::applyContentsChange reconciles it (an edit spanning a
    // boundary would be charged entirely to one scene).
    for (int k = cuts.size() - 1; k >= 0; --k) {
        QTextCursor c(document());
        c.setPosition(cuts.at(k).first);
        c.setPosition(cuts.at(k).second, QTextCursor::KeepAnchor);
        c.removeSelectedText();
    }
    QTextCursor caret = textCursor();
    caret.setPosition(cuts.first().first);
    setTextCursor(caret);
    return true;
}
