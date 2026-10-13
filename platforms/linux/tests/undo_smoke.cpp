// undo_smoke.cpp — EP-026 S2 (SP-178 / T-0605): undo / redo on Linux, end to end.
//
// A REAL `EditorShell` loads a real project; keys go in through `QTest` (the real dispatch, ShortcutOverride included —
// `feedback_test_through_the_real_dispatch`); every claim about text is read back FROM DISK, never from the widget.
//
// ✅ What it proves (SP-178 AC1–AC5, AC7):
//   • a sentence typed continuously is ONE undo step; the next sentence another (the core decides, SP-177);
//   • Ctrl+Z / Ctrl+Shift+Z undo and redo, and the scene file follows; ⛔ Ctrl+Y does nothing;
//   • a paste is its own step;
//   • a delete across a scene break is ONE step that restores EVERY scene ([I-0270]);
//   • a split stops undo with Apple's notice; its reload keeps the SAME history (no new session);
//   • history survives close → reopen: the first undo warns "previous session" once, and the clean reopen raised no
//     external-change barrier (the save path reported its bytes).
// ⚠️ NOT covered (live pass only): the 45 s idle timer on a real clock, Edit ▸ Undo with the editor unfocused, how it feels.
//
//   argv[1] = project dir to create; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen (set by the script).

#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "HistoryCapture.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptEscapes.hpp"
#include "ScriviBridge.hpp"

#include <QApplication>
#include <QClipboard>
#include <QEventLoop>
#include <QMessageBox>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <cstdio>
#include <memory>

namespace {

int failures = 0;

// Typed text is stored ESCAPED — every ASCII mark gets a backslash (EP-049's ruled write half).
QString esc(const QString& typed) { return ManuscriptEscapes::escape(typed); }

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

// Every modal notice is dismissed and its text recorded (the barrier / session notices are `QMessageBox::exec`).
QStringList notices;
void armNoticeCatcher(QObject* owner)
{
    auto* t = new QTimer(owner);
    t->setInterval(20);
    QObject::connect(t, &QTimer::timeout, [] {
        if (auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            notices.append(box->text());
            box->accept();
        }
    });
    t->start();
}

struct Rig {
    ScriviBridge bridge;
    QString projectDir;
    QString appSupport;
    QStringList sceneIDs;
    std::unique_ptr<EditorShell> shell;
    ManuscriptEditor* editor = nullptr;

