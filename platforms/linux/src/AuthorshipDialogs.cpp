#include "AuthorshipDialogs.hpp"

#include <QComboBox>
#include <QCoreApplication>
#include <QHash>
#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QTabWidget>
#include <QVBoxLayout>

#include <algorithm>

#include "AppSupport.hpp"
#include "ScriviBridge.hpp"

namespace {

const char* kUnnamed = QT_TRANSLATE_NOOP("Authorship", "Unnamed author");

QString unnamedLabel() { return QCoreApplication::translate("Authorship", kUnnamed); }

QString authorLabel(const QVariantMap& a)
{
    return a.value(QStringLiteral("unnamed")).toBool() ? unnamedLabel() : a.value(QStringLiteral("displayName")).toString();
}

QStringList penNames(ScriviBridge& bridge)
{
    return bridge.getPenNames(scrivi::linux_app::appSupportRoot()).value(QStringLiteral("penNames")).toStringList();
}

QString defaultPenName(ScriviBridge& bridge)
{
    return bridge.getPenNames(scrivi::linux_app::appSupportRoot()).value(QStringLiteral("default")).toString();
}

QLabel* errorLabel(QWidget* parent)
{
    auto* l = new QLabel(parent);
    l->setStyleSheet(QStringLiteral("color: #c0392b;"));
    l->setWordWrap(true);
    l->hide();
    return l;
}

QPushButton* button(const QString& text, QWidget* parent) { return new QPushButton(text, parent); }

} // namespace

// MARK: — Settings

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("settingsDialog"));
    setWindowTitle(tr("Settings"));
    resize(460, 380);
    auto* layout = new QVBoxLayout(this);
    auto* tabs = new QTabWidget(this);
    tabs->addTab(new PenNamesPane(tabs), tr("Pen Names"));
    layout->addWidget(tabs);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    layout->addWidget(buttons);
}

