// buffers_smoke.cpp — EP-026 S5 (SP-179 / T-0606): the copy buffers 1–9 on Linux, end to end.
//
// REAL project windows opened through `AppEnvironment` (as the app opens them), keys through `QTest` into the real
// editor (the real dispatch, ShortcutOverride included — `feedback_test_through_the_real_dispatch`). Every claim about
// text is read back FROM DISK; every claim about a slot through `scrivi_buffers_get` — the Mac's view of the same file.
//
// ✅ What it proves (SP-179 AC1–AC7):
//   • Ctrl+N copies the BALANCED source into slot N — no undo step, the system clipboard untouched;
//   • Ctrl+Shift+N (arriving as the SHIFTED key, `!`, as X11 delivers it) pastes it — ONE undo step, stored text verbatim;
//     the paste keeps ITS own formatting ([I-0289]); an empty slot pastes nothing;
//   • Alt+N cuts into slot N — ONE undo step, its history node tagged with the slot; across scenes, one grouped step;
//   • (a slot holding scenes — the Mac's or Linux's — is `structured_paste_smoke`'s, EP-026 S4);
//   • View ▸ Show Buffers shows ONE palette that follows the FRONT project window; its button follows Ctrl+Shift / Alt;
//     a click acts with the modifiers held; clear empties a slot; closing it unchecks the item in EVERY window;
//   • the slots survive the window closing and reopening.
// ⚠️ NOT covered (live pass only): GNOME and RDP delivering the chords, where the compositor puts the palette.
//
//   argv[1] = a scratch directory for the two projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "BufferService.hpp"
#include "BuffersPalette.hpp"
#include "EditorShell.hpp"
#include "HistoryCapture.hpp"
#include "ManuscriptEditor.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>

#include <cstdio>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

// Every modal notice is dismissed and its text recorded.
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

void settle() { QTest::qWait(60); }

struct Project {
    QString dir;
    QString id;
    QStringList sceneIDs;
};

// A project whose scenes are seeded on disk BEFORE the first open (each scene's floor, not undoable edits).
Project makeProject(ScriviBridge& bridge, const QString& appSupport, const QString& dir, const QStringList& seeds)
{
    Project p;
    p.dir = dir;
    p.id = bridge.createProject(dir, appSupport, QStringLiteral("Buffers"), QStringLiteral("buffers"))
               .value(QStringLiteral("projectID")).toString();
    QVariantList scenes = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList();
    const QString chapterID = scenes.first().toMap().value(QStringLiteral("chapterID")).toString();
    for (int i = 1; i < seeds.size(); ++i) {
        bridge.createScene(dir, appSupport, p.id, chapterID, scenes.last().toMap().value(QStringLiteral("sceneID")).toString());
        scenes = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList();
    }
    for (int i = 0; i < seeds.size(); ++i) {
        const QVariantMap s = scenes.at(i).toMap();
        p.sceneIDs.append(s.value(QStringLiteral("sceneID")).toString());
        bridge.saveScene(p.id, dir, appSupport, p.sceneIDs.last(), s.value(QStringLiteral("metadataPath")).toString(),
                         s.value(QStringLiteral("contentPath")).toString(), seeds.at(i), 0, 0, 0.0);
    }
    bridge.closeProject(dir);
    return p;
}

// Opens `p` in its own window through the app's own path, and waits for the load to settle.
ScriviWindow* openWindow(AppEnvironment& env, const Project& p)
{
    ScriviWindow* w = env.openProjectWindow(p.dir, QStringLiteral("Buffers"), {}, p.id);
    for (int i = 0; i < 3000 && env.windows().window(p.id) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(p.id) == w, QStringLiteral("project window loaded: %1").arg(p.dir));
    return w;
}

void activate(ScriviWindow* w)
{
    w->raiseToFront();
    check(QTest::qWaitForWindowActive(w), QStringLiteral("the window became active"));
    settle();
}

ManuscriptEditor* editorOf(ScriviWindow* w) { return w->editorShell()->findChild<ManuscriptEditor*>(); }

QString disk(ScriviBridge& bridge, ScriviWindow* w, const Project& p, int i)
{
    w->editorShell()->saveDirtyScenes();
    return bridge.readSceneTexts(p.dir, {p.sceneIDs.at(i)}).value(p.sceneIDs.at(i));
}

QString slot(const Project& p, const QString& id)
{
    return ScriviBridge::buffersGet(p.dir, id).value(QStringLiteral("text")).toString();
}

