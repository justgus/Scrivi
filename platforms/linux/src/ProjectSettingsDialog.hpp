#pragma once

#include <QDialog>
#include <QVariantMap>

class ProjectPreferences;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;

// EP-048 S5a (SP-182, T-0609) — Project ▸ Project Settings… on Linux. ✅ Apple's sheet (`ProjectSettingsSheet.swift`), with
// only what Linux has built ([SP-182] Q2): Project (Title, Subtitle) and Typography (Typeface, Text size, and [SP-183]'s
// Paragraph indent and Indent), [SP-184] "Show chapter titles". ✅ EP-026 S3 ([SP-204]) adds Undo History: the two values Apple's sheet
// holds, read off the GUI thread and written when the dialog closes (Apple writes them on Done).
// Changes apply LIVE, as Apple's do — each control writes its preference as it changes; Close only closes.
class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectSettingsDialog(ProjectPreferences* prefs, QWidget* parent = nullptr);

    // For smokes.
    QLineEdit* titleField() const { return title_; }
    QLineEdit* subtitleField() const { return subtitle_; }
    QComboBox* typefaceBox() const { return typeface_; }
    QSpinBox* sizeBox() const { return size_; }
    QComboBox* indentBox() const { return indent_; }
    QDoubleSpinBox* indentEmBox() const { return indentEm_; }
    QSpinBox* capacityBox() const { return capacity_; }
    QSpinBox* staleDaysBox() const { return staleDays_; }
    // True once the history settings were read (the boxes are disabled until then).
    bool historyLoaded() const { return historyLoaded_; }

    // Writes Undo History as the dialog closes (Close, Esc or the window's close).
    void done(int result) override;

private:
    ProjectPreferences* prefs_;
    QLineEdit* title_;
    QLineEdit* subtitle_;
    QComboBox* typeface_;
    QSpinBox* size_;
    QComboBox* indent_;
    QDoubleSpinBox* indentEm_;
    QSpinBox* capacity_;
    QSpinBox* staleDays_;
    QVariantMap historySettings_;   // as read: `idleRolloverHours` is kept, not edited
    bool historyLoaded_ = false;
};
