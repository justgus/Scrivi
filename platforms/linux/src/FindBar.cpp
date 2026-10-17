#include "FindBar.hpp"
#include "DiagnosticLog.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QShortcut>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

#include "ManuscriptEditor.hpp"
#include "SceneDocument.hpp"

QList<QAction*> addFindMenu(QMenu* edit, QObject* context, std::function<void(FindCommand)> run)
{
    struct Item { const char* title; QList<QKeySequence> keys; FindCommand command; };
    const Item items[] = {
        {QT_TRANSLATE_NOOP("FindMenu", "Find…"), {QKeySequence(Qt::CTRL | Qt::Key_F)}, FindCommand::ShowFind},
        {QT_TRANSLATE_NOOP("FindMenu", "Find and Replace…"), {QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F)},
         FindCommand::ShowFindAndReplace},
        {QT_TRANSLATE_NOOP("FindMenu", "Find Next"), {QKeySequence(Qt::CTRL | Qt::Key_G), QKeySequence(Qt::Key_F3)},
         FindCommand::NextMatch},
        {QT_TRANSLATE_NOOP("FindMenu", "Find Previous"),
         {QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G), QKeySequence(Qt::SHIFT | Qt::Key_F3)}, FindCommand::PreviousMatch},
        {QT_TRANSLATE_NOOP("FindMenu", "Use Selection for Find"), {QKeySequence(Qt::CTRL | Qt::Key_E)},
         FindCommand::UseSelectionForFind},
    };
    QMenu* menu = edit->addMenu(QMenu::tr("&Find"));
    QList<QAction*> out;
    for (const Item& item : items) {
        QAction* a = menu->addAction(QCoreApplication::translate("FindMenu", item.title));
        a->setShortcuts(item.keys);
        const FindCommand command = item.command;
        QObject::connect(a, &QAction::triggered, context, [run, command]() { run(command); });
        out.append(a);
    }
    return out;
}

