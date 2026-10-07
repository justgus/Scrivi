// presenter_smoke — EP-048 S2 (SP-166, T-0595): the Linux manuscript presenter.
//
// Drives the REAL ManuscriptEditor + SceneDocument + ManuscriptPresenter, wired as EditorShell wires them,
// with the REAL analyzer (ScriviCore via ScriviBridge::analyzeMarkdown). Offscreen.
// Each check names the AC it is evidence for. ⚠️ The live pass on the rig is still owed — a suite
// cannot say whether a writer can read it (`feedback_live_pass_finds_what_suites_cannot`).

#include <QApplication>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextLayout>

#include <cmath>
#include <cstdio>

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

struct Harness {
    SceneDocument doc;
    ManuscriptEditor editor;
    ManuscriptPresenter presenter{&ScriviBridge::analyzeMarkdown};

    // Scene 1 = `body`; scene 2 (same chapter unless `newChapter`) = `second`.
    explicit Harness(const QString& body, const QString& second = QStringLiteral("Tail"), bool newChapter = true)
    {
        QList<SceneDocument::Input> in;
        in.push_back({"s1", "c1", "Scene 1", "One", "s1", "", "", "", body});
        in.push_back({"s2", newChapter ? "c2" : "c1", "Scene 2", newChapter ? "Two" : "One", "s2", "", "", "", second});
        doc.build(in);
        editor.setDocument(doc.document());
        editor.setSceneDocument(&doc);
        QObject::connect(editor.document(), &QTextDocument::contentsChange,
                         [this](int pos, int removed, int added) { doc.applyContentsChange(pos, removed, added); });
        presenter.setSceneDocument(&doc);
        presenter.attach(doc.document());
        editor.setPresenter(&presenter);
        editor.resize(900, 600);
        editor.show();
        QApplication::processEvents();   // Qt highlights on the next event-loop turn after attach
    }
    int base(int scene = 0) const { return doc.segments().at(scene).bodyStart; }
    QString body(int scene = 0) const { return doc.bodyText(scene); }
    void select(int start, int end)
    {
        QTextCursor c = editor.textCursor();
        c.setPosition(base() + start);
        c.setPosition(base() + end, QTextCursor::KeepAnchor);
        editor.setTextCursor(c);
    }
    void caret(int at) { select(at, at); }
    int pos() const { return editor.textCursor().position() - base(); }
    void key(int k, Qt::KeyboardModifiers m = Qt::NoModifier, const QString& text = QString())
    {
        QKeyEvent e(QEvent::KeyPress, k, m, text);
        QApplication::sendEvent(&editor, &e);
        QApplication::processEvents();
    }
    // The x of a caret at body offset `at` (laid out with the presenter's formats).
    qreal x(int at)
    {
        QTextCursor c(doc.document());
        c.setPosition(base() + at);
        return editor.cursorRect(c).x();
    }
    // The highlighter's point size at body offset `at` (-1 = no size applied: drawn as stored).
    qreal drawnSize(int at)
    {
        const QTextBlock b = doc.document()->findBlock(base() + at);
        const int rel = base() + at - b.position();
        for (const auto& r : b.layout()->formats()) {
            if (rel >= r.start && rel < r.start + r.length && r.format.hasProperty(QTextFormat::FontPointSize)) {
                return r.format.fontPointSize();
            }
        }
        return -1;
    }
    // DRAWN hidden: the presenter applied its 0.01 pt format (⛔ not merely "no format").
    bool drawnHidden(int at) { const qreal s = drawnSize(at); return s > 0 && s < 0.1; }
    bool hidden(int at) { return presenter.isHidden(base() + at); }
};

void escapes()
{
    // L4: an escape backslash is hidden and takes no width; a hard break's too.
    Harness h(QStringLiteral("Mr\\. Smith said \\*no\\*."));
    check(h.hidden(2), "L4: escape backslash hidden");
    check(!h.hidden(3), "L4: the escaped mark is visible");
    check(std::abs(h.x(3) - h.x(2)) < 0.5, QStringLiteral("L4: a hidden backslash has no width (%1 vs %2)").arg(h.x(2)).arg(h.x(3)));
    check(h.body() == QStringLiteral("Mr\\. Smith said \\*no\\*."), "L3: stored text untouched");
    Harness hb(QStringLiteral("line one\\\nline two"));
    check(hb.hidden(8), "L4: hard-break backslash hidden (a line follows)");
    Harness he(QStringLiteral("end.\\\n\nnext."));
    check(!he.hidden(4), "L4: a backslash before a blank line stays literal (E1 AC6, Q1 = (a))");
}

