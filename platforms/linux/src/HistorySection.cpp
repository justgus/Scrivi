#include "HistorySection.hpp"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSettings>
#include <QToolButton>
#include <QVBoxLayout>

#include "AsyncCall.hpp"
#include "HistoryCapture.hpp"
#include "ScriviBridge.hpp"
#include "ThemeColours.hpp"

namespace {

// Apple's `@AppStorage("historyCardSceneOnly")`: per device, not per project — a viewing preference.
const QString kSceneOnlyKey = QStringLiteral("historyCardSceneOnly");
constexpr int kEventIDRole = Qt::UserRole;
constexpr int kOnSpineRole = Qt::UserRole + 1;
constexpr int kParentRole = Qt::UserRole + 2;
constexpr int kSceneRole = Qt::UserRole + 3;
constexpr int kOffsetRole = Qt::UserRole + 4;
constexpr int kSpanRole = Qt::UserRole + 5;

struct Node {
    QString eventID, parentID, kind, sceneID, preview, barrierKind, barrierNote, whitespaceKind;
    int childCount = 0, changeOffset = 0, changeLength = 0, removedLength = 0;
    bool onSpine = false, isCurrent = false;

    explicit Node(const QVariantMap& m)
        : eventID(m.value(QStringLiteral("eventID")).toString()),
          parentID(m.value(QStringLiteral("parentID")).toString()),   // ⚠️ absent on the root (I-0094)
          kind(m.value(QStringLiteral("kind"), QStringLiteral("typing")).toString()),
          sceneID(m.value(QStringLiteral("sceneID")).toString()),
          preview(m.value(QStringLiteral("preview")).toString()),
          barrierKind(m.value(QStringLiteral("barrierKind")).toString()),
          barrierNote(m.value(QStringLiteral("barrierNote")).toString()),
          whitespaceKind(m.value(QStringLiteral("whitespaceKind")).toString()),
          childCount(int(m.value(QStringLiteral("childIDs")).toList().size())),   // ⚠️ absent on a leaf
          changeOffset(m.value(QStringLiteral("changeOffsetUtf8")).toInt()),
          changeLength(m.value(QStringLiteral("changeLength")).toInt()),
          removedLength(m.value(QStringLiteral("removedLength")).toInt()),
          onSpine(m.value(QStringLiteral("onPrimarySpine")).toBool()),
          isCurrent(m.value(QStringLiteral("isCurrent")).toBool())
    {
    }
    // T-0398: a cut removes from the scene too.
    bool isDeletion() const
    {
        return (changeLength == 0 && removedLength > 0) || kind == QStringLiteral("delete") || kind == QStringLiteral("cut");
    }
};

// T-0397: an all-whitespace change is NAMED (Apple's `whitespaceLabel`).
QString whitespaceLabel(const QString& kind)
{
    const QStringList parts = kind.split(QLatin1Char(':'));
    const int count = parts.size() == 2 ? parts.at(1).toInt() : 0;
    if (count <= 0) {
        return {};
    }
    if (parts.at(0) == QStringLiteral("newline")) {
        return count == 1 ? HistorySection::tr("⏎ new paragraph") : HistorySection::tr("⏎ %1 new paragraphs").arg(count);
    }
    if (parts.at(0) == QStringLiteral("tab")) {
        return count == 1 ? HistorySection::tr("⇥ tab") : HistorySection::tr("⇥ %1 tabs").arg(count);
    }
    if (parts.at(0) == QStringLiteral("space")) {
        return count == 1 ? HistorySection::tr("␣ space") : HistorySection::tr("␣ %1 spaces").arg(count);
    }
    return {};
}

// Apple's `HistoryNodeRow.label`.
QString label(const Node& n)
{
    if (!n.barrierKind.isEmpty()) {
        return n.barrierNote.isEmpty() ? n.barrierKind : n.barrierNote;
    }
    const QString ws = whitespaceLabel(n.whitespaceKind);
    if (!ws.isEmpty()) {
        return n.isDeletion() ? HistorySection::tr("Deleted %1").arg(ws) : ws;
    }
    const QString preview = n.preview.trimmed();
    if (!preview.isEmpty()) {
        return n.isDeletion() ? HistorySection::tr("Deleted %1").arg(preview) : preview;
    }
    if (n.kind == QStringLiteral("barrier")) {
        return HistorySection::tr("(structural change)");
    }
    if (n.isDeletion() && n.removedLength > 0) {
        return n.removedLength == 1 ? HistorySection::tr("Deleted 1 byte")
                                    : HistorySection::tr("Deleted %1 bytes").arg(n.removedLength);
    }
    return HistorySection::tr("(no text)");
}

// Apple's glyphs, in characters every Linux UI font carries: current ◉ · barrier ⊝ · deletion ⊖ · on the line ● · branch ○.
QString glyph(const Node& n)
{
    if (n.isCurrent) {
        return QStringLiteral("◉");
    }
    if (n.kind == QStringLiteral("barrier") || n.kind == QStringLiteral("structural")) {
        return QStringLiteral("⊝");
    }
    if (n.isDeletion()) {
        return QStringLiteral("⊖");
    }
    return n.onSpine ? QStringLiteral("●") : QStringLiteral("○");
}

}   // namespace

