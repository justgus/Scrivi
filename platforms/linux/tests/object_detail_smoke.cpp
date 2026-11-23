// object_detail_smoke.cpp — [SP-235] (EP-059 B1–B4; T-0697): the object detail view, as the Mac's
// (`ObjectDetailSheet.swift`, `ObjectImageSection.swift`).
//
// A REAL project window through `AppEnvironment`, with a world and an object linked to a scene.
//
// ✅ What it proves:
//   §1 a card row's activation opens the view: Name, Subtitle, Notes, Tags and the metadata shown; Save off until an edit;
//   §2 Save merges per field with disk (a rename made elsewhere survives), patches (an unknown key survives), writes tags as
//      [{"v"}], refuses a duplicate tag, says "Changes saved" and keeps the view open; the Inspector follows;
//   §3 Undo Changes reverts and says so;
//   §4 closing with unsaved changes asks: Cancel keeps it open, Discard closes and writes nothing;
//   §5 the image: added, replaced (the old asset removed), chosen from the world's images, removed — a SHARED asset kept;
//   §6 B4: the world goes away while it is open — the fields stay, read-only, the at-risk banner shows, nothing is written;
//      the world comes back — they unlock and Save writes the held edits;
//   §7 B3: an object whose world is away at open is named and explained.
//   [SP-236] (EP-059 C1–C4; T-0698, T-0699), as the Mac's (`ObjectRelationsSection`, `ObjectRelationPicker`,
//   `ObjectSourcesSection`, `SourcesCard`):
//   §8  C1/C2: Related groups by the core's label; Relate… offers only types that fit and the objects they can point at,
//       creates the edge, and the other object reads it from its own end;
//   §9  C3: a related object opens in the same window; Back returns, named; Back with changes asks Save · Discard (no
//       Cancel); opening another with changes asks Save · Discard · Cancel; a scene row asks for the scene; Remove;
//   §10 C4: New Source… creates a source documenting the object; the citation shows its fields and who cites it; Cite…
//       offers the project's sources (one already cited disabled); Remove Citation; a source opens in place;
//   §11 C4: the scene's Sources card lists the sources its objects cite, "via" them;
//   §12 read-only: Relate…, Cite… and New Source… are off while the object's world is away.
//
//   argv[1] = a scratch directory; $XDG_DATA_HOME drives appSupportRoot; Qt offscreen.

#include "AppEnvironment.hpp"
#include "AppSupport.hpp"
#include "EditorShell.hpp"
#include "ManuscriptTypography.hpp"
#include "ObjectDetailDialog.hpp"
#include "ObjectKindScope.hpp"
#include "SceneInspector.hpp"
#include "ScriviBridge.hpp"
#include "ScriviWindow.hpp"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QMenu>
#include <QSignalSpy>
#include <QComboBox>
#include <QTabWidget>
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

QTreeWidgetItem* row(SceneInspector* inspector, const QString& startsWith)
{
    auto* host = inspector->findChild<QWidget*>(QStringLiteral("worldbuildingCards"));
    for (QTreeWidget* tree : host->findChildren<QTreeWidget*>(QStringLiteral("objectRows"))) {
        for (int i = 0; tree->isVisibleTo(host) && i < tree->topLevelItemCount(); ++i) {
            // A pending row reads "⚠ Name".
            const QString text = tree->topLevelItem(i)->text(0);
            if (text.startsWith(startsWith) || text.startsWith(QStringLiteral("⚠ ") + startsWith)) { return tree->topLevelItem(i); }
        }
    }
    return nullptr;
}

QJsonObject objectJson(ScriviBridge& bridge, const QString& dir, const QString& kind, const QString& id, const QString& world)
{
    return QJsonDocument::fromJson(bridge.openObject(dir, kind, id, world).value(QStringLiteral("objectJson")).toString().toUtf8())
        .object();
}

QStringList tagsOf(const QJsonObject& json)
{
    QStringList out;
    for (const QJsonValue& v : json.value(QStringLiteral("tags")).toArray()) { out << v.toObject().value("v").toString(); }
    return out;
}

QStringList worldAssets(ScriviBridge& bridge, const QString& dir, const QString& world)
{
    QStringList out;
    for (const QVariant& a : bridge.listAssets(dir, QStringLiteral("image"), world).value(QStringLiteral("assets")).toList()) {
        out << a.toMap().value(QStringLiteral("assetID")).toString();
    }
    return out;
}

QStringList chips(ObjectDetailDialog* d)
{
    QStringList out;
    for (QFrame* f : d->findChildren<QFrame*>(QStringLiteral("tagChip"))) { out << f->property("tag").toString(); }
    return out;
}

template <typename T>
T* part(ObjectDetailDialog* d, const char* name)
{
    return d == nullptr ? nullptr : d->findChild<T*>(QString::fromLatin1(name));
}

QAction* action(ObjectDetailDialog* d, const char* name)
{
    for (QAction* a : d->findChildren<QAction*>()) {
        if (a->objectName() == QLatin1String(name)) { return a; }
    }
    return nullptr;
}

void clickButton(QMessageBox* box, const QString& text)
{
    for (QAbstractButton* b : box != nullptr ? box->buttons() : QList<QAbstractButton*>{}) {
        if (b->text() == text) { b->click(); return; }
    }
    check(false, QStringLiteral("the prompt has “%1”").arg(text));
}

QTreeWidgetItem* relation(ObjectDetailDialog* d, const QString& group, const QString& startsWith)
{
    auto* tree = part<QTreeWidget>(d, "detailRelations");
    for (int g = 0; tree != nullptr && g < tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem* top = tree->topLevelItem(g);
        for (int i = 0; top->text(0) == group && i < top->childCount(); ++i) {
            if (top->child(i)->text(0).startsWith(startsWith)) { return top->child(i); }
        }
    }
    return nullptr;
}