FindBar::FindBar(ManuscriptEditor* editor, QWidget* parent)
    : QWidget(parent), editor_(editor)
{
    setObjectName(QStringLiteral("findBar"));
    query_ = new QLineEdit(this);
    query_->setObjectName(QStringLiteral("findQuery"));
    query_->setPlaceholderText(tr("Find"));
    query_->setClearButtonEnabled(true);
    query_->installEventFilter(this);
    count_ = new QLabel(this);
    count_->setObjectName(QStringLiteral("findCount"));
    auto* previous = new QToolButton(this);
    previous->setArrowType(Qt::LeftArrow);
    previous->setToolTip(tr("Find Previous"));
    auto* next = new QToolButton(this);
    next->setArrowType(Qt::RightArrow);
    next->setToolTip(tr("Find Next"));
    mode_ = new QComboBox(this);
    mode_->addItem(tr("Contains"));
    mode_->addItem(tr("Starts With"));
    mode_->addItem(tr("Whole Word"));
    ignoreCase_ = new QCheckBox(tr("Ignore Case"), this);
    ignoreCase_->setChecked(true);   // [SP-164] Q6: on by default
    auto* done = new QPushButton(tr("Done"), this);

    // SP-181: the Replace row (Q1), under the find row.
    replaceRow_ = new QWidget(this);
    replaceField_ = new QLineEdit(replaceRow_);
    replaceField_->setObjectName(QStringLiteral("replaceField"));
    replaceField_->setPlaceholderText(tr("Replace"));
    replaceField_->setClearButtonEnabled(true);
    auto* replaceOne = new QPushButton(tr("Replace"), replaceRow_);
    auto* replaceAllButton = new QPushButton(tr("Replace All"), replaceRow_);
    auto* replaceLine = new QHBoxLayout(replaceRow_);
    replaceLine->setContentsMargins(0, 0, 0, 0);
    replaceLine->addWidget(replaceField_, 1);
    replaceLine->addWidget(replaceOne);
    replaceLine->addWidget(replaceAllButton);
    replaceRow_->hide();
    connect(replaceField_, &QLineEdit::returnPressed, this, &FindBar::replace);
    connect(replaceOne, &QPushButton::clicked, this, &FindBar::replace);
    connect(replaceAllButton, &QPushButton::clicked, this, &FindBar::replaceAll);

    auto* rows = new QVBoxLayout(this);
    rows->setContentsMargins(6, 4, 6, 4);
    rows->setSpacing(4);
    auto* row = new QHBoxLayout;
    rows->addLayout(row);
    rows->addWidget(replaceRow_);
    row->addWidget(query_, 1);
    row->addWidget(count_);
    row->addWidget(previous);
    row->addWidget(next);
    row->addWidget(mode_);
    row->addWidget(ignoreCase_);
    row->addWidget(done);

    refresh_ = new QTimer(this);
    refresh_->setSingleShot(true);
    refresh_->setInterval(150);   // an edit while the bar is open: refresh the count and highlights once typing pauses
    connect(refresh_, &QTimer::timeout, this, [this]() {
        updateHighlights();
        updateCount();
    });

    connect(query_, &QLineEdit::textEdited, this, &FindBar::queryChanged);
    connect(query_, &QLineEdit::returnPressed, this, [this]() { step(true); });
    connect(previous, &QToolButton::clicked, this, [this]() { step(false); });
    connect(next, &QToolButton::clicked, this, [this]() { step(true); });
    connect(mode_, &QComboBox::currentIndexChanged, this, &FindBar::queryChanged);
    connect(ignoreCase_, &QCheckBox::toggled, this, &FindBar::queryChanged);
    connect(done, &QPushButton::clicked, this, &FindBar::closeBar);
    auto* escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escape->setContext(Qt::WidgetWithChildrenShortcut);
    connect(escape, &QShortcut::activated, this, &FindBar::closeBar);
    // Highlights follow the view: only the matches on screen are drawn.
    // ⚠️ Both skip while an edit has made the matches stale — the refresh timer catches up once typing pauses, so a
    // keystroke never rebuilds the presented manuscript.
    connect(editor_->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        if (isVisible() && !matchesStale_) {
            updateHighlights();
        }
    });
    connect(editor_, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        if (isVisible() && !matchesStale_) {
            updateCount();
        }
    });
    hide();
}

void FindBar::attach(SceneDocument* doc, ManuscriptPresenter* presenter)
{
    doc_ = doc;
    presenter_ = presenter;
    presented_ = {};
    presentedStale_ = true;
    matchesStale_ = true;
    if (doc_ != nullptr) {
        connect(doc_->document(), &QTextDocument::contentsChange, this, &FindBar::onContentsChange, Qt::UniqueConnection);
    }
}

void FindBar::run(FindCommand command)
{
    switch (command) {
    case FindCommand::ShowFind: showFind(false); break;
    case FindCommand::ShowFindAndReplace: showFind(true); break;
    case FindCommand::NextMatch: step(true); break;
    case FindCommand::PreviousMatch: step(false); break;
    case FindCommand::UseSelectionForFind: useSelection(); break;
    }
}

void FindBar::showFind(bool withReplace)
{
    replaceRow_->setVisible(withReplace);
    note_.clear();
    show();
    query_->setFocus();
    query_->selectAll();
    updateHighlights();
    updateCount();
}

void FindBar::closeBar()
{
    note_.clear();
    hide();
    editor_->setExtraSelections({});
    editor_->setFocus();
}

const ManuscriptFind::PresentedText& FindBar::presented()
{
    if (presentedStale_ && doc_ != nullptr) {
        const DiagnosticLog::Span span("find: refresh presented text");
        QElapsedTimer t;
        t.start();
        const int rebuilt = presented_.refresh(*doc_, presenter_);
        presentedStale_ = false;
        if (rebuilt > 0) {
            ++builds_;
            lastRebuilt_ = rebuilt;
            qInfo("[SCRIVI-FIND] presented text: %d scene(s) rebuilt, %d units, %lld ms", rebuilt, presented_.length(),
                  t.elapsed());
        }
    }
    return presented_;
}

