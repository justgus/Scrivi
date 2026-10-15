// background_wait_smoke.cpp — SP-181: [I-0304] a failed save stays dirty, and [I-0305] a close or quit WAITS for the
// project's background writes, behind "Please wait — saving updates…".
//
// REAL project windows opened through `AppEnvironment` (as the app opens them); Replace All driven through the real find
// bar; every claim about text read back FROM DISK.
//
// ✅ What it proves (SP-181 AC7–AC8):
//   • a scene whose save FAILS stays dirty, so the next save writes it — the ordinary autosave and Replace All's off-main
//     save alike ([I-0304]);
//   • closing a window while Replace All's writes run leaves it open behind the wait dialog, and it closes by itself, with
//     the writes on disk, when they land ([I-0305]);
//   • Quit does the same for every window, behind ONE dialog.
// ⚠️ The failed save is made by a read-only scene directory, so this must run as a NON-ROOT user (root ignores it).
//
//   argv[1] = a scratch directory for the projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "FindBar.hpp"
#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QPointer>
#include <QProgressDialog>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>

#include <cstdio>
#include <unistd.h>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

void settle() { QTest::qWait(60); }

struct Project {
    QString dir;
    QString id;
    QStringList sceneIDs;
    QStringList contentPaths;
};

Project makeProject(ScriviBridge& bridge, const QString& appSupport, const QString& dir, const QStringList& seeds)
{
    Project p;
    p.dir = dir;
    p.id = bridge.createProject(dir, appSupport, QStringLiteral("Wait"), QStringLiteral("wait"))
               .value(QStringLiteral("projectID")).toString();
    QVariantList scenes = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList();
    const QString chapterID = scenes.first().toMap().value(QStringLiteral("chapterID")).toString();
    for (int i = 1; i < seeds.size(); ++i) {
        bridge.createScene(dir, appSupport, p.id, chapterID, scenes.last().toMap().value(QStringLiteral("sceneID")).toString());
        scenes = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList();
    }
    for (int i = 0; i < seeds.size(); ++i) {
        const QVariantMap s = scenes.at(i).toMap();
        p.sceneIDs.append(s.value(QStringLiteral("sceneID")).toString());
        p.contentPaths.append(s.value(QStringLiteral("contentPath")).toString());
        bridge.saveScene(p.id, dir, appSupport, p.sceneIDs.last(), s.value(QStringLiteral("metadataPath")).toString(),
                         p.contentPaths.last(), seeds.at(i), 0, 0, 0.0);
    }
    bridge.closeProject(dir);
    return p;
}

ScriviWindow* openWindow(AppEnvironment& env, const Project& p)
{
    ScriviWindow* w = env.openProjectWindow(p.dir, QStringLiteral("Wait"), {}, p.id);
    for (int i = 0; i < 3000 && env.windows().window(p.id) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(p.id) == w, QStringLiteral("project window loaded: %1").arg(p.dir));
    return w;
}

ManuscriptEditor* editorOf(ScriviWindow* w) { return w->editorShell()->findChild<ManuscriptEditor*>(); }

QString disk(ScriviBridge& bridge, const Project& p, int i)
{
    return bridge.readSceneTexts(p.dir, {p.sceneIDs.at(i)}).value(p.sceneIDs.at(i));
}

// The directory holding scene `i`'s file: made read-only, every write into it fails.
QString sceneDir(const Project& p, int i)
{
    const QFileInfo fi(p.contentPaths.at(i));
    return (fi.isAbsolute() ? fi : QFileInfo(p.dir + QLatin1Char('/') + p.contentPaths.at(i))).absolutePath();
}
void setWritable(const QString& dir, bool on)
{
    QFile::Permissions perms = QFile::ReadOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup;
    if (on) {
        perms |= QFile::WriteOwner;
    }
    QFile::setPermissions(dir, perms);
}

void typeAtEnd(ManuscriptEditor* e, const QString& marker, const QString& typed)
{
    const QTextCursor found = e->document()->find(marker);
    QTextCursor c = e->textCursor();
    c.setPosition(found.selectionEnd());
    e->setTextCursor(c);
    e->setFocus();
    QTest::keyClicks(e, typed);
    settle();
}

// Starts a Replace All through the find bar; it returns with the work in flight.
FindBar* startReplaceAll(ScriviWindow* w, const QString& query, const QString& replacement)
{
    w->editorShell()->find(FindCommand::ShowFindAndReplace);
    auto* bar = w->editorShell()->findChild<FindBar*>();
    bar->queryField()->setText(query);
    emit bar->queryField()->textEdited(query);
    bar->replaceField()->setText(replacement);
    settle();
    bar->replaceAll();
    return bar;
}
bool waitFor(const std::function<bool()>& cond, int ms = 10000)
{
    for (int i = 0; i < ms / 10 && !cond(); ++i) {
        QTest::qWait(10);
    }
    return cond();
}

