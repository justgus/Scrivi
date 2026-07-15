// scene_save_smoke — headless per-scene save check for SP-062 (T-0239, EP-022 AC3).
//
// Proves the write→save→reopen loop at the bridge level WITHOUT the GUI: create a
// project, open it, save the active scene's body with new Markdown via the same
// ScriviBridge::saveScene the editor's auto-save uses, then REOPEN and assert the
// new bytes came back (both via scrivi_open_project's activeScene.markdown and via
// a fresh scrivi_open_scene). Confirms scrivi_save_scene wrote the real .md.
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-save.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot (see AppSupport).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QCoreApplication>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include "AppSupport.hpp"
#include "ScriviBridge.hpp"

namespace {
constexpr auto kNewBody = "Rewritten body — the quick brown fox.\n\nSecond paragraph.";
} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_save_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Save Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    // --- create + open ----------------------------------------------------
    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Save Project"),
                             QStringLiteral("scene-save"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    if (opened.value(QStringLiteral("mode")).toString() != QStringLiteral("ready")) {
        std::fprintf(stderr, "FAIL: open not ready\n");
        return 1;
    }
    const QVariantList scenes = opened.value(QStringLiteral("scenes")).toList();
    if (scenes.isEmpty()) {
        std::fprintf(stderr, "FAIL: no scenes to save into\n");
        return 1;
    }
    const QVariantMap scene = scenes.first().toMap();
    const QString sceneID      = scene.value(QStringLiteral("sceneID")).toString();
    const QString metadataPath = scene.value(QStringLiteral("metadataPath")).toString();
    const QString contentPath  = scene.value(QStringLiteral("contentPath")).toString();

    // --- save a new body + surface state via the editor's save path -------
    // Non-zero scene-local caret + scroll fraction exercise the T-0247 (SP-064)
    // restore round-trip: these must come back from open_project's restored{}.
    constexpr long long kAnchor = 7;
    constexpr long long kFocus  = 12;
    constexpr double    kScroll = 0.375;
    const QVariantMap saveResult = bridge.saveScene(
        projectID, projectPath, appSupport, sceneID, metadataPath, contentPath,
        QString::fromUtf8(kNewBody), kAnchor, kFocus, kScroll);
    if (!saveResult.value(QStringLiteral("saved")).toBool()) {
        std::fprintf(stderr, "FAIL: saveScene did not report saved=true\n");
        return 1;
    }

    // --- reopen and assert the new bytes came back ------------------------
    // (a) via open_scene (reads the scene's .md fresh).
    const QVariantMap reread =
        bridge.openScene(projectPath, appSupport, projectID, sceneID);
    if (reread.value(QStringLiteral("markdown")).toString()
        != QString::fromUtf8(kNewBody)) {
        std::fprintf(stderr, "FAIL: open_scene markdown != saved body\n"
                             "  got: %s\n",
                     reread.value(QStringLiteral("markdown")).toString()
                         .toUtf8().constData());
        return 1;
    }

    // (b) via a full reopen of the project (activeScene should carry the new body,
    //     since the saved scene is the only/active one).
    opened = bridge.openProject(projectPath, appSupport);
    const QString activeMd = opened.value(QStringLiteral("activeScene")).toMap()
                                 .value(QStringLiteral("markdown")).toString();
    if (activeMd != QString::fromUtf8(kNewBody)) {
        std::fprintf(stderr, "FAIL: reopened activeScene markdown != saved body\n");
        return 1;
    }

    // (c) surface state round-trip (T-0247/SP-064): the saved scene-local caret +
    //     scroll must come back through open_project's restored{} for the active scene.
    const QVariantMap restored = opened.value(QStringLiteral("restored")).toMap();
    if (restored.value(QStringLiteral("anchor")).toLongLong() != kAnchor
        || restored.value(QStringLiteral("focus")).toLongLong() != kFocus) {
        std::fprintf(stderr,
                     "FAIL: restored caret != saved (anchor %lld/%lld, focus %lld/%lld)\n",
                     restored.value(QStringLiteral("anchor")).toLongLong(), kAnchor,
                     restored.value(QStringLiteral("focus")).toLongLong(), kFocus);
        return 1;
    }
    if (qAbs(restored.value(QStringLiteral("scroll")).toDouble() - kScroll) > 1e-6) {
        std::fprintf(stderr, "FAIL: restored scroll %f != saved %f\n",
                     restored.value(QStringLiteral("scroll")).toDouble(), kScroll);
        return 1;
    }

    std::fprintf(stderr, "OK: edit → saveScene → reopen shows the new bytes + restored "
                         "caret/scroll on disk.\n");
    std::printf("%s\n", projectID.toUtf8().constData());
    return 0;
}
