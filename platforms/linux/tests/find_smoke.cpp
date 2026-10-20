// find_smoke — SP-176 / T-0603 (EP-048 S4, L8's Find half): Find over what the writer SEES, on Linux.
//
// Drives the REAL ManuscriptEditor + SceneDocument + ManuscriptPresenter + FindBar, wired as EditorShell wires them, through
// the REAL dispatch: the shared Edit ▸ Find builder's QAction shortcuts (QTest key clicks on an active window — `sendEvent`
// would skip Qt's shortcut map) and the bar's own keys. What it proves: a match is of the PRESENTED text (across a hidden
// escape, a hidden marker, a hidden heading prefix; a visible list prefix matches); never across a scene; never in a chapter
// title; the options; Find changes nothing; an edit never rebuilds the presented text per keystroke. Then it MEASURES the
// build and a search on a ~1.85 MB manuscript (Apple's dumas figure).
// ⚠️ The live pass on the rig is still owed (`feedback_live_pass_finds_what_suites_cannot`).

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QSet>
#include <QTest>
#include <QTextCursor>
#include <QVBoxLayout>

#include <cstdio>

#include "FindBar.hpp"
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
        std::fprintf(stderr, "FAIL: %s\n", qPrintable(what));
        ++failures;
    }
}

void same(const QString& actual, const QString& expected, const QString& what)
{
    check(actual == expected, QStringLiteral("%1\n   got:      %2\n   expected: %3").arg(what, actual, expected));
}

struct Harness {
    // ⚠️ Destroyed in reverse: the presenter detaches, then the window (editor + bar), then the document.
    SceneDocument doc;
    QMainWindow window;
    ManuscriptEditor* editor = new ManuscriptEditor;
    FindBar* bar = new FindBar(editor);
    ManuscriptPresenter presenter{&ScriviBridge::analyzeMarkdown};
    QSet<int> dirty;   // as EditorShell::onContentsChange marks it

