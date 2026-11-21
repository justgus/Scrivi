// navigator_search_smoke.cpp — [SP-230] (EP-059 E, T-0692 + T-0249): Linux names an untitled scene as the Mac does, has the
// Mac's six Go To commands, and searches the Navigator with the Mac's rules.
//
// REAL project windows through `AppEnvironment`; the Go To commands through their menu actions.
//
// ✅ What it proves:
//   §1 E1 the first-line rule on the Mac's own cases (`ScriviInteropTests.swift`, presentedFirstLine); the Navigator names
//      untitled scenes by it, "Scene N" when blank; a name follows typing;
//   §2 E2 Go to Scene / Chapter / Manuscript Start and End move the caret there;
//   §3 E3 search: a chapter-title match keeps its scenes; a name or body match keeps the scene; bodies as the page shows them;
//      case and diacritics ignored; every chapter open while searching; cleared, the writer's chapters again.
//
//   argv[1] = a scratch directory for the projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "ManuscriptTypography.hpp"
#include "EditorShell.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptPresenter.hpp"
#include "NavigatorTree.hpp"
#include "SceneDocument.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTreeView>

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

struct Project {
    QString dir;
    QString id;
    QStringList contentPaths;
};

// Chapters of scenes, `layout[c]` = the scene texts of chapter c; chapter titles from `titles` (empty = untitled).
Project makeProject(ScriviBridge& bridge, const QString& appSupport, const QString& dir, const QList<QStringList>& layout,
                    const QStringList& titles, const QString& settingsJson = {})
{
    Project p;
    p.dir = dir;
    p.id = bridge.createProject(dir, appSupport, QStringLiteral("Titles"), QStringLiteral("titles"))
               .value(QStringLiteral("projectID")).toString();
    auto scenes = [&] { return bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList(); };
    QVariantList all = scenes();
    QString chapterID = all.first().toMap().value(QStringLiteral("chapterID")).toString();
    QString lastScene = all.first().toMap().value(QStringLiteral("sceneID")).toString();
    for (int c = 0; c < layout.size(); ++c) {
        for (int k = (c == 0 ? 1 : 0); k < layout.at(c).size(); ++k) {
            const QVariantMap made = c > 0 && k == 0
                                         ? bridge.createChapter(dir, appSupport, p.id, chapterID)
                                         : bridge.createScene(dir, appSupport, p.id, chapterID, lastScene);
            if (c > 0 && k == 0) {
                chapterID = made.value(QStringLiteral("chapterID")).toString();
                lastScene = made.value(QStringLiteral("firstSceneID")).toString();
            } else {
                lastScene = made.value(QStringLiteral("sceneID")).toString();
            }
        }
    }
    all = scenes();
    QStringList texts;
    for (const QStringList& chapter : layout) {
        texts += chapter;
    }
    QString seenChapter;
    int chapter = -1;
    for (int i = 0; i < all.size() && i < texts.size(); ++i) {
        const QVariantMap s = all.at(i).toMap();
        p.contentPaths.append(s.value(QStringLiteral("contentPath")).toString());
        bridge.saveScene(p.id, dir, appSupport, s.value(QStringLiteral("sceneID")).toString(),
                         s.value(QStringLiteral("metadataPath")).toString(), p.contentPaths.last(), texts.at(i), 0, 0, 0.0);
        if (s.value(QStringLiteral("chapterID")).toString() != seenChapter) {
            seenChapter = s.value(QStringLiteral("chapterID")).toString();
            ++chapter;
            if (chapter < titles.size() && !titles.at(chapter).isEmpty()) {
                bridge.renameChapter(dir, s.value(QStringLiteral("chapterMetadataPath")).toString(), titles.at(chapter));
            }
        }
    }
    if (!settingsJson.isEmpty()) {
        bridge.putProjectSettings(dir, settingsJson);   // as the Mac would have written it
    }
    bridge.closeProject(dir);
    return p;
}

ScriviWindow* openWindow(AppEnvironment& env, const Project& p)
{
    ScriviWindow* w = env.openProjectWindow(p.dir, QStringLiteral("Titles"), {}, p.id);
    for (int i = 0; i < 3000 && env.windows().window(p.id) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(p.id) == w, QStringLiteral("project window loaded: %1").arg(p.dir));
    w->resize(900, 700);
    w->raiseToFront();
    QTest::qWait(200);
    return w;
}

