#include "RecentsStore.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace {
constexpr auto kPath       = "path";
constexpr auto kTitle      = "title";
constexpr auto kLastOpened = "lastOpened";
} // namespace

RecentsStore::RecentsStore(QObject* parent) : QObject(parent) {}

void RecentsStore::configure(const QString& appSupportRoot)
{
    filePath_ = QDir(appSupportRoot).filePath(QStringLiteral("recents.json"));
    load();
}

void RecentsStore::addOrUpdate(const QString& path, const QString& title)
{
    if (path.isEmpty()) {
        return;
    }

    // Drop any existing entry for this path so the refreshed one lands at front.
    for (int i = 0; i < entries_.size(); ++i) {
        if (entries_.at(i).toMap().value(QLatin1String(kPath)).toString() == path) {
            entries_.removeAt(i);
            break;
        }
    }

    QVariantMap entry;
    entry.insert(QLatin1String(kPath), path);
    entry.insert(QLatin1String(kTitle), title);
    entry.insert(QLatin1String(kLastOpened),
                 QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    entries_.prepend(entry);
    save();
    emit entriesChanged();
}

void RecentsStore::remove(const QString& path)
{
    bool changed = false;
    for (int i = 0; i < entries_.size(); ++i) {
        if (entries_.at(i).toMap().value(QLatin1String(kPath)).toString() == path) {
            entries_.removeAt(i);
            changed = true;
            break;
        }
    }
    if (changed) {
        save();
        emit entriesChanged();
    }
}

void RecentsStore::load()
{
    entries_.clear();

    QFile file(filePath_);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        // Missing file is normal on first launch — start empty.
        emit entriesChanged();
        return;
    }

    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        // Corrupt file — tolerate it (start empty), don't crash.
        emit entriesChanged();
        return;
    }

    for (const QJsonValue& v : doc.array()) {
        if (!v.isObject()) {
            continue;
        }
        const QJsonObject o = v.toObject();
        const QString path = o.value(QLatin1String(kPath)).toString();
        if (path.isEmpty()) {
            continue;   // skip malformed rows
        }
        QVariantMap entry;
        entry.insert(QLatin1String(kPath), path);
        entry.insert(QLatin1String(kTitle), o.value(QLatin1String(kTitle)).toString());
        entry.insert(QLatin1String(kLastOpened),
                     o.value(QLatin1String(kLastOpened)).toString());
        entries_.append(entry);
    }

    emit entriesChanged();
}

void RecentsStore::save() const
{
    QJsonArray arr;
    for (const QVariant& v : entries_) {
        const QVariantMap m = v.toMap();
        QJsonObject o;
        o.insert(QLatin1String(kPath), m.value(QLatin1String(kPath)).toString());
        o.insert(QLatin1String(kTitle), m.value(QLatin1String(kTitle)).toString());
        o.insert(QLatin1String(kLastOpened),
                 m.value(QLatin1String(kLastOpened)).toString());
        arr.append(o);
    }

    // Atomic write so a crash mid-save never corrupts the recents file.
    QSaveFile out(filePath_);
    if (!out.open(QIODevice::WriteOnly)) {
        return;   // best-effort; a failed persist is not fatal to the session
    }
    out.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    out.commit();
}
