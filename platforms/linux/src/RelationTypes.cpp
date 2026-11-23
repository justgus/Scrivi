#include "RelationTypes.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

#include "AsyncCall.hpp"
#include "ObjectKindScope.hpp"
#include "ScriviBridge.hpp"

namespace {

const QString kScene = QStringLiteral("scene");   // the core's sentinel for a scene end (§5.2)

QString capitalised(const QString& s) { return s.isEmpty() ? s : s.left(1).toUpper() + s.mid(1); }

QString kindName(const QString& kind) { return kind.isEmpty() ? QObject::tr("Any kind") : capitalised(kind); }

QLabel* redLabel(QWidget* parent)
{
    auto* l = new QLabel(parent);
    l->setStyleSheet(QStringLiteral("color: #c0392b;"));
    l->setWordWrap(true);
    l->hide();
    return l;
}

QLabel* hintLabel(QWidget* parent)
{
    auto* l = new QLabel(parent);
    l->setWordWrap(true);
    l->setEnabled(false);
    return l;
}

} // namespace

// ── The form ─────────────────────────────────────────────────────────────────────────────────────────────────────────

RelationTypeDialog::RelationTypeDialog(ScriviBridge* bridge, QString projectRootPath, QVariantMap editing,
                                       const QString& initialSourceKind, QWidget* parent)
    : QDialog(parent), bridge_(bridge), root_(std::move(projectRootPath)), editing_(std::move(editing))
{
    setObjectName(QStringLiteral("relationTypeDialog"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(editing_.isEmpty() ? tr("New Relationship Type") : tr("Edit Relationship Type"));
    const bool locked = editing_.value(QStringLiteral("inUse")).toBool();

    auto* col = new QVBoxLayout(this);
    auto* labels = new QFormLayout;
    forward_ = new QLineEdit(this);
    forward_->setPlaceholderText(tr("mentor of"));
    symmetric_ = new QCheckBox(tr("Same both ways"), this);
    symmetric_->setEnabled(!locked);
    inverse_ = new QLineEdit(this);
    inverse_->setPlaceholderText(tr("mentored by"));
    inverseLabel_ = new QLabel(tr("Reads from the other end"), this);
    labels->addRow(tr("Reads from this end"), forward_);
    labels->addRow(QString(), symmetric_);
    labels->addRow(inverseLabel_, inverse_);
    col->addLayout(labels);
    labelsFooter_ = hintLabel(this);
    col->addWidget(labelsFooter_);

    auto* kinds = new QFormLayout;
    from_ = new QComboBox(this);
    to_ = new QComboBox(this);
    for (QComboBox* box : {from_, to_}) {
        box->addItem(tr("Any kind"), QString());
        box->addItem(tr("Scene"), kScene);
        // Derived from the core, never restated (CLAUDE.md's standing rule).
        for (const ObjectKindScope::KindInfo& k : ObjectKindScope::kinds(bridge_)) { box->addItem(capitalised(k.kind), k.kind); }
        box->setEnabled(!locked);
    }
    fromLabel_ = new QLabel(this);
    toLabel_ = new QLabel(tr("To"), this);
    kinds->addRow(fromLabel_, from_);
    kinds->addRow(toLabel_, to_);
    col->addLayout(kinds);
    if (locked) {
        QLabel* why = hintLabel(this);
        why->setObjectName(QStringLiteral("relationTypeLocked"));
        why->setText(tr("Relationships use this type, so its kinds and Same both ways can't change. Its labels can."));
        col->addWidget(why);
    }
    error_ = redLabel(this);
    col->addWidget(error_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    save_ = buttons->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    save_->setDefault(true);
    col->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(save_, &QPushButton::clicked, this, &RelationTypeDialog::save);

    if (editing_.isEmpty()) {
        from_->setCurrentIndex(std::max(0, from_->findData(initialSourceKind)));
    } else {
        forward_->setText(editing_.value(QStringLiteral("forwardLabel")).toString());
        inverse_->setText(editing_.value(QStringLiteral("inverseLabel")).toString());
        symmetric_->setChecked(editing_.value(QStringLiteral("symmetric")).toBool());
        from_->setCurrentIndex(std::max(0, from_->findData(editing_.value(QStringLiteral("sourceKind")).toString())));
        to_->setCurrentIndex(std::max(0, to_->findData(editing_.value(QStringLiteral("targetKind")).toString())));
    }
    connect(forward_, &QLineEdit::textChanged, this, &RelationTypeDialog::refresh);
    connect(inverse_, &QLineEdit::textChanged, this, &RelationTypeDialog::refresh);
    connect(symmetric_, &QCheckBox::toggled, this, &RelationTypeDialog::refresh);
    refresh();
    resize(420, sizeHint().height());
}

void RelationTypeDialog::refresh()
{
    const bool sym = symmetric_->isChecked();
    inverseLabel_->setVisible(!sym);
    inverse_->setVisible(!sym);
    toLabel_->setVisible(!sym);
    to_->setVisible(!sym);
    fromLabel_->setText(sym ? tr("Between") : tr("From"));
    labelsFooter_->setText(sym ? tr("Both ends read “%1”, as “sibling of” does.").arg(forward_->text().trimmed())
                               : tr("Each end of a relationship reads its own label."));
    save_->setEnabled(!saving_ && !forward_->text().trimmed().isEmpty() && (sym || !inverse_->text().trimmed().isEmpty()));
}

void RelationTypeDialog::save()
{
    const bool sym = symmetric_->isChecked();
    const QString forward = forward_->text().trimmed();
    const QString source = from_->currentData().toString();
    // A symmetric type relates one kind to the same kind: its ends are sorted, not directed (§5.3).
    const QString target = sym ? source : to_->currentData().toString();
    QJsonObject type{{QStringLiteral("code"), editing_.value(QStringLiteral("code")).toString()},
                     {QStringLiteral("forwardLabel"), forward},
                     {QStringLiteral("inverseLabel"), sym ? forward : inverse_->text().trimmed()},
                     {QStringLiteral("symmetric"), sym},
                     {QStringLiteral("canonicalDirection"), sym ? QStringLiteral("lexical") : QStringLiteral("source-to-target")}};
    if (!source.isEmpty()) { type.insert(QStringLiteral("sourceKind"), source); }   // absent = any kind
    if (!target.isEmpty()) { type.insert(QStringLiteral("targetKind"), target); }
    const QString json = QString::fromUtf8(QJsonDocument(type).toJson(QJsonDocument::Compact));

    struct Saved {
        QString code, error;
    };
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_;
    saving_ = true;
    refresh();
    error_->hide();
    // ⛔ Not on the GUI thread: the project may be on a share.
    AsyncCall::run<Saved>(
        this,
        [handle, root, json]() -> Saved {
            WorkerBridge bridge(handle);
            QString message;
            QObject::connect(&*bridge, &ScriviBridge::errorOccurred, [&message](int, const QString& m) { message = m; });
            const QVariantMap r = bridge->upsertRelationType(root, json);
            if (bridge->lastCallFailed()) {
                return {QString(), message.isEmpty() ? QObject::tr("The relationship type could not be saved.") : message};
            }
            return {r.value(QStringLiteral("code")).toString(), QString()};
        },
        [this](const Saved& s) {
            saving_ = false;
            if (!s.error.isEmpty()) {
                error_->setText(s.error);
                error_->show();
                refresh();
                return;
            }
            savedCode_ = s.code;
            accept();
        },
        [this] {
            saving_ = false;
            error_->setText(tr("Saving is taking longer than expected."));
            error_->show();
            refresh();
        });
}

// ── Project Settings ▸ Relationship Types ────────────────────────────────────────────────────────────────────────────

RelationTypesBox::RelationTypesBox(ScriviBridge* bridge, QString projectRootPath, QWidget* parent)
    : QGroupBox(tr("Relationship Types"), parent), bridge_(bridge), root_(std::move(projectRootPath))
{
    setObjectName(QStringLiteral("relationTypesBox"));
    auto* col = new QVBoxLayout(this);
    rows_ = new QVBoxLayout;
    col->addLayout(rows_);
    error_ = redLabel(this);
    col->addWidget(error_);
    auto* add = new QPushButton(tr("New Relationship Type…"), this);
    add->setObjectName(QStringLiteral("relationTypeNew"));
    auto* addRow = new QHBoxLayout;
    addRow->addWidget(add);
    addRow->addStretch();
    col->addLayout(addRow);
    QLabel* hint = hintLabel(this);
    hint->setText(tr("A relationship reads one label from each end, such as “mentor of” and “mentored by”."));
    col->addWidget(hint);
    connect(add, &QPushButton::clicked, this, [this] { showForm({}); });
    load();
}

void RelationTypesBox::load()
{
    struct Listed {
        QVariantList types;
        bool failed = false;
    };
    const WorkerBridge::Handle handle = WorkerBridge::handleFor(bridge_);
    const QString root = root_;
    AsyncCall::run<Listed>(
        this,
        [handle, root]() -> Listed {
            WorkerBridge bridge(handle);
            Listed l;
            l.types = bridge->listRelationTypes(root).value(QStringLiteral("types")).toList();
            l.failed = bridge->lastCallFailed();
            return l;
        },
        [this](const Listed& l) {
            while (QLayoutItem* item = rows_->takeAt(0)) {
                delete item->widget();
                delete item;
            }
            error_->setVisible(l.failed);
            error_->setText(tr("This project's relationship types could not be read."));
            for (const QVariant& v : l.types) {
                const QVariantMap t = v.toMap();
                const bool sym = t.value(QStringLiteral("symmetric")).toBool();
                const QString forward = t.value(QStringLiteral("forwardLabel")).toString();
                const QString source = t.value(QStringLiteral("sourceKind")).toString();
                auto* row = new QWidget(this);
                row->setObjectName(QStringLiteral("relationTypeRow"));
                row->setProperty("code", t.value(QStringLiteral("code")));
                auto* line = new QHBoxLayout(row);
                line->setContentsMargins(0, 2, 0, 2);
                auto* text = new QVBoxLayout;
                auto* title = new QLabel(sym ? forward : tr("%1 · %2").arg(forward, t.value(QStringLiteral("inverseLabel")).toString()), row);
                title->setObjectName(QStringLiteral("relationTypeLabels"));
                QLabel* ends = hintLabel(row);
                ends->setObjectName(QStringLiteral("relationTypeEnds"));
                ends->setText(sym ? tr("Between %1").arg(kindName(source))
                                  : tr("%1 → %2").arg(kindName(source), kindName(t.value(QStringLiteral("targetKind")).toString())));
                text->addWidget(title);
                text->addWidget(ends);
                line->addLayout(text, 1);
                if (t.value(QStringLiteral("builtIn")).toBool()) {
                    // [SP-238] Q3: words, not a lock — Qt has no standard lock icon and themes differ.
                    QLabel* builtIn = hintLabel(row);
                    builtIn->setObjectName(QStringLiteral("relationTypeBuiltIn"));
                    builtIn->setText(tr("Built in"));
                    builtIn->setToolTip(tr("Built in: every project has it, and it can't be changed."));
                    line->addWidget(builtIn);
                } else {
                    auto* edit = new QPushButton(tr("Edit…"), row);
                    edit->setObjectName(QStringLiteral("relationTypeEdit"));
                    connect(edit, &QPushButton::clicked, this, [this, t] { showForm(t); });
                    line->addWidget(edit);
                }
                rows_->addWidget(row);
            }
        },
        [this] {
            error_->setText(tr("This project's relationship types are taking longer than expected to read."));
            error_->show();
        });
}

void RelationTypesBox::showForm(const QVariantMap& type)
{
    form_ = new RelationTypeDialog(bridge_, root_, type, QString(), this);
    connect(form_, &QDialog::finished, this, [this](int result) {
        form_ = nullptr;
        if (result == QDialog::Accepted) { load(); }
    });
    form_->open();
}
