// scene_rename_smoke — headless rename-scene / rename-chapter check for EP-023
// (SP-066, T-0254/T-0255/T-0257, AC3). No GUI.
//
// Exercises both halves of the navigator Rename path:
//   1. ScriviCore side: bridge.renameScene / bridge.renameChapter write the sidecar
//      `title`, so a reopen shows the new title (and a blank/whitespace title clears it
//      to empty — ScriviCore's normaliseTitle).
//   2. Editor side: SceneDocument::setSceneTitle updates the navigator-label authority
//      (no document text change for a scene); setChapterTitle updates every member's
//      chapterTitle AND rewrites the live in-document heading in place via
//      reflowBoundaryAt (offset map stays well-formed, later offsets shift).
//
//   argv[1] = project dir to create (e.g. <tmp>/scene-rename.scrivi)
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

// The offset map must match the document text for every segment (splice invariant).
void checkMapWellFormed(const SceneDocument& doc, const char* stage)
{
    int prevEnd = -1;
    for (int i = 0; i < doc.segments().size(); ++i) {
        const SceneSegment& s = doc.segments().at(i);
        char msg[160];
        std::snprintf(msg, sizeof(msg),
                      "[%s] segment %d bodyStart past previous body end", stage, i);
        check(s.bodyStart > prevEnd, msg);
        std::snprintf(msg, sizeof(msg),
                      "[%s] segment %d bodyLength matches extracted text", stage, i);
        check(doc.bodyText(i).length() == s.bodyLength, msg);
        prevEnd = s.bodyStart + s.bodyLength;
    }
}

// Fetch a scene's derived title from a fresh open_project (the reload authority).
QString reloadedSceneTitle(ScriviBridge& bridge, const QString& projectPath,
                           const QString& appSupport, const QString& sceneID)
{
    const QVariantMap opened = bridge.openProject(projectPath, appSupport);
    for (const QVariant& v : opened.value(QStringLiteral("scenes")).toList()) {
        const QVariantMap s = v.toMap();
        if (s.value(QStringLiteral("sceneID")).toString() == sceneID) {
            return s.value(QStringLiteral("title")).toString();
        }
    }
    return {};
}

