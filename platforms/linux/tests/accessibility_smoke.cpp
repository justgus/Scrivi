// accessibility_smoke — EP-050 S4 ([SP-203], T-0659): what Orca is given for the Linux manuscript.
//
// Drives the REAL ManuscriptEditor + SceneDocument + ManuscriptPresenter, wired as EditorShell wires them, and reads them
// through `QAccessibleTextInterface` — the interface Qt's AT-SPI bridge answers Orca from ([SP-202]'s measurement route).
// ✅ The page at rest (Apple A1): no escapes, no `##`, no emphasis markers; chapter titles; a divider as "Scene break", and the
// one closing a chapter as "End of chapter" ([SP-184] Q4, Apple's words), before the next chapter's title.
// ✅ Every member agrees with that text (AC1) · the caret both ways (AC2) · cost. ⚠️ Qt's events need a platform bridge
// (offscreen has none: Qt sends nothing) — accessibility_atspi.sh checks them over AT-SPI.

#include <QAccessible>
#include <QAccessibleInterface>
#include <QApplication>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QTest>
#include <QTextCursor>

#include <cstdio>

#include "ManuscriptAccessibility.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptFind.hpp"
#include "ManuscriptPresenter.hpp"
#include "SceneDocument.hpp"
#include "ScriviBridge.hpp"

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what.toUtf8().constData());
        ++failures;
    }
}

void same(const QString& actual, const QString& expected, const QString& what)
{
    check(actual == expected, QStringLiteral("%1\n   got:      «%2»\n   expected: «%3»").arg(what, actual, expected));
}

struct Harness {
    SceneDocument doc;
    QMainWindow window;
    ManuscriptEditor* editor = new ManuscriptEditor;
    ManuscriptPresenter presenter{&ScriviBridge::analyzeMarkdown};
    bool applyingDelta = false;

