// branch_smoke.cpp — EP-026 S3 ([SP-204] / T-0662): branching undo on Linux, end to end.
//
// A REAL `EditorShell` loads a real project; keys go in through `QTest` (the real dispatch, ShortcutOverride included —
// `feedback_test_through_the_real_dispatch`); every claim about text is read back FROM DISK.
//
// ✅ What it proves (SP-204 AC1–AC6):
//   • at a branch point Ctrl+Shift+Z opens the chooser (never an undo, [I-0320]); choosing redoes onto that branch; Esc
//     redoes nothing; the highlight starts on the primary;
//   • the branch mark: in the gutter, on the branch's line, only with Markup Hints on; never in the text or a save; a click
//     opens the chooser; a mark whose line has never been laid out is skipped, not read (step 0's crash), and appears once
//     scrolled to;
//   • a branch written to the log BEFORE this window opened (through the core, as the Mac writes it) is chosen on Linux;
//   • the History section: closed by default, fetched on a history change only (never per scene, [I-0331]), a click on a
//     branch re-primaries it and the next Ctrl+Shift+Z follows it;
//   • Project Settings ▸ Undo History reads and writes the core's settings;
//   • Project ▸ Purge Stale History Branches… lists a stale branch and purges it only after its confirmation.
// ⚠️ NOT covered (live pass only): how the chooser and the mark look on GNOME, Orca on the mark.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "HistoryCapture.hpp"
#include "HistorySection.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptEscapes.hpp"
#include "ProjectPreferences.hpp"
#include "ProjectSettingsDialog.hpp"
#include "SceneInspector.hpp"
#include "ScriviBridge.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSpinBox>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextLayout>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>

#include <cstdio>
#include <cstdlib>
#include <memory>

namespace {

int failures = 0;

QString esc(const QString& typed) { return ManuscriptEscapes::escape(typed); }

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

// The modal history notices (a barrier, "Undo a change from a previous session?") are accepted and recorded — Undo.
QStringList notices;
void armNoticeCatcher(QObject* owner)
{
    auto* t = new QTimer(owner);
    t->setInterval(20);
    QObject::connect(t, &QTimer::timeout, [] {
        auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        if (box != nullptr && box->objectName() == QStringLiteral("historyNotice")) {
            notices.append(box->text());
            box->accept();
        }
    });
    t->start();
}

bool waitFor(const std::function<bool()>& done, int ms = 10000)
{
    for (int waited = 0; waited < ms; waited += 20) {
        if (done()) {
            return true;
        }
        QTest::qWait(20);
    }
    return done();
}

struct Rig {
    ScriviBridge bridge;
    QString projectDir;
    QString appSupport;
    QStringList sceneIDs;
    std::unique_ptr<EditorShell> shell;
    ManuscriptEditor* editor = nullptr;