QString reloadedChapterTitle(ScriviBridge& bridge, const QString& projectPath,
                             const QString& appSupport, const QString& chapterID)
{
    const QVariantMap opened = bridge.openProject(projectPath, appSupport);
    for (const QVariant& v : opened.value(QStringLiteral("scenes")).toList()) {
        const QVariantMap s = v.toMap();
        if (s.value(QStringLiteral("chapterID")).toString() == chapterID) {
            return s.value(QStringLiteral("chapterTitle")).toString();
        }
    }
    return {};
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 2) {
        std::fprintf(stderr, "usage: scene_rename_smoke <projectDir>\n");
        return 2;
    }
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scene Rename Tester"), appSupport);
    if (!bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }

    const QVariantMap created =
        bridge.createProject(projectPath, appSupport,
                             QStringLiteral("Scene Rename Project"),
                             QStringLiteral("scene-rename"));
    const QString projectID = created.value(QStringLiteral("projectID")).toString();
    if (projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // Fixture: one chapter with the default scene s0, plus a second chapter (s1).
    QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QVariantMap firstScene =
        opened.value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString s0  = firstScene.value(QStringLiteral("sceneID")).toString();
    const QString ch1 = firstScene.value(QStringLiteral("chapterID")).toString();
    const QVariantMap chRes = bridge.createChapter(projectPath, appSupport, projectID);
    const QString ch2 = chRes.value(QStringLiteral("chapterID")).toString();
    const QString s1  = chRes.value(QStringLiteral("firstSceneID")).toString();
    check(!s1.isEmpty() && !ch2.isEmpty(), "fixture: created chapter 2 (s1)");

    // ==========================================================================
    // Case A — rename a SCENE (s0). Sidecar title round-trips; the map label updates
    //          with no document text change.
    // ==========================================================================
    {
        opened = bridge.openProject(projectPath, appSupport);
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        const int docLenBefore = doc.document()->characterCount();

        const QString metaPath = doc.segments().at(doc.sceneIndexForScene(s0)).metadataPath;
        const QVariantMap r =
            bridge.renameScene(projectPath, metaPath, QStringLiteral("Opening Scene"));
        check(!r.isEmpty(), "A: renameScene returned an ok result");
        check(r.value(QStringLiteral("newTitle")).toString()
                  == QStringLiteral("Opening Scene"),
              "A: result echoes the new title");

        // Reload authority: the sidecar persisted the title.
        check(reloadedSceneTitle(bridge, projectPath, appSupport, s0)
                  == QStringLiteral("Opening Scene"),
              "A: renamed scene title persisted on reopen");

        // Map update: setSceneTitle changes the label, not the document text.
        doc.setSceneTitle(doc.sceneIndexForScene(s0), QStringLiteral("Opening Scene"));
        check(doc.segments().at(doc.sceneIndexForScene(s0)).title
                  == QStringLiteral("Opening Scene"),
              "A: SceneDocument label updated");
        check(doc.document()->characterCount() == docLenBefore,
              "A: scene rename left the document text unchanged");
        checkMapWellFormed(doc, "A/post");
    }

    // ==========================================================================
    // Case B — rename a CHAPTER (ch1). Sidecar title persists; setChapterTitle rewrites
    //          the live heading in place and the offset map stays well-formed.
    // ==========================================================================
    {
        opened = bridge.openProject(projectPath, appSupport);
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);

        const int first = doc.firstSegmentOfChapter(ch1);
        check(first >= 0, "B: located chapter 1's first segment");
        const QString chMetaPath = doc.segments().at(first).chapterMetadataPath;
        check(!chMetaPath.isEmpty(), "B: chapter carries its metadata path");
        const int s0StartBefore = doc.bodyStartForScene(s0);
        // The heading currently shown for chapter 1 (its title, or "Chapter" if empty).
        const QString oldHeading = doc.segments().at(first).chapterTitle.isEmpty()
                                       ? QStringLiteral("Chapter")
                                       : doc.segments().at(first).chapterTitle;

        const QVariantMap r =
            bridge.renameChapter(projectPath, chMetaPath, QStringLiteral("Beginnings"));
        check(!r.isEmpty(), "B: renameChapter returned an ok result");
        check(reloadedChapterTitle(bridge, projectPath, appSupport, ch1)
                  == QStringLiteral("Beginnings"),
              "B: renamed chapter title persisted on reopen");

        // Live heading rewrite: the heading changes oldHeading → "Beginnings"; chapter 1
        // is the first chapter, so its scene bodies shift by the heading-length delta.
        const int delta = QStringLiteral("Beginnings").length() - oldHeading.length();
        doc.setChapterTitle(ch1, QStringLiteral("Beginnings"));
        check(doc.segments().at(first).chapterTitle == QStringLiteral("Beginnings"),
              "B: chapter title updated on the segment");
        check(doc.bodyStartForScene(s0) == s0StartBefore + delta,
              "B: body shifted by the heading-length delta");
        check(doc.document()->toPlainText().startsWith(QStringLiteral("Beginnings")),
              "B: live document heading reads the new chapter title");
        checkMapWellFormed(doc, "B/post");
    }

    // ==========================================================================
    // Case C — BLANK chapter title clears the sidecar (ScriviCore normaliseTitle), and
    //          the heading falls back to the app-DERIVED ordinal "Chapter N" — NOT a
    //          generic "Chapter". The Linux app owns chapter numbering (from segment
    //          order, matching macOS ManuscriptTextView), so ch1 (the first chapter)
    //          shows "Chapter 1". This is what lets a delete/insert renumber untitled
    //          chapters with no disk rewrite.
    // ==========================================================================
    {
        opened = bridge.openProject(projectPath, appSupport);
        SceneDocument doc;
        assemble(doc, bridge, projectPath, appSupport, projectID, opened);
        const int first = doc.firstSegmentOfChapter(ch1);
        const QString chMetaPath = doc.segments().at(first).chapterMetadataPath;

        const QVariantMap r =
            bridge.renameChapter(projectPath, chMetaPath, QStringLiteral("   "));
        check(!r.isEmpty(), "C: blank renameChapter returned an ok result");
        check(reloadedChapterTitle(bridge, projectPath, appSupport, ch1).isEmpty(),
              "C: blank title cleared the sidecar (empty on reopen)");

        doc.setChapterTitle(ch1, QString());   // empty custom → derived ordinal
        check(doc.segments().at(first).chapterTitle.isEmpty(),
              "C: custom chapter title cleared on the segment");
        check(doc.chapterHeadingText(first) == QStringLiteral("Chapter 1"),
              "C: derived heading is the ordinal \"Chapter 1\"");
        check(doc.document()->toPlainText().startsWith(QStringLiteral("Chapter 1")),
              "C: live heading shows the derived ordinal, not a generic \"Chapter\"");
        checkMapWellFormed(doc, "C/post");
    }

    // NOTE: renumber-on-delete (a stored "Chapter N" chapter renumbering when an earlier
    // chapter is deleted) is NOT covered here — ScriviCore stamps "Chapter N" as a stored
    // title at creation and does not renumber on delete, so it needs its own mechanism.
    // Tracked as a follow-up Issue (I-0063). The derived-ordinal path above (empty title
    // → "Chapter N" from order) is what SP-066 delivers.

    if (failures == 0) {
        std::fprintf(stderr, "OK: rename-scene + rename-chapter persisted; live label + "
                             "heading updated in place.\n");
        std::printf("%s\n", projectID.toUtf8().constData());
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
