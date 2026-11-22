// inspector_fixture — EP-035 / SP-125: builds a project whose first scene has
// several objects of DIFFERENT kinds related to it, so the Scene Inspector has
// real grouping to show during the AC9 LIVE click-through.
//
// ⚠️ This is a FIXTURE BUILDER, not a test — it asserts nothing. It exists so
// the live pass runs against real data rather than an empty project, because an
// empty panel would exercise only one of the four states T-0483 distinguishes.
//
// ⚠️ Creates its own throwaway project; NEVER point it at real writing work
// (I-0150). Built through ScriviBridge, like every other Linux harness.
//
// The kinds it uses are read from ObjectKindScope — i.e. from the core — so this
// file names no kind and does not become another restatement.
//
//   argv[1] = project dir to create
#include <QCoreApplication>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>
#include <cstdio>
#include "AppSupport.hpp"
#include "ObjectKindScope.hpp"
#include "ScriviBridge.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    const QString projectPath = QString::fromUtf8(argv[1]);
    const QString appSupport  = scrivi::linux_app::appSupportRoot();

    ScriviBridge bridge;
    bridge.bootstrap(QStringLiteral("Fixture Builder"), appSupport);
    const QString projectID = bridge.createProject(projectPath, appSupport,
                         QStringLiteral("Tintagael Live Pass"),
                         QStringLiteral("tintagael-live")).value("projectID").toString();
    const QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QString sceneID = opened.value("scenes").toList().at(0).toMap()
                                .value("sceneID").toString();

    // ⚠️ MOST kinds are world-scoped, so a fixture without a world would relate
    // only `source` objects and show ONE group — which would not exercise the
    // grouping at all. Build a world alongside the project so world-scoped kinds
    // are creatable too.
    const QString worldPath = projectPath + QStringLiteral(".worlds/Eskandar.inkwynworld");
    const QVariantMap world = bridge.createWorld(
        projectPath, worldPath, QStringLiteral("Eskandar"), QStringLiteral("AE"));
    const QString worldID = world.value("worldID").toString();
    std::fprintf(stderr, "world: %s\n",
                 worldID.isEmpty() ? "(none)" : worldID.toUtf8().constData());

    bool imaged = false;   // [SP-234] A6: the first world object gets an image (Linux cannot import one until B2)
    for (const ObjectKindScope::KindInfo& k : ObjectKindScope::kinds(&bridge)) {
        // ⚠️ worldID is threaded from the kind's OWN scope, read from the core —
        // EMPTY for a project-scoped kind, the world's ID for a world-scoped one.
        // Getting this backwards is exactly what SP-104 did, and it blocked
        // object creation outright.
        const QString objectWorldID = k.isWorldScoped ? worldID : QString();
        if (k.isWorldScoped && worldID.isEmpty()) continue;

        for (int i = 1; i <= 2; ++i) {
            const QString name = QStringLiteral("%1 %2").arg(k.kind).arg(i);
            const QVariantMap made = bridge.createObject(
                projectPath, k.kind, name, QString(), objectWorldID);
            const QString objectID = made.value("objectID").toString();
            if (objectID.isEmpty()) {
                std::fprintf(stderr, "  (skipped %s)\n", name.toUtf8().constData());
                continue;
            }
            bridge.createEdge(projectPath, objectID, sceneID,
                              QStringLiteral("cites"), QString());
            std::fprintf(stderr, "related %s (%s)\n",
                         name.toUtf8().constData(), k.kind.toUtf8().constData());
            if (!imaged && k.isWorldScoped) {
                const QString png = projectPath + QStringLiteral(".worlds/fixture-image.png");
                QImage image(64, 64, QImage::Format_ARGB32);
                image.fill(QColor(30, 120, 140));
                image.save(png);
                const QString assetID = bridge.importAsset(projectPath, png, QStringLiteral("image"), name, worldID, projectID)
                                            .value("assetID").toString();
                QJsonObject json = QJsonDocument::fromJson(bridge.openObject(projectPath, k.kind, objectID, worldID)
                                                               .value("objectJson").toString().toUtf8()).object();
                json.insert(QStringLiteral("image"), QJsonObject{{QStringLiteral("assetID"), assetID}});
                bridge.saveObject(projectPath, k.kind, QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
                std::fprintf(stderr, "  image on %s\n", name.toUtf8().constData());
                imaged = !assetID.isEmpty();
            }
        }
    }
    std::fprintf(stderr, "scene: %s\n", sceneID.toUtf8().constData());
    return 0;
}
