// story_structure_smoke — headless story-structure persistence check for EP-025
// (SP-081, T-0332, AC4). No GUI.
//
// Exercises the story-structure deliverables through the bridge (the layer
// EditorShell's structure/assignment slots call), verified by reopen:
//   1. Set a built-in structure ("three-act") with its band layout
//      (setStoryStructure). Reopen: getStoryStructure reports hasStructure + the layout.
//   2. Assign a scene to a band (assignSceneToBand). Reopen: getSceneStoryTime reports
//      the scene's bandID.
//   3. Re-proportion the bands (updateBandLayout with edited proportions). Reopen: the
//      new proportions persisted.
//   4. Remove the structure (removeStoryStructure). Reopen: no structure, BUT the
//      scene's bandID assignment is PRESERVED (the AC4 contract).
//
//   argv[1] = project dir to create (e.g. <tmp>/story-structure.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include "AppSupport.hpp"
#include "ScriviBridge.hpp"
#include "StoryStructures.hpp"

namespace {

int failures = 0;
void check(bool cond, const char* what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    namespace story = scrivi::linux_app::story;

    if (argc < 2) {
        std::fprintf(stderr, "usage: story_structure_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Story Structure Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Story Structure Project"),
                             QStringLiteral("story-structure"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QString s0 =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap()
            .value(QStringLiteral("sceneID")).toString();
    check(!s0.isEmpty(), "fixture: first scene present");

    // Baseline: no structure.
    check(!bridge.getStoryStructure(projectPath)
               .value(QStringLiteral("hasStructure")).toBool(),
          "baseline: no structure");

    // --- 1. Set the three-act structure ---------------------------------------
    const story::Structure* threeAct = story::structureForID(QStringLiteral("three-act"));
    check(threeAct != nullptr && threeAct->bands.size() == 3, "three-act preset present");
    bridge.setStoryStructure(projectPath, QStringLiteral("three-act"),
                             story::bandLayoutJSON(threeAct->bands));

    opened = bridge.openProject(projectPath, appSupport);
    QVariantMap ss = bridge.getStoryStructure(projectPath);
    check(ss.value(QStringLiteral("hasStructure")).toBool(), "structure persisted");
    check(ss.value(QStringLiteral("structureID")).toString() == QStringLiteral("three-act"),
          "structureID persisted");
    check(story::parseBandLayout(ss.value(QStringLiteral("bandLayoutJSON")).toString()).size() == 3,
          "3 bands persisted");

    // --- 2. Assign s0 to act-ii -----------------------------------------------
    bridge.assignSceneToBand(projectPath, s0, QStringLiteral("act-ii"));
    opened = bridge.openProject(projectPath, appSupport);
    check(bridge.getSceneStoryTime(projectPath, s0)
              .value(QStringLiteral("bandID")).toString() == QStringLiteral("act-ii"),
          "scene assigned to act-ii persisted");

    // --- 3. Re-proportion the bands -------------------------------------------
    QList<story::Band> bands = threeAct->bands;
    bands[0].proportion = 0.5;   // Act I grows
    bands[1].proportion = 0.25;
    bands[2].proportion = 0.25;
    bridge.updateBandLayout(projectPath, story::bandLayoutJSON(bands));
    opened = bridge.openProject(projectPath, appSupport);
    ss = bridge.getStoryStructure(projectPath);
    const QList<story::Band> reloaded =
        story::parseBandLayout(ss.value(QStringLiteral("bandLayoutJSON")).toString());
    check(reloaded.size() == 3 && qAbs(reloaded.at(0).proportion - 0.5) < 1e-6,
          "re-proportioned band layout persisted (Act I = 0.5)");

    // --- 4. Remove the structure — assignment PRESERVED (AC4) -----------------
    bridge.removeStoryStructure(projectPath);
    opened = bridge.openProject(projectPath, appSupport);
    check(!bridge.getStoryStructure(projectPath)
               .value(QStringLiteral("hasStructure")).toBool(),
          "structure removed");
    check(bridge.getSceneStoryTime(projectPath, s0)
              .value(QStringLiteral("bandID")).toString() == QStringLiteral("act-ii"),
          "scene bandID PRESERVED after structure removal (AC4)");

    if (failures == 0) {
        std::printf("OK: set structure + assign + re-proportion + remove(keeps assignment) passed.\n");
        std::printf("projectID: %s\n", projectID.toUtf8().constData());
        std::printf("PASS: story-structure layout + assignment persisted; remove kept the assignment.\n");
    }
    return failures == 0 ? 0 : 1;
}
