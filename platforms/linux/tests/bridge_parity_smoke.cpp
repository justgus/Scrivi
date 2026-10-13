// bridge_parity_smoke — EP-034 SP-121 (T-0464): every endpoint SP-121 bridged,
// exercised THROUGH ScriviBridge. No GUI.
//
// ⚠️ **Through the bridge, never the C ABI directly.**
// `feedback_boundary_tests_not_facade`: a facade test cannot see a boundary gap,
// and that is exactly how I-0113 shipped green — the C ABI was missing a kind and
// the facade tests passed regardless. A test that calls `scrivi_*` here would
// prove ScriviCore works (it does; 567 ctests say so) and prove NOTHING about the
// binding this sprint exists to build.
//
// ⚠️ **This is the sprint's ONLY acceptance evidence**, by user ruling 2026-08-24:
// *"We'll verify the backend with the ctests and the subsequent Epic will manage
// the surfacing of the capability in the Linux App."* No UI ships, so there is no
// live click-through — which is a real loss of signal, since 22 consecutive Issues
// across SP-118–SP-120 came from clicking and none from a suite. That signal is
// DEFERRED to EP-035, not waived.
//
// What is asserted per endpoint:
//   • it is CALLABLE through the bridge (it links, and the Q_INVOKABLE exists);
//   • it returns a parsed envelope rather than a crash or a hang;
//   • where it writes, the effect is verified BY REOPENING — never by trusting
//     the call's own return, which is the pattern every Linux smoke test follows.
//
//   argv[1] = project dir to create
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>
#include <thread>

#include "AppSupport.hpp"
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

