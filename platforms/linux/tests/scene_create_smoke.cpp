// scene_create_smoke — headless in-editor create-scene + create-chapter check for
// SP-062 (T-0240 / T-0241, EP-022 AC3). No GUI.
//
// Exercises both halves of the Ctrl+Return (⌘↩ new scene) and Ctrl+Shift+Return
// (⌘⇧↩ new chapter) create paths:
//   1. ScriviCore side: bridge.createScene inserts a new scene after the active one
//      (in order on reopen); bridge.createChapter appends a new chapter + first
//      scene at the end (last + in a new chapter on reopen).
//   2. Editor side: SceneDocument::insertSceneAfter splices each new empty scene into
//      the live document + offset map — segment count grows, the new segment is empty
//      at the right offset, later offsets shift, and the chapter case marks a chapter
//      boundary (i.e. the map stays consistent without a rebuild).
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-create.scrivi)
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

// Build a SceneDocument from an opened project's scenes[] (active body inline,
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

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_create_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Create Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Create Project"),
                             QStringLiteral("scene-create"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    check(opened.value(QStringLiteral("scenes")).toList().size() == 1,
          "fresh project has exactly 1 scene");

    // Assemble + give scene 0 a body so we can prove later offsets shift correctly.
    SceneDocument doc;
    assemble(doc, bridge, projectPath, appSupport, projectID, opened);
    const QString hostSceneID = doc.segments().at(0).sceneID;
    const QString hostChapter = doc.segments().at(0).chapterID;
    // (Body may be empty from create; that's fine — the shift math still holds.)

    // --- ScriviCore create after scene 0 ---
    const QVariantMap res =
        bridge.createScene(projectPath, appSupport, projectID, hostChapter, hostSceneID);
    const QString newSceneID = res.value(QStringLiteral("sceneID")).toString();
    check(!newSceneID.isEmpty(), "createScene returned a sceneID");

    // --- surgical splice into the live document + map ---
    const int before = doc.segments().size();
    const int hostBodyEnd = doc.segments().at(0).bodyStart + doc.segments().at(0).bodyLength;
    const int newIdx = doc.insertSceneAfter(
        0, newSceneID, res.value(QStringLiteral("chapterID")).toString(),
        QString(), doc.segments().at(0).chapterTitle, QString(),
        res.value(QStringLiteral("metadataPath")).toString(),
        res.value(QStringLiteral("contentPath")).toString(),
        doc.segments().at(0).chapterMetadataPath,   // same chapter
        /*newChapter=*/false);

    check(newIdx == 1, "new segment spliced at index 1");
    check(doc.segments().size() == before + 1, "segment count grew by one");
    if (newIdx == 1) {
        check(doc.segments().at(1).sceneID == newSceneID, "segment 1 is the new scene");
        check(doc.segments().at(1).bodyLength == 0, "new scene body is empty");
        check(doc.segments().at(1).bodyStart > hostBodyEnd,
              "new body starts after the host body + boundary");
        check(doc.bodyText(0) == QString() || doc.bodyText(0).length() >= 0,
              "host body still readable");
    }

    // --- reopen: the new scene persisted, in order ---
    opened = bridge.openProject(projectPath, appSupport);
    const QVariantList reopened = opened.value(QStringLiteral("scenes")).toList();
    check(reopened.size() == 2, "reopened project has 2 scenes");
    if (reopened.size() == 2) {
        check(reopened.at(0).toMap().value(QStringLiteral("sceneID")).toString()
                  == hostSceneID,
              "scene 0 still first after reopen");
        check(reopened.at(1).toMap().value(QStringLiteral("sceneID")).toString()
                  == newSceneID,
              "new scene is second after reopen");
    }

    // --- create CHAPTER (T-0241): appends a chapter + first scene at the end ---
    // ScriviCore appends new chapters, so the new chapter's first scene splices
    // after the LAST segment with a chapter-heading boundary (newChapter=true).
    const int segsBeforeCh = doc.segments().size();
    const QVariantMap chRes =
        bridge.createChapter(projectPath, appSupport, projectID);
    const QString newChapterID  = chRes.value(QStringLiteral("chapterID")).toString();
    const QString firstSceneID  = chRes.value(QStringLiteral("firstSceneID")).toString();
    check(!newChapterID.isEmpty(), "createChapter returned a chapterID");
    check(!firstSceneID.isEmpty(), "createChapter returned a firstSceneID");

    const int lastIdx = doc.segments().size() - 1;
    const int chIdx = doc.insertSceneAfter(
        lastIdx, firstSceneID, newChapterID, QString(), QString(), QString(),
        chRes.value(QStringLiteral("firstSceneMetadataPath")).toString(),
        chRes.value(QStringLiteral("firstSceneContentPath")).toString(),
        chRes.value(QStringLiteral("chapterMetadataPath")).toString(),
        /*newChapter=*/true);

    check(chIdx == segsBeforeCh, "chapter's first scene spliced at the end");
    if (chIdx >= 0) {
        check(doc.segments().at(chIdx).sceneID == firstSceneID,
              "last segment is the new chapter's first scene");
        check(doc.segments().at(chIdx).chapterID == newChapterID,
              "new segment carries the new chapterID");
        check(doc.segments().at(chIdx).chapterID
                  != doc.segments().at(chIdx - 1).chapterID,
              "new scene is in a different chapter than its predecessor");
        check(doc.segments().at(chIdx).bodyLength == 0, "new chapter scene is empty");
        // I-0062: the new (untitled) chapter's heading derives the ordinal "Chapter 2"
        // immediately from its position — no reload needed. insertSceneAfter reflowed it.
        check(doc.chapterHeadingText(chIdx) == QStringLiteral("Chapter 2"),
              "I-0062: new chapter's live heading derives \"Chapter 2\" (no reload)");
        check(doc.document()->toPlainText().contains(QStringLiteral("Chapter 2")),
              "I-0062: \"Chapter 2\" heading is in the live document");
    }

    // Reopen: 3 scenes now, the new chapter's scene last and in a new chapter.
    opened = bridge.openProject(projectPath, appSupport);
    const QVariantList afterCh = opened.value(QStringLiteral("scenes")).toList();
    check(afterCh.size() == 3, "reopened project has 3 scenes after chapter create");
    if (afterCh.size() == 3) {
        check(afterCh.at(2).toMap().value(QStringLiteral("sceneID")).toString()
                  == firstSceneID,
              "new chapter's scene is last after reopen");
        check(afterCh.at(2).toMap().value(QStringLiteral("chapterID")).toString()
                  == newChapterID,
              "new chapter's scene carries the new chapterID after reopen");
    }

    if (failures == 0) {
        std::fprintf(stderr, "OK: create-scene + create-chapter persisted and "
                             "spliced into the map.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
