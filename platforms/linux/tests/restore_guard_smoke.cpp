// restore_guard_smoke — EP-043 / SP-147 (T-0566/T-0567), the R6 guard + the AC4 skip.
//
// ⚠️ [I-0150] is what this pays for: on Apple, `xcodebuild test` LAUNCHED the app
// and rewrote a real project. ✅ Linux's guard lives in ONE place —
// `AppEnvironment::projectsToRestore()` — and this asserts it from the outside.
//
// ⛔ TWO THINGS ARE ASSERTED, and the second is the load-bearing one ([SP-147] AC6):
//   • under a guard signal, the restore funnel returns NOTHING;
//   • and `session.ini` is left BYTE-IDENTICAL — ⛔ a guard that cleared or
//     rewrote it would lose the writer's session on the first test run.
//
// ✅ Run THREE times by restore_guard_smoke.sh: once with NO guard signal (the
// negative control — ⚠️ it proves the funnel is not vacuously empty, so the
// suppressed passes mean something), then once per signal. [SP-147] AC7:
// ⛔ removing the guard must turn the suppressed passes RED.
//
// Usage: restore_guard_smoke <appSupportRoot> restore|suppressed
// Exit 0 on success; non-zero with a FAIL line.
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRect>
#include <cstdio>

#include "AppEnvironment.hpp"
#include "SessionStore.hpp"

namespace {
int failures = 0;
void ck(bool ok, const char* what)
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) { ++failures; }
}

QByteArray readAll(const QString& path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
}  // namespace

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    if (argc < 3) {
        std::fprintf(stderr, "usage: restore_guard_smoke <root> restore|suppressed\n");
        return 2;
    }
    const QString root       = QString::fromLocal8Bit(argv[1]);
    const bool    suppressed = QString::fromLocal8Bit(argv[2]) == QLatin1String("suppressed");

    // ---- Seed: A open at quit, B closed by the writer earlier, C open at quit
    //      but MOVED since (its path no longer resolves) ------------------------
    const QString pathA    = root + QStringLiteral("/a.scrivi");
    const QString pathGone = root + QStringLiteral("/moved-away.scrivi");
    QDir().mkpath(pathA);
    {
        SessionStore seed(root);
        seed.clearAll();
        SessionStore::Entry a;
        a.projectID  = QStringLiteral("proj-A");
        a.path       = pathA;
        a.frame      = QRect(40, 50, 1200, 800);
        a.paneSizes  = {240, 580, 400};
        a.outerSizes = {620, 120};
        seed.record(a);
        SessionStore::Entry b;
        b.projectID = QStringLiteral("proj-B");
        b.path      = QStringLiteral("/projects/b.scrivi");
        b.frame     = QRect(10, 10, 900, 700);
        seed.record(b);
        seed.setClosed(b.projectID);
        SessionStore::Entry c;
        c.projectID = QStringLiteral("proj-C");
        c.path      = pathGone;
        c.frame     = QRect(20, 20, 800, 600);
        seed.record(c);
    }

    AppEnvironment env(root);
    const QByteArray before = readAll(env.sessionStore().filePath());
    ck(!before.isEmpty(), "seeded session.ini exists");

    const QList<SessionStore::Entry> toRestore = env.projectsToRestore();

    if (suppressed) {
        // ⛔ [R6] THE GUARD.
        ck(toRestore.isEmpty(), "[R6] a guarded run restores NOTHING");
        // ⛔ [AC6] AND WRITES NOTHING.
        ck(readAll(env.sessionStore().filePath()) == before,
           "[AC6] session.ini is BYTE-IDENTICAL after a suppressed restore");
        ck(env.sessionStore().openProjectIDs().size() == 2,
           "[AC6] the open set SURVIVES suppression");
        ck(env.resolvableProjectsToRestore().isEmpty(),
           "[R6] the AC4 filter goes through the guard too");
    } else {
        // ✅ NEGATIVE CONTROL — without a signal the funnel DOES return the open set.
        ck(toRestore.size() == 2, "unguarded: the TWO projects open at quit, ⛔ not closed B");

        // ✅ [AC4] — skip what no longer resolves, ⛔ and keep its record.
        const QByteArray beforeFilter = readAll(env.sessionStore().filePath());
        const QList<SessionStore::Entry> resolvable = env.resolvableProjectsToRestore();
        ck(resolvable.size() == 1, "[AC4] only the project whose path RESOLVES is restored");
        ck(!resolvable.isEmpty() && resolvable.first().projectID == QLatin1String("proj-A"),
           "[AC4] ...and it is proj-A");
        ck(!resolvable.isEmpty() && resolvable.first().frame == QRect(40, 50, 1200, 800),
           "[AC5] its geometry comes with it");
        ck(!resolvable.isEmpty() && resolvable.first().paneSizes == QList<int>{240, 580, 400},
           "[AC5] ...and its splitter sizes");
        ck(readAll(env.sessionStore().filePath()) == beforeFilter,
           "[AC4][R-Q2] a SKIP writes nothing");
        ck(env.sessionStore().openProjectIDs().contains(QStringLiteral("proj-C"))
               && env.sessionStore().entry(QStringLiteral("proj-C")).frame == QRect(20, 20, 800, 600),
           "[R-Q2] the skipped project KEEPS its record and geometry");
    }

    // ---- A writer CLOSING a window (not quitting) ---------------------------
    // ✅ Independent of the guard: records the final state AND marks it closed.
    SessionStore::Entry closing = env.sessionStore().entry(QStringLiteral("proj-A"));
    closing.frame = QRect(60, 70, 1000, 700);
    env.projectWindowReleasing(closing);
    ck(!env.sessionStore().openProjectIDs().contains(QStringLiteral("proj-A")),
       "a window CLOSED by the writer is no longer open");
    ck(env.sessionStore().entry(QStringLiteral("proj-A")).frame == QRect(60, 70, 1000, 700),
       "[R-Q2] ...and its final geometry is KEPT");

    // ---- [I-0264] Wayland: a position the platform never reported is NOT recorded
    {
        const QRect prev(300, 200, 900, 600);
        const QRect wayland(0, 0, 1100, 700);    // what `geometry()` says on Wayland
        ck(AppEnvironment::recordableFrame(wayland, prev, /*positionKnown=*/false)
               == QRect(300, 200, 1100, 700),
           "[I-0264] Wayland: keep the STORED position, take the new SIZE");
        ck(AppEnvironment::recordableFrame(wayland, QRect(), false) == QRect(0, 0, 1100, 700),
           "[I-0264] Wayland, nothing stored: size only, at the origin");
        ck(AppEnvironment::recordableFrame(QRect(50, 60, 800, 500), prev, /*positionKnown=*/true)
               == QRect(50, 60, 800, 500),
           "[I-0264] X11: the real position IS recorded");
    }

    if (failures > 0) {
        std::fprintf(stderr, "FAIL: %d assertion(s)\n", failures);
        return 1;
    }
    return 0;
}
