// structural_baseline_smoke — EP-053 S2a (SP-187 / T-0615): what each structural operation costs on Linux TODAY.
//
// A REAL `EditorShell` loads a real project; each operation runs through the shell's own slot (or the real key dispatch
// for copy and paste). For each it prints the baseline S2b ([SP-191]) is measured against: core calls on the GUI thread,
// project loads (a structural reload is one), and full document builds. The manuscript is read back FROM DISK after each,
// and every scene's text is checked against the document (SCRIVI_VERIFY_SCENE_TEXT, set by the script).
// ⚠️ It asserts what the operations DO, not what they cost: the costs are the baseline, recorded in SP-187.
// ✅ [SP-191]: and now that they are range calls — no project load and no document build per operation; the scene list
// equal to the disk after every operation, undo and redo (`listMatchesDisk`, ⛔ [I-0314] shipped because nothing compared
// them); each operation undone and redone in ONE step; Q5's cue and an ignored repeat; Q7 and Q8 at a scene's start.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <cstdio>
#include <functional>
#include <memory>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

// A structural operation may ask first (a chapter split renumbers the chapters after it): answer yes.
void armConfirmer(QObject* owner)
{
    auto* t = new QTimer(owner);
    t->setInterval(20);
    QObject::connect(t, &QTimer::timeout, [] {
        if (auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            if (QAbstractButton* yes = box->button(QMessageBox::Yes)) { yes->click(); }
            else if (QAbstractButton* ok = box->button(QMessageBox::Ok)) { ok->click(); }
            else { box->accept(); }
        }
    });
    t->start();
}

struct Rig {
    ScriviBridge bridge;
    QString projectDir, appSupport, projectID;
    std::unique_ptr<EditorShell> shell;
    ManuscriptEditor* editor = nullptr;

