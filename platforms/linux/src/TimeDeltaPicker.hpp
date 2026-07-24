#pragma once

#include <QDialog>
#include <QList>
#include <QString>

class QSpinBox;
class QComboBox;

// TimeDeltaPicker — the Linux Time Delta Picker dialog (EP-025 / SP-080, T-0327).
//
// The Qt mirror of Apple's TimeDeltaPicker (TimelineStripView.swift). A modal dialog
// that lets a writer position a scene in story-time relative to an ANCHOR:
//   • an amount spinbox + unit combo (Minutes…Years) + direction (after / before);
//   • an "anchor" combo choosing what the delta is measured from — the previous scene's
//     end, the story epoch (Story Open), or the END of ANY OTHER scene in the manuscript
//     (a convenience so "Scene 14, immediately after 'We go to the Lab'" is one click);
//   • a scene-duration row (amount + unit);
//   • "Immediately after previous" (reset the scene to the default gap chain).
//
// Anchoring to another scene is resolved ONCE to an absolute offset (that scene's
// end ± delta) and stored as a plain "manual" offset — there is NO persisted
// "anchored-to" link (user decision 2026-07-23: choosing a scene is only a data-entry
// convenience; the stored model is unchanged). If the anchor scene later moves, this
// scene does not auto-follow.
//
// Direction "before" + an anchor at/near the epoch yields a NEGATIVE absolute offset:
// scenes legitimately occur before Story Open (flashbacks, I-0088). The result is NOT
// floored at 0 — the backend stores signed story-time, and the shell/panel render
// negatives.
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

    // One selectable anchor scene: its title (for the combo) and its END offset
    // (offset + duration) in story-time. The edited scene is NOT included by the caller.
    struct AnchorScene {
        QString sceneID;
        QString title;
        qint64  endMs = 0;
    };

    // `referenceName` is the previous scene's title (for the prompt/anchor combo; empty
    // → no previous scene, so the previous-scene anchor is omitted). `rawOffsetMs` is
    // the scene's proposed absolute offset (from the drag or its current position);
    // `previousSceneEndMs` is the previous-scene anchor; `currentDurationMs` seeds the
    // duration row; `defaultDurationMs` is used on reset; `epochLabel` names the origin
    // (the Story Open anchor sits at absolute offset 0). `otherScenes` are every OTHER
    // scene the writer may anchor to (by end offset); empty → only previous/epoch anchors.
    TimeDeltaPicker(const QString& referenceName,
                    qint64 rawOffsetMs,
                    qint64 previousSceneEndMs,
                    qint64 currentDurationMs,
                    qint64 defaultDurationMs,
                    const QString& epochLabel,
                    const QList<AnchorScene>& otherScenes,
                    QWidget* parent = nullptr);

    Outcome outcome() const { return outcome_; }
    // Absolute story-time offset chosen (valid when outcome() == SetOffset).
    qint64  resultOffsetMs() const { return resultOffsetMs_; }
    // Scene duration chosen (valid for SetOffset; = defaultDurationMs on ResetDefault).
    qint64  resultDurationMs() const { return resultDurationMs_; }

private:
    // The offset the delta is measured from, per the current anchor combo selection.
    // Looked up from anchorValues_ (parallel to the combo's items).
    qint64 anchorMs() const;
    // Spinner → absolute offset: anchorMs() ± (amount × unit). May be negative.
    qint64 spinnerOffsetMs() const;
    qint64 chosenDurationMs() const;
    // Seed amount/unit/direction + duration from the raw offset (Apple bestFit).
    void   initialiseFromRaw();

    // "Set" / "Immediately after" handlers set outcome_ + the results, then accept().
    void   commitSetOffset();
    void   commitReset();

    const qint64             previousSceneEndMs_;
    const qint64             rawOffsetMs_;
    const qint64             currentDurationMs_;
    const qint64             defaultDurationMs_;
    const QString            referenceName_;
    const QString            epochLabel_;
    const QList<AnchorScene> otherScenes_;

    // True when there is a previous scene (referenceName non-empty).
    const bool hasPrevious_;

    // Anchor offset (ms) for each item in anchor_, in the same order the items are added:
    // [previous-scene end?] , [epoch = 0] , [each other scene's end…]. anchorMs() indexes
    // this by anchor_->currentIndex().
    QList<qint64> anchorValues_;
    // The combo index that best matches rawOffsetMs_ (seeded in the ctor).
    int           seedAnchorIndex_ = 0;

    QSpinBox*  amount_    = nullptr;
    QComboBox* unit_      = nullptr;
    QComboBox* direction_ = nullptr;
    QComboBox* anchor_    = nullptr;   // items: [prev?] [Story Open] [other scenes…]
    QSpinBox*  durAmount_ = nullptr;
    QComboBox* durUnit_   = nullptr;

    Outcome outcome_          = Outcome::Cancelled;
    qint64  resultOffsetMs_   = 0;
    qint64  resultDurationMs_ = 0;
};