    explicit Harness(const QList<SceneDocument::Input>& in)
    {
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
        bar->attach(&doc, &presenter);
        auto* column = new QWidget;
        auto* layout = new QVBoxLayout(column);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(bar);
        layout->addWidget(editor, 1);
        window.setCentralWidget(column);   // owns the editor and the bar
        QMenu* edit = window.menuBar()->addMenu(QStringLiteral("Edit"));
        addFindMenu(edit, &window, [this](FindCommand c) { bar->run(c); });
        window.resize(900, 600);
        window.show();
        window.activateWindow();
        check(QTest::qWaitForWindowActive(&window), QStringLiteral("the window became active (shortcuts need it)"));
        editor->setFocus();
        QApplication::processEvents();
    }
    int base(int scene) const { return doc.segments().at(scene).bodyStart; }
    QString body(int scene) const { return doc.bodyText(scene); }
    void caret(int scene, int at)
    {
        QTextCursor c = editor->textCursor();
        c.setPosition(base(scene) + at);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    void select(int start, int end)
    {
        QTextCursor c = editor->textCursor();
        c.setPosition(start);
        c.setPosition(end, QTextCursor::KeepAnchor);
        editor->setTextCursor(c);
        QApplication::processEvents();
    }
    // The STORED text of a document range.
    QString stored(const MarkdownRange& r)
    {
        QTextCursor c(doc.document()->begin());
        c.setPosition(r.start);
        c.setPosition(r.end, QTextCursor::KeepAnchor);
        QString t = c.selectedText();
        t.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        return t;
    }
    // The STORED text the editor has selected.
    QString selected() const
    {
        QString t = editor->textCursor().selectedText();
        t.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        return t;
    }
    std::pair<int, int> selection() const
    {
        const QTextCursor c = editor->textCursor();
        return {c.selectionStart(), c.selectionEnd()};
    }
    void key(QWidget* target, Qt::Key k, Qt::KeyboardModifiers m = Qt::NoModifier)
    {
        QTest::keyClick(target, k, m);
        QApplication::processEvents();
    }
    // Where a real key press goes: the focused widget (the shortcut map is consulted first).
    QWidget* focus() const { return QApplication::focusWidget() != nullptr ? QApplication::focusWidget() : editor; }
    void typeQuery(const QString& text)
    {
        QTest::keyClicks(bar->queryField(), text);
        settle();
    }
    // [T-0613]: the search runs after the typing pause, over text built ahead — wait for both.
    bool settle(int ms = 60000)
    {
        for (int i = 0; i < ms / 10 && !bar->searchSettled(); ++i) {
            QTest::qWait(10);
        }
        QApplication::processEvents();
        return bar->searchSettled();
    }
};

SceneDocument::Input scene(const QString& id, const QString& chapter, const QString& chapterTitle, const QString& body)
{
    return {id, chapter, id, chapterTitle, id, QString(), QString(), QString(), body};
}

int count(const QString& query, const ManuscriptFind::PresentedText& p, ManuscriptFind::Options o = {})
{
    return int(p.find(query, o).size());
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // ── The presented map: escapes, markers and heading prefixes hidden; list prefixes visible; per scene; no titles ──
    {
        const QString s1 = QStringLiteral("Mr\\. Smith said **bold** here.\n\n## The claim stated plainly\n\n- a list item ends");
        const QString s2 = QStringLiteral("ing the second scene. More *bold* text.");
        Harness h({scene("s1", "c1", "Chapter Zeta", s1), scene("s2", "c1", "Chapter Zeta", s2)});
        const auto p = ManuscriptFind::PresentedText::build(h.doc, &h.presenter);
        check(count("Mr. Smith", p) == 1, QStringLiteral("`Mr. Smith` matches across the hidden escape"));
        check(count("bold here", p) == 1, QStringLiteral("`bold here` matches across the hidden closing marker"));
        check(count("The claim", p) == 1, QStringLiteral("a heading matches without its hidden `## `"));
        check(count("- a list", p) == 1, QStringLiteral("a list prefix is visible, so it matches"));
        check(count("**", p) == 0, QStringLiteral("hidden markers never match"));
        check(count("\\", p) == 0, QStringLiteral("a hidden escape backslash never matches"));
        check(count("##", p) == 0, QStringLiteral("a hidden heading prefix never matches"));
        check(count("endsing", p) == 0 && count("ends\ning", p) == 0, QStringLiteral("a match never crosses a scene"));
        check(count("Zeta", p) == 0, QStringLiteral("chapter titles are never searched ([SP-164] Q4)"));
        check(count("bold", p) == 2, QStringLiteral("`bold` in both scenes"));

        using M = ManuscriptFind::Mode;
        check(count("smith", p) == 1, QStringLiteral("Ignore Case (default) finds `smith`"));
        check(count("smith", p, {M::Contains, false}) == 0, QStringLiteral("case-sensitive: `smith` is not found"));
        check(count("mith", p) == 1 && count("mith", p, {M::StartsWith, true}) == 0,
              QStringLiteral("Starts With: `mith` is not the start of a word"));
        check(count("Smi", p, {M::StartsWith, true}) == 1 && count("Smi", p, {M::WholeWord, true}) == 0,
              QStringLiteral("Whole Word: `Smi` is not a whole word"));
        check(count("Smith", p, {M::WholeWord, true}) == 1, QStringLiteral("Whole Word: `Smith`"));

        // The document ranges: the escape pair is never split; a following closing marker is not taken.
        same(h.stored(p.find(QStringLiteral("Mr. Smith"), {})[0]), QStringLiteral("Mr\\. Smith"),
             QStringLiteral("the `Mr. Smith` match is its stored text"));
        same(h.stored(p.find(QStringLiteral(". Smith"), {})[0]), QStringLiteral("\\. Smith"),
             QStringLiteral("a match starting at an escaped mark takes its `\\`"));
        same(h.stored(p.find(QStringLiteral("bold here"), {})[0]), QStringLiteral("bold** here"),
             QStringLiteral("`bold here` spans the hidden `**`"));
        const auto b = p.find(QStringLiteral("bold"), {});
        same(h.stored(b[0]), QStringLiteral("bold"), QStringLiteral("`bold` stops before its closing marker"));
        // Selected in the editor, it is SNAPPED so it never starts just after hidden markup (L5; design §7).
        h.select(b[0].start, b[0].end);
        same(h.selected(), QStringLiteral("**bold"), QStringLiteral("the editor holds `bold` as `**bold` (the opener is atomic)"));
        same(p.presentedOf(h.base(0), h.base(0) + 29), QStringLiteral("Mr. Smith said bold here"),
             QStringLiteral("presentedOf: the stored range as the writer sees it"));
    }

    // ── The real dispatch: Edit ▸ Find's shortcuts and the bar's keys; Find changes nothing ──
    {
        const QString s1 = QStringLiteral("One **bold** word, then bold again.");
        const QString s2 = QStringLiteral("A last BOLD one.");
        Harness h({scene("s1", "c1", "One", s1), scene("s2", "c1", "One", s2)});
        h.caret(0, 0);
        h.key(h.focus(), Qt::Key_F, Qt::ControlModifier);
        check(h.bar->isVisible(), QStringLiteral("Ctrl+F shows the find bar"));
        check(h.bar->queryField()->hasFocus(), QStringLiteral("Ctrl+F puts the focus in the query"));
        h.typeQuery(QStringLiteral("bold"));
        const auto all = h.bar->matches();
        check(all.size() == 3, QStringLiteral("incremental: 3 matches of `bold` (Ignore Case), got %1").arg(all.size()));
        // The first `bold` is `**bold**` — the editor holds it snapped as `**bold` (see above).
        check(h.selection() == std::pair{all[0].start - 2, all[0].end}, QStringLiteral("incremental: the first match is selected"));
        same(h.bar->countText(), QStringLiteral("1 of 3"), QStringLiteral("the count"));
        h.key(h.focus(), Qt::Key_G, Qt::ControlModifier);
        check(h.selection() == std::pair{all[1].start, all[1].end}, QStringLiteral("Ctrl+G: the next match"));
        h.key(h.focus(), Qt::Key_F3);
        check(h.selection() == std::pair{all[2].start, all[2].end}, QStringLiteral("F3: the next match (scene 2)"));
        h.key(h.bar->queryField(), Qt::Key_Return);
        check(h.selection() == std::pair{all[0].start - 2, all[0].end}, QStringLiteral("Return in the query: next, wrapping"));
        h.key(h.focus(), Qt::Key_G, Qt::ControlModifier | Qt::ShiftModifier);
        check(h.selection() == std::pair{all[2].start, all[2].end}, QStringLiteral("Ctrl+Shift+G: previous, wrapping"));
        h.key(h.focus(), Qt::Key_F3, Qt::ShiftModifier);
        check(h.selection() == std::pair{all[1].start, all[1].end}, QStringLiteral("Shift+F3: previous"));
        h.key(h.bar->queryField(), Qt::Key_Return, Qt::ShiftModifier);
        check(h.selection() == std::pair{all[0].start - 2, all[0].end}, QStringLiteral("Shift+Return in the query: previous"));
        same(h.bar->countText(), QStringLiteral("1 of 3"), QStringLiteral("the count follows the selection"));

        h.bar->ignoreCaseBox()->setChecked(false);
        check(h.bar->matches().size() == 2, QStringLiteral("Ignore Case off: `BOLD` no longer matches"));
        h.bar->ignoreCaseBox()->setChecked(true);
        h.bar->modeBox()->setCurrentIndex(2);
        h.bar->queryField()->setText(QStringLiteral("bol"));
        Q_EMIT h.bar->queryField()->textEdited(QStringLiteral("bol"));
        h.settle();
        same(h.bar->countText(), QStringLiteral("Not found"), QStringLiteral("Whole Word: `bol` is not found"));
        h.bar->modeBox()->setCurrentIndex(0);

        same(h.body(0), s1, QStringLiteral("Find changed nothing in scene 1"));
        same(h.body(1), s2, QStringLiteral("Find changed nothing in scene 2"));
        check(h.dirty.isEmpty(), QStringLiteral("Find marks no scene dirty"));

        h.key(h.bar->queryField(), Qt::Key_Escape);
        check(!h.bar->isVisible(), QStringLiteral("Esc closes the bar"));
        check(h.editor->hasFocus(), QStringLiteral("Esc returns the focus to the manuscript"));
        check(h.editor->extraSelections().isEmpty(), QStringLiteral("closing clears the highlights"));

        // Use Selection for Find: the PRESENTED text of the selection.
        h.select(h.base(0) + 4, h.base(0) + 18);   // "**bold** word,"
        h.key(h.focus(), Qt::Key_E, Qt::ControlModifier);
        same(h.bar->queryField()->text(), QStringLiteral("bold word,"), QStringLiteral("Ctrl+E: the selection as the writer sees it"));
        check(h.bar->isVisible(), QStringLiteral("Use Selection shows the bar"));
    }

    // ── [T-0613] Typing never waits: the search runs after a 500 ms pause, over text built ahead of time ──
    {
        Harness h({scene("s1", "c1", "One", QStringLiteral("One **bold** word, then bold again.")),
                   scene("s2", "c1", "One", QStringLiteral("A last BOLD one."))});
        for (int i = 0; i < 500 && !h.bar->presentedReady(); ++i) {
            QTest::qWait(10);
        }
        check(h.bar->presentedReady(), QStringLiteral("[T-0613] built ahead after the load, before any Find"));
        const int builds = h.bar->presentedBuilds();
        h.caret(0, 0);
        h.key(h.focus(), Qt::Key_F, Qt::ControlModifier);
        QTest::keyClicks(h.bar->queryField(), QStringLiteral("bold"));   // the writer's pace: no wait between keys
        check(h.bar->countText().isEmpty() && !h.editor->textCursor().hasSelection(),
              QStringLiteral("[T-0613] a keystroke searches nothing: no count, no selection (count: %1)").arg(h.bar->countText()));
        QTest::qWait(FindBar::kQueryPauseMs / 2);
        check(h.bar->countText().isEmpty(), QStringLiteral("[T-0613] still nothing half a pause after the last key"));
        check(h.settle(), QStringLiteral("[T-0613] the search ran once typing paused"));
        same(h.bar->countText(), QStringLiteral("1 of 3"), QStringLiteral("[T-0613] … and found every match"));
        check(h.editor->textCursor().hasSelection(), QStringLiteral("[T-0613] … selecting the first"));
        check(h.bar->presentedBuilds() == builds, QStringLiteral("[T-0613] the search built nothing (it was built ahead)"));
        // Return during the pause searches at once.
        h.bar->queryField()->selectAll();
        QTest::keyClicks(h.bar->queryField(), QStringLiteral("one"));
        h.key(h.bar->queryField(), Qt::Key_Return);
        check(h.bar->searchSettled() && h.bar->countText().endsWith(QStringLiteral(" of 2")),
              QStringLiteral("[T-0613] Return runs a pending search at once (count: %1)").arg(h.bar->countText()));
    }

    // ── [T-0613] An action before the build is complete waits for it ("Searching…"); no slice holds the GUI thread long ──
    {
        QList<SceneDocument::Input> in;
        const QString para = QStringLiteral("It was the *best* of times %1, Mr\\. Defarge said nothing at all to anyone %2. ");
        for (int i = 0; i < 1186; ++i) {
            QString body;
            for (int k = 0; body.size() < 1550; ++k) {
                body += para.arg(i).arg(k);
            }
            in.push_back(scene(QStringLiteral("s%1").arg(i), QStringLiteral("c%1").arg(i / 20), QStringLiteral("Chapter"), body));
        }
        Harness h(in);
        check(!h.bar->presentedReady(), QStringLiteral("[T-0613] precondition: 1,186 scenes are not built in the first idle turns"));
        h.key(h.focus(), Qt::Key_F, Qt::ControlModifier);
        h.bar->queryField()->setText(QStringLiteral("Mr. Defarge"));
        h.key(h.bar->queryField(), Qt::Key_Return);
        same(h.bar->countText(), QStringLiteral("Searching…"), QStringLiteral("[T-0613] Next before the build: \"Searching…\""));
        check(h.settle(), QStringLiteral("[T-0613] … then the build completed and the search ran"));
        check(h.bar->countText().contains(QStringLiteral(" of ")) && h.editor->textCursor().hasSelection(),
              QStringLiteral("[T-0613] … to a match (count: %1)").arg(h.bar->countText()));
        std::printf("measure: built ahead in slices; the longest held the GUI thread %lld ms\n",
                    static_cast<long long>(h.bar->longestSliceMs()));
        check(h.bar->longestSliceMs() < 100, QStringLiteral("[T-0613] no slice held the GUI thread 100 ms (longest %1 ms)")
                                                  .arg(h.bar->longestSliceMs()));
    }

    // ── An edit while the bar is open: no rebuild per keystroke; the count catches up once typing pauses ──
    {
        Harness h({scene("s1", "c1", "One", QStringLiteral("cat and cat.")), scene("s2", "c1", "One", QStringLiteral("dog"))});
        h.key(h.focus(), Qt::Key_F, Qt::ControlModifier);
        h.typeQuery(QStringLiteral("cat"));
        same(h.bar->countText(), QStringLiteral("1 of 2"), QStringLiteral("two cats"));
        const int builds = h.bar->presentedBuilds();
        h.caret(1, 3);
        QTest::keyClicks(h.editor, QStringLiteral(" cat cat"));
        QApplication::processEvents();
        check(h.bar->presentedBuilds() == builds, QStringLiteral("typing did not rebuild the presented text (%1 → %2)")
                                                      .arg(builds).arg(h.bar->presentedBuilds()));
        QTest::qWait(400);
        check(h.bar->presentedBuilds() == builds + 1, QStringLiteral("one rebuild once typing paused"));
        check(h.bar->scenesRebuilt() == 1, QStringLiteral("only the edited scene was rebuilt (%1)").arg(h.bar->scenesRebuilt()));
        same(h.bar->countText(), QStringLiteral("4 matches"), QStringLiteral("the count caught up"));
    }

    // ── Measured: ~1.85 MB, 1,186 scenes with emphasis, escapes and headings (Apple's dumas: 166–178 ms build) ──
    {
        QList<SceneDocument::Input> in;
        const QString para = QStringLiteral(
            "It was the *best* of times %1, it was the **worst** of times\\; Mr\\. Defarge said nothing at all to anyone %2. ");
        int bytes = 0;
        for (int i = 0; i < 1186; ++i) {
            QString body = QStringLiteral("## Part %1\n\n").arg(i);
            for (int k = 0; body.size() < 1550; ++k) {
                body += para.arg(i).arg(k);   // ⚠️ every paragraph UNIQUE: the presenter caches analysis by block text
                if (body.size() % 3 == 0) {
                    body += QStringLiteral("\n\n");
                }
            }
            bytes += int(body.toUtf8().size());
            in.push_back(scene(QStringLiteral("s%1").arg(i), QStringLiteral("c%1").arg(i / 20), QStringLiteral("Chapter"), body));
        }
        Harness h(in);
        QElapsedTimer t;
        t.start();
        auto p = ManuscriptFind::PresentedText::build(h.doc, &h.presenter);
        const qint64 buildMs = t.elapsed();
        t.restart();
        const int defarge = int(p.find(QStringLiteral("Mr. Defarge"), {}).size());
        const qint64 findMs = t.elapsed();
        t.restart();
        const int e = int(p.find(QStringLiteral("e"), {}).size());
        const qint64 commonMs = t.elapsed();
        std::printf("measure: %d bytes, %d scenes; presented %d units built in %lld ms\n", bytes, int(in.size()), p.length(),
                    buildMs);
        std::printf("measure: `Mr. Defarge` %d matches in %lld ms; `e` %d matches in %lld ms\n", defarge, findMs, e, commonMs);
        // An edit in one scene: only it is rebuilt.
        const int at = h.base(600) + 10;
        QTextCursor c(h.doc.document());
        c.setPosition(at);
        c.insertText(QStringLiteral("Mr. Defarge "));
        p.markEdited(h.doc, at);
        t.restart();
        const int rebuilt = p.refresh(h.doc, &h.presenter);
        const qint64 refreshMs = t.elapsed();
        std::printf("measure: an edit in one scene → %d scene(s) rebuilt in %lld ms\n", rebuilt, refreshMs);
        check(rebuilt == 1, QStringLiteral("an edit rebuilds one scene (%1)").arg(rebuilt));
        check(int(p.find(QStringLiteral("Mr. Defarge"), {}).size()) == defarge + 1, QStringLiteral("the edit is found"));
        check(defarge > 1186, QStringLiteral("`Mr. Defarge` found in every scene (%1)").arg(defarge));
        // ⚠️ A guard against a blow-up (a quadratic build is minutes), not a speed target: the time is printed above.
        // Builds are Debug (-O0) until release; CI's Debug build is ~7 s here.
        check(buildMs < 30000, QStringLiteral("the build stays in seconds, not minutes (%1 ms)").arg(buildMs));
    }

    if (failures == 0) {
        std::puts("find-ok");
        return 0;
    }
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
}
