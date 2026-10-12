#include "BuffersPalette.hpp"

#include "HistoryCapture.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

QString actionName(BuffersPalette::Action a)
{
    switch (a) {
    case BuffersPalette::Action::Copy:  return QStringLiteral("copy");
    case BuffersPalette::Action::Paste: return QStringLiteral("paste");
    case BuffersPalette::Action::Cut:   return QStringLiteral("cut");
    }
    return {};
}

// Standard Cut / Copy / Paste iconography (Apple: `doc.on.doc` / `doc.on.clipboard` / `scissors`), with the word when the
// theme has no icon.
void dressButton(QToolButton* b, BuffersPalette::Action a)
{
    static const char* themes[] = {"edit-copy", "edit-paste", "edit-cut"};
    static const char* words[] = {QT_TRANSLATE_NOOP("BuffersPalette", "Copy"), QT_TRANSLATE_NOOP("BuffersPalette", "Paste"),
                                  QT_TRANSLATE_NOOP("BuffersPalette", "Cut")};
    const int i = static_cast<int>(a);
    const QIcon icon = QIcon::fromTheme(QString::fromLatin1(themes[i]));
    b->setIcon(icon);
    b->setText(QCoreApplication::translate("BuffersPalette", words[i]));
    b->setToolButtonStyle(icon.isNull() ? Qt::ToolButtonTextOnly : Qt::ToolButtonIconOnly);
}

}   // namespace

BuffersPalette::BuffersPalette(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus)
{
    setObjectName(QStringLiteral("buffersPalette"));
    setWindowTitle(tr("Buffers"));
    setAttribute(Qt::WA_ShowWithoutActivating);   // ✅ never takes the editor's focus (Apple: `becomesKeyOnlyIfNeeded`)
    setFocusPolicy(Qt::NoFocus);
    setMinimumSize(300, 320);

    auto* outer = new QVBoxLayout(this);
    auto* title = new QLabel(tr("Copy Buffers"), this);
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);
    outer->addWidget(title);
    // The hint names all three chords and how the button follows the modifiers (Apple's hint, Linux's keys).
    auto* hint = new QLabel(tr("Ctrl+1–9 copy · Ctrl+Shift+1–9 paste · Alt+1–9 cut · hold Ctrl+Shift or Alt to "
                               "change the button"), this);
    hint->setWordWrap(true);
    QFont small = hint->font();
    small.setPointSizeF(small.pointSizeF() * 0.85);
    hint->setFont(small);
    hint->setForegroundRole(QPalette::PlaceholderText);
    outer->addWidget(hint);

    auto* grid = new QGridLayout;
    grid->setColumnStretch(1, 1);
    int r = 0;
    for (const QString& id : BufferService::slotIDs()) {
        Row row;
        row.bufferID = id;
        row.number = new QLabel(id, this);
        QFont mono = row.number->font();
        mono.setBold(true);
        row.number->setFont(mono);
        row.preview = new QLabel(this);
        row.preview->setObjectName(QStringLiteral("bufferPreview%1").arg(id));
        row.preview->setTextFormat(Qt::PlainText);
        row.when = new QLabel(this);
        row.when->setFont(small);
        row.when->setForegroundRole(QPalette::PlaceholderText);
        auto* texts = new QVBoxLayout;
        texts->setSpacing(0);
        texts->addWidget(row.preview);
        texts->addWidget(row.when);
        row.action = new QToolButton(this);
        row.action->setObjectName(QStringLiteral("bufferAction%1").arg(id));
        row.action->setFocusPolicy(Qt::NoFocus);
        row.action->setAutoRaise(true);
        row.clear = new QToolButton(this);
        row.clear->setObjectName(QStringLiteral("bufferClear%1").arg(id));
        row.clear->setFocusPolicy(Qt::NoFocus);
        row.clear->setAutoRaise(true);
        row.clear->setText(QStringLiteral("✕"));
        grid->addWidget(row.number, r, 0);
        grid->addLayout(texts, r, 1);
        grid->addWidget(row.action, r, 2);
        grid->addWidget(row.clear, r, 3);
        ++r;

        connect(row.action, &QToolButton::clicked, this, [this, id]() {
            // ✅ The modifiers held AT THE CLICK decide — read from the press itself (`eventFilter`), as the chord would.
            const Action a = actionFor(clickMods_);
            if (a == Action::Paste && (service_ == nullptr || service_->text(id).isEmpty())) {
                return;   // nothing to paste (Apple renders it inert)
            }
            emit actionRequested(a, id);
        });
        connect(row.clear, &QToolButton::clicked, this, [this, id]() {
            if (service_ != nullptr && !service_->text(id).isEmpty()) {
                emit clearRequested(id);
            }
        });
        rows_.append(row);
    }
    outer->addLayout(grid);
    outer->addStretch(1);

    // The held modifiers, from every key event in the app — the palette itself never has focus.
    qApp->installEventFilter(this);
    showAction(Action::Copy);
    rebuild();
}

