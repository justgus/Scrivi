// timeline_events_smoke — headless historical-events + imported-timelines + export
// persistence check for EP-025 (SP-082, T-0344, AC5). No GUI.
//
// Exercises the timeline-events deliverables through the bridge (the layer EditorShell's
// SP-082 slots call), verified by reopen / re-list:
//   1. Historical events: create two (createHistoricalEvent) → list reports both with
//      their offsets; update one (updateHistoricalEvent — new title + offset) → list
//      reflects it; delete one (deleteHistoricalEvent) → list drops it.
//   2. Import: write a fixture .scrivi-timeline.json, import it (importExternalTimeline
//      with an epoch offset + grey shade) → listImportedTimelines reports it (count 1,
//      the shade, visible); the stored file exists on disk with its events.
//   3. Hide/show: setImportedTimelineVisible(false) → list shows visible=false; (true)
//      → visible=true again.
//   4. Update offset: updateImportedTimelineOffset → the stored file's epochOffsetMs
//      changes.
//   5. Export: exportProjectTimeline → a non-empty body with schema
//      scrivi.externalTimeline.v1 whose events include the surviving historical event.
//   6. Remove: removeImportedTimeline → list count back to 0.
//
//   argv[1] = project dir to create (e.g. <tmp>/timeline-events.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

