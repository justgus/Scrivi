// world_warning_smoke.cpp — [SP-231] (EP-059 D, T-0497 + T-0693): on Linux a world is created from the Worlds dialog,
// Reconnect Worlds re-reads the project's worlds, and the world-warning strip says when one is away, as on the Mac.
//
// A REAL project window through `AppEnvironment`; the Worlds dialog through Project ▸ Manage Worlds… (`manageWorlds`).
//
// ✅ What it proves:
//   §1 D1 Create… makes `<folder>/<name>.inkwynworld`, bound and listed; a second one of the same name is refused;
//   §2 D3 a world moved away: the strip names it and the links held pending toward it;
//   §3 × hides it until ANOTHER world goes away; View ▸ Show World Warnings hides and shows it;
//   §4 D2 the worlds moved back + Reconnect Worlds (the dialog's button): the strip is gone;
//   §5 Q4 a broken imported timeline file is named in the Timeline panel's banner.
//
//   argv[1] = a scratch directory; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ManuscriptTypography.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"
#include "WorldWarningStrip.hpp"
#include "WorldsDialog.hpp"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTest>
#include <QTimer>

#include <cstdio>
#include <functional>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

void settle(EditorShell* shell)
{
    for (int i = 0; i < 500 && !shell->worldsRefreshSettled(); ++i) {
        QTest::qWait(10);
    }
    QTest::qWait(20);
    check(shell->worldsRefreshSettled(), QStringLiteral("the worlds read settled"));
}

