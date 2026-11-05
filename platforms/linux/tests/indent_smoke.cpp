// indent_smoke.cpp — [SP-183] (EP-048 S5b, T-0610): the manuscript's fixed line heights and first-line indent on Linux,
// drawn by `ManuscriptLayout` with ZERO characters in the file; and [I-0351] (a repaint walked the whole document).
//
// REAL project windows opened through `AppEnvironment`; the dialog through Project ▸ Project Settings…; keys through `QTest`
// into the real editor. What is DRAWN is read from each block's laid-out lines; every claim about a file is read FROM DISK.
//
// ✅ What it proves:
//   §1 Book convention (the default): no indent at a scene's start, after a heading, a list or a quote; a heading, a list
//      item and a later line of a block never indented; body lines 1.45 × size, a heading's 1.25 × its own size, a chapter
//      title's Heading 1 line, a separator a body line, a blank line ⅓ of a body line;
//   §2 an edit to the paragraph BEFORE re-draws the next one (a `#` typed, then undone); the indent survives typing, Return,
//      undo, a paste, Replace All and a structured paste;
//   §3 the dialog: Every paragraph · None (a blank line full height again; Indent disabled) · the amount in ems; each saved
//      under Apple's keys; no text change, no scene file written, no history step;
//   §4 a reload keeps it; a file the Mac wrote is drawn as written; a mode from a later Scrivi is drawn as Book and KEPT;
//   §5 [I-0351]: a keystroke asks for a bounded number of block rectangles, however many scenes come before the caret.
// ⚠️ NOT covered (live pass only): how it LOOKS beside the Mac, Orca, GNOME, the rig's fonts.
//
//   argv[1] = a scratch directory for the projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "FindBar.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptLayout.hpp"
#include "ManuscriptTypography.hpp"
#include "ProjectPreferences.hpp"
#include "ProjectSettingsDialog.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenuBar>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QTimer>

#include <cstdio>
#include <functional>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

void settle() { QTest::qWait(60); }

struct Project {
    QString dir;
    QString id;
    QStringList sceneIDs;
    QStringList contentPaths;
};

Project makeProject(ScriviBridge& bridge, const QString& appSupport, const QString& dir, const QStringList& seeds,
                    const QString& settingsJson = {})
{
    Project p;
    p.dir = dir;
    p.id = bridge.createProject(dir, appSupport, QStringLiteral("Indent"), QStringLiteral("indent"))
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
        p.contentPaths.append(s.value(QStringLiteral("contentPath")).toString());
        bridge.saveScene(p.id, dir, appSupport, p.sceneIDs.last(), s.value(QStringLiteral("metadataPath")).toString(),
                         p.contentPaths.last(), seeds.at(i), 0, 0, 0.0);
    }
    if (!settingsJson.isEmpty()) {
        bridge.putProjectSettings(dir, settingsJson);   // as the Mac would have written it
    }
    bridge.closeProject(dir);
    return p;
}

ScriviWindow* openWindow(AppEnvironment& env, const Project& p)
{
    ScriviWindow* w = env.openProjectWindow(p.dir, QStringLiteral("Indent"), {}, p.id);
    for (int i = 0; i < 3000 && env.windows().window(p.id) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(p.id) == w, QStringLiteral("project window loaded: %1").arg(p.dir));
    w->raiseToFront();
    settle();
    return w;
}

ManuscriptEditor* editorOf(ScriviWindow* w) { return w->editorShell()->findChild<ManuscriptEditor*>(); }

QAction* menuAction(ScriviWindow* w, const QString& text)
{
    for (QAction* top : w->menuBar()->actions()) {
        if (top->menu() == nullptr) {
            continue;
        }
        for (QAction* a : top->menu()->actions()) {
            if (a->text() == text) {
                return a;
            }
        }
    }
    return nullptr;
}

