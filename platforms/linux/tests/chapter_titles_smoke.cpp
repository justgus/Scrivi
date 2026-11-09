// chapter_titles_smoke.cpp — [SP-184] (EP-048 S5c, T-0611): Writing Surface ▸ Show chapter titles in manuscript (ON by
// default, Q1 re-ruled), and the divider that closes a chapter ([T-0575]), on Linux.
//
// REAL project windows through `AppEnvironment`; the dialog through Project ▸ Project Settings…. What is PAINTED is read
// from the viewport's pixels; what Orca reads from the presented page; every claim about a file FROM DISK.
// (Every structural edit, undo and redo with titles OFF is `structural_baseline_smoke`'s second run.)
//
// ✅ What it proves:
//   §1 the default (no setting): titles drawn; a divider before every scene but the first — the one closing a chapter in
//      the accent at 60% (Q3), the others in the rule colour; the page reads "End of chapter" / "Scene break" (Q4, Q6);
//   §2 the toggle: titles gone, every divider kept, the page's words kept; no text of a scene changed, no scene dirtied or
//      written, no history step; the caret and the top of the view kept; the choice saved; and back on;
//   §3 a file the Mac wrote (`false`) is drawn as written; a save of another setting does not pin the key (Q5);
//   §4 the one boundary builder: a rename and a new chapter with titles off add no title;
//   §5 the toggle on a dumas-sized manuscript (1,186 scenes, 58 chapters) is a small op — timed.
// ⚠️ NOT covered (live pass only): how the accent LOOKS on GNOME; Orca itself.
//
//   argv[1] = a scratch directory for the projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptFind.hpp"
#include "ManuscriptPresenter.hpp"
#include "ManuscriptTypography.hpp"
#include "ProjectPreferences.hpp"
#include "ProjectSettingsDialog.hpp"
#include "SceneDocument.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"
#include "ThemeColours.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenuBar>
#include <QScrollBar>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QTimer>

#include <algorithm>
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

// The colour painted across the middle of each divider's blank block, top of the view (the project fits on screen).
QList<QColor> paintedDividers(ManuscriptEditor* e, const SceneDocument& doc)
{
    e->verticalScrollBar()->setValue(0);
    e->viewport()->repaint();
    QApplication::processEvents();
    const QImage shot = e->viewport()->grab().toImage();
    QList<QColor> out;
    for (const SceneDocument::Divider& d : doc.dividers()) {
        const QTextBlock gap = e->document()->findBlock(d.pos);
        double top = 0;
        for (QTextBlock b = e->document()->begin(); b.isValid() && b != gap; b = b.next()) {
            top += e->document()->documentLayout()->blockBoundingRect(b).height();
        }
        // The 1 px line is somewhere in the block's height: the first row there that is not the page's colour.
        const QColor page = e->palette().color(QPalette::Base);
        const int from = int(top), to = int(top + e->document()->documentLayout()->blockBoundingRect(gap).height());
        QColor found;
        for (int y = std::max(0, from); y <= to && y < shot.height() && !found.isValid(); ++y) {
            if (shot.pixelColor(shot.width() / 2, y) != page) {
                found = shot.pixelColor(shot.width() / 2, y);
            }
        }
        out.append(found);
    }
    return out;
}
bool sameColour(const QColor& a, const QColor& b)
{
    return qAbs(a.red() - b.red()) <= 8 && qAbs(a.green() - b.green()) <= 8 && qAbs(a.blue() - b.blue()) <= 8;
}
QString name(const QColor& c) { return c.isValid() ? c.name() : QStringLiteral("nothing"); }

