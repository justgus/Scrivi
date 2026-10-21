// range_smoke — EP-053 S2a (SP-187 / T-0615): the range endpoints, reached THROUGH ScriviBridge, and their delta decoded
// into `RangeDelta`. No GUI.
//
// ⚠️ Through the bridge, never the C ABI directly (`feedback_boundary_tests_not_facade`). The core's own behaviour is
// ScriviCore's tests (`RangeOperationCApiTests.cpp`); this proves the BINDING: every argument reaches the core, every
// answer comes back, and the delta's fields land in the C++ type. Every write is checked by REOPENING the project.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <cstdio>

#include "AppSupport.hpp"
#include "RangeDelta.hpp"
#include "ScriviBridge.hpp"

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

struct Row {
    QString sceneID, chapterID, text;
    bool operator==(const Row&) const = default;
};

struct Fixture {
    ScriviBridge bridge;
    QString root, appSupport, projectID;

    // The manuscript as the DISK has it: a fresh open, then the scene texts.
    QList<Row> rows()
    {
        const QVariantMap opened = bridge.openProject(root, appSupport);
        QStringList ids;
        QList<Row> out;
        for (const QVariant& v : opened.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap s = v.toMap();
            out.append({s.value(QStringLiteral("sceneID")).toString(), s.value(QStringLiteral("chapterID")).toString(), {}});
            ids.append(out.last().sceneID);
        }
        const QHash<QString, QString> texts = bridge.readSceneTexts(root, ids);
        for (Row& r : out) { r.text = texts.value(r.sceneID); }
        return out;
    }

    void save(const QVariantMap& scene, const QString& text)
    {
        const QVariantMap r = bridge.saveScene(projectID, root, appSupport, scene.value(QStringLiteral("sceneID")).toString(),
                                               scene.value(QStringLiteral("metadataPath")).toString(),
                                               scene.value(QStringLiteral("contentPath")).toString(), text, 0, 0, 0.0);
        check(!bridge.lastCallFailed(), QStringLiteral("fixture: save %1").arg(text));
    }
};

