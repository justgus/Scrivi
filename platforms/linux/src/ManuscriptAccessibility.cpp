#include "ManuscriptAccessibility.hpp"

#include <QAbstractTextDocumentLayout>
#include <QAccessible>
#include <QAccessibleWidget>
#include <QCoreApplication>
#include <QPointer>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextBoundaryFinder>
#include <QTextCursor>
#include <QTextLayout>
#include <QTimer>

#include <algorithm>
#include <vector>

#include "DiagnosticLog.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptFind.hpp"
#include "SceneDocument.hpp"

namespace ManuscriptAccessibility {

namespace {

class Interface : public QAccessibleWidget, public QAccessibleTextInterface
{
public:
    explicit Interface(ManuscriptEditor* editor) : QAccessibleWidget(editor, QAccessible::EditableText), editor_(editor) {}
    ~Interface() override { QObject::disconnect(changes_); }

    void* interface_cast(QAccessible::InterfaceType t) override
    {
        if (t == QAccessible::TextInterface) {
            return static_cast<QAccessibleTextInterface*>(this);
        }
        return QAccessibleWidget::interface_cast(t);
    }

    QAccessible::State state() const override
    {
        QAccessible::State s = QAccessibleWidget::state();
        s.editable = !editor_->isReadOnly();
        s.multiLine = true;
        s.selectableText = true;
        return s;
    }

    QString text(QAccessible::Text t) const override
    {
        return t == QAccessible::Value ? text(0, characterCount()) : QAccessibleWidget::text(t);
    }

    // ── The map: the page as presented when there is a manuscript, the document as it is otherwise ──

    // Brought up to date at every call: only the scenes an edit touched are rebuilt (`PresentedText::refresh`).
    const ManuscriptFind::PresentedText* page() const
    {
        SceneDocument* sd = editor_->sceneDocument();
        QTextDocument* doc = editor_->document();
        if (sd == nullptr || editor_->presenter() == nullptr || sd->document() != doc) {
            return nullptr;
        }
        if (doc != doc_ || sd != sceneDoc_) {
            QObject::disconnect(changes_);
            page_ = {};
            doc_ = doc;
            sceneDoc_ = sd;
            changes_ = QObject::connect(doc, &QTextDocument::contentsChange, editor_, [this](int pos, int, int added) {
                page_.markEditedRange(*sceneDoc_, pos, pos + added);
            });
        }
        page_.refresh(*sd, editor_->presenter());
        return &page_;
    }
    int toPage(int pos) const
    {
        const auto* p = page();
        return p != nullptr ? p->pageIndex(pos) : pos;
    }
    int toDocument(int k) const
    {
        const auto* p = page();
        return p != nullptr ? p->documentPosition(k) : std::clamp(k, 0, editor_->document()->characterCount() - 1);
    }
    // A page range's document range: its end just after its last presented character (Apple's `storageRange`).
    std::pair<int, int> documentRange(int start, int end) const
    {
        const int a = toDocument(start);
        return {a, end <= start ? a : toDocument(std::min(end, characterCount()) - 1) + 1};
    }

    // ── Text ──

    int characterCount() const override
    {
        const auto* p = page();
        return p != nullptr ? p->pageLength() : editor_->document()->characterCount() - 1;
    }
    QString text(int start, int end) const override
    {
        if (const auto* p = page()) {
            return p->pageText(start, end);
        }
        QTextCursor c(editor_->document());
        c.setPosition(std::clamp(start, 0, characterCount()));
        c.setPosition(std::clamp(end, 0, characterCount()), QTextCursor::KeepAnchor);
        return c.selectedText().replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    }

