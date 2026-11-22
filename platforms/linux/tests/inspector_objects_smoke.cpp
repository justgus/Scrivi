// inspector_objects_smoke.cpp — [SP-234] (EP-059 A3–A6; T-0696): the object verbs on a kind's card, as the Mac's
// (`ObjectCard.swift`, `ObjectPickerView.swift`).
//
// A REAL project window through `AppEnvironment`, with a world, objects of the core's kinds, and a layout that shows two
// kinds' cards on two scenes.
//
// ✅ What it proves:
//   §1 Add <Kind>… lists every object of the kind across worlds, grouped, searchable; the scene's are ticked and disabled;
//      a pick links it (one edge);
//   §2 New creates in place with the world threaded and links it; nothing is written before Create; the relation type is
//      DERIVED from the vocabulary (Q5) — the kind-specific one where the vocabulary has it, else the general one;
//   §3 Rename (F2, Q2) patches displayName and keeps every other field;
//   §4 Remove from Scene (⊖) deletes the edge only;
//   §5 a draft left at a scene boundary asks (Q3): Keep Editing names the origin and Create files it THERE; Discard writes
//      nothing; an untouched draft just closes;
//   §6 a row shows its object's image (A6, Q4).
//
//   argv[1] = a scratch directory; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ManuscriptTypography.hpp"
#include "ObjectKindScope.hpp"
#include "SceneInspector.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFrame>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
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

bool waitFor(const std::function<bool()>& done, int ms = 8000)
{
    for (int i = 0; i < ms / 10; ++i) {
        if (done()) { return true; }
        QTest::qWait(10);
    }
    return done();
}

QFrame* card(SceneInspector* inspector, const QString& type)
{
    QApplication::processEvents();
    auto* host = inspector->findChild<QWidget*>(QStringLiteral("worldbuildingCards"));
    for (QFrame* f : host->findChildren<QFrame*>(QStringLiteral("card:") + type, Qt::FindDirectChildrenOnly)) {
        if (f->isVisibleTo(host)) { return f; }
    }
    return nullptr;
}

template <typename T>
T* in(SceneInspector* inspector, const QString& type, const QString& name)
{
    QFrame* f = card(inspector, type);
    return f == nullptr ? nullptr : f->findChild<T*>(name);
}

QTreeWidgetItem* row(SceneInspector* inspector, const QString& type, const QString& startsWith)
{
    auto* tree = in<QTreeWidget>(inspector, type, QStringLiteral("objectRows"));
    for (int i = 0; tree != nullptr && i < tree->topLevelItemCount(); ++i) {
        if (tree->topLevelItem(i)->text(0).startsWith(startsWith)) { return tree->topLevelItem(i); }
    }
    return nullptr;
}

// The scene's edges to an object named `name`, by relation type.
QStringList edgeTypes(ScriviBridge& bridge, const QString& dir, const QString& sceneID, const QString& name)
{
    QStringList out;
    for (const QVariant& e : bridge.listEdgesFor(dir, sceneID).value(QStringLiteral("edges")).toList()) {
        const QVariantMap m = e.toMap();
        if (m.value(QStringLiteral("otherDisplayName")).toString() == name) {
            out << m.value(QStringLiteral("relationType")).toString();
        }
    }
    return out;
}

QVariantMap objectNamed(ScriviBridge& bridge, const QString& dir, const QString& name)
{
    for (const QVariant& o : bridge.listObjects(dir, QString()).value(QStringLiteral("objects")).toList()) {
        if (o.toMap().value(QStringLiteral("displayName")).toString() == name) { return o.toMap(); }
    }
    return {};
}

QMessageBox* prompt(SceneInspector* inspector)
{
    QMessageBox* box = nullptr;
    waitFor([&] {
        box = inspector->findChild<QMessageBox*>(QStringLiteral("unfinishedDraftPrompt"));
        return box != nullptr && box->isVisible();
    }, 3000);
    return box;
}