void headings()
{
    Harness h(QStringLiteral("Body first.\n\n## Chapter heading\n\nAfter."));
    h.caret(0);
    const int line = 13;   // "## Chapter heading" starts here
    check(h.hidden(line) && h.hidden(line + 2), "L6: heading prefix hidden off its line");
    check(h.drawnSize(line + 5) > h.doc.document()->defaultFont().pointSizeF() * 1.2, "L6: heading drawn larger");
    h.caret(line + 6);
    check(h.pos() == line + 6, "caret rests inside the heading text");
    check(!h.hidden(line), "L6: prefix revealed with the caret on its line");
    check(h.drawnSize(line) > 0.5, "L6: revealed prefix drawn at body size, not 0.01 pt");
    h.caret(0);
    check(h.hidden(line) && h.drawnHidden(line), "L6: prefix hidden again when the caret leaves");
    check(h.body() == QStringLiteral("Body first.\n\n## Chapter heading\n\nAfter."), "L3: stored text untouched");
}

void emphasis()
{
    //                      0123456789012345678
    Harness h(QStringLiteral("a **bold** and *it* z"));
    h.caret(0);
    check(h.hidden(2) && h.hidden(3) && h.hidden(8) && h.hidden(9), "L6: bold markers hidden");
    check(h.drawnHidden(2) && h.drawnHidden(9), "L6: bold markers DRAWN hidden");
    check(std::abs(h.x(4) - h.x(2)) < 0.5, "L6: hidden `**` has no width");
    check(h.hidden(15) && h.hidden(18), "L6: italic markers hidden");
    // Span reveal (SP-162 amendment): at the span's first character.
    h.caret(4);
    check(h.pos() == 4, "caret at the bold span's first character");
    check(!h.hidden(2) && !h.hidden(9), "L6: span markers revealed at its first character");
    h.caret(6);
    check(h.hidden(2) && h.hidden(9), "L6: hidden again in the MIDDLE of the span (SP-162 amendment A)");
    h.caret(8);
    check(!h.hidden(8), "L6: revealed at the span's last character");
}

void caretSnap()
{
    // L5: no caret stop inside a hidden run; home sides as Apple's (§3.6).
    Harness h(QStringLiteral("Mr\\. Smith"));
    h.caret(3);   // between `\` and `.` — unreachable
    check(h.pos() == 2, QStringLiteral("L5: caret inside an escape goes home BEFORE it (got %1)").arg(h.pos()));
    h.caret(2);
    h.key(Qt::Key_Right);
    check(h.pos() == 4, QStringLiteral("L5: → from an escape's home passes the escaped mark (got %1)").arg(h.pos()));
    h.key(Qt::Key_Left);
    check(h.pos() == 2, QStringLiteral("L5: ← back across it (got %1)").arg(h.pos()));

    Harness b(QStringLiteral("x **bold** y"));
    b.caret(3);   // inside the opener
    check(b.pos() == 4, QStringLiteral("L5: inside an opener → home AFTER it (got %1)").arg(b.pos()));
    b.caret(9);   // inside the closer
    check(b.pos() == 8, QStringLiteral("L5: inside a closer → home BEFORE it (got %1)").arg(b.pos()));
    b.caret(4);
    b.key(Qt::Key_Left);
    check(b.pos() == 1, QStringLiteral("L5: ← from an opener's home passes ONE visible character (got %1)").arg(b.pos()));
    b.caret(8);
    b.key(Qt::Key_Right);
    check(b.pos() == 11, QStringLiteral("L5: → from a closer's home passes ONE visible character (got %1)").arg(b.pos()));

    // Selection: never ends inside a run; shrinking moves the end BACK (SP-161 step 1).
    Harness s(QStringLiteral("ab\\*cd"));
    s.select(0, 3);
    check(s.editor.textCursor().selectionEnd() - s.base() == 4, "L5: a growing selection takes the escape's mark with it");
    s.select(0, 5);
    s.key(Qt::Key_Left, Qt::ShiftModifier);
    s.key(Qt::Key_Left, Qt::ShiftModifier);
    check(s.editor.textCursor().selectionEnd() - s.base() <= 2,
          QStringLiteral("L5: shift-← does not stall at a hidden escape (end %1)").arg(s.editor.textCursor().selectionEnd() - s.base()));
}

