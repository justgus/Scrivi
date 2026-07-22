#include "TimeDeltaPicker.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <array>
#include <cstdlib>

namespace {
// Unit → milliseconds, matching Apple's TimeDeltaPicker.DeltaUnit. Months/years use
// Apple's fixed approximations (30d / 365d) so the two platforms agree.
struct UnitDef { const char* label; qint64 ms; };
constexpr std::array<UnitDef, 6> kUnits = {{
    {"Minutes", 60'000LL},
    {"Hours",   3'600'000LL},
    {"Days",    86'400'000LL},
    {"Weeks",   604'800'000LL},
    {"Months",  2'592'000'000LL},
    {"Years",   31'536'000'000LL},
}};

// Largest unit that divides `ms` to a whole ≥ 1 (Apple's bestFit): returns the unit
// index + the amount in that unit.
std::pair<int, int> bestFit(qint64 ms)
{
    for (int i = static_cast<int>(kUnits.size()) - 1; i >= 1; --i) {
        if (ms >= kUnits[i].ms) {
            return {i, static_cast<int>(std::max<qint64>(1, ms / kUnits[i].ms))};
        }
    }
    return {0, static_cast<int>(std::max<qint64>(1, ms / kUnits[0].ms))};
}
} // namespace

TimeDeltaPicker::TimeDeltaPicker(const QString& referenceName,
                                 qint64 rawOffsetMs,
                                 qint64 previousSceneEndMs,
                                 qint64 currentDurationMs,
                                 qint64 defaultDurationMs,
                                 const QString& epochLabel,
                                 QWidget* parent)
    : QDialog(parent),
      previousSceneEndMs_(previousSceneEndMs),
      rawOffsetMs_(rawOffsetMs),
      currentDurationMs_(currentDurationMs),
      defaultDurationMs_(defaultDurationMs)
{
    setWindowTitle(tr("Set Time Delta"));
    setModal(true);

    auto* root = new QVBoxLayout(this);

    // Prompt: time measured from the previous scene's end (or the epoch).
    const QString prompt = referenceName.isEmpty()
        ? tr("Time from %1:").arg(epochLabel.isEmpty() ? tr("story open") : epochLabel)
        : tr("Time after \"%1\" ends:").arg(referenceName);
    root->addWidget(new QLabel(prompt, this));

    // --- Amount + unit + direction row -----------------------------------
    auto* deltaRow = new QGridLayout;
    amount_ = new QSpinBox(this);
    amount_->setRange(0, 1'000'000);
    unit_ = new QComboBox(this);
    for (const auto& u : kUnits) { unit_->addItem(tr(u.label)); }
    direction_ = new QComboBox(this);
    direction_->addItem(tr("Later"));   // index 0
    direction_->addItem(tr("Before"));  // index 1
    deltaRow->addWidget(amount_,    0, 0);
    deltaRow->addWidget(unit_,      0, 1);
    deltaRow->addWidget(direction_, 0, 2);
    root->addLayout(deltaRow);

    // --- Scene-duration row ----------------------------------------------
    auto* durRow = new QGridLayout;
    durRow->addWidget(new QLabel(tr("Scene duration:"), this), 0, 0);
    durAmount_ = new QSpinBox(this);
    durAmount_->setRange(1, 1'000'000);
    durUnit_ = new QComboBox(this);
    for (const auto& u : kUnits) { durUnit_->addItem(tr(u.label)); }
    durRow->addWidget(durAmount_, 0, 1);
    durRow->addWidget(durUnit_,   0, 2);
    root->addLayout(durRow);

    // --- Buttons ----------------------------------------------------------
    // "Immediately after previous" resets to the default gap chain; the standard
    // button box's OK commits the manual placement, Cancel dismisses unchanged.
    auto* resetBtn = new QPushButton(tr("Immediately after previous"), this);
    resetBtn->setToolTip(tr("Return this scene to the default timeline chain"));
    connect(resetBtn, &QPushButton::clicked, this, &TimeDeltaPicker::commitReset);
    root->addWidget(resetBtn);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Set"));
    connect(buttons, &QDialogButtonBox::accepted, this, &TimeDeltaPicker::commitSetOffset);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);

    initialiseFromRaw();
}

void TimeDeltaPicker::initialiseFromRaw()
{
    // Direction + magnitude of the raw offset relative to the previous scene's end.
    const qint64 delta = std::llabs(rawOffsetMs_ - previousSceneEndMs_);
    direction_->setCurrentIndex(rawOffsetMs_ >= previousSceneEndMs_ ? 0 : 1);
    const auto [unitIdx, amt] = bestFit(delta);
    unit_->setCurrentIndex(unitIdx);
    amount_->setValue(amt);

    const qint64 dur = currentDurationMs_ > 0 ? currentDurationMs_ : defaultDurationMs_;
    const auto [durUnitIdx, durAmt] = bestFit(dur);
    durUnit_->setCurrentIndex(durUnitIdx);
    durAmount_->setValue(durAmt);
}

qint64 TimeDeltaPicker::spinnerOffsetMs() const
{
    const qint64 delta =
        static_cast<qint64>(amount_->value()) * kUnits[unit_->currentIndex()].ms;
    return direction_->currentIndex() == 0   // 0 = Later
        ? previousSceneEndMs_ + delta
        : previousSceneEndMs_ - delta;
}

qint64 TimeDeltaPicker::chosenDurationMs() const
{
    const qint64 dur =
        static_cast<qint64>(durAmount_->value()) * kUnits[durUnit_->currentIndex()].ms;
    return std::max<qint64>(1, dur);
}

void TimeDeltaPicker::commitSetOffset()
{
    outcome_          = Outcome::SetOffset;
    resultOffsetMs_   = std::max<qint64>(0, spinnerOffsetMs());   // never before the epoch
    resultDurationMs_ = chosenDurationMs();
    accept();
}

void TimeDeltaPicker::commitReset()
{
    outcome_          = Outcome::ResetDefault;
    resultOffsetMs_   = 0;   // recomputed by the chain; not used on reset
    resultDurationMs_ = defaultDurationMs_ > 0 ? defaultDurationMs_ : 3'600'000LL;
    accept();
}
