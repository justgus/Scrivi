#include "ProjectSettingsDialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

#include <QLabel>

#include "AsyncCall.hpp"
#include "AuthorshipDialogs.hpp"
#include "ManuscriptTypography.hpp"
#include "ProjectPreferences.hpp"
#include "ScriviBridge.hpp"

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

    // ── Authors (EP-058 S3, [SP-211]; Apple's section, after Project): who wrote it, and who this machine writes as ──
    if (ScriviBridge* bridge = prefs->bridge(); bridge != nullptr) {
        auto* authors = new AuthorsBox(bridge, prefs->projectRootPath(), QString(), this);
        connect(authors, &AuthorsBox::writingAsChanged, this,
                [bridge](const QString& id, const QString& name) { bridge->adoptWritingAs(id, name); });
        layout->addWidget(authors);
    }

    // ── Writing Surface ([SP-184]; Apple's section, label and place) ──
    auto* surface = new QGroupBox(tr("Writing Surface"), this);
    auto* surfaceForm = new QVBoxLayout(surface);
    chapterTitles_ = new QCheckBox(tr("Show chapter titles in manuscript"), surface);
    chapterTitles_->setChecked(prefs->showChapterTitles());
    surfaceForm->addWidget(chapterTitles_);
    layout->addWidget(surface);

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
    // ✅ EP-047 S3 (P3, P10), [SP-183]: drawn, never typed — zero characters in the manuscript. Apple's rows and order.
    indent_ = new QComboBox(type);
    indent_->addItem(tr("Book convention"), QStringLiteral("book"));
    indent_->addItem(tr("Every paragraph"), QStringLiteral("every"));
    indent_->addItem(tr("None"), QStringLiteral("none"));
    // A mode from a later Scrivi is drawn as Book convention and shown as such (Apple's Picker shows no row for it).
    indent_->setCurrentIndex(std::max(0, indent_->findData(prefs->paragraphIndent())));
    indentEm_ = new QDoubleSpinBox(type);
    indentEm_->setRange(ManuscriptTypography::kMinIndentEm, ManuscriptTypography::kMaxIndentEm);
    indentEm_->setSingleStep(0.5);
    indentEm_->setDecimals(1);
    indentEm_->setSuffix(tr(" em"));
    indentEm_->setValue(prefs->indentEm());
    indentEm_->setEnabled(indent_->currentData().toString() != QLatin1String("none"));
    typeForm->addRow(tr("Typeface"), typeface_);
    typeForm->addRow(tr("Text size"), size_);
    typeForm->addRow(tr("Paragraph indent"), indent_);
    typeForm->addRow(tr("Indent"), indentEm_);
    layout->addWidget(type);

    // ── Undo History (EP-026 S3, [SP-204]; Apple's section, its wording) ──
    auto* history = new QGroupBox(tr("Undo History"), this);
    auto* historyForm = new QFormLayout(history);
    const auto hint = [history](const QString& text) {
        auto* l = new QLabel(text, history);
        l->setWordWrap(true);
        l->setEnabled(false);
        return l;
    };
    capacity_ = new QSpinBox(history);
    capacity_->setObjectName(QStringLiteral("historyCapacity"));
    capacity_->setRange(0, 10000000);
    capacity_->setEnabled(false);
    staleDays_ = new QSpinBox(history);
    staleDays_->setObjectName(QStringLiteral("historyStaleDays"));
    staleDays_->setRange(0, 36500);
    staleDays_->setEnabled(false);
    historyForm->addRow(tr("Maximum undo events"), capacity_);
    historyForm->addRow(hint(tr("Oldest edits fall off once this limit is reached. The current text is always kept.")));
    historyForm->addRow(tr("Stale after (days)"), staleDays_);
    historyForm->addRow(hint(tr("Abandoned branches untouched for this many days can be purged from Project ▸ Purge "
                                "Stale History Branches….")));
    layout->addWidget(history);
    // ⛔ Not on the GUI thread: `project.json` may be on a share.
    const QString root = prefs->projectRootPath();
    AsyncCall::run<QVariantMap>(
        this, [root]() { return ScriviBridge::historyGetSettings(root); },
        [this](const QVariantMap& s) {
            if (s.isEmpty()) {
                return;   // unread: the boxes stay disabled and nothing is written on close
            }
            historySettings_ = s;
            capacity_->setValue(s.value(QStringLiteral("capacityEvents")).toInt());
            staleDays_->setValue(s.value(QStringLiteral("staleBranchDays")).toInt());
            capacity_->setEnabled(true);
            staleDays_->setEnabled(true);
            historyLoaded_ = true;
        },
        {});

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    // Live, as Apple's bindings are.
    connect(title_, &QLineEdit::textEdited, this, [this](const QString& t) { prefs_->setTitle(t); });
    connect(subtitle_, &QLineEdit::textEdited, this, [this](const QString& t) { prefs_->setSubtitle(t); });
    connect(typeface_, &QComboBox::activated, this,
            [this](int index) { prefs_->setTypeface(typeface_->itemData(index).toString()); });
    connect(size_, &QSpinBox::valueChanged, this, [this](int v) { prefs_->setTextSize(v); });
    connect(indent_, &QComboBox::activated, this, [this](int index) {
        const QString mode = indent_->itemData(index).toString();
        indentEm_->setEnabled(mode != QLatin1String("none"));
        prefs_->setParagraphIndent(mode);
    });
    connect(chapterTitles_, &QCheckBox::toggled, this, [this](bool on) { prefs_->setShowChapterTitles(on); });
    connect(indentEm_, &QDoubleSpinBox::valueChanged, this, [this](double v) { prefs_->setIndentEm(v); });
}

void ProjectSettingsDialog::done(int result)
{
    // Apple's `saveHistorySettings` (on Done): only what was read, and only when it changed; `idleRolloverHours` kept.
    if (historyLoaded_ && (capacity_->value() != historySettings_.value(QStringLiteral("capacityEvents")).toInt()
                           || staleDays_->value() != historySettings_.value(QStringLiteral("staleBranchDays")).toInt())) {
        QVariantMap s = historySettings_;
        s.insert(QStringLiteral("capacityEvents"), capacity_->value());
        s.insert(QStringLiteral("staleBranchDays"), staleDays_->value());
        const QString root = prefs_->projectRootPath();
        // Written off the GUI thread; the write outlives the dialog (its context is the application).
        AsyncCall::run<QVariantMap>(qApp, [root, s]() { return ScriviBridge::historySetSettings(root, s); }, {}, {}, 0);
    }
    QDialog::done(result);
}