PenNamesPane::PenNamesPane(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    list_ = new QListWidget(this);
    list_->setObjectName(QStringLiteral("penNamesList"));
    layout->addWidget(list_);

    auto* row = new QHBoxLayout;
    auto* makeDefault = button(tr("Make Default"), this);
    auto* up = button(tr("Move Up"), this);
    auto* down = button(tr("Move Down"), this);
    auto* remove = button(tr("Remove"), this);
    for (auto* b : {makeDefault, up, down, remove}) {
        row->addWidget(b);
    }
    row->addStretch();
    layout->addLayout(row);

    auto* addRow = new QHBoxLayout;
    newName_ = new QLineEdit(this);
    newName_->setPlaceholderText(tr("New pen name"));
    auto* add = button(tr("Add"), this);
    addRow->addWidget(newName_);
    addRow->addWidget(add);
    layout->addLayout(addRow);
    auto* hint = new QLabel(tr("The default (in bold) is offered for a new project, and when Scrivi asks who wrote a "
                               "project."), this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    layout->addWidget(hint);
    error_ = errorLabel(this);
    layout->addWidget(error_);

    auto names = [this]() {
        QStringList n;
        for (int i = 0; i < list_->count(); ++i) {
            n << list_->item(i)->data(Qt::UserRole).toString();
        }
        return n;
    };
    connect(add, &QPushButton::clicked, this, [this, names]() {
        const QString n = newName_->text().trimmed();
        if (n.isEmpty()) {
            return;
        }
        save(names() << n, defaultName_.isEmpty() ? n : defaultName_);
        if (error_->isHidden()) {
            newName_->clear();
        }
    });
    connect(newName_, &QLineEdit::returnPressed, add, &QPushButton::click);
    connect(makeDefault, &QPushButton::clicked, this, [this, names]() {
        if (auto* it = list_->currentItem()) {
            save(names(), it->data(Qt::UserRole).toString());
        }
    });
    auto move = [this, names](int offset) {
        const int i = list_->currentRow();
        QStringList n = names();
        if (i < 0 || i + offset < 0 || i + offset >= n.size()) {
            return;
        }
        n.swapItemsAt(i, i + offset);
        save(n, defaultName_);
        list_->setCurrentRow(i + offset);
    };
    connect(up, &QPushButton::clicked, this, [move]() { move(-1); });
    connect(down, &QPushButton::clicked, this, [move]() { move(1); });
    connect(remove, &QPushButton::clicked, this, [this, names]() {
        const int i = list_->currentRow();
        if (i < 0) {
            return;
        }
        QStringList n = names();
        const QString gone = n.takeAt(i);
        save(n, defaultName_ == gone ? n.value(0) : defaultName_);
    });
    connect(list_, &QListWidget::itemChanged, this, [this, names](QListWidgetItem* it) {
        const QString old = it->data(Qt::UserRole).toString();
        const QString now = it->text().trimmed();
        if (now == old) {
            return;
        }
        QStringList n = names();
        n[list_->row(it)] = now;
        save(n, defaultName_ == old ? now : defaultName_);
    });
    load();
}

void PenNamesPane::load()
{
    ScriviBridge bridge;
    const QVariantMap menu = bridge.getPenNames(scrivi::linux_app::appSupportRoot());
    defaultName_ = menu.value(QStringLiteral("default")).toString();
    const QSignalBlocker block(list_);
    list_->clear();
    for (const QString& n : menu.value(QStringLiteral("penNames")).toStringList()) {
        auto* it = new QListWidgetItem(n, list_);
        it->setData(Qt::UserRole, n);
        if (n == defaultName_) {
            QFont bold = it->font();
            bold.setBold(true);
            it->setFont(bold);
            it->setToolTip(tr("Default"));
        }
        it->setFlags(it->flags() | Qt::ItemIsEditable);
    }
}

void PenNamesPane::save(QStringList names, QString defaultName)
{
    ScriviBridge bridge;
    QString message;
    connect(&bridge, &ScriviBridge::errorOccurred, this, [&message](int, const QString& m) { message = m; });
    bridge.putPenNames(scrivi::linux_app::appSupportRoot(), names, defaultName);
    error_->setText(message);
    error_->setVisible(!message.isEmpty());
    const int row = list_->currentRow();
    load();
    list_->setCurrentRow(row);
}

// MARK: — Authors

AuthorsBox::AuthorsBox(ScriviBridge* bridge, QString rootPath, QString projectRootPath, QWidget* parent)
    : QGroupBox(tr("Authors"), parent), bridge_(bridge), root_(std::move(rootPath)), projectRoot_(std::move(projectRootPath))
{
    setObjectName(QStringLiteral("authorsBox"));
    auto* layout = new QVBoxLayout(this);
    list_ = new QListWidget(this);
    list_->setObjectName(QStringLiteral("authorsList"));
    list_->setMaximumHeight(120);
    layout->addWidget(list_);

    auto* row = new QHBoxLayout;
    auto* up = button(tr("Move Up"), this);
    auto* down = button(tr("Move Down"), this);
    auto* remove = button(tr("Remove"), this);
    for (auto* b : {up, down, remove}) {
        row->addWidget(b);
    }
    row->addStretch();
    layout->addLayout(row);

    auto* addRow = new QHBoxLayout;
    add_ = new QComboBox(this);
    add_->setEditable(true);
    add_->lineEdit()->setPlaceholderText(tr("Add an author"));
    auto* addButton = button(tr("Add"), this);
    addRow->addWidget(add_, 1);
    addRow->addWidget(addButton);
    layout->addLayout(addRow);

    auto* asRow = new QHBoxLayout;
    asRow->addWidget(new QLabel(tr("I write as"), this));
    writingAsBox_ = new QComboBox(this);
    writingAsBox_->setObjectName(QStringLiteral("writingAsBox"));
    asRow->addWidget(writingAsBox_, 1);
    layout->addLayout(asRow);
    auto* hint = new QLabel(tr("In byline order. Your edits here are recorded as the author you write as."), this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    layout->addWidget(hint);
    error_ = errorLabel(this);
    layout->addWidget(error_);

    connect(up, &QPushButton::clicked, this, [this]() { move(-1); });
    connect(down, &QPushButton::clicked, this, [this]() { move(1); });
    connect(remove, &QPushButton::clicked, this, [this]() {
        const int i = list_->currentRow();
        if (i < 0 || list_->count() == 1) {   // a work keeps at least one author (the core refuses none)
            return;
        }
        QVariantList e = edits();
        e.removeAt(i);
        write(e);
    });
    connect(addButton, &QPushButton::clicked, this, [this]() {
        const QString n = add_->currentText().trimmed();
        if (n.isEmpty()) {
            return;
        }
        add_->setCurrentText(QString());
        write(edits() << QVariantMap{{QStringLiteral("displayName"), n}});
    });
    connect(add_->lineEdit(), &QLineEdit::returnPressed, addButton, &QPushButton::click);
    connect(list_, &QListWidget::itemChanged, this, [this](QListWidgetItem*) {
        if (!loading_) {
            write(edits());
        }
    });
    connect(writingAsBox_, &QComboBox::activated, this, [this](int index) {
        const QString id = writingAsBox_->itemData(index).toString();
        if (id.isEmpty() || id == writingAs_) {
            return;
        }
        bridge_->setWritingAs(root_, id);
        load();
        for (const QVariant& a : authors_) {
            const QVariantMap m = a.toMap();
            if (m.value(QStringLiteral("personaID")).toString() == writingAs_) {
                emit writingAsChanged(writingAs_, m.value(QStringLiteral("displayName")).toString());
            }
        }
    });
    connect(bridge_, &ScriviBridge::errorOccurred, this, [this](int, const QString& m) {
        error_->setText(m);
        error_->show();
    });
    load();
}

void AuthorsBox::load()
{
    const QVariantMap view = bridge_->getAuthors(root_, projectRoot_);
    authors_ = view.value(QStringLiteral("authors")).toList();
    writingAs_ = view.value(QStringLiteral("writingAs")).toMap().value(QStringLiteral("personaID")).toString();

    loading_ = true;
    list_->clear();
    writingAsBox_->clear();
    if (writingAs_.isEmpty()) {
        writingAsBox_->addItem(QStringLiteral("—"), QString());
    }
    for (const QVariant& a : authors_) {
        const QVariantMap m = a.toMap();
        const bool unnamed = m.value(QStringLiteral("unnamed")).toBool();
        // An unnamed author's row says so; typing a name over it renames it (its ID kept).
        auto* it = new QListWidgetItem(m.value(QStringLiteral("displayName")).toString(), list_);
        it->setFlags(it->flags() | Qt::ItemIsEditable);
        if (unnamed) {
            it->setText(unnamedLabel());
            it->setForeground(palette().placeholderText());
            it->setData(Qt::UserRole, true);   // the shown text is a label, not a name
        }
        writingAsBox_->addItem(authorLabel(m), m.value(QStringLiteral("personaID")));
    }
    writingAsBox_->setCurrentIndex(std::max(0, writingAsBox_->findData(writingAs_)));
    loading_ = false;

    QStringList offered;
    for (const QString& n : penNames(*bridge_)) {
        bool listed = false;
        for (const QVariant& a : authors_) {
            listed = listed || a.toMap().value(QStringLiteral("displayName")).toString() == n;
        }
        if (!listed) {
            offered << n;
        }
    }
    const QString typed = add_->currentText();
    add_->clear();
    add_->addItems(offered);
    add_->setCurrentText(typed);
}

QVariantList AuthorsBox::edits() const
{
    QVariantList out;
    for (int i = 0; i < list_->count() && i < authors_.size(); ++i) {
        const QVariantMap a = authors_.at(i).toMap();
        const QListWidgetItem* it = list_->item(i);
        const bool stillUnnamed = it->data(Qt::UserRole).toBool() && it->text() == unnamedLabel();
        const QString typed = it->text().trimmed();
        // An unnamed author left as it is keeps its old name (it stays unnamed); an emptied name is not a rename.
        out << QVariantMap{{QStringLiteral("personaID"), a.value(QStringLiteral("personaID"))},
                           {QStringLiteral("displayName"), stillUnnamed || typed.isEmpty()
                                                               ? a.value(QStringLiteral("displayName")).toString() : typed}};
    }
    return out;
}

void AuthorsBox::write(const QVariantList& list)
{
    error_->hide();
    bridge_->setAuthors(root_, list);
    for (const QVariant& e : list) {
        const QVariantMap m = e.toMap();
        if (m.value(QStringLiteral("personaID")).toString().isEmpty()) {
            bridge_->rememberPenName(scrivi::linux_app::appSupportRoot(), m.value(QStringLiteral("displayName")).toString());
        }
    }
    load();
    for (const QVariant& a : authors_) {   // a rename reaches the window's author too
        const QVariantMap m = a.toMap();
        if (m.value(QStringLiteral("personaID")).toString() == writingAs_) {
            emit writingAsChanged(writingAs_, m.value(QStringLiteral("displayName")).toString());
        }
    }
}

void AuthorsBox::move(int offset)
{
    const int i = list_->currentRow();
    QVariantList e = edits();
    if (i < 0 || i + offset < 0 || i + offset >= e.size()) {
        return;
    }
    e.swapItemsAt(i, i + offset);
    write(e);
    list_->setCurrentRow(i + offset);
}

WorldAuthorsDialog::WorldAuthorsDialog(ScriviBridge* bridge, const QString& worldName, const QString& worldPath,
                                       const QString& projectRootPath, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("worldAuthorsDialog"));
    setWindowTitle(tr("Authors of “%1”").arg(worldName));
    resize(420, 340);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new AuthorsBox(bridge, worldPath, projectRootPath, this));
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    layout->addWidget(buttons);
}

