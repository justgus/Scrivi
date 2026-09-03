// package_folder_smoke — EP-035 / SP-127 (I-0185): a package is a LEAF, and a
// path inside one resolves UP to it.
//
// ⚠️ THE DEFECT. `.scrivi` and `.scrivworld` are DIRECTORIES, so `QFileDialog`
// walks into them. The writer double-clicks the project she wants and finds
// herself looking at `manuscript/` and `objects/` — the package's plumbing —
// one level below what she meant to choose.
//
// ⚠️ It still WORKED (the dialog returns the directory it is viewing), which is
// why no test caught it and why it reads as a papercut rather than a failure.
// ⚠️ But it lands on the app's two most common actions — Open Project and Add
// Existing World — and it makes the writer doubt she chose the right thing.
//
// ⚠️ INVISIBLE ON APPLE: Cocoa treats a package as an opaque bundle, so a
// double-click selects it. ⚠️ Every Qt platform re-earns this — Windows next.
//
// ⚠️ THE ASSERTION THAT MATTERS is the LAST one: a REAL double-click on a package
// row must CHOOSE it and must NOT navigate into it. ⚠️ An earlier fix only
// resolved the RETURN VALUE back up to the package after letting the dialog
// descend — ⚠️ which left the writer standing inside the package looking at its
// internals, i.e. it did not fix the complaint at all.
//
// What is asserted, against REAL directories on disk:
//   • a `.scrivi` / `.scrivworld` directory is recognised as a package;
//   • an ordinary directory is NOT;
//   • a path INSIDE a package resolves UP to the package — at any depth;
//   • an ordinary path is returned UNCHANGED (no surprise rewriting);
//   • a package nested under another resolves to the INNERMOST one.
//
//   argv[1] = a scratch directory to build the fixture in
//
// Exit 0 on success; non-zero with a FAIL line.

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QListView>
#include <QString>
#include <QTest>
#include <QTimer>

#include <cstdio>

#include "PackageFolderDialog.hpp"

namespace {

int fail(const QString& what)
{
    std::fprintf(stderr, "FAIL: %s\n", qPrintable(what));
    return 1;
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <scratch-dir>\n", argv[0]);
        return 2;
    }

    const QString root = QString::fromLocal8Bit(argv[1]);
    QDir base(root);

    // A realistic layout: a project package, a world package, and a plain folder.
    const QString proj  = base.absoluteFilePath(QStringLiteral("the-lone-golem.scrivi"));
    const QString world = base.absoluteFilePath(QStringLiteral("Eskandar.scrivworld"));
    const QString plain = base.absoluteFilePath(QStringLiteral("projects"));
    for (const QString& d : {proj, world, plain}) {
        if (!QDir().mkpath(d)) { return fail(QStringLiteral("could not create %1").arg(d)); }
    }
    // The plumbing a writer lands in when the picker descends.
    const QString inside = proj + QStringLiteral("/manuscript/chapter-001");
    if (!QDir().mkpath(inside)) { return fail(QStringLiteral("could not create %1").arg(inside)); }

    // 1 — recognition
    if (!PackageFolderDialog::isPackage(proj))  { return fail(QStringLiteral(".scrivi not recognised")); }
    if (!PackageFolderDialog::isPackage(world)) { return fail(QStringLiteral(".scrivworld not recognised")); }
    if (PackageFolderDialog::isPackage(plain))  { return fail(QStringLiteral("plain folder wrongly treated as a package")); }

    // 2 — ⚠️ THE ASSERTION THAT MATTERS: descending still lands on the package.
    if (PackageFolderDialog::packageRootFor(inside) != proj) {
        return fail(QStringLiteral("a path INSIDE a package did not resolve up to it (got %1)")
                        .arg(PackageFolderDialog::packageRootFor(inside)));
    }
    // One level down, too.
    if (PackageFolderDialog::packageRootFor(proj + QStringLiteral("/manuscript")) != proj) {
        return fail(QStringLiteral("one level inside a package did not resolve up"));
    }
    // Selecting the package itself is already correct and must stay so.
    if (PackageFolderDialog::packageRootFor(proj) != proj) {
        return fail(QStringLiteral("selecting the package itself did not return it"));
    }

    // 3 — ⚠️ ordinary paths must NOT be rewritten. A chooser that silently moves
    // the writer's selection somewhere else is worse than one that descends.
    if (PackageFolderDialog::packageRootFor(plain) != plain) {
        return fail(QStringLiteral("an ordinary folder was rewritten"));
    }

    // 4 — a package inside a package resolves to the INNERMOST, which is what a
    // writer viewing that folder is actually looking at.
    const QString nested = proj + QStringLiteral("/nested.scrivworld");
    if (!QDir().mkpath(nested + QStringLiteral("/objects"))) {
        return fail(QStringLiteral("could not create nested package"));
    }
    if (PackageFolderDialog::packageRootFor(nested + QStringLiteral("/objects")) != nested) {
        return fail(QStringLiteral("a nested package did not resolve to the innermost"));
    }

    // 5 — ⚠️ THE REAL INTERACTION. Everything above tests pure functions; this
    // drives an actual double-click through the actual dialog, because the
    // earlier fix passed every pure-function check while still descending.
    QString entered;
    QString returned;
    bool    clicked = false;

    QTimer::singleShot(600, [&]() {
        for (QWidget* w : QApplication::topLevelWidgets()) {
            auto* d = qobject_cast<QFileDialog*>(w);
            if (d == nullptr || !d->isVisible()) { continue; }
            QObject::connect(d, &QFileDialog::directoryEntered,
                             [&entered](const QString& p) { entered = p; });
            auto* lv = d->findChild<QListView*>(QStringLiteral("listView"));
            if (lv == nullptr) { d->reject(); return; }
            for (int r = 0; r < lv->model()->rowCount(lv->rootIndex()); ++r) {
                const QModelIndex i = lv->model()->index(r, 0, lv->rootIndex());
                if (i.data().toString() == QStringLiteral("Eskandar.scrivworld")) {
                    clicked = true;
                    QTest::mouseDClick(lv->viewport(), Qt::LeftButton, Qt::NoModifier,
                                       lv->visualRect(i).center());
                    return;
                }
            }
            d->reject();
        }
    });
    // Safety net so a regression cannot hang the suite.
    QTimer::singleShot(5000, []() { QApplication::quit(); });

    returned = PackageFolderDialog::choose(nullptr, QStringLiteral("t"), root);

    if (!clicked) { return fail(QStringLiteral("the package row was never double-clicked")); }
    if (returned != world) {
        return fail(QStringLiteral("a double-click did not CHOOSE the package (got '%1')").arg(returned));
    }
    if (entered.contains(QStringLiteral(".scrivworld"))) {
        return fail(QStringLiteral("the dialog NAVIGATED INTO the package (%1) — it must be atomic")
                        .arg(entered));
    }

    std::printf("package folder: a double-click CHOOSES the package and never enters it\n");
    return 0;
}
