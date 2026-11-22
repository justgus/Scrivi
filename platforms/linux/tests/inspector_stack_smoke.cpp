// inspector_stack_smoke.cpp — [SP-233] (EP-059 B5, B6, A7; T-0695, T-0491): the Scene Inspector's Writing and Worldbuilding
// tabs are the Mac's card stacks, read from and written to `inspector-layout.json`.
//
// A REAL project window through `AppEnvironment`, with a world and objects linked to the first scene, and a layout written
// beforehand as the Mac writes it.
//
// ✅ What it proves:
//   §1 the scene's Writing cards come from the layout, in its order; a card this build lacks is reported and removable;
//      Worldbuilding with no cards says so (Q3);
//   §2 + offers one Worldbuilding card per world-scoped kind, DERIVED from the core; adding one shows the scene's objects of
//      that kind, labelled; a present card is greyed in +; an empty card says the scene has none;
//   §3 the stack's sort orders the cards and each card's rows (A7, Q2);
//   §4 collapse and ✕ persist;
//   §5 the History card's collapse drives the History section;
//   §6 Apply Layout to All Scenes… writes every scene and the default; a second scene has its own cards.
//
//   argv[1] = a scratch directory; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "HistorySection.hpp"
#include "ManuscriptTypography.hpp"
#include "ObjectKindScope.hpp"
#include "SceneInspector.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QTest>
#include <QToolButton>
#include <QTreeWidget>

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

bool waitFor(const std::function<bool()>& done, int ms = 5000)
{
    for (int i = 0; i < ms / 10; ++i) {
        if (done()) { return true; }
        QTest::qWait(10);
    }
    return done();
}

// The stack's cards, top to bottom, by typeID.
QStringList cardTypes(SceneInspector* inspector, const QString& stack)
{
    QApplication::processEvents();   // a card added to a shown layout is shown on the next pass of the event loop
    QStringList out;
    auto* host = inspector->findChild<QWidget*>(stack + QStringLiteral("Cards"));
    if (host == nullptr) { return {QStringLiteral("<no host>")}; }
    QList<QFrame*> frames;
    for (QFrame* f : host->findChildren<QFrame*>(QString(), Qt::FindDirectChildrenOnly)) {
        if (f->objectName().startsWith(QStringLiteral("card:")) && f->isVisibleTo(host)) { frames << f; }
    }
    std::sort(frames.begin(), frames.end(), [](QFrame* a, QFrame* b) { return a->y() < b->y(); });
    for (QFrame* f : frames) { out << f->objectName().mid(5); }
    return out;
}

QFrame* card(SceneInspector* inspector, const QString& stack, const QString& type)
{
    QApplication::processEvents();
    auto* host = inspector->findChild<QWidget*>(stack + QStringLiteral("Cards"));
    for (QFrame* f : host->findChildren<QFrame*>(QStringLiteral("card:") + type, Qt::FindDirectChildrenOnly)) {
        if (f->isVisibleTo(host)) { return f; }
    }
    return nullptr;
}

// The + or ⋯ menu as the writer opens it, then its action by text.
QAction* menuItem(SceneInspector* inspector, const QString& buttonName, const QString& text)
{
    auto* button = inspector->findChild<QToolButton*>(buttonName);
    if (button == nullptr || button->menu() == nullptr) { return nullptr; }
    emit button->menu()->aboutToShow();
    for (QAction* a : button->menu()->actions()) {
        if (a->text() == text) { return a; }
    }
    return nullptr;
}

QStringList rows(QFrame* f)
{
    QStringList out;
    auto* tree = f == nullptr ? nullptr : f->findChild<QTreeWidget*>(QStringLiteral("objectRows"));
    for (int i = 0; tree != nullptr && i < tree->topLevelItemCount(); ++i) { out << tree->topLevelItem(i)->text(0); }
    return out;
}

const QJsonObject layoutOnDisk(ScriviBridge& bridge, const QString& dir)
{
    return QJsonObject::fromVariantMap(bridge.getInspectorLayout(dir).value(QStringLiteral("document")).toMap());
}

