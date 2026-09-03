#include "WorldsDialog.hpp"

#include "ScriviBridge.hpp"
#include "PackageFolderDialog.hpp"
#include "ThemeColours.hpp"
#include "WorldStatusText.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QColor>
#include <QPaintEvent>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QVariantList>

namespace {

// ⚠️ The path a row should SHOW.
//
// `packagePath` is non-empty ONLY when the world resolved (scrivi.h:275 and
// WorldStore.hpp) — so for the unavailable worlds, which are precisely the ones
// a writer needs to locate, it is EMPTY BY DESIGN. `lastKnownPackagePath` is
// "where resolution looked", carried regardless of outcome, and it exists for
// exactly this case.
//
// ⚠️ It is NOT proof the world is there. It is shown as a hint to help the writer
// recognise which world this is and where it used to live — never as a claim.
QString displayPath(const QString& packagePath, const QString& lastKnown)
{
    return packagePath.isEmpty() ? lastKnown : packagePath;
}

// Middle-elides a path to fit, because the informative end of a world path is
// its TAIL — the package name — and left-truncation would hide it.
//
// ⚠️ I-0184 — THE WIDTH MUST BE THE LABEL'S OWN, NOT A CONSTANT.
//
// ⚠️ This previously elided against a hardcoded 360 px. That is not the width the
// label actually gets: the row also holds a marker, a status label and up to two
// buttons, and the dialog is resizable. When the real width came in under 360 the
// path was cut to little more than an ellipsis, and ⚠️ because the elide happened
// ONCE at construction, WIDENING THE DIALOG NEVER REVEALED MORE. The user
// reported the row as showing only a name and a status — the path was there and
// unreadable, which is worse than absent because it looks deliberate.
//
// ✅ Re-elide on every resize, from the full path kept in `full_`.
class ElidingPathLabel : public QLabel {
public:
    ElidingPathLabel(QString path, QWidget* parent)
        : QLabel(parent), full_(std::move(path))
    {
        setToolTip(full_);                 // the full path stays reachable
        setProperty("fullPath", full_);    // and testable
        QFont f = font();
        f.setPointSizeF(f.pointSizeF() - 1.0);
        setFont(f);
        ThemeColours::applyTextColour(this, ThemeColours::deemphasised(palette()));
        // ⚠️ Without this the label demands its FULL natural width and pushes the
        // buttons off the row instead of shrinking — the elide never engages.
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        setMinimumWidth(80);
        setText(full_);
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        applyElide();
    }

    // ⚠️ resizeEvent is NOT enough on its own. A widget laid out but never shown
    // gets no resize event, so the FIRST paint would use the unelided text and
    // Qt would clip the tail — the exact symptom this fix exists to remove.
    // Eliding at paint time makes the width the label ACTUALLY has authoritative,
    // whatever sequence of events got it there.
    void paintEvent(QPaintEvent* event) override
    {
        applyElide();
        QLabel::paintEvent(event);
    }

private:
    void applyElide()
    {
        const int avail = contentsRect().width();
        if (avail <= 0) { return; }
        const QString elided = QFontMetrics(font()).elidedText(full_, Qt::ElideMiddle, avail);
        // ⚠️ Guard against re-entry: setText() inside paintEvent would schedule
        // another paint, and an unconditional set would loop forever.
        if (elided != text()) { setText(elided); }
    }

    QString full_;
};

QLabel* makeElidedPathLabel(const QString& path, QWidget* parent)
{
    return new ElidingPathLabel(path, parent);
}

}  // namespace

