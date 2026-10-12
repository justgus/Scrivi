#include "BufferService.hpp"

#include "ManuscriptEscapes.hpp"
#include "ScriviBridge.hpp"

#include <QVariantList>
#include <QVariantMap>

const QStringList& BufferService::slotIDs()
{
    static const QStringList ids{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"),
                                 QStringLiteral("4"), QStringLiteral("5"), QStringLiteral("6"),
                                 QStringLiteral("7"), QStringLiteral("8"), QStringLiteral("9")};
    return ids;
}

BufferService::BufferService(const QString& projectRootPath, QObject* parent)
    : QObject(parent), root_(projectRootPath)
{
    refresh();
}

void BufferService::refresh()
{
    const QVariantMap listed = ScriviBridge::buffersList(root_);
    // ✅ A success always carries `count` (0 for an empty list), so {} is a failed call — which keeps the previous
    // mirror, as Apple's `try?`. (⚠️ `buffers` itself is absent for an empty list: `project_envelope_empty_vs_failed`.)
    if (!listed.contains(QStringLiteral("count"))) {
        return;
    }
    QList<Slot> listedSlots;
    for (const QVariant& v : listed.value(QStringLiteral("buffers")).toList()) {
        const QVariantMap m = v.toMap();
        listedSlots.append({m.value(QStringLiteral("bufferID")).toString(), m.value(QStringLiteral("text")).toString(),
                      m.value(QStringLiteral("updatedAt")).toString(), m.contains(QStringLiteral("fragment"))});
    }
    slots_ = listedSlots;
    emit changed();
}

const BufferService::Slot* BufferService::find(const QString& bufferID) const
{
    for (const Slot& s : slots_) {
        if (s.bufferID == bufferID) {
            return &s;
        }
    }
    return nullptr;
}

QString BufferService::text(const QString& bufferID) const
{
    const Slot* s = find(bufferID);
    return s != nullptr ? s->text : QString();
}

QString BufferService::updatedAt(const QString& bufferID) const
{
    const Slot* s = find(bufferID);
    return s != nullptr ? s->updatedAt : QString();
}

bool BufferService::isStructured(const QString& bufferID) const
{
    const Slot* s = find(bufferID);
    return s != nullptr && s->structured;
}

QString BufferService::preview(const QString& bufferID) const
{
    // EP-045 AC4: a slot holds STORED text; the palette shows what the writer sees (Apple: `preview(forSlot:)`).
    const QString presented = ManuscriptEscapes::map(text(bufferID)).presented;
    for (const QString& line : presented.split(QLatin1Char('\n'))) {
        if (!line.trimmed().isEmpty()) {
            return line.trimmed();
        }
    }
    return {};
}

bool BufferService::load(const QString& text, const QString& bufferID)
{
    if (ScriviBridge::buffersLoad(root_, bufferID, text).isEmpty()) {
        return false;   // {} = the call failed (logged by the bridge); a success carries {bufferID, updatedAt}
    }
    refresh();
    return true;
}

bool BufferService::clear(const QString& bufferID)
{
    const bool cleared = ScriviBridge::buffersClear(root_, bufferID).value(QStringLiteral("cleared")).toBool();
    refresh();
    return cleared;
}
