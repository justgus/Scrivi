#include "ProjectSettingsDialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "ManuscriptTypography.hpp"
#include "ProjectPreferences.hpp"

ProjectSettingsDialog::ProjectSettingsDialog(ProjectPreferences* prefs, QWidget* parent)
    : QDialog(parent), prefs_(prefs)
{
    setObjectName(QStringLiteral("projectSettingsDialog"));
    setWindowTitle(tr("Project Settings"));
    auto* layout = new QVBoxLayout(this);

    // ── Project ──
    auto* project = new QGroupBox(tr("Project"), this);
    auto* projectForm = new QFormLayout(project);
    title_ = new QLineEdit(prefs->title(), project);
    title_->setPlaceholderText(tr("Untitled"));
    subtitle_ = new QLineEdit(prefs->subtitle(), project);
    subtitle_->setPlaceholderText(tr("Optional"));
    projectForm->addRow(tr("Title"), title_);
    projectForm->addRow(tr("Subtitle"), subtitle_);
    layout->addWidget(project);

    // ── Typography (EP-047 P2, P5a, P8: a BUNDLED face and a size, per project — it travels) ──
    auto* type = new QGroupBox(tr("Typography"), this);
    auto* typeForm = new QFormLayout(type);
    typeface_ = new QComboBox(type);
    for (const BundledFonts::Face& face : BundledFonts::manifest().faces) {
        typeface_->addItem(face.name, face.name);
        if (!face.family.isEmpty()) {
            typeface_->setItemData(typeface_->count() - 1, QFont(face.family), Qt::FontRole);   // each name in its own face
        }
    }
    // A face from a later Scrivi stays selected (and stored) — drawn in the default meanwhile (Apple's rule).
    if (typeface_->findData(prefs->typeface()) < 0) {
        typeface_->addItem(tr("%1 (not in this version)").arg(prefs->typeface()), prefs->typeface());
    }
    typeface_->setCurrentIndex(typeface_->findData(prefs->typeface()));
    size_ = new QSpinBox(type);
    size_->setRange(int(ManuscriptTypography::kMinSize), int(ManuscriptTypography::kMaxSize));
    size_->setSuffix(tr(" pt"));
    size_->setValue(int(prefs->textSize()));
    typeForm->addRow(tr("Typeface"), typeface_);
    typeForm->addRow(tr("Text size"), size_);
    layout->addWidget(type);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    // Live, as Apple's bindings are.
    connect(title_, &QLineEdit::textEdited, this, [this](const QString& t) { prefs_->setTitle(t); });
    connect(subtitle_, &QLineEdit::textEdited, this, [this](const QString& t) { prefs_->setSubtitle(t); });
    connect(typeface_, &QComboBox::activated, this,
            [this](int index) { prefs_->setTypeface(typeface_->itemData(index).toString()); });
    connect(size_, &QSpinBox::valueChanged, this, [this](int v) { prefs_->setTextSize(v); });
}
