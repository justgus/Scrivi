// format_smoke — SP-175 / T-0602 (EP-048 S3b): the Format commands and balanced edits on Linux.
//
// Drives the REAL ManuscriptEditor + SceneDocument + ManuscriptPresenter, wired as EditorShell wires them, through the REAL
// dispatch: the shared Format-menu builder's QAction shortcuts (QTest key clicks on an active window — `sendEvent` would
// skip Qt's shortcut map), keyPressEvent, and the clipboard paths. The edits are ScriviCore's (`scrivi_markdown_*`), whose
// answers SP-174 proved against Apple's; what this proves is the WIRING: the scene slice, the offsets (an emoji and a
// chapter title sit before the text), one edit per gesture, the pending pair kept out of the scene text and the dirty set.
// ⚠️ The live pass on the rig is still owed (`feedback_live_pass_finds_what_suites_cannot`).

#include <QApplication>
#include <QClipboard>
#include <QMainWindow>
#include <QMenuBar>
#include <QMimeData>
#include <QSet>
#include <QSignalSpy>
#include <QTest>
#include <QTextCursor>

#include <cstdio>

#include "FormatMenu.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptPresenter.hpp"
#include "SceneDocument.hpp"
#include "ScriviBridge.hpp"

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", qPrintable(what));
        ++failures;
    }
}

void same(const QString& actual, const QString& expected, const QString& what)
{
    check(actual == expected, QStringLiteral("%1\n   got:      %2\n   expected: %3").arg(what, actual.toHtmlEscaped(),
                                                                                       expected.toHtmlEscaped()));
}

struct Harness {
    // ⚠️ Declared in this order so they are destroyed in reverse: the presenter detaches, then the window (and the editor
    // it owns) goes, and only then the document the editor shows.
    SceneDocument doc;
    QMainWindow window;
    ManuscriptEditor* editor = new ManuscriptEditor;
    ManuscriptPresenter presenter{&ScriviBridge::analyzeMarkdown};
    QSet<int> dirty;   // as EditorShell::onContentsChange marks it

