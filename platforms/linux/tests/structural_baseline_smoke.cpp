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
#include "SceneInspector.hpp"
#include "ScriviBridge.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QClipboard>
#include <QEventLoop>
#include <QMessageBox>
#include <QScrollBar>
#include <QTest>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QTreeWidget>

#include <array>
#include <atomic>
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

// [I-0318]: Qt's own warnings (a caret set past the end is only a qWarning), passed on as before.
QStringList qtWarnings;
QtMessageHandler previousHandler = nullptr;
void catchWarnings(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    qtWarnings.append(message);
    if (previousHandler != nullptr) { previousHandler(type, context, message); }
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
    // ✅ [SP-191] rig pass: every chapter heading is DRAWN as one (bold), and no scene line is — judged from what the
    // presenter actually laid out, not from the text. A Markdown heading inside a scene (`## …`) is bold by design.
    void headingsDrawnRight(const QString& what)
    {
        int headings = 0;
        for (QTextBlock b = editor->document()->begin(); b.isValid(); b = b.next()) {
            const QString text = b.text();
            if (text.trimmed().isEmpty() || text.startsWith(QLatin1Char('#'))) { continue; }
            bool bold = false;
            for (const QTextLayout::FormatRange& f : b.layout()->formats()) {
                if (f.start == 0 && f.length >= text.size() && f.format.fontWeight() == QFont::Bold) { bold = true; }
            }
            const bool heading = shell->sceneDocument().sceneIndexForEditablePosition(b.position()) < 0;
            headings += heading ? 1 : 0;
            check(bold == heading, QStringLiteral("%1: \"%2\" is drawn as %3").arg(what, text,
                                                                                     bold ? QStringLiteral("a heading")
                                                                                          : QStringLiteral("body text")));
        }
        check(headings == chapterCount(), QStringLiteral("%1: one heading per chapter").arg(what));
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
        const qint64 made = ScriviBridge::guiThreadCallCount() - calls;   // before the checks below make their own
        if (structural) {
            check(shell->dirtySceneCount() == 0, QStringLiteral("%1: no scene left dirty (the core wrote them)").arg(what));
        }
        std::printf("[EP-053 baseline] %s: guiThreadCoreCalls=%lld projectLoads=%d documentBuilds=%d\n", what,
                    qlonglong(made), shell->loadCount() - loads,
                    shell->sceneDocument().builds() - builds);
        check(shell->sceneDocument().textMatchesDocument(), QStringLiteral("%1: every scene's text is its body").arg(what));
        // ✅ [SP-191]: nothing re-reads the project, and nothing rebuilds the document.
        check(shell->loadCount() == loads, QStringLiteral("%1: no project load").arg(what));
        check(shell->sceneDocument().builds() == builds, QStringLiteral("%1: no document build").arg(what));
        listMatchesDisk(QString::fromUtf8(what));
        headingsDrawnRight(QString::fromUtf8(what));
        return made;
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
        r.headingsDrawnRight(what);
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("%1: its undo landed").arg(what));
        check(r.state() == before, QStringLiteral("%1: undone in ONE step").arg(what));
        r.listMatchesDisk(what + QStringLiteral(", undone"));
        r.headingsDrawnRight(what + QStringLiteral(", undone"));
        check(r.landing([&] { r.control(Qt::Key_Z, Qt::ShiftModifier); }), QStringLiteral("%1: its redo landed").arg(what));
        check(r.state() == after, QStringLiteral("%1: redone exactly").arg(what));
        r.listMatchesDisk(what + QStringLiteral(", redone"));
        r.headingsDrawnRight(what + QStringLiteral(", redone"));
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
    // ── [SP-191] Q5: a repeat while the edit is in flight is ignored, not queued. ✅ [SP-196] v0.2 Q5: an Undo pressed
    //    meanwhile WAITS ITS TURN — it undoes the split once the split has landed ─────────────────────────────────────────
    {
        const int scenes = int(r.shell->sceneDocument().segments().size());
        int landed = 0;
        const auto counter = QObject::connect(r.shell.get(), &EditorShell::structuralEditFinished, [&](bool ok) { landed += ok; });
        r.caretAt(QStringLiteral("Alpha one. "));
        r.shell->splitScene();
        r.shell->splitScene();    // pressed again at once
        r.control(Qt::Key_Z);     // and Undo
        check(int(r.shell->sceneDocument().segments().size()) == scenes, QStringLiteral("Q5: the undo did not run ahead"));
        for (int i = 0; i < 300 && landed < 2; ++i) { QTest::qWait(50); }
        QTest::qWait(300);
        QObject::disconnect(counter);
        check(landed == 2 && int(r.shell->sceneDocument().segments().size()) == scenes,
              QStringLiteral("Q5: the split, then its undo (%1 landed)").arg(landed));
        r.listMatchesDisk(QStringLiteral("after Q5's undo"));
        check(r.landing([&] { r.control(Qt::Key_Z, Qt::ShiftModifier); }), QStringLiteral("(redone)"));
        check(int(r.shell->sceneDocument().segments().size()) == scenes + 1,
              QStringLiteral("Q5: ONE new scene to redo — the repeat was ignored"));
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
        r.headingsDrawnRight(QStringLiteral("after the heading cases"));
    }
    // ── [SP-191] rig pass: a chapter inserted BEFORE another renumbers it — and it must still be drawn as a heading ─────
    {
        check(r.landing([&] { r.caretAt(QStringLiteral("Gamma ")); r.shell->splitChapter(); }), QStringLiteral("(chapter 2)"));
        r.headingsDrawnRight(QStringLiteral("two chapters"));
        check(r.landing([&] { r.caretAt(QStringLiteral("Beta ")); r.shell->splitChapter(); }),
              QStringLiteral("a chapter inserted before chapter 2"));
        check(r.editor->document()->toPlainText().contains(QStringLiteral("Chapter 3")), QStringLiteral("(renumbered to 3)"));
        r.headingsDrawnRight(QStringLiteral("after the insert"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        r.headingsDrawnRight(QStringLiteral("after its undo"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        r.headingsDrawnRight(QStringLiteral("one chapter again"));
    }

    // ── [SP-188] (design R4, §4.3): a cut, Backspace or Delete across scenes REMOVES THE STRUCTURE — one `rangeCut` on the
    //    worker, the delta applied, undone by a paste and redone by the cut. ⛔ It was [I-0270]'s "keep the scenes". ───────
    {
        const auto bodyEnd = [&](int i) {
            const SceneSegment& seg = r.shell->sceneDocument().segments().at(i);
            return seg.bodyStart + seg.bodyLength;
        };
        const auto selectPositions = [&](int a, int b) {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(a);
            c.setPosition(b, QTextCursor::KeepAnchor);
            r.editor->setTextCursor(c);
            QApplication::processEvents();
        };
        const auto scenes = [&] { return int(r.shell->sceneDocument().segments().size()); };
        const QStringList before = r.state();
        const int n = scenes();
        const auto holding = [&](const QString& text) {
            return r.shell->sceneDocument().sceneIndexForCaret(int(r.editor->document()->find(text).selectionStart()));
        };
        const int ia = holding(QStringLiteral("Alpha two.")), ib = holding(QStringLiteral("Beta ")), ig = holding(QStringLiteral("Gamma "));
        const QString beta = r.shell->sceneDocument().segments().at(ib).sceneID;

        // Ctrl+X: joined directly, the scenes between gone; the measured path (no load, no build, list = disk).
        // The selection made FIRST, so the count starts at the key press. ✅ Traced 2026-10-22 (a temporary envelope print, in
        // a copy of the repo): both are the Scene Inspector's `scrivi_get_scene_notes` as the delta lands — none is the cut's.
        r.select(QStringLiteral("Alpha two."), QStringLiteral("Gamma "));
        QTest::qWait(200);
        check(r.measured("cut across scenes", [&] { r.control(Qt::Key_X); }) <= 2,
              QStringLiteral("cut across scenes: at most 2 GUI-thread core calls (the Inspector's notes reads)"));
        check(scenes() == n - (ig - ia) && r.shell->sceneDocument().segments().at(ia).text.endsWith(QStringLiteral("body."))
                  && r.texts().join(QString()).indexOf(QStringLiteral("Alpha two.")) < 0,
              QStringLiteral("cut: the ends joined, two scenes gone (got \"%1\", %2 scenes)")
                  .arg(r.shell->sceneDocument().segments().at(0).text).arg(scenes()));
        check(QApplication::clipboard()->text().startsWith(QStringLiteral("Alpha two.")), QStringLiteral("cut: other apps get the text"));
        // Q4: the clipboard's Range is the cut's — a paste elsewhere moves the scenes WITH their identity.
        check(r.landing([&] { r.caretAt(QStringLiteral("Delta body.")); r.control(Qt::Key_V); }), QStringLiteral("Q4: the paste landed"));
        check(r.shell->sceneDocument().sceneIndexForScene(beta) >= 0, QStringLiteral("Q4: the cut scene came back under its own ID"));
        r.listMatchesDisk(QStringLiteral("Q4: the paste of the cut"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the paste undone)"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the cut undone)"));
        check(r.state() == before, QStringLiteral("cut: undone exactly"));

        // Backspace and Delete: the same cut, the clipboard untouched.
        for (const Qt::Key key : {Qt::Key_Backspace, Qt::Key_Delete}) {
            QApplication::clipboard()->setText(QStringLiteral("kept"));
            roundTrip(key == Qt::Key_Backspace ? QStringLiteral("Backspace across scenes") : QStringLiteral("Delete across scenes"), [&] {
                r.select(QStringLiteral("Alpha two."), QStringLiteral("Gamma "));
                QTest::keyClick(r.editor, key);
            });
            check(QApplication::clipboard()->text() == QStringLiteral("kept"), QStringLiteral("Backspace / Delete leave the clipboard alone"));
        }

        // Q2: Backspace over exactly one scene separator merges the scenes.
        roundTrip(QStringLiteral("Backspace over a separator"), [&] {
            selectPositions(bodyEnd(0), r.shell->sceneDocument().segments().at(1).bodyStart);
            QTest::keyClick(r.editor, Qt::Key_Backspace);
        });
        check(r.landing([&] {
            selectPositions(bodyEnd(0), r.shell->sceneDocument().segments().at(1).bodyStart);
            QTest::keyClick(r.editor, Qt::Key_Backspace);
        }) && scenes() == n - 1, QStringLiteral("Q2: one scene fewer"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));

        // Q5: through the separator after scene 2, scenes 1 and 2 go whole — none is left empty.
        check(r.landing([&] {
            selectPositions(r.shell->sceneDocument().segments().at(1).bodyStart, r.shell->sceneDocument().segments().at(3).bodyStart);
            QTest::keyClick(r.editor, Qt::Key_Backspace);
        }), QStringLiteral("Q5: the cut landed"));
        check(scenes() == n - 2 && r.texts().indexOf(QString()) < 0, QStringLiteral("Q5: two scenes gone whole, none left empty (%1)")
                                                                          .arg(r.texts().join(QStringLiteral(" | "))));
        r.listMatchesDisk(QStringLiteral("Q5"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        check(r.state() == before, QStringLiteral("Q5: undone exactly"));

        // Q1: a whole chapter, from its title — the chapter goes; its undo brings it back.
        check(r.landing([&] { r.caretAt(QStringLiteral("Gamma "), true); r.shell->splitChapter(); }), QStringLiteral("(chapter 2)"));
        const QStringList twoChapters = r.state();
        check(r.landing([&] {
            selectPositions(bodyEnd(ig - 1), bodyEnd(scenes() - 1));
            QTest::keyClick(r.editor, Qt::Key_Backspace);
        }), QStringLiteral("Q1: the cut landed"));
        check(r.chapterCount() == 1 && scenes() == ig, QStringLiteral("Q1: the chapter and its scenes are gone (%1 scenes)").arg(scenes()));
        check(r.state().mid(0, ig) == twoChapters.mid(0, ig), QStringLiteral("Q1: the scenes before it untouched"));
        r.listMatchesDisk(QStringLiteral("Q1"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        check(r.state() == twoChapters, QStringLiteral("Q1: undone exactly, chapter and all"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(chapter 2 undone)"));

        // Q6: Ctrl+V over a cross-scene selection — the cut, then the paste; two undo steps.
        r.select(QStringLiteral("one."), QStringLiteral("Beta"));
        r.control(Qt::Key_C);
        QApplication::processEvents();
        int landed = 0;
        const auto counter = QObject::connect(r.shell.get(), &EditorShell::structuralEditFinished, [&](bool) { ++landed; });
        r.select(QStringLiteral("Gamma "), QStringLiteral("Delta body"));
        r.control(Qt::Key_V);
        for (int i = 0; i < 300 && landed < 2; ++i) { QTest::qWait(50); }
        QObject::disconnect(counter);
        check(landed == 2, QStringLiteral("Q6: the cut and then the paste landed (%1)").arg(landed));
        r.listMatchesDisk(QStringLiteral("Q6"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the paste undone)"));
        check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the cut undone)"));
        check(r.state() == before, QStringLiteral("Q6: two undos restore it exactly"));
    }

    // ── [I-0318]: a cut of most of the document, then a paste at its end — the caret is never set past the end ─────────
    {
        const QStringList before = r.state();
        previousHandler = qInstallMessageHandler(catchWarnings);
        for (const bool backward : {false, true}) {
            const QString what = backward ? QStringLiteral("I-0318 (selected backward)") : QStringLiteral("I-0318");
            qtWarnings.clear();
            check(r.landing([&] {
                r.select(QStringLiteral("one."), QStringLiteral("Delta bo"));
                if (backward) {   // the anchor at the end, as a drag from the bottom up leaves it
                    QTextCursor c = r.editor->textCursor();
                    const int a = c.anchor(), f = c.position();
                    c.setPosition(f);
                    c.setPosition(a, QTextCursor::KeepAnchor);
                    r.editor->setTextCursor(c);
                }
                r.control(Qt::Key_X);
            }), QStringLiteral("%1: the cut landed").arg(what));
            check(r.landing([&] { r.editor->moveCursor(QTextCursor::End); r.control(Qt::Key_V); }),
                  QStringLiteral("%1: the paste at the end landed").arg(what));
            r.listMatchesDisk(what);
            check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the paste undone)"));
            check(r.landing([&] { r.control(Qt::Key_Z); }), QStringLiteral("(the cut undone)"));
            check(r.state() == before, QStringLiteral("%1: two undos restore it exactly").arg(what));
            for (const QString& w : std::as_const(qtWarnings)) {
                check(!w.contains(QStringLiteral("out of range")), QStringLiteral("%1: %2").arg(what, w));
            }
        }
        qInstallMessageHandler(previousHandler);
    }

    r.shell->flushHistory();
    r.shell->saveDirtyScenes();
    // ── ✅ [SP-197] EP-052 AC8: a GATE — the caret, a selection and a scroll make NO file operation on the GUI thread ─────
    // (the real Inspector follows each one: its read, the notes included, runs on a worker). Over a share each is a round trip.
    {
        QList<ScriviBridge::GuiCoreCall> calls;
        ScriviBridge::guiCoreCallObserver() = [&](const ScriviBridge::GuiCoreCall& c) { calls.append(c); };
        const auto noFileOps = [&](const char* gesture, const std::function<void()>& act) {
            QTest::qWait(300);                 // the previous gesture's follow-ups land first
            calls.clear();
            act();
            QTest::qWait(500);                 // the Inspector's read lands, and its revision check runs
            qint64 ops = 0;
            QStringList who;
            for (const auto& c : calls) {
                ops += c.fileOps;
                if (c.fileOps > 0) { who.append(c.endpoint); }
            }
            check(ops == 0, QStringLiteral("SP-197: %1 makes no file operation on the GUI thread (%2)")
                                .arg(QString::fromUtf8(gesture), who.join(QStringLiteral(", "))));
        };
        const auto at = [&](int scene, int offset) {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(r.shell->sceneDocument().segments().at(scene).bodyStart + offset);
            r.editor->setTextCursor(c);
            QApplication::processEvents();
        };
        noFileOps("a click in the same scene", [&] { at(0, 1); at(0, 3); });
        noFileOps("a click into another scene (the Inspector follows)", [&] { at(2, 1); });
        noFileOps("a drag-select across scenes", [&] {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(r.shell->sceneDocument().segments().at(0).bodyStart + 1);
            c.setPosition(r.shell->sceneDocument().segments().at(1).bodyStart + 1, QTextCursor::KeepAnchor);
            r.editor->setTextCursor(c);
            QApplication::processEvents();
        });
        noFileOps("a scroll", [&] { r.editor->verticalScrollBar()->setValue(r.editor->verticalScrollBar()->maximum()); });
        ScriviBridge::guiCoreCallObserver() = nullptr;
    }
    // ── ✅ [SP-197] rig: scenes passed by while a read is out start NO read of their own ─────────────────────────────────
    // (On the rig every scrolled-past scene's read ran, each holding the project's disk lock, and a cut waited 10 minutes.)
    // No event is processed between the ten scene changes, so the first read cannot land before the last change.
    {
        std::atomic<int> reads{0};
        ScriviBridge::workerCoreCallObserver() = [&](const ScriviBridge::GuiCoreCall& c) {
            if (c.endpoint == QLatin1String("scrivi_load_scene_inspector")) { ++reads; }
        };
        auto* inspector = r.shell->findChild<SceneInspector*>();
        check(inspector != nullptr, QStringLiteral("SP-197 rig: the shell has an Inspector"));
        if (inspector != nullptr) {
            QTest::qWait(500);   // the earlier gestures' reads land first
            reads = 0;
            const auto& segs = r.shell->sceneDocument().segments();
            for (int i = 0; i < 10; ++i) { inspector->setScene(segs.at(i % 2).sceneID); }
            QTest::qWait(1500);
            ScriviBridge::workerCoreCallObserver() = nullptr;
            check(reads.load() >= 1 && reads.load() <= 4,
                  QStringLiteral("SP-197 rig: ten scene changes make one read and one for the last (%1 reads)").arg(reads.load()));
            auto* tree = inspector->findChild<QTreeWidget*>();
            check(tree != nullptr && tree->isEnabled(), QStringLiteral("SP-197 rig: the last scene's read landed (cards enabled)"));
        }
        ScriviBridge::workerCoreCallObserver() = nullptr;
    }
    // ── [SP-188] AC11 / EP-052 v0.2: the main-thread inventory (on demand) ────────────────────────────────────────────────
    // ⚠️ A MEASUREMENT, not a gate: every GUI-thread core call each writer gesture makes, with its file operations (the cost
    // over a share, one network round trip each). The Mac's twin is `MainThreadInventoryTests`. Run with SCRIVI_INVENTORY=1.
    if (qEnvironmentVariableIsSet("SCRIVI_INVENTORY")) {
        QList<ScriviBridge::GuiCoreCall> calls;
        ScriviBridge::guiCoreCallObserver() = [&](const ScriviBridge::GuiCoreCall& c) { calls.append(c); };
        const auto measure = [&](const char* gesture, const std::function<void()>& act) {
            QTest::qWait(300);                 // the previous gesture's follow-ups land first
            calls.clear();
            act();
            QTest::qWait(300);                 // this gesture's debounced ones
            QStringList order;
            QHash<QString, std::array<qint64, 3>> by;   // calls, file ops, ms
            qint64 ops = 0;
            for (const auto& c : calls) {
                if (!by.contains(c.endpoint)) { order.append(c.endpoint); }
                auto& e = by[c.endpoint];
                e[0] += 1; e[1] += c.fileOps; e[2] += c.ms;
                ops += c.fileOps;
            }
            QStringList detail;
            for (const QString& k : order) {
                detail.append(QStringLiteral("%1×%2 (%3 ops, %4 ms)").arg(k).arg(by[k][0]).arg(by[k][1]).arg(by[k][2]));
            }
            std::printf("[INVENTORY] %s | calls %lld | file ops %lld | %s\n", gesture, qlonglong(calls.size()), qlonglong(ops), detail.isEmpty() ? "—" : qUtf8Printable(detail.join(QStringLiteral(", "))));
        };
        const auto at = [&](int scene, int offset) {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(r.shell->sceneDocument().segments().at(scene).bodyStart + offset);
            r.editor->setTextCursor(c);
            QApplication::processEvents();
        };
        measure("click in the same scene", [&] { at(0, 1); at(0, 3); });
        measure("click into another scene (the Inspector follows)", [&] { at(2, 1); });
        measure("drag-select across scenes", [&] {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(r.shell->sceneDocument().segments().at(0).bodyStart + 1);
            c.setPosition(r.shell->sceneDocument().segments().at(1).bodyStart + 1, QTextCursor::KeepAnchor);
            r.editor->setTextCursor(c);
            QApplication::processEvents();
        });
        measure("scroll", [&] { r.editor->verticalScrollBar()->setValue(r.editor->verticalScrollBar()->maximum()); });
        at(2, 1);
        measure("type a word", [&] { QTest::keyClicks(r.editor, QStringLiteral("quick ")); });
        measure("end a sentence", [&] { QTest::keyClicks(r.editor, QStringLiteral("Done. ")); });
        measure("autosave (the save timer)", [&] { r.shell->saveDirtyScenes(); });
        QTest::keyClicks(r.editor, QStringLiteral("again. "));
        measure("Ctrl+Z (typing)", [&] { r.shell->undo(); });
        measure("Ctrl+Shift+Z (typing)", [&] { r.shell->redo(); });
        measure("Ctrl+X across scenes (the GUI thread's part)", [&] {
            QTextCursor c = r.editor->textCursor();
            c.setPosition(r.shell->sceneDocument().segments().at(0).bodyStart + 1);
            c.setPosition(r.shell->sceneDocument().segments().at(1).bodyStart + 1, QTextCursor::KeepAnchor);
            r.editor->setTextCursor(c);
            r.landing([&] { r.control(Qt::Key_X); });
        });
        measure("Ctrl+Z of that cut (the GUI thread's part)", [&] { r.landing([&] { r.control(Qt::Key_Z); }); });
        ScriviBridge::guiCoreCallObserver() = nullptr;
    }
    r.shell->releaseProject();
    r.shell->hide();
    r.shell.reset();

    // ── [SP-188] Q3: a cut from inside bold — the survivor balanced by the core, its undo exact (a project of its own) ────
    {
        Rig b;
        b.projectDir = r.projectDir + QStringLiteral("-bold");
        b.appSupport = r.appSupport;
        b.bridge.bootstrap(QStringLiteral("Baseline Tester"), b.appSupport);
        b.projectID = b.bridge.createProject(b.projectDir, b.appSupport, QStringLiteral("Bold"), QStringLiteral("bold"))
                          .value(QStringLiteral("projectID")).toString();
        const QVariantMap s0 = b.bridge.openProject(b.projectDir, b.appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
        const auto saveIn = [&](const QVariantMap& sc, const QString& text) {
            b.bridge.saveScene(b.projectID, b.projectDir, b.appSupport, sc.value(QStringLiteral("sceneID")).toString(),
                               sc.value(QStringLiteral("metadataPath")).toString(), sc.value(QStringLiteral("contentPath")).toString(),
                               text, 0, 0, 0.0);
        };
        saveIn(s0, QStringLiteral("**Alpha beta.**"));
        saveIn(b.bridge.createScene(b.projectDir, b.appSupport, b.projectID, s0.value(QStringLiteral("chapterID")).toString(),
                                    s0.value(QStringLiteral("sceneID")).toString()),
               QStringLiteral("Gamma delta."));
        check(b.load(), QStringLiteral("Q3: the bold project loaded"));
        armConfirmer(b.shell.get());   // an unexpected notice must fail the smoke, not hang it
        const QStringList before = b.state();
        check(b.landing([&] {
            QTextCursor c = b.editor->textCursor();
            const QList<SceneSegment>& segs = b.shell->sceneDocument().segments();
            c.setPosition(segs.at(0).bodyStart + 10);   // **Alpha be|ta.**
            c.setPosition(segs.at(1).bodyStart + 6, QTextCursor::KeepAnchor);   // Gamma |delta.
            b.editor->setTextCursor(c);
            b.control(Qt::Key_X);
        }), QStringLiteral("Q3: the cut landed"));
        check(b.texts() == QStringList({QStringLiteral("**Alpha be**delta.")}),
              QStringLiteral("Q3: the survivor balanced (got %1)").arg(b.texts().join(QStringLiteral(" | "))));
        b.listMatchesDisk(QStringLiteral("Q3"));
        // Q4: the clipboard's Range has NO join — a paste at the survivor itself is a plain paste, never an undo record
        // (with the join it would first put the plain join back: `**Alpha bedelta.`).
        check(b.landing([&] {
            QTextCursor c = b.editor->textCursor();
            const SceneSegment& seg = b.shell->sceneDocument().segments().at(0);
            c.setPosition(seg.bodyStart + seg.bodyLength);
            b.editor->setTextCursor(c);
            b.control(Qt::Key_V);
        }), QStringLiteral("Q4: the paste at the survivor landed"));
        // ... and its pieces are the copy's, balanced (`**ta.**`), not the stored `ta.**`.
        check(b.texts() == QStringList({QStringLiteral("**Alpha be**delta.**ta.**"), QStringLiteral("Gamma ")}),
              QStringLiteral("Q4: a plain paste of the balanced pieces (got %1)").arg(b.texts().join(QStringLiteral(" | "))));
        check(b.landing([&] { b.control(Qt::Key_Z); }), QStringLiteral("(the paste undone)"));
        check(b.landing([&] { b.control(Qt::Key_Z); }), QStringLiteral("(undone)"));
        check(b.state() == before, QStringLiteral("Q3: undone exactly"));
        b.listMatchesDisk(QStringLiteral("Q3, undone"));
        b.shell->flushHistory();
        b.shell->releaseProject();
        b.shell->hide();
        b.shell.reset();
    }
    // ── [I-0313] (Apple's): typing into the first of two EMPTY scenes stays in that scene (a project of its own) ────────
    {
        Rig e;
        e.projectDir = r.projectDir + QStringLiteral("-empty");
        e.appSupport = r.appSupport;
        e.bridge.bootstrap(QStringLiteral("Baseline Tester"), e.appSupport);
        e.projectID = e.bridge.createProject(e.projectDir, e.appSupport, QStringLiteral("Empty"), QStringLiteral("empty"))
                          .value(QStringLiteral("projectID")).toString();
        const QVariantMap s0 = e.bridge.openProject(e.projectDir, e.appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
        const auto saveIn = [&](const QVariantMap& sc, const QString& text) {
            e.bridge.saveScene(e.projectID, e.projectDir, e.appSupport, sc.value(QStringLiteral("sceneID")).toString(),
                               sc.value(QStringLiteral("metadataPath")).toString(), sc.value(QStringLiteral("contentPath")).toString(),
                               text, 0, 0, 0.0);
        };
        saveIn(s0, QStringLiteral("Beta."));
        QVariantMap prev = s0;
        for (const QString& body : {QString(), QString(), QStringLiteral("Gamma.")}) {
            const QVariantMap made = e.bridge.createScene(e.projectDir, e.appSupport, e.projectID,
                                                          s0.value(QStringLiteral("chapterID")).toString(),
                                                          prev.value(QStringLiteral("sceneID")).toString());
            saveIn(made, body);
            prev = made;
        }
        check(e.load(), QStringLiteral("I-0313: the empty-scenes project loaded"));
        armConfirmer(e.shell.get());
        e.caretAtSceneStart(1);
        QTest::keyClicks(e.editor, QStringLiteral("Z"));
        QApplication::processEvents();
        check(e.texts() == QStringList({QStringLiteral("Beta."), QStringLiteral("Z"), QString(), QStringLiteral("Gamma.")}),
              QStringLiteral("I-0313: the Z is in the first empty scene (got %1)").arg(e.texts().join(QStringLiteral(" | "))));
        e.shell->flushHistory();
        e.shell->releaseProject();
        e.shell->hide();
        e.shell.reset();
    }
    if (failures == 0) { std::printf("structural_baseline_smoke: all checks passed\n"); }
    return failures == 0 ? 0 : 1;
}
