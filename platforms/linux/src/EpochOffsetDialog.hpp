#pragma once

#include <QDialog>
#include <QList>
#include <QString>

class QSpinBox;
class QComboBox;
class QLabel;

// EpochOffsetDialog — set the epoch offset when importing an external timeline
// (EP-025 / SP-082, T-0342). The Qt mirror of Apple's EpochOffsetDialog
// (TimelineStripView.swift), shown after the writer picks a .scrivi-timeline.json and
// before it is stored (§4.11 / FR-066–FR-067). It presents:
//   • the source timeline name + epoch label (read-only, from the picked file);
//   • a signed amount spinbox + unit combo (Minutes…Years) → an epochOffsetMs that is
//     ADDED to every event's own offset to place it in this project's story time;
//   • a live preview of how many events fall INSIDE vs OUTSIDE the current project
//     window [windowMinMs, windowMaxMs] at the chosen offset (only in-window events
//     render, §6.7 / FR-061).
//
// It computes nothing against the backend: exec() returns Accepted with
// epochOffsetMs(), which the caller (EditorShell) passes to importExternalTimeline.
class EpochOffsetDialog : public QDialog
{
    Q_OBJECT

public:
    // `eventOffsets` are the source events' own offsetMs (pre-offset), used to compute
    // the in/out-of-window preview. `windowMinMs`/`windowMaxMs` bound the project's
    // current story-time window.
    EpochOffsetDialog(const QString& sourceName,
                      const QString& epochLabel,
                      const QList<qint64>& eventOffsets,
                      qint64 windowMinMs,
                      qint64 windowMaxMs,
                      QWidget* parent = nullptr);

    // The chosen offset in ms (signed): amount × unit × direction.
    qint64 epochOffsetMs() const;

private:
    void updatePreview();

    QSpinBox* amount_    = nullptr;
    QComboBox* unit_     = nullptr;
    QComboBox* direction_ = nullptr;   // Later (+) / Before (−)
    QLabel*   preview_   = nullptr;

    QList<qint64> eventOffsets_;
    qint64        windowMinMs_ = 0;
    qint64        windowMaxMs_ = 1;
};