    Harness(const QString& first, const QString& second)
    {
        QList<SceneDocument::Input> in;
        in.push_back({"s1", "c1", "Scene 1", "One", "s1", "", "", "", first});
        in.push_back({"s2", "c2", "Scene 2", "Two", "s2", "", "", "", second});
        doc.build(in);
        editor->setDocument(doc.document());
        editor->setSceneDocument(&doc);
        QObject::connect(editor->document(), &QTextDocument::contentsChange, [this](int pos, int removed, int added) {
            const int idx = doc.applyContentsChange(pos, removed, added);
            if (idx >= 0 && !doc.pendingEditInProgress()) {
                dirty.insert(idx);
            }
        });
        presenter.setSceneDocument(&doc);
        presenter.attach(doc.document());
        editor->setPresenter(&presenter);
        window.setCentralWidget(editor);   // owns the editor
        addFormatMenu(window.menuBar(), &window, [this](ManuscriptEditor::Format f) { editor->applyFormat(f); });
        window.resize(900, 600);
        window.show();
        window.activateWindow();
        check(QTest::qWaitForWindowActive(&window), QStringLiteral("the window became active (shortcuts need it)"));
        editor->setFocus();
        QApplication::processEvents();
    }
    int base(int scene) const { return doc.segments().at(scene).bodyStart; }
    QString body(int scene) const { return doc.bodyText(scene); }
    void select(int scene, int start, int end)
    {
        QTextCursor c = editor->textCursor();
        c.setPosition(base(scene) + start);
        c.setPosition(base(scene) + end, QTextCursor::KeepAnchor);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void caret(int scene, int at) { select(scene, at, at); }
    std::pair<int, int> selection(int scene) const
    {
        const QTextCursor c = editor->textCursor();
        return {c.selectionStart() - base(scene), c.selectionEnd() - base(scene)};
    }
    void key(Qt::Key k, Qt::KeyboardModifiers m = Qt::NoModifier)
    {
        QTest::keyClick(editor, k, m);
        QApplication::processEvents();
    }
    // A Ctrl+letter as X11 delivers it: the CONTROL character as its text. ⚠️ `QTest::keyClick` sends the plain letter
    // ("c" for Ctrl+C), which the editor rightly takes as typing — measured: it typed "c" over the selection.
    void control(Qt::Key k)
    {
        const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
        QTest::sendKeyEvent(QTest::Press, editor, k, text, Qt::ControlModifier);
        QTest::sendKeyEvent(QTest::Release, editor, k, text, Qt::ControlModifier);
        QApplication::processEvents();
    }
    void type(const QString& text)
    {
        QTest::keyClicks(editor, text);
        QApplication::processEvents();
    }
};

// The core's own answer, applied — what the editor must have produced.
QString applied(const QString& text, const MarkdownEdit& e)
{
    QString out = text;
    out.replace(e.range.start, e.range.end - e.range.start, e.replacement);
    return out;
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // ── Bold / Italic on a selection, in a scene AFTER a chapter title and an emoji (offsets are UTF-16) ──
    {
        const QString s2 = QStringLiteral("👋 some words here.");
        Harness h(QStringLiteral("Alpha **bold** one."), s2);
        h.select(1, 8, 13);   // "words" (👋 is two units)
        h.key(Qt::Key_B, Qt::ControlModifier);
        const auto core = ScriviBridge::markdownEmphasisEdit(s2, 8, 13, MarkdownAnalysis::kBold);
        check(core.has_value(), QStringLiteral("core: bold edit"));
        same(h.body(1), applied(s2, *core), QStringLiteral("Ctrl+B: the core's edit, at the scene's offsets"));
        same(h.body(1), QStringLiteral("👋 some **words** here."), QStringLiteral("Ctrl+B bolds the selection"));
        check(h.selection(1) == std::pair{core->selection.start, core->selection.end},
              QStringLiteral("Ctrl+B: the selection the core returns (%1,%2)").arg(h.selection(1).first).arg(h.selection(1).second));
        check(h.dirty.contains(1) && !h.dirty.contains(0), QStringLiteral("Ctrl+B marks scene 2 dirty, and only it"));
        h.key(Qt::Key_B, Qt::ControlModifier);
        same(h.body(1), s2, QStringLiteral("Ctrl+B again toggles it off"));
        h.select(1, 8, 13);
        h.key(Qt::Key_I, Qt::ControlModifier);
        same(h.body(1), QStringLiteral("👋 some *words* here."), QStringLiteral("Ctrl+I writes `*`"));
        same(h.body(0), QStringLiteral("Alpha **bold** one."), QStringLiteral("scene 1 untouched"));
    }

    // ── Caret IN a word → the word; the caret stays ──
    {
        Harness h(QStringLiteral("x"), QStringLiteral("Some words here"));
        h.caret(1, 7);
        h.key(Qt::Key_B, Qt::ControlModifier);
        same(h.body(1), QStringLiteral("Some **words** here"), QStringLiteral("Ctrl+B with the caret in a word bolds the word"));
        check(h.selection(1) == std::pair{9, 9}, QStringLiteral("the caret stays on the same character (9)"));
    }

    // ── The PENDING pair: between words; typed into → real; left → gone; space goes before; ⌫ removes ──
    {
        Harness h(QStringLiteral("x"), QStringLiteral("Some  words"));
        h.dirty.clear();
        h.caret(1, 5);   // between the two spaces
        h.key(Qt::Key_B, Qt::ControlModifier);
        check(h.doc.pending().has_value(), QStringLiteral("Ctrl+B between words makes a pending pair"));
        same(h.doc.document()->toPlainText().mid(h.base(1), 15), QStringLiteral("Some **** words"),
             QStringLiteral("the pair is in the document"));
        same(h.body(1), QStringLiteral("Some  words"), QStringLiteral("…but NOT in the scene text (bodyText)"));
        check(h.dirty.isEmpty(), QStringLiteral("…and the scene is NOT dirty"));
        check(h.selection(1) == std::pair{7, 7}, QStringLiteral("the caret sits between the hints"));
        h.type(QStringLiteral("x"));
        check(!h.doc.pending().has_value(), QStringLiteral("typing into the pair makes it real"));
        same(h.body(1), QStringLiteral("Some **x** words"), QStringLiteral("…and it is scene text now"));
        check(h.dirty.contains(1), QStringLiteral("…and the scene is dirty"));
    }
    {
        Harness h(QStringLiteral("x"), QStringLiteral("Some  words"));
        h.dirty.clear();
        h.caret(1, 5);
        h.key(Qt::Key_I, Qt::ControlModifier);
        h.key(Qt::Key_Right);
        check(!h.doc.pending().has_value(), QStringLiteral("moving the caret away removes the pair"));
        same(h.doc.document()->toPlainText().mid(h.base(1), 11), QStringLiteral("Some  words"),
             QStringLiteral("…from the document too"));
        check(h.dirty.isEmpty(), QStringLiteral("…and nothing was ever dirty"));
        h.caret(1, 5);
        h.key(Qt::Key_B, Qt::ControlModifier);
        h.type(QStringLiteral(" "));
        same(h.body(1), QStringLiteral("Some   words"), QStringLiteral("a space typed into the pair goes BEFORE it"));
        check(h.doc.pending().has_value(), QStringLiteral("…and the pair stays pending"));
        h.key(Qt::Key_Backspace);
        check(!h.doc.pending().has_value(), QStringLiteral("⌫ inside the pair takes it away"));
        same(h.body(1), QStringLiteral("Some   words"), QStringLiteral("…and deletes nothing else"));
        h.key(Qt::Key_B, Qt::ControlModifier);
        h.key(Qt::Key_B, Qt::ControlModifier);
        check(!h.doc.pending().has_value(), QStringLiteral("Ctrl+B again on the pair takes it away"));
    }

    // ── Headings, body, lists, Return in a list ──
    {
        Harness h(QStringLiteral("x"), QStringLiteral("Title line\n\nalpha\n\nbeta"));
        h.caret(1, 3);
        h.key(Qt::Key_2, Qt::ControlModifier | Qt::AltModifier);
        same(h.body(1), QStringLiteral("## Title line\n\nalpha\n\nbeta"), QStringLiteral("Ctrl+Alt+2: heading 2"));
        h.key(Qt::Key_0, Qt::ControlModifier | Qt::AltModifier);
        same(h.body(1), QStringLiteral("Title line\n\nalpha\n\nbeta"), QStringLiteral("Ctrl+Alt+0: body"));
        h.select(1, 12, 23);
        h.key(Qt::Key_L, Qt::ControlModifier | Qt::AltModifier);
        same(h.body(1), QStringLiteral("Title line\n\n- alpha\n\n- beta"), QStringLiteral("Ctrl+Alt+L: bulleted"));
        h.select(1, 12, h.body(1).size());
        h.key(Qt::Key_N, Qt::ControlModifier | Qt::AltModifier);
        same(h.body(1), QStringLiteral("Title line\n\n1. alpha\n\n2. beta"), QStringLiteral("Ctrl+Alt+N: numbered"));
        h.caret(1, 20);   // after "1. alpha"
        h.key(Qt::Key_Return);
        same(h.body(1), QStringLiteral("Title line\n\n1. alpha\n\n2. \n\n3. beta"),
             QStringLiteral("Return in an item: the next number, later items renumbered"));
        h.key(Qt::Key_Return);
        same(h.body(1), QStringLiteral("Title line\n\n1. alpha\n\n\n\n2. beta"), QStringLiteral("Return on the empty item ends it"));
    }

    // ── Balanced cut, copy and own paste (EP-046 AC12) ──
    {
        Harness h(QStringLiteral("x"), QStringLiteral("**bold** and"));
        h.select(1, 4, 11);
        h.control(Qt::Key_X);
        same(h.body(1), QStringLiteral("**bo**d"), QStringLiteral("a cut from inside bold closes the span"));
        const QMimeData* m = QApplication::clipboard()->mimeData();
        same(m ? m->text() : QString(), QStringLiteral("ld an"), QStringLiteral("other apps get the presented text"));
        same(m ? QString::fromUtf8(m->data(QStringLiteral("application/x-scrivi-manuscript-source"))) : QString(),
             QStringLiteral("**ld** an"), QStringLiteral("Scrivi's own clipboard gets the BALANCED source"));
    }
    {
        Harness h(QStringLiteral("x"), QStringLiteral("**abcd** xy"));
        h.select(1, 9, 11);   // "xy" (plain)
        h.control(Qt::Key_C);
        h.caret(1, 4);        // between "ab" and "cd" — inside the bold
        h.control(Qt::Key_V);
        const auto core = ScriviBridge::markdownBalancedEdit(QStringLiteral("**abcd** xy"), 4, 4, QStringLiteral("xy"), true);
        check(core.has_value(), QStringLiteral("core: own paste into bold is rewritten"));
        same(h.body(1), applied(QStringLiteral("**abcd** xy"), *core),
             QStringLiteral("Scrivi's own paste keeps ITS formatting (plain xy stays plain inside bold)"));
    }
    {
        Harness h(QStringLiteral("x"), QStringLiteral("a **b** c"));
        h.select(1, 4, 5);
        h.key(Qt::Key_Backspace);
        same(h.body(1), QStringLiteral("a  c"), QStringLiteral("⌫ of a span's only character removes the span"));
    }

    // ── Across scenes: a command is refused; a cut is the SHELL's structural cut ([SP-188]) — the editor asks for it and
    //    changes nothing itself (its balanced join is the core's: `RangeOperationCApiTests` [sp188], and
    //    `structural_baseline_smoke` through a real shell) ──
    {
        Harness h(QStringLiteral("Alpha **bold one**"), QStringLiteral("**two** words"));
        const QString doc0 = h.doc.document()->toPlainText();
        QTextCursor c = h.editor->textCursor();
        c.setPosition(h.base(0) + 10);
        c.setPosition(h.base(1) + 4, QTextCursor::KeepAnchor);
        h.editor->setTextCursor(c);
        h.key(Qt::Key_B, Qt::ControlModifier);
        same(h.doc.document()->toPlainText(), doc0, QStringLiteral("Ctrl+B across a chapter title changes nothing"));
        h.editor->setTextCursor(c);
        QSignalSpy asked(h.editor, &ManuscriptEditor::crossSceneCutRequested);
        h.control(Qt::Key_X);
        check(asked.size() == 1 && asked.at(0).at(0).toInt() == 1,
              QStringLiteral("cross-scene Ctrl+X asks the shell for the structural cut, keeping the clipboard's Range"));
        same(h.doc.document()->toPlainText(), doc0, QStringLiteral("…and the editor changes nothing itself"));
    }

    // ── Typing over a selection that crosses a span edge (Q3) ──
    {
        Harness h(QStringLiteral("x"), QStringLiteral("**bold** and"));
        h.select(1, 4, 11);
        h.type(QStringLiteral("X"));
        same(h.body(1), QStringLiteral("**boX**d"), QStringLiteral("typing over a span edge keeps it balanced"));
    }

    if (failures == 0) {
        std::printf("format-ok\n");
        return 0;
    }
    return 1;
}
