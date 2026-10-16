#pragma once

#include <QDialog>

class ProjectPreferences;
class QComboBox;
class QLineEdit;
class QSpinBox;

// EP-048 S5a (SP-182, T-0609) — Project ▸ Project Settings… on Linux. ✅ Apple's sheet (`ProjectSettingsSheet.swift`), with
// only what Linux has built ([SP-182] Q2): Project (Title, Subtitle) and Typography (Typeface, Text size). [SP-183] adds the
// indent rows, [SP-184] "Show chapter titles", EP-026 S3 Undo History.
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

private:
    ProjectPreferences* prefs_;
    QLineEdit* title_;
    QLineEdit* subtitle_;
    QComboBox* typeface_;
    QSpinBox* size_;
};