// The document range of `text`'s first occurrence (stored form — Linux shows the backslashes).
QPair<int, int> found(ManuscriptEditor* e, const QString& text)
{
    const QTextCursor c = e->document()->find(text);
    return {c.selectionStart(), c.selectionEnd()};
}
void select(ManuscriptEditor* e, int a, int b)
{
    QTextCursor c = e->textCursor();
    c.setPosition(a);
    c.setPosition(b, QTextCursor::KeepAnchor);
    e->setTextCursor(c);
    QApplication::processEvents();
}
void caret(ManuscriptEditor* e, int pos) { select(e, pos, pos); }

// Chords as X11 delivers them: Ctrl+digit and Alt+digit carry the digit; Ctrl+Shift+digit the SHIFTED key (`!`…).
void copyChord(ManuscriptEditor* e, int n) { QTest::keyClick(e, Qt::Key(Qt::Key_0 + n), Qt::ControlModifier); settle(); }
void cutChord(ManuscriptEditor* e, int n) { QTest::keyClick(e, Qt::Key(Qt::Key_0 + n), Qt::AltModifier); settle(); }
void pasteChord(ManuscriptEditor* e, int n)
{
    static const Qt::Key shifted[] = {Qt::Key_Exclam, Qt::Key_At, Qt::Key_NumberSign, Qt::Key_Dollar, Qt::Key_Percent,
                                      Qt::Key_AsciiCircum, Qt::Key_Ampersand, Qt::Key_Asterisk, Qt::Key_ParenLeft};
    static const char* text = "!@#$%^&*(";
    const Qt::Key k = shifted[n - 1];
    const QString t(QChar::fromLatin1(text[n - 1]));
    QTest::sendKeyEvent(QTest::Press, e, k, t, Qt::ControlModifier | Qt::ShiftModifier);
    QTest::sendKeyEvent(QTest::Release, e, k, t, Qt::ControlModifier | Qt::ShiftModifier);
    settle();
}
void control(ManuscriptEditor* e, Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
{
    const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
    QTest::sendKeyEvent(QTest::Press, e, k, text, Qt::ControlModifier | extra);
    QTest::sendKeyEvent(QTest::Release, e, k, text, Qt::ControlModifier | extra);
    settle();
}
void undo(ManuscriptEditor* e) { control(e, Qt::Key_Z); }
void redo(ManuscriptEditor* e) { control(e, Qt::Key_Z, Qt::ShiftModifier); }

// What pasting `source` (Scrivi's own copy) at scene-local `pos` must store — the core's balanced edit.
QString pasted(const QString& scene, int pos, const QString& source, bool keepsOwnFormatting = true)
{
    const auto edit = ScriviBridge::markdownBalancedEdit(scene, pos, pos, source, keepsOwnFormatting);
    QString out = scene;
    if (!edit) {
        return out.insert(pos, source);
    }
    return out.replace(edit->range.start, edit->range.end - edit->range.start, edit->replacement);
}

QAction* menuAction(ScriviWindow* w, const QString& text)
{
    for (QAction* a : w->menuBar()->findChildren<QAction*>()) {
        if (a->text() == text) {
            return a;
        }
    }
    return nullptr;
}

QString historyLog(const Project& p)
{
    QFile f(p.dir + QStringLiteral("/history/log-000001.jsonl"));
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);   // a crash must not swallow the FAIL lines
    if (argc < 2) {
        std::fprintf(stderr, "usage: buffers_smoke <scratchDir>\n");
        return 2;
    }
    armNoticeCatcher(&app);

    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const QString s0 = QStringLiteral("x **bold text** y");
    const QString s1 = QStringLiteral("Bravo two");
    const Project A = makeProject(bridge, appSupport, scratch + QStringLiteral("/a.scrivi"), {s0, s1});
    const Project B = makeProject(bridge, appSupport, scratch + QStringLiteral("/b.scrivi"), {QStringLiteral("Other")});

    AppEnvironment env(appSupport);
    ScriviWindow* wa = openWindow(env, A);
    activate(wa);
    ManuscriptEditor* ea = editorOf(wa);
    EditorShell* sa = wa->editorShell();
    check(ea != nullptr && sa->buffers() != nullptr, QStringLiteral("the editor and its buffers exist"));
    check(env.frontProjectWindow() == wa, QStringLiteral("the active project window is the front project"));
    ea->setFocus();

    // ── 1. Ctrl+1 copies the BALANCED source — no undo step, the clipboard untouched (AC1) ───────────────────────────
    QGuiApplication::clipboard()->setText(QStringLiteral("sentinel"));
    check(QGuiApplication::clipboard()->text() == QStringLiteral("sentinel"), QStringLiteral("precondition: the clipboard works"));
    caret(ea, found(ea, QStringLiteral("bold")).first);
    copyChord(ea, 9);
    check(!ScriviBridge::buffersGet(A.dir, QStringLiteral("9")).value(QStringLiteral("present")).toBool(),
          QStringLiteral("Ctrl+9 with nothing selected stored nothing"));
    {
        const auto r = found(ea, QStringLiteral("old te"));
        select(ea, r.first, r.second);
    }
    copyChord(ea, 1);
    check(slot(A, QStringLiteral("1")) == QStringLiteral("**old te**"),
          QStringLiteral("Ctrl+1 stored the balanced copy (got \"%1\")").arg(slot(A, QStringLiteral("1"))));
    check(!sa->canUndo(), QStringLiteral("a copy into a slot is NOT an undo step"));
    check(QGuiApplication::clipboard()->text() == QStringLiteral("sentinel"), QStringLiteral("the system clipboard is untouched"));
    check(disk(bridge, wa, A, 0) == s0, QStringLiteral("a copy changes no text"));

    // ── 2. Ctrl+Shift+1 (arriving as `!`) pastes — ONE undo step; redo re-applies (AC2) ─────────────────────────────────
    caret(ea, found(ea, QStringLiteral("text** y")).second);
    pasteChord(ea, 1);
    const QString afterPaste = pasted(s0, s0.size(), QStringLiteral("**old te**"));
    check(disk(bridge, wa, A, 0) == afterPaste,
          QStringLiteral("Ctrl+Shift+1 pasted slot 1 (got \"%1\", wanted \"%2\")").arg(disk(bridge, wa, A, 0), afterPaste));
    undo(ea);
    check(disk(bridge, wa, A, 0) == s0, QStringLiteral("ONE Ctrl+Z took the whole paste back"));
    redo(ea);
    check(disk(bridge, wa, A, 0) == afterPaste, QStringLiteral("Ctrl+Shift+Z put it back"));
    undo(ea);

    // ── 3. The paste keeps ITS formatting: plain text pasted inside bold stays plain ([I-0289], [SP-163]) ────────────────
    {
        const auto r = found(ea, QStringLiteral("Bravo"));
        select(ea, r.first, r.second);
    }
    copyChord(ea, 2);
    check(slot(A, QStringLiteral("2")) == QStringLiteral("Bravo"), QStringLiteral("Ctrl+2 stored plain text"));
    {
        const int insideBold = found(ea, QStringLiteral("bold")).first + 2;   // between "bo" and "ld"
        caret(ea, insideBold);
        pasteChord(ea, 2);
        const int local = s0.indexOf(QStringLiteral("bold")) + 2;
        const QString own = pasted(s0, local, QStringLiteral("Bravo"), true);
        check(own != pasted(s0, local, QStringLiteral("Bravo"), false), QStringLiteral("precondition: the two pastes differ"));
        check(disk(bridge, wa, A, 0) == own,
              QStringLiteral("a slot pasted inside bold keeps its own (plain) formatting (got \"%1\")").arg(disk(bridge, wa, A, 0)));
        undo(ea);
        check(disk(bridge, wa, A, 0) == s0, QStringLiteral("…and undoes in one step"));
    }

    // ── 4. Stored text round-trips VERBATIM — an escape is never escaped twice (EP-045 AC4) ──────────────────────────────
    {
        caret(ea, found(ea, QStringLiteral("Bravo two")).second);
        QTest::keyClicks(ea, QStringLiteral(" a.b"));
        settle();
        const QString typed = disk(bridge, wa, A, 1);
        check(typed == QStringLiteral("Bravo two a\\.b"), QStringLiteral("typed text is stored escaped (got \"%1\")").arg(typed));
        const auto r = found(ea, QStringLiteral("a\\.b"));
        select(ea, r.first, r.second);
        copyChord(ea, 3);
        check(slot(A, QStringLiteral("3")) == QStringLiteral("a\\.b"), QStringLiteral("the slot holds the stored form"));
        caret(ea, r.second);
        pasteChord(ea, 3);
        check(disk(bridge, wa, A, 1) == QStringLiteral("Bravo two a\\.ba\\.b"),
              QStringLiteral("pasted verbatim (got \"%1\")").arg(disk(bridge, wa, A, 1)));
        undo(ea);
        check(disk(bridge, wa, A, 1) == typed, QStringLiteral("the paste undoes in one step"));
    }

    // ── 5. Alt+4 cuts into slot 4 — ONE undo step, tagged with the slot; the slot keeps the text (AC3) ─────────────────
    {
        const QString before = disk(bridge, wa, A, 1);
        const auto r = found(ea, QStringLiteral("Bravo "));
        select(ea, r.first, r.second);
        cutChord(ea, 4);
        check(slot(A, QStringLiteral("4")) == QStringLiteral("Bravo "), QStringLiteral("Alt+4 stored the cut text"));
        check(disk(bridge, wa, A, 1) == before.mid(6), QStringLiteral("Alt+4 removed it (got \"%1\")").arg(disk(bridge, wa, A, 1)));
        sa->flushHistory();
        check(historyLog(A).contains(QStringLiteral("\"bufferID\":\"4\"")), QStringLiteral("the cut's history node carries slot 4"));
        undo(ea);
        check(disk(bridge, wa, A, 1) == before, QStringLiteral("ONE Ctrl+Z restored the cut"));
        check(slot(A, QStringLiteral("4")) == QStringLiteral("Bravo "), QStringLiteral("…and the slot keeps the text"));
    }

    // ── 6. An empty slot pastes nothing ───────────────────────────────────────────────────────────────────────────────
    {
        const QString before = disk(bridge, wa, A, 1);
        caret(ea, found(ea, QStringLiteral("two")).second);
        pasteChord(ea, 5);
        check(disk(bridge, wa, A, 1) == before, QStringLiteral("Ctrl+Shift+5 on an empty slot changed nothing"));
    }

    // ── 7. Across a scene break: Alt+6 cuts from each scene — ONE grouped step ([SP-179] Q1: flat text) ──────────────────
    {
        const QString d0 = disk(bridge, wa, A, 0);
        const QString d1 = disk(bridge, wa, A, 1);
        const int a = found(ea, QStringLiteral(" y")).first;
        const int b = found(ea, QStringLiteral("Bravo")).second;
        select(ea, a, b);
        cutChord(ea, 6);
        check(slot(A, QStringLiteral("6")).startsWith(QStringLiteral(" y")) && slot(A, QStringLiteral("6")).endsWith(QStringLiteral("Bravo")),
              QStringLiteral("the slot holds the flat text across the break (got \"%1\")").arg(slot(A, QStringLiteral("6"))));
        check(disk(bridge, wa, A, 0) == QStringLiteral("x **bold text**") && !disk(bridge, wa, A, 1).startsWith(QStringLiteral("Bravo")),
              QStringLiteral("each scene lost its part (\"%1\" / \"%2\")").arg(disk(bridge, wa, A, 0), disk(bridge, wa, A, 1)));
        undo(ea);
        check(disk(bridge, wa, A, 0) == d0 && disk(bridge, wa, A, 1) == d1, QStringLiteral("ONE Ctrl+Z restored both scenes"));
    }

    // ── 8. (A slot the Mac filled with SCENES rebuilds them since EP-026 S4 — `structured_paste_smoke` §5. [SP-179] Q1's
    //       plain-text paste and its notice are gone.) ─────────────────────────────────────────────────────────────────

    // ── 9. View ▸ Show Buffers: ONE palette, following the FRONT project (AC5) ────────────────────────────────────────────
    ScriviWindow* wb = openWindow(env, B);
    activate(wb);
    check(env.frontProjectWindow() == wb, QStringLiteral("B became the front project"));
    QAction* showA = menuAction(wa, QStringLiteral("Show Buffers"));
    QAction* showB = menuAction(wb, QStringLiteral("Show Buffers"));
    check(showA != nullptr && showB != nullptr && showA->shortcut() == QKeySequence(QStringLiteral("Ctrl+Alt+B")),
          QStringLiteral("View ▸ Show Buffers exists, on Ctrl+Alt+B"));
    if (showB != nullptr) {
        showB->trigger();
        settle();
    }
    BuffersPalette* palette = env.buffersPalette();
    check(palette != nullptr && palette->isVisible(), QStringLiteral("the palette is shown"));
    check(showA != nullptr && showA->isChecked(), QStringLiteral("…and EVERY window's item is checked"));
    if (palette != nullptr) {
        auto preview = [palette](int n) {
            return palette->findChild<QLabel*>(QStringLiteral("bufferPreview%1").arg(n))->text();
        };
        check(palette->service() == wb->bufferService() && preview(1) == QStringLiteral("empty"),
              QStringLiteral("it shows B's (empty) slots"));
        activate(wa);
        check(env.frontProjectWindow() == wa && palette->service() == wa->bufferService(),
              QStringLiteral("activating A re-points it at A"));
        check(preview(1) == QStringLiteral("**old te**") && preview(4) == QStringLiteral("Bravo"),
              QStringLiteral("…and shows A's slots (\"%1\", \"%2\")").arg(preview(1), preview(4)));
        check(!palette->isActiveWindow(), QStringLiteral("the palette never took the focus"));

        // ── 10. The button follows the modifiers, exactly as the chords ([I-0291]) ──
        auto mod = [ea](QTest::KeyAction act, Qt::Key k, Qt::KeyboardModifiers held) {
            QTest::sendKeyEvent(act, ea, k, QString(), held);
            QApplication::processEvents();
        };
        mod(QTest::Press, Qt::Key_Control, Qt::NoModifier);
        check(palette->shownAction() == BuffersPalette::Action::Copy, QStringLiteral("Ctrl alone: copy"));
        mod(QTest::Press, Qt::Key_Shift, Qt::ControlModifier);
        check(palette->shownAction() == BuffersPalette::Action::Paste, QStringLiteral("Ctrl+Shift: paste"));
        mod(QTest::Release, Qt::Key_Shift, Qt::ControlModifier | Qt::ShiftModifier);
        mod(QTest::Release, Qt::Key_Control, Qt::ControlModifier);
        check(palette->shownAction() == BuffersPalette::Action::Copy, QStringLiteral("released: copy"));
        mod(QTest::Press, Qt::Key_Alt, Qt::NoModifier);
        check(palette->shownAction() == BuffersPalette::Action::Cut, QStringLiteral("Alt: cut"));
        mod(QTest::Release, Qt::Key_Alt, Qt::AltModifier);

        // ── 11. A click with Ctrl+Shift held pastes into the FRONT project ──
        const QString before = disk(bridge, wa, A, 1);
        const int end = found(ea, QStringLiteral("two")).second;
        caret(ea, end);
        QTest::mouseClick(palette->findChild<QToolButton*>(QStringLiteral("bufferAction4")), Qt::LeftButton,
                          Qt::ControlModifier | Qt::ShiftModifier);
        settle();
        check(disk(bridge, wa, A, 1) == pasted(before, before.indexOf(QStringLiteral("two")) + 3, QStringLiteral("Bravo ")),
              QStringLiteral("the palette's Ctrl+Shift click pasted slot 4 into A (got \"%1\")").arg(disk(bridge, wa, A, 1)));
        check(disk(bridge, wb, B, 0) == QStringLiteral("Other"), QStringLiteral("…and nothing into B"));
        undo(ea);

        // ── 12. Clear empties a slot; closing the palette unchecks every window ──
        QTest::mouseClick(palette->findChild<QToolButton*>(QStringLiteral("bufferClear2")), Qt::LeftButton);
        settle();
        check(!ScriviBridge::buffersGet(A.dir, QStringLiteral("2")).value(QStringLiteral("present")).toBool()
                  && preview(2) == QStringLiteral("empty"), QStringLiteral("✕ cleared slot 2"));
        palette->close();
        settle();
        check(!env.buffersPaletteVisible() && !showA->isChecked() && !showB->isChecked(),
              QStringLiteral("closing the palette unchecked View ▸ Show Buffers in every window"));
    }

    // ── 13. The slots outlive the window (AC4) ─────────────────────────────────────────────────────────────────────────
    wa->close();
    settle();
    ScriviWindow* wa2 = openWindow(env, A);
    check(wa2->bufferService() != nullptr && wa2->bufferService()->text(QStringLiteral("4")) == QStringLiteral("Bravo ")
              && wa2->bufferService()->text(QStringLiteral("1")) == QStringLiteral("**old te**"),
          QStringLiteral("a reopened project shows its slots"));

    env.quitApplication();
    QApplication::processEvents();
    if (failures == 0) {
        std::printf("buffers-ok\n");
        return 0;
    }
    return 1;
}
