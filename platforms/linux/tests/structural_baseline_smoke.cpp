// structural_baseline_smoke — EP-053 S2a (SP-187 / T-0615): what each structural operation costs on Linux TODAY.
//
// A REAL `EditorShell` loads a real project; each operation runs through the shell's own slot (or the real key dispatch
// for copy and paste). For each it prints the baseline S2b ([SP-191]) is measured against: core calls on the GUI thread,
// project loads (a structural reload is one), and full document builds. The manuscript is read back FROM DISK after each,
// and every scene's text is checked against the document (SCRIVI_VERIFY_SCENE_TEXT, set by the script).
// ⚠️ It asserts what the operations DO, not what they cost: the costs are the baseline, recorded in SP-187.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <cstdio>
#include <functional>
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

// A structural operation may ask first (a chapter split renumbers the chapters after it): answer yes.
void armConfirmer(QObject* owner)
{
    auto* t = new QTimer(owner);
    t->setInterval(20);
    QObject::connect(t, &QTimer::timeout, [] {
        if (auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            if (QAbstractButton* yes = box->button(QMessageBox::Yes)) { yes->click(); }
            else if (QAbstractButton* ok = box->button(QMessageBox::Ok)) { ok->click(); }
            else { box->accept(); }
        }
    });
    t->start();
}

struct Rig {
    ScriviBridge bridge;
    QString projectDir, appSupport, projectID;
    std::unique_ptr<EditorShell> shell;
    ManuscriptEditor* editor = nullptr;