QStringList typesIn(const QJsonValue& array)
{
    QStringList out;
    for (const QJsonValue& v : array.toArray()) { out << v.toObject().value(QStringLiteral("type")).toString(); }
    return out;
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: inspector_stack_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const QString dir = scratch + QStringLiteral("/stack.inkwyn");
    const QString projectID = bridge.createProject(dir, appSupport, QStringLiteral("Stack"), QStringLiteral("stack"))
                                  .value(QStringLiteral("projectID")).toString();
    const QVariantMap first = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString sceneID = first.value(QStringLiteral("sceneID")).toString();
    const QString sceneB = bridge.createScene(dir, appSupport, projectID, first.value(QStringLiteral("chapterID")).toString(),
                                              sceneID).value(QStringLiteral("sceneID")).toString();
    check(!sceneB.isEmpty(), QStringLiteral("fixture: a second scene"));

    // A world, two characters and a location linked to the first scene. The kinds are the CORE's: this file names none.
    const QString worldID = bridge.createWorld(dir, scratch + QStringLiteral("/W.inkwynworld"), QStringLiteral("W"),
                                               QStringLiteral("AE")).value(QStringLiteral("worldID")).toString();
    QList<ObjectKindScope::KindInfo> worldKinds;
    for (const auto& k : ObjectKindScope::kinds(&bridge)) {
        if (k.isWorldScoped) { worldKinds << k; }
    }
    check(worldKinds.size() >= 3 && !worldID.isEmpty(), QStringLiteral("fixture: a world and at least three world kinds"));
    const ObjectKindScope::KindInfo kA = worldKinds.at(0);   // two objects
    const ObjectKindScope::KindInfo kB = worldKinds.at(1);   // one object
    const ObjectKindScope::KindInfo kC = worldKinds.at(2);   // none
    auto link = [&](const QString& kind, const QString& name) {
        const QString id = bridge.createObject(dir, kind, name, QString(), worldID).value(QStringLiteral("objectID")).toString();
        bridge.createEdge(dir, id, sceneID, QStringLiteral("cites"), QString());
    };
    link(kA.kind, QStringLiteral("Abel"));   // added first …
    link(kA.kind, QStringLiteral("Zed"));    // … so Recently Added puts Zed first
    link(kB.kind, QStringLiteral("Harbour"));
    auto title = [](const QString& subdir) { QString t = subdir; t[0] = t[0].toUpper(); return t; };
    const QString tA = QStringLiteral("objects.") + kA.subdir;
    const QString tB = QStringLiteral("objects.") + kB.subdir;
    const QString tC = QStringLiteral("objects.") + kC.subdir;

    // The layout as the Mac writes it: this scene's Writing stack with a card Linux lacks, and a key Linux does not model.
    QJsonObject entryTags{{QStringLiteral("type"), QStringLiteral("tags")}, {QStringLiteral("collapsed"), false},
                          {QStringLiteral("futureEntryKey"), 7}};
    const QJsonObject doc{
        {QStringLiteral("schema"), QStringLiteral("scrivi.inspector-layout.v1")},
        {QStringLiteral("selectedTab"), QStringLiteral("worldbuilding")},
        {QStringLiteral("inspectorHidden"), false},
        {QStringLiteral("stackSort"), QJsonObject{{QStringLiteral("writing"), QStringLiteral("manual")},
                                                  {QStringLiteral("worldbuilding"), QStringLiteral("manual")}}},
        {QStringLiteral("defaultStacks"), QJsonObject{{QStringLiteral("worldbuilding"), QJsonArray{}},
                                                      {QStringLiteral("writing"), QJsonArray{entryTags}}}},
        {QStringLiteral("scenes"), QJsonObject{{sceneID, QJsonObject{
            {QStringLiteral("writing"), QJsonArray{entryTags,
                                                   QJsonObject{{QStringLiteral("type"), QStringLiteral("todo")}},
                                                   QJsonObject{{QStringLiteral("type"), QStringLiteral("sources")}}}},
            {QStringLiteral("worldbuilding"), QJsonArray{}}}}}},
        {QStringLiteral("futureKeyFromANewerScrivi"), true}};
    bridge.putInspectorLayout(dir, QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Compact)));
    bridge.closeProject(dir);

    AppEnvironment env(appSupport);
    QPointer<ScriviWindow> w = env.openProjectWindow(dir, QStringLiteral("Stack"), {}, projectID);
    check(waitFor([&] { return env.windows().window(projectID) == w; }, 30000), QStringLiteral("project window loaded"));
    w->resize(1400, 800);
    QTest::qWait(200);
    auto* inspector = w->findChild<SceneInspector*>();
    check(inspector != nullptr, QStringLiteral("the window has a Scene Inspector"));
    if (inspector == nullptr) { return 1; }
    inspector->setScene(sceneID);
    auto* worldHost = inspector->findChild<QWidget*>(QStringLiteral("worldbuildingCards"));
    check(waitFor([&] { return worldHost != nullptr && worldHost->isEnabled(); }), QStringLiteral("the scene's read landed"));
    QTest::qWait(100);

    // ── §1 the layout's cards ─────────────────────────────────────────────────────────────────────────────────────────
    check(cardTypes(inspector, QStringLiteral("writing")) == QStringList{"tags", "todo", "sources"},
          QStringLiteral("§1 Writing shows the layout's cards in order: %1").arg(cardTypes(inspector, "writing").join(", ")));
    QFrame* unknown = card(inspector, QStringLiteral("writing"), QStringLiteral("sources"));
    bool explained = false;
    for (QLabel* l : unknown != nullptr ? unknown->findChildren<QLabel*>() : QList<QLabel*>{}) {
        explained = explained || l->text() == QStringLiteral("This card isn't available in this version.");
    }
    check(explained, QStringLiteral("§1 a card this build lacks is reported, not dropped"));
    if (unknown != nullptr) { unknown->findChild<QToolButton*>(QStringLiteral("cardRemove"))->click(); }
    QTest::qWait(50);
    check(cardTypes(inspector, QStringLiteral("writing")) == QStringList{"tags", "todo"}, QStringLiteral("§1 ✕ removes it"));
    check(cardTypes(inspector, QStringLiteral("worldbuilding")).isEmpty(), QStringLiteral("§1 Worldbuilding has no cards (Q3)"));
    bool emptySaid = false;
    for (QLabel* l : worldHost->findChildren<QLabel*>()) {
        emptySaid = emptySaid || (l->isVisibleTo(worldHost) && l->text() == QStringLiteral("No cards yet. Use + to add one."));
    }
    check(emptySaid, QStringLiteral("§1 … and says so"));

    // ── §2 + derived from the core; a kind's card shows the scene's objects ───────────────────────────────────────────
    int offered = 0;
    for (const auto& k : worldKinds) {
        offered += menuItem(inspector, QStringLiteral("worldbuildingAddCard"), title(k.subdir)) != nullptr ? 1 : 0;
    }
    check(offered == worldKinds.size(), QStringLiteral("§2 + offers every world-scoped kind the core lists (%1 of %2)")
                                            .arg(offered).arg(worldKinds.size()));
    QAction* addA = menuItem(inspector, QStringLiteral("worldbuildingAddCard"), title(kA.subdir));
    check(addA != nullptr && addA->isEnabled(), QStringLiteral("§2 + ▸ %1").arg(title(kA.subdir)));
    if (addA != nullptr) { addA->trigger(); }
    menuItem(inspector, QStringLiteral("worldbuildingAddCard"), title(kB.subdir))->trigger();
    menuItem(inspector, QStringLiteral("worldbuildingAddCard"), title(kC.subdir))->trigger();
    QTest::qWait(50);
    check(cardTypes(inspector, QStringLiteral("worldbuilding")) == QStringList{tA, tB, tC},
          QStringLiteral("§2 the cards, in the order added: %1").arg(cardTypes(inspector, "worldbuilding").join(", ")));
    const QStringList rowsA = rows(card(inspector, QStringLiteral("worldbuilding"), tA));
    check(rowsA.size() == 2 && rowsA.at(0).startsWith(QStringLiteral("Abel — ")) && rowsA.at(1).startsWith(QStringLiteral("Zed — ")),
          QStringLiteral("§2 the card lists the scene's objects of its kind, labelled, in manual order: %1").arg(rowsA.join(" | ")));
    check(rows(card(inspector, QStringLiteral("worldbuilding"), tB)).size() == 1, QStringLiteral("§2 one object in the second card"));
    bool noneSaid = false;
    for (QLabel* l : card(inspector, QStringLiteral("worldbuilding"), tC)->findChildren<QLabel*>()) {
        noneSaid = noneSaid || l->text() == QStringLiteral("No %1 in this scene yet.").arg(kC.subdir);
    }
    check(noneSaid, QStringLiteral("§2 an empty card says the scene has none"));
    QAction* again = menuItem(inspector, QStringLiteral("worldbuildingAddCard"), title(kA.subdir));
    check(again != nullptr && !again->isEnabled(), QStringLiteral("§2 a present card is greyed in +"));
    QJsonObject disk;   // the store writes off the GUI thread: wait for it
    check(waitFor([&] {
              disk = layoutOnDisk(bridge, dir);
              return typesIn(disk.value("scenes")[sceneID][QStringLiteral("worldbuilding")]) == QStringList{tA, tB, tC};
          }),
          QStringLiteral("§2 the scene's Worldbuilding stack is written"));
    check(typesIn(disk.value("scenes")[sceneID][QStringLiteral("writing")]) == QStringList{"tags", "todo"},
          QStringLiteral("§2 the Writing stack beside it is kept"));
    check(disk.value("scenes")[sceneID]["writing"].toArray().at(0).toObject().value("futureEntryKey").toInt() == 7,
          QStringLiteral("§2 a key Linux does not model, inside a card entry, is kept"));
    check(disk.value("futureKeyFromANewerScrivi").toBool(), QStringLiteral("§2 a top-level key Linux does not model is kept"));

    // ── §3 the stack's sort: cards and rows ──────────────────────────────────────────────────────────────────────────
    menuItem(inspector, QStringLiteral("worldbuildingStackOptions"), QStringLiteral("Name"))->trigger();
    QStringList byName = {tA, tB, tC};
    std::sort(byName.begin(), byName.end(), [&](const QString& a, const QString& b) {
        return title(a.mid(8)).localeAwareCompare(title(b.mid(8))) < 0;
    });
    check(cardTypes(inspector, QStringLiteral("worldbuilding")) == byName, QStringLiteral("§3 Name orders the cards"));
    check(rows(card(inspector, QStringLiteral("worldbuilding"), tA)).value(0).startsWith(QStringLiteral("Abel")),
          QStringLiteral("§3 … and each card's rows"));
    QAction* nameItem = menuItem(inspector, QStringLiteral("worldbuildingStackOptions"), QStringLiteral("Name"));
    check(nameItem != nullptr && nameItem->isChecked(), QStringLiteral("§3 the menu shows the sort chosen"));
    menuItem(inspector, QStringLiteral("worldbuildingStackOptions"), QStringLiteral("Recently Added"))->trigger();
    check(cardTypes(inspector, QStringLiteral("worldbuilding")) == QStringList{tC, tB, tA}, QStringLiteral("§3 Recently Added"));
    check(rows(card(inspector, QStringLiteral("worldbuilding"), tA)).value(0).startsWith(QStringLiteral("Zed")),
          QStringLiteral("§3 … newest row first"));
    check(waitFor([&] { return layoutOnDisk(bridge, dir)["stackSort"]["worldbuilding"].toString() == "recentlyAdded"; }),
          QStringLiteral("§3 the sort is written per stack"));
    menuItem(inspector, QStringLiteral("worldbuildingStackOptions"), QStringLiteral("Manual"))->trigger();
    check(cardTypes(inspector, QStringLiteral("worldbuilding")) == QStringList{tA, tB, tC}, QStringLiteral("§3 Manual"));

    // ── §4 collapse and ✕ persist ────────────────────────────────────────────────────────────────────────────────────
    card(inspector, QStringLiteral("worldbuilding"), tA)->findChild<QToolButton*>(QStringLiteral("cardCollapse"))->click();
    QTest::qWait(50);
    auto* rowsWidget = card(inspector, QStringLiteral("worldbuilding"), tA)->findChild<QTreeWidget*>(QStringLiteral("objectRows"));
    check(rowsWidget != nullptr && !rowsWidget->isVisibleTo(card(inspector, "worldbuilding", tA)),
          QStringLiteral("§4 a collapsed card hides its body"));
    check(waitFor([&] {
              return layoutOnDisk(bridge, dir)["scenes"][sceneID]["worldbuilding"].toArray().at(0)["collapsed"].toBool();
          }),
          QStringLiteral("§4 collapse is written"));
    card(inspector, QStringLiteral("worldbuilding"), tC)->findChild<QToolButton*>(QStringLiteral("cardRemove"))->click();
    QTest::qWait(50);
    check(cardTypes(inspector, QStringLiteral("worldbuilding")) == QStringList{tA, tB}, QStringLiteral("§4 ✕ removes a card"));

    // ── §5 the History card drives the History section ───────────────────────────────────────────────────────────────
    HistorySection* history = inspector->historySection();
    check(!history->isExpanded(), QStringLiteral("§5 History is closed with no History card (Q1)"));
    menuItem(inspector, QStringLiteral("writingAddCard"), QStringLiteral("History"))->trigger();
    QTest::qWait(50);
    check(cardTypes(inspector, QStringLiteral("writing")) == QStringList{"tags", "todo", "history"} && history->isExpanded(),
          QStringLiteral("§5 the History card, added open, opens the section"));
    card(inspector, QStringLiteral("writing"), QStringLiteral("history"))->findChild<QToolButton*>("cardCollapse")->click();
    QTest::qWait(50);
    check(!history->isExpanded(), QStringLiteral("§5 collapsing the card closes it"));

    // ── §6 a second scene has its own cards; Apply Layout to All Scenes… ─────────────────────────────────────────────
    inspector->setScene(sceneB);
    check(waitFor([&] { return worldHost->isEnabled() && cardTypes(inspector, QStringLiteral("worldbuilding")).isEmpty(); }),
          QStringLiteral("§6 the second scene has the default (empty) Worldbuilding stack"));
    check(cardTypes(inspector, QStringLiteral("writing")) == QStringList{"tags"},
          QStringLiteral("§6 … and the default Writing stack: %1").arg(cardTypes(inspector, "writing").join(", ")));
    inspector->setScene(sceneID);
    check(waitFor([&] { return worldHost->isEnabled() && cardTypes(inspector, QStringLiteral("worldbuilding")).size() == 2; }),
          QStringLiteral("§6 back on the first scene, its cards"));
    QAction* all = menuItem(inspector, QStringLiteral("worldbuildingStackOptions"), QStringLiteral("Apply Layout to All Scenes…"));
    check(all != nullptr && all->isEnabled(), QStringLiteral("§6 Apply Layout to All Scenes… is offered"));
    if (all != nullptr) { all->trigger(); }
    check(waitFor([&] {
              const QJsonObject d = layoutOnDisk(bridge, dir);
              return typesIn(d["scenes"][sceneB]["worldbuilding"]) == QStringList{tA, tB}
                  && typesIn(d["defaultStacks"]["worldbuilding"]) == QStringList{tA, tB};
          }),
          QStringLiteral("§6 every scene and the default have the stack"));
    inspector->setScene(sceneB);
    check(waitFor([&] { return worldHost->isEnabled() && cardTypes(inspector, QStringLiteral("worldbuilding")).size() == 2; }),
          QStringLiteral("§6 the second scene shows it"));

    w->close();
    QTest::qWait(100);
    if (failures == 0) { std::printf("inspector-stack-ok\n"); }
    std::printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