    bool load()
    {
        shell = std::make_unique<EditorShell>();
        QEventLoop loop;
        bool ok = false;
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) { ok = r; loop.quit(); });
        shell->load(projectDir, appSupport, QStringLiteral("Baseline"));
        QTimer::singleShot(60000, &loop, &QEventLoop::quit);
        loop.exec();
        editor = shell->findChild<ManuscriptEditor*>();
        if (!ok || editor == nullptr) { return false; }
        shell->show();
        shell->activateWindow();
        QTest::qWaitForWindowActive(shell.get());
        editor->setFocus();
        QApplication::processEvents();
        return true;
    }
    QStringList texts()
    {
        shell->saveDirtyScenes();
        QStringList ids;
        for (const QVariant& v : bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList()) {
            ids.append(v.toMap().value(QStringLiteral("sceneID")).toString());
        }
        const QHash<QString, QString> t = bridge.readSceneTexts(projectDir, ids);
        QStringList out;
        for (const QString& id : ids) { out.append(t.value(id)); }
        return out;
    }
    // ✅ [SP-191] The scene list IS the disk: identity, chapter, paths and text, against a fresh open.
    void listMatchesDisk(const QString& what)
    {
        shell->saveDirtyScenes();
        QStringList disk;
        QStringList ids;
        const QVariantList scenes = bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList();
        for (const QVariant& v : scenes) { ids.append(v.toMap().value(QStringLiteral("sceneID")).toString()); }
        const QHash<QString, QString> t = bridge.readSceneTexts(projectDir, ids);
        for (const QVariant& v : scenes) {
            const QVariantMap m = v.toMap();
            disk.append(QStringList{m.value(QStringLiteral("sceneID")).toString(), m.value(QStringLiteral("chapterID")).toString(),
                                    m.value(QStringLiteral("metadataPath")).toString(),
                                    m.value(QStringLiteral("contentPath")).toString(),
                                    t.value(m.value(QStringLiteral("sceneID")).toString())}.join(QLatin1Char('|')));
        }
        QStringList list;
        for (const SceneSegment& seg : shell->sceneDocument().segments()) {
            list.append(QStringList{seg.sceneID, seg.chapterID, seg.metadataPath, seg.contentPath, seg.text}.join(QLatin1Char('|')));
        }
        check(list == disk, QStringLiteral("%1: the scene list is what is on disk\n  list: %2\n  disk: %3")
                                .arg(what, list.join(QStringLiteral(" ; ")), disk.join(QStringLiteral(" ; "))));
    }
    // The scenes as the editor holds them: identity, chapter, text.
    QStringList state() const
    {
        QStringList out;
        for (const SceneSegment& seg : shell->sceneDocument().segments()) {
            out.append(seg.sceneID + QLatin1Char('|') + seg.chapterID + QLatin1Char('|') + seg.text);
        }
        return out;
    }
    // [SP-191] `act` starts a structural edit: wait for it to land.
    bool landing(const std::function<void()>& act)
    {
        QEventLoop loop;
        bool ok = false;
        bool done = false;
        QObject::connect(shell.get(), &EditorShell::structuralEditFinished, &loop, [&](bool r) { ok = r; done = true; loop.quit(); });
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        act();
        if (!done) { loop.exec(); }
        QTest::qWait(60);
        return ok;
    }
    void caretAtSceneStart(int index)
    {
        QTextCursor c = editor->textCursor();
        c.setPosition(shell->sceneDocument().segments().at(index).bodyStart);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    int chapterCount()
    {
        QStringList seen;
        for (const QVariant& v : bridge.openProject(projectDir, appSupport).value(QStringLiteral("scenes")).toList()) {
            const QString c = v.toMap().value(QStringLiteral("chapterID")).toString();
            if (seen.isEmpty() || seen.last() != c) { seen.append(c); }
        }
        return seen.size();
    }
    void caretAt(const QString& marker, bool before = false)
    {
        const QTextCursor found = editor->document()->find(marker);
        QTextCursor c = editor->textCursor();
        c.setPosition(before ? found.selectionStart() : found.selectionEnd());
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void select(const QString& from, const QString& to)
    {
        const QTextCursor a = editor->document()->find(from);
        const QTextCursor b = editor->document()->find(to);
        QTextCursor c = editor->textCursor();
        c.setPosition(a.selectionStart());
        c.setPosition(b.selectionEnd(), QTextCursor::KeepAnchor);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void control(Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier | extra);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier | extra);
    }
    // Runs `act`, lets any background work and reload land, then prints the baseline. Returns the GUI-thread core calls.
    qint64 measured(const char* what, const std::function<void()>& act, bool structural = true)
    {
        const qint64 calls = ScriviBridge::guiThreadCallCount();
        const int loads = shell->loadCount();
        const int builds = shell->sceneDocument().builds();
        act();
        if (structural) {
            check(editor->isReadOnly(), QStringLiteral("%1: Q5's cue — read-only at once, in the key press's own turn").arg(what));
        }
        for (int i = 0; i < 300; ++i) {   // ≤ 15 s for a worker to land
            QTest::qWait(50);
            if (!editor->isReadOnly()) { break; }
        }
        QTest::qWait(200);
        std::printf("[EP-053 baseline] %s: guiThreadCoreCalls=%lld projectLoads=%d documentBuilds=%d\n", what,
                    qlonglong(ScriviBridge::guiThreadCallCount() - calls), shell->loadCount() - loads,
                    shell->sceneDocument().builds() - builds);
        check(shell->sceneDocument().textMatchesDocument(), QStringLiteral("%1: every scene's text is its body").arg(what));
        // ✅ [SP-191]: nothing re-reads the project, and nothing rebuilds the document.
        check(shell->loadCount() == loads, QStringLiteral("%1: no project load").arg(what));
        check(shell->sceneDocument().builds() == builds, QStringLiteral("%1: no document build").arg(what));
        listMatchesDisk(QString::fromUtf8(what));
        return ScriviBridge::guiThreadCallCount() - calls;
    }
};

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: structural_baseline_smoke <projectDir>\n");
        return 2;
    }
    Rig r;
    r.projectDir = QString::fromUtf8(argv[1]);
    r.appSupport = scrivi::linux_app::appSupportRoot();
    r.bridge.bootstrap(QStringLiteral("Baseline Tester"), r.appSupport);
    r.projectID = r.bridge.createProject(r.projectDir, r.appSupport, QStringLiteral("Baseline"), QStringLiteral("baseline"))
                      .value(QStringLiteral("projectID")).toString();
    if (r.projectID.isEmpty()) {
        std::printf("FAIL: no project\n");
        return 1;
    }
    // One chapter: "Alpha one. Alpha two." · "Beta body." · "Gamma body." · "## Delta\nDelta body."
    const QVariantMap first = r.bridge.openProject(r.projectDir, r.appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
    auto save = [&](const QVariantMap& s, const QString& text) {
        r.bridge.saveScene(r.projectID, r.projectDir, r.appSupport, s.value(QStringLiteral("sceneID")).toString(),
                           s.value(QStringLiteral("metadataPath")).toString(), s.value(QStringLiteral("contentPath")).toString(),
                           text, 0, 0, 0.0);
    };
    save(first, QStringLiteral("Alpha one. Alpha two."));
    QVariantMap prev = first;
    for (const QString& body : {QStringLiteral("Beta body."), QStringLiteral("Gamma body."), QStringLiteral("## Delta\nDelta body.")}) {
        const QVariantMap made = r.bridge.createScene(r.projectDir, r.appSupport, r.projectID,
                                                      first.value(QStringLiteral("chapterID")).toString(),
                                                      prev.value(QStringLiteral("sceneID")).toString());
        save(made, body);
        prev = made;
    }
    if (!r.load()) {
        std::printf("FAIL: the project did not load\n");
        return 1;
    }
    armConfirmer(r.shell.get());

    // ✅ [I-0315]: the caret moving into the new scene writes NOTHING for the scene it left (the notes were not changed) —
    // what is left is the new scene's notes read and one more. Before, the Inspector rewrote the outline and the to-do too.
    check(r.measured("split scene", [&] { r.caretAt(QStringLiteral("Alpha one. ")); r.shell->splitScene(); }) <= 2,
          QStringLiteral("split scene: at most 2 GUI-thread core calls ([I-0315]: no notes written on the scene switch)"));
    check(r.texts() == QStringList({QStringLiteral("Alpha one. "), QStringLiteral("Alpha two."), QStringLiteral("Beta body."),
                                    QStringLiteral("Gamma body."), QStringLiteral("## Delta\nDelta body.")}),
          QStringLiteral("split: the scenes on disk"));

    r.measured("merge scene", [&] { r.caretAt(QStringLiteral("Alpha two."), true); r.shell->mergeScene(); });
    check(r.texts().size() == 4, QStringLiteral("merge: four scenes on disk"));

    r.measured("split chapter", [&] { r.caretAt(QStringLiteral("Beta "), false); r.shell->splitChapter(); });
    check(r.chapterCount() == 2, QStringLiteral("split chapter: two chapters on disk"));

    r.measured("merge chapter", [&] {
        r.caretAt(QStringLiteral("body."), true);   // the start of chapter 2's first scene ("body." after the split)
        QTextCursor c = r.editor->textCursor();
        const int idx = r.shell->sceneDocument().sceneIndexForCaret(c.position());
        c.setPosition(r.shell->sceneDocument().segments().at(idx).bodyStart);
        r.editor->setTextCursor(c);
        r.shell->mergeChapter();
    });
    check(r.chapterCount() == 1, QStringLiteral("merge chapter: one chapter on disk"));

    r.measured("copy across scenes", [&] { r.select(QStringLiteral("one."), QStringLiteral("Beta")); r.control(Qt::Key_C); },
               /*structural=*/false);
    const QStringList beforePaste = r.texts();
    r.measured("structured paste", [&] { r.caretAt(QStringLiteral("Gamma ")); r.control(Qt::Key_V); });
    check(r.texts().size() == beforePaste.size() + 1, QStringLiteral("structured paste: one scene more on disk"));
    r.measured("undo of the paste", [&] { r.control(Qt::Key_Z); });
    check(r.texts() == beforePaste, QStringLiteral("undo of the paste: the manuscript as it was before the paste"));

    // ── [SP-191] Q3: each structural edit undoes and redoes in ONE step — the list the disk after each ─────────────────
    // (Starts from: chapter 1 [Alpha one. Alpha two. · Beta · Gamma body. · ## Delta] — whatever the above left — one chapter.)
    const auto roundTrip = [&](const QString& what, const std::function<void()>& op) {
        const QStringList before = r.state();
        check(r.landing(op), QStringLiteral("%1 landed").arg(what));
        const QStringList after = r.state();
        check(after != before, QStringLiteral("%1 changed the manuscript").arg(what));
        r.listMatchesDisk(what);
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("%1: its undo landed").arg(what));
        check(r.state() == before, QStringLiteral("%1: undone in ONE step").arg(what));
        r.listMatchesDisk(what + QStringLiteral(", undone"));
        check(r.landing([&] { r.control(Qt::Key_Z, Qt::ShiftModifier); }), QStringLiteral("%1: its redo landed").arg(what));
        check(r.state() == after, QStringLiteral("%1: redone exactly").arg(what));
        r.listMatchesDisk(what + QStringLiteral(", redone"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("%1: undone again").arg(what));
    };
    const QStringList start = r.state();
    roundTrip(QStringLiteral("split"), [&] { r.caretAt(QStringLiteral("Alpha one. ")); r.shell->splitScene(); });
    roundTrip(QStringLiteral("split at a scene's start"), [&] { r.caretAtSceneStart(1); r.shell->splitScene(); });
    roundTrip(QStringLiteral("merge"), [&] { r.caretAtSceneStart(1); r.shell->mergeScene(); });
    roundTrip(QStringLiteral("chapter split"), [&] { r.caretAt(QStringLiteral("Alpha ")); r.shell->splitChapter(); });
    roundTrip(QStringLiteral("chapter split at a scene's start"), [&] { r.caretAtSceneStart(1); r.shell->splitChapter(); });
    check(r.landing([&] { r.caretAtSceneStart(1); r.shell->splitChapter(); }), QStringLiteral("(a second chapter, kept)"));
    roundTrip(QStringLiteral("chapter merge"), [&] { r.caretAtSceneStart(1); r.shell->mergeChapter(); });
    check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the kept chapter undone)"));
    check(r.state() == start, QStringLiteral("every scene kept its identity and text through all of it"));
    r.listMatchesDisk(QStringLiteral("the end of the round trips"));

    // ── [SP-191] Q7: ⌃↩ at a scene's start puts the new scene BEFORE it, the caret into it ─────────────────────────────
    {
        const QString second = r.shell->sceneDocument().segments().at(1).sceneID;
        check(r.landing([&] { r.caretAtSceneStart(1); r.shell->splitScene(); }), QStringLiteral("Q7: the split landed"));
        const QList<SceneSegment>& segs = r.shell->sceneDocument().segments();
        check(segs.size() == start.size() + 1 && segs.at(1).text.isEmpty() && segs.at(2).sceneID == second,
              QStringLiteral("Q7: a new, empty scene before it; the scene keeps its identity"));
        check(r.editor->textCursor().position() == segs.at(1).bodyStart, QStringLiteral("Q7: the caret is in the new scene"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
    }
    // ── [SP-191] Q8: ⌃⇧↩ at a scene's start is a relabel — no scene made, no text moved ───────────────────────────────
    {
        const QStringList texts = r.texts();
        check(r.landing([&] { r.caretAtSceneStart(1); r.shell->splitChapter(); }), QStringLiteral("Q8: the relabel landed"));
        check(r.texts() == texts && r.chapterCount() == 2, QStringLiteral("Q8: two chapters, the same scenes and texts"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        r.caretAtSceneStart(0);
        r.shell->splitChapter();   // a chapter's first scene: nothing happens
        QTest::qWait(200);
        check(r.chapterCount() == 1 && !r.editor->isReadOnly(), QStringLiteral("Q8: at a chapter's first scene, nothing"));
    }
    // ── [SP-191] Q5: a repeat while the edit is in flight is ignored, not queued ───────────────────────────────────────
    {
        const int scenes = int(r.shell->sceneDocument().segments().size());
        check(r.landing([&] {
            r.caretAt(QStringLiteral("Alpha one. "));
            r.shell->splitScene();
            r.shell->splitScene();    // pressed again at once
            r.control(Qt::Key_Z);     // and Undo
        }), QStringLiteral("Q5: the split landed"));
        QTest::qWait(300);
        check(int(r.shell->sceneDocument().segments().size()) == scenes + 1,
              QStringLiteral("Q5: ONE new scene — the repeat and the undo were ignored"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        r.listMatchesDisk(QStringLiteral("after Q5"));
    }

    // ── [SP-191] live pass: a scene that STARTS WITH A HEADING. The caret rests after the hidden `## `, so the edit anchors
    //    before it: the heading goes with its scene, and a paste's last part keeps its own line ───────────────────────────
    {
        const QStringList before = r.state();
        const int delta = int(r.shell->sceneDocument().segments().size()) - 1;
        const QString deltaID = r.shell->sceneDocument().segments().at(delta).sceneID;
        const QString deltaText = QStringLiteral("## Delta\nDelta body.");
        const auto caretAfterPrefix = [&] {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(r.shell->sceneDocument().segments().at(r.shell->sceneDocument().sceneIndexForScene(deltaID)).bodyStart + 3);
            r.editor->setTextCursor(c);
            QApplication::processEvents();
        };
        check(r.landing([&] { caretAfterPrefix(); r.shell->splitScene(); }), QStringLiteral("heading: the split landed"));
        {
            const QList<SceneSegment>& segs = r.shell->sceneDocument().segments();
            check(segs.size() == before.size() + 1 && segs.at(delta).text.isEmpty() && segs.at(delta + 1).sceneID == deltaID &&
                      segs.at(delta + 1).text == deltaText,
                  QStringLiteral("heading: ⌃↩ puts a new scene BEFORE it — the `## ` stays with its heading"));
        }
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        check(r.landing([&] { caretAfterPrefix(); r.shell->splitChapter(); }), QStringLiteral("heading: the chapter split landed"));
        {
            const QList<SceneSegment>& segs = r.shell->sceneDocument().segments();
            check(r.chapterCount() == 2 && segs.at(delta).sceneID == deltaID && segs.at(delta).text == deltaText &&
                      segs.at(delta - 1).text == QStringLiteral("Gamma body."),
                  QStringLiteral("heading: ⌃⇧↩ moves the whole scene, `## ` included, into the new chapter"));
        }
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        r.select(QStringLiteral("Alpha two."), QStringLiteral("Beta"));
        r.control(Qt::Key_C);
        QApplication::processEvents();
        check(r.landing([&] { caretAfterPrefix(); r.control(Qt::Key_V); }), QStringLiteral("heading: the paste landed"));
        {
            const QList<SceneSegment>& segs = r.shell->sceneDocument().segments();
            const int at = r.shell->sceneDocument().sceneIndexForScene(deltaID);
            check(at == delta + 1 && segs.at(delta).text == QStringLiteral("Alpha two.") &&
                      segs.at(at).text == QStringLiteral("Beta\n") + deltaText,
                  QStringLiteral("heading: the pasted part keeps its own line — the heading stays a heading (got \"%1\")")
                      .arg(at >= 0 ? segs.at(at).text : QString()));
        }
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        check(r.state() == before, QStringLiteral("heading: each undone exactly"));
        r.listMatchesDisk(QStringLiteral("after the heading cases"));
    }

    r.shell->flushHistory();
    r.shell->saveDirtyScenes();
    r.shell->releaseProject();
    r.shell->hide();
    r.shell.reset();
    if (failures == 0) { std::printf("structural_baseline_smoke: all checks passed\n"); }
    return failures == 0 ? 0 : 1;
}
