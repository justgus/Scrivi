// scene_merge_smoke — headless scene/chapter merge check for EP-028 (SP-076, T-0306,
// AC5/AC6/AC7 Linux). No GUI.
//
// Exercises the two merge deliverables through the bridge (the layer EditorShell's merge
// slots call), verified by reopen — the same division as scene_reorder_smoke (which tests
// the bridge, not onSceneDropped, because the live QDropEvent/keyPress path needs a GUI):
// ✅ [SP-191] (Q3e): the merges are RANGE calls now — `scrivi_merge_scene` / `scrivi_merge_chapter` are retired.
//   Case A — a scene merge = ScriviBridge::rangeCut of the zero-length range across the break: the two bodies join
//            DIRECTLY (Q3a, no blank line), the merged scene's files are removed, the survivor keeps its identity; and
//            the cut's own undo (a rangePaste of what it removed) brings the scene back. Reopen: as described.
//   Case B — a chapter merge = ScriviBridge::rangeRelabel of the chapter's scenes into the previous chapter: every scene
//            RELOCATES (nothing lost — the I-0083 fix), the emptied chapter is removed, no text changes; the inverse
//            relabel re-creates the chapter. Reopen: as described.
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-merge.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>
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

// Ordered sceneIDs in manuscript order, from an opened project's scenes[].
QStringList orderedScenes(const QVariantMap& opened)
{
    QStringList out;
    for (const QVariant& v : opened.value(QStringLiteral("scenes")).toList()) {
        out.append(v.toMap().value(QStringLiteral("sceneID")).toString());
    }
    return out;
}