// Project ▸ Manage Worlds… with `inDialog` run on the open dialog, which is then closed.
void withWorldsDialog(EditorShell* shell, const std::function<void(WorldsDialog*)>& inDialog)
{
    bool ran = false;
    QTimer::singleShot(100, [&]() {
        auto* dlg = qobject_cast<WorldsDialog*>(QApplication::activeModalWidget());
        check(dlg != nullptr, QStringLiteral("Manage Worlds… opened the Worlds dialog"));
        if (dlg != nullptr) {
            inDialog(dlg);
            ran = true;
            dlg->accept();
        }
    });
    shell->manageWorlds();
    check(ran, QStringLiteral("the dialog step ran"));
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: world_warning_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch + QStringLiteral("/worlds"));
    QDir().mkpath(scratch + QStringLiteral("/away"));
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);

    const QString dir = scratch + QStringLiteral("/worlds-test.inkwyn");
    const QString projectID = bridge.createProject(dir, appSupport, QStringLiteral("Worlds"), QStringLiteral("worlds"))
                                  .value(QStringLiteral("projectID")).toString();
    // §5's fixture: an imported timeline file that does not parse.
    QDir().mkpath(dir + QStringLiteral("/objects/imported-timelines"));
    {
        QFile broken(dir + QStringLiteral("/objects/imported-timelines/broken.scrivi-timeline.json"));
        check(broken.open(QIODevice::WriteOnly), QStringLiteral("wrote the broken timeline file"));
        broken.write("not a timeline");
    }

    AppEnvironment env(appSupport);
    ScriviWindow* w = env.openProjectWindow(dir, QStringLiteral("Worlds"), {}, projectID);
    for (int i = 0; i < 3000 && env.windows().window(projectID) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(projectID) == w, QStringLiteral("project window loaded"));
    w->resize(900, 700);
    w->raiseToFront();
    QTest::qWait(200);
    EditorShell* shell = w->editorShell();
    WorldWarningStrip* strip = shell->worldWarningStrip();
    settle(shell);
    check(strip != nullptr && !strip->isVisibleTo(shell), QStringLiteral("§0 no worlds: no strip"));

    // ── §1 D1: Create… ─────────────────────────────────────────────────────────────────────────────────────────────────
    const QString eskandar = scratch + QStringLiteral("/worlds/Eskandar.inkwynworld");
    withWorldsDialog(shell, [&](WorldsDialog* dlg) {
        auto* name = dlg->findChild<QLineEdit*>(QStringLiteral("newWorldName"));
        auto* create = dlg->findChild<QPushButton*>(QStringLiteral("createWorldButton"));
        check(name != nullptr && create != nullptr && !create->isEnabled(), QStringLiteral("§1 Create… waits for a name"));
        name->setText(QStringLiteral("Eskandar"));
        check(create->isEnabled(), QStringLiteral("§1 a name enables Create…"));
        check(dlg->createWorldIn(scratch + QStringLiteral("/worlds")), QStringLiteral("§1 the world was created"));
        check(name->text().isEmpty(), QStringLiteral("§1 the name field clears"));
        name->setText(QStringLiteral("Eskandar"));
        check(!dlg->createWorldIn(scratch + QStringLiteral("/worlds")), QStringLiteral("§1 a second Eskandar there is refused"));
        name->clear();
    });
    settle(shell);
    check(QFileInfo(eskandar).isDir(), QStringLiteral("§1 Eskandar.inkwynworld is on disk"));
    const QVariantList worlds = bridge.listWorlds(dir).value(QStringLiteral("worlds")).toList();
    check(worlds.size() == 1 && worlds.first().toMap().value(QStringLiteral("displayName")).toString() == QStringLiteral("Eskandar")
              && worlds.first().toMap().value(QStringLiteral("status")).toString() == QStringLiteral("available"),
          QStringLiteral("§1 the project binds it, available (%1 worlds)").arg(worlds.size()));
    const QString eskandarID = worlds.isEmpty() ? QString() : worlds.first().toMap().value(QStringLiteral("worldID")).toString();
    check(!strip->isVisibleTo(shell), QStringLiteral("§1 an available world: no strip"));

    // ── §2 D3: a world away ────────────────────────────────────────────────────────────────────────────────────────────
    // One link toward the world: a project source citing a world character.
    const QString charID = bridge.createObject(dir, QStringLiteral("character"), QStringLiteral("Edmond"),
                                               QStringLiteral("edmond"), eskandarID).value(QStringLiteral("objectID")).toString();
    const QString sourceID = bridge.createObject(dir, QStringLiteral("source"), QStringLiteral("Notes"),
                                                 QStringLiteral("notes"), QString()).value(QStringLiteral("objectID")).toString();
    check(!bridge.createEdge(dir, sourceID, charID, QStringLiteral("cites"), QString())
               .value(QStringLiteral("edgeID")).toString().isEmpty(),
          QStringLiteral("§2 a source cites a world character"));
    const QString eskandarAway = scratch + QStringLiteral("/away/Eskandar.inkwynworld");
    check(QDir().rename(eskandar, eskandarAway), QStringLiteral("§2 moved Eskandar away"));
    shell->refreshWorlds();
    settle(shell);
    check(strip->isVisibleTo(shell), QStringLiteral("§2 the strip shows"));
    check(strip->text().contains(QStringLiteral("1 world is unavailable")) && strip->text().contains(QStringLiteral("“Eskandar” is "))
              && strip->text().contains(QStringLiteral("1 link is held pending")),
          QStringLiteral("§2 it names the world and the link: %1").arg(strip->text()));
    check(strip->geometry().bottom() == strip->parentWidget()->height() - 1,
          QStringLiteral("§2 it lies along the manuscript's bottom edge"));

    // ── §3 ×, and View ▸ Show World Warnings ───────────────────────────────────────────────────────────────────────────
    strip->dismiss();
    check(!strip->isVisibleTo(shell), QStringLiteral("§3 × hides it"));
    shell->refreshWorlds();
    settle(shell);
    check(!strip->isVisibleTo(shell), QStringLiteral("§3 the same world away: it stays hidden"));
    const QString avalon = scratch + QStringLiteral("/worlds/Avalon.inkwynworld");
    const QString avalonAway = scratch + QStringLiteral("/away/Avalon.inkwynworld");
    bridge.createWorld(dir, avalon, QStringLiteral("Avalon"), QString());
    check(QDir().rename(avalon, avalonAway), QStringLiteral("§3 moved Avalon away"));
    shell->refreshWorlds();
    settle(shell);
    check(strip->isVisibleTo(shell) && strip->text().contains(QStringLiteral("2 worlds are unavailable"))
              && strip->text().contains(QStringLiteral("No links are affected yet")),
          QStringLiteral("§3 another world away: it returns: %1").arg(strip->text()));
    auto* toggle = w->findChild<QAction*>(QStringLiteral("showWorldWarningsAction"));
    check(toggle != nullptr && toggle->isChecked(), QStringLiteral("§3 View ▸ Show World Warnings is on"));
    if (toggle != nullptr) {
        toggle->trigger();
        check(!strip->isVisibleTo(shell) && !shell->worldWarningsVisible(), QStringLiteral("§3 unchecked: hidden"));
        toggle->trigger();
        check(strip->isVisibleTo(shell), QStringLiteral("§3 checked: shown again"));
    }

    // ── §4 D2: back, and Reconnect Worlds ──────────────────────────────────────────────────────────────────────────────
    check(QDir().rename(eskandarAway, eskandar) && QDir().rename(avalonAway, avalon), QStringLiteral("§4 moved both back"));
    withWorldsDialog(shell, [&](WorldsDialog* dlg) {
        auto* reconnect = dlg->findChild<QPushButton*>(QStringLiteral("reconnectWorldsButton"));
        check(reconnect != nullptr, QStringLiteral("§4 the dialog has Reconnect Worlds"));
        if (reconnect != nullptr) { reconnect->click(); }
    });
    settle(shell);
    check(!strip->isVisibleTo(shell), QStringLiteral("§4 Reconnect: the strip is gone: %1").arg(strip->text()));

    // ── §5 Q4: the broken imported timeline ────────────────────────────────────────────────────────────────────────────
    auto* banner = shell->findChild<QLabel*>(QStringLiteral("rejectedImportsBanner"));
    check(banner != nullptr && banner->isVisibleTo(shell)
              && banner->text().contains(QStringLiteral("1 imported timeline could not be loaded"))
              && banner->text().contains(QStringLiteral("broken.scrivi-timeline.json")),
          QStringLiteral("§5 the Timeline panel names the broken file: %1").arg(banner != nullptr ? banner->text() : QString()));

    w->close();
    QTest::qWait(100);
    if (failures == 0) {
        std::printf("world-warning-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
