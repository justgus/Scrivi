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
    bridge.createProject(projectPath, appSupport,
                         QStringLiteral("Tintagael Live Pass"),
                         QStringLiteral("tintagael-live"));
    const QVariantMap opened = bridge.openProject(projectPath, appSupport);
    const QString sceneID = opened.value("scenes").toList().at(0).toMap()
                                .value("sceneID").toString();

    // ⚠️ MOST kinds are world-scoped, so a fixture without a world would relate
    // only `source` objects and show ONE group — which would not exercise the
    // grouping at all. Build a world alongside the project so world-scoped kinds
    // are creatable too.
    const QString worldPath = projectPath + QStringLiteral(".worlds/Eskandar.scrivworld");
    const QVariantMap world = bridge.createWorld(
        projectPath, worldPath, QStringLiteral("Eskandar"), QStringLiteral("AE"));
    const QString worldID = world.value("worldID").toString();
    std::fprintf(stderr, "world: %s\n",
                 worldID.isEmpty() ? "(none)" : worldID.toUtf8().constData());

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
        }
    }
    std::fprintf(stderr, "scene: %s\n", sceneID.toUtf8().constData());
    return 0;
}