WorldsDialog::WorldsDialog(ScriviBridge* bridge, QString projectRootPath, QWidget* parent)
    : QDialog(parent), bridge_(bridge), projectRootPath_(std::move(projectRootPath))
{
    setWindowTitle(tr("Worlds"));
    setModal(true);
    resize(560, 420);

    auto* layout = new QVBoxLayout(this);

    auto* heading = new QLabel(tr("Worlds"), this);
    QFont hf = heading->font();
    hf.setBold(true);
    hf.setPointSizeF(hf.pointSizeF() + 1.0);
    heading->setFont(hf);
    layout->addWidget(heading);

    // ⚠️ Mirrors Apple's explanatory copy word for word. It answers the question a
    // first-time writer actually has — what a world IS, and why sources are not in
    // it — which no amount of UI affordance conveys on its own.
    auto* blurb = new QLabel(
        tr("A world holds your characters, locations, items, and everything else "
           "you invent, and can be shared between projects. Only sources — your "
           "research and references — belong to the project itself."),
        this);
    blurb->setWordWrap(true);
    ThemeColours::applyTextColour(blurb, ThemeColours::deemphasised(blurb->palette()));
    layout->addWidget(blurb);

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    ThemeColours::applyTextColour(errorLabel_, ThemeColours::attention(errorLabel_->palette()));
    errorLabel_->hide();
    layout->addWidget(errorLabel_);

    // The list, scrollable — a project may bind more worlds than fit.
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::StyledPanel);
    auto* rowsHost = new QWidget(scroll);
    rowsLayout_ = new QVBoxLayout(rowsHost);
    rowsLayout_->setContentsMargins(4, 4, 4, 4);
    rowsLayout_->setSpacing(2);
    rowsLayout_->addStretch();
    scroll->setWidget(rowsHost);
    layout->addWidget(scroll, 1);

    // ⚠️ An explicit empty state, never a blank box: "this project uses no worlds"
    // is information, whereas an empty panel reads as a failure to load.
    emptyLabel_ = new QLabel(tr("This project uses no worlds yet."), this);
    ThemeColours::applyTextColour(emptyLabel_, ThemeColours::deemphasised(emptyLabel_->palette()));
    emptyLabel_->hide();
    layout->addWidget(emptyLabel_);

    auto* buttons = new QDialogButtonBox(this);
    auto* addBtn = buttons->addButton(tr("Add Existing World…"), QDialogButtonBox::ActionRole);
    connect(addBtn, &QPushButton::clicked, this, &WorldsDialog::addExistingWorld);
    auto* closeBtn = buttons->addButton(QDialogButtonBox::Close);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(buttons);

    reload();
}

void WorldsDialog::reload()
{
    worlds_.clear();

    const QVariantMap result = bridge_->listWorlds(projectRootPath_);
    // ⚠️ An empty envelope is AMBIGUOUS — appendToArray omits the key entirely for
    // an empty list, so `{}` means "no worlds" OR "the call failed"
    // (project_envelope_empty_vs_failed). lastCallFailed() is the only way to tell.
    if (bridge_->lastCallFailed()) {
        showActionError(tr("Could not read this project's worlds."));
        rebuildRows();
        return;
    }

    const QVariantList list = result.value(QStringLiteral("worlds")).toList();
    for (const QVariant& v : list) {
        const QVariantMap m = v.toMap();
        Entry e;
        e.worldID              = m.value(QStringLiteral("worldID")).toString();
        e.displayName          = m.value(QStringLiteral("displayName")).toString();
        e.status               = m.value(QStringLiteral("status")).toString();
        e.statusReason         = m.value(QStringLiteral("statusReason")).toString();
        e.packagePath          = m.value(QStringLiteral("packagePath")).toString();
        e.lastKnownPackagePath = m.value(QStringLiteral("lastKnownPackagePath")).toString();
        worlds_.append(e);
    }
    rebuildRows();
}

void WorldsDialog::rebuildRows()
{
    // Drop the existing rows, keeping the trailing stretch.
    while (rowsLayout_->count() > 1) {
        QLayoutItem* item = rowsLayout_->takeAt(0);
        if (QWidget* w = item->widget()) { w->deleteLater(); }
        delete item;
    }

    emptyLabel_->setVisible(worlds_.isEmpty());

    for (const Entry& e : worlds_) {
        rowsLayout_->insertWidget(rowsLayout_->count() - 1, makeRow(e));
    }
}

QWidget* WorldsDialog::makeRow(const Entry& e)
{
    auto* row = new QWidget(this);
    auto* h = new QHBoxLayout(row);
    h->setContentsMargins(2, 2, 2, 2);
    h->setSpacing(8);

    const bool available = WorldStatusText::isAvailable(e.status);

    // ⚠️ A text marker rather than an icon: the rig is driven over RDP where icon
    // themes vary, and a glyph that fails to load leaves NO signal at all.
    auto* marker = new QLabel(available ? QStringLiteral("●") : QStringLiteral("▲"), row);
    ThemeColours::applyTextColour(
        marker, available ? ThemeColours::deemphasised(marker->palette())
                          : ThemeColours::attention(marker->palette()));
    marker->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);  // I-0184
    h->addWidget(marker);

    auto* text = new QVBoxLayout;
    text->setSpacing(0);
    auto* name = new QLabel(e.displayName.isEmpty() ? e.worldID : e.displayName, row);
    // ⚠️ I-0184: a long displayName (or a bare worldID fallback, which is a UUID)
    // must not set the column's minimum width and squeeze the path out.
    name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    text->addWidget(name);
    text->addWidget(makeElidedPathLabel(displayPath(e.packagePath, e.lastKnownPackagePath), row));
    h->addLayout(text, 1);

    if (!available) {
        // ⚠️ Says what is actually wrong and NEVER guesses. A wrong "missing"
        // would invite restoring from backup when the volume was merely
        // disconnected (Doc 2 §7.2.1).
        auto* status = new QLabel(WorldStatusText::writerDescription(e.status), row);
        ThemeColours::applyTextColour(status, ThemeColours::attention(status->palette()));
        // ⚠️ I-0184: the status text is SHORT and fixed — it must not take slack
        // from the path column, which is the part that needs every pixel. Without
        // this the two compete and the path loses.
        status->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        // T-0440: the core's reason, when it has one, as a tooltip rather than
        // inline — it is a diagnostic code, not writer-facing prose.
        if (!e.statusReason.isEmpty()) { status->setToolTip(e.statusReason); }
        h->addWidget(status);

        // ⚠️ Offered ONLY when there is something to fix. Relinking a world that
        // resolves fine is not a repair — it is a way to bind the WRONG package.
        auto* locate = new QPushButton(tr("Locate…"), row);
        locate->setToolTip(tr("Find where “%1” has moved to").arg(e.displayName));
        connect(locate, &QPushButton::clicked, this, [this, e]() { relinkWorld(e); });
        h->addWidget(locate);
    }

    auto* remove = new QPushButton(tr("Remove"), row);
    remove->setToolTip(tr("Remove this world from the project"));
    connect(remove, &QPushButton::clicked, this, [this, e]() { removeWorldReference(e); });
    h->addWidget(remove);

    return row;
}