QProgressDialog* waitDialog()
{
    for (QWidget* w : QApplication::topLevelWidgets()) {
        if (auto* d = qobject_cast<QProgressDialog*>(w); d != nullptr && d->isVisible()
                                                         && d->objectName() == QStringLiteral("savingUpdatesDialog")) {
            return d;
        }
    }
    return nullptr;
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: background_wait_smoke <scratchDir>\n");
        return 2;
    }
    if (geteuid() == 0) {
        std::printf("FAIL: run as a non-root user — root writes into a read-only directory, so no save can be made to fail\n");
        return 1;
    }

    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const Project A = makeProject(bridge, appSupport, scratch + QStringLiteral("/a.scrivi"),
                                  {QStringLiteral("cat one"), QStringLiteral("Bravo"), QStringLiteral("cat three")});
    const Project B = makeProject(bridge, appSupport, scratch + QStringLiteral("/b.scrivi"), {QStringLiteral("cat b")});

    AppEnvironment env(appSupport);
    QPointer<ScriviWindow> wa = openWindow(env, A);
    wa->raiseToFront();
    ManuscriptEditor* ea = editorOf(wa);

    // ── §1 [I-0304] A failed autosave stays dirty; the next save writes it ─────────────────────────────────────────────
    typeAtEnd(ea, QStringLiteral("Bravo"), QStringLiteral(" typed"));
    setWritable(sceneDir(A, 1), false);
    wa->editorShell()->saveDirtyScenes();
    check(disk(bridge, A, 1) == QStringLiteral("Bravo"), QStringLiteral("§1 precondition: the save failed (%1)").arg(disk(bridge, A, 1)));
    setWritable(sceneDir(A, 1), true);
    wa->editorShell()->saveDirtyScenes();
    check(disk(bridge, A, 1) == QStringLiteral("Bravo typed"),
          QStringLiteral("§1 the scene stayed dirty, so the next save wrote it (%1)").arg(disk(bridge, A, 1)));

    // ── §2 [I-0304] Replace All's off-main save: a failed write stays dirty too ────────────────────────────────────────
    setWritable(sceneDir(A, 0), false);
    FindBar* bar = startReplaceAll(wa, QStringLiteral("cat"), QStringLiteral("cow"));
    check(waitFor([&] { return bar->countText() == QStringLiteral("2 replaced"); }),
          QStringLiteral("§2 Replace All landed (%1)").arg(bar->countText()));
    check(disk(bridge, A, 0) == QStringLiteral("cat one"), QStringLiteral("§2 precondition: its writes failed (%1)").arg(disk(bridge, A, 0)));
    setWritable(sceneDir(A, 0), true);
    wa->editorShell()->saveDirtyScenes();
    check(disk(bridge, A, 0) == QStringLiteral("cow one") && disk(bridge, A, 2) == QStringLiteral("cow three"),
          QStringLiteral("§2 the failed scenes stayed dirty, so the next save wrote them (%1 / %2)")
              .arg(disk(bridge, A, 0), disk(bridge, A, 2)));

    // ── §3 [I-0305] Closing the window while Replace All's writes run WAITS, then closes ───────────────────────────────
    startReplaceAll(wa, QStringLiteral("cow"), QStringLiteral("dog"));
    check(wa->editorShell()->backgroundWorkInProgress(), QStringLiteral("§3 precondition: the Replace All is in flight"));
    wa->close();
    QApplication::processEvents();
    check(wa != nullptr && wa->isVisible(), QStringLiteral("§3 the window did NOT close while the work ran"));
    QProgressDialog* d = waitDialog();
    check(d != nullptr && d->labelText() == QStringLiteral("Please wait — saving updates…"),
          QStringLiteral("§3 … the wait dialog shows instead"));
    check(waitFor([&] { return wa == nullptr; }), QStringLiteral("§3 … and the window closed by itself when the work landed"));
    check(waitDialog() == nullptr, QStringLiteral("§3 … the dialog went with it"));
    check(disk(bridge, A, 0) == QStringLiteral("dog one") && disk(bridge, A, 2) == QStringLiteral("dog three"),
          QStringLiteral("§3 the writes are on disk (%1 / %2)").arg(disk(bridge, A, 0), disk(bridge, A, 2)));

    // ── §4 [I-0305] Quit waits for every window's work, behind ONE dialog ──────────────────────────────────────────────
    QPointer<ScriviWindow> wa2 = openWindow(env, A);
    QPointer<ScriviWindow> wb = openWindow(env, B);
    startReplaceAll(wb, QStringLiteral("cat"), QStringLiteral("dog"));
    check(wb->editorShell()->backgroundWorkInProgress(), QStringLiteral("§4 precondition: B's Replace All is in flight"));
    env.quitApplication();
    QApplication::processEvents();
    check(wa2 != nullptr && wb != nullptr && wb->isVisible(), QStringLiteral("§4 Quit closed no window while the work ran"));
    int dialogs = 0;
    for (QWidget* w : QApplication::topLevelWidgets()) {
        dialogs += (w->objectName() == QStringLiteral("savingUpdatesDialog") && w->isVisible()) ? 1 : 0;
    }
    check(dialogs == 1, QStringLiteral("§4 … ONE wait dialog shows (%1)").arg(dialogs));
    check(waitFor([&] { return wa2 == nullptr && wb == nullptr; }), QStringLiteral("§4 … and every window closed when it landed"));
    check(disk(bridge, B, 0) == QStringLiteral("dog b"), QStringLiteral("§4 B's writes are on disk (%1)").arg(disk(bridge, B, 0)));

    QApplication::processEvents();
    if (failures == 0) {
        std::printf("background-wait-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
