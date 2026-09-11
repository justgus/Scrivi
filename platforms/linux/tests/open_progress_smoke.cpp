// open_progress_smoke.cpp — SP-128 / T-0500 ([I-0195]) — project-open progress.
//
// ⚠️ WHAT THIS PROVES, and what it deliberately does not.
//
// ✅ It proves the progress reported by a REAL `EditorShell::load()` is
// DETERMINATE: a known total, arriving BEFORE the per-scene reads, advancing one
// scene at a time, ending exactly at the total. ⚠️ That is the claim SP-128 §2a
// rules on -- `files read / files to read` is a COUNT, never an estimate -- and
// it is the difference between a progress bar and a spinner.
//
// ⚠️ It does NOT prove the UI stays responsive under a SLOW mount. That needs a
// real `cache=none` share and a human watching, which is T-0501's live pass.
// ⚠️ Docker has no slow mount, so a green run here is NOT evidence about the
// defect [I-0195] was filed for -- only about the arithmetic underneath the fix.

#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ScriviBridge.hpp"

#include <QApplication>
#include <QEventLoop>
#include <QTimer>

#include <cstdio>
#include <vector>

namespace {

int failures = 0;

void check(bool cond, const char* what)
{
    if (!cond) { std::printf("FAIL: %s\n", what); ++failures; }
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: open_progress_smoke <projectDir>\n");
        return 2;
    }
    const QString projectDir = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: bridge did not bootstrap\n");
        return 1;
    }

    // A project with SEVERAL scenes -- one scene would make the fraction trivial
    // and would not exercise the loop that actually costs the time.
    const QVariantMap created = bridge.createProject(
        projectDir, appSupport, QStringLiteral("Open Progress"),
        QStringLiteral("open-progress"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: could not create the project\n");
        return 1;
    }

    // Add scenes so the load has real per-scene reads to report on.
    constexpr int kExtraScenes = 4;
    const QVariantMap opened0 = bridge.openProject(projectDir, appSupport);
    const QVariantList scenes0 = opened0.value(QStringLiteral("scenes")).toList();
    QString chapterID;
    if (!scenes0.isEmpty()) {
        chapterID = scenes0.first().toMap().value(QStringLiteral("chapterID")).toString();
    }
    for (int i = 0; i < kExtraScenes; ++i) {
        // Append after the last scene: createScene takes an afterSceneID.
        const QVariantMap now = bridge.openProject(projectDir, appSupport);
        const QVariantList list = now.value(QStringLiteral("scenes")).toList();
        const QString afterID =
            list.isEmpty() ? QString()
                           : list.last().toMap().value(QStringLiteral("sceneID")).toString();
        bridge.createScene(projectDir, appSupport, projectID, chapterID, afterID);
    }

    const QVariantMap opened = bridge.openProject(projectDir, appSupport);
    const int expectedTotal = opened.value(QStringLiteral("scenes")).toList().size();
    std::printf("scenes in project: %d\n", expectedTotal);
    check(expectedTotal >= 2, "the fixture must have MORE THAN ONE scene");

    // ⚠️ EditorShell OWNS its own bootstrapped bridge (EditorShell.cpp:79), so
    // nothing is injected here -- the project simply has to exist on disk, which
    // the bridge above has just ensured.
    EditorShell shell;

    std::vector<QPair<int, int>> seen;
    QObject::connect(&shell, &EditorShell::loadProgress, &shell,
                     [&seen](int done, int total) { seen.emplace_back(done, total); });

    QEventLoop loop;
    bool finishedOk = false;
    QObject::connect(&shell, &EditorShell::loadFinished, &shell,
                     [&](bool ok) { finishedOk = ok; loop.quit(); });

    shell.load(projectDir, appSupport, QStringLiteral("Open Progress"));

    // The load is asynchronous by construction (T-0499); wait for it.
    QTimer::singleShot(60000, &loop, &QEventLoop::quit);
    loop.exec();

    check(finishedOk, "the project loaded");
    check(!seen.empty(), "progress was reported at all");

    if (!seen.empty()) {
        // ✅ THE TOTAL IS KNOWN FROM THE FIRST REPORT -- that is what makes it
        // determinate. A spinner has no total; an estimate would change.
        const int total = seen.front().second;
        check(total == expectedTotal, "the FIRST report already carries the real total");

        bool totalStable = true;
        bool monotonic   = true;
        int last = -1;
        for (const auto& p : seen) {
            if (p.second != total) { totalStable = false; }
            if (p.first < last)    { monotonic = false; }
            last = p.first;
        }
        // ⚠️ A total that moves is an ESTIMATE, and an estimate that grows is how
        // a "progress" bar ends up going backwards in front of a writer.
        check(totalStable, "the total NEVER changes mid-load");
        check(monotonic,   "progress never goes BACKWARDS");

        check(seen.front().first == 0, "progress starts at 0");
        check(seen.back().first == total, "progress ENDS at the total, not short of it");
        // One report before the loop plus one per scene.
        check(static_cast<int>(seen.size()) == total + 1,
              "one report per scene, plus the initial zero");
    }

    if (failures == 0) {
        std::printf("PASS: open progress is determinate — known total, "
                    "monotonic, ends exactly at the total.\n");
    }
    return failures == 0 ? 0 : 1;
}
