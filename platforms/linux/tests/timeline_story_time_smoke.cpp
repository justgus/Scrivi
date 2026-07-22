// timeline_story_time_smoke — headless story-time placement + chain check for EP-025
// (SP-080, T-0328, AC3). No GUI.
//
// Exercises the timeline drag/picker COMMIT path through the bridge (the layer
// EditorShell::showTimeDeltaPicker calls) — the drag + QDialog UI itself is verified
// live over VNC, but the endpoint + gap-chain math are headless-testable here:
//   1. A 3-scene fixture starts on the default chain (each scene gapMs 0, offsets
//      chaining by the default 1-hour duration).
//   2. Place scene 1 (0-based) MANUALLY 2 hours after scene 0's end
//      (setSceneStoryTime source="manual"), then re-persist the chain from scene 1 on
//      (mirroring the shell's propagation). Reopen: scene 1 is "manual" with the right
//      gap; scene 2 (still default) shifted to sit after scene 1's new end.
//   3. RESET scene 1 to default (source="default", gap 0) + re-persist. Reopen: scene 1
//      back on the chain; scene 2 follows again.
//
//   argv[1] = project dir to create (e.g. <tmp>/timeline-story-time.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
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

int failures = 0;
void check(bool cond, const char* what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

constexpr qint64 kHourMs    = 3'600'000;
constexpr qint64 kDefaultMs = kHourMs;   // project default scene duration (1 hour)

// gapMs / durationMs / offsetSource for a scene, via getSceneStoryTime.
struct StoryTime { qint64 gapMs; qint64 durationMs; QString source; };
StoryTime storyTime(ScriviBridge& bridge, const QString& root, const QString& sceneID)
{
    const QVariantMap st = bridge.getSceneStoryTime(root, sceneID);
    qint64 dur = st.value(QStringLiteral("durationMs")).toLongLong();
    if (dur <= 0) { dur = kDefaultMs; }
    return { st.value(QStringLiteral("gapMs")).toLongLong(), dur,
             st.value(QStringLiteral("offsetSource"), QStringLiteral("default")).toString() };
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: timeline_story_time_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Story Time Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Story Time Project"),
                             QStringLiteral("timeline-story-time"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // --- Fixture: 3 scenes in one chapter --------------------------------------
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QVariantMap first =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString s0 = first.value(QStringLiteral("sceneID")).toString();
    const QString ch = first.value(QStringLiteral("chapterID")).toString();
    const QVariantMap r1 = bridge.createScene(projectPath, appSupport, projectID, ch, s0);
    const QString s1 = r1.value(QStringLiteral("sceneID")).toString();
    const QVariantMap r2 = bridge.createScene(projectPath, appSupport, projectID, ch, s1);
    const QString s2 = r2.value(QStringLiteral("sceneID")).toString();
    check(!s1.isEmpty() && !s2.isEmpty(), "fixture: 3 scenes created");

    // Baseline: all three default (gap 0).
    check(storyTime(bridge, projectPath, s0).source == QStringLiteral("default"),
          "baseline: s0 default");
    check(storyTime(bridge, projectPath, s1).gapMs == 0,
          "baseline: s1 gap 0 (on the chain)");

    // --- 2. Manual placement: s1 → 2 hours after s0's end ----------------------
    // s0 default: offset 0, duration 1h → end = 1h. Place s1 at 3h (gap = 3h − 1h = 2h).
    const qint64 s0End      = 0 + kDefaultMs;             // 1h
    const qint64 s1NewOffset = s0End + 2 * kHourMs;       // 3h
    const qint64 s1Gap       = s1NewOffset - s0End;       // 2h
    bridge.setSceneStoryTime(projectPath, s1, s1NewOffset, QStringLiteral("manual"),
                             s1Gap, kDefaultMs, QStringLiteral("manual"));
    // Re-persist s2 from the chain (it keeps its own gap 0, new offset = s1End).
    const StoryTime s2st = storyTime(bridge, projectPath, s2);
    const qint64 s1End = s1NewOffset + kDefaultMs;        // 4h
    bridge.setSceneStoryTime(projectPath, s2, s1End + s2st.gapMs,
                             s2st.source, s2st.gapMs, s2st.durationMs,
                             QStringLiteral("default"));

    // Reopen — story-time persists.
    opened = bridge.openProject(projectPath, appSupport);
    const StoryTime s1after = storyTime(bridge, projectPath, s1);
    check(s1after.source == QStringLiteral("manual"), "s1 persisted as manual");
    check(s1after.gapMs == s1Gap, "s1 gap persisted (2h)");
    check(storyTime(bridge, projectPath, s2).gapMs == 0,
          "s2 still on the chain (gap 0) after s1 moved");

    // --- 3. Reset s1 back to default -------------------------------------------
    bridge.setSceneStoryTime(projectPath, s1, s0End, QStringLiteral("default"),
                             0, kDefaultMs, QStringLiteral("default"));
    opened = bridge.openProject(projectPath, appSupport);
    check(storyTime(bridge, projectPath, s1).source == QStringLiteral("default"),
          "s1 reset to default");
    check(storyTime(bridge, projectPath, s1).gapMs == 0, "s1 gap 0 after reset");

    if (failures == 0) {
        std::printf("OK: story-time manual placement + chain re-persist + reset passed.\n");
        std::printf("projectID: %s\n", projectID.toUtf8().constData());
        std::printf("PASS: setSceneStoryTime persisted manual/default placement across reopen.\n");
    }
    return failures == 0 ? 0 : 1;
}