BuffersPalette::~BuffersPalette()
{
    qApp->removeEventFilter(this);
}

BuffersPalette::Action BuffersPalette::actionFor(Qt::KeyboardModifiers mods)
{
    mods &= Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier;
    if (mods == (Qt::ControlModifier | Qt::ShiftModifier)) {
        return Action::Paste;
    }
    if (mods == Qt::AltModifier) {
        return Action::Cut;
    }
    return Action::Copy;   // none, Ctrl alone, Shift alone… — the default, matching Ctrl+N
}

void BuffersPalette::setService(BufferService* service)
{
    if (service_ == service) {
        rebuild();
        return;
    }
    disconnect(changedConnection_);
    disconnect(destroyedConnection_);
    service_ = service;
    if (service != nullptr) {
        changedConnection_ = connect(service, &BufferService::changed, this, &BuffersPalette::rebuild);
        // ⚠️ A project's service dies with its release. By `destroyed` it is no longer a BufferService, so it is let go
        // of explicitly — never read — before the rows are redrawn empty.
        destroyedConnection_ = connect(service, &QObject::destroyed, this, [this]() {
            service_ = nullptr;
            rebuild();
        });
    }
    rebuild();
}

void BuffersPalette::hideQuietly()
{
    closingProgrammatically_ = true;
    hide();
    closingProgrammatically_ = false;
}

void BuffersPalette::rebuild()
{
    for (Row& row : rows_) {
        const QString preview = service_ != nullptr ? service_->preview(row.bufferID) : QString();
        const bool empty = service_ == nullptr || service_->text(row.bufferID).isEmpty();
        row.preview->setText(empty ? tr("empty") : preview);
        row.preview->setForegroundRole(empty ? QPalette::PlaceholderText : QPalette::WindowText);
        const QString when = empty ? QString() : HistoryCapture::friendlyTimestamp(service_->updatedAt(row.bufferID));
        row.when->setText(when);
        row.when->setVisible(!when.isEmpty());
        row.clear->setEnabled(!empty);
        row.clear->setToolTip(empty ? tr("Buffer %1 is empty").arg(row.bufferID) : tr("Clear buffer %1").arg(row.bufferID));
        updateButton(row);
    }
}

void BuffersPalette::showAction(Action action)
{
    shown_ = action;
    for (Row& row : rows_) {
        updateButton(row);
    }
}

void BuffersPalette::updateButton(Row& row) const
{
    dressButton(row.action, shown_);
    row.action->setProperty("bufferAction", actionName(shown_));
    const bool empty = service_ == nullptr || service_->text(row.bufferID).isEmpty();
    const QString& id = row.bufferID;
    switch (shown_) {
    case Action::Copy:
        row.action->setToolTip(tr("Copy the selected manuscript text into buffer %1 (Ctrl+%1)").arg(id));
        break;
    case Action::Paste:
        row.action->setToolTip(empty ? tr("Buffer %1 is empty — nothing to paste").arg(id)
                                     : tr("Paste buffer %1 at the cursor, replacing any selection (Ctrl+Shift+%1)").arg(id));
        break;
    case Action::Cut:
        row.action->setToolTip(tr("Cut the selected manuscript text into buffer %1 (Alt+%1)").arg(id));
        break;
    }
    row.action->setEnabled(service_ != nullptr && !(shown_ == Action::Paste && empty));
}

bool BuffersPalette::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        for (const Row& row : rows_) {
            if (watched == row.action) {
                clickMods_ = static_cast<QMouseEvent*>(event)->modifiers();
                break;
            }
        }
    }
    if (isVisible() && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)) {
        const auto* key = static_cast<QKeyEvent*>(event);
        Qt::KeyboardModifiers mods = key->modifiers();
        // ⚠️ A modifier key's OWN event does not yet (press) / still (release) carry its flag — X11 and Qt agree on that.
        Qt::KeyboardModifier own = Qt::NoModifier;
        switch (key->key()) {
        case Qt::Key_Control: own = Qt::ControlModifier; break;
        case Qt::Key_Shift:   own = Qt::ShiftModifier; break;
        case Qt::Key_Alt:     own = Qt::AltModifier; break;
        case Qt::Key_Meta:    own = Qt::MetaModifier; break;
        default: break;
        }
        if (own != Qt::NoModifier) {
            mods = event->type() == QEvent::KeyPress ? (mods | own) : (mods & ~own);
        }
        const Action a = actionFor(mods);
        if (a != shown_) {
            showAction(a);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void BuffersPalette::showEvent(QShowEvent* event)
{
    showAction(actionFor(QGuiApplication::queryKeyboardModifiers()));
    QWidget::showEvent(event);
}

void BuffersPalette::closeEvent(QCloseEvent* event)
{
    if (!closingProgrammatically_) {
        emit closedByWriter();
    }
    QWidget::closeEvent(event);
}
