// inspector_layout_smoke — EP-035 / SP-126 (T-0486): `inspector-layout.json`
// PATCHED, never reconstructed.
//
// ⚠️ **THIS IS THE SPRINT'S DATA-LOSS TEST**, and it is the one that matters
// most. Linux does not model `stackSort`, `defaultStacks` or per-scene `scenes`.
// ⚠️ **If the store ever serialises a typed struct instead of patching the
// decoded document, every one of those keys is DELETED** — and the loss is
// invisible until the writer reopens the project on the Mac and finds her card
// layout gone.
//
// The fixture below is a REAL Apple-written layout shape, taken from
// `the-lone-golem.scrivi` (35 world objects, 4 scenes with per-scene stacks).
//
// What is asserted:
//   • an Apple-written document round-trips with EVERY key intact after a
//     `selectedTab` change — including keys this build has never heard of;
//   • `selectedTab` actually persists;
//   • an UNKNOWN tab degrades to `writing` rather than failing the load;
//   • a MISSING file yields Apple's ruled defaults rather than an error;
//   • ⚠️ a CORRUPT file is NOT overwritten — the damaged layout is left for a
//     human, because destroying the evidence is worse than losing the layout.
//
//   argv[1] = a scratch directory to use as a project root
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <cstdio>

#include "InspectorLayoutStore.hpp"
#include "ScriviBridge.hpp"

namespace {

int failures = 0;
int checks   = 0;

void check(bool cond, const char* what)
{
    ++checks;
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

// An Apple-written layout, including a key this build does not model at all
// (`futureKeyFromANewerScrivi`) to prove unknown keys survive too.
const char* kAppleLayout = R"JSON({
  "schema": "scrivi.inspector-layout.v1",
  "selectedTab": "worldbuilding",
  "inspectorHidden": false,
  "defaultStacks": {
    "worldbuilding": [],
    "writing": [
      { "collapsed": false, "type": "tags" },
      { "collapsed": true,  "type": "outline" },
      { "collapsed": false, "type": "todo" }
    ]
  },
  "stackSort": { "worldbuilding": "name", "writing": "manual" },
  "scenes": {
    "scene_abc": {
      "worldbuilding": [
        { "collapsed": false, "type": "objects.characters" },
        { "collapsed": true,  "type": "objects.locations" }
      ]
    }
  },
  "futureKeyFromANewerScrivi": { "nested": [1, 2, 3] }
})JSON";

QString writeFile(const QString& dir, const char* body)
{
    QDir().mkpath(dir);
    const QString path = QDir(dir).filePath(QStringLiteral("inspector-layout.json"));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) { return {}; }
    f.write(body);
    f.close();
    return path;
}

