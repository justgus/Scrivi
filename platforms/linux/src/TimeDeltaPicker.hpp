#pragma once

#include <QDialog>
#include <QString>

class QSpinBox;
class QComboBox;

// TimeDeltaPicker — the Linux Time Delta Picker dialog (EP-025 / SP-080, T-0327).
//
// The Qt mirror of Apple's TimeDeltaPicker (TimelineStripView.swift). A modal dialog
// that lets a writer position a scene in story-time relative to the previous scene's
// END:
//   • an amount spinbox + unit combo (Minutes…Years) + direction (Later / Before);
//   • a scene-duration row (amount + unit);
//   • "Immediately after previous" (reset the scene to the default gap chain).
//
// It computes NOTHING against the backend — exec() returns a result the caller
// (EditorShell, T-0328) turns into scrivi_set_scene_story_time calls + chain
// propagation. Seeded from the dragged/current offset so the spinner opens on the
// writer's just-placed position (Apple's initialiseSpinner + bestFit).
class TimeDeltaPicker : public QDialog
{
    Q_OBJECT

public:
    // How the dialog was dismissed.
    enum class Outcome {
        Cancelled,   // no change
        SetOffset,   // manual placement at resultOffsetMs(), duration resultDurationMs()
        ResetDefault // return the scene to the default gap chain (duration = default)
    };

    // `referenceName` is the previous scene's title (for the prompt; empty → "story
    // open"). `rawOffsetMs` is the scene's proposed absolute offset (from the drag or
    // its current position); `previousSceneEndMs` is the anchor the delta is measured
    // from; `currentDurationMs` seeds the duration row; `defaultDurationMs` is used on
    // reset; `epochLabel` names the origin.
    TimeDeltaPicker(const QString& referenceName,
                    qint64 rawOffsetMs,
                    qint64 previousSceneEndMs,
                    qint64 currentDurationMs,
                    qint64 defaultDurationMs,
                    const QString& epochLabel,
                    QWidget* parent = nullptr);

    Outcome outcome() const { return outcome_; }
    // Absolute story-time offset chosen (valid when outcome() == SetOffset).
    qint64  resultOffsetMs() const { return resultOffsetMs_; }
    // Scene duration chosen (valid for SetOffset; = defaultDurationMs on ResetDefault).
    qint64  resultDurationMs() const { return resultDurationMs_; }

private:
    // Spinner → absolute offset: previousSceneEnd ± (amount × unit).
    qint64 spinnerOffsetMs() const;
    qint64 chosenDurationMs() const;
    // Seed amount/unit/direction + duration from the raw offset (Apple bestFit).
    void   initialiseFromRaw();

    // "Set" / "Immediately after" handlers set outcome_ + the results, then accept().
    void   commitSetOffset();
    void   commitReset();

    const qint64 previousSceneEndMs_;
    const qint64 rawOffsetMs_;
    const qint64 currentDurationMs_;
    const qint64 defaultDurationMs_;

    QSpinBox*  amount_    = nullptr;
    QComboBox* unit_      = nullptr;
    QComboBox* direction_ = nullptr;
    QSpinBox*  durAmount_ = nullptr;
    QComboBox* durUnit_   = nullptr;

    Outcome outcome_          = Outcome::Cancelled;
    qint64  resultOffsetMs_   = 0;
    qint64  resultDurationMs_ = 0;
};
