// scene_merge_smoke — headless scene/chapter merge check for EP-028 (SP-076, T-0306,
// AC5/AC6/AC7 Linux). No GUI.
//
// Exercises the two merge deliverables through the bridge (the layer EditorShell's merge
// slots call), verified by reopen — the same division as scene_reorder_smoke (which tests
// the bridge, not onSceneDropped, because the live QDropEvent/keyPress path needs a GUI):
//   Case A — ScriviBridge::mergeScene: a scene merges into the previous scene of the SAME
//            chapter; the two bodies are joined (blank-line), the merged scene's files are
//            removed, and the survivor keeps its identity. Reopen: one fewer scene, the
//            survivor holds both bodies, order otherwise intact.
//   Case B — ScriviBridge::mergeChapter: a whole chapter merges into the previous chapter;
//            every scene RELOCATES into the predecessor (nothing lost — the I-0083 fix),
//            the emptied chapter is removed. Reopen: one fewer chapter, every scene present
//            and in order, bodies intact.
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-merge.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>
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

    // ==========================================================================
    // Case A — mergeScene: merge s1 into s0 (same chapter, s1 is NOT chapter-first).
    //          Survivor s0 keeps its files + gains s1's body; s1's files removed.
    // ==========================================================================
    {
        const QVariantMap r = bridge.mergeScene(projectPath, s1);
        check(!r.isEmpty(), "A: mergeScene returned an ok result");
        check(r.value(QStringLiteral("merged")).toBool(), "A: result reports merged=true");
        // Survivor is the predecessor (s0); the merged (absorbed) scene is s1.
        check(r.value(QStringLiteral("survivorSceneID")).toString() == s0,
              "A: survivor is s0 (the predecessor)");
        check(r.value(QStringLiteral("mergedSceneID")).toString() == s1,
              "A: merged (absorbed) scene is s1");

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        const QStringList sc = orderedScenes(re);
        // s1 is gone; order is s0, s2, s3, s4.
        check(sc == (QStringList{s0, s2, s3, s4}),
              "A: reopened order is s0,s2,s3,s4 (s1 absorbed)");
        check(!sc.contains(s1), "A: s1's files removed (not on disk)");
        // s0 now holds both bodies joined by a blank line.
        const QString s0Body =
            bridge.openScene(projectPath, appSupport, projectID, s0)
                .value(QStringLiteral("markdown")).toString();
        check(s0Body == QStringLiteral("Body of scene zero.\n\nBody of scene one."),
              "A: s0 holds both bodies joined by a blank line");
    }

    // ==========================================================================
    // Case B — mergeChapter: merge chapter 2 into chapter 1. Every scene of ch2
    //          (s3, s4) RELOCATES into ch1 after its last scene; ch2 is removed.
    //          Nothing lost on reopen (I-0083).
    // ==========================================================================
    {
        const QVariantMap re0 = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(re0).size() == 2, "B: two chapters before the merge");

        const QVariantMap r = bridge.mergeChapter(projectPath, ch2);
        check(!r.isEmpty(), "B: mergeChapter returned an ok result");
        check(r.value(QStringLiteral("merged")).toBool(), "B: result reports merged=true");
        check(r.value(QStringLiteral("survivorChapterID")).toString() == ch1,
              "B: survivor chapter is ch1 (the predecessor)");
        check(r.value(QStringLiteral("mergedChapterID")).toString() == ch2,
              "B: merged (absorbed) chapter is ch2");
        check(r.value(QStringLiteral("scenesRelocated")).toInt() == 2,
              "B: two scenes (s3, s4) relocated");

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        const QStringList sc = orderedScenes(re);
        const QStringList ch = orderedChapters(re);
        // One chapter now; every scene present in order: s0, s2, s3, s4.
        check(ch.size() == 1 && ch.at(0) == ch1, "B: only chapter 1 remains");
        check(sc == (QStringList{s0, s2, s3, s4}),
              "B: every scene survived, in order (I-0083 — nothing lost)");
        // Relocated scenes now belong to ch1.
        for (const QVariant& v : re.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap m = v.toMap();
            const QString id = m.value(QStringLiteral("sceneID")).toString();
            if (id == s3 || id == s4) {
                check(m.value(QStringLiteral("chapterID")).toString() == ch1,
                      "B: relocated scene now belongs to chapter 1");
            }
        }
        // Bodies intact after the relocation (files renamed, content preserved).
        check(bridge.openScene(projectPath, appSupport, projectID, s3)
                      .value(QStringLiteral("markdown")).toString()
                  == QStringLiteral("Body of scene three."),
              "B: s3 body intact after relocation");
        check(bridge.openScene(projectPath, appSupport, projectID, s4)
                      .value(QStringLiteral("markdown")).toString()
                  == QStringLiteral("Body of scene four."),
              "B: s4 body intact after relocation");
    }

    if (failures == 0) {
        std::fprintf(stderr, "OK: scene merge + chapter merge (I-0083) persistence passed.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
