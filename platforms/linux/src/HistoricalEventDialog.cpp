#include "HistoricalEventDialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

HistoricalEventDialog::HistoricalEventDialog(const QString& windowTitle,
                                             const QString& title,
                                             const QString& description,
                                             const QStringList& tags,
                                             QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(windowTitle);
    setModal(true);

    auto* form = new QFormLayout;

    titleEdit_ = new QLineEdit(title, this);
    titleEdit_->setPlaceholderText(tr("e.g. The Battle of the Iron Gate"));
    form->addRow(tr("Title"), titleEdit_);

    descriptionEdit_ = new QPlainTextEdit(description, this);
    descriptionEdit_->setPlaceholderText(tr("Optional description…"));
    descriptionEdit_->setFixedHeight(80);
    form->addRow(tr("Description"), descriptionEdit_);

    tagsEdit_ = new QLineEdit(tags.join(QStringLiteral(", ")), this);
    tagsEdit_->setPlaceholderText(tr("comma, separated, tags"));
    form->addRow(tr("Tags"), tagsEdit_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // A historical event must have a title (the C ABI required field, §6.5): gate OK on
    // a non-empty title so we never send a blank-title create/update.
    QPushButton* okButton = buttons->button(QDialogButtonBox::Ok);
    auto updateOk = [this, okButton] {
        okButton->setEnabled(!titleEdit_->text().trimmed().isEmpty());
    };
    connect(titleEdit_, &QLineEdit::textChanged, this, updateOk);
    updateOk();

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    titleEdit_->setFocus();
}

QString HistoricalEventDialog::title() const
{
    return titleEdit_->text().trimmed();
}

QString HistoricalEventDialog::description() const
{
    return descriptionEdit_->toPlainText();
}

QStringList HistoricalEventDialog::tags() const
{
    QStringList out;
    const QStringList raw = tagsEdit_->text().split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString& t : raw) {
        const QString trimmed = t.trimmed();
        if (!trimmed.isEmpty()) {
            out.append(trimmed);
        }
    }
    return out;
}
