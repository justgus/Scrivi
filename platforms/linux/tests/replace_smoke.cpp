// replace_smoke.cpp — SP-181 (EP-048 S4b, T-0608): Replace and Replace All over the presented text, on Linux.
//
// A REAL `EditorShell` loads a real project; the find bar is driven as the writer drives it (Ctrl+Alt+F through the real
// dispatch, its fields and buttons); every claim about the manuscript is read back FROM DISK.
//
// ✅ What it proves (SP-181 AC1–AC4):
//   • Find and Replace… (Ctrl+Alt+F) opens the bar WITH the Replace row; Find… (Ctrl+F) without;
//   • Replace types over the CURRENT match — escaped, in the match's style — then finds the next; with no current match it
//     only finds (Q2: "find first, then replace");
//   • Replace All returns at once with the editor locked — the work is off the GUI thread (Q3) — and when it lands every
//     match is replaced, escaped and balanced, as ONE undo step that Ctrl+Z / Ctrl+Shift+Z replay;
//   • only the scenes holding a match are written: an unmatched scene's file is untouched, and an unmatched scene with
//     unsaved typing is left to its own autosave.
// ⚠️ NOT covered (live pass only): the rig's network share, how long a large Replace All takes there, GNOME's bindings.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "FindBar.hpp"
#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <cstdio>
#include <memory>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

struct Scene { QString sceneID; QString contentPath; QString text; };

struct Rig {
    ScriviBridge bridge;
    QString projectDir;
    QString appSupport;
    QString projectID;
    std::unique_ptr<EditorShell> shell;
    ManuscriptEditor* editor = nullptr;
    FindBar* bar = nullptr;

    bool load()
    {
        shell = std::make_unique<EditorShell>();
        QEventLoop loop;
        bool ok = false;
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) { ok = r; loop.quit(); });
        shell->load(projectDir, appSupport, QStringLiteral("Replace"));
        QTimer::singleShot(60000, &loop, &QEventLoop::quit);
        loop.exec();
        editor = shell->findChild<ManuscriptEditor*>();
        bar = shell->findChild<FindBar*>();
        if (!ok || editor == nullptr || bar == nullptr) {
            return false;
        }
        // Edit ▸ Find, as `ScriviWindow` builds it — its shortcuts live in the window.
        auto* menu = new QMenu(shell.get());
        shell->addActions(addFindMenu(menu, shell.get(), [this](FindCommand c) { shell->find(c); }));
        shell->show();
        shell->activateWindow();
        check(QTest::qWaitForWindowActive(shell.get()), QStringLiteral("the window became active"));
        editor->setFocus();
        QApplication::processEvents();
        return true;
    }
    QList<Scene> manuscript()
    {
        QList<Scene> out;
        QStringList ids;
        for (const QVariant& v : bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList()) {
            const QVariantMap s = v.toMap();
            out.append({s.value(QStringLiteral("sceneID")).toString(), s.value(QStringLiteral("contentPath")).toString(), {}});
            ids.append(out.last().sceneID);
        }
        const QHash<QString, QString> texts = bridge.readSceneTexts(projectDir, ids);
        for (Scene& s : out) {
            s.text = texts.value(s.sceneID);
        }
        return out;
    }
    // On disk, WITHOUT saving first — what the Replace All itself wrote.
    QStringList disk()
    {
        QStringList t;
        for (const Scene& s : manuscript()) {
            t.append(s.text);
        }
        return t;
    }
    // On disk after the autosave has had its say.
    QStringList saved()
    {
        shell->saveDirtyScenes();
        return disk();
    }
    QDateTime modified(const QString& contentPath) const
    {
        const QFileInfo fi(contentPath);
        return (fi.isAbsolute() ? fi : QFileInfo(projectDir + QLatin1Char('/') + contentPath)).lastModified();
    }
    void caretAt(const QString& marker)
    {
        const QTextCursor found = editor->document()->find(marker);
        QTextCursor c = editor->textCursor();
        c.setPosition(found.selectionEnd());
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    QString selected() const { return editor->textCursor().selectedText(); }
    void control(Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier | extra);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier | extra);
        settle();
    }
    void search(const QString& query, const QString& replacement)
    {
        bar->queryField()->setText(query);
        emit bar->queryField()->textEdited(query);   // the incremental search, as typing it does
        bar->replaceField()->setText(replacement);
        settle();
        for (int i = 0; i < 6000 && !bar->searchSettled(); ++i) {   // [T-0613]: after the typing pause
            QTest::qWait(10);
        }
    }
    void press(const QString& button)
    {
        for (QPushButton* b : bar->findChildren<QPushButton*>()) {
            if (b->text() == button) {
                QTest::mouseClick(b, Qt::LeftButton);
                settle();
                return;
            }
        }
        check(false, QStringLiteral("the bar has a \"%1\" button").arg(button));
    }
    // Waits for Replace All to land (the bar reports it).
    bool landed(const QString& count)
    {
        for (int i = 0; i < 500 && bar->countText() != count; ++i) {
            QTest::qWait(10);
        }
        return bar->countText() == count;
    }
    static void settle() { QTest::qWait(60); }
};