// Distinct chapterIDs in manuscript order.
QStringList orderedChapters(const QVariantMap& opened)
{
    QStringList out;
    for (const QVariant& v : opened.value(QStringLiteral("scenes")).toList()) {
        const QString ch = v.toMap().value(QStringLiteral("chapterID")).toString();
        if (out.isEmpty() || out.last() != ch) {
            if (!out.contains(ch)) {
                out.append(ch);
            }
        }
    }
    return out;
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_merge_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Merge Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Merge Project"),
                             QStringLiteral("scene-merge"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    auto saveBody = [&](const QString& sceneID, const QString& meta,
                        const QString& content, const QString& body) {
        bridge.saveScene(projectID, projectPath, appSupport, sceneID, meta, content,
                         body, 0, 0, 0.0);
    };

    // --- Fixture: chapter 1 {s0, s1, s2}, chapter 2 {s3, s4} --------------------
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QVariantMap firstScene =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString s0 = firstScene.value(QStringLiteral("sceneID")).toString();
    const QString ch1 = firstScene.value(QStringLiteral("chapterID")).toString();
    saveBody(s0, firstScene.value(QStringLiteral("metadataPath")).toString(),
             firstScene.value(QStringLiteral("contentPath")).toString(),
             QStringLiteral("Body of scene zero."));

    const QVariantMap r1 = bridge.createScene(projectPath, appSupport, projectID, ch1, s0);
    const QString s1 = r1.value(QStringLiteral("sceneID")).toString();
    saveBody(s1, r1.value(QStringLiteral("metadataPath")).toString(),
             r1.value(QStringLiteral("contentPath")).toString(),
             QStringLiteral("Body of scene one."));
    const QVariantMap r2 = bridge.createScene(projectPath, appSupport, projectID, ch1, s1);
    const QString s2 = r2.value(QStringLiteral("sceneID")).toString();
    saveBody(s2, r2.value(QStringLiteral("metadataPath")).toString(),
             r2.value(QStringLiteral("contentPath")).toString(),
             QStringLiteral("Body of scene two."));

    const QVariantMap chRes = bridge.createChapter(projectPath, appSupport, projectID);
    const QString ch2 = chRes.value(QStringLiteral("chapterID")).toString();
    const QString s3 = chRes.value(QStringLiteral("firstSceneID")).toString();
    saveBody(s3, chRes.value(QStringLiteral("firstSceneMetadataPath")).toString(),
             chRes.value(QStringLiteral("firstSceneContentPath")).toString(),
             QStringLiteral("Body of scene three."));
    const QVariantMap r4 = bridge.createScene(projectPath, appSupport, projectID, ch2, s3);
    const QString s4 = r4.value(QStringLiteral("sceneID")).toString();
    saveBody(s4, r4.value(QStringLiteral("metadataPath")).toString(),
             r4.value(QStringLiteral("contentPath")).toString(),
             QStringLiteral("Body of scene four."));

    check(!s1.isEmpty() && !s2.isEmpty() && !s3.isEmpty() && !s4.isEmpty()
              && !ch2.isEmpty(),
          "fixture: s1, s2, chapter 2 (s3), s4 created");

    opened = bridge.openProject(projectPath, appSupport);
    check(opened.value(QStringLiteral("scenes")).toList().size() == 5,
          "fixture has 5 scenes across 2 chapters");
    check(orderedScenes(opened) == (QStringList{s0, s1, s2, s3, s4}),
          "fixture order is s0,s1,s2,s3,s4");

    const auto bodyOf = [&](const QString& id) {
        return bridge.openScene(projectPath, appSupport, projectID, id).value(QStringLiteral("markdown")).toString();
    };
    const auto chapterOf = [&](const QVariantMap& re, const QString& id) {
        for (const QVariant& v : re.value(QStringLiteral("scenes")).toList()) {
            if (v.toMap().value(QStringLiteral("sceneID")).toString() == id) {
                return v.toMap().value(QStringLiteral("chapterID")).toString();
            }
        }
        return QString();
    };
    const auto entry = [](const QString& sceneID, const QString& chapterID, qint64 start, qint64 whole) {
        return QJsonObject{{QStringLiteral("sceneID"), sceneID}, {QStringLiteral("chapterID"), chapterID},
                           {QStringLiteral("chapterTitle"), QString()}, {QStringLiteral("sceneTitle"), QString()},
                           {QStringLiteral("start"), start}, {QStringLiteral("length"), 0},
                           {QStringLiteral("sceneLength"), whole}, {QStringLiteral("text"), QString()}};
    };
    const auto rangeJson = [](const QJsonArray& entries) {
        return QString::fromUtf8(QJsonDocument(QJsonObject{{QStringLiteral("schema"), QStringLiteral("scrivi.range.v1")},
                                                           {QStringLiteral("scenes"), entries}})
                                     .toJson(QJsonDocument::Compact));
    };
    const auto jsonOf = [](const QVariant& v) {
        return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(v.toMap())).toJson(QJsonDocument::Compact));
    };

    // ==========================================================================
    // Case A — a scene merge: s1 into s0 — the zero-length range across their break, cut.
    // ==========================================================================
    {
        const qint64 s0Bytes = bodyOf(s0).toUtf8().size();
        const qint64 s1Bytes = bodyOf(s1).toUtf8().size();
        const QVariantMap r = bridge.rangeCut(projectPath, rangeJson(QJsonArray{entry(s0, ch1, s0Bytes, s0Bytes),
                                                                              entry(s1, ch1, 0, s1Bytes)}),
                                              bridge.displayName());
        check(!r.isEmpty(), "A: the cut of the break returned an ok result");
        check(r.value(QStringLiteral("undoCaretSceneID")).toString() == s0, "A: the survivor is s0 (its seam is the undo's)");

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        check(orderedScenes(re) == (QStringList{s0, s2, s3, s4}), "A: reopened order is s0,s2,s3,s4 (s1 absorbed)");
        check(bodyOf(s0) == QStringLiteral("Body of scene zero.Body of scene one."),
              "A: s0 holds both bodies joined DIRECTLY (Q3a)");

        // Its undo: a paste of what it removed, at the survivor's seam — s1 comes back, under its own ID.
        const QVariantMap u = bridge.rangePaste(projectPath, jsonOf(r.value(QStringLiteral("range"))), s0, s0Bytes,
                                                bodyOf(s0), bridge.displayName());
        check(!u.isEmpty(), "A: the undo (a paste of the removed range) returned an ok result");
        check(orderedScenes(bridge.openProject(projectPath, appSupport)) == (QStringList{s0, s1, s2, s3, s4}),
              "A: the undo restored s1, in place, under its own ID");
        check(bodyOf(s0) == QStringLiteral("Body of scene zero.") && bodyOf(s1) == QStringLiteral("Body of scene one."),
              "A: and both bodies as they were");
        // Merge again, so case B starts from s0,s2 | s3,s4.
        check(!bridge.rangeCut(projectPath, rangeJson(QJsonArray{entry(s0, ch1, s0Bytes, s0Bytes),
                                                                 entry(s1, ch1, 0, s1Bytes)}),
                               bridge.displayName()).isEmpty(),
              "A: merged again");
    }

    // ==========================================================================
    // Case B — a chapter merge: chapter 2's scenes (s3, s4) relabelled into chapter 1. Nothing lost (I-0083).
    // ==========================================================================
    {
        const QVariantMap re0 = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(re0).size() == 2, "B: two chapters before the merge");

        const QVariantMap r = bridge.rangeRelabel(projectPath, rangeJson(QJsonArray{entry(s3, ch2, 0, 0), entry(s4, ch2, 0, 0)}),
                                                  ch1, bridge.displayName());
        check(!r.isEmpty(), "B: the relabel returned an ok result");
        check(r.value(QStringLiteral("fromChapterID")).toString() == ch2, "B: it reports the chapter the scenes left");

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(re).size() == 1 && orderedChapters(re).at(0) == ch1, "B: only chapter 1 remains");
        check(orderedScenes(re) == (QStringList{s0, s2, s3, s4}), "B: every scene survived, in order (I-0083 — nothing lost)");
        check(chapterOf(re, s3) == ch1 && chapterOf(re, s4) == ch1, "B: the relocated scenes belong to chapter 1");
        check(bodyOf(s3) == QStringLiteral("Body of scene three.") && bodyOf(s4) == QStringLiteral("Body of scene four."),
              "B: bodies intact after the relocation");

        // Its undo: the inverse relabel re-creates chapter 2 (from the sidecar the result carries).
        const QVariantMap u = bridge.rangeRelabel(projectPath, jsonOf(r.value(QStringLiteral("range"))), ch2,
                                                  bridge.displayName());
        check(!u.isEmpty(), "B: the inverse relabel returned an ok result");
        const QVariantMap back = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(back) == (QStringList{ch1, ch2}), "B: chapter 2 is back, under its own ID");
        check(chapterOf(back, s3) == ch2 && chapterOf(back, s4) == ch2, "B: with its scenes");
    }

    if (failures == 0) {
        std::fprintf(stderr, "OK: scene merge + chapter merge (I-0083) persistence passed.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
