#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <qqmlintegration.h>

// RecentsStore — the recent-projects store for the Linux app (SP-059 / T-0224).
//
// The Qt analogue of Apple's ProjectBookmarkStore. There is NO scrivi_* recents
// endpoint — recents are an app-side concern — so this owns the whole thing.
//
// State is a JSON file (recents.json) under appSupportRoot holding an ordered
// list of {path, title, lastOpened} entries, newest-first. Because Linux has no
// security-scoped bookmarks, entries are plain absolute paths.
//
// Operations:
//   • addOrUpdate(path, title) — moves the path to the front, refreshes
//     lastOpened (add if new). Called on create (and, in SP-060, on open).
//   • remove(path) — drops an entry by path.
//   • entries — the current list, newest-first, exposed to QML as a model.
//
// A missing or corrupt file is tolerated: the store starts empty and never
// crashes. Must round-trip across process restarts (asserted headless, T-0228).
class RecentsStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    // The recents list, newest-first. Each element is a QVariantMap with keys
    // {path, title, lastOpened}. Bound by Landing.qml's ListView model.
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)

public:
    explicit RecentsStore(QObject* parent = nullptr);

    // Points the store at appSupportRoot and loads recents.json from it (or
    // starts empty if absent/corrupt). Call once at launch before use.
    Q_INVOKABLE void configure(const QString& appSupportRoot);

    QVariantList entries() const { return entries_; }

    // Adds `path` (or moves it to the front if already present), refreshing its
    // title and lastOpened timestamp, then persists. lastOpened is an ISO-8601
    // UTC string.
    Q_INVOKABLE void addOrUpdate(const QString& path, const QString& title);

    // Removes the entry with the given path (if any) and persists.
    Q_INVOKABLE void remove(const QString& path);

signals:
    void entriesChanged();

private:
    void load();
    void save() const;

    QString      filePath_;   // <appSupportRoot>/recents.json
    QVariantList entries_;     // newest-first {path,title,lastOpened} maps
};