    // The page range of the unit of `type` holding page unit `k`.
    std::pair<int, int> segmentAt(int k, QAccessible::TextBoundaryType type) const
    {
        const int n = characterCount();
        if (type == QAccessible::NoBoundary) {
            return {0, n};
        }
        k = std::clamp(k, 0, std::max(n - 1, 0));
        if (type == QAccessible::CharBoundary || n == 0) {
            return {k, std::min(k + 1, n)};
        }
        // Paragraphs are the document's blocks, mapped (a divider's words are its blank block).
        QTextDocument* doc = editor_->document();
        const int pos = toDocument(k);
        const QTextBlock block = doc->findBlock(pos);
        int a = toPage(block.position());
        int b = std::max(toPage(block.position() + block.length()), k + 1);
        a = std::min(a, k);
        if (type == QAccessible::LineBoundary) {
            // ✅ VISUAL lines (Apple: AppKit's); hidden characters are zero-width, so the lines are the same in both spaces.
            doc->documentLayout()->blockBoundingRect(block);   // lays the block out if it has not been yet
            const QTextLayout* layout = block.layout();
            if (layout != nullptr && layout->lineCount() > 1) {
                const QTextLine line = layout->lineForTextPosition(pos - block.position());
                if (line.isValid()) {
                    const int la = toPage(block.position() + line.textStart());
                    const int lb = toPage(block.position() + line.textStart() + line.textLength());
                    const bool last = line.lineNumber() == layout->lineCount() - 1;
                    a = std::max(a, std::min(la, k));
                    b = last ? b : std::min(b, std::max(lb, k + 1));
                }
            }
            return {a, b};
        }
        if (type == QAccessible::ParagraphBoundary) {
            return {a, b};
        }
        // Words and sentences: Qt's boundary finder over the paragraph, as presented.
        const QString para = text(a, b);
        QTextBoundaryFinder f(type == QAccessible::WordBoundary ? QTextBoundaryFinder::Word : QTextBoundaryFinder::Sentence,
                              para);
        f.setPosition(k - a);
        const int s = f.isAtBoundary() ? k - a : std::max(int(f.toPreviousBoundary()), 0);
        f.setPosition(k - a);
        const qsizetype e = f.toNextBoundary();
        return {a + s, e < 0 ? b : a + int(e)};
    }
    QString textAtOffset(int k, QAccessible::TextBoundaryType type, int* start, int* end) const override
    {
        if (type == QAccessible::CharBoundary && k >= characterCount()) {
            *start = *end = characterCount();
            return {};
        }
        const auto [a, b] = segmentAt(k, type);
        *start = a;
        *end = b;
        return text(a, b);
    }
    QString textBeforeOffset(int k, QAccessible::TextBoundaryType type, int* start, int* end) const override
    {
        const int at = segmentAt(k, type).first;
        if (at <= 0) {
            *start = *end = 0;
            return {};
        }
        return textAtOffset(at - 1, type, start, end);
    }
    QString textAfterOffset(int k, QAccessible::TextBoundaryType type, int* start, int* end) const override
    {
        const int at = segmentAt(k, type).second;
        if (at >= characterCount()) {
            *start = *end = characterCount();
            return {};
        }
        return textAtOffset(at, type, start, end);
    }

    // ── Caret and selection (AC2) ──

    int cursorPosition() const override { return toPage(editor_->textCursor().position()); }
    void setCursorPosition(int k) override { editor_->placeCaret(toDocument(k)); }

    void selection(int i, int* start, int* end) const override
    {
        const QTextCursor c = editor_->textCursor();
        *start = *end = 0;
        if (i == 0 && c.hasSelection()) {
            *start = toPage(c.selectionStart());
            *end = toPage(c.selectionEnd());
        }
    }
    int selectionCount() const override { return editor_->textCursor().hasSelection() ? 1 : 0; }
    void addSelection(int start, int end) override { setSelection(0, start, end); }
    void removeSelection(int i) override
    {
        if (i == 0) {
            QTextCursor c = editor_->textCursor();
            c.clearSelection();
            editor_->setTextCursor(c);
        }
    }
    // The ends go through the editor's own selection rule (`normalizeCaret` → `snapSelection`).
    void setSelection(int i, int start, int end) override
    {
        if (i != 0) {
            return;
        }
        const auto [a, b] = documentRange(start, end);
        QTextCursor c = editor_->textCursor();
        c.setPosition(a);
        c.setPosition(b, QTextCursor::KeepAnchor);
        editor_->setTextCursor(c);
    }

    // ── Geometry ──

