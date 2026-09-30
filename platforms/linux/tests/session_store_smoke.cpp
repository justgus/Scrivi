// session_store_smoke — EP-043 / SP-147 (T-0565).
//
// ⚠️ `SessionStore` is the file that decides what comes back after a quit, so its
// round-trip is worth pinning independently of any window code. ✅ It owns no
// widgets, which is exactly why this can run with no display.
//
// ⛔ TWO BEHAVIOURS HERE ARE RULINGS, NOT CONVENIENCES, and each is asserted:
//   • [R-Q2] closing a project must NOT discard its geometry — a project closed
//     today reopens where the writer left it tomorrow.
//   • an empty projectID must NOT be recorded — a failed load has no identity and
//     must not become a phantom row the next restore tries to reopen.
//
// Exit 0 on success; non-zero with a FAIL line.
#include <QCoreApplication>
#include <QDir>
#include <QRect>
#include <cstdio>

#include "SessionStore.hpp"

namespace {
int failures = 0;
void ck(bool ok, const char* what)
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) { ++failures; }
}
}  // namespace

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    const QString root = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::tempPath();
    SessionStore store(root);
    store.clearAll();

    ck(store.entries().isEmpty(),        "starts empty");
    ck(store.openProjectIDs().isEmpty(), "no open projects at start");

    SessionStore::Entry a;
    a.projectID  = QStringLiteral("PROJ-A");
    a.path       = QStringLiteral("/tmp/a.scrivi");
    a.title      = QStringLiteral("Project A");
    a.frame      = QRect(100, 120, 1220, 760);
    a.maximized  = false;
    a.paneSizes  = {240, 580, 400};
    a.outerSizes = {620, 120};
    store.record(a);

    const SessionStore::Entry got = store.entry(QStringLiteral("PROJ-A"));
    ck(got.projectID == a.projectID,   "projectID round-trips");
    ck(got.path      == a.path,        "path round-trips (an ATTRIBUTE, not the key)");
    ck(got.title     == a.title,       "title round-trips (T-0567)");
    ck(got.frame     == a.frame,       "frame round-trips");
    ck(got.maximized == false,         "maximized round-trips");
    ck(got.paneSizes  == a.paneSizes,  "3-pane splitter sizes round-trip");
    ck(got.outerSizes == a.outerSizes, "outer splitter sizes round-trip");
    ck(store.openProjectIDs().contains(a.projectID), "recorded project reads back OPEN");

    // A second project, maximized.
    SessionStore::Entry b;
    b.projectID = QStringLiteral("PROJ-B");
    b.path      = QStringLiteral("/tmp/b.scrivi");
    b.maximized = true;
    store.record(b);
    ck(store.entries().size() == 2,                 "two projects recorded");
    ck(store.openProjectIDs().size() == 2,          "both read back OPEN");
    ck(store.entry(b.projectID).maximized == true,  "maximized=true round-trips");
    ck(!store.entry(b.projectID).frame.isValid(),
       "an unstored frame reads back INVALID (caller keeps defaults)");

    // ⛔ [R-Q2] — THE RULING: closing must not discard geometry.
    store.setClosed(a.projectID);
    ck(!store.openProjectIDs().contains(a.projectID), "closed project is no longer OPEN");
    const SessionStore::Entry afterClose = store.entry(a.projectID);
    ck(afterClose.frame      == a.frame,      "[R-Q2] geometry SURVIVES a close");
    ck(afterClose.paneSizes  == a.paneSizes,  "[R-Q2] splitter sizes SURVIVE a close");
    ck(afterClose.path       == a.path,       "[R-Q2] path survives a close");

    // Reopening it restores the OPEN flag without needing the geometry re-supplied.
    SessionStore::Entry reopen = afterClose;
    store.record(reopen);
    ck(store.openProjectIDs().contains(a.projectID), "re-recording marks it OPEN again");
    ck(store.entry(a.projectID).frame == a.frame,    "geometry still intact after reopen");

    // ⚠️ T-0566 `setOpen` — marks OPEN and touches NOTHING else. ⛔ `record()`
    // would write a fresh window's `maximized=false` over the stored state.
    store.record(b);   // B is maximized, from above
    store.setClosed(b.projectID);
    store.setOpen(b.projectID, QStringLiteral("/tmp/b-moved.scrivi"), QString());
    ck(store.openProjectIDs().contains(b.projectID), "setOpen marks a closed project OPEN");
    ck(store.entry(b.projectID).maximized == true,   "setOpen does NOT clobber maximized");
    ck(store.entry(b.projectID).path == QLatin1String("/tmp/b-moved.scrivi"),
       "setOpen updates the path ([R-Q2]: a moved project keeps its record)");

    // ⛔ An empty projectID must never be recorded.
    SessionStore::Entry empty;
    empty.path = QStringLiteral("/tmp/nameless.scrivi");
    store.record(empty);
    ck(store.entries().size() == 2, "an EMPTY projectID is NOT recorded (no phantom row)");

    // A fresh store over the same root sees the persisted state — the "restart" case.
    SessionStore reread(root);
    ck(reread.entries().size() == 2,                    "a FRESH store reads the same file");
    ck(reread.openProjectIDs().contains(a.projectID),   "open set survives a new instance");
    ck(reread.entry(a.projectID).paneSizes == a.paneSizes,
       "splitter sizes survive a new instance (the RESTART case)");

    store.clearAll();
    ck(store.entries().isEmpty(), "clearAll empties the file");

    std::printf(failures == 0 ? "\nALL PASS\n" : "\n%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