// [I-0357]: the lines drawn in the chapter-title face must be exactly the chapter titles shown — read from the
// presenter's formats on each line, against the headings the document says it holds.
void titleFaceRight(ScriviWindow* w, const QString& what)
{
    const ManuscriptEditor* e = editorOf(w);
    const SceneDocument& doc = w->editorShell()->sceneDocument();
    // A chapter title is Heading 1 (EP-047 P7); the fixture has no Markdown headings of its own to confuse it with.
    const double titleSize = e->document()->defaultFont().pointSizeF() * ManuscriptPresenter::headingScale(1);
    QStringList expected;
    if (doc.showChapterTitles()) {
        for (int i = 0; i < doc.segments().size(); ++i) {
            if (i == 0 || doc.segments().at(i).chapterID != doc.segments().at(i - 1).chapterID) {
                expected << doc.chapterHeadingText(i);
            }
        }
    }
    QStringList faced;
    for (QTextBlock b = e->document()->begin(); b.isValid(); b = b.next()) {
        const auto formats = b.layout()->formats();
        if (std::any_of(formats.cbegin(), formats.cend(),
                        [&](const QTextLayout::FormatRange& f) { return f.format.fontPointSize() == titleSize; })) {
            faced << b.text();
        }
    }
    check(faced == expected, QStringLiteral("%1: the title face on exactly the titles (drawn: %2 | expected: %3)")
                                 .arg(what, faced.join(QStringLiteral(" / ")).left(160), expected.join(QStringLiteral(" / "))));
}

QString page(ScriviWindow* w)
{
    const ManuscriptFind::PresentedText t = ManuscriptFind::PresentedText::build(w->editorShell()->sceneDocument(), presenterOf(w));
    return t.pageText(0, t.pageLength());
}

