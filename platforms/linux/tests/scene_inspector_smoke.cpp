// scene_inspector_smoke — EP-035 / SP-125 (T-0480–T-0483): the Scene Inspector's
// READ PATH, exercised through ScriviBridge and ObjectKindScope. No GUI.
//
// ⚠️ **Through the bridge, never the C ABI directly**
// (`feedback_boundary_tests_not_facade`): a facade test cannot see a boundary
// gap, and that is exactly how I-0113 shipped green.
//
// ⚠️ **This does NOT replace the live click-through.** EP-035 AC9 requires a
// human pass on every surface-shipping sprint, because 22 consecutive Issues
// across SP-118–SP-120 came from a writer clicking and NONE from any suite. What
// this covers is the part a suite genuinely can: the read path's data decisions.
//
// What is asserted:
//   • ObjectKindScope DERIVES the partition from the core — `source` reports
//     project-scoped and `character` world-scoped because the CORE says so, not
//     because this file says so. If the core's answer ever changes, this test
//     changes with it. That is the property restatement destroys.
//   • listEdgesFor returns the far endpoint ALREADY RESOLVED, including a label
//     projected for the queried endpoint — the value the panel must READ and
//     never recompute (a known trap for this Epic; it lands in EP-037).
//   • ⚠️ An EMPTY list and a FAILED call are DISTINGUISHABLE. A scene with no
//     edges yields {"ok":true,"result":{}}, byte-identical to the empty map
//     parseEnvelope returns on failure — so the panel cannot tell "no objects"
//     from "unreadable" by the result alone. lastCallFailed() is what separates
//     them, and T-0483 (absence is never deletion) depends on it entirely.
//   • The scene→object grouping keeps only endpoints the core vouches for, so a
//     scene-to-scene edge does not appear in a tab that lists OBJECTS.
//
//   argv[1] = project dir to create
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QCoreApplication>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include "AppSupport.hpp"
#include "ObjectKindScope.hpp"
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

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_inspector_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport  = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Inspector Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Inspector Project"),
                             QStringLiteral("scene-inspector"));
    if (created.value(QStringLiteral("projectID")).toString().isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    const QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QString sceneID =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap()
            .value(QStringLiteral("sceneID")).toString();
    check(!sceneID.isEmpty(), "fixture: first scene present");

    // ================================================================
    // T-0480 — the partition is DERIVED, never restated
    // ================================================================

    check(ObjectKindScope::isLoaded(&bridge),
          "ObjectKindScope loads the partition from the core");

    const QList<ObjectKindScope::KindInfo> kinds = ObjectKindScope::kinds(&bridge);
    check(!kinds.isEmpty(), "ObjectKindScope reports at least one kind");

    // ⚠️ Read the CORE's answer for each kind and assert ObjectKindScope agrees
    // with it — rather than asserting against a list written here, which would
    // make this test itself occurrence nine.
    const QVariantList raw =
        bridge.listObjectKinds().value(QStringLiteral("kinds")).toList();
    check(raw.size() == kinds.size(),
          "ObjectKindScope reports exactly the kinds the core does");

    bool everyKindAgrees = true;
    for (const QVariant& k : raw) {
        const QVariantMap m = k.toMap();
        const QString kind = m.value(QStringLiteral("kind")).toString();
        const bool coreSays = m.value(QStringLiteral("isWorldScoped")).toBool();
        if (ObjectKindScope::isWorldScoped(&bridge, kind) != coreSays) {
            everyKindAgrees = false;
            std::fprintf(stderr, "  kind `%s`: core=%d scope=%d\n",
                         kind.toUtf8().constData(), coreSays,
                         ObjectKindScope::isWorldScoped(&bridge, kind));
        }
    }
    check(everyKindAgrees, "every kind's scope matches the core's answer exactly");

    // ⚠️ The unknown-kind fallback is world-scoped ON PURPOSE: it makes the app
    // ASK for a world (failing loudly and recoverably) instead of silently
    // passing an empty worldID, which is the SP-104 outage. It names no kinds, so
    // it is a safety default and not a restatement.
    check(ObjectKindScope::isWorldScoped(&bridge, QStringLiteral("no-such-kind")),
          "an unknown kind falls back to world-scoped (fails loudly, not silently)");

    // ================================================================
    // T-0483 — an EMPTY list and a FAILED call must be distinguishable
    // ================================================================

    // A brand-new scene has no edges at all. ⚠️ This is the case that makes the
    // whole state-separation necessary: the result map is EMPTY, exactly as it
    // would be if the call had failed.
    const QVariantMap noEdges = bridge.listEdgesFor(projectPath, sceneID);
    check(noEdges.isEmpty(), "fixture: a fresh scene's edge listing is an empty map");
    check(!bridge.lastCallFailed(),
          "an EMPTY edge listing is not reported as a failure (absence != error)");

    // And the converse: a genuinely failing call must report failure, so the two
    // really are separable rather than both reading as "fine".
    //
    // ⚠️ A NONEXISTENT PROJECT PATH IS NOT SUCH A CALL, which is worth recording
    // because it is the obvious thing to reach for and it is wrong.
    // RelationshipStore::replay treats a missing edge log as "no log = empty
    // graph" and SUCCEEDS (RelationshipStore.cpp:72-74) — deliberately, since a
    // project that has never related anything has no log to read. So a bad path
    // yields an OK, EMPTY envelope, indistinguishable from a real empty scene,
    // and it cannot be used to prove failure is detected. `listObjects` with an
    // unrecognised kind is a real error by construction: the core refuses it
    // rather than returning an empty listing, precisely so a typo never reads as
    // "you have no characters" (scrivi_c_api.cpp, unknownObjectKindError).
    bridge.listObjects(projectPath, QStringLiteral("definitely-not-a-kind"));
    check(bridge.lastCallFailed(),
          "a FAILED listing is reported as a failure");

    // ⚠️ And the flag must be per-call, not sticky: a good call after a bad one
    // must clear it, or every state after the first error reads as broken.
    bridge.listObjects(projectPath, QString());
    check(!bridge.lastCallFailed(),
          "the failure flag is per-call, not sticky");

    // ================================================================
    // T-0481 — the scene's objects, resolved by the core
    // ================================================================

    // A project-scoped kind and a world-scoped one would need a world; use the
    // project-scoped `source`, chosen FROM the core's answer rather than assumed.
    QString projectScopedKind;
    for (const ObjectKindScope::KindInfo& info : kinds) {
        if (!info.isWorldScoped) {
            projectScopedKind = info.kind;
            break;
        }
    }
    check(!projectScopedKind.isEmpty(),
          "the core reports at least one project-scoped kind");

    const QVariantMap madeObj =
        bridge.createObject(projectPath, projectScopedKind,
                            QStringLiteral("The Stairs of Tintagael"),
                            QString(), QString());
    const QString objectID = madeObj.value(QStringLiteral("objectID")).toString();
    check(!objectID.isEmpty(), "createObject returns an objectID");

    // Relate it to the scene. ⚠️ ONE edge, never two — the inverse is a read-time
    // label projection, and writing a reverse edge is the trap the canonical
    // design exists to prevent.
    const QVariantMap edge =
        bridge.createEdge(projectPath, objectID, sceneID,
                          QStringLiteral("cites"), QString());
    check(!edge.value(QStringLiteral("edgeID")).toString().isEmpty(),
          "createEdge relates the object to the scene");

    // Now ask the SCENE for its edges — the panel's read path, step 1.
    const QVariantMap edgesResult = bridge.listEdgesFor(projectPath, sceneID);
    check(!bridge.lastCallFailed(), "the scene's edges are readable");
    const QVariantList edges = edgesResult.value(QStringLiteral("edges")).toList();
    check(edges.size() == 1, "the scene reports exactly one edge");

    if (!edges.isEmpty()) {
        const QVariantMap e = edges.at(0).toMap();
        check(e.value(QStringLiteral("otherID")).toString() == objectID,
              "the far endpoint is the object, resolved by the core");
        check(e.value(QStringLiteral("otherDisplayName")).toString()
                  == QStringLiteral("The Stairs of Tintagael"),
              "the far endpoint's display name arrives already resolved");
        check(e.value(QStringLiteral("otherKind")).toString() == projectScopedKind,
              "the far endpoint's KIND travels on the edge (I-0124: without it a "
              "pending endpoint cannot be attributed to a group)");
        // ⚠️ The label is projected for the endpoint we ASKED about. The panel
        // must READ this; recomputing direction in Qt is the same defect class as
        // restating the kind list.
        check(!e.value(QStringLiteral("label")).toString().isEmpty(),
              "the edge label arrives ALREADY RESOLVED for the queried endpoint");
        check(!e.value(QStringLiteral("otherPending")).toBool(),
              "a reachable object is not pending");
    }

    // Step 2 of the read path: one object listing per load, used to confirm each
    // far endpoint rather than a lookup per edge.
    const QVariantMap objectsResult = bridge.listObjects(projectPath, QString());
    check(!bridge.lastCallFailed(), "the object index is readable");
    QSet<QString> knownIDs;
    for (const QVariant& o : objectsResult.value(QStringLiteral("objects")).toList()) {
        knownIDs.insert(o.toMap().value(QStringLiteral("objectID")).toString());
    }
    check(knownIDs.contains(objectID),
          "the related object is confirmed against the index");

    // ⚠️ The confirmation filter must keep this tab to OBJECTS. A scene endpoint
    // is not in the object index and is not pending, so it is correctly excluded
    // — which is what stops a scene-to-scene edge appearing as an object row.
    check(!knownIDs.contains(sceneID),
          "a scene is not an object, so a scene endpoint is filtered out");

    std::fprintf(stderr, "checks: %d, failures: %d\n", checks, failures);
    if (failures > 0) {
        return 1;
    }
    std::printf("%s\n", created.value(QStringLiteral("projectID")).toString()
                            .toUtf8().constData());
    return 0;
}
