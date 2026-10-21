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
    // [SP-191] `act` starts a structural edit — a split, a merge, or an undo / redo across one: wait for it to LAND (it runs
    // on a worker; nothing reloads). True when it was applied.
    bool landing(const std::function<void()>& act)
    {
        QEventLoop loop;
        bool ok = false;
        bool done = false;
        QObject::connect(shell.get(), &EditorShell::structuralEditFinished, &loop, [&](bool r) {
            ok = r;
            done = true;
            loop.quit();
        });
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        const int loads = shell->loadCount();
        act();
        if (!done) {
            loop.exec();
        }
        settle();
        check(shell->loadCount() == loads, QStringLiteral("[SP-191] the structural edit reloaded nothing"));
        return ok;
    }
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
    check(rig.landing([&] { QTest::keyClick(rig.editor, Qt::Key_Backspace, Qt::ControlModifier); }),   // merge into the previous
          QStringLiteral("the merge landed"));
    rig.editor->setFocus();
    Rig::settle();
    notices.clear();
    const QString merged = rig.disk(0);
    check(merged.contains(QStringLiteral("Bravo two.")), QStringLiteral("the merge joined the scenes (got: \"%1\")").arg(merged));
    {
        // [I-0300]: the caret lands at the JOIN SEAM — the end of the predecessor's own text. ✅ [SP-191] Q3a: joined
        // DIRECTLY, no blank line.
        check(!merged.contains(QStringLiteral("\n\nBravo two.")), QStringLiteral("Q3a: the merge joined directly (got: \"%1\")").arg(merged));
        const int seam = rig.editor->document()->find(QStringLiteral("Bravo two.")).selectionStart();
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


    // ── 5. ✅ [SP-191] Q3: a split is ONE undoable step — no "Can't undo past…"; it keeps the SAME history ────────────
    HistoryCapture* before = rig.shell->history();
    rig.caretAt(QStringLiteral("Charlie three."));
    rig.type(QStringLiteral(" Then more."));
    rig.caretAt(QStringLiteral("Then"), /*before=*/true);
    check(rig.landing([&] { QTest::keyClick(rig.editor, Qt::Key_Return, Qt::ControlModifier); }),   // a mid-scene split
          QStringLiteral("the split landed"));
    rig.editor->setFocus();
    Rig::settle();
    // [I-0300]: the caret lands at the start of the new (tail) scene.
    check(rig.editor->textCursor().position() == rig.editor->document()->find(QStringLiteral("Then more")).selectionStart(),
          QStringLiteral("[I-0300] after the split the caret is at the start of the new scene (at %1, want %2)")
              .arg(rig.editor->textCursor().position())
              .arg(rig.editor->document()->find(QStringLiteral("Then more")).selectionStart()));
    check(rig.shell->history() == before && before->isOpen(), QStringLiteral("the split kept the SAME history"));
    check(rig.shell->canUndo(), QStringLiteral("Q3: the split is undoable — no barrier"));

    // After the split the scene holds only the HEAD of its old text. The next edit there, undone, takes back that edit
    // alone — it must not bring back the tail that moved to the new scene.
    const QString head = rig.disk(2);
    rig.caretAt(QStringLiteral("Charlie three."));
    rig.type(QStringLiteral(" Added."));
    rig.undo();
    check(rig.disk(2) == head,
          QStringLiteral("undoing an edit after a split restores the split scene's HEAD, not its pre-split text (got: \"%1\", want \"%2\")")
              .arg(rig.disk(2), head));
    // The split itself undoes in ONE step, and redoes. (The tail as STORED: a typed '.' is escaped, EP-045.)
    QString tail;
    for (const SceneSegment& seg : rig.shell->sceneDocument().segments()) {
        if (seg.text.startsWith(QStringLiteral("Then more"))) { tail = seg.text; }
    }
    const QString whole = head + tail;
    check(rig.landing([&] { rig.undo(); }), QStringLiteral("Ctrl+Z over the split landed"));
    check(rig.disk(2) == whole, QStringLiteral("ONE Ctrl+Z took the split back (got: \"%1\", want \"%2\")").arg(rig.disk(2), whole));
    check(rig.landing([&] { rig.redo(); }), QStringLiteral("Ctrl+Shift+Z over the split landed"));
    check(rig.disk(2) == head, QStringLiteral("and Ctrl+Shift+Z split it again (got: \"%1\")").arg(rig.disk(2)));

    // [I-0302]: at a barrier (a scene delete still records one) nothing can be undone, but Edit ▸ Undo stays available — to
    // show the notice.
    rig.shell->history()->recordBarrier(QStringLiteral("sceneDelete"), QStringLiteral("Can't undo past deleting a scene"));
    check(!rig.shell->canUndo() && rig.shell->undoAvailable(),
          QStringLiteral("[I-0302] at the barrier Undo is available (canUndo %1, undoAvailable %2)")
              .arg(rig.shell->canUndo()).arg(rig.shell->undoAvailable()));
    notices.clear();
    rig.undo();
    check(notices == QStringList{QStringLiteral("Can't undo past deleting a scene")},
          QStringLiteral("undo stopped at the barrier with Apple's notice — and no session warning (notices: %1)")
              .arg(notices.join(QStringLiteral(" / "))));

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
    check(rig.landing([&] { QTest::keyClick(rig.editor, Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier); }),
          QStringLiteral("the chapter split landed"));
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
        check(rig.landing([&] { QTest::keyClick(rig.editor, Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier); }),
              QStringLiteral("the chapter split at the end landed"));
    }
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