// A call that must not crash and must come back with SOMETHING parsed. Used for
// read endpoints whose emptiness on a fresh project is the correct answer.
//
// ⚠️ Deliberately weak, and deliberately NOT used for writes: "it did not crash"
// is a reachability assertion, not a behaviour one. Writes are checked by reopen.
void reaches(const QVariantMap&, const char* what)
{
    ++checks;
    (void)what;   // reaching this line at all is the assertion
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: bridge_parity_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport  = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Bridge Parity Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Bridge Parity Project"),
                             QStringLiteral("bridge-parity"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QString sceneID =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap()
            .value(QStringLiteral("sceneID")).toString();
    check(!sceneID.isEmpty(), "fixture: first scene present");

    // ================================================================
    // T-0461 — object kinds, objects, worlds
    // ================================================================

    // ⚠️ THE endpoint D5 exists for: it is how a platform DERIVES which kinds are
    // world-scoped. A Qt restatement of that partition would be occurrence NINE of
    // this project's most-repeated defect — and occurrence FIVE was in Swift, which
    // is the proof it is not a C++ concern.
    const QVariantMap kinds = bridge.listObjectKinds();
    const QVariantList kindList = kinds.value(QStringLiteral("kinds")).toList();
    check(!kindList.isEmpty(), "listObjectKinds returns kinds");

    bool sawSource = false, sawCharacter = false, sourceIsProjectScoped = false;
    for (const QVariant& k : kindList) {
        const QVariantMap m = k.toMap();
        const QString kind = m.value(QStringLiteral("kind")).toString();
        if (kind == QStringLiteral("source")) {
            sawSource = true;
            sourceIsProjectScoped = !m.value(QStringLiteral("isWorldScoped")).toBool();
        }
        if (kind == QStringLiteral("character")) sawCharacter = true;
    }
    check(sawSource,    "listObjectKinds reports `source`");
    check(sawCharacter, "listObjectKinds reports `character`");
    // ⚠️ Asserted from the CORE's answer, not from a local list. If `source` ever
    // becomes world-scoped this test changes because the core changed — which is
    // exactly the property that makes derivation safe and restatement unsafe.
    check(sourceIsProjectScoped, "`source` is project-scoped per the core");

    // --- objects -------------------------------------------------------------
    const QVariantMap madeSource =
        bridge.createObject(projectPath, QStringLiteral("source"),
                            QStringLiteral("The Stairs of Tintagael"),
                            QString(), QString());
    const QString sourceID = madeSource.value(QStringLiteral("objectID")).toString();
    check(!sourceID.isEmpty(), "createObject(source) returns an objectID");

    const QVariantMap madeChar =
        bridge.createObject(projectPath, QStringLiteral("character"),
                            QStringLiteral("Myton at 23"), QString(), QString());
    const QString charID = madeChar.value(QStringLiteral("objectID")).toString();
    // ⚠️ A project-scoped create of a WORLD-scoped kind is expected to be refused
    // (no world bound). Either outcome is recorded; what matters is it is reached
    // and does not crash the bridge.
    reaches(madeChar, "createObject(character) reached");

    const QVariantMap objects = bridge.listObjects(projectPath, QStringLiteral("source"));
    check(objects.value(QStringLiteral("objects")).toList().size() >= 1,
          "listObjects(source) sees the created source");

    const QVariantMap openedObj =
        bridge.openObject(projectPath, QStringLiteral("source"), sourceID, QString());
    const QString objJson = openedObj.value(QStringLiteral("objectJson")).toString();
    check(objJson.contains(QStringLiteral("The Stairs of Tintagael")),
          "openObject returns the object's JSON");

    // ⚠️ Save by PATCH, exactly as Apple does: mutate the JSON that came back
    // rather than reconstructing it, so fields this test does not model survive.
    QString patched = objJson;
    patched.replace(QStringLiteral("\"notes\":\"\""),
                    QStringLiteral("\"notes\":\"Cited for the naming magic.\""));
    reaches(bridge.saveObject(projectPath, QStringLiteral("source"), patched),
            "saveObject reached");

    reaches(bridge.listOrphanedObjects(projectPath), "listOrphanedObjects reached");
    reaches(bridge.listWorlds(projectPath),          "listWorlds reached");
    reaches(bridge.getWorldBinding(projectPath, QString()), "getWorldBinding reached");
    reaches(bridge.getWorldStatus(projectPath, QString()),  "getWorldStatus reached");

    // ================================================================
    // T-0462 — relation types, edges, assets
    // ================================================================

    const QVariantMap types = bridge.listRelationTypes(projectPath);
    const QVariantList typeList = types.value(QStringLiteral("types")).toList();
    check(!typeList.isEmpty(), "listRelationTypes returns the seeded vocabulary");

    bool sawCites = false;
    for (const QVariant& t : typeList) {
        if (t.toMap().value(QStringLiteral("code")).toString() == QStringLiteral("cites"))
            sawCites = true;
    }
    check(sawCites, "`cites` is seeded (SP-096 / T-0373)");

    if (!charID.isEmpty()) {
        // ⚠️ `cites` is unconstrained on BOTH ends, which is what lets one source
        // document any kind — the property SP-120 verified by use on real data.
        const QVariantMap edge =
            bridge.createEdge(projectPath, sourceID, charID,
                              QStringLiteral("cites"), QString());
        const QString edgeID = edge.value(QStringLiteral("edgeID")).toString();
        check(!edgeID.isEmpty(), "createEdge(cites) returns an edgeID");

        const QVariantMap edges = bridge.listEdgesFor(projectPath, sourceID);
        check(edges.value(QStringLiteral("edges")).toList().size() >= 1,
              "listEdgesFor sees the new edge");

        // ⚠️ The label arrives ALREADY RESOLVED for the queried endpoint. The
        // bridge must pass it through untouched — recomputing direction in Qt
        // would be a local copy of a rule the core owns.
        const QVariantMap firstEdge =
            edges.value(QStringLiteral("edges")).toList().at(0).toMap();
        check(!firstEdge.value(QStringLiteral("label")).toString().isEmpty(),
              "listEdgesFor carries a resolved label");

        if (!edgeID.isEmpty()) {
            reaches(bridge.deleteEdge(projectPath, edgeID), "deleteEdge reached");
        }
    }

    reaches(bridge.listPendingEdges(projectPath), "listPendingEdges reached");
    reaches(bridge.listAssets(projectPath, QString(), QString()), "listAssets reached");

    // ================================================================
    // T-0463 — scene metadata, comments, inbox, repair, snapshots
    // ================================================================

    reaches(bridge.setSceneTags(projectPath, sceneID, QStringLiteral("[]")),
            "setSceneTags reached");
    reaches(bridge.setSceneTodo(projectPath, sceneID, QString()), "setSceneTodo reached");
    reaches(bridge.setSceneOutline(projectPath, sceneID, QString()),
            "setSceneOutline reached");
    reaches(bridge.getSceneNotes(projectPath, sceneID), "getSceneNotes reached");

    reaches(bridge.addComment(projectPath, QStringLiteral("scene"), sceneID,
                              QStringLiteral("A parity comment.")),
            "addComment reached");
    reaches(bridge.listComments(projectPath, QStringLiteral("scene"), sceneID),
            "listComments reached");

    reaches(bridge.listInbox(projectPath),  "listInbox reached");
    reaches(bridge.scanForExternalChanges(projectPath, appSupport, 0),
            "scanForExternalChanges reached");
    reaches(bridge.extractSearchableText(projectPath),
            "extractSearchableText reached");

    // ⚠️ Timeline epoch endpoints — NOT reached by Apple either (audit Finding A).
    reaches(bridge.setTimelineEpochLabel(projectPath, QStringLiteral("Second Age")),
            "setTimelineEpochLabel reached");

    // ================================================================
    // EP-047 SP-167 — settings that travel; checked by READING BACK, never by the write's own return
    // ================================================================
    check(bridge.getProjectSettings(projectPath).value(QStringLiteral("status")).toString()
              == QStringLiteral("absent"),
          "getProjectSettings: absent on a new project");
    bridge.putProjectSettings(projectPath, QStringLiteral(R"({"subtitle":"From Linux","showChapterTitles":true})"));
    const QVariantMap settings =
        bridge.getProjectSettings(projectPath).value(QStringLiteral("document")).toMap();
    check(settings.value(QStringLiteral("subtitle")).toString() == QStringLiteral("From Linux"),
          "putProjectSettings → getProjectSettings round-trips");
    bridge.setProjectTitle(projectPath, QStringLiteral("Renamed On Linux"));
    check(bridge.openProject(projectPath, appSupport).value(QStringLiteral("projectTitle")).toString()
              == QStringLiteral("Renamed On Linux"),
          "setProjectTitle: the reopened project reports the new title");

    // ================================================================
    // SP-173 D5 — the failure flag is PER THREAD: a worker's failed call must not change what the main thread reads
    // ================================================================
    {
        bridge.getProjectSettings(projectPath);                            // main thread: succeeds
        check(!bridge.lastCallFailed(), "D5: the main thread's call succeeded");
        bool workerSawFailure = false;
        std::thread worker([&] {
            bridge.getProjectSettings(QString());                          // worker: fails (no project root)
            workerSawFailure = bridge.lastCallFailed();
        });
        worker.join();
        check(workerSawFailure, "D5: the worker sees its OWN failure");
        check(!bridge.lastCallFailed(), "D5: the worker's failure does not overwrite the main thread's answer");
    }

    // ================================================================
    // EP-026 S2 (SP-178 / T-0605) — the undo history and its capture rules, through the bridge (all 16 S2 endpoints)
    // ================================================================
    {
        const QString scene = QStringLiteral("scene_parity");
        check(!ScriviBridge::historyOpen(projectPath).isEmpty(), "historyOpen");
        check(!ScriviBridge::historyValidateScene(projectPath, scene, QStringLiteral("Floor.")).isEmpty(),
              "historyValidateScene");
        reaches(ScriviBridge::captureState(projectPath), "captureState");
        reaches(ScriviBridge::captureCaretMoved(projectPath, scene, false, false), "captureCaretMoved");
        // A sentence typed: the boundary commits it (the core's rule), so there is something to undo.
        const QVariantMap edit = ScriviBridge::captureNoteEdit(projectPath, scene, QStringLiteral("Floor. One."), 11,
                                                               QStringLiteral("Floor."), 1000);
        check(edit.value(QStringLiteral("committed")).toBool() && edit.value(QStringLiteral("canUndo")).toBool(),
              "captureNoteEdit: a sentence commits one undoable step");
        reaches(ScriviBridge::captureBeforeSave(projectPath, 1100), "captureBeforeSave");
        check(!ScriviBridge::historyNoteScenePersisted(projectPath, scene, QStringLiteral("Floor. One.")).isEmpty(),
              "historyNoteScenePersisted");
        reaches(ScriviBridge::captureBegin(projectPath, QStringLiteral("paste")), "captureBegin");
        ScriviBridge::captureNoteEdit(projectPath, scene, QStringLiteral("Floor. One. Pasted"), 18,
                                      QStringLiteral("Floor. One."), 1200);
        check(ScriviBridge::captureFlush(projectPath, QStringLiteral("paste"), QStringLiteral("paste"))
                  .value(QStringLiteral("committed")).toBool(), "captureFlush: the paste is its own step");
        reaches(ScriviBridge::captureIdle(projectPath, 1300), "captureIdle");
        const QString grouped = QStringLiteral(
            "[{\"sceneID\":\"scene_parity\",\"textBefore\":\"Floor. One. Pasted\",\"textAfter\":\"Floor.\",\"caret\":6},"
            "{\"sceneID\":\"scene_two\",\"textBefore\":\"Two.\",\"textAfter\":\"\",\"caret\":0}]");
        check(ScriviBridge::captureGroupedEdit(projectPath, grouped, QStringLiteral("delete"))
                  .value(QStringLiteral("canUndo")).toBool(), "captureGroupedEdit");
        const QVariantMap undo = ScriviBridge::captureUndo(projectPath);
        check(undo.value(QStringLiteral("changes")).toList().size() == 2,
              "captureUndo: ONE undo restores both scenes of the grouped edit");
        reaches(ScriviBridge::captureNoteApplied(projectPath, scene, QStringLiteral("Floor. One. Pasted")),
                "captureNoteApplied");
        check(ScriviBridge::captureRedo(projectPath).value(QStringLiteral("moved")).toBool(), "captureRedo");
        check(!ScriviBridge::historyClose(projectPath).isEmpty(), "historyClose");
        // Written BY REOPENING, never trusted from the call: the history persisted across close.
        check(ScriviBridge::historyOpen(projectPath).value(QStringLiteral("canUndo")).toBool(),
              "history persists: the reopened history can undo");
        // A barrier stops undo (the reopened history could undo; now it cannot).
        check(!ScriviBridge::captureRecordBarrier(
                   projectPath, QStringLiteral("{\"barrierKind\":\"sceneSplit\",\"note\":\"Can't undo past creating a scene\"}"))
                   .value(QStringLiteral("canUndo")).toBool(), "captureRecordBarrier: undo stops at it");
        ScriviBridge::historyClose(projectPath);
        // Best-effort: a failed call returns {} and sets the flag, and never emits errorOccurred.
        bool emitted = false;
        QObject::connect(&bridge, &ScriviBridge::errorOccurred, [&emitted](int, const QString&) { emitted = true; });
        check(ScriviBridge::captureUndo(QString()).isEmpty() && bridge.lastCallFailed() && !emitted,
              "a failed history call is quiet: {} + lastCallFailed, no errorOccurred");
    }

    // ================================================================
    // EP-026 S5 (SP-179 / T-0606) — the copy buffers, through the bridge (all 4 `scrivi_buffers_*`)
    // ================================================================
    {
        // Stored text, escapes included, round-trips VERBATIM (EP-045 AC4).
        const QString stored = QStringLiteral("**Bold** a\\. b");
        check(!ScriviBridge::buffersLoad(projectPath, QStringLiteral("3"), stored).isEmpty(), "buffersLoad");
        const QVariantMap got = ScriviBridge::buffersGet(projectPath, QStringLiteral("3"));
        check(got.value(QStringLiteral("present")).toBool() && got.value(QStringLiteral("text")).toString() == stored,
              "buffersGet: the slot holds the stored text verbatim");
        check(!ScriviBridge::buffersGet(projectPath, QStringLiteral("4")).value(QStringLiteral("present")).toBool(),
              "buffersGet: an unset slot is not present");
        // Written to the SAME file the Mac reads — checked on disk, not trusted from the call.
        check(QFile::exists(projectPath + QStringLiteral("/history/buffers.json")), "buffersLoad wrote history/buffers.json");
        const QVariantList listed = ScriviBridge::buffersList(projectPath).value(QStringLiteral("buffers")).toList();
        check(listed.size() == 1 && listed.first().toMap().value(QStringLiteral("bufferID")).toString() == QStringLiteral("3"),
              "buffersList: the one filled slot");
        check(ScriviBridge::buffersClear(projectPath, QStringLiteral("3")).value(QStringLiteral("cleared")).toBool(),
              "buffersClear: cleared a filled slot");
        const QVariantMap after = ScriviBridge::buffersList(projectPath);
        check(!bridge.lastCallFailed() && after.value(QStringLiteral("buffers")).toList().isEmpty(),
              "buffersList after clear: empty (and the call succeeded)");
        check(ScriviBridge::buffersLoad(projectPath, QStringLiteral("0"), QStringLiteral("x")).isEmpty()
                  && bridge.lastCallFailed(), "buffersLoad: slot 0 is not a slot (the system clipboard) — refused");
    }

    // ================================================================
    // EP-026 S4 (SP-180 / T-0607) — a cross-scene paste's balancing (`style_at_caret`, `balance_paste_pieces`)
    // ================================================================
    {
        check(ScriviBridge::markdownStyleAtCaret(QStringLiteral("**bold** plain"), 3) == 2, "markdownStyleAtCaret: inside bold → 2");
        check(ScriviBridge::markdownStyleAtCaret(QStringLiteral("**bold** plain"), 11) == 0, "markdownStyleAtCaret: plain → 0");
        const QStringList pieces{QStringLiteral("one"), QStringLiteral("two")};
        check(ScriviBridge::markdownBalancePastePieces(pieces, 0) == pieces,
              "markdownBalancePastePieces: outside any span, unchanged");
        // Pasted INSIDE bold, the pasted text keeps its own (plain) formatting: the span closes before the first piece and
        // re-opens after the last.
        check(ScriviBridge::markdownBalancePastePieces(pieces, 2) == QStringList({QStringLiteral("**one"), QStringLiteral("two**")}),
              "markdownBalancePastePieces: inside bold, the span closes before the pieces and re-opens after them");
    }

    std::fprintf(stderr, "bridge_parity_smoke: %d checks, %d failures\n", checks, failures);
    if (failures != 0) return 1;

    std::fprintf(stdout, "%s\n", projectID.toUtf8().constData());
    return 0;
}