HistorySection::HistorySection(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("historySection"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // ✅ Q1: collapsible, closed by default.
    header_ = new QToolButton(this);
    header_->setObjectName(QStringLiteral("historyHeader"));
    header_->setText(tr("History"));
    header_->setCheckable(true);
    header_->setChecked(false);
    header_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    header_->setArrowType(Qt::RightArrow);
    header_->setAutoRaise(true);
    QFont bold = header_->font();
    bold.setBold(true);
    header_->setFont(bold);
    layout->addWidget(header_);

    body_ = new QWidget(this);
    auto* bodyLayout = new QVBoxLayout(body_);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(3);

    auto* scope = new QHBoxLayout;
    sceneOnly_ = new QCheckBox(tr("This scene only"), body_);
    sceneOnly_->setObjectName(QStringLiteral("historySceneOnly"));
    sceneOnly_->setChecked(QSettings().value(kSceneOnlyKey, true).toBool());   // Apple's default: ON
    count_ = new QLabel(body_);
    count_->setEnabled(false);
    scope->addWidget(sceneOnly_);
    scope->addStretch();
    scope->addWidget(count_);
    bodyLayout->addLayout(scope);

    notice_ = new QLabel(body_);
    notice_->setWordWrap(true);
    notice_->setEnabled(false);
    bodyLayout->addWidget(notice_);

    list_ = new QListWidget(body_);
    list_->setObjectName(QStringLiteral("historyRows"));
    list_->setAccessibleName(tr("Undo history"));
    list_->setUniformItemSizes(true);
    list_->setMinimumHeight(160);
    bodyLayout->addWidget(list_, 1);

    truncated_ = new QLabel(body_);
    truncated_->setEnabled(false);
    bodyLayout->addWidget(truncated_);

    // [I-0108]: a BADGE — the count, stated as project-wide, and where to manage it ([SP-204] Q3).
    stale_ = new QLabel(body_);
    stale_->setObjectName(QStringLiteral("historyStale"));
    stale_->setWordWrap(true);
    stale_->setEnabled(false);
    bodyLayout->addWidget(stale_);

    body_->setVisible(false);
    layout->addWidget(body_, 1);

    connect(header_, &QToolButton::toggled, this, [this](bool open) {
        header_->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
        body_->setVisible(open);
        if (open && !loaded_) {
            reload();   // nothing is fetched while the section is closed
        }
    });
    connect(sceneOnly_, &QCheckBox::toggled, this, [this](bool on) {
        QSettings().setValue(kSceneOnlyKey, on);
        render();
    });
    connect(list_, &QListWidget::itemClicked, this, &HistorySection::select);
    connect(list_, &QListWidget::itemActivated, this, &HistorySection::select);
}

bool HistorySection::isExpanded() const
{
    return header_->isChecked();
}

void HistorySection::setHistory(HistoryCapture* history)
{
    if (history_ != nullptr) {
        disconnect(history_, nullptr, this, nullptr);
    }
    history_ = history;
    tree_.clear();
    staleCount_ = 0;
    loaded_ = false;
    if (history_ != nullptr) {
        connect(history_, &HistoryCapture::historyChanged, this, [this] {
            loaded_ = false;
            if (isExpanded()) {
                reload();
            }
        });
    }
    if (isExpanded()) {
        reload();
    } else {
        render();
    }
}

void HistorySection::setScene(const QString& sceneID)
{
    if (sceneID == sceneID_) {
        return;
    }
    sceneID_ = sceneID;
    if (isExpanded()) {
        render();   // [I-0331]: a re-filter, never a re-fetch
    }
}

void HistorySection::setCaret(const QString& sceneID, int caretByte)
{
    if (sceneID == caretScene_ && caretByte == caretByte_) {
        return;
    }
    caretScene_ = sceneID;
    caretByte_ = caretByte;
    if (isExpanded()) {
        markCaretRow();   // only the bold moves: a rebuild per keystroke would jump the list back to its top
    }
}

void HistorySection::markCaretRow()
{
    // I-0106: ONE row is "where the caret is" — the first match in the shown order (the most recent wins a shared boundary).
    bool found = false;
    for (int i = 0; i < list_->count(); ++i) {
        QListWidgetItem* item = list_->item(i);
        const int offset = item->data(kOffsetRole).toInt();
        const bool here = !found && caretByte_ >= 0 && caretScene_ == sceneID_
                          && item->data(kSceneRole).toString() == sceneID_
                          && caretByte_ >= offset && caretByte_ < offset + item->data(kSpanRole).toInt();
        found = found || here;
        if (item->font().bold() != here) {
            QFont f = item->font();
            f.setBold(here);
            item->setFont(f);
        }
    }
}

void HistorySection::reload()
{
    if (history_ == nullptr || !history_->isOpen()) {
        render();
        return;
    }
    loaded_ = true;
    const int generation = ++generation_;
    const QString root = history_->projectRootPath();
    struct Fetched {
        QVariantMap tree;
        int stale = 0;
        bool failed = false;
    };
    // ⛔ Off the GUI thread: the core's history lock can be held by a worker (a structural step, a Replace All).
    AsyncCall::run<Fetched>(
        this,
        [root]() {
            Fetched f;
            f.tree = ScriviBridge::historyGetTree(root);
            f.failed = f.tree.isEmpty();
            f.stale = int(ScriviBridge::historyListStaleBranches(root).value(QStringLiteral("branches")).toList().size());
            return f;
        },
        [this, generation](const Fetched& f) {
            if (generation != generation_) {
                return;   // a newer fetch is on its way
            }
            tree_ = f.tree;
            loadFailed_ = f.failed;
            staleCount_ = f.stale;
            render();
        },
        [this, generation] {
            if (generation == generation_) {
                loadFailed_ = true;
                loaded_ = false;
                render();
            }
        });
}

void HistorySection::render()
{
    list_->clear();
    truncated_->clear();
    stale_->clear();
    count_->clear();
    const auto showNotice = [this](const QString& text) {
        notice_->setText(text);
        notice_->setVisible(!text.isEmpty());
    };
    if (history_ == nullptr || !history_->isOpen()) {
        showNotice(tr("Undo history isn't open for this project."));
        return;
    }
    if (loadFailed_) {
        showNotice(tr("Couldn't read the undo history."));
        return;
    }
    const QVariantList all = tree_.value(QStringLiteral("nodes")).toList();
    if (all.isEmpty()) {
        showNotice(loaded_ && !tree_.isEmpty() ? tr("No history yet.") : QString());
        return;
    }

    // Apple's `orderedNodes`: the scope first (the root belongs to no scene), then the primary line newest first, then the
    // branches — so the line the writer is on reads top-down and abandoned branches sit below it.
    const bool sceneOnly = sceneOnly_->isChecked();
    QList<Node> spine;
    QList<Node> branches;
    for (const QVariant& v : all) {
        const Node n(v.toMap());
        if (sceneOnly && n.sceneID != sceneID_) {
            continue;
        }
        (n.onSpine ? spine : branches).append(n);
    }
    std::reverse(spine.begin(), spine.end());
    const QList<Node> shown = spine + branches;
    count_->setText(sceneOnly ? tr("%1 of %2").arg(shown.size()).arg(all.size()) : QString::number(all.size()));
    showNotice(shown.isEmpty() ? tr("No history for this scene yet.") : QString());

    const QColor secondary = ThemeColours::deemphasised(palette());
    QColor currentBackground = palette().color(QPalette::Highlight);
    currentBackground.setAlphaF(0.12);
    for (const Node& n : shown) {
        QString text = glyph(n) + QStringLiteral("  ") + label(n).simplified();
        if (n.childCount >= 2) {
            text += QStringLiteral("  ⋔");   // this point has more than one continuation
        }
        auto* item = new QListWidgetItem(text, list_);
        item->setData(kEventIDRole, n.eventID);
        item->setData(kSceneRole, n.sceneID);
        item->setData(kOffsetRole, n.changeOffset);
        item->setData(kSpanRole, std::max(n.changeLength, 1));   // I-0106: a deletion gets a one-byte span
        if (!n.onSpine) {
            item->setForeground(secondary);
            item->setToolTip(tr("Switch to this branch"));
        } else if (n.childCount >= 2) {
            item->setToolTip(tr("This point has more than one continuation"));
        }
        if (n.isCurrent) {
            item->setBackground(currentBackground);
        }
        item->setData(kOnSpineRole, n.onSpine);
        item->setData(kParentRole, n.parentID);
    }
    markCaretRow();
    if (tree_.value(QStringLiteral("truncated")).toBool()) {
        truncated_->setText(tr("Showing %1 of %2 events.")
                                .arg(all.size())
                                .arg(tree_.value(QStringLiteral("totalNodeCount")).toInt()));
    }
    if (staleCount_ > 0) {
        stale_->setText((staleCount_ == 1 ? tr("⚠ 1 abandoned branch") : tr("⚠ %1 abandoned branches").arg(staleCount_)) +
                        QLatin1Char('\n') + tr("Across the whole project — Project ▸ Purge Stale History Branches…"));
    }
}

void HistorySection::select(QListWidgetItem* item)
{
    // A node off the current line re-primaries its fork so redo walks it; one on the line moves nothing (Apple's `select`).
    if (item == nullptr || history_ == nullptr || item->data(kOnSpineRole).toBool()) {
        return;
    }
    const QString parentID = item->data(kParentRole).toString();
    if (!parentID.isEmpty()) {
        history_->selectBranch(parentID, item->data(kEventIDRole).toString());   // historyChanged → reload
    }
}
