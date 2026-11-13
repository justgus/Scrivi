// authorship_smoke.cpp — EP-058 S3 ([SP-211], T-0672): authorship on Linux, THROUGH the bridge and real project windows.
//
// ✅ What it proves (SP-211 AC1–AC5):
//   §1 the bridge's five calls: pen names round-trip and a repeated name is refused; a project created under a pen name
//      opens asking nothing, writing as that author, and the name joined the menu;
//   §2 Who wrote…?: a project credited to "Scrivi" opens a window with the question up; Esc does not dismiss it; the answer
//      renames the author (its ID kept), and a scene typed afterwards is recorded as that author — read FROM DISK;
//   §3 Which of these authors are you?: another identity's project asks it; choosing an author adopts it;
//   §4 a world's first read takes the project's author; its list is then its own;
//   §5 the menus: Edit ▸ Settings… on Ctrl+, (also with no project open), Project Settings… on Ctrl+Alt+,; Project Settings
//      shows the Authors section.
// ⚠️ NOT covered (live pass only): how the dialogs look, the desktop's own shortcuts.
//
//   argv[1] = a scratch directory; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "AuthorshipDialogs.hpp"
#include "EditorShell.hpp"
#include "LandingWindow.hpp"
#include "ManuscriptEditor.hpp"
#include "ProjectSettingsDialog.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QTest>
#include <QTimer>

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

void settle() { QTest::qWait(60); }

QAction* menuAction(QMainWindow* w, const QString& text)
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

ScriviWindow* openWindow(AppEnvironment& env, const QString& dir, const QString& id)
{
    ScriviWindow* w = env.openProjectWindow(dir, QStringLiteral("Byline"), {}, id);
    for (int i = 0; i < 3000 && env.windows().window(id) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(id) == w, QStringLiteral("project window loaded: %1").arg(dir));
    settle();
    return w;
}

QStringList authorNames(ScriviBridge& bridge, const QString& root)
{
    QStringList out;
    for (const QVariant& a : bridge.getAuthors(root).value(QStringLiteral("authors")).toList()) {
        out << a.toMap().value(QStringLiteral("displayName")).toString();
    }
    return out;
}