// The dividers as drawn, and as read, against the chapters: `scenes` per chapter.
void dividersRight(ScriviWindow* w, const QList<int>& scenes, const QString& what)
{
    const SceneDocument& doc = w->editorShell()->sceneDocument();
    const QList<SceneDocument::Divider> ds = doc.dividers();
    int total = 0;
    for (int n : scenes) {
        total += n;
    }
    check(ds.size() == total - 1, QStringLiteral("%1: a divider before every scene but the first (%2)").arg(what).arg(ds.size()));
    const QList<QColor> painted = paintedDividers(editorOf(w), doc);
    const QColor accent = ThemeColours::chapterEnd(editorOf(w)->palette()), rule = ThemeColours::rule(editorOf(w)->palette());
    int chapterEnds = 0;
    for (int i = 0; i < ds.size() && i < painted.size(); ++i) {
        chapterEnds += ds.at(i).chapterEnd ? 1 : 0;
        check(sameColour(painted.at(i), ds.at(i).chapterEnd ? accent : rule),
              QStringLiteral("%1: divider %2 (%3) painted %4, want %5").arg(what).arg(i)
                  .arg(ds.at(i).chapterEnd ? QStringLiteral("chapter end") : QStringLiteral("scene break"),
                       name(painted.at(i)), name(ds.at(i).chapterEnd ? accent : rule)));
    }
    check(chapterEnds == scenes.size() - 1, QStringLiteral("%1: %2 chapter ends").arg(what).arg(chapterEnds));
    const QString read = page(w);
    check(read.count(QStringLiteral("End of chapter")) == chapterEnds && read.count(QStringLiteral("Scene break")) == ds.size() - chapterEnds,
          QStringLiteral("%1: the page reads \"End of chapter\" ×%2 and \"Scene break\" ×%3 (got %4, %5)")
              .arg(what).arg(chapterEnds).arg(ds.size() - chapterEnds)
              .arg(read.count(QStringLiteral("End of chapter"))).arg(read.count(QStringLiteral("Scene break"))));
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: chapter_titles_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);

    const QList<QStringList> layout = {{QStringLiteral("Alpha one."), QStringLiteral("Alpha two.")},
                                       {QStringLiteral("Beta one."), QStringLiteral("Beta two.")},
                                       {QStringLiteral("Gamma one.")}};
    const QList<int> shape = {2, 2, 1};
    const Project A = makeProject(bridge, appSupport, scratch + QStringLiteral("/a.scrivi"), layout,
                                  {QStringLiteral("The Abbé"), QString(), QString()});
    const Project B = makeProject(bridge, appSupport, scratch + QStringLiteral("/b.scrivi"), layout, {},
                                  QStringLiteral(R"({"showChapterTitles":false})"));
    const Project C = makeProject(bridge, appSupport, scratch + QStringLiteral("/c.scrivi"), layout, {});

    AppEnvironment env(appSupport);

    // ── §1 The default: titles ON; the dividers ────────────────────────────────────────────────────────────────────────
    ScriviWindow* wa = openWindow(env, A);
    ManuscriptEditor* ea = editorOf(wa);
    QString text = ea->document()->toPlainText();
    check(wa->editorShell()->preferences()->showChapterTitles() && text.contains(QStringLiteral("The Abbé"))
              && text.contains(QStringLiteral("Chapter 2")) && text.contains(QStringLiteral("Chapter 3")),
          QStringLiteral("§1 no setting: titles ON (Q1 re-ruled), drawn: %1").arg(text.left(60).replace('\n', '|')));
    check(!QFile::exists(A.dir + QStringLiteral("/project-settings.json")), QStringLiteral("§1 … and the default is not written"));
    dividersRight(wa, shape, QStringLiteral("§1 titles on"));
    titleFaceRight(wa, QStringLiteral("§1 titles on"));
    check(page(wa).contains(QStringLiteral("The Abbé")), QStringLiteral("§1 the page reads the title too"));

    // ── §2 The toggle ───────────────────────────────────────────────────────────────────────────────────────────────────
    const QStringList before = [&] {
        QStringList t;
        for (const SceneSegment& s : wa->editorShell()->sceneDocument().segments()) {
            t << s.text;
        }
        return t;
    }();
    const QDateTime at0 = modified(A, 0), at4 = modified(A, 4);
    const bool couldUndo = wa->editorShell()->canUndo();
    {
        QTextCursor c(ea->document());
        c.setPosition(ea->document()->find(QStringLiteral("Beta two.")).selectionStart() + 4);
        ea->setTextCursor(c);
    }
    QTest::qWait(20);
    withDialog(wa, [](ProjectSettingsDialog* d) {
        check(d->chapterTitlesBox()->isChecked(), QStringLiteral("§2 the dialog shows titles on"));
        d->chapterTitlesBox()->setChecked(false);
    });
    text = ea->document()->toPlainText();
    check(!text.contains(QStringLiteral("The Abbé")) && !text.contains(QStringLiteral("Chapter 2")),
          QStringLiteral("§2 off: no title drawn (%1)").arg(text.left(60).replace('\n', '|')));
    check(settingsFile(A).value(QStringLiteral("showChapterTitles")) == QJsonValue(false), QStringLiteral("§2 … the choice saved"));
    dividersRight(wa, shape, QStringLiteral("§2 titles off"));
    titleFaceRight(wa, QStringLiteral("§2 titles off"));
    {
        QStringList after;
        for (const SceneSegment& s : wa->editorShell()->sceneDocument().segments()) {
            after << s.text;
        }
        check(after == before && wa->editorShell()->sceneDocument().textMatchesDocument(), QStringLiteral("§2 … no scene's text changed"));
        check(wa->editorShell()->dirtySceneCount() == 0, QStringLiteral("§2 … no scene dirtied (%1)").arg(wa->editorShell()->dirtySceneCount()));
        wa->editorShell()->saveDirtyScenes();
        check(modified(A, 0) == at0 && modified(A, 4) == at4, QStringLiteral("§2 … no scene file written"));
        check(wa->editorShell()->canUndo() == couldUndo, QStringLiteral("§2 … no history step"));
        const int caret = ea->textCursor().position();
        check(caret == ea->document()->find(QStringLiteral("Beta two.")).selectionStart() + 4,
              QStringLiteral("§2 … and the caret kept in its scene (at %1)").arg(caret));
    }
    withDialog(wa, [](ProjectSettingsDialog* d) { d->chapterTitlesBox()->setChecked(true); });
    text = ea->document()->toPlainText();
    check(text.contains(QStringLiteral("The Abbé")) && text.contains(QStringLiteral("Chapter 3")), QStringLiteral("§2 back on: titles drawn again"));
    dividersRight(wa, shape, QStringLiteral("§2 titles on again"));
    titleFaceRight(wa, QStringLiteral("§2 titles on again"));
    check(wa->editorShell()->dirtySceneCount() == 0 && wa->editorShell()->canUndo() == couldUndo,
          QStringLiteral("§2 … and again nothing dirtied or recorded"));

    // ── §3 A file the Mac wrote; a save of another setting does not pin the key (Q5) ──────────────────────────────────
    ScriviWindow* wb = openWindow(env, B);
    check(!wb->editorShell()->preferences()->showChapterTitles() && !editorOf(wb)->document()->toPlainText().contains(QStringLiteral("Chapter")),
          QStringLiteral("§3 the Mac's `false` is drawn as written"));
    dividersRight(wb, shape, QStringLiteral("§3 the Mac's file"));
    ScriviWindow* wc = openWindow(env, C);
    withDialog(wc, [](ProjectSettingsDialog* d) { d->indentEmBox()->setValue(2.0); });
    check(settingsFile(C).value(QStringLiteral("indentEm")).toDouble() == 2.0 && !settingsFile(C).contains(QStringLiteral("showChapterTitles")),
          QStringLiteral("§3 a save of another setting does not pin Show chapter titles: %1")
              .arg(QString::fromUtf8(QJsonDocument(settingsFile(C)).toJson(QJsonDocument::Compact))));

    // ── §4 The one boundary builder: a rename and a new chapter with titles off add no title ──────────────────────────
    {
        QList<SceneDocument::Input> in;
        for (int i = 0; i < 4; ++i) {
            const QString id = QStringLiteral("s%1").arg(i);
            in.push_back({id, QStringLiteral("c%1").arg(i / 2), id, QString(), id, {}, {}, {}, QStringLiteral("Scene %1.").arg(i)});
        }
        SceneDocument doc;
        doc.setShowChapterTitles(false);
        doc.build(in);
        check(doc.document()->toPlainText() == QStringLiteral("Scene 0.\n\nScene 1.\n\nScene 2.\n\nScene 3."),
              QStringLiteral("§4 built without titles: %1").arg(doc.document()->toPlainText().replace('\n', '|')));
        doc.setChapterTitle(QStringLiteral("c1"), QStringLiteral("Renamed"));
        check(!doc.document()->toPlainText().contains(QStringLiteral("Renamed")) && doc.textMatchesDocument(),
              QStringLiteral("§4 a rename draws no title"));
        doc.insertSceneAfter(3, QStringLiteral("s4"), QStringLiteral("c2"), QStringLiteral("Scene 4"), QStringLiteral("New"), QStringLiteral("s4"), {}, {}, {}, true);
        check(!doc.document()->toPlainText().contains(QStringLiteral("New")) && doc.dividers().size() == 4 && doc.dividers().last().chapterEnd,
              QStringLiteral("§4 a new chapter draws no title, and a chapter-end divider: %1").arg(doc.document()->toPlainText().replace('\n', '|')));
        doc.setShowChapterTitles(true);
        check(doc.document()->toPlainText().contains(QStringLiteral("Renamed")) && doc.document()->toPlainText().contains(QStringLiteral("New"))
                  && doc.textMatchesDocument(),
              QStringLiteral("§4 … and both titles drawn once titles show"));
    }

    // ── §5 The toggle on a dumas-sized manuscript is a small op ───────────────────────────────────────────────────────
    {
        QList<SceneDocument::Input> in;
        for (int i = 0; i < 1186; ++i) {
            const QString id = QStringLiteral("s%1").arg(i);
            in.push_back({id, QStringLiteral("c%1").arg(i * 58 / 1186), id, QString(), id, {}, {}, {}, QStringLiteral("Scene %1 text.\n\nMore.").arg(i)});
        }
        SceneDocument doc;
        doc.build(in);
        QElapsedTimer t;
        t.start();
        doc.setShowChapterTitles(false);
        const qint64 off = t.restart();
        doc.setShowChapterTitles(true);
        const qint64 on = t.elapsed();
        std::printf("§5 toggle on 1,186 scenes / 58 chapters: off %lld ms, on %lld ms\n", off, on);
        check(off < 250 && on < 250, QStringLiteral("§5 the toggle is a small op (off %1 ms, on %2 ms; the watchdog's line is 250 ms)").arg(off).arg(on));
        check(doc.textMatchesDocument(), QStringLiteral("§5 … and every scene's text is its body"));
    }

    env.quitApplication();
    QApplication::processEvents();
    if (failures == 0) {
        std::printf("titles-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
