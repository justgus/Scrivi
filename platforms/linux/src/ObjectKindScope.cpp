#include "ObjectKindScope.hpp"

#include "ScriviBridge.hpp"

#include <QVariantList>
#include <QVariantMap>

namespace ObjectKindScope {
namespace {

// Process-wide cache. A kind's scope is a property of the MODEL, not of a
// project, so this is deliberately not keyed by project root — see the header.
struct Table {
    QList<KindInfo>    kinds;
    QHash<QString, bool> worldScoped;
    bool loaded = false;
};

Table& table()
{
    static Table t;
    return t;
}

// Ask the core once. A failed call leaves `loaded` false so a later call can
// retry — the bridge is not ready until identity bootstraps, and a surface may
// well ask before that.
void loadIfNeeded(ScriviBridge* bridge)
{
    Table& t = table();
    if (t.loaded || bridge == nullptr) {
        return;
    }

    const QVariantMap result = bridge->listObjectKinds();
    if (result.isEmpty()) {
        // ⚠️ Deliberately leaves the table EMPTY rather than filling in a
        // hardcoded partition. An empty table sends every kind through the
        // world-scoped path, which fails visibly instead of corrupting scope.
        return;
    }

    const QVariantList entries = result.value(QStringLiteral("kinds")).toList();
    for (const QVariant& entry : entries) {
        const QVariantMap m = entry.toMap();
        KindInfo info;
        info.kind          = m.value(QStringLiteral("kind")).toString();
        info.subdir        = m.value(QStringLiteral("subdir")).toString();
        info.isWorldScoped = m.value(QStringLiteral("isWorldScoped")).toBool();
        if (info.kind.isEmpty()) {
            continue;
        }
        // ⚠️ A duplicate kind resolves to world-scoped (||), never to the last
        // value seen: the same conservative direction as the unknown-kind
        // fallback, for the same reason. (Apple's Swift original had to make
        // this same choice — its Dictionary(uniqueKeysWithValues:) would have
        // TRAPPED, crashing the app outright.)
        const bool prior = t.worldScoped.value(info.kind, false);
        t.worldScoped.insert(info.kind, prior || info.isWorldScoped);
        t.kinds.append(info);
    }

    t.loaded = !t.kinds.isEmpty();
}

} // namespace

const QList<KindInfo>& kinds(ScriviBridge* bridge)
{
    loadIfNeeded(bridge);
    return table().kinds;
}

bool isWorldScoped(ScriviBridge* bridge, const QString& kind)
{
    loadIfNeeded(bridge);
    // Unknown kind → true. Fails loudly (asks for a world) rather than silently
    // writing an object to the wrong scope.
    return table().worldScoped.value(kind, true);
}

bool isLoaded(ScriviBridge* bridge)
{
    loadIfNeeded(bridge);
    return table().loaded;
}

} // namespace ObjectKindScope