void atomic()
{
    // L9 part: ⌫ / ⌦ never delete a hidden marker alone.
    Harness o(QStringLiteral("x **bold**"));
    o.caret(4);   // home after the opener
    o.key(Qt::Key_Backspace);
    check(o.body() == QStringLiteral("x**bold**"), QStringLiteral("L9: ⌫ after an opener deletes the character before it (got '%1')").arg(o.body()));
    Harness c(QStringLiteral("**bold** y"));
    c.caret(6);   // home before the closer
    c.key(Qt::Key_Delete);
    check(c.body() == QStringLiteral("**bold**y"), QStringLiteral("L9: ⌦ before a closer deletes the character after it (got '%1')").arg(c.body()));
    Harness p(QStringLiteral("## Two"));
    p.caret(3);
    p.key(Qt::Key_Backspace);
    check(p.body() == QStringLiteral("Two"), QStringLiteral("L9: ⌫ at a heading's start removes the whole prefix (got '%1')").arg(p.body()));
}

void blocksAndEdits()
{
    // A Markdown block never crosses a scene edge.
    Harness h(QStringLiteral("text *a"), QStringLiteral("b* text"), false);
    check(!h.presenter.stopAt(h.base(0) + 5).has_value(), "L6: no emphasis across a scene edge (scene 1)");
    check(!h.presenter.stopAt(h.base(1) + 1).has_value(), "L6: no emphasis across a scene edge (scene 2)");
    // A CHAPTER HEADING is not scene text: a title with Markdown in it is drawn as stored.
    Harness ch(QStringLiteral("body"), QStringLiteral("more"), true);
    QList<SceneDocument::Input> in;
    in.push_back({"s1", "c1", "Scene 1", "*Draft*", "s1", "", "", "", "body"});
    ch.doc.build(in);
    QApplication::processEvents();
    const int title = ch.doc.document()->toPlainText().indexOf(QStringLiteral("*Draft*"));
    check(title >= 0 && !ch.presenter.stopAt(title).has_value(), "L6: a chapter heading's `*` is not a marker (not scene text)");
    // An edit on line 2 re-renders line 1 of the same block.
    Harness m(QStringLiteral("*a\nb"));
    check(!m.hidden(0), "before: a lone `*` is literal");
    m.caret(4);
    m.key(Qt::Key_Asterisk, Qt::NoModifier, QStringLiteral("*"));
    QApplication::processEvents();
    check(m.body() == QStringLiteral("*a\nb\\*"), QStringLiteral("typed `*` is escaped (EP-049) — got '%1'").arg(m.body()));
    // Typing escapes, so build the closer through the document directly (as a file would hold it).
    Harness m2(QStringLiteral("*a\nb"));
    QTextCursor c(m2.doc.document());
    c.setPosition(m2.base() + 4);
    c.insertText(QStringLiteral("*"));
    QApplication::processEvents();
    check(m2.hidden(0), "L6: editing line 2 re-renders line 1 of the same block (`*` now a marker)");
    check(m2.drawnHidden(0), "L6: line 1's marker is DRAWN hidden after the edit");
    // AC7 / [I-0280]: a fence is drawn as stored — and does not crash.
    Harness f(QStringLiteral("```\nfenced *x*\n```"));
    check(!f.hidden(11), "AC7: a fenced block is drawn as stored");
    Harness t(QStringLiteral("    some **bold** text"));
    check(t.hidden(9), "AC7: an indented block is prose — its emphasis renders");
}

void cost()
{
    // A synthetic manuscript the size of dumas (~1.8 MB, ~6,000 blocks, emphasis and escapes in each).
    QString body;
    const QString para = QStringLiteral("He wrote at speed\\, for *serial* publication \\(and was **paid** by the line\\)\\; "
                                        "the pattern is real\\. D\\'Artagnan arrives in Paris with a yellow horse and no money\\.");
    // ⚠️ Each paragraph DISTINCT: the presenter caches analyses by block text, so identical paragraphs
    // would measure the cache, not the analyzer (the first run did: 3 calls for 1.8 MB).
    for (int i = 0; body.size() < 1'800'000; ++i) {
        body += QString::number(i) + QStringLiteral(" ") + para + QStringLiteral("\n\n");
    }
    QElapsedTimer t;
    t.start();
    Harness h(body);
    const qint64 open = t.elapsed();
    h.caret(h.body().size() - 10);
    t.restart();
    h.key(Qt::Key_A, Qt::NoModifier, QStringLiteral("a"));
    const qint64 keystroke = t.elapsed();
    std::printf("cost: %lld chars, %d analyzer calls; attach + first highlight %lld ms; keystroke near the end %lld ms\n",
                static_cast<long long>(body.size()), h.presenter.analyzerCalls(), static_cast<long long>(open),
                static_cast<long long>(keystroke));
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    escapes();
    headings();
    emphasis();
    caretSnap();
    atomic();
    blocksAndEdits();
    cost();
    if (failures == 0) {
        std::printf("presenter-ok\n");
        return 0;
    }
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
}
