#pragma once

#include <QAbstractItemView>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QListView>
#include <QMouseEvent>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QTreeView>
#include <QWidget>

// PackageFolderDialog — inside Scrivi, a `.scrivi` / `.scrivworld` package is an
// ATOM (EP-035 / SP-127, I-0185).
//
// ## ⚠️ The rule, stated by the user and not negotiable
//
// > *"The writer must perceive the folder structure as if it were a single file…
// > in the app it must appear as if it were an atomic thing."*
//
// ⚠️ **Descending into a package must be IMPOSSIBLE, not merely recoverable.** A
// double-click on a package CHOOSES it. It never enters it. The writer must never
// be shown `manuscript/`, `objects/`, `world.json` — that is the package's
// plumbing, and seeing it invites her to treat the parts as things she may move.
//
// ⚠️ **An earlier version of this file got this WRONG**: it let the dialog descend
// and then resolved the path back up to the package on accept. ⚠️ **That fixes the
// RETURN VALUE while leaving the writer standing inside the package looking at its
// internals — which is the actual complaint.** ✅ **Prevent the navigation.**
//
// ⚠️ On disk it is plainly a directory and that is fine — the user is explicit
// that nothing should change there. ⚠️ The atomicity is a property of the APP.
//
// ## ⚠️ Why this is not a Qt option
//
// Qt has no "treat this directory as a leaf" flag. Cocoa does (packages are
// bundles), which is ⚠️ **why this defect is INVISIBLE ON APPLE and why every Qt
// platform re-earns it — Windows next**
// (`Scrivi_Platform_Porting_Outline_v0_1.md`).
//
// So: a NON-NATIVE `QFileDialog` (the native GTK one owns its own input handling
// and cannot be told any of this), plus an event filter on both item views that
// swallows the double-click on a package and accepts the dialog instead.
namespace PackageFolderDialog {

// The package suffixes Scrivi owns. ⚠️ Both are directories on disk.
inline QStringList packageSuffixes()
{
    return {QStringLiteral("scrivi"), QStringLiteral("scrivworld")};
}

inline bool isPackage(const QString& path)
{
    if (path.isEmpty()) { return false; }
    const QFileInfo fi(path);
    if (!fi.isDir()) { return false; }
    return packageSuffixes().contains(fi.suffix(), Qt::CaseInsensitive);
}

// If `path` sits INSIDE a package, return that package; otherwise return `path`.
//
// ⚠️ Kept as a BACKSTOP even though navigation is now blocked. A writer can still
// reach the inside of a package by typing a path, or by opening the dialog with a
// startDir already within one. ⚠️ Blocking the click is the fix; this makes the
// return value correct if some other route gets there anyway.
inline QString packageRootFor(const QString& path)
{
    QDir d(path);
    while (true) {
        const QString cur = d.absolutePath();
        if (isPackage(cur)) { return cur; }
        if (!d.cdUp()) { return path; }   // hit the filesystem root
    }
}

namespace detail {

// ⚠️ `QFileDialog` re-declares `done()`/`accept()` as PROTECTED, so a filter
// cannot close the dialog from outside. This subclass exists only to expose it.
class ChooserDialog : public QFileDialog {
public:
    using QFileDialog::QFileDialog;

    // ⚠️ `selectFile()` does NOT work here: in Directory mode `selectedFiles()`
    // reports the directory the dialog is VIEWING, so a package chosen by
    // double-click was reported as its PARENT. Record the choice explicitly and
    // let `choose()` prefer it.
    void chooseAndClose(const QString& path)
    {
        chosen = path;
        done(QDialog::Accepted);
    }

    QString chosen;   // empty unless a package was double-clicked
};

// Swallows the interaction that would enter a package, and accepts instead.
class PackageAtomFilter : public QObject {
public:
    PackageAtomFilter(ChooserDialog* dlg, QObject* parent)
        : QObject(parent), dlg_(dlg) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() != QEvent::MouseButtonDblClick) {
            return QObject::eventFilter(watched, event);
        }
        auto* view = qobject_cast<QAbstractItemView*>(watched->parent());
        if (view == nullptr) { return QObject::eventFilter(watched, event); }

        auto* me = static_cast<QMouseEvent*>(event);
        const QModelIndex idx = view->indexAt(me->position().toPoint());
        if (!idx.isValid()) { return QObject::eventFilter(watched, event); }

        const QString path = pathFor(idx);
        if (!isPackage(path)) { return QObject::eventFilter(watched, event); }

        // ⚠️ A package was double-clicked. CHOOSE it — do not enter it. Returning
        // true here is what makes the package atomic: Qt never sees the event and
        // never navigates.
        //
        // ⚠️ Accept via the event loop rather than inline: we are inside the
        // view's event handling, and tearing the dialog down underneath it is how
        // you get a use-after-free.
        chosen_ = path;
        QTimer::singleShot(0, dlg_, [this]() { dlg_->chooseAndClose(chosen_); });
        return true;
    }

private:
    // The dialog's view may sit behind a proxy model; resolve either shape.
    QString pathFor(const QModelIndex& idx) const
    {
        QModelIndex source = idx;
        const QAbstractItemModel* model = idx.model();
        while (const auto* proxy = qobject_cast<const QSortFilterProxyModel*>(model)) {
            source = proxy->mapToSource(source);
            model  = proxy->sourceModel();
        }
        if (const auto* fsm = qobject_cast<const QFileSystemModel*>(model)) {
            return fsm->filePath(source);
        }
        return {};
    }

    ChooserDialog* dlg_ = nullptr;
    QString        chosen_;
};

}  // namespace detail

// Shows the chooser. Returns an absolute path, or "" if cancelled.
inline QString choose(QWidget* parent, const QString& title, const QString& startDir)
{
    detail::ChooserDialog dlg(parent, title, startDir);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly, true);
    // ⚠️ MUST be non-native: the GTK dialog owns its own double-click handling and
    // exposes no way to make a directory behave as a leaf.
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    // ✅ "Choose", not "Open" — the writer is picking a thing, not opening a folder.
    dlg.setLabelText(QFileDialog::Accept, QObject::tr("Choose"));

    // ⚠️ Filter the VIEWPORTS: item views deliver mouse events to the viewport,
    // not to the view itself.
    auto* filter = new detail::PackageAtomFilter(&dlg, &dlg);
    for (auto* view : dlg.findChildren<QAbstractItemView*>()) {
        view->viewport()->installEventFilter(filter);
    }

    if (dlg.exec() != QDialog::Accepted) { return {}; }

    // ⚠️ A double-clicked package wins over selectedFiles(), which in Directory
    // mode reports the VIEWED directory — i.e. the package's parent.
    if (!dlg.chosen.isEmpty()) { return dlg.chosen; }

    const QStringList picked = dlg.selectedFiles();
    if (picked.isEmpty()) { return {}; }

    // ⚠️ Backstop — see packageRootFor(). Navigation is blocked, but a typed path
    // can still land inside a package.
    return packageRootFor(picked.first());
}

}  // namespace PackageFolderDialog
