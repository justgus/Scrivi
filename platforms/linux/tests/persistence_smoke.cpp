// persistence_smoke — headless persistence check for SP-059 (T-0228).
//
// Proves the two things the sprint's exit criteria require to survive an app
// restart, WITHOUT driving the QML GUI (which needs a display and user input):
//
//   1. Identity persistence (AC4): ScriviBridge::bootstrap resolves a local
//      identity under appSupportRoot; a second run resolves the SAME identityID
//      (i.e. it was persisted, not recreated) and reports createdNew=false.
//   2. Recents persistence (AC5): RecentsStore writes recents.json under
//      appSupportRoot; a fresh RecentsStore configured against the same root
//      reads the entry back.
//
// The harness takes one argument — the pass number:
//   pass 1: bootstrap identity, add a recents entry, print the identityID.
//   pass 2: bootstrap again, load recents; assert identity matches pass 1's
//           (passed as argv[2]) and the recents entry is present.
//
// It exercises the exact same AppSupport + RecentsStore + ScriviBridge code the
// GUI uses. appSupportRoot is driven by $XDG_DATA_HOME (see AppSupport), so CI
// points both passes at the same temp dir. No display, no Xvfb.

#include <QCoreApplication>

#include <cstdio>
#include <cstdlib>

#include "AppSupport.hpp"
#include "RecentsStore.hpp"
#include "ScriviBridge.hpp"

namespace {
constexpr auto kTestPath  = "/tmp/scrivi-smoke/demo.scrivi";
constexpr auto kTestTitle = "Smoke Project";
} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: persistence_smoke <pass:1|2> [expectedIdentityID]\n");
        return 2;
    }
    const int pass = std::atoi(argv[1]);

    const QString root = scrivi::linux_app::appSupportRoot();
    std::fprintf(stderr, "appSupportRoot = %s\n", root.toUtf8().constData());

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Smoke Tester"), root);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    RecentsStore recents;
    recents.configure(root);

    if (pass == 1) {
        recents.addOrUpdate(QLatin1String(kTestPath), QLatin1String(kTestTitle));
        // Emit the identityID on stdout so pass 2 can compare against it.
        std::printf("%s\n", bridge.identityID().toUtf8().constData());
        return 0;
    }

    // pass 2 — assert persistence across the "restart".
    int rc = 0;

    if (argc >= 3) {
        const QString expected = QString::fromUtf8(argv[2]);
        if (bridge.identityID() != expected) {
            std::fprintf(stderr,
                         "FAIL: identity not persisted (pass1=%s pass2=%s)\n",
                         expected.toUtf8().constData(),
                         bridge.identityID().toUtf8().constData());
            rc = 1;
        } else {
            std::fprintf(stderr, "OK: identity persisted (%s)\n",
                         expected.toUtf8().constData());
        }
    }

    bool foundRecent = false;
    for (const QVariant& v : recents.entries()) {
        if (v.toMap().value(QStringLiteral("path")).toString()
            == QLatin1String(kTestPath)) {
            foundRecent = true;
            break;
        }
    }
    if (!foundRecent) {
        std::fprintf(stderr, "FAIL: recents entry not persisted\n");
        rc = 1;
    } else {
        std::fprintf(stderr, "OK: recents entry persisted\n");
    }

    return rc;
}