QJsonObject sidecar(const QString& dir, const QString& metadataPath)
{
    QFile f(QDir::isAbsolutePath(metadataPath) ? metadataPath : dir + QLatin1Char('/') + metadataPath);
    return f.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(f.readAll()).object() : QJsonObject();
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: authorship_smoke <scratchDir>\n");
        return 2;
    }
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);

    // ── §1 the bridge's calls ──────────────────────────────────────────────────────────────────────────────────────────
    check(bridge.getPenNames(appSupport).value(QStringLiteral("penNames")).toStringList().isEmpty(), QStringLiteral("§1 no pen names yet"));
    bridge.putPenNames(appSupport, {QStringLiteral("J. D. Quill")}, QStringLiteral("J. D. Quill"));
    check(bridge.getPenNames(appSupport).value(QStringLiteral("default")).toString() == QStringLiteral("J. D. Quill"),
          QStringLiteral("§1 pen names round-trip"));
    bridge.putPenNames(appSupport, {QStringLiteral("A"), QStringLiteral("A")}, QString());
    check(bridge.lastCallFailed(), QStringLiteral("§1 a repeated pen name is refused"));

    const QString named = scratch + QStringLiteral("/named.scrivi");
    const QVariantMap createdNamed = bridge.createProject(named, appSupport, QStringLiteral("Named"), QStringLiteral("named"),
                                                          QStringLiteral("Jane Doe"));
    const QVariantMap openedNamed = bridge.openProject(named, appSupport);
    check(openedNamed.value(QStringLiteral("authorPrompt")).toString() == QStringLiteral("none"),
          QStringLiteral("§1 a project created under a pen name asks nothing"));
    check(openedNamed.value(QStringLiteral("writingAs")).toMap().value(QStringLiteral("displayName")).toString()
              == QStringLiteral("Jane Doe"), QStringLiteral("§1 … and writes as it"));
    check(bridge.getPenNames(appSupport).value(QStringLiteral("penNames")).toStringList().contains(QStringLiteral("Jane Doe")),
          QStringLiteral("§1 the author's name joined the pen names"));
    bridge.closeProject(named);

    // ── §2 Who wrote…? ─────────────────────────────────────────────────────────────────────────────────────────────────
    const QString old = scratch + QStringLiteral("/old.scrivi");
    const QString oldID = bridge.createProject(old, appSupport, QStringLiteral("Old"), QStringLiteral("old"))
                              .value(QStringLiteral("projectID")).toString();   // credited to "Scrivi", as before SP-211
    const QVariantMap before = bridge.getAuthors(old).value(QStringLiteral("authors")).toList().value(0).toMap();
    check(before.value(QStringLiteral("unnamed")).toBool(), QStringLiteral("§2 an author named Scrivi is unnamed"));
    bridge.closeProject(old);

    AppEnvironment env(appSupport);
    ScriviWindow* wo = openWindow(env, old, oldID);
    auto* question = wo->findChild<AuthorQuestionDialog*>();
    check(question != nullptr && question->isVisible(), QStringLiteral("§2 Who wrote…? is up over the window"));
    if (question != nullptr) {
        QTest::keyClick(question, Qt::Key_Escape);
        settle();
        check(question->isVisible(), QStringLiteral("§2 Esc does not dismiss it"));
        auto fields = question->findChildren<QComboBox*>();
        check(fields.size() == 1, QStringLiteral("§2 one field for the one unnamed author"));
        check(!fields.isEmpty() && fields.first()->currentText() == QStringLiteral("J. D. Quill"),
              QStringLiteral("§2 … filled with the default pen name"));
        if (!fields.isEmpty()) {
            fields.first()->setCurrentText(QStringLiteral("Ann Ink"));
        }
        if (auto* go = question->findChild<QPushButton*>(QStringLiteral("continueButton"))) {
            go->click();
        }
        settle();
    }
    check(wo->findChild<AuthorQuestionDialog*>() == nullptr || !wo->findChild<AuthorQuestionDialog*>()->isVisible(),
          QStringLiteral("§2 the answer closes it"));
    const QVariantMap after = bridge.getAuthors(old).value(QStringLiteral("authors")).toList().value(0).toMap();
    check(after.value(QStringLiteral("displayName")).toString() == QStringLiteral("Ann Ink")
              && after.value(QStringLiteral("personaID")) == before.value(QStringLiteral("personaID")),
          QStringLiteral("§2 the author is renamed, its ID kept"));
    auto* shellBridge = wo->editorShell()->findChild<ScriviBridge*>();
    check(shellBridge != nullptr && shellBridge->displayName() == QStringLiteral("Ann Ink"),
          QStringLiteral("§2 the window writes as the answer"));

    auto* editor = wo->editorShell()->findChild<ManuscriptEditor*>();
    check(editor != nullptr, QStringLiteral("§2 the manuscript is there"));
    if (editor != nullptr) {
        editor->setFocus();
        QTest::keyClicks(editor, QStringLiteral("Typed after naming."));
        wo->editorShell()->saveDirtyScenes();
        settle();
        bridge.waitForWrites(old, 0, 5000);
    }
    const QVariantMap scene = bridge.openProject(old, appSupport).value(QStringLiteral("scenes")).toList().value(0).toMap();
    const QJsonObject by = sidecar(old, scene.value(QStringLiteral("metadataPath")).toString()).value(QStringLiteral("modifiedBy")).toObject();
    check(by.value(QStringLiteral("displayNameAtModification")).toString() == QStringLiteral("Ann Ink")
              && by.value(QStringLiteral("personaID")).toString() == before.value(QStringLiteral("personaID")).toString(),
          QStringLiteral("§2 the scene's edit is recorded as Ann Ink (read from disk: %1)")
              .arg(QString::fromUtf8(QJsonDocument(by).toJson(QJsonDocument::Compact))));

    // ── §5 the menus and Project Settings ▸ Authors (with this window still open) ─────────────────────────────────────
    QAction* appSettings = menuAction(wo, QStringLiteral("Settings…"));
    check(appSettings != nullptr && appSettings->shortcut() == QKeySequence(Qt::CTRL | Qt::Key_Comma),
          QStringLiteral("§5 Edit ▸ Settings… on Ctrl+,"));
    if (appSettings != nullptr) {
        const QList<QAction*> editItems = appSettings->associatedObjects().isEmpty() ? QList<QAction*>()
            : qobject_cast<QMenu*>(appSettings->associatedObjects().first()) != nullptr
                ? qobject_cast<QMenu*>(appSettings->associatedObjects().first())->actions() : QList<QAction*>();
        check(editItems.size() >= 2 && editItems.last() == appSettings && editItems.at(editItems.size() - 2)->isSeparator(),
              QStringLiteral("§5 Settings… is last in Edit, below a rule"));
    }
    LandingWindow landing(nullptr, &env);   // main() builds the app's; this one is the same class
    QAction* landingSettings = landing.findChild<QAction*>(QStringLiteral("appSettingsAction"));   // [SP-216]: its menu bar's
    check(landingSettings != nullptr && landingSettings->isEnabled()
              && landingSettings->shortcut() == QKeySequence(Qt::CTRL | Qt::Key_Comma),
          QStringLiteral("§5 the landing window has Ctrl+, for Settings"));
    QAction* projectSettings = menuAction(wo, QStringLiteral("Project Settings…"));
    check(projectSettings != nullptr && projectSettings->shortcut() == QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Comma),
          QStringLiteral("§5 Project Settings… on Ctrl+Alt+,"));
    if (projectSettings != nullptr && !projectSettings->associatedObjects().isEmpty()) {
        auto* menu = qobject_cast<QMenu*>(projectSettings->associatedObjects().first());
        const QList<QAction*> items = menu != nullptr ? menu->actions() : QList<QAction*>();
        check(items.size() >= 2 && items.last() == projectSettings && items.at(items.size() - 2)->isSeparator(),
              QStringLiteral("§5 Project Settings… is last in Project, below a rule"));
    }
    if (projectSettings != nullptr) {
        bool ran = false;
        QTimer::singleShot(0, wo, [&] {
            auto* d = qobject_cast<ProjectSettingsDialog*>(QApplication::activeModalWidget());
            auto* box = d != nullptr ? d->findChild<AuthorsBox*>() : nullptr;
            check(box != nullptr && box->list()->count() == 1 && box->list()->item(0)->text() == QStringLiteral("Ann Ink"),
                  QStringLiteral("§5 Project Settings shows the Authors"));
            ran = true;
            if (d != nullptr) {
                d->reject();
            }
        });
        projectSettings->trigger();
        check(ran, QStringLiteral("§5 Project Settings was driven"));
    }
    if (appSettings != nullptr) {
        bool ran = false;
        QTimer::singleShot(0, wo, [&] {
            auto* d = qobject_cast<SettingsDialog*>(QApplication::activeModalWidget());
            auto* pane = d != nullptr ? d->findChild<PenNamesPane*>() : nullptr;
            check(pane != nullptr && pane->list()->count() >= 3, QStringLiteral("§5 Settings ▸ Pen Names lists the names"));
            ran = true;
            if (d != nullptr) {
                d->reject();
            }
        });
        appSettings->trigger();
        check(ran, QStringLiteral("§5 Settings was driven"));
    }

    // ── §3 Which of these authors are you? ───────────────────────────────────────────────────────────────────────────
    // Another machine: its identity is not this one. ⚠️ On Linux the identity is not kept in the app-support folder (as on
    // the Mac, where the keychain holds it), so a second folder is the SAME identity — the project's member file is given
    // another device's identity instead, exactly as a project made elsewhere arrives.
    const QString shared = scratch + QStringLiteral("/shared.scrivi");
    const QString sharedID = bridge.createProject(shared, appSupport, QStringLiteral("Shared"), QStringLiteral("shared"),
                                                  QStringLiteral("Sam Roe"))
                                 .value(QStringLiteral("projectID")).toString();
    bridge.closeProject(shared);
    {
        QFile members(shared + QStringLiteral("/identities/project-members.json"));
        check(members.open(QIODevice::ReadOnly), QStringLiteral("§3 the member file is there"));
        QByteArray text = members.readAll();
        members.close();
        text.replace(bridge.identityID().toUtf8(), QByteArrayLiteral("identity_other_device"));
        check(members.open(QIODevice::WriteOnly | QIODevice::Truncate) && members.write(text) == text.size(),
              QStringLiteral("§3 the member file now names another device"));
    }
    check(bridge.getAuthors(shared).value(QStringLiteral("authorPrompt")).toString() == QStringLiteral("chooseAuthor"),
          QStringLiteral("§3 this identity is not yet one of the authors"));
    ScriviWindow* ws = openWindow(env, shared, sharedID);
    auto* choose = ws->findChild<AuthorQuestionDialog*>();
    check(choose != nullptr && choose->isVisible(), QStringLiteral("§3 Which of these authors are you? is up"));
    if (choose != nullptr) {
        for (QPushButton* b : choose->findChildren<QPushButton*>()) {
            if (b->text() == QStringLiteral("Sam Roe")) {
                b->click();
                break;
            }
        }
        settle();
    }
    auto* sharedBridge = ws->editorShell()->findChild<ScriviBridge*>();
    check(sharedBridge != nullptr && sharedBridge->displayName() == QStringLiteral("Sam Roe"),
          QStringLiteral("§3 the window writes as the chosen author"));
    check(bridge.getAuthors(shared).value(QStringLiteral("authorPrompt")).toString() == QStringLiteral("none"),
          QStringLiteral("§3 asked once: this identity is now a member"));

    // ── §4 a world's authors ────────────────────────────────────────────────────────────────────────────────────────
    const QString world = scratch + QStringLiteral("/Eskandar.scriviworld");
    bridge.createWorld(named, world, QStringLiteral("Eskandar"), QString());
    const QVariantMap seeded = bridge.getAuthors(world, named);
    check(authorNames(bridge, world) == QStringList{QStringLiteral("Jane Doe")},
          QStringLiteral("§4 a world's first read takes the project's author"));
    bridge.setAuthors(world, {QVariantMap{{QStringLiteral("displayName"), QStringLiteral("Sam Roe")}},
                              QVariantMap{{QStringLiteral("personaID"), seeded.value(QStringLiteral("authors")).toList().value(0)
                                                                            .toMap().value(QStringLiteral("personaID"))},
                                          {QStringLiteral("displayName"), QStringLiteral("Jane Doe")}}});
    check(authorNames(bridge, world) == QStringList({QStringLiteral("Sam Roe"), QStringLiteral("Jane Doe")}),
          QStringLiteral("§4 … then keeps its own list"));
    check(authorNames(bridge, named) == QStringList{QStringLiteral("Jane Doe")}, QStringLiteral("§4 the project's list is its own"));

    if (failures == 0) {
        std::printf("authorship-ok\n");
    }
    std::fflush(stdout);
    std::_Exit(failures == 0 ? 0 : 1);
}