// MARK: — The questions on open

AuthorQuestionDialog::AuthorQuestionDialog(ScriviBridge* bridge, QString projectRootPath, const QString& title,
                                           QWidget* parent)
    : QDialog(parent), bridge_(bridge), root_(std::move(projectRootPath)), title_(title.isEmpty() ? tr("this project") : title)
{
    setObjectName(QStringLiteral("authorQuestionDialog"));
    setWindowTitle(tr("Author"));
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    setMinimumWidth(400);
    auto* outer = new QVBoxLayout(this);
    body_ = new QVBoxLayout;
    outer->addLayout(body_);
    error_ = errorLabel(this);
    outer->addWidget(error_);
    connect(bridge_, &ScriviBridge::errorOccurred, this, [this](int, const QString& m) {
        error_->setText(m);
        error_->show();
    });
    view_ = bridge_->getAuthors(root_);
    rebuild();
}

QComboBox* AuthorQuestionDialog::nameField(const QString& text)
{
    auto* field = new QComboBox(this);
    field->setEditable(true);
    field->addItems(penNames(*bridge_));
    field->setCurrentText(text);
    return field;
}

void AuthorQuestionDialog::rebuild()
{
    while (QLayoutItem* item = body_->takeAt(0)) {
        if (item->widget() != nullptr) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    nameFields_.clear();
    const QString prompt = view_.value(QStringLiteral("authorPrompt")).toString();
    const QVariantList authors = view_.value(QStringLiteral("authors")).toList();
    auto* heading = new QLabel(this);
    QFont f = heading->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() + 3);
    heading->setFont(f);
    heading->setWordWrap(true);
    body_->addWidget(heading);
    auto* blurb = new QLabel(this);
    blurb->setWordWrap(true);
    body_->addWidget(blurb);

    if (prompt == QLatin1String("nameAuthor")) {
        heading->setText(tr("Who wrote “%1”?").arg(title_));
        blurb->setText(tr("Scrivi needs the author’s name — it goes in the byline when you export."));
        // [SP-210] Q6: one field per unnamed author, in byline order, the first offered the default pen name.
        QString first = defaultPenName(*bridge_);
        for (const QVariant& a : authors) {
            const QVariantMap m = a.toMap();
            if (m.value(QStringLiteral("unnamed")).toBool()) {
                nameFields_.append({m.value(QStringLiteral("personaID")).toString(), nameField(first)});
                first.clear();
            }
        }
        if (authors.isEmpty()) {
            nameFields_.append({QString(), nameField(first)});
        }
        for (int i = 0; i < nameFields_.size(); ++i) {
            auto* row = new QHBoxLayout;
            row->addWidget(new QLabel(nameFields_.size() > 1 ? tr("Author %1").arg(i + 1) : tr("Author"), this));
            row->addWidget(nameFields_[i].second, 1);
            body_->addLayout(row);
        }
        auto* go = new QPushButton(tr("Continue"), this);
        go->setObjectName(QStringLiteral("continueButton"));
        go->setDefault(true);
        connect(go, &QPushButton::clicked, this, &AuthorQuestionDialog::answerNames);
        auto* row = new QHBoxLayout;
        row->addStretch();
        row->addWidget(go);
        body_->addLayout(row);
    } else if (prompt == QLatin1String("chooseAuthor")) {
        heading->setText(tr("Which of these authors are you?"));
        blurb->setText(tr("Your edits to “%1” on this device are recorded as the author you choose.").arg(title_));
        for (const QVariant& a : authors) {
            const QVariantMap m = a.toMap();
            auto* b = new QPushButton(authorLabel(m), this);
            const QString id = m.value(QStringLiteral("personaID")).toString();
            connect(b, &QPushButton::clicked, this, [this, id]() { choose(id); });
            body_->addWidget(b);
        }
        auto* someone = new QPushButton(tr("Someone else…"), this);
        connect(someone, &QPushButton::clicked, this, [this, someone]() {
            someone->hide();
            nameFields_.append({QString(), nameField(defaultPenName(*bridge_))});
            auto* row = new QHBoxLayout;
            row->addWidget(new QLabel(tr("Your name"), this));
            row->addWidget(nameFields_.last().second, 1);
            body_->addLayout(row);
            auto* me = new QPushButton(tr("Add Me as an Author"), this);
            me->setDefault(true);
            connect(me, &QPushButton::clicked, this, &AuthorQuestionDialog::addSelf);
            body_->addWidget(me, 0, Qt::AlignRight);
        });
        body_->addWidget(someone);
    } else {
        accept();
    }
}