void WorldsDialog::relinkWorld(const Entry& e)
{
    // ⚠️ A `.scrivworld` package is a DIRECTORY. A file picker cannot select one,
    // so this must be the directory chooser.
    //
    // ⚠️ Starts from the last known path's PARENT when there is one: a writer
    // looking for a moved world is usually looking near where it used to be.
    QString startDir;
    const QString known = displayPath(e.packagePath, e.lastKnownPackagePath);
    if (!known.isEmpty()) {
        startDir = QFileInfo(known).absolutePath();
    }

    // ⚠️ I-0185: a `.scrivworld` is a DIRECTORY, so a plain directory picker
    // walks INTO it and the writer ends up looking at `world.json`. This chooser
    // treats a package as a leaf — and still resolves upward if she descends.
    const QString picked = PackageFolderDialog::choose(
        this, tr("Locate “%1”").arg(e.displayName), startDir);
    if (picked.isEmpty()) { return; }   // cancelled — not an error

    bridge_->relinkWorld(projectRootPath_, e.worldID, picked);
    if (bridge_->lastCallFailed()) {
        // ⚠️ The core VERIFIES worldID before accepting a relink, so a failure
        // here usually means the writer picked a package that is a DIFFERENT
        // world. ⚠️ Surface it — suppressing it would let her believe a repair
        // happened that did not.
        showActionError(tr("“%1” could not be relinked to that folder. "
                           "Check that it is the right .scrivworld package.")
                            .arg(e.displayName));
        return;
    }

    showActionError(QString());   // clear
    // ⚠️ I-0130: refresh the WHOLE app, not just this dialog. reload() re-reads
    // only this list; the Scene Inspector's status line lives elsewhere and would
    // keep reporting the old state until the writer changed scenes.
    emit worldsChanged();
    reload();
}

void WorldsDialog::addExistingWorld()
{
    // ⚠️ I-0185 — see relinkWorld(). Same defect, same fix.
    const QString picked = PackageFolderDialog::choose(
        this, tr("Add Existing World"), QString());
    if (picked.isEmpty()) { return; }

    bridge_->addWorld(projectRootPath_, picked);
    if (bridge_->lastCallFailed()) {
        showActionError(tr("That folder could not be added as a world. "
                           "Check that it is a .scrivworld package."));
        return;
    }

    showActionError(QString());
    emit worldsChanged();
    reload();
}

void WorldsDialog::removeWorldReference(const Entry& e)
{
    // ⚠️ "Remove" next to a world she just built reads as "delete my world" unless
    // the copy rules it out. It removes THIS PROJECT'S REFERENCE only — the
    // package on disk is never touched (scrivi.h:264).
    const QString name = e.displayName.isEmpty() ? e.worldID : e.displayName;
    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("Remove World"));
    box.setText(tr("Remove “%1” from this project?").arg(name));
    box.setInformativeText(
        tr("“%1” will no longer be part of this project, and anything in this "
           "project that refers to it will stop resolving. The world package "
           "itself is not deleted — it stays on disk and can be added back.")
            .arg(name));
    box.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    box.setDefaultButton(QMessageBox::Cancel);
    if (box.exec() != QMessageBox::Yes) { return; }

    bridge_->removeWorldReference(projectRootPath_, e.worldID);
    if (bridge_->lastCallFailed()) {
        showActionError(tr("“%1” could not be removed from this project.").arg(name));
        return;
    }

    showActionError(QString());
    emit worldsChanged();
    reload();
}

void WorldsDialog::showActionError(const QString& text)
{
    errorLabel_->setText(text);
    errorLabel_->setVisible(!text.isEmpty());
}
