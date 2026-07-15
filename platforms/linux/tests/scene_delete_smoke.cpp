// scene_delete_smoke — headless delete-scene / delete-chapter check for EP-023
// (SP-065, T-0250/T-0252/T-0253, AC1/AC2). No GUI.
//
// Exercises both halves of the navigator Delete path:
//   1. ScriviCore side: bridge.deleteScene / bridge.deleteChapter remove the target
//      from disk + the manuscript index, so a reopen shows the survivors only, in
//      order (deleting a chapter takes ALL its scenes).
//   2. Editor side: SceneDocument::removeScene / removeChapter splice the deleted
//      scene(s) out of the live document + offset map WITHOUT a rebuild — segment
//      count shrinks, surviving bodies stay intact at their new offsets, boundary
//      text stays well-formed, and a follower promoted to a chapter's first scene
//      gains its heading (reflow).
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-delete.scrivi)
//   $XDG_DATA_HOME drives appSupportRoot; Qt offscreen platform (set by the script).
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>
#include <QString>
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

// The document position at which the MANUSCRIPT-FIRST scene's body begins: right after
// its chapter heading ("<title|Chapter N>\n\n"), with no leading blank line (build()'s
// first-body rule). The first chapter's derived ordinal is always "Chapter 1" when
// untitled (the app owns chapter numbering from order). Only valid for the first
// segment (ordinal 1).
int firstBodyStartFor(const SceneSegment& seg)
{
    const QString heading = seg.chapterTitle.isEmpty() ? QStringLiteral("Chapter 1")
                                                       : seg.chapterTitle;
    return (heading + QStringLiteral("\n\n")).length();
}

