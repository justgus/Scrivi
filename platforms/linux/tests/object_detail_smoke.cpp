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
    const QString sceneA = bridge.openProject(dir, appSupport).value(QStringLiteral("scenes")).toList().at(0).toMap()
                               .value(QStringLiteral("sceneID")).toString();
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
    check(!worldAssets(bridge, dir, worldID).contains(firstID), QStringLiteral("§5 … and the old asset removed"));
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

    if (failures == 0) { std::printf("object-detail-ok\n"); }
    return failures == 0 ? 0 : 1;
}
