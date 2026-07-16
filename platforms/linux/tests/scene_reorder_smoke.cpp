// scene_reorder_smoke — headless scene/chapter reorder + I-0064 split + I-0063
// renumber check for EP-023 (SP-067, T-0258…T-0263, AC4 + I-0063/I-0064). No GUI.
//
// Exercises the three deliverables that share the reorder orchestration:
//   1. SceneDocument::moveScene splice (pure, no I/O): within-chapter move, cross-chapter
//      move (source/target/after correct), chapter-first re-promotion when the moved
//      scene vacated a chapter's first slot, and reflowAllChapterHeadings renumbering —
//      all with the offset map staying well-formed (no rebuild).
//   2. ScriviBridge::reorderScene / reorderChapter persistence: the moves land on disk in
//      the new manuscript order, verified by reopen.
//   3. I-0064 split orchestration (bridge-level replay of EditorShell's steps):
//      create_chapter → reorder_chapter(K, afterC) → reorder_scene(followers → K) →
//      save head/tail, for BOTH the end-of-scene and mid-scene cases; then
//      I-0063 renumber (rename created "Chapter N" chapters to their new ordinal).
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-reorder.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include "AppSupport.hpp"
#include "SceneDocument.hpp"
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

// Assemble a SceneDocument from an opened project's scenes[] (active body inline,
// others via open_scene) — the same assembly EditorShell::load does.
void assemble(SceneDocument& doc, ScriviBridge& bridge, const QString& projectPath,
              const QString& appSupport, const QString& projectID,
              const QVariantMap& opened)
{
    const QVariantList scenes = opened.value(QStringLiteral("scenes")).toList();
    const QVariantMap active = opened.value(QStringLiteral("activeScene")).toMap();
    const QString activeID = active.value(QStringLiteral("sceneID")).toString();
    const QString activeMd = active.value(QStringLiteral("markdown")).toString();

    QList<SceneDocument::Input> in;
    for (const QVariant& v : scenes) {
        const QVariantMap s = v.toMap();
        const QString id = s.value(QStringLiteral("sceneID")).toString();
        SceneDocument::Input i;
        i.sceneID      = id;
        i.chapterID    = s.value(QStringLiteral("chapterID")).toString();
        i.title        = s.value(QStringLiteral("title")).toString();
        i.chapterTitle = s.value(QStringLiteral("chapterTitle")).toString();
        i.slug         = s.value(QStringLiteral("slug")).toString();
        i.metadataPath = s.value(QStringLiteral("metadataPath")).toString();
        i.contentPath  = s.value(QStringLiteral("contentPath")).toString();
        i.chapterMetadataPath = s.value(QStringLiteral("chapterMetadataPath")).toString();
        i.markdown = (id == activeID)
                         ? activeMd
                         : bridge.openScene(projectPath, appSupport, projectID, id)
                               .value(QStringLiteral("markdown")).toString();
        in.append(i);
    }
    doc.build(in);
}

// Assert the offset map matches the document text for every segment: each body's
// recorded [bodyStart, bodyStart+bodyLength) really holds that scene's text, bodies are
// strictly ordered. This is the invariant a splice must preserve.
void checkMapWellFormed(const SceneDocument& doc, const char* stage)
{
    int prevEnd = -1;
    for (int i = 0; i < doc.segments().size(); ++i) {
        const SceneSegment& s = doc.segments().at(i);
        char msg[160];
        std::snprintf(msg, sizeof(msg),
                      "[%s] segment %d bodyStart >= previous body end", stage, i);
        check(s.bodyStart > prevEnd, msg);
        std::snprintf(msg, sizeof(msg),
                      "[%s] segment %d bodyLength matches extracted text", stage, i);
        check(doc.bodyText(i).length() == s.bodyLength, msg);
        prevEnd = s.bodyStart + s.bodyLength;
    }
}

// Distinct chapterIDs in manuscript order, from an opened project's scenes[].
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