void AuthorQuestionDialog::answerNames()
{
    QHash<QString, QString> typed;
    for (const auto& [id, field] : nameFields_) {
        if (field->currentText().trimmed().isEmpty()) {
            return;   // every author needs a name
        }
        typed.insert(id, field->currentText().trimmed());
    }
    QVariantList list;
    for (const QVariant& a : view_.value(QStringLiteral("authors")).toList()) {
        const QVariantMap m = a.toMap();
        const QString id = m.value(QStringLiteral("personaID")).toString();
        list << QVariantMap{{QStringLiteral("personaID"), id},
                            {QStringLiteral("displayName"), typed.value(id, m.value(QStringLiteral("displayName")).toString())}};
    }
    if (list.isEmpty()) {
        list << QVariantMap{{QStringLiteral("displayName"), typed.value(QString())}};
    }
    bridge_->setAuthors(root_, list);
    if (bridge_->lastCallFailed()) {
        return;
    }
    for (const QString& n : typed) {
        bridge_->rememberPenName(scrivi::linux_app::appSupportRoot(), n);
    }
    QVariantMap view = bridge_->getAuthors(root_);
    const QVariantList authors = view.value(QStringLiteral("authors")).toList();
    if (view.value(QStringLiteral("writingAs")).toMap().isEmpty() && authors.size() == 1) {
        // A project with no authors at all: the name just given is this writer's.
        bridge_->setWritingAs(root_, authors.first().toMap().value(QStringLiteral("personaID")).toString());
        view = bridge_->getAuthors(root_);
    }
    finish(view);
}