    bool load()
    {
        shell = std::make_unique<EditorShell>();
        QEventLoop loop;
        bool ok = false;
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) { ok = r; loop.quit(); });
        shell->load(projectDir, appSupport, QStringLiteral("Branch"));
        QTimer::singleShot(60000, &loop, &QEventLoop::quit);
        loop.exec();
        editor = shell->findChild<ManuscriptEditor*>();
        if (!ok || editor == nullptr) {
            return false;
        }
        shell->resize(900, 500);
        shell->show();
        shell->activateWindow();
        check(QTest::qWaitForWindowActive(shell.get()), QStringLiteral("the window became active (shortcuts need it)"));
        editor->setFocus();
        settle();
        return true;
    }
    void close()
    {
        shell->flushHistory();
        shell->saveDirtyScenes();
        shell->releaseProject();
        shell->hide();
        shell.reset();
        editor = nullptr;
    }
    QString disk(int i)
    {
        shell->saveDirtyScenes();
        return bridge.readSceneTexts(projectDir, {sceneIDs.at(i)}).value(sceneIDs.at(i));
    }
    void caretAt(const QString& marker)
    {
        const QTextCursor found = editor->document()->find(marker);
        QTextCursor c = editor->textCursor();
        c.setPosition(found.selectionEnd());
        editor->setTextCursor(c);
        settle();
    }
    void type(const QString& text) { QTest::keyClicks(editor, text); settle(); }
    void control(Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier | extra);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier | extra);
        settle();
    }
    void undo() { control(Qt::Key_Z); }
    void redo() { control(Qt::Key_Z, Qt::ShiftModifier); }
    QList<QAbstractButton*> marks() const
    {
        QList<QAbstractButton*> visible;
        for (QAbstractButton* b : editor->findChildren<QAbstractButton*>(QStringLiteral("branchMark"))) {
            if (b->isVisible()) {
                visible.append(b);
            }
        }
        return visible;
    }
    static QMenu* chooser() { return qobject_cast<QMenu*>(QApplication::activePopupWidget()); }
    // The chooser's branch rows (enabled, with an eventID).
    static QList<QAction*> rows(QMenu* menu)
    {
        QList<QAction*> out;
        for (QAction* a : menu->actions()) {
            if (a->isEnabled() && !a->data().toString().isEmpty()) {
                out.append(a);
            }
        }
        return out;
    }
    // Move the chooser's highlight with the arrow keys until it reaches a row whose text holds `needle`, then Return.
    static bool chooseByKeys(QMenu* menu, const QString& needle)
    {
        for (int i = 0; i < 8; ++i) {
            if (menu->activeAction() != nullptr && menu->activeAction()->text().contains(needle)) {
                QTest::keyClick(menu, Qt::Key_Return);
                settle();
                return true;
            }
            QTest::keyClick(menu, Qt::Key_Down);
        }
        return false;
    }
    HistorySection* history() const { return shell->findChild<SceneInspector*>()->historySection(); }
    static void settle() { QTest::qWait(60); }
};

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: branch_smoke <projectDir>\n");
        return 2;
    }

    armNoticeCatcher(&app);
    // A guard: a modal nobody answers fails the run instead of hanging it.
    QTimer::singleShot(240000, [] {
        std::printf("FAIL: the smoke did not finish in 240 s (modal: %s)\n",
                    QApplication::activeModalWidget() != nullptr ? qUtf8Printable(QApplication::activeModalWidget()->objectName()) : "none");
        std::fflush(stdout);
        std::_Exit(1);
    });

    Rig rig;
    rig.projectDir = QString::fromUtf8(argv[1]);
    rig.appSupport = scrivi::linux_app::appSupportRoot();
    rig.bridge.bootstrap(QStringLiteral("Scrivi"), rig.appSupport);
    const QString projectID = rig.bridge.createProject(rig.projectDir, rig.appSupport, QStringLiteral("Branch"),
                                                       QStringLiteral("branch")).value(QStringLiteral("projectID")).toString();
    check(!projectID.isEmpty(), QStringLiteral("project created"));

    // Three scenes, seeded on disk before the first open. A LONG scene 1 keeps scenes 0 and 2 a screen apart: the window
    // opens at the END, so scene 0's lines are never laid out (`QPlainTextEdit` lays out lazily — step 0's finding).
    QString longScene = QStringLiteral("Bravo two.");
    for (int i = 0; i < 400; ++i) {
        longScene += QStringLiteral("\n\nFiller paragraph %1 keeps Charlie off the first screen.").arg(i);
    }
    const QStringList seeds{QStringLiteral("Alpha one."), longScene, QStringLiteral("Charlie three.")};
    const QString midMarker = QStringLiteral("Filler paragraph 200 keeps Charlie off the first screen.");
    const qsizetype midAt = longScene.indexOf(midMarker) + midMarker.size();
    {
        QVariantList scenes = rig.bridge.openProject(rig.projectDir, rig.appSupport).value(QStringLiteral("scenes")).toList();
        const QString chapterID = scenes.first().toMap().value(QStringLiteral("chapterID")).toString();
        for (int i = 1; i < 3; ++i) {
            rig.bridge.createScene(rig.projectDir, rig.appSupport, projectID, chapterID,
                                   scenes.last().toMap().value(QStringLiteral("sceneID")).toString());
            scenes = rig.bridge.openProject(rig.projectDir, rig.appSupport).value(QStringLiteral("scenes")).toList();
        }
        for (int i = 0; i < 3; ++i) {
            const QVariantMap s = scenes.at(i).toMap();
            rig.sceneIDs.append(s.value(QStringLiteral("sceneID")).toString());
            rig.bridge.saveScene(projectID, rig.projectDir, rig.appSupport, rig.sceneIDs.last(),
                                 s.value(QStringLiteral("metadataPath")).toString(),
                                 s.value(QStringLiteral("contentPath")).toString(), seeds.at(i), 0, 0, 0.0);
        }
        rig.bridge.closeProject(rig.projectDir);
    }

    // ── 0. A branch in scene 2 written BEFORE this window opens — through the core, as the Mac writes the log ──────────
    {
        const QString root = rig.projectDir;
        // In the MIDDLE of the long scene: the window opens at the end, and the middle is never laid out.
        const QString c = rig.sceneIDs.at(1);
        check(!ScriviBridge::historyOpen(root).isEmpty(), QStringLiteral("the core opened the history for the 'Mac' writes"));
        ScriviBridge::historyValidateScene(root, c, seeds.at(1));
        const QString base = seeds.at(1);
        const auto write = [&](const QString& added) {
            const QString after = QString(base).insert(midAt, added);
            ScriviBridge::captureNoteEdit(root, c, after, int(midAt + added.size()), base, HistoryCapture::nowMs());
            ScriviBridge::captureFlush(root, QStringLiteral("test"));
            ScriviBridge::historyNoteScenePersisted(root, c, after);
        };
        write(esc(QStringLiteral(" Mac first.")));
        ScriviBridge::captureUndo(root);
        write(esc(QStringLiteral(" Mac second.")));
        ScriviBridge::captureUndo(root);   // the pointer rests on the branch point, as the Mac left it
        ScriviBridge::historyNoteScenePersisted(root, c, base);
        check(!ScriviBridge::captureState(root).value(QStringLiteral("redoFork")).toMap().isEmpty(),
              QStringLiteral("the 'Mac' log ends on a branch point"));
        ScriviBridge::historyClose(root);
    }

    check(rig.load(), QStringLiteral("project loaded"));
    check(rig.shell->history() != nullptr && rig.shell->history()->isOpen(), QStringLiteral("the undo history opened"));

    // ── 1. A branch point whose line was never laid out: skipped, not read; Ctrl+Shift+Z still asks, at the caret ────
    check(!rig.shell->history()->redoForkNow().isEmpty(), QStringLiteral("Linux reads the Mac's branch point on open"));
    rig.caretAt(QStringLiteral("Charlie three."));   // the end, where the window opened
    const int midPos = rig.shell->sceneDocument().segments().at(1).bodyStart + int(midAt);
    check(rig.editor->document()->findBlock(midPos).layout()->lineCount() == 0,
          QStringLiteral("the fixture holds: the branch point's line has never been laid out"));
    check(rig.marks().isEmpty(), QStringLiteral("no mark for a branch whose line is off screen (and no crash reading it)"));
    rig.redo();
    QMenu* menu = Rig::chooser();
    check(menu != nullptr && menu->objectName() == QStringLiteral("branchChooser"),
          QStringLiteral("Ctrl+Shift+Z at the Mac's branch point opened the chooser (at the caret: no mark on screen)"));
    if (menu != nullptr) {
        check(Rig::rows(menu).size() == 2, QStringLiteral("the chooser offers both of the Mac's branches"));
        check(menu->activeAction() != nullptr && menu->activeAction()->isChecked(),
              QStringLiteral("the highlight starts on the primary branch"));
        check(Rig::chooseByKeys(menu, QStringLiteral("Mac first")), QStringLiteral("↓ reached the Mac's first branch"));
    }
    check(rig.disk(1) == QString(seeds.at(1)).insert(midAt, esc(QStringLiteral(" Mac first."))),
          QStringLiteral("choosing the Mac's FIRST branch redid it on Linux"));
    rig.undo();
    check(rig.disk(1) == seeds.at(1), QStringLiteral("undo returned to the branch point"));
    check(Rig::chooser() == nullptr, QStringLiteral("[I-0320] an undo landing on a branch point never opens the chooser"));

    // On the line where the branches part (the undo's caret can rest a line below it): the mark is beside it.
    rig.caretAt(midMarker);
    check(waitFor([&] { return rig.marks().size() == 1; }), QStringLiteral("on screen, ONE mark for the branch point's line"));
    if (!rig.marks().isEmpty()) {
        const QRect caret = rig.editor->cursorRect();
        const QRect mark(rig.marks().first()->mapTo(rig.editor, QPoint(0, 0)), rig.marks().first()->size());
        const int caretY = rig.editor->viewport()->mapTo(rig.editor, caret.center()).y();
        check(std::abs(mark.center().y() - caretY) <= caret.height(),
              QStringLiteral("the mark sits on the branch's line (mark y %1, line y %2)").arg(mark.center().y()).arg(caretY));
        check(mark.right() < rig.editor->viewport()->mapTo(rig.editor, QPoint(0, 0)).x(),
              QStringLiteral("the mark is in the gutter, left of the text"));
        check(rig.marks().first()->accessibleName() == QStringLiteral("Ctrl+Shift+Z will ask which branch to redo")
                  && !rig.marks().first()->toolTip().isEmpty(),
              QStringLiteral("the mark has a tooltip and an accessible name"));
    }
    check(!rig.editor->document()->toPlainText().contains(QStringLiteral("Ctrl+Shift+Z")),
          QStringLiteral("the mark never enters the text"));

    // ── 2. A branch made HERE: undo, then type — the chooser, the keys, Esc ──────────────────────────────────────────
    rig.caretAt(QStringLiteral("Alpha one."));
    rig.type(QStringLiteral(" Linux one."));
    rig.undo();
    rig.type(QStringLiteral(" Linux two."));
    rig.undo();
    check(rig.disk(0) == QStringLiteral("Alpha one."), QStringLiteral("back at the branch point in scene 0"));
    check(Rig::chooser() == nullptr, QStringLiteral("[I-0320] no chooser after the undo"));
    check(waitFor([&] { return rig.marks().size() == 1; }),
          QStringLiteral("ONE mark on screen: scene 0's two branches part on one line; the Mac's are off screen (%1)")
              .arg(rig.marks().size()));

    rig.redo();
    menu = Rig::chooser();
    check(menu != nullptr, QStringLiteral("Ctrl+Shift+Z opened the chooser"));
    if (menu != nullptr) {
        QTest::keyClick(menu, Qt::Key_Escape);
        Rig::settle();
    }
    check(Rig::chooser() == nullptr && rig.disk(0) == QStringLiteral("Alpha one."), QStringLiteral("Esc redid nothing"));

    rig.redo();
    menu = Rig::chooser();
    if (menu != nullptr) {
        check(menu->activeAction() != nullptr && menu->activeAction()->text().contains(QStringLiteral("Linux two")),
              QStringLiteral("the newest branch is primary, and highlighted first"));
        check(Rig::chooseByKeys(menu, QStringLiteral("Linux one")), QStringLiteral("↓ reached the older branch"));
    }
    check(rig.disk(0) == QStringLiteral("Alpha one.") + esc(QStringLiteral(" Linux one.")),
          QStringLiteral("choosing the older branch redid it (got: \"%1\")").arg(rig.disk(0)));
    rig.undo();
    check(rig.disk(0) == QStringLiteral("Alpha one."), QStringLiteral("undo back to the branch point"));

    // ── 3. The mark: hints off hides it; on shows it; a click opens the chooser ────────────────────────────────────────
    const QString before = rig.editor->document()->toPlainText();
    rig.shell->setMarkupHints(false);
    check(waitFor([&] { return rig.marks().isEmpty(); }), QStringLiteral("[T-0620] Q2: no mark with Markup Hints off"));
    rig.shell->setMarkupHints(true);
    check(waitFor([&] { return rig.marks().size() == 1; }), QStringLiteral("the mark is back with Markup Hints on"));
    check(rig.editor->document()->toPlainText() == before, QStringLiteral("toggling the marks changed no text"));
    check(rig.disk(0) == QStringLiteral("Alpha one."), QStringLiteral("toggling the marks wrote nothing to the scene"));
    if (!rig.marks().isEmpty()) {
        QTest::mouseClick(rig.marks().first(), Qt::LeftButton);
        Rig::settle();
        menu = Rig::chooser();
        check(menu != nullptr, QStringLiteral("[T-0620] Q3: a click on the mark opened the chooser"));
        if (menu != nullptr) {
            QTest::keyClick(menu, Qt::Key_Escape);
            Rig::settle();
        }
    }

    // ── 4. The History section: closed by default; fetched on a history change only; a click re-primaries ──────────────
    HistorySection* section = rig.history();
    check(!section->isExpanded() && section->fetches() == 0, QStringLiteral("[SP-204] Q1: History is closed and unfetched"));
    section->header()->click();
    check(waitFor([&] { return section->rows()->count() > 0; }), QStringLiteral("opened, History lists this scene's events"));
    const int fetched = section->fetches();
    rig.caretAt(QStringLiteral("Charlie three."));
    rig.caretAt(QStringLiteral("Alpha one."));
    check(section->fetches() == fetched, QStringLiteral("[I-0331] a scene change re-filtered, never re-fetched"));
    {
        int bold = 0;
        for (int i = 0; i < section->rows()->count(); ++i) {
            bold += section->rows()->item(i)->font().bold() ? 1 : 0;
        }
        check(bold == 1, QStringLiteral("[I-0106] ONE row is bold for the caret (%1)").arg(bold));
    }
    QListWidgetItem* older = nullptr;
    for (int i = 0; i < section->rows()->count(); ++i) {
        if (section->rows()->item(i)->text().contains(QStringLiteral("Linux two"))) {
            older = section->rows()->item(i);
        }
    }
    // After choosing "Linux one", "Linux two" is the branch OFF the line.
    check(older != nullptr && older->toolTip() == QStringLiteral("Switch to this branch"),
          QStringLiteral("the other branch is listed off the line"));
    if (older != nullptr) {
        section->rows()->setCurrentItem(older);
        emit section->rows()->itemClicked(older);
        check(waitFor([&] { return section->fetches() > fetched; }), QStringLiteral("re-primarying re-fetched the tree"));
    }
    rig.redo();
    menu = Rig::chooser();
    check(menu != nullptr && menu->activeAction() != nullptr && menu->activeAction()->text().contains(QStringLiteral("Linux two")),
          QStringLiteral("after the History click, the chooser's primary is the branch clicked"));
    if (menu != nullptr) {
        QTest::keyClick(menu, Qt::Key_Return);
        Rig::settle();
    }
    check(rig.disk(0) == QStringLiteral("Alpha one.") + esc(QStringLiteral(" Linux two.")),
          QStringLiteral("Return redid the branch History chose (got: \"%1\")").arg(rig.disk(0)));

    // ── 5. Project Settings ▸ Undo History ───────────────────────────────────────────────────────────────────────
    {
        ProjectSettingsDialog dlg(rig.shell->preferences(), rig.shell.get());
        check(waitFor([&] { return dlg.historyLoaded(); }), QStringLiteral("the Undo History settings were read"));
        const QVariantMap was = ScriviBridge::historyGetSettings(rig.projectDir);
        check(dlg.capacityBox()->value() == was.value(QStringLiteral("capacityEvents")).toInt()
                  && dlg.staleDaysBox()->value() == was.value(QStringLiteral("staleBranchDays")).toInt(),
              QStringLiteral("the dialog shows the core's values"));
        dlg.capacityBox()->setValue(12345);
        dlg.staleDaysBox()->setValue(3);
        dlg.done(QDialog::Rejected);   // Close
        check(waitFor([&] {
                  const QVariantMap now = ScriviBridge::historyGetSettings(rig.projectDir);
                  return now.value(QStringLiteral("capacityEvents")).toInt() == 12345
                         && now.value(QStringLiteral("staleBranchDays")).toInt() == 3;
              }),
              QStringLiteral("closing the dialog wrote both values"));
        check(ScriviBridge::historyGetSettings(rig.projectDir).value(QStringLiteral("idleRolloverHours"))
                  == was.value(QStringLiteral("idleRolloverHours")),
              QStringLiteral("idleRolloverHours kept"));
    }

    rig.close();

    // ── 6. Purge: a branch made stale (its record aged on disk, as weeks would), listed, purged only on confirmation ────
    {
        // Age every record of the abandoned "Linux one" branch: rewrite the timestamps in the log to 2020.
        QDir dir(rig.projectDir + QStringLiteral("/history"));
        int aged = 0;
        for (const QString& name : dir.entryList({QStringLiteral("*.jsonl"), QStringLiteral("*.json")}, QDir::Files)) {
            QFile f(dir.filePath(name));
            if (!f.open(QIODevice::ReadOnly)) {
                continue;
            }
            QString text = QString::fromUtf8(f.readAll());
            f.close();
            const QString was = text;
            text.replace(QRegularExpression(QStringLiteral("\"(timestamp|createdAt|tipTimestamp)\":\"20\\d\\d-")),
                         QStringLiteral("\"\\1\":\"2020-"));
            if (text != was && f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                f.write(text.toUtf8());
                f.close();
                ++aged;
            }
        }
        check(aged > 0, QStringLiteral("the history's records were aged (found %1 file(s))").arg(aged));
    }
    check(rig.load(), QStringLiteral("project re-loaded"));
    int answered = 0;
    QTimer driver;
    driver.setInterval(50);
    QObject::connect(&driver, &QTimer::timeout, [&] {
        auto* dlg = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dlg == nullptr) {
            return;
        }
        if (auto* confirm = qobject_cast<QMessageBox*>(dlg)) {
            ++answered;
            QAbstractButton* pick = nullptr;
            for (QAbstractButton* b : confirm->buttons()) {
                const bool purge = b->text().startsWith(QStringLiteral("Purge"));
                if ((answered == 1) != purge) {   // the first answer Cancels, the second Purges
                    pick = b;
                }
            }
            check(confirm->defaultButton() != nullptr && !confirm->defaultButton()->text().startsWith(QStringLiteral("Purge")),
                  QStringLiteral("Purge is never the default button"));
            if (pick != nullptr) {
                pick->click();
            }
            return;
        }
        if (dlg->objectName() != QStringLiteral("staleBranchesDialog")) {
            return;
        }
        auto* list = dlg->findChild<QListWidget*>(QStringLiteral("staleBranches"));
        auto* purge = dlg->findChild<QPushButton*>(QStringLiteral("purgeBranch"));
        if (list == nullptr || purge == nullptr || !purge->isEnabled()) {
            if (answered >= 2 && list != nullptr && list->count() == 1 && list->item(0)->flags() == Qt::NoItemFlags) {
                dlg->reject();   // purged: only "No stale branches." is left
            }
            return;
        }
        QTimer::singleShot(0, purge, &QPushButton::click);   // ⚠️ queued: the confirmation's exec must not run INSIDE this timer
    });
    driver.start();
    QTimer::singleShot(20000, [] {
        if (auto* d = QApplication::activeModalWidget()) {
            std::printf("FAIL: the purge dialog never finished (stuck on %s)\n", qUtf8Printable(d->objectName()));
            ++failures;
            static_cast<QDialog*>(d)->reject();
        }
    });
    const qsizetype staleBefore =
        ScriviBridge::historyListStaleBranches(rig.projectDir).value(QStringLiteral("branches")).toList().size();
    check(staleBefore >= 2, QStringLiteral("aged, the abandoned branches are stale (%1)").arg(staleBefore));
    rig.shell->purgeStaleBranches();
    driver.stop();
    check(answered >= 2, QStringLiteral("the purge asked first; Cancel kept it; Purge took it (answers: %1)").arg(answered));
    const QVariantMap stale = ScriviBridge::historyListStaleBranches(rig.projectDir);
    check(stale.value(QStringLiteral("branches")).toList().isEmpty(), QStringLiteral("nothing stale is left"));
    check(answered == staleBefore + 1, QStringLiteral("one Cancel, then one confirmation per branch (%1 answers, %2 branches)")
                                           .arg(answered).arg(staleBefore));
    check(rig.disk(0) == QStringLiteral("Alpha one.") + esc(QStringLiteral(" Linux two.")),
          QStringLiteral("the purge touched no scene text"));
    rig.close();

    if (failures == 0) {
        std::printf("branch-ok\n");
    }
    return failures == 0 ? 0 : 1;
}
