// settings_smoke.cpp — SP-182 (EP-048 S5a, T-0609): Linux's Project Settings dialog, the bundled typefaces and Markup Hints.
//
// REAL project windows opened through `AppEnvironment`; the dialog through Project ▸ Project Settings…; keys through `QTest`
// into the real editor (the real dispatch — `feedback_test_through_the_real_dispatch`). Every claim about a file is read back
// FROM DISK.
//
// ✅ What it proves (SP-182 AC1–AC3):
//   §1 the dialog writes the shared files: the title to `project.json` (an empty one not written); only the keys the writer
//      CHOSE to `project-settings.json` (no default pinned);
//   §2 a file the Mac wrote is read as written, and a key this build does not know survives a save; a face this build does
//      not bundle is KEPT and drawn in the default;
//   §3 the type: the manuscript, headings and chapter titles in the project's face and size; a change re-presents only (no
//      text change, no scene file written, no history step); the face survives typing, Return, undo, a paste, a structured
//      paste and a reload;
//   §4 Markup Hints: Ctrl+Shift+H through the real dispatch; OFF reveals nothing and the caret rules hold; persisted.
//   §5 [SP-214] the PDF: the core's print page paginated by `writeManuscriptPdf` — the project's bundled face embedded, the
//      title page then the chapter, through the bridge; a long manuscript laid out to its end, its text black on a dark
//      desktop (the SP-214 live pass).
//   §6 [SP-215] File ▸ Copy as HTML, Import ▸ and Export ▸ in the Mac's order; the formats as submenus (AC7), a chapter's
//      without ePub; Current Chapter enabled at a chapter, and its PDF… straight to the save dialog.
// ⚠️ NOT covered (live pass only): how the faces LOOK, GNOME's bindings, the rig's Qt 6.10.
//
//   argv[1] = a scratch directory for the projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "HistoryCapture.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptPdf.hpp"
#include "ManuscriptPresenter.hpp"
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
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSpinBox>
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
    p.id = bridge.createProject(dir, appSupport, QStringLiteral("Settings"), QStringLiteral("settings"),
                                QStringLiteral("Test Author"))   // named: "Scrivi" would ask Who wrote…? (SP-211)
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
    ScriviWindow* w = env.openProjectWindow(p.dir, QStringLiteral("Settings"), {}, p.id);
    for (int i = 0; i < 3000 && env.windows().window(p.id) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(p.id) == w, QStringLiteral("project window loaded: %1").arg(p.dir));
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

// Opens Project ▸ Project Settings… and runs `act` on the dialog while it is up, then closes it.
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
    QAction* settings = menuAction(w, QStringLiteral("Project Settings…"));
    check(settings != nullptr, QStringLiteral("Project ▸ Project Settings… exists"));
    if (settings != nullptr) {
        settings->trigger();   // exec() returns when the timer above closes it
    }
    check(ran, QStringLiteral("the dialog was driven"));
    settle();
}

QJsonObject settingsFile(const Project& p)
{
    QFile f(p.dir + QStringLiteral("/project-settings.json"));
    return f.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(f.readAll()).object() : QJsonObject();
}
QString projectJsonTitle(const Project& p)
{
    QFile f(p.dir + QStringLiteral("/project.json"));
    return f.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(f.readAll()).object().value(QStringLiteral("title")).toString()
                                       : QString();
}
QString disk(ScriviBridge& bridge, const Project& p, int i)
{
    return bridge.readSceneTexts(p.dir, {p.sceneIDs.at(i)}).value(p.sceneIDs.at(i));
}
QDateTime modified(const Project& p, int i)
{
    const QFileInfo fi(p.contentPaths.at(i));
    return (fi.isAbsolute() ? fi : QFileInfo(p.dir + QLatin1Char('/') + p.contentPaths.at(i))).lastModified();
}

QString familyOf(const QString& faceName)
{
    const BundledFonts::Face* f = BundledFonts::face(faceName);
    return f != nullptr ? f->family : QString();
}

// ⛔ No character in the manuscript carries its own family: every run draws in the document's default font — so the face
// cannot be lost to an insert, a paste or an undo (the presenter's formats are layout formats, not character formats).
bool noRunOverridesTheFace(QTextDocument* doc)
{
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat f = it.fragment().charFormat();
            if (f.hasProperty(QTextFormat::FontFamilies) || f.hasProperty(QTextFormat::FontPointSize)) {
                return false;
            }
        }
    }
    return true;
}

