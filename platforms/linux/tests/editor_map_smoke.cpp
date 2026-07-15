// editor_map_smoke — headless check for the SP-062 (T-0238) editable-viewport
// offset map + boundary classification, with NO GUI.
//
// SceneDocument is the edit-routing authority for the editable continuous viewport:
//   • it classifies document positions as editable (inside a scene body) vs.
//     protected (chapter heading / scene separator);
//   • it keeps the per-scene body offsets in sync with edits (applyContentsChange)
//     without rebuilding the document.
// This harness builds a two-chapter / three-scene document with known bodies and
// asserts that classification + map maintenance behave, so the logic that keeps
// edits inside their owning scene is covered before it reaches the GUI.
//
// No ScriviCore or project I/O — pure SceneDocument logic. QApplication (not
// QCoreApplication) because SceneDocument installs a QPlainTextDocumentLayout, a
// Qt Widgets type; the offscreen platform (set by the driving script) means no
// window is shown.

#include <QApplication>
#include <QString>
#include <QTextCursor>

#include <cstdio>

#include "SceneDocument.hpp"

namespace {

int failures = 0;

void check(bool cond, const char* what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Canonical fixture: chapter "One" with scenes S1,S2; chapter "Two" with scene
    // S3. Distinct single-line bodies, easy to locate. Built in place — SceneDocument
    // is non-copyable (it owns a QTextDocument by value).
    SceneDocument doc;
    {
        QList<SceneDocument::Input> in;
        in.push_back({"s1", "c1", "Scene 1", "One", "s1", "", "", "AAAA"});
        in.push_back({"s2", "c1", "Scene 2", "One", "s2", "", "", "BBBB"});
        in.push_back({"s3", "c2", "Scene 3", "Two", "s3", "", "", "CCCC"});
        doc.build(in);
    }
    const auto& segs = doc.segments();

    // ---- structure ----
    check(segs.size() == 3, "fixture has 3 segments");
    check(doc.bodyText(0) == QStringLiteral("AAAA"), "seg0 body is AAAA");
    check(doc.bodyText(1) == QStringLiteral("BBBB"), "seg1 body is BBBB");
    check(doc.bodyText(2) == QStringLiteral("CCCC"), "seg2 body is CCCC");

    // ---- sceneID → index lookup (SP-063 T-0244 navigator-click promotion) ----
    check(doc.sceneIndexForScene(QStringLiteral("s1")) == 0, "sceneIndexForScene(s1)==0");
    check(doc.sceneIndexForScene(QStringLiteral("s2")) == 1, "sceneIndexForScene(s2)==1");
    check(doc.sceneIndexForScene(QStringLiteral("s3")) == 2, "sceneIndexForScene(s3)==2");
    check(doc.sceneIndexForScene(QStringLiteral("nope")) == -1,
          "sceneIndexForScene(unknown)==-1");

    // ---- caret position → owning scene (SP-063 T-0243 scroll/active mapping) ----
    // sceneIndexForCaret maps any document position to the scene it belongs to; the
    // scroll-driven active-scene promotion resolves the visible midpoint through it.
    check(doc.sceneIndexForCaret(segs.at(0).bodyStart) == 0, "caret in body0 → scene0");
    check(doc.sceneIndexForCaret(segs.at(1).bodyStart) == 1, "caret in body1 → scene1");
    check(doc.sceneIndexForCaret(segs.at(2).bodyStart) == 2, "caret in body2 → scene2");
    // A position in the seg1→seg2 gap belongs to the preceding scene (seg1).
    check(doc.sceneIndexForCaret(segs.at(1).bodyStart + segs.at(1).bodyLength + 1) == 1,
          "caret in seg1→seg2 gap → scene1 (trailing scene owns the gap)");

    // ---- boundary classification ----
    // Inside each body (a middle position) is editable.
    for (int i = 0; i < segs.size(); ++i) {
        const int mid = segs.at(i).bodyStart + 1;
        char msg[64];
        std::snprintf(msg, sizeof msg, "mid of body %d is editable", i);
        check(doc.isEditablePosition(mid), msg);
    }
    // The gap between seg0's end and seg1's start (the separator) is NOT editable.
    const int gap01 = segs.at(0).bodyStart + segs.at(0).bodyLength + 1;
    check(gap01 < segs.at(1).bodyStart, "there is a real gap between seg0 and seg1");
    check(!doc.isEditablePosition(gap01), "separator gap (seg0→seg1) is protected");
    // The very start of the document (chapter heading region) is not editable
    // unless seg0 begins at 0; the heading precedes it, so start-of-doc is protected.
    check(segs.at(0).bodyStart > 0, "chapter heading precedes first body");
    check(!doc.isEditablePosition(0), "start of document (heading) is protected");

    // A range that straddles the seg0→seg1 boundary must be rejected; a range
    // wholly inside seg0 accepted.
    check(doc.isEditableRange(segs.at(0).bodyStart,
                              segs.at(0).bodyStart + segs.at(0).bodyLength),
          "full seg0 body range is editable");
    check(!doc.isEditableRange(segs.at(0).bodyStart, segs.at(1).bodyStart),
          "range spanning the seg0→seg1 boundary is rejected");

    // ---- map maintenance on an insert inside seg0 ----
    // Insert "XX" at the start of seg0's body (the way a keystroke would), then feed
    // the same (pos, removed, added) to applyContentsChange and re-check everything.
    const int seg1StartBefore = segs.at(1).bodyStart;
    const int seg2StartBefore = segs.at(2).bodyStart;
    const int insertPos = segs.at(0).bodyStart;
    {
        QTextCursor c(doc.document());
        c.setPosition(insertPos);
        c.insertText(QStringLiteral("XX"));
    }
    const int touched = doc.applyContentsChange(insertPos, 0, 2);
    check(touched == 0, "insert at seg0 start reported seg0 dirty");
    check(doc.bodyText(0) == QStringLiteral("XXAAAA"), "seg0 body grew to XXAAAA");
    check(doc.segments().at(1).bodyStart == seg1StartBefore + 2,
          "seg1 start shifted by +2");
    check(doc.segments().at(2).bodyStart == seg2StartBefore + 2,
          "seg2 start shifted by +2");
    check(doc.bodyText(1) == QStringLiteral("BBBB"), "seg1 body unchanged (BBBB)");
    check(doc.bodyText(2) == QStringLiteral("CCCC"), "seg2 body unchanged (CCCC)");

    // ---- map maintenance on a deletion inside seg1 ----
    // Delete the last char of seg1's body.
    const int seg1 = 1;
    const int delPos = doc.segments().at(seg1).bodyStart
                       + doc.segments().at(seg1).bodyLength - 1;
    const int seg2StartBeforeDel = doc.segments().at(2).bodyStart;
    {
        QTextCursor c(doc.document());
        c.setPosition(delPos);
        c.setPosition(delPos + 1, QTextCursor::KeepAnchor);
        c.removeSelectedText();
    }
    const int touchedDel = doc.applyContentsChange(delPos, 1, 0);
    check(touchedDel == 1, "delete in seg1 reported seg1 dirty");
    check(doc.bodyText(1) == QStringLiteral("BBB"), "seg1 body shrank to BBB");
    check(doc.segments().at(2).bodyStart == seg2StartBeforeDel - 1,
          "seg2 start shifted by -1 after seg1 deletion");
    check(doc.bodyText(2) == QStringLiteral("CCCC"), "seg2 body still CCCC");

    if (failures == 0) {
        std::fprintf(stderr, "OK: offset map + boundary classification hold across "
                             "insert and delete.\n");
        std::printf("editor-map-ok\n");
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