QJsonObject readFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) { return {}; }
    const QJsonDocument d = QJsonDocument::fromJson(f.readAll());
    return d.object();
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: inspector_layout_smoke <scratchDir>\n");
        return 2;
    }
    const QString base = QString::fromUtf8(argv[1]);

    // ⚠️ T-0537: THE SUBJECT CHANGED, THE ASSERTIONS DID NOT.
    //
    // The store no longer opens `inspector-layout.json` itself — it goes through
    // ScriviCore ([SP-141]'s endpoints). ✅ So this test now needs a bootstrapped
    // bridge. ⛔ EVERY assertion below is unchanged: they were always about the
    // SCHEMA's behaviour (lossless round trip, unknown tab degrades, missing file
    // defaults, corrupt file untouched), never about the Qt class.
    //
    // ✅ That is deliberate — a test that keeps its assertions and changes only its
    // subject is the independent witness this Epic has twice paid for lacking
    // ([I-0214], [I-0215]).
    const QString appSupport = QDir(base).filePath(QStringLiteral("appsupport"));
    QDir().mkpath(appSupport);
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Inspector Layout Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    // ================================================================
    // 1 — ⚠️ THE ROUND TRIP: an Apple document survives a write
    // ================================================================
    {
        const QString root = QDir(base).filePath(QStringLiteral("apple"));
        const QString path = writeFile(root, kAppleLayout);
        check(!path.isEmpty(), "fixture: Apple layout written");

        const QJsonObject before = readFile(path);

        InspectorLayoutStore store;
        store.load(&bridge, root);
        check(store.selectedTab() == QLatin1String("worldbuilding"),
              "selectedTab is read from the Apple document");

        store.setSelectedTab(QStringLiteral("writing"));

        const QJsonObject after = readFile(path);

        // The one key we changed.
        check(after.value(QStringLiteral("selectedTab")).toString() == QLatin1String("writing"),
              "selectedTab was persisted");

        // ⚠️ EVERY OTHER KEY MUST BE BYTE-EQUAL. This is the assertion the whole
        // class exists to satisfy.
        for (const QString& key : before.keys()) {
            if (key == QLatin1String("selectedTab")) { continue; }
            const bool same = after.contains(key) && after.value(key) == before.value(key);
            if (!same) {
                std::fprintf(stderr, "  lost or altered key: %s\n", key.toUtf8().constData());
            }
            check(same, "an Apple-written key survived the round trip");
        }

        // ⚠️ Named explicitly as well as in the loop, so a failure reads as the
        // real-world consequence rather than as an anonymous key mismatch.
        check(after.value(QStringLiteral("stackSort")) == before.value(QStringLiteral("stackSort")),
              "stackSort intact (a writer's per-stack sort is not destroyed)");
        check(after.value(QStringLiteral("scenes")) == before.value(QStringLiteral("scenes")),
              "per-scene card layouts intact (the writer's Apple-side layout survives)");
        check(after.value(QStringLiteral("defaultStacks")) == before.value(QStringLiteral("defaultStacks")),
              "defaultStacks intact");
        check(after.contains(QStringLiteral("futureKeyFromANewerScrivi")),
              "a key this build does not model at all still survives");
    }

    // ================================================================
    // 1b — ⚠️ `inspectorHidden` PERSISTS, AND LOSES NOTHING DOING IT
    //      (EP-043 / SP-145 T-0553, [I-0251]; Sprint AC6)
    // ================================================================
    //
    // ⛔ THE DEFECT THIS GUARDS: the store wrote `inspectorHidden: false` into its
    // default document and NOTHING read it or wrote it back, so a writer who hid
    // the inspector got it back on every launch — ⚠️ while Apple persisted the same
    // key in the same file to satisfy Doc 2 AC4.
    //
    // ⚠️ IT IS TESTED HERE, beside the `selectedTab` round trip, DELIBERATELY: this
    // file is the schema's data-loss test, and `inspectorHidden` is now a second
    // writer to the same document. ⛔ A second writer is exactly what [I-0215] cost
    // once, so it is held to the same lossless standard, not a weaker one.
    {
        const QString root = QDir(base).filePath(QStringLiteral("hidden"));
        const QString path = writeFile(root, kAppleLayout);
        check(!path.isEmpty(), "fixture: Apple layout written for the hidden test");

        const QJsonObject before = readFile(path);

        InspectorLayoutStore store;
        store.load(&bridge, root);

        // The fixture says `false`, so that is what a fresh read must report.
        check(!store.inspectorHidden(),
              "inspectorHidden is read from the Apple document (false)");

        store.setInspectorHidden(true);

        const QJsonObject after = readFile(path);
        check(after.value(QStringLiteral("inspectorHidden")).toBool() == true,
              "inspectorHidden was PERSISTED — the writer's choice survives a quit");

        // ⚠️ THE SAME LOSSLESS BAR AS SECTION 1. A second writer to this document
        // must carry every other key through untouched.
        for (const QString& key : before.keys()) {
            if (key == QLatin1String("inspectorHidden")) { continue; }
            const bool same = after.contains(key) && after.value(key) == before.value(key);
            if (!same) {
                std::fprintf(stderr, "  lost or altered key: %s\n", key.toUtf8().constData());
            }
            check(same, "an Apple key survived an inspectorHidden write");
        }
        check(after.contains(QStringLiteral("futureKeyFromANewerScrivi")),
              "an unmodelled key survives an inspectorHidden write too");

        // ⚠️ A re-read must AGREE with what was written. ✅ This is the half that
        // actually proves persistence: writing the file is not the same as being
        // able to read the value back, and [I-0242] is what "written but never read"
        // costs.
        InspectorLayoutStore reread;
        reread.load(&bridge, root);
        check(reread.inspectorHidden(),
              "a re-opened project reports the inspector as hidden");

        // ⛔ A NO-OP MUST NOT WRITE. ⚠️ `setInspectorVisible` is called on EVERY
        // project open (to restore), so a store that wrote unconditionally would add
        // a write to the open path of every project — the [I-0234] class.
        const QJsonObject beforeNoop = readFile(path);
        reread.setInspectorHidden(true);   // already true
        check(readFile(path) == beforeNoop,
              "setting the SAME value writes nothing");
    }

    // ================================================================
    // 1b2 — [I-0255] `timelineHidden` + `navigatorHidden`: absent ⇒ shown,
    //       persisted, lossless, re-read agrees (same keys Apple writes)
    // ================================================================
    {
        const QString root = QDir(base).filePath(QStringLiteral("hidden-panes"));
        const QString path = writeFile(root, kAppleLayout);
        check(!path.isEmpty(), "fixture: Apple layout written for the panes test");
        const QJsonObject before = readFile(path);

        InspectorLayoutStore store;
        store.load(&bridge, root);
        check(!store.timelineHidden(), "absent timelineHidden reads as SHOWN");
        check(!store.navigatorHidden(), "absent navigatorHidden reads as SHOWN");

        store.setTimelineHidden(true);
        store.setNavigatorHidden(true);

        const QJsonObject after = readFile(path);
        check(after.value(QStringLiteral("timelineHidden")).toBool() == true,
              "timelineHidden was PERSISTED");
        check(after.value(QStringLiteral("navigatorHidden")).toBool() == true,
              "navigatorHidden was PERSISTED");
        for (const QString& key : before.keys()) {
            const bool same = after.contains(key) && after.value(key) == before.value(key);
            if (!same) {
                std::fprintf(stderr, "  lost or altered key: %s\n", key.toUtf8().constData());
            }
            check(same, "an Apple key survived the timeline/navigator writes");
        }

        InspectorLayoutStore reread;
        reread.load(&bridge, root);
        check(reread.timelineHidden() && reread.navigatorHidden(),
              "a re-opened project reports both panes as hidden");

        const QJsonObject beforeNoop = readFile(path);
        reread.setTimelineHidden(true);
        reread.setNavigatorHidden(true);
        check(readFile(path) == beforeNoop, "setting the SAME values writes nothing");
    }

    // ================================================================
    // 1c — ⚠️ an ABSENT `inspectorHidden` means SHOWN, not hidden
    // ================================================================
    //
    // ⛔ THE FAILURE THIS FORBIDS: defaulting a missing key to `true` would hide the
    // inspector on every project written before this key existed — a silent
    // regression for every existing manuscript, on first open after the fix ships.
    {
        const QString root = QDir(base).filePath(QStringLiteral("hidden-absent"));
        writeFile(root, R"({"schema":"scrivi.inspector-layout.v1","selectedTab":"writing"})");
        InspectorLayoutStore store;
        store.load(&bridge, root);
        check(!store.inspectorHidden(),
              "an ABSENT inspectorHidden reads as SHOWN (pre-existing projects keep their pane)");
    }

    // ================================================================
    // 2 — an UNKNOWN tab degrades rather than failing
    // ================================================================
    {
        const QString root = QDir(base).filePath(QStringLiteral("unknowntab"));
        writeFile(root, R"({"schema":"scrivi.inspector-layout.v1","selectedTab":"holodeck"})");
        InspectorLayoutStore store;
        store.load(&bridge, root);
        // ⚠️ A newer Scrivi may name a tab this build lacks. Refusing to open the
        // project over that would be far worse than showing Writing.
        check(store.selectedTab() == QLatin1String("writing"),
              "an unknown tab degrades to the ruled default");
    }

    // ================================================================
    // 3 — a MISSING file is not an error
    // ================================================================
    {
        const QString root = QDir(base).filePath(QStringLiteral("nofile"));
        QDir().mkpath(root);
        InspectorLayoutStore store;
        store.load(&bridge, root);
        check(store.selectedTab() == QLatin1String("writing"),
              "a missing layout file yields Apple's ruled default");

        store.setSelectedTab(QStringLiteral("properties"));
        InspectorLayoutStore reread;
        reread.load(&bridge, root);
        check(reread.selectedTab() == QLatin1String("properties"),
              "a layout file is created on first write and reads back");
    }

    // ================================================================
    // 4 — ⚠️ a CORRUPT file is NOT overwritten
    // ================================================================
    {
        const QString root = QDir(base).filePath(QStringLiteral("corrupt"));
        const QString path = writeFile(root, "{ this is not json");
        InspectorLayoutStore store;
        store.load(&bridge, root);
        store.setSelectedTab(QStringLiteral("worldbuilding"));

        QFile f(path);
        f.open(QIODevice::ReadOnly);
        const QByteArray still = f.readAll();
        f.close();
        // ⚠️ Losing a layout is annoying; destroying the evidence of HOW it broke
        // is worse. The setter must be inert against a document that never loaded.
        check(still.startsWith("{ this is not json"),
              "a corrupt layout file is left untouched for a human to inspect");
    }

    std::fprintf(stderr, "checks: %d, failures: %d\n", checks, failures);
    return failures > 0 ? 1 : 0;
}
