// chapter_reorder_smoke — headless chapter drag-reorder check for EP-023
// (SP-073, T-0294…T-0296, AC5). No GUI.
//
// Exercises the three pieces that make a chapter move as a container:
//   1. SceneDocument::moveChapter splice (pure, no I/O): the chapter block — heading +
//      every member scene, bodies intact, member order preserved — moves between other
//      chapters, to the manuscript front, and to the end, with the offset map staying
//      well-formed (no rebuild) and no-op landings refused.
//   2. ScriviBridge::reorderChapter persistence: the move lands on disk in the new
//      manuscript order, verified by reopen, with every scene body intact.
//   3. The SP-073 stale-path refresh (the I-0074/I-0079 class): scrivi_reorder_chapter
//      RESLUGS the moved chapter's folder, so paths captured before the call are stale.
//      The reorder envelope reports the chapter's new metadataPath;
//      SceneDocument::refreshChapterPaths re-bases the members' paths on it, and a
//      save through the REFRESHED paths must land (a save through the stale ones
//      would write into the vanished folder).
//
//   argv[1] = project dir to create (e.g. <tmp>/chapter-reorder.scrivi)
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
// recorded [bodyStart, bodyStart+bodyLength) really holds that scene's text, bodies
// are strictly ordered. This is the invariant a splice must preserve.
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
        std::fprintf(stderr, "usage: chapter_reorder_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Chapter Reorder Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Chapter Reorder Project"),
                             QStringLiteral("chapter-reorder"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // --- Fixture: ch1 {s0, s1}, ch2 {s2}, ch3 {s3} ------------------------------
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QVariantMap firstScene =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString s0 = firstScene.value(QStringLiteral("sceneID")).toString();
    const QString ch1 = firstScene.value(QStringLiteral("chapterID")).toString();

    auto saveBody = [&](const QString& sceneID, const QString& meta,
                        const QString& content, const QString& body) {
        bridge.saveScene(projectID, projectPath, appSupport, sceneID, meta, content,
                         body, 0, 0, 0.0);
    };
    auto bodyOnDisk = [&](const QString& sceneID) {
        return bridge.openScene(projectPath, appSupport, projectID, sceneID)
            .value(QStringLiteral("markdown")).toString();
    };

    saveBody(s0, firstScene.value(QStringLiteral("metadataPath")).toString(),
             firstScene.value(QStringLiteral("contentPath")).toString(),
             QStringLiteral("Body of scene zero."));

    const QVariantMap r1 = bridge.createScene(projectPath, appSupport, projectID, ch1, s0);
    const QString s1 = r1.value(QStringLiteral("sceneID")).toString();
    saveBody(s1, r1.value(QStringLiteral("metadataPath")).toString(),
             r1.value(QStringLiteral("contentPath")).toString(),
             QStringLiteral("Body of scene one."));

    const QVariantMap ch2Res = bridge.createChapter(projectPath, appSupport, projectID);
    const QString ch2 = ch2Res.value(QStringLiteral("chapterID")).toString();
    const QString s2 = ch2Res.value(QStringLiteral("firstSceneID")).toString();
    saveBody(s2, ch2Res.value(QStringLiteral("firstSceneMetadataPath")).toString(),
             ch2Res.value(QStringLiteral("firstSceneContentPath")).toString(),
             QStringLiteral("Body of scene two."));

    const QVariantMap ch3Res = bridge.createChapter(projectPath, appSupport, projectID);
    const QString ch3 = ch3Res.value(QStringLiteral("chapterID")).toString();
    const QString s3 = ch3Res.value(QStringLiteral("firstSceneID")).toString();
    saveBody(s3, ch3Res.value(QStringLiteral("firstSceneMetadataPath")).toString(),
             ch3Res.value(QStringLiteral("firstSceneContentPath")).toString(),
             QStringLiteral("Body of scene three."));

    check(!s1.isEmpty() && !ch2.isEmpty() && !s2.isEmpty()
              && !ch3.isEmpty() && !s3.isEmpty(),
          "fixture: s1, chapter 2 (s2), chapter 3 (s3) created");

    opened = bridge.openProject(projectPath, appSupport);
    check(orderedChapters(opened) == (QStringList{ch1, ch2, ch3}),
          "fixture chapter order is ch1,ch2,ch3");
    check(orderedScenes(opened) == (QStringList{s0, s1, s2, s3}),
          "fixture scene order is s0,s1,s2,s3");

    // ==========================================================================
    // Case A — SceneDocument::moveChapter splice (pure): move ch1 after ch2 →
    //          ch2,ch1,ch3; the block keeps its member order + bodies; then move
    //          ch3 to the front (afterChapterID empty) → ch3,ch2,ch1; no-op and
    //          bad-anchor moves are refused.
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        check(doc.segments().size() == 4, "A: assembled 4 segments");
        checkMapWellFormed(doc, "A/pre");

        const int newIdx = doc.moveChapter(ch1, ch2);
        check(newIdx >= 0, "A: moveChapter(ch1 after ch2) returned a valid index");
        check(doc.segments().at(0).chapterID == ch2, "A: ch2 now leads the manuscript");
        check(doc.segments().at(newIdx).sceneID == s0
                  && doc.segments().at(newIdx + 1).sceneID == s1,
              "A: ch1's block kept its member order (s0, s1)");
        check(doc.segments().last().chapterID == ch3, "A: ch3 still last");
        check(doc.bodyText(doc.sceneIndexForScene(s0))
                      == QStringLiteral("Body of scene zero.")
                  && doc.bodyText(doc.sceneIndexForScene(s1))
                      == QStringLiteral("Body of scene one."),
              "A: moved bodies intact");
        checkMapWellFormed(doc, "A/mid");

        const int frontIdx = doc.moveChapter(ch3, QString());
        check(frontIdx == 0, "A: moveChapter(ch3 to front) put its first scene at 0");
        check(doc.segments().at(0).chapterID == ch3, "A: ch3 now leads");
        checkMapWellFormed(doc, "A/post");

        // Refusals: landing where it already is, after itself, unknown anchor.
        check(doc.moveChapter(ch3, QString()) == -1, "A: front-to-front is a no-op");
        check(doc.moveChapter(ch2, ch3) == -1,
              "A: landing after the current predecessor is a no-op");
        check(doc.moveChapter(ch2, ch2) == -1, "A: can't land after itself");
        check(doc.moveChapter(ch2, QStringLiteral("chapter_nope")) == -1,
              "A: unknown anchor is refused");
        checkMapWellFormed(doc, "A/refusals");
    }

    // ==========================================================================
    // Case B — persistence + stale-path refresh: reorderChapter(ch1 after ch2) on
    //          disk. The envelope reports ch1's post-reslug metadataPath; refresh a
    //          SceneDocument's captured paths from it and save s0 THROUGH THE
    //          REFRESHED PATHS (a save through the stale pre-reorder paths would
    //          write into the vanished folder — the I-0074/I-0079 class). Reopen
    //          asserts the order AND the new body.
    // ==========================================================================
    {
        // Assemble BEFORE the reorder so the captured paths are the stale ones.
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        const QString stalePath =
            doc.segments().at(doc.sceneIndexForScene(s0)).contentPath;

        const QVariantMap r = bridge.reorderChapter(projectPath, ch1, ch2);
        check(!r.isEmpty(), "B: reorderChapter returned an ok result");
        const QString newChapterMeta =
            r.value(QStringLiteral("metadataPath")).toString();
        check(!newChapterMeta.isEmpty(),
              "B: reorder envelope reports the chapter's new metadataPath");

        // Mirror EditorShell::onChapterDropped: re-base the moved chapter's paths.
        doc.refreshChapterPaths(ch1, newChapterMeta);
        const SceneSegment seg =
            doc.segments().at(doc.sceneIndexForScene(s0));
        check(seg.contentPath != stalePath,
              "B: refresh re-based s0's contentPath off the reslugged folder");
        check(seg.chapterMetadataPath == newChapterMeta,
              "B: refresh applied the chapter's new sidecar path");

        // Save through the REFRESHED paths and prove it landed.
        saveBody(s0, seg.metadataPath, seg.contentPath,
                 QStringLiteral("Body of scene zero, revised."));

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(re) == (QStringList{ch2, ch1, ch3}),
              "B: reopened chapter order is ch2,ch1,ch3");
        check(orderedScenes(re) == (QStringList{s2, s0, s1, s3}),
              "B: reopened scene order is s2,s0,s1,s3");
        check(bodyOnDisk(s0) == QStringLiteral("Body of scene zero, revised."),
              "B: save through the refreshed paths landed");
        check(bodyOnDisk(s1) == QStringLiteral("Body of scene one.")
                  && bodyOnDisk(s2) == QStringLiteral("Body of scene two.")
                  && bodyOnDisk(s3) == QStringLiteral("Body of scene three."),
              "B: every other body intact after the reorder");
    }

    // ==========================================================================
    // Case C — move-to-front and move-to-last persistence: ch3 to the manuscript
    //          front (afterChapterID empty), then ch2 to the end. Reopen each time.
    // ==========================================================================
    {
        const QVariantMap rFront = bridge.reorderChapter(projectPath, ch3, QString());
        check(!rFront.isEmpty(), "C: reorderChapter to the front returned ok");
        QVariantMap re = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(re) == (QStringList{ch3, ch2, ch1}),
              "C: reopened chapter order is ch3,ch2,ch1 after move-to-front");

        const QVariantMap rLast = bridge.reorderChapter(projectPath, ch2, ch1);
        check(!rLast.isEmpty(), "C: reorderChapter to the end returned ok");
        re = bridge.openProject(projectPath, appSupport);
        check(orderedChapters(re) == (QStringList{ch3, ch1, ch2}),
              "C: reopened chapter order is ch3,ch1,ch2 after move-to-last");
        check(bodyOnDisk(s0) == QStringLiteral("Body of scene zero, revised.")
                  && bodyOnDisk(s1) == QStringLiteral("Body of scene one.")
                  && bodyOnDisk(s2) == QStringLiteral("Body of scene two.")
                  && bodyOnDisk(s3) == QStringLiteral("Body of scene three."),
              "C: every body intact after both moves");
    }

    if (failures == 0) {
        std::fprintf(stderr, "OK: chapter block splice + persistence + stale-path "
                             "refresh all passed.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