    explicit Harness(const QList<SceneDocument::Input>& in)
    {
        doc.build(in);
        editor->setDocument(doc.document());
        editor->setSceneDocument(&doc);
        // ⚠️ Not while a delta is applied: its document edits are the core's, already in the scene list — the shell holds
        // `loading_` for the same reason (`EditorShell::applyDelta`). Re-reported, they corrupt the list (CI red since I-0346).
        QObject::connect(editor->document(), &QTextDocument::contentsChange, [this](int pos, int removed, int added) {
            if (!applyingDelta) {
                doc.applyContentsChange(pos, removed, added);
            }
        });
        presenter.setSceneDocument(&doc);
        presenter.attach(doc.document());
        editor->setPresenter(&presenter);
        window.setCentralWidget(editor);
        window.resize(900, 600);
        window.show();
        window.activateWindow();   // key input (accessibility_atspi types into its own X server) and Orca's focus
        QTest::qWaitForWindowExposed(&window);
        editor->setFocus();
        QApplication::processEvents();
    }
    QAccessibleTextInterface* text() const { return QAccessible::queryAccessibleInterface(editor)->textInterface(); }
    QString page() const { return text()->text(0, text()->characterCount()); }
    int stored(int scene, const QString& body, const QString& at) const
    {
        return doc.segments().at(scene).bodyStart + int(body.indexOf(at));
    }
    void caret(int pos)
    {
        QTextCursor c = editor->textCursor();
        c.setPosition(pos);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
};

SceneDocument::Input scene(const QString& id, const QString& chapter, const QString& chapterTitle, const QString& body)
{
    return {id, chapter, id, chapterTitle, id, QString(), QString(), QString(), body};
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    ManuscriptAccessibility::install();
    QAccessible::setActive(true);

    const QString s1 = QStringLiteral("Mr\\. Smith said **bold** and *soft* here.\n\n## The claim stated plainly\n\n- a list item");
    const QString s2 = QStringLiteral("Second scene.");
    const QString s3 = QStringLiteral("The third scene, 1\\. not a list.");
    Harness h({scene("s1", "c1", "Chapter One", s1), scene("s2", "c1", "Chapter One", s2),
               scene("s3", "c2", "Chapter Two", s3)});
    QAccessibleTextInterface* t = h.text();
    check(t != nullptr, QStringLiteral("the editor has a text interface"));
    if (t == nullptr) {
        return 1;
    }
    // accessibility_atspi.sh: the same page read over AT-SPI, the way Orca reads it, while this holds.
    if (const int hold = qEnvironmentVariableIntValue("SCRIVI_A11Y_HOLD_MS"); hold > 0) {
        QTest::qWait(hold);
    }

    // ── A1: the page at rest ──
    const QString page = QStringLiteral(
        "Chapter One\n\nMr. Smith said bold and soft here.\n\nThe claim stated plainly\n\n- a list item"
        "\nScene break\nSecond scene.\nEnd of chapter\nChapter Two\n\nThe third scene, 1. not a list.");   // ✅ [SP-184] Q4
    same(h.page(), page, QStringLiteral("text(0, n) is the page at rest"));
    check(t->characterCount() == page.size(), QStringLiteral("characterCount = the page's length"));
    same(QAccessible::queryAccessibleInterface(h.editor)->text(QAccessible::Value), page, QStringLiteral("Value = the page"));

    // ── AC1: every member agrees with the text ──
    for (int k = 0; k < page.size(); ++k) {
        if (t->text(k, k + 1) != page.mid(k, 1)) {
            check(false, QStringLiteral("text(%1, %2) is the page's unit").arg(k).arg(k + 1));
            break;
        }
    }
    int a = 0, b = 0;
    const int smith = int(page.indexOf(QStringLiteral("Smith")));
    same(t->textAtOffset(smith, QAccessible::LineBoundary, &a, &b), QStringLiteral("Mr. Smith said bold and soft here.\n"),
         QStringLiteral("the line at 'Smith'"));
    check(t->text(a, b) == page.mid(a, b - a), QStringLiteral("textAtOffset's range is its text"));
    const int brk = int(page.indexOf(QStringLiteral("Scene break")));
    same(t->textAtOffset(brk + 3, QAccessible::LineBoundary, &a, &b), QStringLiteral("Scene break\n"),
         QStringLiteral("a divider reads as its words, a line of its own"));
    same(t->textAfterOffset(int(page.indexOf(QStringLiteral("- a list"))), QAccessible::LineBoundary, &a, &b),
         QStringLiteral("Scene break\n"), QStringLiteral("the line after the list item is the divider"));
    same(t->textBeforeOffset(brk, QAccessible::LineBoundary, &a, &b), QStringLiteral("- a list item\n"),
         QStringLiteral("the line before the divider is the list item"));
    same(t->textAtOffset(int(page.indexOf(QStringLiteral("old"))), QAccessible::WordBoundary, &a, &b),
         QStringLiteral("bold"), QStringLiteral("the word at 'old' is 'bold', no markers"));
    same(t->textAtOffset(int(page.indexOf(QStringLiteral("claim"))), QAccessible::ParagraphBoundary, &a, &b),
         QStringLiteral("The claim stated plainly\n"), QStringLiteral("a heading's paragraph has no '## '"));

    // ── AC2: the caret both ways ──
    const int boldPage = int(page.indexOf(QStringLiteral("bold")));
    const int boldStored = h.stored(0, s1, QStringLiteral("bold"));
    h.caret(boldStored);
    check(t->cursorPosition() == boldPage, QStringLiteral("Scrivi's caret at 'bold' is reported at the page's 'bold' (got %1, want %2)")
                                               .arg(t->cursorPosition()).arg(boldPage));
    h.caret(h.stored(0, s1, QStringLiteral(" here")));
    t->setCursorPosition(boldPage);
    QApplication::processEvents();
    check(h.editor->textCursor().position() == boldStored,
          QStringLiteral("a caret Orca places at 'bold' lands before 'b' (got %1, want %2)")
              .arg(h.editor->textCursor().position()).arg(boldStored));
    // Orca placing EVERY body unit reads back the same unit: no drift ([I-0282]). Leads snap into a body: skipped; a list
    // prefix's caret rule puts the caret after it (`- `), as for the writer.
    const QList<QString> bodies{QStringLiteral("Mr. Smith said bold and soft here.\n\nThe claim stated plainly\n\n- a list item"),
                                s2, QStringLiteral("The third scene, 1. not a list.")};
    const int listAt = int(page.indexOf(QStringLiteral("- a list item")));
    for (const QString& body : bodies) {
        const int from = int(page.indexOf(body));
        for (int k = from; k < from + body.size(); ++k) {
            if (k == listAt || k == listAt + 1) {
                continue;
            }
            t->setCursorPosition(k);
            QApplication::processEvents();
            if (t->cursorPosition() != k) {
                check(false, QStringLiteral("Orca placed the caret at %1 «%2» and reads it back at %3")
                                 .arg(k).arg(page.mid(k, 1)).arg(t->cursorPosition()));
                break;
            }
        }
    }
    const int dotStored = h.stored(0, s1, QStringLiteral(". Smith"));
    t->setCursorPosition(int(page.indexOf(QStringLiteral(". Smith"))));
    QApplication::processEvents();
    check(h.editor->textCursor().position() != dotStored, QStringLiteral("a placed caret never splits an escape pair"));
    // ⚠️ The step rule: with the writer's caret just before the `\`, placing Orca's caret ON the escaped '.' is a
    // PLACEMENT — read as a → step it would land after the '.', one unit on (the drift Apple's AC2 forbids).
    h.caret(dotStored - 1);
    t->setCursorPosition(int(page.indexOf(QStringLiteral(". Smith"))));
    QApplication::processEvents();
    check(t->cursorPosition() == int(page.indexOf(QStringLiteral(". Smith"))),
          QStringLiteral("placed on an escaped mark from before its backslash: reads back at %1, want %2")
              .arg(t->cursorPosition()).arg(page.indexOf(QStringLiteral(". Smith"))));

    // A selection Orca sets goes through the editor's selection rule (the opener comes with the word, as when the writer
    // selects it), and reads back as what it named.
    t->setSelection(0, boldPage, boldPage + 4);
    QApplication::processEvents();
    same(h.editor->textCursor().selectedText(), QStringLiteral("**bold"), QStringLiteral("Orca's selection of 'bold', stored"));
    t->selection(0, &a, &b);
    check(a == boldPage && b == boldPage + 4 && t->selectionCount() == 1, QStringLiteral("the selection reads back"));
    same(t->text(a, b), QStringLiteral("bold"), QStringLiteral("the selection's page text"));
    t->removeSelection(0);
    check(t->selectionCount() == 0, QStringLiteral("removeSelection"));

    // ── An edit: the page follows it ──
    h.caret(h.stored(0, s1, QStringLiteral(" here.")) + 5);   // before the '.'
    QTest::keyClicks(h.editor, QStringLiteral("X"));
    QApplication::processEvents();
    check(h.page().contains(QStringLiteral("and soft hereX.")), QStringLiteral("a typed character is on the page: ") + h.page().left(60));
    // An edit that keeps the scene's length (typing over a one-character selection) is on the page too.
    {
        QTextCursor c = h.editor->textCursor();
        c.setPosition(c.position() - 1, QTextCursor::KeepAnchor);
        h.editor->setTextCursor(c);
        QTest::keyClicks(h.editor, QStringLiteral("Z"));
        QApplication::processEvents();
        check(h.page().contains(QStringLiteral("and soft hereZ.")), QStringLiteral("a same-length edit is on the page: ") + h.page().left(60));
    }
    // ── Cost: ~1.8 MB, the way a first Orca query and a keystroke's queries meet it (Apple AC3: recorded) ──
    {
        QList<SceneDocument::Input> big;
        const QString para = QStringLiteral("He said \\*nothing\\* to **her**, and _then_ walked out into the night air. ");
        for (int i = 0; i < 1186; ++i) {
            big.append(scene(QStringLiteral("b%1").arg(i), QStringLiteral("c%1").arg(i / 20), QString(),
                             QStringLiteral("## Part %1\n\n").arg(i) + para.repeated(20)));
        }
        QElapsedTimer loadClock;
        loadClock.start();
        Harness m(big);
        // [I-0349]: the load (attach + the whole manuscript's first highlight) and the events it raised, settled.
        for (int i = 0; i < 20; ++i) { QApplication::processEvents(); }
        std::printf("measure: load of %lld chars, attach + first highlight + its events: %lld ms (accessibility %s)\n",
                    qint64(m.doc.document()->characterCount()), loadClock.elapsed(), QAccessible::isActive() ? "ON" : "off");
        QElapsedTimer timer;
        timer.start();
        const int n = m.text()->characterCount();
        const qint64 first = timer.elapsed();
        m.caret(m.doc.segments().at(600).bodyStart + 40);
        timer.restart();
        QTest::keyClicks(m.editor, QStringLiteral("y"));
        QApplication::processEvents();
        int s = 0, e = 0;
        const int at = m.text()->cursorPosition();
        m.text()->textAtOffset(at, QAccessible::LineBoundary, &s, &e);
        m.text()->textAtOffset(at, QAccessible::WordBoundary, &s, &e);
        const qint64 keystroke = timer.elapsed();
        std::printf("measure: %lld stored chars → %d page units; first query %lld ms; keystroke + caret/line/word %lld ms\n",
                    qint64(m.doc.document()->characterCount()), n, first, keystroke);

        // ✅ [I-0346]: a STRUCTURAL edit mid-book (a split, as a range call's delta lands) rebuilds only what it touched —
        // matched by position, every scene after it was rebuilt (~0.4 s on the VM, after every edit while Orca was on).
        const SceneSegment& mid = m.doc.segments().at(600);
        RangeDelta split;
        RangeDelta::Scene head{RangeDelta::Change::Rewritten, mid.sceneID, mid.chapterID, mid.title, mid.chapterTitle,
                               mid.metadataPath, mid.contentPath, mid.chapterMetadataPath,
                               m.doc.segments().at(599).sceneID, mid.text.left(40), true};
        RangeDelta::Scene tail{RangeDelta::Change::Created, QStringLiteral("split-tail"), mid.chapterID, QString(), mid.chapterTitle,
                               QString(), QString(), mid.chapterMetadataPath, mid.sceneID, mid.text.mid(40), true};
        split.scenes = {head, tail};
        m.applyingDelta = true;
        m.doc.applyRangeDelta(split);
        m.applyingDelta = false;
        QApplication::processEvents();
        timer.restart();
        const QString after = m.text()->text(0, m.text()->characterCount());
        const qint64 structural = timer.elapsed();
        const ManuscriptFind::PresentedText fresh = ManuscriptFind::PresentedText::build(m.doc, &m.presenter);
        check(after == fresh.pageText(0, fresh.pageLength()), QStringLiteral("I-0346: after a split the page is a fresh build's"));
        check(structural * 10 < first, QStringLiteral("I-0346: a split costs under a tenth of a full build (%1 ms vs %2 ms)")
                                          .arg(structural).arg(first));
        std::printf("measure: a split mid-book, then the page: %lld ms (a full build: %lld ms)\n", structural, first);
    }

    if (failures == 0) {
        std::printf("accessibility-ok\n");
    }
    return failures == 0 ? 0 : 1;
}