ManuscriptFind::Options FindBar::options() const
{
    ManuscriptFind::Options o;
    o.mode = ManuscriptFind::Mode(mode_->currentIndex());
    o.ignoreCase = ignoreCase_->isChecked();
    return o;
}

const QVector<MarkdownRange>& FindBar::matches()
{
    if (matchesStale_) {
        matches_ = presented().find(query_->text(), options());
        matchesStale_ = false;
    }
    return matches_;
}

int FindBar::currentIndex() const
{
    const QTextCursor c = editor_->textCursor();
    const MarkdownRange sel{c.selectionStart(), c.selectionEnd()};
    if (!matchesStale_ && selectedIndex_ >= 0 && selectedIndex_ < matches_.size() && sel == selectedAs_) {
        return selectedIndex_;
    }
    const auto it = std::lower_bound(matches_.begin(), matches_.end(), sel.start,
                                     [](const MarkdownRange& m, int s) { return m.start < s; });
    return (!matchesStale_ && it != matches_.end() && *it == sel) ? int(it - matches_.begin()) : -1;
}

void FindBar::select(int index)
{
    const MarkdownRange m = matches_.at(index);
    QTextCursor c = editor_->textCursor();
    c.setPosition(m.start);
    c.setPosition(m.end, QTextCursor::KeepAnchor);
    editor_->setTextCursor(c);
    editor_->ensureCursorVisible();
    const QTextCursor held = editor_->textCursor();
    selectedIndex_ = index;
    selectedAs_ = {held.selectionStart(), held.selectionEnd()};
}

void FindBar::step(bool forward)
{
    if (query_->text().isEmpty()) {
        showFind(replaceRow_->isVisibleTo(this));
        return;
    }
    note_.clear();
    const auto& all = matches();
    if (all.isEmpty()) {
        QApplication::beep();
        updateCount();
        return;
    }
    const QTextCursor c = editor_->textCursor();
    const int current = currentIndex();
    int index = -1;
    if (current >= 0) {
        const int n = int(all.size());
        index = forward ? (current + 1) % n : (current + n - 1) % n;   // wraps
    } else if (forward) {
        const auto it = std::lower_bound(all.begin(), all.end(), c.selectionEnd(),
                                         [](const MarkdownRange& m, int e) { return m.start < e; });
        index = it == all.end() ? 0 : int(it - all.begin());   // wraps
    } else {
        const auto it = std::lower_bound(all.begin(), all.end(), c.selectionStart(),
                                         [](const MarkdownRange& m, int s) { return m.end <= s; });
        index = it == all.begin() ? int(all.size()) - 1 : int(it - all.begin()) - 1;   // wraps
    }
    select(index);
    updateHighlights();
    updateCount();
}

void FindBar::queryChanged()
{
    const DiagnosticLog::Span span("find: query changed");
    // Incremental search (AppKit's incremental find): the first match at or after the selection's start.
    matchesStale_ = true;
    selectedIndex_ = -1;
    note_.clear();
    const auto& all = matches();
    if (!all.isEmpty()) {
        const int from = editor_->textCursor().selectionStart();
        const auto it = std::lower_bound(all.begin(), all.end(), from,
                                         [](const MarkdownRange& m, int s) { return m.start < s; });
        select(it == all.end() ? 0 : int(it - all.begin()));
    }
    updateHighlights();
    updateCount();
}

void FindBar::useSelection()
{
    const QTextCursor c = editor_->textCursor();
    if (!c.hasSelection()) {
        QApplication::beep();
        return;
    }
    const QString text = presented().presentedOf(c.selectionStart(), c.selectionEnd());
    if (text.isEmpty()) {
        QApplication::beep();
        return;
    }
    query_->setText(text);
    matchesStale_ = true;
    show();
    matches();
    updateHighlights();
    updateCount();
}