void AuthorQuestionDialog::choose(const QString& personaID)
{
    bridge_->setWritingAs(root_, personaID);
    if (!bridge_->lastCallFailed()) {
        finish(bridge_->getAuthors(root_));
    }
}

void AuthorQuestionDialog::addSelf()
{
    const QString name = nameFields_.isEmpty() ? QString() : nameFields_.last().second->currentText().trimmed();
    if (name.isEmpty()) {
        return;
    }
    QSet<QString> before;
    QVariantList list;
    for (const QVariant& a : view_.value(QStringLiteral("authors")).toList()) {
        const QVariantMap m = a.toMap();
        before.insert(m.value(QStringLiteral("personaID")).toString());
        list << QVariantMap{{QStringLiteral("personaID"), m.value(QStringLiteral("personaID"))},
                            {QStringLiteral("displayName"), m.value(QStringLiteral("displayName"))}};
    }
    list << QVariantMap{{QStringLiteral("displayName"), name}};
    bridge_->setAuthors(root_, list);
    if (bridge_->lastCallFailed()) {
        return;
    }
    bridge_->rememberPenName(scrivi::linux_app::appSupportRoot(), name);
    for (const QVariant& a : bridge_->getAuthors(root_).value(QStringLiteral("authors")).toList()) {
        const QString id = a.toMap().value(QStringLiteral("personaID")).toString();
        if (!before.contains(id)) {
            choose(id);
            return;
        }
    }
}

// The answer leads to the next question, if any (named, but this identity is not yet one of them); else the window
// writes as the chosen author.
void AuthorQuestionDialog::finish(const QVariantMap& view)
{
    error_->hide();
    const QVariantMap w = view.value(QStringLiteral("writingAs")).toMap();
    if (!w.isEmpty()) {
        emit answered(w.value(QStringLiteral("personaID")).toString(), w.value(QStringLiteral("displayName")).toString());
    }
    view_ = view;
    rebuild();
}
