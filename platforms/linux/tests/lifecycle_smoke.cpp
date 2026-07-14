// lifecycle_smoke — headless full-loop check for SP-060 (T-0233).
//
// Proves the create → open → close → reopen-recent loop the sprint requires,
// WITHOUT driving the QML GUI. It exercises the same ScriviBridge + RecentsStore
// code paths the app uses (createProject / openProject / recents), against a temp
// $XDG_DATA_HOME + a temp project directory.
//
//   pass 1: bootstrap → createProject → openProject (assert mode=="ready",
//           projectID matches create, >=1 scene) → addOrUpdate recents →
//           "close" (drop local state) → reopen from the recents entry (assert
//           same projectID, still ready). Prints the projectID.
//   pass 2 <expectedProjectID>: fresh process (simulated app restart) → the
//           recents entry persists → reopen it (assert mode ready + same
//           projectID as pass 1). Proves the loop survives a restart.
//
// $XDG_DATA_HOME drives appSupportRoot (see AppSupport); the project dir is
// passed as argv[2] (pass 1) so both passes point at the same on-disk .scrivi.

#include <QCoreApplication>

#include <cstdio>
#include <cstdlib>

#include "AppSupport.hpp"
#include "RecentsStore.hpp"
#include "ScriviBridge.hpp"

namespace {
constexpr auto kTitle = "Lifecycle Project";

// Reopens `path` and checks it comes back ready with the expected projectID.
// Returns 0 on success, 1 on failure (logging the reason).
int assertReopen(ScriviBridge& bridge, const QString& appSupport,
                 const QString& path, const QString& expectedProjectID)
{
    const QVariantMap r = bridge.openProject(path, appSupport);
    if (r.value(QStringLiteral("mode")).toString() != QStringLiteral("ready")) {
        std::fprintf(stderr, "FAIL: reopen mode != ready (mode=%s)\n",
                     r.value(QStringLiteral("mode")).toString().toUtf8().constData());
        return 1;
    }
    const QString pid = r.value(QStringLiteral("projectID")).toString();
    if (pid != expectedProjectID) {
        std::fprintf(stderr, "FAIL: reopen projectID mismatch (%s != %s)\n",
                     pid.toUtf8().constData(),
                     expectedProjectID.toUtf8().constData());
        return 1;
    }
    return 0;
}
} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: lifecycle_smoke <pass:1|2> "
                             "<projectDir(pass1) | expectedProjectID(pass2)>\n");
        return 2;
    }
    const int pass = std::atoi(argv[1]);

    const QString appSupport = scrivi::linux_app::appSupportRoot();
    std::fprintf(stderr, "appSupportRoot = %s\n", appSupport.toUtf8().constData());

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Lifecycle Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    RecentsStore recents;
    recents.configure(appSupport);

    if (pass == 1) {
        if (argc < 3) {
            std::fprintf(stderr, "pass 1 needs a project dir argument\n");
            return 2;
        }
        const QString projectPath = QString::fromUtf8(argv[2]);

        // --- create ---
        const QVariantMap created =
            bridge.createProject(projectPath, appSupport,
                                 QLatin1String(kTitle), QStringLiteral("lifecycle-project"));
        const QString createID = created.value(QStringLiteral("projectID")).toString();
        if (createID.isEmpty()) {
            std::fprintf(stderr, "FAIL: create returned no projectID\n");
            return 1;
        }
        std::fprintf(stderr, "OK: created %s\n", createID.toUtf8().constData());

        // --- open (ready) ---
        const QVariantMap opened = bridge.openProject(projectPath, appSupport);
        if (opened.value(QStringLiteral("mode")).toString() != QStringLiteral("ready")) {
            std::fprintf(stderr, "FAIL: open mode != ready\n");
            return 1;
        }
        if (opened.value(QStringLiteral("projectID")).toString() != createID) {
            std::fprintf(stderr, "FAIL: open projectID != create projectID\n");
            return 1;
        }
        const int scenes = opened.value(QStringLiteral("scenes")).toList().size();
        if (scenes < 1) {
            std::fprintf(stderr, "FAIL: opened project has no scenes\n");
            return 1;
        }
        std::fprintf(stderr, "OK: opened ready with %d scene(s)\n", scenes);

        // --- record in recents, then "close" (drop local state) ---
        recents.addOrUpdate(projectPath, QLatin1String(kTitle));

        // --- reopen from the recents entry (same process) ---
        if (recents.entries().isEmpty()) {
            std::fprintf(stderr, "FAIL: recents empty after open\n");
            return 1;
        }
        const QString recentPath =
            recents.entries().first().toMap().value(QStringLiteral("path")).toString();
        if (assertReopen(bridge, appSupport, recentPath, createID) != 0) {
            return 1;
        }
        std::fprintf(stderr, "OK: reopened from recents\n");

        // Emit the projectID so pass 2 can compare across a restart.
        std::printf("%s\n", createID.toUtf8().constData());
        return 0;
    }

    // pass 2 — simulated restart: the recents entry must persist and reopen ready.
    if (argc < 3) {
        std::fprintf(stderr, "pass 2 needs the expected projectID argument\n");
        return 2;
    }
    const QString expectedID = QString::fromUtf8(argv[2]);

    if (recents.entries().isEmpty()) {
        std::fprintf(stderr, "FAIL: recents did not persist across restart\n");
        return 1;
    }
    const QString recentPath =
        recents.entries().first().toMap().value(QStringLiteral("path")).toString();
    if (assertReopen(bridge, appSupport, recentPath, expectedID) != 0) {
        return 1;
    }
    std::fprintf(stderr, "OK: reopened recent after restart (%s)\n",
                 expectedID.toUtf8().constData());
    return 0;
}
