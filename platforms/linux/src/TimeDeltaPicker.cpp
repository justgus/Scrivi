#include "TimeDeltaPicker.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

#include <array>
#include <cstdlib>
#include <limits>

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
                                 const QList<AnchorScene>& otherScenes,
                                 QWidget* parent)
    : QDialog(parent),
      previousSceneEndMs_(previousSceneEndMs),
      rawOffsetMs_(rawOffsetMs),
      currentDurationMs_(currentDurationMs),
      defaultDurationMs_(defaultDurationMs),
      referenceName_(referenceName),
      epochLabel_(epochLabel.isEmpty() ? tr("Story Open") : epochLabel),
      otherScenes_(otherScenes),
      hasPrevious_(!referenceName.isEmpty())
{
    setWindowTitle(tr("Set Time Delta"));
    setModal(true);

    auto* root = new QVBoxLayout(this);

    // Prompt: the delta is measured from the anchor chosen below (previous scene end or
    // the epoch). With no previous scene, the epoch is the only anchor.
    root->addWidget(new QLabel(tr("Place this scene in story-time:"), this));

    // --- Amount + unit + direction row -----------------------------------
    auto* deltaRow = new QGridLayout;
    amount_ = new QSpinBox(this);
    amount_->setRange(0, 1'000'000);
    unit_ = new QComboBox(this);
    for (const auto& u : kUnits) { unit_->addItem(tr(u.label)); }
    direction_ = new QComboBox(this);
    direction_->addItem(tr("after"));   // index 0 (Later)
    direction_->addItem(tr("before"));  // index 1 (Before) → may be a negative offset
    deltaRow->addWidget(amount_,    0, 0);
    deltaRow->addWidget(unit_,      0, 1);
    deltaRow->addWidget(direction_, 0, 2);

    // Anchor combo: what the delta is measured from. "before" the epoch (or a scene at
    // the epoch) yields a NEGATIVE offset — a flashback before Story Open. Items, in
    // order (anchorValues_ runs parallel): the previous scene's end (if any), the epoch
    // (0), then the END of every other scene — so the writer can say "immediately after
    // 'We go to the Lab'". Choosing a scene resolves to an absolute offset once; no
    // persisted anchor link.
    anchor_ = new QComboBox(this);
    // Don't let a long scene title stretch the dialog (a title can be a whole sentence).
    // Cap the width and elide entries to a character budget — like the Scene Navigator —
    // keeping the full title in each item's tooltip. The popup list scrolls if needed.
    anchor_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    anchor_->setMinimumContentsLength(24);
    anchor_->setMaximumWidth(260);
    anchor_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    anchorValues_.clear();
    // Add one combo item with `full` text (used for the tooltip) shown elided to a budget.
    const auto addAnchorItem = [this](const QString& full) {
        constexpr int kMaxChars = 40;   // matches the navigator's readable feel
        QString shown = full;
        if (shown.length() > kMaxChars) {
            shown = shown.left(kMaxChars - 1).trimmed() + QChar(0x2026);   // …
        }
        anchor_->addItem(shown);
        anchor_->setItemData(anchor_->count() - 1, full, Qt::ToolTipRole);
    };
    if (hasPrevious_) {
        addAnchorItem(tr("end of \"%1\" (previous)").arg(referenceName));
        anchorValues_.append(previousSceneEndMs_);
    }
    addAnchorItem(epochLabel_);
    anchorValues_.append(0);   // epoch = Story Open
    for (const AnchorScene& s : otherScenes_) {
        addAnchorItem(tr("end of \"%1\"").arg(
            s.title.isEmpty() ? tr("(untitled scene)") : s.title));
        anchorValues_.append(s.endMs);
    }
    deltaRow->addWidget(anchor_, 0, 3);
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
    // Seed the anchor to whichever choice the raw offset sits closest to, so re-opening
    // the picker shows a sensible "after X" rather than always the previous scene. A tie
    // prefers the earlier item (previous scene / epoch over a later scene).
    seedAnchorIndex_ = 0;
    qint64 best = std::numeric_limits<qint64>::max();
    for (int i = 0; i < anchorValues_.size(); ++i) {
        const qint64 d = std::llabs(rawOffsetMs_ - anchorValues_.at(i));
        if (d < best) {
            best = d;
            seedAnchorIndex_ = i;
        }
    }
    anchor_->setCurrentIndex(seedAnchorIndex_);
    const qint64 seedAnchor = anchorMs();

    // Direction + magnitude of the raw offset relative to the seeded anchor. A raw offset
    // earlier than the anchor → "before".
    const qint64 delta = std::llabs(rawOffsetMs_ - seedAnchor);
    direction_->setCurrentIndex(rawOffsetMs_ >= seedAnchor ? 0 : 1);
    const auto [unitIdx, amt] = bestFit(delta);
    unit_->setCurrentIndex(unitIdx);
    amount_->setValue(amt);

    const qint64 dur = currentDurationMs_ > 0 ? currentDurationMs_ : defaultDurationMs_;
    const auto [durUnitIdx, durAmt] = bestFit(dur);
    durUnit_->setCurrentIndex(durUnitIdx);
    durAmount_->setValue(durAmt);
}

qint64 TimeDeltaPicker::anchorMs() const
{
    const int i = anchor_->currentIndex();
    if (i >= 0 && i < anchorValues_.size()) {
        return anchorValues_.at(i);
    }
    return 0;   // epoch (Story Open) fallback
}

qint64 TimeDeltaPicker::spinnerOffsetMs() const
{
    const qint64 delta =
        static_cast<qint64>(amount_->value()) * kUnits[unit_->currentIndex()].ms;
    // "before" (index 1) subtracts — the result may be negative (before Story Open).
    return direction_->currentIndex() == 0   // 0 = after
        ? anchorMs() + delta
        : anchorMs() - delta;
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
    resultOffsetMs_   = spinnerOffsetMs();   // may be negative — a scene before Story Open
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
