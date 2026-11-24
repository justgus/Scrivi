// toolbar_smoke.cpp — [SP-232] (EP-059 F1, T-0694): every Linux project window has the Mac's toolbar, and every item on it is
// the menus' own action.
//
// A REAL project window through `AppEnvironment`, and the Landing window.
//
// ✅ What it proves:
//   §1 the toolbar is on the project window, with the Mac's items in its order (EP-060 SP-243: 8, View leading), the word
//      under each ([SP-232] Q2);
//   §2 each button IS its menu item's action; each drop-down holds the menus' own actions; the Q3 names in the menus;
//   §3 a toggle on the toolbar changes the pane and the menu's check; a Navigate item moves the caret;
//   §4 a narrow window moves items behind the overflow button; View ▸ Show Toolbar hides and shows it (Q5);
//   §5 the Landing window has none (its Show Toolbar greyed); Close Project takes the toolbar with the project.
//
//   argv[1] = a scratch directory; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "LandingWindow.hpp"
#include "ManuscriptEditor.hpp"
#include "ManuscriptTypography.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QProxyStyle>
#include <QTest>
#include <QTextCursor>
#include <QToolBar>
#include <QToolButton>

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

// The menu bar's action with this text (submenus searched), or nullptr.
QAction* menuAction(QMenuBar* bar, const QString& text)
{
    std::function<QAction*(const QList<QAction*>&)> find = [&](const QList<QAction*>& actions) -> QAction* {
        for (QAction* a : actions) {
            if (a->text() == text) { return a; }
            if (a->menu() != nullptr) {
                if (QAction* inner = find(a->menu()->actions())) { return inner; }
            }
        }
        return nullptr;
    };
    return find(bar->actions());
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: toolbar_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const QString dir = scratch + QStringLiteral("/toolbar.inkwyn");
    const QString projectID = bridge.createProject(dir, appSupport, QStringLiteral("Toolbar"), QStringLiteral("toolbar"))
                                  .value(QStringLiteral("projectID")).toString();
    bridge.closeProject(dir);

    AppEnvironment env(appSupport);
    QPointer<ScriviWindow> w = env.openProjectWindow(dir, QStringLiteral("Toolbar"), {}, projectID);
    for (int i = 0; i < 3000 && env.windows().window(projectID) != w; ++i) {
        QTest::qWait(10);
    }
    check(env.windows().window(projectID) == w, QStringLiteral("project window loaded"));
    w->resize(1400, 700);
    w->raiseToFront();
    QTest::qWait(200);
    EditorShell* shell = w->editorShell();
    QMenuBar* menus = w->menuBar();

    // ── §1 the items, in the Mac's order ──────────────────────────────────────────────────────────────────────────────
    auto* bar = w->findChild<QToolBar*>(QStringLiteral("projectToolbar"));
    check(bar != nullptr && bar->isVisible(), QStringLiteral("§1 the project window has its toolbar"));
    if (bar == nullptr) {
        std::printf("%d failure(s)\n", failures);
        return 1;
    }
    check(bar->toolButtonStyle() == Qt::ToolButtonTextUnderIcon && !bar->isMovable(), QStringLiteral("§1 words under icons, fixed"));
    auto word = [bar](QAction* a) {
        auto* button = qobject_cast<QToolButton*>(bar->widgetForAction(a));
        return button != nullptr && button->menu() != nullptr ? button->text() : a->iconText();
    };
    QStringList words;
    for (QAction* a : bar->actions()) { words << word(a); }
    // EP-060 SP-243: the Mac's [SP-242] shape — 8 items, View leading.
    const QStringList expected = {QStringLiteral("View"), QStringLiteral("Worlds"), QStringLiteral("Navigate"),
                                  QStringLiteral("New"), QStringLiteral("Merge"), QStringLiteral("Import/Export"),
                                  QStringLiteral("Tools"), QStringLiteral("Close Project")};
    check(words == expected, QStringLiteral("§1 the items: %1").arg(words.join(QStringLiteral(" | "))));

    // ── §2 the menus' own actions ──────────────────────────────────────────────────────────────────────────────────────
    const QList<QAction*> items = bar->actions();
    if (items.size() == expected.size()) {
        check(items.at(1) == menuAction(menus, QStringLiteral("Manage Worlds…"))
                  && items.at(7) == menuAction(menus, QStringLiteral("Close Project")),
              QStringLiteral("§2 Worlds and Close Project are the menus' own actions"));
    }
    auto dropDown = [bar](const char* name) {
        auto* button = bar->findChild<QToolButton*>(QString::fromLatin1(name));
        return button != nullptr ? button->menu() : nullptr;
    };
    auto holds = [&](const char* name, const QStringList& texts) {
        QMenu* m = dropDown(name);
        bool all = m != nullptr;
        for (const QString& t : texts) {
            all = all && m->actions().contains(menuAction(menus, t));
        }
        check(all, QStringLiteral("§2 %1 holds the menus' %2").arg(QString::fromLatin1(name), texts.join(QStringLiteral(", "))));
    };
    holds("toolbarView", {QStringLiteral("Show Scene Navigator"), QStringLiteral("Show Inspector"), QStringLiteral("Show Timeline"),
                          QStringLiteral("Show World Warnings"), QStringLiteral("Show Markup Hints"), QStringLiteral("Show Buffers")});
    holds("toolbarNavigate", {QStringLiteral("Go to Scene Start"), QStringLiteral("Go to Scene End"),
                              QStringLiteral("Go to Chapter Start"), QStringLiteral("Go to Chapter End"),
                              QStringLiteral("Go to Manuscript Start"), QStringLiteral("Go to Manuscript End")});
    holds("toolbarNew", {QStringLiteral("New Scene"), QStringLiteral("New Chapter")});
    holds("toolbarMerge", {QStringLiteral("Merge Scene with Previous"), QStringLiteral("Merge Chapter with Previous")});
    holds("toolbarTools", {QStringLiteral("Project Settings…"), QStringLiteral("Settings…"),
                           QStringLiteral("Purge Stale History Branches…"), QStringLiteral("About Inkwyn")});
    // Each drop-down opens with its header (its own word), then a rule; no Qt drop-down arrow; a glyph on every button.
    for (const char* name : {"toolbarView", "toolbarNavigate", "toolbarNew", "toolbarMerge", "toolbarImportExport", "toolbarTools"}) {
        auto* button = bar->findChild<QToolButton*>(QString::fromLatin1(name));
        QMenu* m = button != nullptr ? button->menu() : nullptr;
        const QList<QAction*> acts = m != nullptr ? m->actions() : QList<QAction*>();
        check(acts.size() > 2 && acts.at(0)->text() == button->text() && !acts.at(0)->icon().isNull() && acts.at(1)->isSeparator(),
              QStringLiteral("§2 %1 opens with its header and a rule").arg(QString::fromLatin1(name)));
        check(button != nullptr && !button->icon().isNull() && button->styleSheet().contains(QStringLiteral("menu-indicator")),
              QStringLiteral("§2 %1 has its glyph and no Qt arrow").arg(QString::fromLatin1(name)));
    }
    check(!items.value(1)->icon().isNull() && !items.value(7)->icon().isNull(), QStringLiteral("§2 Worlds and Close have glyphs"));
    // Import/Export: an Import submenu with File ▸ Import's items, an Export submenu with File ▸ Export's and Copy as HTML.
    QMenu* io = dropDown("toolbarImportExport");
    QMenu* importSub = nullptr;
    QMenu* exportSub = nullptr;
    for (QAction* a : io != nullptr ? io->actions() : QList<QAction*>()) {
        if (a->menu() != nullptr && a->menu()->title() == QStringLiteral("Import")) { importSub = a->menu(); }
        if (a->menu() != nullptr && a->menu()->title() == QStringLiteral("Export")) { exportSub = a->menu(); }
    }
    check(importSub != nullptr && importSub->actions().contains(menuAction(menus, QStringLiteral("Manuscript…")))
              && !importSub->icon().isNull(),
          QStringLiteral("§2 Import/Export ▸ Import holds File ▸ Import's items, with its glyph"));
    check(exportSub != nullptr && exportSub->actions().contains(menuAction(menus, QStringLiteral("Copy as HTML")))
              && !exportSub->icon().isNull(),
          QStringLiteral("§2 Import/Export ▸ Export holds Copy as HTML, with its glyph"));
    check(menuAction(menus, QStringLiteral("Split Scene")) == nullptr && menuAction(menus, QStringLiteral("Merge Scene")) == nullptr,
          QStringLiteral("§2 Q3: the Linux names are gone from the menus"));

    // ── §3 they act ────────────────────────────────────────────────────────────────────────────────────────────────────
    QAction* inspectorMenu = menuAction(menus, QStringLiteral("Show Inspector"));
    const bool before = shell->isInspectorVisible();
    inspectorMenu->trigger();   // the View drop-down's item IS this action (§2)
    check(shell->isInspectorVisible() != before && inspectorMenu->isChecked() == shell->isInspectorVisible(),
          QStringLiteral("§3 View ▸ Show Inspector toggles the pane and the menu's check"));
    inspectorMenu->trigger();
    auto* editor = shell->findChild<ManuscriptEditor*>();
    QTextCursor c(editor->document());
    c.movePosition(QTextCursor::End);
    editor->setTextCursor(c);
    QTest::keyClicks(editor, QStringLiteral("Some words."));   // a new project's scene is empty: start and end are one place
    const int end = editor->textCursor().position();
    menuAction(menus, QStringLiteral("Go to Manuscript Start"))->trigger();   // reached through Navigate's drop-down
    check(dropDown("toolbarNavigate")->actions().contains(menuAction(menus, QStringLiteral("Go to Manuscript Start")))
              && editor->textCursor().position() < end,
          QStringLiteral("§3 Navigate ▸ Manuscript Start moves the caret (%1 → %2)").arg(end).arg(editor->textCursor().position()));

    // ── §4 overflow; View ▸ Show Toolbar ──────────────────────────────────────────────────────────────────────────────
    w->resize(420, 700);
    QTest::qWait(100);
    auto* overflow = bar->findChild<QToolButton*>(QStringLiteral("qt_toolbar_ext_button"));
    check(overflow != nullptr && overflow->isVisible(), QStringLiteral("§4 a narrow window: the overflow button"));
    // The live pass: the theme's icon drew nothing. Ours must put ink on the button.
    bool inked = false;
    if (overflow != nullptr) {
        const QImage img = overflow->icon().pixmap(32, 32).toImage();
        for (int y = 0; y < img.height() && !inked; ++y) {
            for (int x = 0; x < img.width() && !inked; ++x) { inked = qAlpha(img.pixel(x, y)) > 0; }
        }
    }
    check(inked && overflow != nullptr && qobject_cast<QProxyStyle*>(overflow->style()) != nullptr,
          QStringLiteral("§4 the overflow button draws the app's own glyph (menu.expansion), never blank"));
    w->resize(1400, 700);
    QTest::qWait(50);
    auto* showToolbar = w->findChild<QAction*>(QStringLiteral("showToolbarAction"));
    check(showToolbar != nullptr && showToolbar->isChecked(), QStringLiteral("§4 View ▸ Show Toolbar is on"));
    if (showToolbar != nullptr) {
        showToolbar->trigger();
        check(!bar->isVisible() && !showToolbar->isChecked(), QStringLiteral("§4 unchecked: no toolbar"));
        showToolbar->trigger();
        check(bar->isVisible(), QStringLiteral("§4 checked: the toolbar again"));
    }

    // ── §5 Landing; Close Project ─────────────────────────────────────────────────────────────────────────────────────
    LandingWindow landing(nullptr, &env);
    auto* landingShowToolbar = landing.findChild<QAction*>(QStringLiteral("showToolbarAction"));
    check(landing.findChild<QToolBar*>() == nullptr && landingShowToolbar != nullptr && !landingShowToolbar->isEnabled(),
          QStringLiteral("§5 the Landing window has no toolbar, and Show Toolbar greyed ([SP-216]: one menu bar)"));
    QPointer<QToolBar> guard(bar);
    items.at(7)->trigger();   // Close Project
    for (int i = 0; i < 300 && !guard.isNull() && guard->isVisible(); ++i) {
        QTest::qWait(10);
    }
    check(guard.isNull() || !guard->isVisible(), QStringLiteral("§5 Close Project takes the toolbar with the project"));

    if (failures == 0) {
        std::printf("toolbar-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