// Parse a {count, eventsJSON} / {count, timelinesJSON} list envelope's inner array.
QJsonArray innerArray(const QVariantMap& listResult, const char* jsonKey, const char* arrayKey)
{
    const QString json = listResult.value(QString::fromLatin1(jsonKey)).toString();
    return QJsonDocument::fromJson(json.toUtf8())
        .object().value(QString::fromLatin1(arrayKey)).toArray();
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: timeline_events_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Timeline Events Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Timeline Events Project"),
                             QStringLiteral("timeline-events"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // --- 1. Historical events: create / list / update / delete ------------
    const QVariantMap ce1 = bridge.createHistoricalEvent(
        projectPath, QStringLiteral("The Sundering"), -31536000000LL,
        QStringLiteral("A cataclysm long ago."), QStringLiteral(""));
    const QString ev1 = ce1.value(QStringLiteral("eventID")).toString();
    check(!ev1.isEmpty(), "createHistoricalEvent #1 returned an eventID");

    const QVariantMap ce2 = bridge.createHistoricalEvent(
        projectPath, QStringLiteral("The Founding"), 0LL,
        QStringLiteral(""), QStringLiteral("{\"tags\":[\"origin\"]}"));
    const QString ev2 = ce2.value(QStringLiteral("eventID")).toString();
    check(!ev2.isEmpty(), "createHistoricalEvent #2 returned an eventID");

    {
        const QVariantMap list = bridge.listHistoricalEvents(projectPath);
        check(list.value(QStringLiteral("count")).toInt() == 2,
              "list reports 2 historical events after two creates");
        const QJsonArray arr = innerArray(list, "eventsJSON", "events");
        check(arr.size() == 2, "eventsJSON has 2 events");
    }

    // Update #1: new title + move it later.
    bridge.updateHistoricalEvent(projectPath, ev1, QStringLiteral("The Great Sundering"),
                                 -15768000000LL, QStringLiteral("Revised note."),
                                 QStringLiteral(""));
    {
        const QVariantMap list = bridge.listHistoricalEvents(projectPath);
        const QJsonArray arr = innerArray(list, "eventsJSON", "events");
        bool found = false;
        for (const QJsonValue& v : arr) {
            const QJsonObject o = v.toObject();
            if (o.value(QStringLiteral("eventID")).toString() == ev1) {
                found = o.value(QStringLiteral("title")).toString()
                            == QStringLiteral("The Great Sundering")
                        && static_cast<qint64>(o.value(QStringLiteral("offsetMs")).toDouble())
                               == -15768000000LL;
            }
        }
        check(found, "updateHistoricalEvent persisted the new title + offset");
    }

    // ================================================================
    // T-0542 / [I-0241] — ⚠️ TAGS SURVIVE A DRAG
    // ================================================================
    //
    // ⚠️ THE DATA-LOSS PATH THIS TASK CLOSED. `onHistoricalEventDragged` re-times an
    // event by calling updateHistoricalEvent, which OVERWRITES ALL FIELDS — so it must
    // re-send the tags. It used to recover them by reading every file in
    // `objects/historical-events/` off disk (`readHistoricalEventTagsFromDisk`), and
    // that function returned an EMPTY list on any failure, ⛔ so a failed read
    // SILENTLY ERASED the writer's tags.
    //
    // ✅ Tags now arrive in the LIST PROJECTION (T-0542's core half) and live in the
    // shell's `histEvents_` cache. This asserts the full loop the drag performs:
    // list → read tags → re-send with a new offset → they are still there.
    //
    // ⚠️ VERIFIED SEPARATELY FROM THE EDIT-DIALOG PREFILL on purpose — they are
    // different mechanisms (`feedback_verify_each_half_separately`).
    {
        // The projection must carry them at all — before T-0542 this was empty.
        QStringList tagsFromList;
        const QVariantMap list = bridge.listHistoricalEvents(projectPath);
        const QJsonArray arr = innerArray(list, "eventsJSON", "events");
        for (const QJsonValue& v : arr) {
            const QJsonObject o = v.toObject();
            if (o.value(QStringLiteral("eventID")).toString() == ev2) {
                for (const QJsonValue& t : o.value(QStringLiteral("tags")).toArray()) {
                    tagsFromList.append(t.toString());
                }
            }
        }
        check(tagsFromList == QStringList{QStringLiteral("origin")},
              "T-0542: listHistoricalEvents PROJECTS tags");

        // The drag: re-send the cached title/description + THOSE tags, new offset.
        QJsonArray reArr;
        for (const QString& t : tagsFromList) { reArr.append(t); }
        QJsonObject reObj;
        reObj.insert(QStringLiteral("tags"), reArr);
        bridge.updateHistoricalEvent(
            projectPath, ev2, QStringLiteral("The Founding"), 86400000LL,
            QStringLiteral(""),
            QString::fromUtf8(QJsonDocument(reObj).toJson(QJsonDocument::Compact)));

        QStringList after;
        qint64 movedTo = 0;
        const QVariantMap list2 = bridge.listHistoricalEvents(projectPath);
        const QJsonArray arr2 = innerArray(list2, "eventsJSON", "events");
        for (const QJsonValue& v : arr2) {
            const QJsonObject o = v.toObject();
            if (o.value(QStringLiteral("eventID")).toString() == ev2) {
                movedTo = static_cast<qint64>(
                    o.value(QStringLiteral("offsetMs")).toDouble());
                for (const QJsonValue& t : o.value(QStringLiteral("tags")).toArray()) {
                    after.append(t.toString());
                }
            }
        }
        check(movedTo == 86400000LL, "T-0542: the drag moved the event");
        check(after == QStringList{QStringLiteral("origin")},
              "T-0542: tags SURVIVED the drag (they used to be silently erased)");
    }

    // Delete #2.
    bridge.deleteHistoricalEvent(projectPath, ev2);
    {
        const QVariantMap list = bridge.listHistoricalEvents(projectPath);
        check(list.value(QStringLiteral("count")).toInt() == 1,
              "list reports 1 historical event after delete");
    }

    // --- 2. Import a fixture .scrivi-timeline.json ------------------------
    // Two events, offsets 0 and +1 year.
    const QString fixture = QStringLiteral(
        "{\"schema\":\"scrivi.externalTimeline.v1\","
        "\"timelineID\":\"timeline_fixture\","
        "\"sourceProjectTitle\":\"The Iron Chronicle\","
        "\"sourceProjectID\":\"project_fixture\","
        "\"exportedAt\":\"2026-06-11T00:00:00Z\","
        "\"epochLabel\":\"The First Age\","
        "\"events\":["
        "{\"eventID\":\"e1\",\"title\":\"Gate Built\",\"offsetMs\":0,\"kind\":\"historical\",\"notes\":null},"
        "{\"eventID\":\"e2\",\"title\":\"Gate Fell\",\"offsetMs\":31536000000,\"kind\":\"historical\",\"notes\":null}"
        "]}");
    const QVariantMap imp = bridge.importExternalTimeline(
        projectPath, fixture, 0LL, QStringLiteral("#8A8A8A"));
    const QString importedID = imp.value(QStringLiteral("timelineID")).toString();
    check(!importedID.isEmpty(), "importExternalTimeline returned a timelineID");

    {
        const QVariantMap list = bridge.listImportedTimelines(projectPath);
        check(list.value(QStringLiteral("count")).toInt() == 1,
              "listImportedTimelines reports 1 after import");
        const QJsonArray arr = innerArray(list, "timelinesJSON", "timelines");
        check(arr.size() == 1, "timelinesJSON has 1 timeline");
        if (!arr.isEmpty()) {
            const QJsonObject o = arr.first().toObject();
            check(o.value(QStringLiteral("visible")).toBool(),
                  "imported timeline is visible by default");
            check(o.value(QStringLiteral("assignedGreyShade")).toString()
                      == QStringLiteral("#8A8A8A"),
                  "imported timeline kept its grey shade");
            check(o.value(QStringLiteral("eventCount")).toInt() == 2,
                  "imported timeline reports 2 events");

            // SP-129/T-0502: the projection must carry the EVENTS THEMSELVES, not just
            // a count. ⚠️ Before SP-129 it carried only `eventCount`, which is why BOTH
            // platforms re-read the stored file to draw their dots. If this regresses,
            // the panel silently draws an EMPTY imported row rather than failing.
            const QJsonArray evs = o.value(QStringLiteral("events")).toArray();
            check(evs.size() == 2, "projection carries the 2 events");
            if (evs.size() == 2) {
                const QJsonObject e0 = evs.first().toObject();
                check(e0.contains(QStringLiteral("projectOffsetMs")),
                      "event carries projectOffsetMs");
                // ⚠️ Pre-RESOLVED by the core (offsetMs + epochOffsetMs), so no platform
                // recomputes it. Assert the sum, not merely the field's presence.
                check(e0.value(QStringLiteral("projectOffsetMs")).toDouble()
                          == e0.value(QStringLiteral("offsetMs")).toDouble()
                             + o.value(QStringLiteral("epochOffsetMs")).toDouble(),
                      "projectOffsetMs == offsetMs + epochOffsetMs");
            }
        }
    }

    // The stored file exists on disk with its events.
    // ⚠️ SP-129/T-0502: this is NO LONGER "the read path the panel uses" — the panel now
    // reads the list projection. This asserts PERSISTENCE only.
    {
        const QDir dir(projectPath + QStringLiteral("/objects/imported-timelines"));
        check(dir.exists(), "imported-timelines dir created");
        const QStringList files = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
        check(files.size() == 1, "one imported-timeline file on disk");
    }

    // --- 3. Hide / show --------------------------------------------------
    bridge.setImportedTimelineVisible(projectPath, importedID, false);
    {
        const QJsonArray arr =
            innerArray(bridge.listImportedTimelines(projectPath), "timelinesJSON", "timelines");
        check(!arr.isEmpty() && !arr.first().toObject().value(QStringLiteral("visible")).toBool(),
              "hide → visible=false persisted");
    }
    bridge.setImportedTimelineVisible(projectPath, importedID, true);
    {
        const QJsonArray arr =
            innerArray(bridge.listImportedTimelines(projectPath), "timelinesJSON", "timelines");
        check(!arr.isEmpty() && arr.first().toObject().value(QStringLiteral("visible")).toBool(),
              "show → visible=true persisted");
    }

    // --- 4. Update epoch offset ------------------------------------------
    bridge.updateImportedTimelineOffset(projectPath, importedID, 63072000000LL);
    {
        const QJsonArray arr =
            innerArray(bridge.listImportedTimelines(projectPath), "timelinesJSON", "timelines");
        check(!arr.isEmpty()
                  && static_cast<qint64>(
                         arr.first().toObject().value(QStringLiteral("epochOffsetMs")).toDouble())
                         == 63072000000LL,
              "updateImportedTimelineOffset persisted the new offset");
    }

    // --- 5. Export -------------------------------------------------------
    {
        const QVariantMap ex = bridge.exportProjectTimeline(projectPath);
        const QString body = ex.value(QStringLiteral("timelineJSON")).toString();
        check(!body.isEmpty(), "exportProjectTimeline returned a non-empty body");
        const QJsonObject o = QJsonDocument::fromJson(body.toUtf8()).object();
        check(o.value(QStringLiteral("schema")).toString()
                  == QStringLiteral("scrivi.externalTimeline.v1"),
              "export body has schema scrivi.externalTimeline.v1");
        // The surviving historical event should appear among the exported events.
        bool foundHist = false;
        for (const QJsonValue& v : o.value(QStringLiteral("events")).toArray()) {
            if (v.toObject().value(QStringLiteral("title")).toString()
                == QStringLiteral("The Great Sundering")) {
                foundHist = true;
            }
        }
        check(foundHist, "export includes the surviving historical event");
    }

    // --- 6. Remove imported ----------------------------------------------
    bridge.removeImportedTimeline(projectPath, importedID);
    {
        const QVariantMap list = bridge.listImportedTimelines(projectPath);
        check(list.value(QStringLiteral("count")).toInt() == 0,
              "removeImportedTimeline → count back to 0");
    }

    if (failures > 0) {
        std::fprintf(stderr, "timeline_events_smoke: %d check(s) FAILED\n", failures);
        return 1;
    }
    std::fprintf(stderr, "PASS: historical events + imported timelines + export round-trip.\n");
    return 0;
}
