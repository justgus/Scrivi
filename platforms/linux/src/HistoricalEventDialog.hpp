#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>

class QLineEdit;
class QPlainTextEdit;

// HistoricalEventDialog — author/edit a project historical event (EP-025 / SP-082,
// T-0341). The Qt mirror of the fields Apple's "New Historical Event" flow collects
// (TimelineStripView.swift): a title (required), an optional description, and optional
// comma-separated tags. The story-time OFFSET is NOT edited here — it comes from the
// drag / click position (New Historical Event Here) or the event's current offset
// (Edit), and is preserved by the caller (EditorShell). This keeps the dialog purely
// about the worldbuilding content, matching the design's §7.7 "Edit Historical Event…".
//
// It talks to no backend: exec() returns Accepted with title()/description()/tags(),
// and the caller turns that into scrivi_create_historical_event /
// scrivi_update_historical_event. OK is disabled while the title is blank (a historical
// event must have a title — the C ABI's required field, §6.5).
class HistoricalEventDialog : public QDialog
{
    Q_OBJECT

public:
    // `windowTitle` distinguishes New vs Edit ("New Historical Event" / "Edit Historical
    // Event"). The three seed args prefill for Edit (all empty for New).
    explicit HistoricalEventDialog(const QString& windowTitle,
                                   const QString& title,
                                   const QString& description,
                                   const QStringList& tags,
                                   QWidget* parent = nullptr);

    QString     title() const;
    QString     description() const;
    QStringList tags() const;   // split + trimmed from the comma-separated field

private:
    QLineEdit*      titleEdit_       = nullptr;
    QPlainTextEdit* descriptionEdit_ = nullptr;
    QLineEdit*      tagsEdit_        = nullptr;
};
