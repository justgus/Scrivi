// background_wait_smoke.cpp — SP-181: [I-0304] a failed save stays dirty, and [I-0305] a close or quit WAITS for the
// project's background writes, behind "Please wait — saving updates…".
//
// REAL project windows opened through `AppEnvironment` (as the app opens them); Replace All driven through the real find
// bar; every claim about text read back FROM DISK.
//
// ✅ What it proves (SP-181 AC7–AC8; EP-052 [SP-195]):
//   • a scene whose save FAILS stays dirty, and the core's held queue writes it once the folder is writable again — the
//     ordinary autosave and Replace All's saves alike ([I-0304], [SP-195] Q1 (a)); the editor is never locked for them;
//   • closing a window while the queue is held ASKS ([I-0322]): Cancel keeps it open, "Close Anyway" closes it;
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
#include <QEventLoop>
#include <QFileInfo>
#include <QLineEdit>
#include <QPointer>
#include <QProgressDialog>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QThreadPool>
#include <QTimer>

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

// Read from the FILE: a core read behind a held queue is refused ([SP-195] Q1 (a)), which is not what this asks.
QString disk(ScriviBridge&, const Project& p, int i)
{
    const QFileInfo fi(p.contentPaths.at(i));
    QFile f(fi.isAbsolute() ? fi.filePath() : p.dir + QLatin1Char('/') + p.contentPaths.at(i));
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
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

    // ── §1 [I-0304] A failed autosave stays dirty; the held queue writes it when it can ([SP-195] Q1 (a)) ───────────────
    // The core retries a held write every 5 s, so each "lands" wait allows 15 s.
    typeAtEnd(ea, QStringLiteral("Bravo"), QStringLiteral(" typed"));
    setWritable(sceneDir(A, 1), false);
    wa->editorShell()->saveDirtyScenes();
    check(waitFor([&] { return wa->editorShell()->writesHeld(); }), QStringLiteral("§1 the failed save holds the queue"));
    check(disk(bridge, A, 1) == QStringLiteral("Bravo"), QStringLiteral("§1 precondition: the save failed (%1)").arg(disk(bridge, A, 1)));
    check(wa->editorShell()->dirtySceneCount() == 1, QStringLiteral("§1 … the scene stays dirty"));
    check(!wa->editorShell()->backgroundWorkInProgress(), QStringLiteral("§1 … a close would not wait on a held queue"));
    check(!ea->isReadOnly(), QStringLiteral("§1 … the editor is not locked"));

    // ── §1b [I-0322] Closing while the queue is held ASKS — Cancel keeps the window ──────────────────────────────────
    int asked = 0;
    EditorShell::confirmHook = [&](int unsaved, const QString& action) {
        ++asked;
        check(unsaved == 1 && action == QStringLiteral("Close"), QStringLiteral("§1b asked about 1 scene, to Close (%1, %2)")
                                                                     .arg(unsaved).arg(action));
        return false;   // Cancel
    };
    wa->close();
    QApplication::processEvents();
    check(asked == 1 && wa != nullptr && wa->isVisible(), QStringLiteral("§1b Cancel kept the window open"));
    EditorShell::confirmHook = nullptr;

    setWritable(sceneDir(A, 1), true);
    check(waitFor([&] { return disk(bridge, A, 1) == QStringLiteral("Bravo typed"); }, 15000),
          QStringLiteral("§1 the held save landed once the folder was writable (%1)").arg(disk(bridge, A, 1)));
    check(waitFor([&] { return wa->editorShell()->dirtySceneCount() == 0 && !wa->editorShell()->writesHeld(); }),
          QStringLiteral("§1 … and the scene is clean"));

    // ── §2 [I-0304] Replace All's saves: a failed write stays dirty too, and lands when it can ───────────────────────
    setWritable(sceneDir(A, 0), false);
    FindBar* bar = startReplaceAll(wa, QStringLiteral("cat"), QStringLiteral("cow"));
    check(waitFor([&] { return bar->countText() == QStringLiteral("2 replaced"); }),
          QStringLiteral("§2 Replace All landed (%1)").arg(bar->countText()));
    check(!ea->isReadOnly(), QStringLiteral("§2 … and the editor is unlocked without waiting for its writes"));
    check(waitFor([&] { return wa->editorShell()->writesHeld(); }), QStringLiteral("§2 its failed write holds the queue"));
    check(disk(bridge, A, 0) == QStringLiteral("cat one"), QStringLiteral("§2 precondition: its writes failed (%1)").arg(disk(bridge, A, 0)));
    setWritable(sceneDir(A, 0), true);
    check(waitFor([&] { return disk(bridge, A, 0) == QStringLiteral("cow one") && disk(bridge, A, 2) == QStringLiteral("cow three"); },
                  15000),
          QStringLiteral("§2 the held writes landed once the folder was writable (%1 / %2)")
              .arg(disk(bridge, A, 0), disk(bridge, A, 2)));
    check(waitFor([&] { return !wa->editorShell()->writesHeld() && !wa->editorShell()->backgroundWorkInProgress(); }),
          QStringLiteral("§2 … and the queue is empty"));

    // ── §2b [SP-196] A structural edit that cannot reach the disk is HELD: the editor stays read-only (the job will
    //    run), a close does not wait on it, and it lands and is applied once the folder is writable again ─────────────────
    {
        const Project C = makeProject(bridge, appSupport, scratch + QStringLiteral("/c.scrivi"), {QStringLiteral("Alpha one. Alpha two.")});
        QPointer<ScriviWindow> wc = openWindow(env, C);
        ManuscriptEditor* ec = editorOf(wc);
        bool landedOk = false;
        bool landed = false;
        QObject::connect(wc->editorShell(), &EditorShell::structuralEditFinished, [&](bool ok) { landedOk = ok; landed = true; });
        const QTextCursor found = ec->document()->find(QStringLiteral("Alpha one. "));
        QTextCursor c = ec->textCursor();
        c.setPosition(found.selectionEnd());
        ec->setTextCursor(c);
        setWritable(sceneDir(C, 0), false);
        wc->editorShell()->splitScene();
        check(waitFor([&] { return wc->editorShell()->writesHeld(); }), QStringLiteral("§2b the split holds the queue"));
        check(!landed && ec->isReadOnly(), QStringLiteral("§2b … the editor stays read-only: the split will still run"));
        check(!wc->editorShell()->backgroundWorkInProgress(), QStringLiteral("§2b … a close would not wait on it"));
        setWritable(sceneDir(C, 0), true);
        check(waitFor([&] { return landed; }, 15000) && landedOk, QStringLiteral("§2b the held split landed once the folder was writable"));
        check(wc->editorShell()->sceneDocument().segments().size() == 2 && !ec->isReadOnly() && !wc->editorShell()->writesHeld(),
              QStringLiteral("§2b … applied: two scenes, the editor editable, nothing held"));
        wc->close();
        check(waitFor([&] { return wc == nullptr || !wc->isVisible(); }), QStringLiteral("§2b … and the window closes"));
    }

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

    // ── §5 [I-0309] A window destroyed while its project is still LOADING: the load's worker touches nothing it freed ──
    // ⚠️ A use-after-free rarely crashes on demand: this section is PROVEN under AddressSanitizer (SP-185), where the old
    // code's worker read the freed bridge (`ScriviBridge::getProjectSettings`). In a plain build it shows only that the
    // workers finish and the process survives them.
    {
        QStringList seeds;
        for (int i = 0; i < 30; ++i) {
            seeds << QStringLiteral("Scene %1 of the loading test.").arg(i);
        }
        const Project C = makeProject(bridge, appSupport, scratch + QStringLiteral("/c.scrivi"), seeds);
        for (int round = 0; round < 6; ++round) {
            auto* shell = new EditorShell;
            shell->load(C.dir, appSupport, QStringLiteral("Loading"));   // no envelope handed over: the worker opens it
            QTest::qWait(round * 4);                                    // a different moment in the worker each round
            delete shell;                                               // what a closed (WA_DeleteOnClose) window does
        }
        check(QThreadPool::globalInstance()->waitForDone(120000), QStringLiteral("§5 every load worker finished"));
        QApplication::processEvents();   // the relays' deferred deletes
    }

    // ── §6 [I-0310] A SHOWN, focused window torn down: the editor's focus-out must not read the document already freed ──
    {
        QStringList seeds{QStringLiteral("Shown and focused.")};
        const Project D = makeProject(bridge, appSupport, scratch + QStringLiteral("/d.scrivi"), seeds);
        auto* shell = new EditorShell;
        QEventLoop loop;
        QObject::connect(shell, &EditorShell::loadFinished, &loop, &QEventLoop::quit);
        shell->load(D.dir, appSupport, QStringLiteral("Shown"));
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        loop.exec();
        shell->show();
        shell->activateWindow();
        check(QTest::qWaitForWindowActive(shell), QStringLiteral("§6 precondition: the window is active"));
        auto* editor = shell->findChild<ManuscriptEditor*>();
        editor->setFocus();
        QApplication::processEvents();
        check(editor->hasFocus(), QStringLiteral("§6 precondition: the editor has the focus"));
        delete shell;   // ⛔ crashed in QPlainTextEdit::focusOutEvent before [I-0310]
        QApplication::processEvents();
    }

    QApplication::processEvents();
    if (failures == 0) {
        std::printf("background-wait-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