void FindBar::onContentsChange(int position, int removed, int added)
{
    if (removed == 0 && added == 0) {
        return;   // a format-only change (the presenter's highlighting)
    }
    // ⚠️ Connected AFTER EditorShell's handler (attach() runs after the shell's connect in load()), so the scene map is
    // already reconciled. A structural change that this misattributes is caught by refresh()'s id/length check.
    if (doc_ != nullptr) {
        presented_.markEdited(*doc_, position);
    }
    presentedStale_ = true;
    selectedIndex_ = -1;
    matchesStale_ = true;
    if (isVisible()) {
        refresh_->start();
    }
}

QString FindBar::countText() const { return count_->text(); }

// ✅ SP-181 Q2 (ruled 2026-10-15: "find first, then replace"): Replace changes the CURRENT match only — typed over it as a
// keystroke would be (Apple: `replaceFound` → `insertText`) — then finds the next. With no current match it only finds.
void FindBar::replace()
{
    if (query_->text().isEmpty()) {
        showFind(true);
        return;
    }
    matches();
    if (currentIndex() >= 0 && !editor_->typeOver(replaceField_->text())) {
        QApplication::beep();   // refused (the editor is locked, or the match touches a boundary)
        return;
    }
    step(true);
}

// ✅ SP-181 Q3: every match, as ONE undo step — the shell's work, off the GUI thread.
void FindBar::replaceAll()
{
    if (query_->text().isEmpty() || editor_->isReadOnly() || !replaceAll_) {
        QApplication::beep();
        return;
    }
    const QVector<MarkdownRange> all = matches();
    if (all.isEmpty()) {
        QApplication::beep();
        updateCount();
        return;
    }
    replaceAll_(all, replaceField_->text(), [this](int replaced) {
        refresh_->stop();
        note_ = replaced == 1 ? tr("1 replaced") : tr("%1 replaced").arg(replaced);
        updateHighlights();
        updateCount();
    });
}

void FindBar::updateCount()
{
    const DiagnosticLog::Span span("find: count");
    if (query_->text().isEmpty()) {
        count_->clear();
        return;
    }
    if (!note_.isEmpty()) {
        count_->setText(note_);
        return;
    }
    const int total = int(matches().size());
    if (total == 0) {
        count_->setText(tr("Not found"));
        return;
    }
    const int current = currentIndex();
    count_->setText(current >= 0 ? tr("%1 of %2").arg(current + 1).arg(total)
                                 : (total == 1 ? tr("1 match") : tr("%1 matches").arg(total)));
}

void FindBar::updateHighlights()
{
    const DiagnosticLog::Span span("find: highlights");
    if (!isVisible() || query_->text().isEmpty()) {
        editor_->setExtraSelections({});
        return;
    }
    const auto& all = matches();
    // Only what is on screen: a common word can match tens of thousands of times in a novel.
    const int first = editor_->cursorForPosition(QPoint(0, 0)).position();
    const QWidget* vp = editor_->viewport();
    const int last = editor_->cursorForPosition(QPoint(vp->width(), vp->height())).position();
    QColor colour = editor_->palette().color(QPalette::Highlight);
    colour.setAlpha(80);
    QList<QTextEdit::ExtraSelection> out;
    auto it = std::lower_bound(all.begin(), all.end(), first, [](const MarkdownRange& m, int s) { return m.end <= s; });
    for (; it != all.end() && it->start <= last; ++it) {
        QTextEdit::ExtraSelection x;
        x.cursor = QTextCursor(editor_->document());
        x.cursor.setPosition(it->start);
        x.cursor.setPosition(it->end, QTextCursor::KeepAnchor);
        x.format.setBackground(colour);
        out.append(x);
    }
    editor_->setExtraSelections(out);
}

bool FindBar::eventFilter(QObject* watched, QEvent* event)
{
    // Shift+Return in the query: Find Previous (Return is Find Next).
    if (watched == query_ && event->type() == QEvent::KeyPress) {
        const auto* k = static_cast<QKeyEvent*>(event);
        if ((k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter) && (k->modifiers() & Qt::ShiftModifier)) {
            step(false);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