    QRect characterRect(int k) const override
    {
        QTextCursor c(editor_->document());
        c.setPosition(toDocument(k));
        QRect r = editor_->cursorRect(c);
        const QString ch = text(k, k + 1);
        r.setWidth(std::max(editor_->fontMetrics().horizontalAdvance(ch), 1));
        return {editor_->viewport()->mapToGlobal(r.topLeft()), r.size()};
    }
    int offsetAtPoint(const QPoint& global) const override
    {
        return toPage(editor_->cursorForPosition(editor_->viewport()->mapFromGlobal(global)).position());
    }
    // Scrolls without moving the caret (Apple's `setAccessibilityVisibleCharacterRange`, mapped — SP-172 Plan 5).
    void scrollToSubstring(int start, int) override
    {
        const QTextBlock block = editor_->document()->findBlock(toDocument(start));
        editor_->verticalScrollBar()->setValue(block.firstLineNumber());
    }
    QString attributes(int, int* start, int* end) const override
    {
        *start = 0;
        *end = characterCount();
        return {};
    }

private:
    ManuscriptEditor* editor_;
    mutable ManuscriptFind::PresentedText page_;
    mutable QTextDocument* doc_ = nullptr;
    mutable SceneDocument* sceneDoc_ = nullptr;
    mutable QMetaObject::Connection changes_;
};

QAccessibleInterface* factory(const QString&, QObject* object)
{
    if (auto* editor = qobject_cast<ManuscriptEditor*>(object)) {
        return new Interface(editor);
    }
    return nullptr;
}

QAccessible::UpdateHandler previousHandler = nullptr;
void translate(QAccessibleEvent* event);

// ⚠️ An installed handler REPLACES the platform bridge's delivery, so a translated event is sent on with the previous
// handler (normally none → AT-SPI) back in place.
void forward(QAccessibleEvent* event)
{
    QAccessible::installUpdateHandler(previousHandler);
    QAccessible::updateAccessibility(event);
    QAccessible::installUpdateHandler(&translate);
}

// Qt's text control reports the editor's caret and edits in STORED positions. They are re-sent in page positions once
// the edit has settled (the scene map and the presenter catch up after Qt's event).
// ✅ [I-0349]: COALESCED — every event of one turn is collected and handled ONCE at its end. A highlight pass raises a
// `TextUpdated` for every block (a load: the whole manuscript, thousands), and handling each — a page refresh, a text
// extraction — held the window 5.5 s after every load. Per turn: ONE insert or removal (a keystroke) is re-sent, in page
// positions, with its text as presented; a burst (a load, a structural edit, a Replace All) re-sends NO text — Orca re-reads
// the caret's line. A `TextUpdated` (format only, or a same-length replace) is never re-sent. The caret and the selection
// are re-sent once, as they are at the end of the turn.
struct Pending {
    QAccessible::Event type;
    int pos = 0, length = 0;
    QString removed;
};
QPointer<ManuscriptEditor> pendingEditor;
std::vector<Pending> pending;

void flush()
{
    const DiagnosticLog::Span span("accessibility: a turn's events re-sent in page positions");
    const QPointer<ManuscriptEditor> editor = std::exchange(pendingEditor, nullptr);
    const std::vector<Pending> events = std::exchange(pending, {});
    QAccessibleInterface* iface = editor ? QAccessible::queryAccessibleInterface(editor) : nullptr;
    QAccessibleTextInterface* t = iface != nullptr ? iface->textInterface() : nullptr;
    if (t == nullptr) {
        return;
    }
    auto* self = static_cast<Interface*>(t);
    bool caret = false, selection = false;
    std::vector<const Pending*> edits;
    for (const Pending& p : events) {
        caret = caret || p.type == QAccessible::TextCaretMoved;
        selection = selection || p.type == QAccessible::TextSelectionChanged;
        if (p.type == QAccessible::TextInserted || p.type == QAccessible::TextRemoved) {
            edits.push_back(&p);
        }
    }
    if (edits.size() == 1) {
        const Pending& p = *edits.front();
        const int a = self->toPage(p.pos);
        if (p.type == QAccessible::TextInserted) {
            QAccessibleTextInsertEvent e(iface, a, t->text(a, self->toPage(p.pos + p.length)));
            forward(&e);
        } else {
            QAccessibleTextRemoveEvent e(iface, a, p.removed);
            forward(&e);
        }
    }
    if (caret || edits.size() > 1) {
        QAccessibleTextCursorEvent e(iface, t->cursorPosition());
        forward(&e);
    }
    if (selection) {
        int a = 0, b = 0;
        t->selection(0, &a, &b);
        QAccessibleTextSelectionEvent e(iface, a, b);
        e.setCursorPosition(t->cursorPosition());
        forward(&e);
    }
}

void translate(QAccessibleEvent* event)
{
    auto* editor = qobject_cast<ManuscriptEditor*>(event->object());
    const QAccessible::Event type = event->type();
    const bool text = type == QAccessible::TextCaretMoved || type == QAccessible::TextSelectionChanged
                      || type == QAccessible::TextInserted || type == QAccessible::TextRemoved
                      || type == QAccessible::TextUpdated;
    if (editor == nullptr || !text) {
        forward(event);
        return;
    }
    if (type == QAccessible::TextUpdated) {
        return;   // a format change (the highlighter) or a same-length replace: nothing for Orca to say
    }
    if (pendingEditor != nullptr && pendingEditor != editor) {
        flush();   // another window's turn: its events first
    }
    Pending p{type};
    if (type == QAccessible::TextInserted) {
        const auto* e = static_cast<QAccessibleTextInsertEvent*>(event);
        p.pos = e->changePosition();
        p.length = int(e->textInserted().size());
    } else if (type == QAccessible::TextRemoved) {
        const auto* e = static_cast<QAccessibleTextRemoveEvent*>(event);
        p.pos = e->changePosition();
        p.removed = e->textRemoved();
    }
    if (pending.empty()) {
        QTimer::singleShot(0, QCoreApplication::instance(), &flush);   // not the editor: a closed window must not strand the queue
    }
    pendingEditor = editor;
    pending.push_back(std::move(p));
}

}   // namespace

void install()
{
    QAccessible::installFactory(&factory);
    previousHandler = QAccessible::installUpdateHandler(&translate);
}

}   // namespace ManuscriptAccessibility