// Ordered sceneIDs in manuscript order.
QStringList orderedScenes(const QVariantMap& opened)
{
    QStringList out;
    for (const QVariant& v : opened.value(QStringLiteral("scenes")).toList()) {
        out.append(v.toMap().value(QStringLiteral("sceneID")).toString());
    }
    return out;
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_reorder_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Reorder Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Reorder Project"),
                             QStringLiteral("scene-reorder"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // --- Fixture: chapter 1 {s0, s1, s2}, chapter 2 {s3, s4} --------------------
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QVariantMap firstScene =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString s0 = firstScene.value(QStringLiteral("sceneID")).toString();
    const QString ch1 = firstScene.value(QStringLiteral("chapterID")).toString();

    // Give each scene a distinct body so a mis-splice is caught by checkMapWellFormed.
    auto saveBody = [&](const QString& sceneID, const QString& meta,
                        const QString& content, const QString& body) {
        bridge.saveScene(projectID, projectPath, appSupport, sceneID, meta, content,
                         body, 0, 0, 0.0);
    };
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
    // Case A — SceneDocument::moveScene WITHIN chapter 1: move s0 to after s2.
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        check(doc.segments().size() == 5, "A: assembled 5 segments");
        checkMapWellFormed(doc, "A/pre");

        const int fromIdx = doc.sceneIndexForScene(s0);
        const int newIdx = doc.moveScene(fromIdx, ch1, QString(), s2);   // after s2
        check(newIdx >= 0, "A: moveScene returned a valid index");
        check(doc.sceneIndexForScene(s0) == doc.sceneIndexForScene(s2) + 1,
              "A: s0 now sits right after s2");
        // s1 was chapter 1's mid scene; after s0 left the first slot it is promoted.
        check(doc.segments().at(0).sceneID == s1, "A: s1 is now chapter 1's first scene");
        check(doc.bodyText(doc.sceneIndexForScene(s0)) ==
                  QStringLiteral("Body of scene zero."),
              "A: s0 kept its body after the move");
        checkMapWellFormed(doc, "A/post");
    }

    // ==========================================================================
    // Case B — SceneDocument::moveScene ACROSS chapters: move s0 into chapter 2 as
    //          its first scene (afterSceneID empty). s1 is promoted to chapter 1's
    //          first scene; s0 gains chapter 2's heading.
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        const int fromIdx = doc.sceneIndexForScene(s0);
        const int newIdx = doc.moveScene(fromIdx, ch2, QString(), QString());   // first in ch2
        check(newIdx >= 0, "B: cross-chapter moveScene returned a valid index");
        check(doc.segments().at(newIdx).chapterID == ch2,
              "B: s0 now belongs to chapter 2");
        check(doc.segments().at(0).sceneID == s1, "B: s1 promoted to chapter 1 first");
        // s0 as chapter 2's first scene must be BEFORE s3 (the old chapter-2 first).
        check(doc.sceneIndexForScene(s0) < doc.sceneIndexForScene(s3),
              "B: s0 precedes s3 in chapter 2");
        checkMapWellFormed(doc, "B/post");
    }

    // ==========================================================================
    // Case C — moveScene persistence via the bridge: reorder s2 into chapter 2 after
    //          s3, reopen, assert the on-disk order changed accordingly.
    // ==========================================================================
    {
        const QVariantMap r =
            bridge.reorderScene(projectPath, s2, ch1, ch2, s3);   // ch2, after s3
        check(!r.isEmpty(), "C: reorderScene returned an ok result");

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        const QStringList order = orderedScenes(re);
        // Expected new order: s0, s1 (ch1), s3, s2, s4 (ch2) — s2 sits right after s3.
        check(order == (QStringList{s0, s1, s3, s2, s4}),
              "C: reopened order is s0,s1,s3,s2,s4 after the cross-chapter reorder");
        // And s2 now reports chapter 2.
        for (const QVariant& v : re.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap m = v.toMap();
            if (m.value(QStringLiteral("sceneID")).toString() == s2) {
                check(m.value(QStringLiteral("chapterID")).toString() == ch2,
                      "C: s2 is now in chapter 2 on disk");
            }
        }
    }

    // Restore s2 to chapter 1 (after s1) so the split cases below start from a known
    // 2-chapter layout: ch1 {s0,s1,s2}, ch2 {s3,s4}.
    bridge.reorderScene(projectPath, s2, ch2, ch1, s1);
    opened = bridge.openProject(projectPath, appSupport);
    check(orderedScenes(opened) == (QStringList{s0, s1, s2, s3, s4}),
          "restore: order back to s0,s1,s2,s3,s4");

    // ==========================================================================
    // Case D — I-0064 END-OF-SCENE split: caret at end of s0 in chapter 1. The
    //          followers {s1, s2} move into a NEW chapter K inserted right after ch1;
    //          K's blank first scene is dropped, so K = {s1, s2}. Replays the bridge
    //          steps EditorShell::onCreateChapterRequested runs.
    // ==========================================================================
    {
        const QVariantMap kRes =
            bridge.createChapter(projectPath, appSupport, projectID);
        const QString kCh = kRes.value(QStringLiteral("chapterID")).toString();
        const QString k0  = kRes.value(QStringLiteral("firstSceneID")).toString();
        check(!kCh.isEmpty() && !k0.isEmpty(), "D: created chapter K + blank K0");

        bridge.reorderChapter(projectPath, kCh, ch1);   // K right after ch1

        // followers of s0 within ch1 = {s1, s2}; reassign into K in order.
        QString afterID;   // empty → first follower becomes K's first
        for (const QString& f : QStringList{s1, s2}) {
            bridge.reorderScene(projectPath, f, ch1, kCh, afterID);
            afterID = f;
        }
        bridge.deleteScene(projectPath, k0);   // drop the redundant blank

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        const QStringList sc = orderedScenes(re);
        const QStringList ch = orderedChapters(re);
        // ch1 {s0}, K {s1,s2}, ch2 {s3,s4}
        check(sc == (QStringList{s0, s1, s2, s3, s4}),
              "D: scene order preserved (s0 | s1,s2 | s3,s4)");
        check(ch.size() == 3, "D: now 3 chapters (K inserted after ch1)");
        check(ch.at(0) == ch1 && ch.at(1) == kCh && ch.at(2) == ch2,
              "D: chapter order is ch1, K, ch2");
        check(!sc.contains(k0), "D: the blank K0 was dropped");
        // s1, s2 now belong to K.
        for (const QVariant& v : re.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap m = v.toMap();
            const QString id = m.value(QStringLiteral("sceneID")).toString();
            if (id == s1 || id == s2) {
                check(m.value(QStringLiteral("chapterID")).toString() == kCh,
                      "D: follower reassigned into chapter K");
            }
        }

        // --- I-0063 renumber: any created ("Chapter N") chapter whose ordinal shifted
        //     is renamed to its new position. K sits at ordinal 2; ch2 is now ordinal 3.
        //     Walk chapters, rename auto-pattern titles to their ordinal, reopen, assert.
        static const QRegularExpression autoPattern(QStringLiteral("^Chapter \\d+$"));
        const QVariantList scenes2 = re.value(QStringLiteral("scenes")).toList();
        int ordinal = 0;
        QString lastCh;
        for (const QVariant& v : scenes2) {
            const QVariantMap m = v.toMap();
            const QString chID = m.value(QStringLiteral("chapterID")).toString();
            if (chID == lastCh) {
                continue;
            }
            lastCh = chID;
            ++ordinal;
            const QString stored = m.value(QStringLiteral("chapterTitle")).toString();
            const QString metaPath =
                m.value(QStringLiteral("chapterMetadataPath")).toString();
            if (autoPattern.match(stored).hasMatch()
                && stored != QStringLiteral("Chapter %1").arg(ordinal)
                && !metaPath.isEmpty()) {
                bridge.renameChapter(projectPath, metaPath,
                                     QStringLiteral("Chapter %1").arg(ordinal));
            }
        }
        const QVariantMap re2 = bridge.openProject(projectPath, appSupport);
        // Every chapter's stored title (if auto-pattern) equals its ordinal.
        int ord2 = 0;
        QString last2;
        for (const QVariant& v : re2.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap m = v.toMap();
            const QString chID = m.value(QStringLiteral("chapterID")).toString();
            if (chID == last2) {
                continue;
            }
            last2 = chID;
            ++ord2;
            const QString stored = m.value(QStringLiteral("chapterTitle")).toString();
            if (autoPattern.match(stored).hasMatch()) {
                char msg[96];
                std::snprintf(msg, sizeof(msg),
                              "D/I-0063: chapter %d stored title is 'Chapter %d'",
                              ord2, ord2);
                check(stored == QStringLiteral("Chapter %1").arg(ord2), msg);
            }
        }
    }

    // ==========================================================================
    // Case E — I-0064 MID-SCENE split, verified as a SceneDocument splice (head/tail):
    //          moveScene isn't the split, but the split's tail lands as K's first scene
    //          and the head stays — here we assert the DOCUMENT-level effect via a fresh
    //          assemble + the bridge state from Case D (s0 | s1,s2 | s3,s4). We split s0
    //          mid-body: head "Body of " stays in s0, tail "scene zero." into a new
    //          chapter's first scene. This asserts the bridge save+reorder produce the
    //          right head/tail on disk (the app layer's reload then shows them).
    // ==========================================================================
    {
        // Re-read s0's current body (it survived Case D untouched).
        QVariantMap re = bridge.openProject(projectPath, appSupport);
        QString s0Meta, s0Content;
        for (const QVariant& v : re.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap m = v.toMap();
            if (m.value(QStringLiteral("sceneID")).toString() == s0) {
                s0Meta    = m.value(QStringLiteral("metadataPath")).toString();
                s0Content = m.value(QStringLiteral("contentPath")).toString();
            }
        }
        const QString body =
            bridge.openScene(projectPath, appSupport, projectID, s0)
                .value(QStringLiteral("markdown")).toString();
        check(body == QStringLiteral("Body of scene zero."), "E: s0 body intact pre-split");

        const int splitAt = QStringLiteral("Body of ").length();
        const QString head = body.left(splitAt);
        const QString tail = body.mid(splitAt);

        const QVariantMap kRes =
            bridge.createChapter(projectPath, appSupport, projectID);
        const QString kCh = kRes.value(QStringLiteral("chapterID")).toString();
        const QString k0  = kRes.value(QStringLiteral("firstSceneID")).toString();
        bridge.reorderChapter(projectPath, kCh, ch1);   // ch1 is s0's chapter now

        // mid-scene: head into s0, tail into k0 (K's first scene). No followers of s0 in
        // ch1 (s0 is ch1's only scene after Case D).
        bridge.saveScene(projectID, projectPath, appSupport, s0, s0Meta, s0Content,
                         head, 0, 0, 0.0);
        bridge.saveScene(projectID, projectPath, appSupport, k0,
                         kRes.value(QStringLiteral("firstSceneMetadataPath")).toString(),
                         kRes.value(QStringLiteral("firstSceneContentPath")).toString(),
                         tail, 0, 0, 0.0);

        re = bridge.openProject(projectPath, appSupport);
        const QString s0After =
            bridge.openScene(projectPath, appSupport, projectID, s0)
                .value(QStringLiteral("markdown")).toString();
        const QString k0After =
            bridge.openScene(projectPath, appSupport, projectID, k0)
                .value(QStringLiteral("markdown")).toString();
        check(s0After == QStringLiteral("Body of "), "E: head stayed in s0");
        check(k0After == QStringLiteral("scene zero."), "E: tail landed in K's first scene");
        // K sits right after ch1 (s0's chapter).
        const QStringList ch = orderedChapters(re);
        const int i1 = ch.indexOf(ch1);
        const int iK = ch.indexOf(kCh);
        check(i1 >= 0 && iK == i1 + 1, "E: new chapter K sits right after s0's chapter");
    }

    if (failures == 0) {
        std::fprintf(stderr, "OK: scene reorder splice + persistence + I-0064 split + "
                             "I-0063 renumber all passed.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