bool drawsIn(ScriviWindow* w, const QString& faceName, double size)
{
    const QFont f = editorOf(w)->document()->defaultFont();
    return f.family() == familyOf(faceName) && f.pointSizeF() == size;
}

// The point size the presenter draws document position `pos` at (its layout formats).
double drawnSize(QTextDocument* doc, int pos)
{
    const QTextBlock b = doc->findBlock(pos);
    const int rel = pos - b.position();
    for (const QTextLayout::FormatRange& r : b.layout()->formats()) {
        if (r.start <= rel && rel < r.start + r.length && r.format.hasProperty(QTextFormat::FontPointSize)) {
            return r.format.fontPointSize();
        }
    }
    return doc->defaultFont().pointSizeF();
}

int positionOf(ManuscriptEditor* e, const QString& text) { return e->document()->find(text).selectionStart(); }

void caretAt(ManuscriptEditor* e, int pos)
{
    QTextCursor c = e->textCursor();
    c.setPosition(pos);
    e->setTextCursor(c);
    QApplication::processEvents();
}

void control(ManuscriptEditor* e, Qt::Key k, Qt::KeyboardModifiers extra = Qt::NoModifier)
{
    const QString text(QChar(char16_t(k - Qt::Key_A + 1)));
    QTest::sendKeyEvent(QTest::Press, e, k, text, Qt::ControlModifier | extra);
    QTest::sendKeyEvent(QTest::Release, e, k, text, Qt::ControlModifier | extra);
    settle();
}