QString shown(const QStringList& l) { return QStringLiteral("[\"%1\"]").arg(l.join(QStringLiteral("\", \""))); }

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: replace_smoke <projectDir>\n");
        return 2;
    }

    Rig rig;
    rig.projectDir = QString::fromUtf8(argv[1]);
    rig.appSupport = scrivi::linux_app::appSupportRoot();
    rig.bridge.bootstrap(QStringLiteral("Scrivi"), rig.appSupport);
    rig.projectID = rig.bridge.createProject(rig.projectDir, rig.appSupport, QStringLiteral("Replace"),
                                             QStringLiteral("replace")).value(QStringLiteral("projectID")).toString();
    check(!rig.projectID.isEmpty(), QStringLiteral("project created"));

    const QStringList seeds{QStringLiteral("a **Smith** b and Smith c"), QStringLiteral("Bravo"),
                            QStringLiteral("cat and **cat** too"), QStringLiteral("Delta"),
                            QStringLiteral("## cat")};   // a heading: Replace All of its first word keeps the `## `
    {
        auto scenes = [&] { return rig.bridge.openProject(rig.projectDir, rig.appSupport).value(QStringLiteral("scenes")).toList(); };
        for (int i = 1; i < seeds.size(); ++i) {
            const QVariantMap last = scenes().last().toMap();
            rig.bridge.createScene(rig.projectDir, rig.appSupport, rig.projectID, last.value(QStringLiteral("chapterID")).toString(),
                                   last.value(QStringLiteral("sceneID")).toString());
        }
        const QVariantList all = scenes();
        check(all.size() == seeds.size(), QStringLiteral("five scenes seeded (got %1)").arg(all.size()));
        for (int i = 0; i < all.size() && i < seeds.size(); ++i) {
            const QVariantMap s = all.at(i).toMap();
            rig.bridge.saveScene(rig.projectID, rig.projectDir, rig.appSupport, s.value(QStringLiteral("sceneID")).toString(),
                                 s.value(QStringLiteral("metadataPath")).toString(),
                                 s.value(QStringLiteral("contentPath")).toString(), seeds.at(i), 0, 0, 0.0);
        }
        rig.bridge.closeProject(rig.projectDir);
    }
    check(rig.disk() == seeds, QStringLiteral("seeded: %1").arg(shown(rig.disk())));
    if (!rig.load()) {
        std::printf("FAIL: the project did not load\n");
        return 1;
    }

    // ── §1 The menu: Ctrl+F without the Replace row, Ctrl+Alt+F with it (SP-176 Q2) ───────────────────────────────────
    rig.control(Qt::Key_F);
    check(rig.bar->isVisible() && !rig.bar->replaceRowShown(), QStringLiteral("§1 Ctrl+F: the bar, no Replace row"));
    QTest::keyClick(rig.editor, Qt::Key_F, Qt::ControlModifier | Qt::AltModifier);
    Rig::settle();
    check(rig.bar->isVisible() && rig.bar->replaceRowShown(), QStringLiteral("§1 Ctrl+Alt+F: the bar WITH the Replace row"));

    // ── §2 Replace: the current match only, typed over (escaped, the match's style), then the next (Q2) ──────────────
    rig.caretAt(QStringLiteral("Delta"));   // the search starts from here and wraps
    rig.search(QStringLiteral("Smith"), QStringLiteral("J. Jones"));
    check(rig.bar->countText() == QStringLiteral("1 of 2"), QStringLiteral("§2 the first Smith is current (%1)").arg(rig.bar->countText()));
    rig.press(QStringLiteral("Replace"));
    check(rig.saved().at(0) == QStringLiteral(R"(a **J\. Jones** b and Smith c)"),
          QStringLiteral("§2 Replace: escaped and bold, the other Smith untouched: %1").arg(rig.disk().at(0)));
    check(rig.selected() == QStringLiteral("Smith"), QStringLiteral("§2 then the next match is selected (%1)").arg(rig.selected()));
    // With no current match, Replace only finds.
    rig.caretAt(QStringLiteral("a **"));
    rig.press(QStringLiteral("Replace"));
    check(rig.saved().at(0) == QStringLiteral(R"(a **J\. Jones** b and Smith c)"), QStringLiteral("§2 a caret, not a match: nothing replaced"));
    check(rig.selected() == QStringLiteral("Smith"), QStringLiteral("§2 … the next match is selected instead"));

    // ── §3 Replace All: off the GUI thread, ONE step, only the matched scenes written (Q3) ──────────────────────────────
    const QList<Scene> before = rig.manuscript();
    const QDateTime bravoAt = rig.modified(before.at(1).contentPath);
    QTest::qWait(20);       // so a rewrite of Bravo's file could not share its old timestamp
    rig.search(QStringLiteral("cat"), QStringLiteral("d*g"));
    // ⚠️ AFTER the search: moving the caret to another scene flushes every dirty scene (the T-0239 cadence), which is
    // the ordinary autosave, not the Replace All.
    rig.caretAt(QStringLiteral("Delta"));
    QTest::keyClicks(rig.editor, QStringLiteral(" Pre"));   // "Delta Pre": unsaved, and holds no match
    Rig::settle();
    rig.bar->replaceAll();
    check(rig.editor->isReadOnly(), QStringLiteral("§3 Replace All returns at once, the editor locked: the work is off the GUI thread"));
    check(!rig.editor->document()->find(QStringLiteral("cat")).isNull(),
          QStringLiteral("§3 … the replacements are not even computed yet when it returns (they are the worker's)"));
    check(rig.landed(QStringLiteral("3 replaced")), QStringLiteral("§3 Replace All landed (%1)").arg(rig.bar->countText()));
    check(!rig.editor->isReadOnly(), QStringLiteral("§3 … and the editor is unlocked"));
    const QStringList written = rig.disk();   // what the Replace All itself wrote — no autosave yet
    check(written.at(2) == QStringLiteral(R"(d\*g and **d\*g** too)") && written.at(4) == QStringLiteral(R"(## d\*g)"),
          QStringLiteral("§3 every match replaced, escaped, the bold one bold: %1").arg(shown(written)));
    check(written.at(3) == QStringLiteral("Delta"), QStringLiteral("§3 the unmatched scene with unsaved typing was NOT written (%1)").arg(written.at(3)));
    check(rig.modified(before.at(1).contentPath) == bravoAt, QStringLiteral("§3 the unmatched clean scene's file is untouched"));
    const QStringList after{QStringLiteral(R"(a **J\. Jones** b and Smith c)"), QStringLiteral("Bravo"),
                            QStringLiteral(R"(d\*g and **d\*g** too)"), QStringLiteral("Delta Pre"), QStringLiteral(R"(## d\*g)")};
    check(rig.saved() == after, QStringLiteral("§3 after the autosave: %1").arg(shown(rig.disk())));

    // ── §4 ONE undo step: Ctrl+Z restores every replaced scene, Ctrl+Shift+Z re-applies it ─────────────────────────────
    rig.editor->setFocus();
    rig.control(Qt::Key_Z);
    const QStringList undone{after.at(0), after.at(1), QStringLiteral("cat and **cat** too"), after.at(3), QStringLiteral("## cat")};
    check(rig.saved() == undone, QStringLiteral("§4 one Ctrl+Z undoes the whole Replace All: %1").arg(shown(rig.disk())));
    rig.control(Qt::Key_Z, Qt::ShiftModifier);
    check(rig.saved() == after, QStringLiteral("§4 Ctrl+Shift+Z re-applies it: %1").arg(shown(rig.disk())));

    rig.shell->flushHistory();
    rig.shell->saveDirtyScenes();
    rig.shell->releaseProject();
    rig.shell->hide();   // ⚠️ as the app does: a SHOWN shell destroyed takes a focus-out on a document already gone
    rig.shell.reset();
    if (failures == 0) {
        std::printf("replace-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
