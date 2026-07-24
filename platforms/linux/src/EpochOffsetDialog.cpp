#include "EpochOffsetDialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <array>

namespace {
// Unit → milliseconds, matching TimeDeltaPicker.cpp's kUnits (Months/years use the
// same 30d/365d approximations so the two dialogs agree).
struct UnitDef { const char* label; qint64 ms; };
constexpr std::array<UnitDef, 6> kUnits = {{
    {"Minutes", 60'000LL},
    {"Hours",   3'600'000LL},
    {"Days",    86'400'000LL},
    {"Weeks",   604'800'000LL},
    {"Months",  2'592'000'000LL},
    {"Years",   31'536'000'000LL},
}};
} // namespace

EpochOffsetDialog::EpochOffsetDialog(const QString& sourceName,
                                     const QString& epochLabel,
                                     const QList<qint64>& eventOffsets,
                                     qint64 windowMinMs,
                                     qint64 windowMaxMs,
                                     QWidget* parent)
    : QDialog(parent)
    , eventOffsets_(eventOffsets)
    , windowMinMs_(windowMinMs)
    , windowMaxMs_(windowMaxMs)
{
    setWindowTitle(tr("Import Timeline"));
    setModal(true);

    auto* form = new QFormLayout;

    auto* nameLabel = new QLabel(
        sourceName.isEmpty() ? tr("(untitled timeline)") : sourceName, this);
    form->addRow(tr("Source"), nameLabel);

    auto* epochLbl = new QLabel(
        epochLabel.isEmpty() ? tr("(no epoch label)") : epochLabel, this);
    form->addRow(tr("Source epoch"), epochLbl);

    // Signed offset row: [amount] [unit ▾] [Later/Before ▾]. Default 0 → events land at
    // their own offsets relative to this project's epoch.
    auto* offsetRow = new QHBoxLayout;
    amount_ = new QSpinBox(this);
    amount_->setRange(0, 1'000'000);
    amount_->setValue(0);
    unit_ = new QComboBox(this);
    for (const auto& u : kUnits) {
        unit_->addItem(QString::fromLatin1(u.label));
    }
    unit_->setCurrentIndex(5);   // Years — the common cross-era import unit
    direction_ = new QComboBox(this);
    direction_->addItem(tr("Later"));
    direction_->addItem(tr("Before"));
    offsetRow->addWidget(amount_);
    offsetRow->addWidget(unit_);
    offsetRow->addWidget(direction_);
    form->addRow(tr("Epoch offset"), offsetRow);

    preview_ = new QLabel(this);
    preview_->setWordWrap(true);
    form->addRow(tr("Preview"), preview_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Import"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    // Live preview as the offset changes.
    connect(amount_, qOverload<int>(&QSpinBox::valueChanged),
            this, &EpochOffsetDialog::updatePreview);
    connect(unit_, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &EpochOffsetDialog::updatePreview);
    connect(direction_, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &EpochOffsetDialog::updatePreview);
    updatePreview();
}

qint64 EpochOffsetDialog::epochOffsetMs() const
{
    const qint64 magnitude =
        static_cast<qint64>(amount_->value()) * kUnits.at(unit_->currentIndex()).ms;
    return (direction_->currentIndex() == 1) ? -magnitude : magnitude;   // 1 = Before
}

void EpochOffsetDialog::updatePreview()
{
    const qint64 off = epochOffsetMs();
    int inside = 0;
    for (const qint64 e : eventOffsets_) {
        const qint64 projected = e + off;
        if (projected >= windowMinMs_ && projected <= windowMaxMs_) {
            ++inside;
        }
    }
    const int total = static_cast<int>(eventOffsets_.size());
    preview_->setText(tr("%1 of %2 event(s) fall within the current timeline window; "
                         "%3 will be clipped.")
                          .arg(inside)
                          .arg(total)
                          .arg(total - inside));
}