// [SP-191] `act` starts a structural edit: wait for it to land (on a worker; nothing reloads).
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
        std::fprintf(stderr, "usage: settings_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    check(BundledFonts::manifest().faces.size() == 7, QStringLiteral("fonts.json lists 7 faces (got %1)").arg(BundledFonts::manifest().faces.size()));
    for (const BundledFonts::Face& f : BundledFonts::manifest().faces) {
        check(f.family == f.name, QStringLiteral("%1 registered (as \"%2\")").arg(f.name, f.family));
        check(QFile::exists(QStringLiteral(":/fonts/%1/OFL.txt").arg(f.folder)), QStringLiteral("%1's OFL.txt ships").arg(f.name));
    }

    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const QString heading = QStringLiteral("# Head line\n\nBody **bold** text.");
    const Project A = makeProject(bridge, appSupport, scratch + QStringLiteral("/a.scrivi"), {heading, QStringLiteral("Bravo scene.")});
    const Project B = makeProject(bridge, appSupport, scratch + QStringLiteral("/b.scrivi"), {QStringLiteral("Mac text.")},
                                  QStringLiteral(R"({"subtitle":"From the Mac","showChapterTitles":true,"typeface":"Newsreader",)"
                                                 R"("textSize":18,"markupHints":false,"futureKey":{"nested":[1,2]}})"));
    const Project C = makeProject(bridge, appSupport, scratch + QStringLiteral("/c.scrivi"), {QStringLiteral("Later text.")},
                                  QStringLiteral(R"({"typeface":"Future Face"})"));

    AppEnvironment env(appSupport);

    // ── §1 The dialog writes the shared files; only CHOSEN keys; the title to project.json ─────────────────────────────
    ScriviWindow* wa = openWindow(env, A);
    wa->raiseToFront();
    check(!QFile::exists(A.dir + QStringLiteral("/project-settings.json")), QStringLiteral("§1 opening wrote no settings file"));
    check(drawsIn(wa, QStringLiteral("Literata"), 16.0), QStringLiteral("§1 no setting: Literata 16 pt (P6, Q5)"));
    withDialog(wa, [](ProjectSettingsDialog* d) {
        check(d->typefaceBox()->count() == 7, QStringLiteral("§1 the dialog lists the 7 faces"));
        check(d->typefaceBox()->itemData(0, Qt::FontRole).value<QFont>().family() == d->typefaceBox()->itemText(0),
              QStringLiteral("§1 … each name drawn in its own face"));
        d->titleField()->selectAll();
        QTest::keyClicks(d->titleField(), QStringLiteral("Renamed"));
        QTest::keyClicks(d->subtitleField(), QStringLiteral("A subtitle"));
    });
    check(projectJsonTitle(A) == QStringLiteral("Renamed"), QStringLiteral("§1 the title is in project.json (%1)").arg(projectJsonTitle(A)));
    QJsonObject fa = settingsFile(A);
    check(fa.value(QStringLiteral("subtitle")).toString() == QStringLiteral("A subtitle"), QStringLiteral("§1 the subtitle is in project-settings.json"));
    check(!fa.contains(QStringLiteral("typeface")) && !fa.contains(QStringLiteral("textSize")) && !fa.contains(QStringLiteral("markupHints"))
              && !fa.contains(QStringLiteral("showChapterTitles")),
          QStringLiteral("§1 no unchosen key was written: %1").arg(QString::fromUtf8(QJsonDocument(fa).toJson(QJsonDocument::Compact))));
    withDialog(wa, [](ProjectSettingsDialog* d) {
        d->titleField()->selectAll();
        QTest::keyClick(d->titleField(), Qt::Key_Delete);   // empty
    });
    check(projectJsonTitle(A) == QStringLiteral("Renamed"), QStringLiteral("§1 an empty title is not written (%1)").arg(projectJsonTitle(A)));

    // ── §3 The type: face and size, re-presented only ─────────────────────────────────────────────────────────────────
    ManuscriptEditor* ea = editorOf(wa);
    QTextDocument* doc = ea->document();
    const QString textBefore = doc->toPlainText();
    const QDateTime at0 = modified(A, 0), at1 = modified(A, 1);
    const bool couldUndo = wa->editorShell()->canUndo();
    QTest::qWait(20);
    withDialog(wa, [](ProjectSettingsDialog* d) {
        const int i = d->typefaceBox()->findData(QStringLiteral("Courier Prime"));
        d->typefaceBox()->setCurrentIndex(i);
        emit d->typefaceBox()->activated(i);
        d->sizeBox()->setValue(20);
    });
    check(drawsIn(wa, QStringLiteral("Courier Prime"), 20.0), QStringLiteral("§3 the manuscript draws in Courier Prime 20 pt"));
    check(settingsFile(A).value(QStringLiteral("typeface")).toString() == QStringLiteral("Courier Prime")
              && settingsFile(A).value(QStringLiteral("textSize")).toDouble() == 20.0,
          QStringLiteral("§3 both choices are in the file"));
    check(ea->font().family() == familyOf(QStringLiteral("Courier Prime")), QStringLiteral("§3 … the editor's own font too"));
    const int headPos = positionOf(ea, QStringLiteral("Head line"));
    check(qAbs(drawnSize(doc, headPos) - 20.0 * 22.0 / 13.0) < 0.01,
          QStringLiteral("§3 a Heading 1 is sized from the face's size (%1)").arg(drawnSize(doc, headPos)));
    check(qAbs(drawnSize(doc, 0) - 20.0 * 22.0 / 13.0) < 0.01,
          QStringLiteral("§3 the chapter title is Heading 1 size (%1, \"%2\")").arg(drawnSize(doc, 0)).arg(doc->firstBlock().text()));
    check(doc->toPlainText() == textBefore, QStringLiteral("§3 a type change changes no text"));
    wa->editorShell()->saveDirtyScenes();
    check(modified(A, 0) == at0 && modified(A, 1) == at1, QStringLiteral("§3 … writes no scene file (nothing was dirtied)"));
    check(wa->editorShell()->canUndo() == couldUndo, QStringLiteral("§3 … and records no history step"));
    check(noRunOverridesTheFace(doc), QStringLiteral("§3 no run carries its own face"));

    // The face survives the writer's edits and a reload.
    caretAt(ea, positionOf(ea, QStringLiteral("Bravo scene.")) + 5);
    QTest::keyClicks(ea, QStringLiteral(" typed"));
    QTest::keyClick(ea, Qt::Key_Return);
    QTest::keyClicks(ea, QStringLiteral("next"));
    settle();
    control(ea, Qt::Key_Z);                                     // undo
    QApplication::clipboard()->setText(QStringLiteral("pasted words"));
    control(ea, Qt::Key_V);                                     // a flat paste
    check(noRunOverridesTheFace(doc) && drawsIn(wa, QStringLiteral("Courier Prime"), 20.0),
          QStringLiteral("§3 the face survives typing, Return, undo and a paste"));
    {
        // A structured paste: Ctrl+C across the scene break, Ctrl+V — ✅ [SP-191] its delta re-renders only the scenes it
        // touched (the new run must carry the face too).
        QTextCursor c = ea->textCursor();
        c.setPosition(positionOf(ea, QStringLiteral("text.")));
        c.setPosition(positionOf(ea, QStringLiteral("Bravo")) + 3, QTextCursor::KeepAnchor);
        ea->setTextCursor(c);
        control(ea, Qt::Key_C);
        caretAt(ea, positionOf(ea, QStringLiteral("Body")));
        check(landing(wa, [&] { control(ea, Qt::Key_V); }), QStringLiteral("§3 the structured paste landed"));
    }
    doc = editorOf(wa)->document();
    check(noRunOverridesTheFace(doc) && drawsIn(wa, QStringLiteral("Courier Prime"), 20.0),
          QStringLiteral("§3 … the face survives a structured paste (%1 %2)")
              .arg(doc->defaultFont().family()).arg(doc->defaultFont().pointSizeF()));

    // ── §4 Markup Hints: Ctrl+Shift+H, OFF reveals nothing, the caret rules hold, persisted ───────────────────────────
    ea = editorOf(wa);
    ManuscriptPresenter* pa = presenterOf(wa);
    wa->activateWindow();
    check(QTest::qWaitForWindowActive(wa), QStringLiteral("§4 the window became active (its shortcuts fire only then)"));
    QAction* hints = menuAction(wa, QStringLiteral("Show Markup Hints"));
    check(hints != nullptr && hints->isCheckable() && hints->isChecked(), QStringLiteral("§4 View ▸ Show Markup Hints, checked (absent = ON)"));
    const int prefix = ea->document()->find(QStringLiteral("# Head")).selectionStart();
    caretAt(ea, prefix + 4);                                    // on the heading's line
    check(!pa->isHidden(prefix), QStringLiteral("§4 hints ON: the caret's line shows its heading prefix"));
    const QString textAt = ea->document()->toPlainText();
    ea->setFocus();
    QTest::keyClick(ea, Qt::Key_H, Qt::ControlModifier | Qt::ShiftModifier);   // the real dispatch
    settle();
    check(!wa->editorShell()->markupHints() && hints != nullptr && !hints->isChecked(), QStringLiteral("§4 Ctrl+Shift+H turned them OFF"));
    check(pa->isHidden(prefix), QStringLiteral("§4 hints OFF: the prefix stays hidden beside the caret"));
    caretAt(ea, prefix);                                        // into the prefix: the caret goes home, after it
    check(ea->textCursor().position() == prefix + 2,
          QStringLiteral("§4 … and the caret rules hold (caret %1, prefix ends %2)").arg(ea->textCursor().position()).arg(prefix + 2));
    check(ea->document()->toPlainText() == textAt, QStringLiteral("§4 a toggle changes no text"));
    check(settingsFile(A).value(QStringLiteral("markupHints")) == QJsonValue(false), QStringLiteral("§4 OFF is persisted"));
    QTest::keyClick(ea, Qt::Key_H, Qt::ControlModifier | Qt::ShiftModifier);
    settle();
    caretAt(ea, prefix + 4);
    check(wa->editorShell()->markupHints() && !pa->isHidden(prefix), QStringLiteral("§4 Ctrl+Shift+H turns them back ON"));

    // ── §2 A file the Mac wrote: read as written; an unknown key survives; an unbundled face is kept ──────────────────
    ScriviWindow* wb = openWindow(env, B);
    ProjectPreferences* pb = wb->editorShell()->preferences();
    check(pb != nullptr && pb->subtitle() == QStringLiteral("From the Mac") && pb->typeface() == QStringLiteral("Newsreader")
              && pb->textSize() == 18.0 && !pb->markupHints(),
          QStringLiteral("§2 the Mac's file is read as written"));
    check(drawsIn(wb, QStringLiteral("Newsreader"), 18.0), QStringLiteral("§2 … and drawn: Newsreader 18 pt"));
    check(!presenterOf(wb)->hintsEnabled(), QStringLiteral("§2 … with Markup Hints off"));
    withDialog(wb, [](ProjectSettingsDialog* d) {
        check(d->typefaceBox()->currentData().toString() == QStringLiteral("Newsreader") && d->sizeBox()->value() == 18,
              QStringLiteral("§2 the dialog shows the Mac's choices"));
        d->sizeBox()->setValue(19);
    });
    const QJsonObject fb = settingsFile(B);
    check(fb.value(QStringLiteral("futureKey")).toObject().value(QStringLiteral("nested")).toArray().size() == 2
              && fb.value(QStringLiteral("showChapterTitles")).toBool() && fb.value(QStringLiteral("textSize")).toDouble() == 19.0,
          QStringLiteral("§2 a save keeps every key this build does not know: %1")
              .arg(QString::fromUtf8(QJsonDocument(fb).toJson(QJsonDocument::Compact))));

    ScriviWindow* wc = openWindow(env, C);
    check(wc->editorShell()->preferences()->typeface() == QStringLiteral("Future Face"), QStringLiteral("§2 an unbundled face is kept"));
    check(drawsIn(wc, QStringLiteral("Literata"), 16.0), QStringLiteral("§2 … and drawn in the default"));
    withDialog(wc, [](ProjectSettingsDialog* d) {
        check(d->typefaceBox()->currentText() == QStringLiteral("Future Face (not in this version)"),
              QStringLiteral("§2 … and shown as not in this version (%1)").arg(d->typefaceBox()->currentText()));
        d->sizeBox()->setValue(17);
    });
    check(settingsFile(C).value(QStringLiteral("typeface")).toString() == QStringLiteral("Future Face"),
          QStringLiteral("§2 … and still stored after a save"));

    // ── §5 [SP-214] The PDF: the print page paginated here, in the project's face ──────────────────────────────────────────
    const QVariantMap print = bridge.exportManuscript(B.dir, {{QStringLiteral("format"), QStringLiteral("html")},
                                                             {QStringLiteral("document"), QStringLiteral("print")},
                                                             {QStringLiteral("defaultTypeface"), QStringLiteral("Literata")}});
    check(!bridge.lastCallFailed() && print.value(QStringLiteral("typeface")).toString() == QStringLiteral("Newsreader"),
          QStringLiteral("§5 the print page names the project's face"));
    const QString pdfPath = scratch + QStringLiteral("/b.pdf");
    const QString pdfError = writeManuscriptPdf(print.value(QStringLiteral("text")).toString(), true, pdfPath);
    QFile pdf(pdfPath);   // boundary-ok: the smoke's own export, outside any package
    const QByteArray bytes = pdf.open(QIODevice::ReadOnly) ? pdf.readAll() : QByteArray();
    check(pdfError.isEmpty() && bytes.startsWith("%PDF-"), QStringLiteral("§5 a PDF is written (%1)").arg(pdfError));
    check(bytes.contains("Newsreader"), QStringLiteral("§5 … with the face embedded"));
    // Two pages: the title page, and the one chapter starting its own (page-break-before).
    check(bytes.count("/Type /Page\n") == 2, QStringLiteral("§5 … of two pages"));

    // SP-214 live pass: on a dark desktop the text was drawn white, and a long manuscript lost ~80% of its text (its headings'
    // size fix re-dirtied a layout Qt never finished). 60 chapters of 40 paragraphs, under a palette whose text is white.
    QString chapters;
    for (int c = 0; c < 60; ++c) {
        chapters += QStringLiteral("<h2 class=\"chapter\">Chapter %1</h2>\n").arg(c + 1);
        for (int p = 0; p < 40; ++p) {
            chapters += QStringLiteral("<p>%1</p>\n").arg(QStringLiteral("A sentence long enough to wrap the line. ").repeated(6));
        }
    }
    QPalette dark = QApplication::palette();
    dark.setColor(QPalette::Text, Qt::white);
    QApplication::setPalette(dark);
    QString longPage = print.value(QStringLiteral("text")).toString();
    longPage.replace(QStringLiteral("</body>"), chapters + QStringLiteral("</body>"));
    const QString longPath = scratch + QStringLiteral("/long.pdf");
    const QString longError = writeManuscriptPdf(longPage, true, longPath);
    QFile longPdf(longPath);   // boundary-ok: the smoke's own export, outside any package
    const QByteArray longBytes = longPdf.open(QIODevice::ReadOnly) ? longPdf.readAll() : QByteArray();
    // Each chapter is over 6 pages of text; the broken layout gave ~2.
    check(longError.isEmpty() && longBytes.count("/Type /Page\n") > 1 + 60 * 6,
          QStringLiteral("§5 a long manuscript is laid out to its end (%1 pages)").arg(longBytes.count("/Type /Page\n")));
    // The pages' drawing, inflated: qUncompress wants zlib data behind a 4-byte size, a hint it grows past.
    QByteArray drawing;
    for (qsizetype at = longBytes.indexOf("stream\n"); at >= 0; at = longBytes.indexOf("stream\n", at)) {
        at += 7;
        const qsizetype end = longBytes.indexOf("endstream", at);
        if (end < 0) {
            break;
        }
        const qsizetype dict = longBytes.lastIndexOf(" obj", at);   // the stream's own dictionary, from its "N 0 obj"
        if (longBytes.mid(dict, at - dict).contains("/FlateDecode")) {
            drawing += qUncompress(QByteArray("\x00\x01\x00\x00", 4) + longBytes.mid(at, end - at));
        }
        at = end + 9;   // past "endstream", which ends in "stream" too
    }
    check(drawing.contains("BT") && !drawing.contains("1 1 1 scn"), QStringLiteral("§5 … its text black on a dark desktop"));

    // ── §6 [SP-215] File ▸ Copy as HTML, Import ▸, Export ▸, through the real menu bar ────────────────────────────────────
    QMenu* fileMenu = wa->menuBar()->actions().value(0)->menu();
    const auto texts = [](QMenu* m) {
        QStringList out;
        for (QAction* a : m->actions()) {
            if (!a->isSeparator()) { out << a->text(); }
        }
        return out;
    };
    const QStringList fileItems = texts(fileMenu);
    const qsizetype copyAt = fileItems.indexOf(QStringLiteral("Copy as HTML"));
    check(copyAt >= 0 && fileItems.mid(copyAt, 3) == QStringList{QStringLiteral("Copy as HTML"), QStringLiteral("Import"),
                                                                   QStringLiteral("Export")},
          QStringLiteral("§6 File holds Copy as HTML, Import, Export in order (%1)").arg(fileItems.join(QStringLiteral(", "))));
    QMenu* importMenu = nullptr;
    QMenu* exportMenu = nullptr;
    for (QAction* a : fileMenu->actions()) {
        if (a->text() == QStringLiteral("Import")) { importMenu = a->menu(); }
        if (a->text() == QStringLiteral("Export")) { exportMenu = a->menu(); }
    }
    check(importMenu != nullptr && texts(importMenu) == QStringList{QStringLiteral("Timeline…"), QStringLiteral("Archive…")},
          QStringLiteral("§6 Import ▸ Timeline…, Archive…"));
    check(exportMenu != nullptr && texts(exportMenu) == QStringList{QStringLiteral("Manuscript"), QStringLiteral("Current Chapter"),
                                                                    QStringLiteral("Timeline…"), QStringLiteral("Archive…")},
          QStringLiteral("§6 Export ▸ Manuscript, Current Chapter, Timeline…, Archive…"));
    if (exportMenu != nullptr) {
        // AC7: the format is the item — a submenu each, a chapter without ePub.
        QMenu* manuscriptMenu = exportMenu->actions().value(0)->menu();
        QMenu* chapterMenu = exportMenu->actions().value(1)->menu();
        const QStringList chapterFormats{QStringLiteral("Markdown…"), QStringLiteral("HTML…"), QStringLiteral("Word Document…"),
                                         QStringLiteral("PDF…")};
        check(manuscriptMenu != nullptr && texts(manuscriptMenu) == QStringList{QStringLiteral("Markdown…"), QStringLiteral("HTML…"),
                                                                                QStringLiteral("Word Document…"), QStringLiteral("ePub…"),
                                                                                QStringLiteral("PDF…")},
              QStringLiteral("§6 Export ▸ Manuscript ▸ Markdown…, HTML…, Word Document…, ePub…, PDF…"));
        check(chapterMenu != nullptr && texts(chapterMenu) == chapterFormats,
              QStringLiteral("§6 Export ▸ Current Chapter ▸ the same, without ePub (%1)")
                  .arg(chapterMenu != nullptr ? texts(chapterMenu).join(QStringLiteral(", ")) : QString()));
        caretAt(ea, positionOf(ea, QStringLiteral("Bravo scene.")) + 2);
        Q_EMIT exportMenu->aboutToShow();
        check(exportMenu->actions().value(1)->isEnabled(), QStringLiteral("§6 Current Chapter is enabled with the caret in a chapter"));
        // Current Chapter ▸ PDF… goes straight to the save dialog: no format or scope question.
        bool saved = false;
        QString other;
        QTimer::singleShot(0, wa, [&] {
            QWidget* modal = QApplication::activeModalWidget();
            saved = qobject_cast<QFileDialog*>(modal) != nullptr;
            if (!saved && modal != nullptr) { other = modal->windowTitle(); }
            if (modal != nullptr) { modal->close(); }
        });
        if (chapterMenu != nullptr) { chapterMenu->actions().value(3)->trigger(); }
        settle();
        check(saved, QStringLiteral("§6 Current Chapter ▸ PDF… opens the save dialog, with no question first (%1)").arg(other));
    }

    env.quitApplication();
    QApplication::processEvents();
    if (failures == 0) {
        std::printf("settings-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