void withDialog(ScriviWindow* w, const std::function<void(ProjectSettingsDialog*)>& act)
{
    bool ran = false;
    QTimer::singleShot(0, w, [&] {
        auto* d = qobject_cast<ProjectSettingsDialog*>(QApplication::activeModalWidget());
        check(d != nullptr, QStringLiteral("Project Settings… opens the dialog"));
        if (d != nullptr) {
            act(d);
            ran = true;
            d->reject();
        }
    });
    if (QAction* settings = menuAction(w, QStringLiteral("Project Settings…"))) {
        settings->trigger();
    }
    check(ran, QStringLiteral("the dialog was driven"));
    settle();
}

QJsonObject settingsFile(const Project& p)
{
    QFile f(p.dir + QStringLiteral("/project-settings.json"));
    return f.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(f.readAll()).object() : QJsonObject();
}
QDateTime modified(const Project& p, int i)
{
    const QFileInfo fi(p.contentPaths.at(i));
    return (fi.isAbsolute() ? fi : QFileInfo(p.dir + QLatin1Char('/') + p.contentPaths.at(i))).lastModified();
}

// One em of the body text, px — Apple's `size`, measured from the editor's own screen.
double em(ManuscriptEditor* e) { return e->document()->defaultFont().pointSizeF() * e->logicalDpiY() / 72.0; }

// How block `b` is DRAWN: its first line's indent and its lines' pitch, px.
struct Drawn { double indent = -1; double pitch = -1; int lines = 0; };
Drawn drawn(QTextDocument* doc, const QTextBlock& b)
{
    if (!b.isValid()) {
        return {};
    }
    const QRectF r = doc->documentLayout()->blockBoundingRect(b);   // laid out now, as painting would
    const QTextLayout* tl = b.layout();
    if (tl->lineCount() == 0) {
        return {};
    }
    const double foot = b.next().isValid() ? 0 : doc->documentMargin();
    return {tl->lineAt(0).x() - doc->documentMargin(), (r.height() - foot) / tl->lineCount(), tl->lineCount()};
}
QTextBlock lineOf(QTextDocument* doc, const QString& text) { return doc->find(text).block(); }
double indentOf(ManuscriptEditor* e, const QString& text) { return drawn(e->document(), lineOf(e->document(), text)).indent; }
bool near(double a, double b) { return qAbs(a - b) < 0.5; }
QString px(double v) { return QString::number(v, 'f', 1); }

void caretAt(ManuscriptEditor* e, int pos)
{
    QTextCursor c = e->textCursor();
    c.setPosition(pos);
    e->setTextCursor(c);
    QApplication::processEvents();
}
int positionOf(ManuscriptEditor* e, const QString& text) { return e->document()->find(text).selectionStart(); }