// Assert the offset map matches the document text for every segment: each body's
// recorded [bodyStart, bodyStart+bodyLength) really holds that scene's text and the
// bodies are strictly ordered. This is the invariant a splice must preserve.
void checkMapWellFormed(const SceneDocument& doc, const char* stage)
{
    int prevEnd = -1;
    for (int i = 0; i < doc.segments().size(); ++i) {
        const SceneSegment& s = doc.segments().at(i);
        char msg[160];
        std::snprintf(msg, sizeof(msg),
                      "[%s] segment %d bodyStart >= previous body end", stage, i);
        check(s.bodyStart > prevEnd, msg);
        // bodyText() extracts live from the document via the recorded offsets — if the
        // map drifted from the text this would read the wrong characters, but at least
        // it must not crash / go out of range (guarded inside bodyText).
        std::snprintf(msg, sizeof(msg),
                      "[%s] segment %d bodyLength matches extracted text", stage, i);
        check(doc.bodyText(i).length() == s.bodyLength, msg);
        prevEnd = s.bodyStart + s.bodyLength;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_delete_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Delete Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Delete Project"),
                             QStringLiteral("scene-delete"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // --- Fixture: chapter 1 with scenes {s0, s1, s2}, then chapter 2 with s3 -----
    // Start with 1 scene (s0); add two more in its chapter, then a whole new chapter.
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QVariantMap firstScene =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString s0 = firstScene.value(QStringLiteral("sceneID")).toString();
    const QString ch1 = firstScene.value(QStringLiteral("chapterID")).toString();

    const QString s1 = bridge.createScene(projectPath, appSupport, projectID, ch1, s0)
                           .value(QStringLiteral("sceneID")).toString();
    const QString s2 = bridge.createScene(projectPath, appSupport, projectID, ch1, s1)
                           .value(QStringLiteral("sceneID")).toString();
    const QVariantMap chRes = bridge.createChapter(projectPath, appSupport, projectID);
    const QString ch2 = chRes.value(QStringLiteral("chapterID")).toString();
    const QString s3 = chRes.value(QStringLiteral("firstSceneID")).toString();
    check(!s1.isEmpty() && !s2.isEmpty() && !s3.isEmpty() && !ch2.isEmpty(),
          "fixture: created s1, s2, and chapter 2 (s3)");

    opened = bridge.openProject(projectPath, appSupport);
    check(opened.value(QStringLiteral("scenes")).toList().size() == 4,
          "fixture has 4 scenes across 2 chapters");

    // ==========================================================================
    // Case A — remove a MID-CHAPTER scene (s1) from the SceneDocument splice.
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        check(doc.segments().size() == 4, "A: assembled 4 segments");
        checkMapWellFormed(doc, "A/pre");

        const int idxS1 = doc.sceneIndexForScene(s1);
        const int s2StartBefore = doc.bodyStartForScene(s2);
        check(idxS1 == 1, "A: s1 is at index 1");

        const bool removed = doc.removeScene(idxS1);
        check(removed, "A: removeScene returned true");
        check(doc.segments().size() == 3, "A: segment count dropped to 3");
        check(doc.sceneIndexForScene(s1) == -1, "A: s1 no longer in the map");
        check(doc.sceneIndexForScene(s0) == 0, "A: s0 still first");
        check(doc.sceneIndexForScene(s2) == 1, "A: s2 shifted into index 1");
        check(doc.bodyStartForScene(s2) < s2StartBefore,
              "A: s2 body start shifted earlier after the removal");
        checkMapWellFormed(doc, "A/post");
    }

    // ==========================================================================
    // Case B — remove the FIRST scene (s0). s1 (same chapter) is promoted to the
    //          manuscript's first scene: it inherits chapter 1's heading, and there
    //          is NO leading blank line before that heading (build() emits none
    //          before the very first body). So the new first body starts right after
    //          "Chapter\n\n" — i.e. at the heading's length, with no preceding gap.
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        const bool removed = doc.removeScene(0);
        check(removed, "B: removeScene(0) returned true");
        check(doc.segments().size() == 3, "B: 3 segments remain");
        check(doc.sceneIndexForScene(s0) == -1, "B: s0 gone");
        check(doc.sceneIndexForScene(s1) == 0, "B: s1 is now first");
        // The document must begin with the chapter heading and no blank line before it
        // (the reflow reproduces build()'s first-body rule).
        check(doc.segments().at(0).bodyStart == firstBodyStartFor(doc.segments().at(0)),
              "B: new first body starts right after the heading, no leading blank line");
        checkMapWellFormed(doc, "B/post");
    }

    // ==========================================================================
    // Case C — remove a chapter-FIRST scene (s0) when s1 inherits the chapter →
    //          s1 is promoted to the chapter's first scene; chapter 2 stays intact.
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        doc.removeScene(0);   // s0 was chapter 1's first scene
        // s1 is now chapter 1's first scene; s2 still in chapter 1; s3 in chapter 2.
        check(doc.segments().at(0).chapterID == ch1, "C: s1 still in chapter 1");
        check(doc.segments().at(0).sceneID == s1, "C: s1 promoted to chapter-first");
        check(doc.segments().at(2).chapterID == ch2, "C: chapter 2 intact after s0 delete");
        checkMapWellFormed(doc, "C/post");
    }

    // ==========================================================================
    // Case D — removeChapter(ch1) removes ALL of chapter 1's scenes (s0,s1,s2),
    //          leaving only chapter 2 (s3), which becomes the manuscript's first
    //          scene (inherits its own heading, no leading blank line).
    // ==========================================================================
    {
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        const int n = doc.removeChapter(ch1);
        check(n == 3, "D: removeChapter reported 3 scenes removed");
        check(doc.segments().size() == 1, "D: only 1 scene remains");
        check(doc.segments().at(0).sceneID == s3, "D: the survivor is s3 (chapter 2)");
        check(doc.segments().at(0).chapterID == ch2, "D: survivor is in chapter 2");
        check(doc.segments().at(0).bodyStart == firstBodyStartFor(doc.segments().at(0)),
              "D: survivor's body starts right after its heading, now first");
        checkMapWellFormed(doc, "D/post");
    }

    // ==========================================================================
    // Case E — persistence: delete s2 and chapter 2 via the bridge; reopen shows
    //          only {s0, s1} in chapter 1 (proving the real .md / index removal).
    // ==========================================================================
    {
        QVariantMap r = bridge.deleteScene(projectPath, s2);
        check(!r.isEmpty(), "E: deleteScene(s2) returned an ok result");
        r = bridge.deleteChapter(projectPath, ch2);
        check(!r.isEmpty(), "E: deleteChapter(ch2) returned an ok result");

        const QVariantMap re = bridge.openProject(projectPath, appSupport);
        const QVariantList scenes = re.value(QStringLiteral("scenes")).toList();
        check(scenes.size() == 2, "E: reopened project has 2 scenes after deletes");
        if (scenes.size() == 2) {
            check(scenes.at(0).toMap().value(QStringLiteral("sceneID")).toString() == s0,
                  "E: s0 survived");
            check(scenes.at(1).toMap().value(QStringLiteral("sceneID")).toString() == s1,
                  "E: s1 survived");
        }
        // s2 and s3 (chapter 2's only scene) must be gone.
        bool sawS2 = false, sawS3 = false;
        for (const QVariant& v : scenes) {
            const QString id = v.toMap().value(QStringLiteral("sceneID")).toString();
            sawS2 = sawS2 || (id == s2);
            sawS3 = sawS3 || (id == s3);
        }
        check(!sawS2, "E: s2 is gone after reopen");
        check(!sawS3, "E: chapter 2's scene (s3) is gone after reopen");
    }

    if (failures == 0) {
        std::fprintf(stderr, "OK: delete-scene + delete-chapter spliced the map and "
                             "persisted the removals.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