    bool load()
    {
        shell = std::make_unique<EditorShell>();
        QEventLoop loop;
        bool ok = false;
        // ⚠️ Scoped to `loop`, not the shell: a later reload (a split) would otherwise quit a dead loop.
        QObject::connect(shell.get(), &EditorShell::loadFinished, &loop, [&](bool r) { ok = r; loop.quit(); });
        shell->load(projectDir, appSupport, QStringLiteral("Undo"));
        QTimer::singleShot(60000, &loop, &QEventLoop::quit);
        loop.exec();
        editor = shell->findChild<ManuscriptEditor*>();
        if (!ok || editor == nullptr) {
            return false;
        }
        shell->show();
        shell->activateWindow();
        check(QTest::qWaitForWindowActive(shell.get()), QStringLiteral("the window became active (shortcuts need it)"));
        editor->setFocus();
        QApplication::processEvents();
        return true;
    }
    // ScriviWindow's teardown order: flush → save → stamp → release (closes the history).
    void close()
    {
        shell->flushHistory();
        shell->saveDirtyScenes();
        shell->releaseProject();
        shell->hide();   // as a window closes before it is destroyed (a visible shell's focus-out reads its dying document)
        shell.reset();
        editor = nullptr;
    }
    QString disk(int i)
    {
        shell->saveDirtyScenes();
        return bridge.readSceneTexts(projectDir, {sceneIDs.at(i)}).value(sceneIDs.at(i));
    }
    // The caret right after `marker`'s first occurrence (or before it with `before`).
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
    void type(const QString& text) { QTest::keyClicks(editor, text); QApplication::processEvents(); }
    void key(Qt::Key k, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::keyClick(editor, k, m); settle(); }
    // A Ctrl chord as X11 delivers it: the CONTROL character as its text (format_smoke's finding).
    void control(Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier | extra);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier | extra);
        settle();
    }
    void undo() { control(Qt::Key_Z); }
    void redo() { control(Qt::Key_Z, Qt::ShiftModifier); }
    static void settle() { QTest::qWait(60); }   // the notice catcher and queued work
};

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);   // a crash must not swallow the FAIL lines
    if (argc < 2) {
        std::fprintf(stderr, "usage: undo_smoke <projectDir>\n");
        return 2;
    }
    armNoticeCatcher(&app);

    Rig rig;
    rig.projectDir = QString::fromUtf8(argv[1]);
    rig.appSupport = scrivi::linux_app::appSupportRoot();
    rig.bridge.bootstrap(QStringLiteral("Scrivi"), rig.appSupport);
    const QString projectID = rig.bridge.createProject(rig.projectDir, rig.appSupport, QStringLiteral("Undo"),
                                                       QStringLiteral("undo")).value(QStringLiteral("projectID")).toString();
    check(!projectID.isEmpty(), QStringLiteral("project created"));

    // Three scenes, seeded on disk BEFORE the first open (so they are each scene's floor, not undoable edits).
    const QStringList seeds{QStringLiteral("Alpha one."), QStringLiteral("Bravo two."), QStringLiteral("Charlie three.")};
    {
        QVariantList scenes = rig.bridge.openProject(rig.projectDir, rig.appSupport).value(QStringLiteral("scenes")).toList();
        const QString chapterID = scenes.first().toMap().value(QStringLiteral("chapterID")).toString();
        for (int i = 1; i < 3; ++i) {
            rig.bridge.createScene(rig.projectDir, rig.appSupport, projectID, chapterID,
                                   scenes.last().toMap().value(QStringLiteral("sceneID")).toString());
            scenes = rig.bridge.openProject(rig.projectDir, rig.appSupport).value(QStringLiteral("scenes")).toList();
        }
        for (int i = 0; i < 3; ++i) {
            const QVariantMap s = scenes.at(i).toMap();
            rig.sceneIDs.append(s.value(QStringLiteral("sceneID")).toString());
            rig.bridge.saveScene(projectID, rig.projectDir, rig.appSupport, rig.sceneIDs.last(),
                                 s.value(QStringLiteral("metadataPath")).toString(),
                                 s.value(QStringLiteral("contentPath")).toString(), seeds.at(i), 0, 0, 0.0);
        }
        rig.bridge.closeProject(rig.projectDir);
    }

    // ── 1. First open: the history opens; a sentence is ONE step ──────────────────────────────────────────────────
    check(rig.load(), QStringLiteral("project loaded"));
    check(rig.shell->history() != nullptr && rig.shell->history()->isOpen(), QStringLiteral("the undo history opened"));
    check(!rig.shell->canUndo(), QStringLiteral("nothing to undo on a fresh open (the seeds are the floor)"));

    rig.caretAt(QStringLiteral("Alpha one."));
    rig.type(QStringLiteral(" Now is the winter of our discontent."));
    rig.type(QStringLiteral(" Made glorious summer."));
    check(rig.disk(0) == QStringLiteral("Alpha one.") + esc(QStringLiteral(" Now is the winter of our discontent. Made glorious summer.")),
          QStringLiteral("typed text reached disk"));
    check(rig.shell->canUndo(), QStringLiteral("typing made something to undo"));

    rig.undo();
    check(rig.disk(0) == QStringLiteral("Alpha one.") + esc(QStringLiteral(" Now is the winter of our discontent.")),
          QStringLiteral("Ctrl+Z took back ONE sentence (got: \"%1\")").arg(rig.disk(0)));
    rig.undo();
    check(rig.disk(0) == QStringLiteral("Alpha one."),
          QStringLiteral("a second Ctrl+Z took back the first sentence — typed continuously, it was ONE step (got: \"%1\")")
              .arg(rig.disk(0)));
    check(!rig.shell->canUndo(), QStringLiteral("undo stops at the floor"));

    // ⛔ Ctrl+Y is not redo (EP-026 Q3).
    rig.control(Qt::Key_Y);
    check(rig.disk(0) == QStringLiteral("Alpha one."), QStringLiteral("Ctrl+Y did nothing"));

    rig.redo();
    rig.redo();
    check(rig.disk(0) == QStringLiteral("Alpha one.") + esc(QStringLiteral(" Now is the winter of our discontent. Made glorious summer.")),
          QStringLiteral("Ctrl+Shift+Z redid both sentences (got: \"%1\")").arg(rig.disk(0)));
    check(!rig.shell->canRedo(), QStringLiteral("nothing left to redo"));

    // ── 2. A paste is its own step ────────────────────────────────────────────────────────────────────────────────
    rig.caretAt(QStringLiteral("Bravo two."));
    rig.type(QStringLiteral(" Typed"));
    QApplication::clipboard()->setText(QStringLiteral(" pasted"));
    rig.control(Qt::Key_V);
    check(rig.disk(1) == QStringLiteral("Bravo two. Typed pasted"), QStringLiteral("the paste landed (got: \"%1\")")
                                                                        .arg(rig.disk(1)));
    rig.undo();
    check(rig.disk(1) == QStringLiteral("Bravo two. Typed"),
          QStringLiteral("Ctrl+Z took back the paste alone (got: \"%1\")").arg(rig.disk(1)));
    rig.undo();
    check(rig.disk(1) == QStringLiteral("Bravo two."), QStringLiteral("then the typing before it"));

    // ── 3. A delete across a scene break is ONE step restoring every scene ([I-0270]) ─────────────────────────────
    const QString s0 = rig.disk(0);
    const QString s1 = rig.disk(1);
    const QString s2 = rig.disk(2);
    rig.select(QStringLiteral("Made glorious"), QStringLiteral("Charlie"));
    rig.key(Qt::Key_Backspace);
    check(rig.disk(0) != s0 && rig.disk(1).isEmpty() && rig.disk(2) == QStringLiteral(" three."),
          QStringLiteral("the cross-scene delete removed text from all three scenes (got: \"%1\" | \"%2\" | \"%3\")")
              .arg(rig.disk(0), rig.disk(1), rig.disk(2)));
    rig.undo();
    check(rig.disk(0) == s0 && rig.disk(1) == s1 && rig.disk(2) == s2,
          QStringLiteral("ONE Ctrl+Z restored every scene (got: \"%1\" | \"%2\" | \"%3\")")
              .arg(rig.disk(0), rig.disk(1), rig.disk(2)));
    rig.redo();
    check(rig.disk(1).isEmpty(), QStringLiteral("ONE Ctrl+Shift+Z re-applied it"));
    rig.undo();

    // ── 4. Close → reopen: history persists; the first undo warns once; no external-change barrier ─────────────────
    rig.close();
    check(rig.load(), QStringLiteral("project reopened"));
    check(rig.shell->canUndo(), QStringLiteral("history survived close → reopen"));
    notices.clear();
    rig.undo();   // the newest step is the cross-scene redo's undo… i.e. the previous session's last applied step
    check(notices.contains(QStringLiteral("Undoing changes from a previous session")),
          QStringLiteral("the first undo into the previous session warned (notices: %1)").arg(notices.join(QStringLiteral(" / "))));
    for (const QString& n : std::as_const(notices)) {
        check(!n.startsWith(QStringLiteral("Can't undo")),
              QStringLiteral("no barrier on a clean reopen — the save path reported its bytes (got: %1)").arg(n));
    }
    const int warnings = notices.count(QStringLiteral("Undoing changes from a previous session"));
    rig.undo();
    check(notices.count(QStringLiteral("Undoing changes from a previous session")) == warnings,
          QStringLiteral("the session warning is shown ONCE per crossing"));
    rig.redo();
    rig.redo();

    // ── 4b. A merge rewrites the survivor on disk: the next edit there, undone, must keep the merged-in text ─────────
    rig.caretAt(QStringLiteral("Bravo two."), /*before=*/true);   // the start of the second scene
    {
        QEventLoop loop;
        QObject::connect(rig.shell.get(), &EditorShell::loadFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        QTest::keyClick(rig.editor, Qt::Key_Backspace, Qt::ControlModifier);   // merge into the previous scene → a reload
        loop.exec();
    }
    rig.editor->setFocus();
    Rig::settle();
    notices.clear();
    const QString merged = rig.disk(0);
    check(merged.contains(QStringLiteral("Bravo two.")), QStringLiteral("the merge joined the scenes (got: \"%1\")").arg(merged));
    {
        // [I-0300]: the caret lands at the JOIN SEAM — the end of the predecessor's own text, before the blank-line join —
        // once the reload has finished (it was placed on the document the reload replaced).
        const int seam = rig.editor->document()->find(QStringLiteral("Bravo two.")).selectionStart()
                         - (merged.contains(QStringLiteral("\n\nBravo two.")) ? 2 : 0);
        check(rig.editor->textCursor().position() == seam,
              QStringLiteral("[I-0300] after a merge the caret is at the seam (at %1, want %2)")
                  .arg(rig.editor->textCursor().position()).arg(seam));
    }
    rig.caretAt(QStringLiteral("Bravo two."));
    rig.type(QStringLiteral(" Joined."));
    rig.undo();
    check(rig.disk(0) == merged,
          QStringLiteral("undoing an edit after a merge keeps the merged-in text (got: \"%1\", want \"%2\")")
              .arg(rig.disk(0), merged));


    // ── 5. A split stops undo with Apple's notice; its reload keeps the SAME history ───────────────────────────────
    HistoryCapture* before = rig.shell->history();
    rig.caretAt(QStringLiteral("Charlie three."));
    rig.type(QStringLiteral(" Then more."));
    rig.caretAt(QStringLiteral("Then"), /*before=*/true);
    {
        QEventLoop loop;
        QObject::connect(rig.shell.get(), &EditorShell::loadFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        QTest::keyClick(rig.editor, Qt::Key_Return, Qt::ControlModifier);   // mid-scene split → a reload
        loop.exec();
    }
    rig.editor->setFocus();
    Rig::settle();
    // [I-0300]: the caret lands at the start of the new (tail) scene — placed AFTER the asynchronous reload, not before it.
    check(rig.editor->textCursor().position() == rig.editor->document()->find(QStringLiteral("Then more")).selectionStart(),
          QStringLiteral("[I-0300] after the split the caret is at the start of the new scene (at %1, want %2)")
              .arg(rig.editor->textCursor().position())
              .arg(rig.editor->document()->find(QStringLiteral("Then more")).selectionStart()));
    check(rig.shell->history() == before && before->isOpen(), QStringLiteral("the split's reload kept the SAME history"));
    notices.clear();
    rig.undo();
    check(notices == QStringList{QStringLiteral("Can't undo past creating a scene")},
          QStringLiteral("undo stopped at the split with Apple's notice — and no session warning (notices: %1)")
              .arg(notices.join(QStringLiteral(" / "))));


    // After the split the scene holds only the HEAD of its old text. The next edit there, undone, takes back that edit
    // alone — it must not bring back the tail that moved to the new scene.
    const QString head = rig.disk(2);
    rig.caretAt(QStringLiteral("Charlie three."));
    rig.type(QStringLiteral(" Added."));
    rig.undo();
    check(rig.disk(2) == head,
          QStringLiteral("undoing an edit after a split restores the split scene's HEAD, not its pre-split text (got: \"%1\", want \"%2\")")
              .arg(rig.disk(2), head));

    // ── 6. The split wrote the scene files directly; reported to history, a clean reopen raises no external change ──
    rig.close();
    check(rig.load(), QStringLiteral("project reopened after the split"));
    notices.clear();
    rig.undo();
    rig.undo();
    for (const QString& n : std::as_const(notices)) {
        check(!n.contains(QStringLiteral("outside Scrivi")),
              QStringLiteral("no external-change barrier after the split's own writes (got: %1)").arg(n));
    }

    // ── 7. [I-0300] A new chapter mid-scene (Ctrl+Shift+Return): the caret moves INTO the new chapter ─────────────────
    rig.caretAt(QStringLiteral("two."), /*before=*/true);
    {
        QEventLoop loop;
        QObject::connect(rig.shell.get(), &EditorShell::loadFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        QTest::keyClick(rig.editor, Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        loop.exec();
    }
    Rig::settle();
    check(rig.editor->textCursor().position() == rig.editor->document()->find(QStringLiteral("two.")).selectionStart(),
          QStringLiteral("[I-0300] after a new chapter the caret is at the start of its first scene (at %1, want %2)")
              .arg(rig.editor->textCursor().position())
              .arg(rig.editor->document()->find(QStringLiteral("two.")).selectionStart()));
    // …and at the END of the last scene (the rig's step 24): a new, EMPTY chapter — the caret goes into its blank scene.
    {
        QTextCursor end = rig.editor->textCursor();
        end.movePosition(QTextCursor::End);
        rig.editor->setTextCursor(end);
        QApplication::processEvents();
        QEventLoop loop;
        QObject::connect(rig.shell.get(), &EditorShell::loadFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        QTest::keyClick(rig.editor, Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        loop.exec();
    }
    Rig::settle();
    {
        QTextCursor docEnd(rig.editor->document());
        docEnd.movePosition(QTextCursor::End);
        check(rig.editor->textCursor().position() == docEnd.position(),
              QStringLiteral("[I-0300] after a new chapter at the end, the caret is in its blank scene (at %1, want %2)")
                  .arg(rig.editor->textCursor().position())
                  .arg(docEnd.position()));
    }

    rig.close();
    if (failures == 0) {
        std::printf("undo-ok\n");
    }
    return failures == 0 ? 0 : 1;
}