void control(ManuscriptEditor* e, Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
{
    const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
    QTest::sendKeyEvent(QTest::Press, e, k, text, Qt::ControlModifier | extra);
    QTest::sendKeyEvent(QTest::Release, e, k, text, Qt::ControlModifier | extra);
    settle();
}

bool landing(ScriviWindow* w, const std::function<void()>& act)
{
    QEventLoop loop;
    bool ok = false;
    bool done = false;
    QObject::connect(w->editorShell(), &EditorShell::structuralEditFinished, &loop, [&](bool r) {
        ok = r;
        done = true;
        loop.quit();
    });
    QTimer::singleShot(30000, &loop, &QEventLoop::quit);
    act();
    if (!done) {
        loop.exec();
    }
    settle();
    return ok;
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: indent_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);

    const QString scene0 = QStringLiteral(
        "First para one.\n\nSecond para two.\n\n# A heading\n\nAfter heading.\n\n- a list item\n\nAfter list.\n\n"
        "> a quote\n\nAfter quote.\n\nLine one of a block\nline two of it.");
    const QString scene1 = QStringLiteral("Scene two opens.\n\nScene two second.");
    const Project A = makeProject(bridge, appSupport, scratch + QStringLiteral("/a.scrivi"), {scene0, scene1});
    const Project B = makeProject(bridge, appSupport, scratch + QStringLiteral("/b.scrivi"),
                                  {QStringLiteral("Mac one.\n\nMac two.")},
                                  QStringLiteral(R"({"paragraphIndent":"every","indentEm":2.5})"));
    const Project C = makeProject(bridge, appSupport, scratch + QStringLiteral("/c.scrivi"),
                                  {QStringLiteral("Later one.\n\nLater two.")}, QStringLiteral(R"({"paragraphIndent":"hanging"})"));
    QStringList many;
    for (int i = 0; i < 80; ++i) {
        many << QStringLiteral("Scene %1 opens here.\n\nScene %1 goes on.").arg(i);
    }
    const Project D = makeProject(bridge, appSupport, scratch + QStringLiteral("/d.scrivi"), many);

    AppEnvironment env(appSupport);

    // ── §1 Book convention, the line heights ───────────────────────────────────────────────────────────────────────────
    ScriviWindow* wa = openWindow(env, A);
    ManuscriptEditor* ea = editorOf(wa);
    QTextDocument* doc = ea->document();
    check(dynamic_cast<ManuscriptLayout*>(doc->documentLayout()) != nullptr, QStringLiteral("§1 the manuscript is laid out by ManuscriptLayout"));
    const double size = em(ea);
    const double indent = 1.5 * size, body = 1.45 * size;
    const QString textBefore = doc->toPlainText();
    const QDateTime at0 = modified(A, 0), at1 = modified(A, 1);
    const bool couldUndo = wa->editorShell()->canUndo();

    struct Expect { const char* text; bool indented; };
    const Expect book[] = {{"First para one.", false}, {"Second para two.", true}, {"# A heading", false}, {"After heading.", false},
                           {"- a list item", false},   {"After list.", false},     {"> a quote", false},   {"After quote.", false},
                           {"Line one of a block", true}, {"line two of it.", false}, {"Scene two opens.", false},
                           {"Scene two second.", true}};
    for (const Expect& x : book) {
        const double got = indentOf(ea, QString::fromUtf8(x.text));
        check(near(got, x.indented ? indent : 0), QStringLiteral("§1 Book: \"%1\" indent %2 (want %3)")
                                                      .arg(QString::fromUtf8(x.text), px(got), px(x.indented ? indent : 0)));
    }
    check(near(drawn(doc, lineOf(doc, QStringLiteral("Second para two."))).pitch, body),
          QStringLiteral("§1 a body line is 1.45 × size (%1, want %2)").arg(px(drawn(doc, lineOf(doc, QStringLiteral("Second para two."))).pitch), px(body)));
    const Drawn heading = drawn(doc, lineOf(doc, QStringLiteral("# A heading")));
    check(near(heading.pitch, size * 22.0 / 13.0 * 1.25), QStringLiteral("§1 a heading line is 1.25 × its own size (%1)").arg(px(heading.pitch)));
    const Drawn title = drawn(doc, doc->firstBlock());
    check(near(title.pitch, size * 22.0 / 13.0 * 1.25) && near(title.indent, 0),
          QStringLiteral("§1 the chapter title is on Heading 1's line, never indented (%1, \"%2\")").arg(px(title.pitch), doc->firstBlock().text()));
    const Drawn gap = drawn(doc, lineOf(doc, QStringLiteral("First para one.")).next());
    check(near(gap.pitch, body / 3.0), QStringLiteral("§1 a blank line between paragraphs is ⅓ of a body line (%1)").arg(px(gap.pitch)));
    const Drawn separator = drawn(doc, lineOf(doc, QStringLiteral("Scene two opens.")).previous());
    check(near(separator.pitch, body), QStringLiteral("§1 a scene separator is a body line (%1)").arg(px(separator.pitch)));
    check(doc->toPlainText() == textBefore, QStringLiteral("§1 drawing it changed no text"));
    check(wa->editorShell()->dirtySceneCount() == 0, QStringLiteral("§1 … dirtied no scene (%1)").arg(wa->editorShell()->dirtySceneCount()));
    wa->editorShell()->saveDirtyScenes();
    check(modified(A, 0) == at0 && modified(A, 1) == at1, QStringLiteral("§1 … wrote no scene file"));
    check(wa->editorShell()->canUndo() == couldUndo, QStringLiteral("§1 … and recorded no history step"));
    check(!QFile::exists(A.dir + QStringLiteral("/project-settings.json")), QStringLiteral("§1 … and pinned no default"));

    // ── §2 An edit to the paragraph before; the edits it survives ─────────────────────────────────────────────────────
    wa->activateWindow();
    check(QTest::qWaitForWindowActive(wa), QStringLiteral("§2 the window became active"));
    // Format ▸ Heading 1 (Ctrl+Alt+1, [SP-175]) — a typed `#` is stored escaped, as on the Mac (EP-049).
    caretAt(ea, positionOf(ea, QStringLiteral("First para one.")) + 3);
    QTest::keyClick(ea, Qt::Key_1, Qt::ControlModifier | Qt::AltModifier);
    settle();
    check(doc->toPlainText().contains(QStringLiteral("# First para one.")), QStringLiteral("§2 Ctrl+Alt+1 made the paragraph before a heading"));
    check(near(indentOf(ea, QStringLiteral("Second para two.")), 0),
          QStringLiteral("§2 … so the next paragraph follows a heading: no indent (%1)").arg(px(indentOf(ea, QStringLiteral("Second para two.")))));
    for (int i = 0; i < 3 && doc->toPlainText().contains(QStringLiteral("# First")); ++i) {
        control(ea, Qt::Key_Z);
    }
    check(!doc->toPlainText().contains(QStringLiteral("# First")), QStringLiteral("§2 … undone"));
    check(near(indentOf(ea, QStringLiteral("Second para two.")), indent), QStringLiteral("§2 … and the indent is back"));

    caretAt(ea, positionOf(ea, QStringLiteral("Second para two.")) + 7);
    QTest::keyClicks(ea, QStringLiteral("typed "));
    settle();
    check(near(indentOf(ea, QStringLiteral("Second typed para")), indent), QStringLiteral("§2 the indent survives typing"));
    caretAt(ea, positionOf(ea, QStringLiteral(" para two.")));
    QTest::keyClick(ea, Qt::Key_Return);
    settle();
    check(near(indentOf(ea, QStringLiteral("Second typed")), indent) && near(indentOf(ea, QStringLiteral(" para two.")), indent),
          QStringLiteral("§2 … and Return (a new paragraph: it follows body text, so it is indented too)"));
    control(ea, Qt::Key_Z);
    check(near(indentOf(ea, QStringLiteral("Second typed para two.")), indent), QStringLiteral("§2 … and its undo"));
    QApplication::clipboard()->setText(QStringLiteral("pasted words "));
    caretAt(ea, positionOf(ea, QStringLiteral("Second typed")));
    control(ea, Qt::Key_V);
    check(near(indentOf(ea, QStringLiteral("pasted words Second")), indent), QStringLiteral("§2 … and a paste at its start"));

    // Replace All (off the GUI thread, ONE step).
    control(ea, Qt::Key_F, Qt::AltModifier);
    auto* bar = wa->editorShell()->findChild<FindBar*>();
    check(bar != nullptr && bar->isVisible(), QStringLiteral("§2 Ctrl+Alt+F shows the find bar"));
    if (bar != nullptr) {
        bar->queryField()->setText(QStringLiteral("para"));
        emit bar->queryField()->textEdited(QStringLiteral("para"));
        bar->replaceField()->setText(QStringLiteral("PARA"));
        for (int i = 0; i < 6000 && !bar->searchSettled(); ++i) {
            QTest::qWait(10);
        }
        bar->replaceAll();
        for (int i = 0; i < 500 && !bar->countText().endsWith(QStringLiteral("replaced")); ++i) {
            QTest::qWait(10);
        }
        check(bar->countText().endsWith(QStringLiteral("replaced")), QStringLiteral("§2 Replace All landed (%1)").arg(bar->countText()));
        settle();
        check(near(indentOf(ea, QStringLiteral("Second typed PARA")), indent) && near(indentOf(ea, QStringLiteral("First PARA one.")), 0),
              QStringLiteral("§2 … and the indents survive it (%1, %2)")
                  .arg(px(indentOf(ea, QStringLiteral("Second typed PARA"))), px(indentOf(ea, QStringLiteral("First PARA one.")))));
        bar->hide();
    }
    {
        // A structured paste: across the scene break, pasted before "After heading." — its delta re-renders the scenes.
        QTextCursor c = ea->textCursor();
        c.setPosition(positionOf(ea, QStringLiteral("it.")));
        c.setPosition(positionOf(ea, QStringLiteral("Scene two opens.")) + 5, QTextCursor::KeepAnchor);
        ea->setTextCursor(c);
        control(ea, Qt::Key_C);
        caretAt(ea, positionOf(ea, QStringLiteral("After heading.")));
        check(landing(wa, [&] { control(ea, Qt::Key_V); }), QStringLiteral("§2 the structured paste landed"));
    }
    ea = editorOf(wa);
    doc = ea->document();
    check(near(indentOf(ea, QStringLiteral("Scene two second.")), indent) && near(indentOf(ea, QStringLiteral("Second typed PARA")), indent),
          QStringLiteral("§2 … and the indents survive a structured paste (%1, %2)")
              .arg(px(indentOf(ea, QStringLiteral("Scene two second."))), px(indentOf(ea, QStringLiteral("Second typed PARA")))));
    wa->editorShell()->saveDirtyScenes();
    for (int i = 0; i < 500 && wa->editorShell()->dirtySceneCount() > 0; ++i) {   // the saves are queued (EP-052)
        QTest::qWait(10);
    }
    check(wa->editorShell()->dirtySceneCount() == 0, QStringLiteral("§2 §2's edits are saved"));

    // ── §3 The dialog: Every · None · the amount; Apple's keys; no text change ────────────────────────────────────────
    const QString textAt = doc->toPlainText();
    const QDateTime bt0 = modified(A, 0), bt1 = modified(A, 1);
    const bool canUndoNow = wa->editorShell()->canUndo();
    QTest::qWait(20);
    withDialog(wa, [](ProjectSettingsDialog* d) {
        check(d->indentBox()->currentData().toString() == QStringLiteral("book") && d->indentEmBox()->value() == 1.5
                  && d->indentEmBox()->isEnabled(),
              QStringLiteral("§3 the dialog shows Book convention, 1.5 em"));
        const int every = d->indentBox()->findData(QStringLiteral("every"));
        d->indentBox()->setCurrentIndex(every);
        emit d->indentBox()->activated(every);
    });
    check(settingsFile(A).value(QStringLiteral("paragraphIndent")).toString() == QStringLiteral("every")
              && !settingsFile(A).contains(QStringLiteral("indentEm")),
          QStringLiteral("§3 Every paragraph is saved under Apple's key; the amount, not chosen, is not"));
    check(near(indentOf(ea, QStringLiteral("First PARA one.")), indent) && near(indentOf(ea, QStringLiteral("After list.")), indent)
              && near(indentOf(ea, QStringLiteral("# A heading")), 0) && near(indentOf(ea, QStringLiteral("- a list item")), 0),
          QStringLiteral("§3 Every: a scene's first paragraph and one after a list are indented; a heading and a list item never"));
    withDialog(wa, [](ProjectSettingsDialog* d) {
        const int none = d->indentBox()->findData(QStringLiteral("none"));
        d->indentBox()->setCurrentIndex(none);
        emit d->indentBox()->activated(none);
        check(!d->indentEmBox()->isEnabled(), QStringLiteral("§3 None disables Indent"));
    });
    check(near(indentOf(ea, QStringLiteral("Scene two second.")), 0)
              && near(drawn(doc, lineOf(doc, QStringLiteral("Scene two opens.")).next()).pitch, body),
          QStringLiteral("§3 None: no indent, and a blank line is a full body line again (%1)")
              .arg(px(drawn(doc, lineOf(doc, QStringLiteral("Scene two opens.")).next()).pitch)));
    withDialog(wa, [](ProjectSettingsDialog* d) {
        const int bookAt = d->indentBox()->findData(QStringLiteral("book"));
        d->indentBox()->setCurrentIndex(bookAt);
        emit d->indentBox()->activated(bookAt);
        d->indentEmBox()->setValue(2.0);
    });
    check(near(indentOf(ea, QStringLiteral("Scene two second.")), 2.0 * size) && settingsFile(A).value(QStringLiteral("indentEm")).toDouble() == 2.0,
          QStringLiteral("§3 the amount in ems: 2.0 em drawn and saved (%1)").arg(px(indentOf(ea, QStringLiteral("Scene two second.")))));
    check(doc->toPlainText() == textAt, QStringLiteral("§3 no change of mode changed text"));
    check(wa->editorShell()->dirtySceneCount() == 0, QStringLiteral("§3 … dirtied no scene (%1)").arg(wa->editorShell()->dirtySceneCount()));
    wa->editorShell()->saveDirtyScenes();
    check(modified(A, 0) == bt0 && modified(A, 1) == bt1, QStringLiteral("§3 … wrote no scene file"));
    check(wa->editorShell()->canUndo() == canUndoNow, QStringLiteral("§3 … and recorded no history step"));

    // ── §4 A reload; a file the Mac wrote; a mode from a later Scrivi ─────────────────────────────────────────────────
    wa->close();
    settle();
    wa = openWindow(env, A);
    ea = editorOf(wa);
    check(near(indentOf(ea, QStringLiteral("Scene two second.")), 2.0 * size) && near(indentOf(ea, QStringLiteral("Scene two opens.")), 0),
          QStringLiteral("§4 a reload draws it as saved"));
    ScriviWindow* wb = openWindow(env, B);
    ManuscriptEditor* eb = editorOf(wb);
    check(near(indentOf(eb, QStringLiteral("Mac one.")), 2.5 * em(eb)) && near(indentOf(eb, QStringLiteral("Mac two.")), 2.5 * em(eb)),
          QStringLiteral("§4 the Mac's file is drawn as written: Every paragraph, 2.5 em (%1)").arg(px(indentOf(eb, QStringLiteral("Mac one.")))));
    ScriviWindow* wc = openWindow(env, C);
    ManuscriptEditor* ec = editorOf(wc);
    check(near(indentOf(ec, QStringLiteral("Later one.")), 0) && near(indentOf(ec, QStringLiteral("Later two.")), 1.5 * em(ec)),
          QStringLiteral("§4 a mode this build does not know is drawn as Book convention"));
    withDialog(wc, [](ProjectSettingsDialog* d) { d->indentEmBox()->setValue(1.0); });
    check(settingsFile(C).value(QStringLiteral("paragraphIndent")).toString() == QStringLiteral("hanging"),
          QStringLiteral("§4 … and KEPT through a save (%1)").arg(settingsFile(C).value(QStringLiteral("paragraphIndent")).toString()));

    // ── §5 [I-0351]: a keystroke's layout work does not grow with the scenes before the caret ─────────────────────────
    ScriviWindow* wd = openWindow(env, D);
    ManuscriptEditor* ed = editorOf(wd);
    auto* layout = dynamic_cast<ManuscriptLayout*>(ed->document()->documentLayout());
    wd->activateWindow();
    check(QTest::qWaitForWindowActive(wd), QStringLiteral("§5 the window became active"));
    caretAt(ed, positionOf(ed, QStringLiteral("Scene 0 opens")) + 5);
    ed->ensureCursorVisible();
    settle();
    if (layout != nullptr) {
        const int before = layout->boundingRectCalls();
        QTest::keyClick(ed, Qt::Key_X);
        settle();
        const int calls = layout->boundingRectCalls() - before;
        check(calls < 1000, QStringLiteral("§5 one keystroke asked for %1 block rectangles (80 scenes; the walk asked for tens of thousands)").arg(calls));
    }

    env.quitApplication();
    QApplication::processEvents();
    if (failures == 0) {
        std::printf("indent-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
