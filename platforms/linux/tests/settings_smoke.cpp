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
// ⚠️ NOT covered (live pass only): how the faces LOOK, GNOME's bindings, the rig's Qt 6.10.
//
//   argv[1] = a scratch directory for the projects; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "HistoryCapture.hpp"
#include "ManuscriptEditor.hpp"
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
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
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
    p.id = bridge.createProject(dir, appSupport, QStringLiteral("Settings"), QStringLiteral("settings"))
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

    env.quitApplication();
    QApplication::processEvents();
    if (failures == 0) {
        std::printf("settings-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