void clickButton(QMessageBox* box, const QString& text)
{
    for (QAbstractButton* b : box != nullptr ? box->buttons() : QList<QAbstractButton*>{}) {
        if (b->text() == text) { b->click(); return; }
    }
    check(false, QStringLiteral("the prompt has “%1”").arg(text));
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: inspector_objects_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const QString dir = scratch + QStringLiteral("/objects.inkwyn");
    const QString projectID = bridge.createProject(dir, appSupport, QStringLiteral("Objects"), QStringLiteral("objects"))
                                  .value(QStringLiteral("projectID")).toString();
    const QVariantMap first = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString sceneA = first.value(QStringLiteral("sceneID")).toString();
    const QString sceneB = bridge.createScene(dir, appSupport, projectID, first.value(QStringLiteral("chapterID")).toString(),
                                              sceneA).value(QStringLiteral("sceneID")).toString();
    const QString worldID = bridge.createWorld(dir, scratch + QStringLiteral("/W.inkwynworld"), QStringLiteral("Westmarch"),
                                               QStringLiteral("AE")).value(QStringLiteral("worldID")).toString();
    check(!sceneB.isEmpty() && !worldID.isEmpty(), QStringLiteral("fixture: two scenes and a world"));

    // The vocabulary is the oracle (Q5): a kind the vocabulary names with a scene endpoint, and the general type.
    QString general;
    QHash<QString, QString> specificByKind;
    for (const QVariant& t : bridge.listRelationTypes(dir).value(QStringLiteral("types")).toList()) {
        const QVariantMap m = t.toMap();
        const QString from = m.value("sourceKind").toString(), to = m.value("targetKind").toString();
        if (from != "scene" && to != "scene") { continue; }
        const QString other = from == "scene" ? to : from;
        if (other.isEmpty() && general.isEmpty()) { general = m.value("code").toString(); }
        if (!other.isEmpty() && !specificByKind.contains(other)) { specificByKind.insert(other, m.value("code").toString()); }
    }
    ObjectKindScope::KindInfo kA, kS;   // kA: no kind-specific type; kS: one with
    for (const auto& k : ObjectKindScope::kinds(&bridge)) {
        if (!k.isWorldScoped) { continue; }
        if (specificByKind.contains(k.kind)) { if (kS.kind.isEmpty()) { kS = k; } }
        else if (kA.kind.isEmpty()) { kA = k; }
    }
    check(!general.isEmpty() && !kA.kind.isEmpty() && !kS.kind.isEmpty(),
          QStringLiteral("fixture: the vocabulary has a general type and a kind-specific one"));
    const QString tA = QStringLiteral("objects.") + kA.subdir;
    const QString tS = QStringLiteral("objects.") + kS.subdir;
    QString titleA = kA.subdir;
    titleA[0] = titleA[0].toUpper();

    // Abel linked to scene A, with a field Linux does not model; Zed unlinked.
    const QString abelID = bridge.createObject(dir, kA.kind, QStringLiteral("Abel"), QString(), worldID)
                               .value(QStringLiteral("objectID")).toString();
    bridge.createEdge(dir, abelID, sceneA, general, QString());
    {
        QJsonObject abel = QJsonDocument::fromJson(bridge.openObject(dir, kA.kind, abelID, worldID)
                                                       .value(QStringLiteral("objectJson")).toString().toUtf8()).object();
        abel.insert(QStringLiteral("subtitle"), QStringLiteral("the first"));
        bridge.saveObject(dir, kA.kind, QString::fromUtf8(QJsonDocument(abel).toJson(QJsonDocument::Compact)));
    }
    bridge.createObject(dir, kA.kind, QStringLiteral("Zed"), QString(), worldID);

    auto cardsJson = [&] {
        return QJsonArray{QJsonObject{{"type", tA}, {"collapsed", false}}, QJsonObject{{"type", tS}, {"collapsed", false}}};
    };
    const QJsonObject doc{
        {QStringLiteral("schema"), QStringLiteral("scrivi.inspector-layout.v1")},
        {QStringLiteral("selectedTab"), QStringLiteral("worldbuilding")},
        {QStringLiteral("scenes"), QJsonObject{{sceneA, QJsonObject{{"writing", QJsonArray{}}, {"worldbuilding", cardsJson()}}},
                                               {sceneB, QJsonObject{{"writing", QJsonArray{}}, {"worldbuilding", cardsJson()}}}}}};
    bridge.putInspectorLayout(dir, QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Compact)));
    bridge.closeProject(dir);

    AppEnvironment env(appSupport);
    QPointer<ScriviWindow> w = env.openProjectWindow(dir, QStringLiteral("Objects"), {}, projectID);
    check(waitFor([&] { return env.windows().window(projectID) == w; }, 30000), QStringLiteral("project window loaded"));
    w->resize(1400, 800);
    QTest::qWait(200);
    auto* inspector = w->findChild<SceneInspector*>();
    check(inspector != nullptr, QStringLiteral("the window has a Scene Inspector"));
    if (inspector == nullptr) { return 1; }
    inspector->setScene(sceneA);
    auto* host = inspector->findChild<QWidget*>(QStringLiteral("worldbuildingCards"));
    auto settled = [&](const std::function<bool()>& also) {
        return waitFor([&] { return host->isEnabled() && also(); });
    };
    check(settled([&] { return row(inspector, tA, "Abel") != nullptr; }), QStringLiteral("the scene's read landed"));

    // ── §1 Add <Kind>… ───────────────────────────────────────────────────────────────────────────────────────────────
    auto* add = in<QToolButton>(inspector, tA, QStringLiteral("objectAdd"));
    check(add != nullptr && add->text() == QStringLiteral("+ Add %1…").arg(titleA), QStringLiteral("§1 the card has Add %1…").arg(titleA));
    if (add != nullptr) { add->click(); }
    QDialog* picker = nullptr;
    QTreeWidget* list = nullptr;
    check(waitFor([&] {
              picker = inspector->findChild<QDialog*>(QStringLiteral("objectPicker"));
              list = picker == nullptr ? nullptr : picker->findChild<QTreeWidget*>(QStringLiteral("pickerList"));
              return list != nullptr && list->topLevelItemCount() == 3;
          }),
          QStringLiteral("§1 the picker lists the world's heading and both objects of the kind"));
    QTreeWidgetItem *head = nullptr, *abelItem = nullptr, *zedItem = nullptr;
    for (int i = 0; list != nullptr && i < list->topLevelItemCount(); ++i) {
        QTreeWidgetItem* it = list->topLevelItem(i);
        if (it->text(0) == QStringLiteral("Westmarch")) { head = it; }
        if (it->text(0).startsWith(QStringLiteral("Abel"))) { abelItem = it; }
        if (it->text(0) == QStringLiteral("Zed")) { zedItem = it; }
    }
    check(head != nullptr && !(head->flags() & Qt::ItemIsEnabled), QStringLiteral("§1 grouped under the world's name"));
    check(abelItem != nullptr && abelItem->text(0).endsWith(QStringLiteral("✓")) && !(abelItem->flags() & Qt::ItemIsEnabled),
          QStringLiteral("§1 the object already in the scene is ticked and disabled"));
    check(zedItem != nullptr && (zedItem->flags() & Qt::ItemIsEnabled), QStringLiteral("§1 the other is offered"));
    auto* search = picker != nullptr ? picker->findChild<QLineEdit*>(QStringLiteral("pickerSearch")) : nullptr;
    if (search != nullptr) { search->setText(QStringLiteral("ze")); }
    check(list != nullptr && list->topLevelItemCount() == 2 && list->topLevelItem(1)->text(0) == QStringLiteral("Zed"),
          QStringLiteral("§1 the search narrows the list"));
    auto* createNew = picker != nullptr ? picker->findChild<QPushButton*>(QStringLiteral("pickerCreateNew")) : nullptr;
    check(createNew != nullptr && createNew->text() == QStringLiteral("Create “ze”…"), QStringLiteral("§1 Create names what was typed"));
    if (list != nullptr) { emit list->itemClicked(list->topLevelItem(1), 0); }
    check(settled([&] { return row(inspector, tA, "Zed") != nullptr; }), QStringLiteral("§1 the pick is linked and shown"));
    check(edgeTypes(bridge, dir, sceneA, QStringLiteral("Zed")) == QStringList{general},
          QStringLiteral("§1 one edge, of the derived type: %1").arg(edgeTypes(bridge, dir, sceneA, "Zed").join(",")));

    // ── §2 New ───────────────────────────────────────────────────────────────────────────────────────────────────────
    const int objectsBefore = bridge.listObjects(dir, QString()).value(QStringLiteral("objects")).toList().size();
    in<QToolButton>(inspector, tA, QStringLiteral("objectNew"))->click();
    QApplication::processEvents();
    auto* name = in<QLineEdit>(inspector, tA, QStringLiteral("draftName"));
    auto* commit = in<QPushButton>(inspector, tA, QStringLiteral("draftCommit"));
    auto* world = in<QComboBox>(inspector, tA, QStringLiteral("draftWorld"));
    check(name != nullptr && commit != nullptr && !commit->isEnabled(), QStringLiteral("§2 a draft in the card; Create needs a name"));
    check(world != nullptr && world->currentText() == QStringLiteral("Westmarch"), QStringLiteral("§2 the world is offered, the available one chosen"));
    check(in<QToolButton>(inspector, tA, QStringLiteral("objectNew")) == nullptr, QStringLiteral("§2 New is hidden while drafting"));
    if (name != nullptr) { QTest::keyClicks(name, QStringLiteral("Nova")); }
    check(commit != nullptr && commit->isEnabled(), QStringLiteral("§2 … and is enabled once named"));
    check(bridge.listObjects(dir, QString()).value(QStringLiteral("objects")).toList().size() == objectsBefore,
          QStringLiteral("§2 nothing is written before Create"));
    if (commit != nullptr) { commit->click(); }
    check(settled([&] { return row(inspector, tA, "Nova") != nullptr; }), QStringLiteral("§2 the new object is in the card"));
    check(objectNamed(bridge, dir, QStringLiteral("Nova")).value(QStringLiteral("worldID")).toString() == worldID,
          QStringLiteral("§2 … created in the world"));
    check(edgeTypes(bridge, dir, sceneA, QStringLiteral("Nova")) == QStringList{general}, QStringLiteral("§2 … linked by the general type"));
    in<QToolButton>(inspector, tS, QStringLiteral("objectNew"))->click();
    QApplication::processEvents();
    QTest::keyClicks(in<QLineEdit>(inspector, tS, QStringLiteral("draftName")), QStringLiteral("Harbour"));
    QTest::keyClick(in<QLineEdit>(inspector, tS, QStringLiteral("draftName")), Qt::Key_Return);
    check(settled([&] { return row(inspector, tS, "Harbour") != nullptr; }), QStringLiteral("§2 Return creates"));
    check(edgeTypes(bridge, dir, sceneA, QStringLiteral("Harbour")) == QStringList{specificByKind.value(kS.kind)},
          QStringLiteral("§2 a kind the vocabulary names is linked by its own type (Q5): %1")
              .arg(edgeTypes(bridge, dir, sceneA, "Harbour").join(",")));

    // ── §3 Rename (F2) ───────────────────────────────────────────────────────────────────────────────────────────────
    auto* tree = in<QTreeWidget>(inspector, tA, QStringLiteral("objectRows"));
    tree->setFocus();
    tree->setCurrentItem(row(inspector, tA, "Abel"));
    QTest::keyClick(tree, Qt::Key_F2);
    QApplication::processEvents();
    name = in<QLineEdit>(inspector, tA, QStringLiteral("draftName"));
    auto* exitButton = in<QPushButton>(inspector, tA, QStringLiteral("draftExit"));
    check(name != nullptr && name->text() == QStringLiteral("Abel"), QStringLiteral("§3 F2 opens the editor on the row"));
    check(in<QComboBox>(inspector, tA, QStringLiteral("draftWorld")) == nullptr, QStringLiteral("§3 the world is shown, not editable"));
    check(exitButton != nullptr && exitButton->text() == QStringLiteral("Done"), QStringLiteral("§3 untouched, the exit is Done"));
    if (name != nullptr) {
        name->selectAll();
        QTest::keyClicks(name, QStringLiteral("Abel Prime"));
    }
    check(exitButton != nullptr && exitButton->text() == QStringLiteral("Revert"), QStringLiteral("§3 edited, it is Revert"));
    in<QPushButton>(inspector, tA, QStringLiteral("draftCommit"))->click();
    check(settled([&] { return row(inspector, tA, "Abel Prime") != nullptr; }), QStringLiteral("§3 renamed in the card"));
    const QJsonObject abel = QJsonDocument::fromJson(bridge.openObject(dir, kA.kind, abelID, worldID)
                                                         .value(QStringLiteral("objectJson")).toString().toUtf8()).object();
    check(abel.value("displayName").toString() == "Abel Prime" && abel.value("subtitle").toString() == "the first",
          QStringLiteral("§3 displayName patched, every other field kept"));

    // ── §4 Remove from Scene ─────────────────────────────────────────────────────────────────────────────────────────
    tree = in<QTreeWidget>(inspector, tA, QStringLiteral("objectRows"));
    auto* minus = qobject_cast<QToolButton*>(tree->itemWidget(row(inspector, tA, "Zed"), 1));
    check(minus != nullptr && minus->isEnabled(), QStringLiteral("§4 a row has ⊖"));
    if (minus != nullptr) { minus->click(); }
    check(settled([&] { return row(inspector, tA, "Zed") == nullptr && row(inspector, tA, "Nova") != nullptr; }),
          QStringLiteral("§4 the row goes"));
    check(edgeTypes(bridge, dir, sceneA, QStringLiteral("Zed")).isEmpty() && !objectNamed(bridge, dir, "Zed").isEmpty(),
          QStringLiteral("§4 the edge is deleted; the object survives"));

    // ── §5 a draft at a scene boundary ───────────────────────────────────────────────────────────────────────────────
    in<QToolButton>(inspector, tA, QStringLiteral("objectNew"))->click();
    QApplication::processEvents();
    inspector->setScene(sceneB);   // untouched: nothing to lose
    QTest::qWait(100);
    check(inspector->findChild<QMessageBox*>(QStringLiteral("unfinishedDraftPrompt")) == nullptr,
          QStringLiteral("§5 an untouched draft just closes"));
    check(settled([&] { return in<QToolButton>(inspector, tA, QStringLiteral("objectNew")) != nullptr; }),
          QStringLiteral("§5 … and New is back"));
    inspector->setScene(sceneA);
    check(settled([&] { return row(inspector, tA, "Nova") != nullptr; }), QStringLiteral("§5 back on scene A"));
    in<QToolButton>(inspector, tA, QStringLiteral("objectNew"))->click();
    QApplication::processEvents();
    QTest::keyClicks(in<QLineEdit>(inspector, tA, QStringLiteral("draftName")), QStringLiteral("Wren"));
    inspector->setScene(sceneB);
    QMessageBox* box = prompt(inspector);
    check(box != nullptr, QStringLiteral("§5 leaving with a named draft asks"));
    clickButton(box, QStringLiteral("Keep Editing"));
    check(settled([&] {
              auto* origin = in<QLabel>(inspector, tA, QStringLiteral("draftOrigin"));
              return origin != nullptr && origin->text() == QStringLiteral("for: Scene 1");
          }),
          QStringLiteral("§5 Keep Editing: the draft names the scene it came from"));
    commit = in<QPushButton>(inspector, tA, QStringLiteral("draftCommit"));
    check(commit != nullptr && commit->text() == QStringLiteral("Create in Scene 1"), QStringLiteral("§5 … and so does Create"));
    if (commit != nullptr) { commit->click(); }
    check(waitFor([&] { return !edgeTypes(bridge, dir, sceneA, QStringLiteral("Wren")).isEmpty(); }),
          QStringLiteral("§5 Create files it against the ORIGIN scene (I-0119)"));
    check(edgeTypes(bridge, dir, sceneB, QStringLiteral("Wren")).isEmpty(), QStringLiteral("§5 … not the one on screen"));
    check(settled([&] { return in<QToolButton>(inspector, tA, QStringLiteral("objectNew")) != nullptr; }),
          QStringLiteral("§5 the draft closes once it is created"));
    in<QToolButton>(inspector, tA, QStringLiteral("objectNew"))->click();
    QApplication::processEvents();
    QTest::keyClicks(in<QLineEdit>(inspector, tA, QStringLiteral("draftName")), QStringLiteral("Ghost"));
    inspector->setScene(sceneA);
    box = prompt(inspector);
    clickButton(box, QStringLiteral("Discard New %1").arg(titleA.chopped(1)));
    QTest::qWait(300);
    check(objectNamed(bridge, dir, QStringLiteral("Ghost")).isEmpty() && in<QLineEdit>(inspector, tA, "draftName") == nullptr,
          QStringLiteral("§5 Discard writes nothing and closes the draft"));

    // ── §6 a row's image (A6) ────────────────────────────────────────────────────────────────────────────────────────
    const QString png = scratch + QStringLiteral("/nova.png");
    QImage image(64, 64, QImage::Format_ARGB32);
    image.fill(Qt::darkCyan);
    image.save(png);
    const QString assetID = bridge.importAsset(dir, png, QStringLiteral("image"), QStringLiteral("Nova"), worldID, projectID)
                                .value(QStringLiteral("assetID")).toString();
    const QString novaID = objectNamed(bridge, dir, QStringLiteral("Nova")).value(QStringLiteral("objectID")).toString();
    QJsonObject nova = QJsonDocument::fromJson(bridge.openObject(dir, kA.kind, novaID, worldID)
                                                   .value(QStringLiteral("objectJson")).toString().toUtf8()).object();
    nova.insert(QStringLiteral("image"), QJsonObject{{QStringLiteral("assetID"), assetID}});
    bridge.saveObject(dir, kA.kind, QString::fromUtf8(QJsonDocument(nova).toJson(QJsonDocument::Compact)));
    check(!assetID.isEmpty() && !objectNamed(bridge, dir, "Nova").value("imagePath").toString().isEmpty(),
          QStringLiteral("§6 fixture: Nova has an image"));
    inspector->setScene(sceneB);
    QTest::qWait(100);
    inspector->setScene(sceneA);
    check(settled([&] {
              QTreeWidgetItem* r = row(inspector, tA, "Nova");
              return r != nullptr && !r->icon(0).isNull();
          }),
          QStringLiteral("§6 the row shows its image"));
    QTreeWidgetItem* plain = row(inspector, tA, "Abel");
    check(plain != nullptr && plain->icon(0).isNull(), QStringLiteral("§6 a row without one is unchanged"));

    w->close();
    QTest::qWait(100);
    if (failures == 0) { std::printf("inspector-objects-ok\n"); }
    std::printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