QString json(const QVariantMap& m) { return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(m)).toJson(QJsonDocument::Compact)); }

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: range_smoke <projectDir>\n");
        return 2;
    }
    Fixture f;
    f.root = QString::fromUtf8(argv[1]);
    f.appSupport = scrivi::linux_app::appSupportRoot();
    f.bridge.bootstrap(QStringLiteral("Range Tester"), f.appSupport);
    if (!f.bridge.ready()) {
        std::fprintf(stderr, "FAIL: identity did not bootstrap\n");
        return 1;
    }
    f.projectID = f.bridge.createProject(f.root, f.appSupport, QStringLiteral("Range Project"), QStringLiteral("range-project"))
                      .value(QStringLiteral("projectID")).toString();
    if (f.projectID.isEmpty()) {
        std::fprintf(stderr, "FAIL: create returned no projectID\n");
        return 1;
    }

    // Two chapters of three scenes: "C<c>S<s> body." — built through the bridge, as the app builds them.
    const QVariantMap first = f.bridge.openProject(f.root, f.appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
    QVariantMap prev = first;
    f.save(first, QStringLiteral("C1S1 body."));
    for (int s = 2; s <= 3; ++s) {
        QVariantMap made = f.bridge.createScene(f.root, f.appSupport, f.projectID, first.value(QStringLiteral("chapterID")).toString(),
                                                prev.value(QStringLiteral("sceneID")).toString());
        f.save(made, QStringLiteral("C1S%1 body.").arg(s));
        prev = made;
    }
    const QVariantMap ch2 = f.bridge.createChapter(f.root, f.appSupport, f.projectID);
    QVariantMap ch2First{{QStringLiteral("sceneID"), ch2.value(QStringLiteral("firstSceneID"))},
                         {QStringLiteral("metadataPath"), ch2.value(QStringLiteral("firstSceneMetadataPath"))},
                         {QStringLiteral("contentPath"), ch2.value(QStringLiteral("firstSceneContentPath"))}};
    f.save(ch2First, QStringLiteral("C2S1 body."));
    prev = ch2First;
    for (int s = 2; s <= 3; ++s) {
        QVariantMap made = f.bridge.createScene(f.root, f.appSupport, f.projectID, ch2.value(QStringLiteral("chapterID")).toString(),
                                                prev.value(QStringLiteral("sceneID")).toString());
        f.save(made, QStringLiteral("C2S%1 body.").arg(s));
        prev = made;
    }
    const QList<Row> start = f.rows();
    check(start.size() == 6, QStringLiteral("fixture: six scenes (got %1)").arg(start.size()));
    if (start.size() != 6) { return 1; }

    // --- rangeRead: across the chapter boundary -------------------------------------------------------------------------
    const QVariantMap read = f.bridge.rangeRead(f.root, start[2].sceneID, 4, start[3].sceneID, 4);
    check(!f.bridge.lastCallFailed(), QStringLiteral("rangeRead succeeds"));
    const QVariantMap range = read.value(QStringLiteral("range")).toMap();
    const QVariantList entries = range.value(QStringLiteral("scenes")).toList();
    check(range.value(QStringLiteral("schema")).toString() == QStringLiteral("scrivi.range.v1"), QStringLiteral("rangeRead: schema"));
    check(entries.size() == 2, QStringLiteral("rangeRead: two entries"));
    if (entries.size() == 2) {
        check(entries[0].toMap().value(QStringLiteral("text")).toString() == QStringLiteral(" body."), QStringLiteral("rangeRead: first text"));
        check(entries[1].toMap().value(QStringLiteral("text")).toString() == QStringLiteral("C2S1"), QStringLiteral("rangeRead: last text"));
    }
    f.bridge.rangeRead(f.root, start[3].sceneID, 0, start[2].sceneID, 0);
    check(f.bridge.lastCallFailed(), QStringLiteral("rangeRead: an end before the start is refused"));

    // --- rangePaste: the range that opens a chapter, pasted mid-scene; the delta decoded ---------------------------------
    const QVariantMap pasted = f.bridge.rangePaste(f.root, json(range), start[1].sceneID, 2, start[1].text, QStringLiteral("Range Tester"));
    check(!f.bridge.lastCallFailed(), QStringLiteral("rangePaste succeeds"));
    const RangeDelta pd = RangeDelta::fromVariant(pasted.value(QStringLiteral("delta")).toMap());
    check(pd.scenes.size() == 3, QStringLiteral("paste delta: caret rewritten, one created, one moved (got %1)").arg(pd.scenes.size()));
    if (pd.scenes.size() == 3) {
        check(pd.scenes[0].change == RangeDelta::Change::Rewritten && pd.scenes[0].sceneID == start[1].sceneID,
              QStringLiteral("paste delta: the caret scene is rewritten"));
        check(pd.scenes[0].hasText && pd.scenes[0].text == QStringLiteral("C1 body."), QStringLiteral("paste delta: the caret text"));
        check(pd.scenes[1].change == RangeDelta::Change::Created && pd.scenes[1].afterSceneID == start[1].sceneID,
              QStringLiteral("paste delta: the created scene follows the caret"));
        check(pd.scenes[1].text == QStringLiteral("C2S1S2 body."), QStringLiteral("paste delta: created text = last piece + tail"));
        check(pd.scenes[2].change == RangeDelta::Change::Moved && pd.scenes[2].sceneID == start[2].sceneID && !pd.scenes[2].hasText,
              QStringLiteral("paste delta: chapter 1's later scene moved [I-0311]"));
        check(pd.scenes[2].chapterID == pd.scenes[1].chapterID, QStringLiteral("paste delta: moved into the new chapter"));
    }
    check(pd.createdChapters.size() == 1 && pd.createdChapters[0].afterChapterID == start[0].chapterID,
          QStringLiteral("paste delta: one chapter created, after chapter 1"));
    check(pd.removedSceneIDs.isEmpty() && pd.removedChapterIDs.isEmpty(), QStringLiteral("paste delta: absent lists decode empty"));
    const QList<Row> afterPaste = f.rows();
    check(afterPaste.size() == 7, QStringLiteral("reopen after paste: seven scenes"));
    if (afterPaste.size() == 7 && pd.scenes.size() == 3) {
        check(afterPaste[2].sceneID == pd.scenes[1].sceneID && afterPaste[2].text == pd.scenes[1].text,
              QStringLiteral("reopen after paste: the created scene is on disk where the delta says"));
    }

    // --- rangeCut: the Range the paste inserted restores the manuscript --------------------------------------------------
    const QVariantMap cut = f.bridge.rangeCut(f.root, json(pasted.value(QStringLiteral("range")).toMap()), QStringLiteral("Range Tester"));
    check(!f.bridge.lastCallFailed(), QStringLiteral("rangeCut succeeds"));
    const RangeDelta cd = RangeDelta::fromVariant(cut.value(QStringLiteral("delta")).toMap());
    check(cd.removedSceneIDs.size() == 1 && cd.removedChapterIDs.size() == 1, QStringLiteral("cut delta: one scene, one chapter removed"));
    check(f.rows() == start, QStringLiteral("reopen after cut: the manuscript is as it started"));

    // --- a stale caret text is refused, and writes nothing ---------------------------------------------------------------
    f.bridge.rangePaste(f.root, json(range), start[4].sceneID, 1, QStringLiteral("not on disk"), QStringLiteral("Range Tester"));
    check(f.bridge.lastCallFailed(), QStringLiteral("rangePaste: a stale caret text is refused"));
    check(f.rows() == start, QStringLiteral("reopen after the refusal: nothing written"));

    f.bridge.closeProject(f.root);
    if (failures == 0) {
        std::printf("range_smoke: all checks passed\n");
    }
    return failures == 0 ? 0 : 1;
}