QStringList sources(ObjectDetailDialog* d)
{
    QStringList out;
    auto* list = part<QListWidget>(d, "detailSources");
    for (int i = 0; list != nullptr && list->isVisible() && i < list->count(); ++i) { out << list->item(i)->text(); }
    return out;
}

void menuAction(QWidget* host, const char* menuName, const QString& text)
{
    QMenu* menu = nullptr;
    waitFor([&] {
        menu = host->findChild<QMenu*>(QString::fromLatin1(menuName));
        return menu != nullptr && menu->isVisible();
    }, 3000);
    for (QAction* a : menu != nullptr ? menu->actions() : QList<QAction*>{}) {
        if (a->text() == text) {
            check(a->isEnabled(), QStringLiteral("the menu's “%1” is enabled").arg(text));
            a->trigger();
            menu->close();
            return;
        }
    }
    check(false, QStringLiteral("the %1 has “%2”").arg(QString::fromLatin1(menuName), text));
}

QString png(const QString& path, const QColor& colour)
{
    QImage image(48, 48, QImage::Format_ARGB32);
    image.fill(colour);
    image.save(path);
    return path;
}

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fprintf(stderr, "usage: object_detail_smoke <scratchDir>\n");
        return 2;
    }
    BundledFonts::registerAll();
    const QString scratch = QString::fromUtf8(argv[1]);
    QDir().mkpath(scratch);
    const QString appSupport = scrivi::linux_app::appSupportRoot();
    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Scrivi"), appSupport);
    const QString dir = scratch + QStringLiteral("/detail.inkwyn");
    const QString projectID = bridge.createProject(dir, appSupport, QStringLiteral("Detail"), QStringLiteral("detail"))
                                  .value(QStringLiteral("projectID")).toString();
    const QVariantMap firstScene = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap();
    const QString sceneA = firstScene.value(QStringLiteral("sceneID")).toString();
    // Untitled, as the Dumas scenes are: Related must name it as the Navigator does (SP-236 live pass, step 2).
    bridge.renameScene(dir, firstScene.value(QStringLiteral("metadataPath")).toString(), QString());
    const QString worldPath = scratch + QStringLiteral("/W.inkwynworld");
    const QString worldID = bridge.createWorld(dir, worldPath, QStringLiteral("Westmarch"), QStringLiteral("AE"))
                                .value(QStringLiteral("worldID")).toString();
    ObjectKindScope::KindInfo kA;
    for (const auto& k : ObjectKindScope::kinds(&bridge)) {
        if (k.isWorldScoped && kA.kind.isEmpty()) { kA = k; }
    }
    QString general;
    for (const QVariant& t : bridge.listRelationTypes(dir).value(QStringLiteral("types")).toList()) {
        const QVariantMap m = t.toMap();
        if (m.value("targetKind").toString() == "scene" && m.value("sourceKind").toString().isEmpty() && general.isEmpty()) {
            general = m.value("code").toString();
        }
    }
    check(!sceneA.isEmpty() && !worldID.isEmpty() && !kA.kind.isEmpty() && !general.isEmpty(),
          QStringLiteral("fixture: a scene, a world, a world-scoped kind and a general relation type"));
    const QString abelID = bridge.createObject(dir, kA.kind, QStringLiteral("Abel"), QString(), worldID)
                               .value(QStringLiteral("objectID")).toString();
    bridge.createEdge(dir, abelID, sceneA, general, QString());
    {
        QJsonObject abel = objectJson(bridge, dir, kA.kind, abelID, worldID);
        abel.insert(QStringLiteral("subtitle"), QStringLiteral("the first"));
        abel.insert(QStringLiteral("tags"), QJsonArray{QJsonObject{{"v", "old"}}});
        // A field this dialog does not edit: a patch keeps it.
        abel.insert(QStringLiteral("attributes"), QJsonArray{QJsonObject{{"k", "eyes"}, {"v", "grey"}}});
        bridge.saveObject(dir, kA.kind, QString::fromUtf8(QJsonDocument(abel).toJson(QJsonDocument::Compact)));
    }
    // An image already in the world, attached to nothing — for Choose Existing….
    const QString spareID = bridge.importAsset(dir, png(scratch + "/spare.png", Qt::darkGreen), QStringLiteral("image"),
                                               QStringLiteral("spare.png"), worldID, projectID)
                                .value(QStringLiteral("assetID")).toString();
    check(!spareID.isEmpty(), QStringLiteral("fixture: a spare image in the world"));
    {   // … which another object already shows: removing it from Abel must not take Bea's image ([I-0369]).
        const QString beaID = bridge.createObject(dir, kA.kind, QStringLiteral("Bea"), QString(), worldID)
                                  .value(QStringLiteral("objectID")).toString();
        QJsonObject bea = objectJson(bridge, dir, kA.kind, beaID, worldID);
        bea.insert(QStringLiteral("image"), QJsonObject{{QStringLiteral("assetID"), spareID}});
        bridge.saveObject(dir, kA.kind, QString::fromUtf8(QJsonDocument(bea).toJson(QJsonDocument::Compact)));
    }
    // [SP-236]: two characters (the one kind `sibling-of` links), Cara in the scene.
    const QString caraID = bridge.createObject(dir, QStringLiteral("character"), QStringLiteral("Cara"), QString(), worldID)
                               .value(QStringLiteral("objectID")).toString();
    const QString dovID = bridge.createObject(dir, QStringLiteral("character"), QStringLiteral("Dov"), QString(), worldID)
                              .value(QStringLiteral("objectID")).toString();
    bridge.createEdge(dir, caraID, sceneA, general, QString());
    // An unlinked source in the project, for Cite….
    const QString annalsID = bridge.createObject(dir, QStringLiteral("source"), QStringLiteral("Annals"), QString(), QString())
                                 .value(QStringLiteral("objectID")).toString();
    check(!caraID.isEmpty() && !dovID.isEmpty() && !annalsID.isEmpty(), QStringLiteral("fixture: Cara, Dov and a source"));
    const QString tA = QStringLiteral("objects.") + kA.subdir;
    const QJsonObject doc{
        {QStringLiteral("schema"), QStringLiteral("scrivi.inspector-layout.v1")},
        {QStringLiteral("selectedTab"), QStringLiteral("worldbuilding")},
        {QStringLiteral("scenes"),
         QJsonObject{{sceneA, QJsonObject{{"writing", QJsonArray{}},
                                          {"worldbuilding", QJsonArray{QJsonObject{{"type", tA}, {"collapsed", false}}}}}}}}};
    bridge.putInspectorLayout(dir, QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Compact)));
    bridge.closeProject(dir);

    AppEnvironment env(appSupport);
    QPointer<ScriviWindow> w = env.openProjectWindow(dir, QStringLiteral("Detail"), {}, projectID);
    check(waitFor([&] { return env.windows().window(projectID) == w; }, 30000), QStringLiteral("project window loaded"));
    w->resize(1400, 800);
    QTest::qWait(200);
    auto* inspector = w->findChild<SceneInspector*>();
    if (inspector == nullptr) { std::printf("FAIL: no Scene Inspector\n"); return 1; }
    inspector->setScene(sceneA);
    check(waitFor([&] { return row(inspector, "Abel") != nullptr; }), QStringLiteral("the scene's read landed"));

    QPointer<ObjectDetailDialog> d;
    auto openAbel = [&](const QString& name) {
        QTreeWidgetItem* r = row(inspector, name);
        if (r != nullptr) { emit r->treeWidget()->itemActivated(r, 0); }   // double-click and Return
        waitFor([&] {
            d = w->findChild<ObjectDetailDialog*>();
            return d != nullptr && d->isVisible();
        }, 3000);
    };
    auto loaded = [&] {
        return waitFor([&] {
            auto* n = part<QLineEdit>(d, "detailName");
            return n != nullptr && n->isVisible() && !n->text().isEmpty();
        });
    };
    auto closed = [&] { return waitFor([&] { return d.isNull(); }, 3000); };

    // ── §1 open ──────────────────────────────────────────────────────────────────────────────────────────────────────
    openAbel(QStringLiteral("Abel"));
    check(d != nullptr && d->windowModality() == Qt::WindowModal, QStringLiteral("§1 the detail view opens over the window"));
    check(loaded(), QStringLiteral("§1 the object's fields are read"));
    check(part<QLineEdit>(d, "detailSubtitle")->text() == QStringLiteral("the first"), QStringLiteral("§1 Subtitle shown"));
    check(chips(d) == QStringList{QStringLiteral("old")}, QStringLiteral("§1 Tags shown as chips"));
    check(part<QLabel>(d, "detailMetadata")->text().contains(QStringLiteral("World: Westmarch")),
          QStringLiteral("§1 the world is shown, as text"));
    check(!action(d, "detailSave")->isEnabled() && !action(d, "detailRevert")->isEnabled(),
          QStringLiteral("§1 Save and Undo Changes wait for an edit"));
    check(part<QLabel>(d, "detailImageMessage")->isVisible() && part<QLabel>(d, "detailImageMessage")->text() == QStringLiteral("No image."),
          QStringLiteral("§1 No image."));

    // ── §2 Save ──────────────────────────────────────────────────────────────────────────────────────────────────────
    part<QLineEdit>(d, "detailSubtitle")->setText(QStringLiteral("the brave"));
    part<QPlainTextEdit>(d, "detailNotes")->setPlainText(QStringLiteral("A note."));
    auto* newTag = part<QLineEdit>(d, "detailNewTag");
    QTest::keyClicks(newTag, QStringLiteral("hero"));
    QTest::keyClick(newTag, Qt::Key_Return);
    QTest::keyClicks(newTag, QStringLiteral("hero"));
    QTest::keyClick(newTag, Qt::Key_Return);
    check(chips(d) == QStringList({QStringLiteral("old"), QStringLiteral("hero")}), QStringLiteral("§2 a tag added, its duplicate refused"));
    check(d->findChild<QFileDialog*>(QStringLiteral("imageFileDialog")) == nullptr,
          QStringLiteral("§2 Return in the tag field does not press Add Image… (SP-235 live pass)"));
    for (QToolButton* x : d->findChildren<QToolButton*>(QStringLiteral("tagRemove"))) {
        if (x->parentWidget()->property("tag").toString() == QStringLiteral("old")) { x->click(); }
    }
    check(chips(d) == QStringList{QStringLiteral("hero")}, QStringLiteral("§2 a chip's ✕ removes it"));
    check(action(d, "detailSave")->isEnabled(), QStringLiteral("§2 an edit enables Save"));
    {   // Meanwhile, another surface renames the object (I-0155): the save must not write the old name back.
        QJsonObject abel = objectJson(bridge, dir, kA.kind, abelID, worldID);
        abel.insert(QStringLiteral("displayName"), QStringLiteral("Abel Renamed"));
        bridge.saveObject(dir, kA.kind, QString::fromUtf8(QJsonDocument(abel).toJson(QJsonDocument::Compact)));
    }
    action(d, "detailSave")->trigger();
    check(waitFor([&] { return part<QLabel>(d, "detailStatus")->text() == QStringLiteral("Changes saved"); }),
          QStringLiteral("§2 the view says Changes saved"));
    const QJsonObject saved = objectJson(bridge, dir, kA.kind, abelID, worldID);
    check(saved.value("displayName").toString() == QStringLiteral("Abel Renamed"),
          QStringLiteral("§2 a field not edited here takes disk's value: %1").arg(saved.value("displayName").toString()));
    check(saved.value("subtitle").toString() == QStringLiteral("the brave") && saved.value("notes").toString() == QStringLiteral("A note."),
          QStringLiteral("§2 the edits are written"));
    check(tagsOf(saved) == QStringList{QStringLiteral("hero")}, QStringLiteral("§2 tags written as [{\"v\"}]"));
    check(saved.value("attributes").toArray() == QJsonArray{QJsonObject{{"k", "eyes"}, {"v", "grey"}}},
          QStringLiteral("§2 a patch: a field the view does not edit survives"));
    check(d != nullptr && d->isVisible(), QStringLiteral("§2 Save keeps the view open"));
    check(waitFor([&] { return part<QLineEdit>(d, "detailName")->text() == QStringLiteral("Abel Renamed"); }),
          QStringLiteral("§2 the view re-reads the name"));
    check(waitFor([&] { return row(inspector, "Abel Renamed") != nullptr; }), QStringLiteral("§2 the Inspector follows"));

    // ── §3 Undo Changes ──────────────────────────────────────────────────────────────────────────────────────────────
    part<QLineEdit>(d, "detailName")->setText(QStringLiteral("Typo"));
    check(part<QLabel>(d, "detailStatus")->isHidden(), QStringLiteral("§3 an edit clears the status"));
    action(d, "detailRevert")->trigger();
    check(part<QLineEdit>(d, "detailName")->text() == QStringLiteral("Abel Renamed")
              && part<QLabel>(d, "detailStatus")->text() == QStringLiteral("Changes reverted"),
          QStringLiteral("§3 Undo Changes reverts and says so"));

    // ── §4 the guarded close ─────────────────────────────────────────────────────────────────────────────────────────
    part<QPlainTextEdit>(d, "detailNotes")->setPlainText(QStringLiteral("Unsaved."));
    action(d, "detailClose")->trigger();
    QMessageBox* box = nullptr;
    check(waitFor([&] {
              box = d->findChild<QMessageBox*>(QStringLiteral("saveChangesPrompt"));
              return box != nullptr && box->isVisible();
          }, 3000),
          QStringLiteral("§4 closing with unsaved changes asks"));
    clickButton(box, QStringLiteral("Cancel"));
    QTest::qWait(50);
    check(d != nullptr && d->isVisible(), QStringLiteral("§4 Cancel keeps it open"));
    QTest::keyClick(d, Qt::Key_Escape);   // Esc takes the same guarded path (I-0250)
    check(waitFor([&] {
              box = d->findChild<QMessageBox*>(QStringLiteral("saveChangesPrompt"));
              return box != nullptr && box->isVisible();
          }, 3000),
          QStringLiteral("§4 Esc asks too"));
    clickButton(box, QStringLiteral("Discard"));
    check(closed(), QStringLiteral("§4 Discard closes"));
    check(objectJson(bridge, dir, kA.kind, abelID, worldID).value("notes").toString() == QStringLiteral("A note."),
          QStringLiteral("§4 … and writes nothing"));

    // ── §5 the image ─────────────────────────────────────────────────────────────────────────────────────────────────
    openAbel(QStringLiteral("Abel Renamed"));
    check(loaded(), QStringLiteral("§5 reopened"));
    auto pickFile = [&](const QString& path) {
        part<QPushButton>(d, "detailImageAdd")->click();
        QFileDialog* chooser = nullptr;
        waitFor([&] {
            chooser = d->findChild<QFileDialog*>(QStringLiteral("imageFileDialog"));
            return chooser != nullptr;
        }, 3000);
        check(chooser != nullptr, QStringLiteral("§5 a file chooser opens"));
        if (chooser != nullptr) {
            emit chooser->fileSelected(path);
            chooser->close();
        }
    };
    auto imageID = [&] {
        return objectJson(bridge, dir, kA.kind, abelID, worldID).value("image").toObject().value("assetID").toString();
    };
    check(part<QPushButton>(d, "detailImageAdd")->text() == QStringLiteral("Add Image…"), QStringLiteral("§5 Add Image…"));
    pickFile(png(scratch + "/red.png", Qt::red));
    check(waitFor([&] { return part<QLabel>(d, "detailImage")->isVisible(); }), QStringLiteral("§5 the image is shown"));
    const QString firstID = imageID();
    check(!firstID.isEmpty() && part<QLabel>(d, "detailImageFile")->isVisible(), QStringLiteral("§5 … attached, its file named"));
    check(part<QPushButton>(d, "detailImageAdd")->text() == QStringLiteral("Replace…"), QStringLiteral("§5 Replace…"));
    pickFile(png(scratch + "/blue.png", Qt::blue));
    check(waitFor([&] { return !imageID().isEmpty() && imageID() != firstID; }), QStringLiteral("§5 replaced"));
    // The new key lands before the old asset goes (attach first, then remove — the same worker call): wait for it.
    check(waitFor([&] { return !worldAssets(bridge, dir, worldID).contains(firstID); }), QStringLiteral("§5 … and the old asset removed"));
    QTest::qWait(200);
    part<QPushButton>(d, "detailImageExisting")->click();
    QListWidget* existing = nullptr;
    check(waitFor([&] {
              existing = d->findChild<QListWidget*>(QStringLiteral("existingImageList"));
              return existing != nullptr && existing->count() == 2;
          }),
          QStringLiteral("§5 Choose Existing… lists the world's images"));
    QListWidgetItem* spare = nullptr;
    for (int i = 0; existing != nullptr && i < existing->count(); ++i) {
        QListWidgetItem* it = existing->item(i);
        if (it->data(Qt::UserRole).toString() == spareID) { spare = it; }
        else { check(it->text().endsWith(QStringLiteral("✓")) && !(it->flags() & Qt::ItemIsEnabled), QStringLiteral("§5 the current one is ticked, disabled")); }
    }
    if (spare != nullptr) { emit existing->itemClicked(spare); }
    check(waitFor([&] { return imageID() == spareID; }), QStringLiteral("§5 an existing image is chosen"));
    QTest::qWait(200);
    // The preview shows only once the core's path for the chosen image is read — the path the shared-asset guard compares.
    check(waitFor([&] { return part<QLabel>(d, "detailImage")->isVisible() && part<QPushButton>(d, "detailImageRemove")->isEnabled(); }),
          QStringLiteral("§5 the chosen image is shown; Remove offered"));
    part<QPushButton>(d, "detailImageRemove")->click();
    check(waitFor([&] {
              auto* m = part<QLabel>(d, "detailImageMessage");
              return m->isVisible() && m->text() == QStringLiteral("No image.");
          }),
          QStringLiteral("§5 removed: No image."));
    check(waitFor([&] { return !objectJson(bridge, dir, kA.kind, abelID, worldID).contains("image"); }),
          QStringLiteral("§5 … the image key gone"));
    check(worldAssets(bridge, dir, worldID).contains(spareID), QStringLiteral("§5 … but a shared asset kept: Bea still shows it"));

    // ── §6 B4: the world goes away while the view is open ────────────────────────────────────────────────────────────
    QTest::qWait(200);
    part<QPlainTextEdit>(d, "detailNotes")->setPlainText(QStringLiteral("Held while away."));
    check(QDir().rename(worldPath, worldPath + QStringLiteral(".away")), QStringLiteral("§6 the world's package is moved away"));
    w->editorShell()->refreshWorlds();
    check(waitFor([&] { return part<QLabel>(d, "detailReadOnly")->isVisible(); }), QStringLiteral("§6 the view turns read-only, explained"));
    check(part<QLabel>(d, "detailAtRisk")->isVisible(), QStringLiteral("§6 the at-risk banner shows while edits are held"));
    check(part<QPlainTextEdit>(d, "detailNotes")->toPlainText() == QStringLiteral("Held while away.")
              && part<QPlainTextEdit>(d, "detailNotes")->isReadOnly() && part<QLineEdit>(d, "detailName")->isReadOnly(),
          QStringLiteral("§6 the fields stay, read-only"));
    check(!action(d, "detailSave")->isEnabled() && !part<QPushButton>(d, "detailImageAdd")->isEnabled()
              && d->findChildren<QToolButton*>(QStringLiteral("tagRemove")).isEmpty() && !part<QLineEdit>(d, "detailNewTag")->isVisible(),
          QStringLiteral("§6 Save, the image and the tags are locked"));
    check(QDir().rename(worldPath + QStringLiteral(".away"), worldPath), QStringLiteral("§6 the world comes back"));
    w->editorShell()->refreshWorlds();
    check(waitFor([&] { return !part<QLabel>(d, "detailReadOnly")->isVisible() && action(d, "detailSave")->isEnabled(); }),
          QStringLiteral("§6 the fields unlock, Save is offered"));
    check(part<QPlainTextEdit>(d, "detailNotes")->toPlainText() == QStringLiteral("Held while away."),
          QStringLiteral("§6 the held edits survived the re-read"));
    action(d, "detailSave")->trigger();
    check(waitFor([&] { return objectJson(bridge, dir, kA.kind, abelID, worldID).value("notes").toString() == "Held while away."; }),
          QStringLiteral("§6 Save writes them"));
    QTest::qWait(200);
    action(d, "detailClose")->trigger();
    check(closed(), QStringLiteral("§6 closes without asking once saved"));

    // ── §7 B3: a world away at open ──────────────────────────────────────────────────────────────────────────────────
    QDir().rename(worldPath, worldPath + QStringLiteral(".away"));
    w->editorShell()->refreshWorlds();
    QTest::qWait(500);
    openAbel(QStringLiteral("Abel Renamed"));
    check(d != nullptr, QStringLiteral("§7 the view opens for an object whose world is away"));
    if (d == nullptr) { return 1; }
    check(waitFor([&] {
              auto* m = part<QLabel>(d, "detailMessage");
              return m != nullptr && m->isVisible() && m->text().contains(QStringLiteral("Westmarch"));
          }),
          QStringLiteral("§7 … names the world: %1").arg(part<QLabel>(d, "detailMessage") ? part<QLabel>(d, "detailMessage")->text() : QString()));
    check(part<QLabel>(d, "detailMessageTitle")->text() == QStringLiteral("Abel Renamed")
              && part<QLabel>(d, "detailMessageHint")->text().contains(QStringLiteral("available again")),
          QStringLiteral("§7 … names the object and says when it will appear"));
    if (d != nullptr) { action(d, "detailClose")->trigger(); }
    check(closed(), QStringLiteral("§7 closes"));
    QDir().rename(worldPath + QStringLiteral(".away"), worldPath);
    w->editorShell()->refreshWorlds();
    QTest::qWait(500);

    // ── §8 C1/C2: Related, and Relate… ───────────────────────────────────────────────────────────────────────────
    auto openObject = [&](const QString& kind, const QString& id, const QString& world, const QString& name) {
        QMetaObject::invokeMethod(w->editorShell(), "onOpenObjectRequested", Q_ARG(QString, kind), Q_ARG(QString, id),
                                  Q_ARG(QString, world), Q_ARG(QString, name), Q_ARG(bool, false));
        waitFor([&] {
            d = w->findChild<ObjectDetailDialog*>();
            return d != nullptr && d->isVisible();
        }, 3000);
    };
    auto showing = [&](const QString& name) {
        return waitFor([&] {
            auto* n = part<QLineEdit>(d, "detailName");
            return n != nullptr && n->isVisible() && n->text() == name;
        });
    };
    openObject(QStringLiteral("character"), caraID, worldID, QStringLiteral("Cara"));
    check(showing(QStringLiteral("Cara")), QStringLiteral("§8 Cara opens"));
    if (d == nullptr) { return 1; }
    check(waitFor([&] { return relation(d, QStringLiteral("appears in"), QString()) != nullptr; }),
          QStringLiteral("§8 Related groups the scene under the core's label, “appears in”"));
    check(relation(d, QStringLiteral("appears in"), QStringLiteral("Untitled")) == nullptr,
          QStringLiteral("§8 an untitled scene is named as the Navigator names it, not “Untitled scene”"));
    check(!action(d, "detailBack")->isVisible(), QStringLiteral("§8 no Back at the first object"));
    part<QPushButton>(d, "detailRelate")->click();
    QDialog* picker = nullptr;
    check(waitFor([&] {
              picker = d->findChild<QDialog*>(QStringLiteral("relatePicker"));
              return picker != nullptr && picker->isVisible();
          }),
          QStringLiteral("§8 Relate… opens the picker"));
    auto* types = picker != nullptr ? picker->findChild<QComboBox*>(QStringLiteral("relateType")) : nullptr;
    QStringList typeLabels;
    for (int i = 0; types != nullptr && i < types->count(); ++i) { typeLabels << types->itemText(i); }
    check(typeLabels.contains(QStringLiteral("sibling of")) && !typeLabels.contains(QStringLiteral("appears in"))
              && !typeLabels.contains(QStringLiteral("takes place at")),
          QStringLiteral("§8 only the types from a character to an object: %1").arg(typeLabels.join(", ")));
    if (types != nullptr) { types->setCurrentIndex(types->findText(QStringLiteral("sibling of"))); }
    auto* list = picker != nullptr ? picker->findChild<QListWidget*>(QStringLiteral("relateList")) : nullptr;
    QListWidgetItem* dovItem = nullptr;
    bool selfListed = false, abelDisabled = false;
    for (int i = 0; list != nullptr && i < list->count(); ++i) {
        QListWidgetItem* it = list->item(i);
        if (it->data(Qt::UserRole).toString() == dovID) { dovItem = it; }
        if (it->data(Qt::UserRole).toString() == caraID) { selfListed = true; }
        if (it->data(Qt::UserRole).toString() == abelID && kA.kind != QLatin1String("character")) {
            abelDisabled = !(it->flags() & Qt::ItemIsEnabled);
        }
    }
    check(dovItem != nullptr && (dovItem->flags() & Qt::ItemIsEnabled) && !selfListed,
          QStringLiteral("§8 Dov offered, Cara herself not"));
    check(kA.kind == QLatin1String("character") || abelDisabled, QStringLiteral("§8 a kind “sibling of” cannot point at is disabled"));
    if (dovItem != nullptr) { emit list->itemClicked(dovItem); }
    check(waitFor([&] { return relation(d, QStringLiteral("sibling of"), QStringLiteral("Dov")) != nullptr; }),
          QStringLiteral("§8 the relationship is created and listed"));
    bool fromDov = false;
    for (const QVariant& v : bridge.listEdgesFor(dir, dovID).value(QStringLiteral("edges")).toList()) {
        const QVariantMap m = v.toMap();
        if (m.value("otherID").toString() == caraID && m.value("label").toString() == QStringLiteral("sibling of")) { fromDov = true; }
    }
    check(fromDov, QStringLiteral("§8 … and Dov reads it from his end (C2)"));

    // ── §9 C3: in-window navigation ──────────────────────────────────────────────────────────────────────────────
    QSignalSpy sceneAsked(d.data(), &ObjectDetailDialog::showSceneRequested);
    if (QTreeWidgetItem* sceneRow = relation(d, QStringLiteral("appears in"), QString())) {
        emit sceneRow->treeWidget()->itemActivated(sceneRow, 0);
    }
    check(sceneAsked.count() == 1 && sceneAsked.at(0).at(0).toString() == sceneA, QStringLiteral("§9 a scene row asks for the scene"));
    if (QTreeWidgetItem* r = relation(d, QStringLiteral("sibling of"), QStringLiteral("Dov"))) { emit r->treeWidget()->itemActivated(r, 0); }
    check(showing(QStringLiteral("Dov")), QStringLiteral("§9 Dov opens in the same window"));
    check(d->windowTitle() == QStringLiteral("Dov") && action(d, "detailBack")->isVisible()
              && action(d, "detailBack")->toolTip().contains(QStringLiteral("Back to Cara")),
          QStringLiteral("§9 titled Dov; Back names Cara"));
    check(waitFor([&] { return relation(d, QStringLiteral("sibling of"), QStringLiteral("Cara")) != nullptr; }),
          QStringLiteral("§9 Dov's Related shows Cara (C2)"));
    part<QPlainTextEdit>(d, "detailNotes")->setPlainText(QStringLiteral("Unsaved Dov."));
    action(d, "detailBack")->trigger();
    QMessageBox* back = nullptr;
    check(waitFor([&] {
              back = d->findChild<QMessageBox*>(QStringLiteral("backPrompt"));
              return back != nullptr && back->isVisible();
          }, 3000),
          QStringLiteral("§9 Back with unsaved changes asks"));
    QStringList backButtons;
    for (QAbstractButton* b : back != nullptr ? back->buttons() : QList<QAbstractButton*>{}) { backButtons << b->text(); }
    check(backButtons == QStringList({QStringLiteral("Save"), QStringLiteral("Discard")}) || backButtons == QStringList({QStringLiteral("Discard"), QStringLiteral("Save")}),
          QStringLiteral("§9 … Save or Discard, no Cancel: %1").arg(backButtons.join(", ")));
    clickButton(back, QStringLiteral("Discard"));
    check(showing(QStringLiteral("Cara")) && !action(d, "detailBack")->isVisible(), QStringLiteral("§9 Discard goes back to Cara"));
    check(objectJson(bridge, dir, QStringLiteral("character"), dovID, worldID).value("notes").toString().isEmpty(),
          QStringLiteral("§9 … and writes nothing to Dov"));
    part<QPlainTextEdit>(d, "detailNotes")->setPlainText(QStringLiteral("Cara's note."));
    if (QTreeWidgetItem* r = relation(d, QStringLiteral("sibling of"), QStringLiteral("Dov"))) { emit r->treeWidget()->itemActivated(r, 0); }
    box = nullptr;
    check(waitFor([&] {
              box = d->findChild<QMessageBox*>(QStringLiteral("saveChangesPrompt"));
              return box != nullptr && box->isVisible();
          }, 3000),
          QStringLiteral("§9 opening another with unsaved changes asks"));
    clickButton(box, QStringLiteral("Cancel"));
    QTest::qWait(100);
    check(part<QLineEdit>(d, "detailName")->text() == QStringLiteral("Cara"), QStringLiteral("§9 Cancel stays"));
    if (QTreeWidgetItem* r = relation(d, QStringLiteral("sibling of"), QStringLiteral("Dov"))) { emit r->treeWidget()->itemActivated(r, 0); }
    box = nullptr;
    waitFor([&] {
        box = d->findChild<QMessageBox*>(QStringLiteral("saveChangesPrompt"));
        return box != nullptr && box->isVisible();
    }, 3000);
    clickButton(box, QStringLiteral("Save"));
    check(showing(QStringLiteral("Dov")), QStringLiteral("§9 Save, then Dov opens"));
    check(objectJson(bridge, dir, QStringLiteral("character"), caraID, worldID).value("notes").toString() == QStringLiteral("Cara's note."),
          QStringLiteral("§9 … Cara's note was saved first"));
    action(d, "detailBack")->trigger();
    check(showing(QStringLiteral("Cara")), QStringLiteral("§9 Back without changes asks nothing"));
    // Remove Relationship, from the row's menu.
    if (QTreeWidgetItem* r = relation(d, QStringLiteral("sibling of"), QStringLiteral("Dov"))) {
        auto* tree = r->treeWidget();
        emit tree->customContextMenuRequested(tree->visualItemRect(r).center());
    }
    menuAction(d, "relationMenu", QStringLiteral("Remove Relationship"));
    check(waitFor([&] { return relation(d, QStringLiteral("sibling of"), QStringLiteral("Dov")) == nullptr; }),
          QStringLiteral("§9 Remove Relationship deletes it"));

    // ── §10 C4: Sources ──────────────────────────────────────────────────────────────────────────────────────────
    check(part<QLabel>(d, "detailNoSources")->isVisible() && part<QLabel>(d, "detailNoSources")->text() == QStringLiteral("No sources cited yet."),
          QStringLiteral("§10 No sources cited yet."));
    part<QPushButton>(d, "detailNewSource")->click();
    QDialog* form = nullptr;
    check(waitFor([&] {
              form = d->findChild<QDialog*>(QStringLiteral("newSourceDialog"));
              return form != nullptr && form->isVisible();
          }, 3000),
          QStringLiteral("§10 New Source… asks for a title"));
    auto* create = form != nullptr ? form->findChild<QPushButton*>(QStringLiteral("newSourceCreate")) : nullptr;
    check(create != nullptr && !create->isEnabled(), QStringLiteral("§10 Create waits for a title"));
    if (form != nullptr) {
        form->findChild<QLineEdit*>(QStringLiteral("newSourceTitle"))->setText(QStringLiteral("Histories"));
        create->click();
    }
    check(waitFor([&] { return sources(d) == QStringList{QStringLiteral("Histories")}; }), QStringLiteral("§10 the new source is cited"));
    QString historiesID;
    for (const QVariant& v : bridge.listEdgesFor(dir, caraID).value(QStringLiteral("edges")).toList()) {
        const QVariantMap m = v.toMap();
        if (m.value("relationType").toString() == "cites" && m.value("from").toString() == m.value("otherID").toString()
            && m.value("otherKind").toString() == "source") {
            historiesID = m.value("otherID").toString();
        }
    }
    check(!historiesID.isEmpty(), QStringLiteral("§10 … a project source —cites→ Cara"));
    {   // Its citation fields, as a writer would have set them; and Dov cites it too.
        QJsonObject h = objectJson(bridge, dir, QStringLiteral("source"), historiesID, QString());
        h.insert(QStringLiteral("attributes"), QJsonArray{QJsonObject{{"k", "author"}, {"v", "Herodotus"}}, QJsonObject{{"k", "year"}, {"v", "c. 430 BC"}}});
        bridge.saveObject(dir, QStringLiteral("source"), QString::fromUtf8(QJsonDocument(h).toJson(QJsonDocument::Compact)));
        bridge.createEdge(dir, historiesID, dovID, QStringLiteral("cites"), QString());
    }
    {
        auto* list = part<QListWidget>(d, "detailSources");
        emit list->itemClicked(list->item(0));
    }
    QDialog* citation = nullptr;
    check(waitFor([&] {
              citation = d->findChild<QDialog*>(QStringLiteral("citationPopup"));
              return citation != nullptr && citation->isVisible();
          }, 3000),
          QStringLiteral("§10 a click shows the citation"));
    if (citation != nullptr) {
        QStringList texts;
        for (QLabel* l : citation->findChildren<QLabel*>()) { texts << l->text(); }
        check(texts.contains(QStringLiteral("Herodotus")) && texts.contains(QStringLiteral("c. 430 BC")) && texts.contains(QStringLiteral("Author")),
              QStringLiteral("§10 … with the fields that are set: %1").arg(texts.join(" | ")));
        check(citation->findChild<QLabel*>(QStringLiteral("citationAttribution"))->text() == QStringLiteral("Cited by Cara, and by Dov"),
              QStringLiteral("§10 … and who cites it: %1").arg(citation->findChild<QLabel*>(QStringLiteral("citationAttribution"))->text()));
        citation->close();
    }
    part<QPushButton>(d, "detailCite")->click();
    QDialog* cite = nullptr;
    check(waitFor([&] {
              cite = d->findChild<QDialog*>(QStringLiteral("citePicker"));
              return cite != nullptr && cite->isVisible();
          }, 3000),
          QStringLiteral("§10 Cite… opens the project's sources"));
    auto* citeList = cite != nullptr ? cite->findChild<QListWidget*>(QStringLiteral("citeList")) : nullptr;
    QListWidgetItem* annals = nullptr;
    bool historiesDisabled = false;
    for (int i = 0; citeList != nullptr && i < citeList->count(); ++i) {
        QListWidgetItem* it = citeList->item(i);
        if (it->data(Qt::UserRole).toString() == annalsID) { annals = it; }
        if (it->data(Qt::UserRole).toString() == historiesID) {
            historiesDisabled = !(it->flags() & Qt::ItemIsEnabled) && it->text().contains(QStringLiteral("Already cited here"));
        }
    }
    check(annals != nullptr && historiesDisabled, QStringLiteral("§10 … one already cited is disabled"));
    if (annals != nullptr) { emit citeList->itemClicked(annals); }
    check(waitFor([&] { return sources(d).contains(QStringLiteral("Annals")); }), QStringLiteral("§10 Cite… cites it"));
    {
        auto* list = part<QListWidget>(d, "detailSources");
        for (int i = 0; i < list->count(); ++i) {
            if (list->item(i)->text() == QStringLiteral("Annals")) {
                emit list->customContextMenuRequested(list->visualItemRect(list->item(i)).center());
            }
        }
    }
    menuAction(d, "sourceMenu", QStringLiteral("Remove Citation"));
    check(waitFor([&] { return sources(d) == QStringList{QStringLiteral("Histories")}; }), QStringLiteral("§10 Remove Citation"));
    {
        auto* list = part<QListWidget>(d, "detailSources");
        emit list->itemClicked(list->item(0));     // a real double-click is a click, then the activation
        emit list->itemActivated(list->item(0));
    }
    QTest::qWait(QApplication::doubleClickInterval() + 200);
    check(showing(QStringLiteral("Histories")), QStringLiteral("§10 a double-click opens the source in place"));
    check(part<QLabel>(d, "detailMetadata")->text().startsWith(QStringLiteral("Source")) && !part<QLabel>(d, "detailMetadata")->text().contains("World:"),
          QStringLiteral("§10 … a project-scoped source"));
    check(d->findChild<QDialog*>(QStringLiteral("citationPopup")) == nullptr, QStringLiteral("§10 a double-click does not also show the citation"));
    action(d, "detailBack")->trigger();
    check(showing(QStringLiteral("Cara")), QStringLiteral("§10 Back to Cara"));

    // ── §12 read-only: the graph is frozen while the world is away ───────────────────────────────────────────────
    QDir().rename(worldPath, worldPath + QStringLiteral(".away"));
    w->editorShell()->refreshWorlds();
    check(waitFor([&] { return part<QLabel>(d, "detailReadOnly")->isVisible(); }), QStringLiteral("§12 read-only"));
    check(!part<QPushButton>(d, "detailRelate")->isEnabled() && !part<QPushButton>(d, "detailCite")->isEnabled()
              && !part<QPushButton>(d, "detailNewSource")->isEnabled()
              && part<QPushButton>(d, "detailRelate")->toolTip().contains(QStringLiteral("frozen")),
          QStringLiteral("§12 Relate…, Cite… and New Source… are off, with the reason"));
    QDir().rename(worldPath + QStringLiteral(".away"), worldPath);
    w->editorShell()->refreshWorlds();
    check(waitFor([&] { return !part<QLabel>(d, "detailReadOnly")->isVisible(); }), QStringLiteral("§12 the world is back"));
    action(d, "detailClose")->trigger();
    check(closed(), QStringLiteral("§12 closes"));

    // ── §11 C4: the scene's Sources card ─────────────────────────────────────────────────────────────────────────
    inspector->findChild<QTabWidget*>()->setCurrentIndex(0);   // Writing
    auto* add = inspector->findChild<QToolButton*>(QStringLiteral("writingAddCard"));
    emit add->menu()->aboutToShow();
    for (QAction* a : add->menu()->actions()) {
        if (a->text() == QStringLiteral("Sources")) { a->trigger(); }
    }
    auto* card = inspector->findChild<QListWidget*>(QStringLiteral("sourcesList"));
    check(waitFor([&] { return card != nullptr && card->isVisible() && card->count() == 1; }),
          QStringLiteral("§11 the Sources card lists the scene's objects' sources"));
    check(card != nullptr && card->count() == 1 && card->item(0)->text() == QStringLiteral("Histories\nvia Cara"),
          QStringLiteral("§11 … “Histories”, via Cara: %1").arg(card != nullptr && card->count() > 0 ? card->item(0)->text() : QString()));
    if (card != nullptr && card->count() > 0) { emit card->itemActivated(card->item(0)); }
    waitFor([&] {
        d = w->findChild<ObjectDetailDialog*>();
        return d != nullptr && d->isVisible();
    }, 3000);
    check(showing(QStringLiteral("Histories")), QStringLiteral("§11 a double-click opens the source's detail"));
    if (d != nullptr) { action(d, "detailClose")->trigger(); }
    check(closed(), QStringLiteral("§11 closes"));

    if (failures == 0) { std::printf("object-detail-ok\n"); }
    return failures == 0 ? 0 : 1;
}