    bool load()
    {
        shell = std::make_unique<EditorShell>();
        QEventLoop loop;
        bool ok = false;
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) { ok = r; loop.quit(); });
        shell->load(projectDir, appSupport, QStringLiteral("Baseline"));
        QTimer::singleShot(60000, &loop, &QEventLoop::quit);
        loop.exec();
        editor = shell->findChild<ManuscriptEditor*>();
        if (!ok || editor == nullptr) { return false; }
        shell->show();
        shell->activateWindow();
        QTest::qWaitForWindowActive(shell.get());
        editor->setFocus();
        QApplication::processEvents();
        return true;
    }
    QStringList texts()
    {
        shell->saveDirtyScenes();
        QStringList ids;
        for (const QVariant& v : bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList()) {
            ids.append(v.toMap().value(QStringLiteral("sceneID")).toString());
        }
        const QHash<QString, QString> t = bridge.readSceneTexts(projectDir, ids);
        QStringList out;
        for (const QString& id : ids) { out.append(t.value(id)); }
        return out;
    }
    int chapterCount()
    {
        QStringList seen;
        for (const QVariant& v : bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList()) {
            const QString c = v.toMap().value(QStringLiteral("chapterID")).toString();
            if (seen.isEmpty() || seen.last() != c) { seen.append(c); }
        }
        return seen.size();
    }
    void caretAt(const QString& marker, bool before = false)
    {
        const QTextCursor found = editor->document()->find(marker);
        QTextCursor c = editor->textCursor();
        c.setPosition(before ? found.selectionStart() : found.selectionEnd());
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void select(const QString& from, const QString& to)
    {
        const QTextCursor a = editor->document()->find(from);
        const QTextCursor b = editor->document()->find(to);
        QTextCursor c = editor->textCursor();
        c.setPosition(a.selectionStart());
        c.setPosition(b.selectionEnd(), QTextCursor::KeepAnchor);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void control(Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier | extra);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier | extra);
    }
    // Runs `act`, lets any background work and reload land, then prints the baseline.
    void measured(const char* what, const std::function<void()>& act)
    {
        const qint64 calls = ScriviBridge::guiThreadCallCount();
        const int loads = shell->loadCount();
        const int builds = shell->sceneDocument().builds();
        act();
        for (int i = 0; i < 300; ++i) {   // ≤ 15 s for a worker + reload to land
            QTest::qWait(50);
            if (!editor->isReadOnly()) { break; }
        }
        QTest::qWait(200);
        std::printf("[EP-053 baseline] %s: guiThreadCoreCalls=%lld projectLoads=%d documentBuilds=%d\n", what,
                    qlonglong(ScriviBridge::guiThreadCallCount() - calls), shell->loadCount() - loads,
                    shell->sceneDocument().builds() - builds);
        check(shell->sceneDocument().textMatchesDocument(), QStringLiteral("%1: every scene's text is its body").arg(what));
    }
};

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: structural_baseline_smoke <projectDir>\n");
        return 2;
    }
    Rig r;
    r.projectDir = QString::fromUtf8(argv[1]);
    r.appSupport = scrivi::linux_app::appSupportRoot();
    r.bridge.bootstrap(QStringLiteral("Baseline Tester"), r.appSupport);
    r.projectID = r.bridge.createProject(r.projectDir, r.appSupport, QStringLiteral("Baseline"), QStringLiteral("baseline"))
                      .value(QStringLiteral("projectID")).toString();
    if (r.projectID.isEmpty()) {
        std::printf("FAIL: no project\n");
        return 1;
    }
    // One chapter: "Alpha one. Alpha two." · "Beta body." · "Gamma body."
    const QVariantMap first = r.bridge.openProject(r.projectDir, r.appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
    auto save = [&](const QVariantMap& s, const QString& text) {
        r.bridge.saveScene(r.projectID, r.projectDir, r.appSupport, s.value(QStringLiteral("sceneID")).toString(),
                           s.value(QStringLiteral("metadataPath")).toString(), s.value(QStringLiteral("contentPath")).toString(),
                           text, 0, 0, 0.0);
    };
    save(first, QStringLiteral("Alpha one. Alpha two."));
    QVariantMap prev = first;
    for (const QString& body : {QStringLiteral("Beta body."), QStringLiteral("Gamma body.")}) {
        const QVariantMap made = r.bridge.createScene(r.projectDir, r.appSupport, r.projectID,
                                                      first.value(QStringLiteral("chapterID")).toString(),
                                                      prev.value(QStringLiteral("sceneID")).toString());
        save(made, body);
        prev = made;
    }
    if (!r.load()) {
        std::printf("FAIL: the project did not load\n");
        return 1;
    }
    armConfirmer(r.shell.get());

    r.measured("split scene", [&] { r.caretAt(QStringLiteral("Alpha one. ")); r.shell->splitScene(); });
    check(r.texts() == QStringList({QStringLiteral("Alpha one. "), QStringLiteral("Alpha two."), QStringLiteral("Beta body."),
                                    QStringLiteral("Gamma body.")}), QStringLiteral("split: the scenes on disk"));

    r.measured("merge scene", [&] { r.caretAt(QStringLiteral("Alpha two."), true); r.shell->mergeScene(); });
    check(r.texts().size() == 3, QStringLiteral("merge: three scenes on disk"));

    r.measured("split chapter", [&] { r.caretAt(QStringLiteral("Beta "), false); r.shell->splitChapter(); });
    check(r.chapterCount() == 2, QStringLiteral("split chapter: two chapters on disk"));

    r.measured("merge chapter", [&] {
        r.caretAt(QStringLiteral("body."), true);   // the start of chapter 2's first scene ("body." after the split)
        QTextCursor c = r.editor->textCursor();
        const int idx = r.shell->sceneDocument().sceneIndexForCaret(c.position());
        c.setPosition(r.shell->sceneDocument().segments().at(idx).bodyStart);
        r.editor->setTextCursor(c);
        r.shell->mergeChapter();
    });
    check(r.chapterCount() == 1, QStringLiteral("merge chapter: one chapter on disk"));

    r.measured("copy across scenes", [&] { r.select(QStringLiteral("one."), QStringLiteral("Beta")); r.control(Qt::Key_C); });
    const QStringList beforePaste = r.texts();
    r.measured("structured paste", [&] { r.caretAt(QStringLiteral("Gamma ")); r.control(Qt::Key_V); });
    check(r.texts().size() == beforePaste.size() + 1, QStringLiteral("structured paste: one scene more on disk"));
    r.measured("undo of the paste", [&] { r.control(Qt::Key_Z); });
    check(r.texts() == beforePaste, QStringLiteral("undo of the paste: the manuscript as it was before the paste"));

    r.shell->flushHistory();
    r.shell->saveDirtyScenes();
    r.shell->releaseProject();
    r.shell->hide();
    r.shell.reset();
    if (failures == 0) { std::printf("structural_baseline_smoke: all checks passed\n"); }
    return failures == 0 ? 0 : 1;
}