ManuscriptEditor* editorOf(ScriviWindow* w) { return w->editorShell()->findChild<ManuscriptEditor*>(); }
ManuscriptPresenter* presenterOf(ScriviWindow* w) { return w->editorShell()->findChild<ManuscriptPresenter*>(); }

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: navigator_search_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);

    const QList<QStringList> layout = {
        {QStringLiteral("## Youth against age\n\nBody."), QStringLiteral(" \n\n")},
        {QStringLiteral("Mr\\. **Smith** said *hi*"), QStringLiteral("Plain text about a café.")},
        {QStringLiteral("- a list item"), QStringLiteral("Gamma.")}};
    const Project P = makeProject(bridge, appSupport, scratch + QStringLiteral("/search.inkwyn"), layout,
                                  {QStringLiteral("The Abbé"), QString(), QString()});
    AppEnvironment env(appSupport);
    ScriviWindow* w = openWindow(env, P);
    EditorShell* shell = w->editorShell();
    ManuscriptEditor* editor = editorOf(w);
    QTreeView* tree = shell->findChild<NavigatorTree*>();
    check(tree != nullptr && editor != nullptr, QStringLiteral("the window has its Navigator and editor"));
    auto chapterRow = [&](int r) { return tree->model()->index(r, 0); };
    auto names = [&] {
        QStringList out;
        for (int c = 0; c < tree->model()->rowCount(); ++c) {
            for (int r = 0; r < tree->model()->rowCount(chapterRow(c)); ++r) {
                out << tree->model()->index(r, 0, chapterRow(c)).data().toString();
            }
        }
        return out.join(QStringLiteral(" | "));
    };

    // ── §1 E1: untitled scenes ──────────────────────────────────────────────────────────────────────────────────────
    {
        ManuscriptPresenter* p = presenterOf(w);
        check(p != nullptr, QStringLiteral("§1 the manuscript has its presenter"));
        if (p != nullptr) {
            auto t = [&](const QString& s) { return p->presentedFirstLine(s); };
            check(t(QStringLiteral("## Youth against age\n\nBody.")) == QStringLiteral("Youth against age"),
                  QStringLiteral("§1 a heading prefix is hidden"));
            check(t(QStringLiteral("\n\nMr\\. **Smith** said *hi*")) == QStringLiteral("Mr. Smith said hi"),
                  QStringLiteral("§1 escapes and emphasis markers are hidden (%1)").arg(t(QStringLiteral("\n\nMr\\. **Smith** said *hi*"))));
            check(t(QStringLiteral("**bold\nline two**")) == QStringLiteral("bold"), QStringLiteral("§1 a pair closing on a later line is a pair"));
            check(t(QStringLiteral("- a list item")) == QStringLiteral("- a list item"), QStringLiteral("§1 a list prefix is shown"));
            check(t(QStringLiteral(" \n\n")).isEmpty(), QStringLiteral("§1 a blank scene has no first line"));
        }
        // A new project's first scene is created titled "Opening Scene": a title is kept as it is.
        const QString want = QStringLiteral("Opening Scene | Scene 2 | Mr. Smith said hi | Plain text about a café. | "
                                            "- a list item | Gamma.");
        check(names() == want, QStringLiteral("§1 the Navigator names untitled scenes as the Mac does (%1)").arg(names()));
        check(shell->sceneName(1) == QStringLiteral("Scene 2"), QStringLiteral("§1 … and so does every other place (sceneName)"));
        // She types into the blank scene: its name follows.
        QTextCursor c(editor->document());
        c.setPosition(shell->sceneDocument().segments().at(1).bodyStart);
        editor->setTextCursor(c);
        QTest::keyClicks(editor, QStringLiteral("Night"));
        QTest::qWait(50);
        check(names().contains(QStringLiteral("| Night")) || names().contains(QStringLiteral("| Night ")),
              QStringLiteral("§1 the name follows her typing (%1)").arg(names()));
    }

    // ── §2 E2: Go To ──────────────────────────────────────────────────────────────────────────────────────────────────
    {
        const auto& segs = shell->sceneDocument().segments();
        auto action = [&](const char* name) { return w->findChild<QAction*>(QString::fromLatin1(name)); };
        auto caret = [&] { return editor->textCursor().position(); };
        auto place = [&](int pos) {
            QTextCursor c(editor->document());
            c.setPosition(pos);
            editor->setTextCursor(c);
        };
        const int inScene3 = segs.at(3).bodyStart + 3;   // chapter 2's second scene
        struct Case { const char* action; int expect; const char* what; };
        const Case cases[] = {
            {"goToSceneStartAction", segs.at(3).bodyStart, "Scene Start"},
            {"goToSceneEndAction", segs.at(3).bodyStart + segs.at(3).bodyLength, "Scene End"},
            {"goToChapterStartAction", segs.at(2).bodyStart, "Chapter Start"},
            {"goToChapterEndAction", segs.at(3).bodyStart + segs.at(3).bodyLength, "Chapter End"},
            // The first scene opens with `## `: the caret never rests inside a heading prefix, so it lands after it (as on the Mac).
            {"goToManuscriptStartAction", segs.at(0).bodyStart + 3, "Manuscript Start"},
            {"goToManuscriptEndAction", segs.at(5).bodyStart + segs.at(5).bodyLength, "Manuscript End"},
        };
        for (const Case& k : cases) {
            QAction* a = action(k.action);
            check(a != nullptr && a->isEnabled(), QStringLiteral("§2 Go to %1 is in the menus, enabled").arg(k.what));
            if (a == nullptr) { continue; }
            place(inScene3);
            a->trigger();
            QTest::qWait(20);
            check(caret() == k.expect, QStringLiteral("§2 Go to %1: caret %2, expected %3").arg(k.what).arg(caret()).arg(k.expect));
        }
        // Q2's keys: Ctrl+Home / Ctrl+End. In the editor and the Navigator their own keys answer first.
        check(action("goToManuscriptStartAction")->shortcut() == QKeySequence(Qt::CTRL | Qt::Key_Home)
                  && action("goToManuscriptEndAction")->shortcut() == QKeySequence(Qt::CTRL | Qt::Key_End),
              QStringLiteral("§2 Manuscript Start / End are on Ctrl+Home / Ctrl+End"));
        place(inScene3);
        QTest::keyClick(editor, Qt::Key_End, Qt::ControlModifier);
        check(caret() == segs.at(5).bodyStart + segs.at(5).bodyLength, QStringLiteral("§2 Ctrl+End in the editor: caret %1").arg(caret()));
        place(inScene3);
    }

    // ── §3 E3: the Navigator's search ──────────────────────────────────────────────────────────────────────────────────
    {
        QLineEdit* field = shell->navigatorSearchField();
        check(field != nullptr && field->isVisibleTo(w), QStringLiteral("§3 the Navigator has a search field"));
        // Per chapter: "x" hidden, else one digit per scene (1 shown), "+" when the chapter is open.
        auto shown = [&] {
            QStringList out;
            for (int c = 0; c < tree->model()->rowCount(); ++c) {
                if (tree->isRowHidden(c, QModelIndex())) { out << QStringLiteral("x"); continue; }
                QString s;
                for (int r = 0; r < tree->model()->rowCount(chapterRow(c)); ++r) {
                    s += tree->isRowHidden(r, chapterRow(c)) ? '0' : '1';
                }
                out << s + (tree->isExpanded(chapterRow(c)) ? QStringLiteral("+") : QString());
            }
            return out.join(',');
        };
        auto search = [&](const QString& q) {
            field->setText(q);
            for (int i = 0; i < 500 && !shell->navigatorSearchSettled(); ++i) { QTest::qWait(10); }
            check(shell->navigatorSearchSettled(), QStringLiteral("§3 “%1”: the body search settles").arg(q));
            return shown();
        };
        QString s = search(QStringLiteral("abbe"));
        check(s == QStringLiteral("11+,x,x"), QStringLiteral("§3 a chapter-title match keeps its scenes; diacritics ignored (%1)").arg(s));
        s = search(QStringLiteral("MR. SMITH"));
        check(s == QStringLiteral("x,10+,x"), QStringLiteral("§3 a name match, as the page shows it; case ignored (%1)").arg(s));
        s = search(QStringLiteral("cafe"));
        check(s == QStringLiteral("x,01+,x"), QStringLiteral("§3 “cafe” finds “café” (%1)").arg(s));
        s = search(QStringLiteral("body"));
        check(s == QStringLiteral("10+,x,x"), QStringLiteral("§3 a body match keeps the scene under its chapter (%1)").arg(s));
        s = search(QStringLiteral("smith said"));
        check(s == QStringLiteral("x,10+,x"), QStringLiteral("§3 bodies are matched as the page shows them, markers hidden (%1)").arg(s));
        s = search(QStringLiteral("zzz"));
        check(s == QStringLiteral("x,x,x"), QStringLiteral("§3 nothing matches: every chapter hidden (%1)").arg(s));
        s = search(QString());
        check(s == QStringLiteral("11,11+,11"), QStringLiteral("§3 cleared: every row back; only the current chapter open (%1)").arg(s));
        check(shell->expandedChapterIDs().isEmpty(), QStringLiteral("§3 … and searching chose no chapter of hers"));
    }

    env.quitApplication();
    QApplication::processEvents();
    if (failures == 0) {
        std::printf("navigator-search-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
