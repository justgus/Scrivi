#include "SessionStore.hpp"

#include <QSettings>
#include <QStringList>
#include <QVariant>

namespace {

// ⚠️ `[project/<projectID>]` — [R-Q2]. ✅ The same shape `timeline-view.ini` uses
// (`timeline/<projectID>`), so a reader of one recognises the other.
constexpr auto kGroupPrefix = "project/";
constexpr auto kKeyPath      = "path";
constexpr auto kKeyTitle     = "title";
constexpr auto kKeyOpen      = "open";
constexpr auto kKeyFrame     = "frame";
constexpr auto kKeyMaximized = "maximized";
constexpr auto kKeyPanes     = "panes";
constexpr auto kKeyOuter     = "outer";

// ⚠️ Splitter sizes are stored as a comma-joined string rather than a QVariantList:
// ✅ it round-trips through INI unambiguously and stays human-readable, ⛔ where a
// QVariantList writes as an opaque `@Variant(...)` blob nobody can inspect or repair.
QString joinSizes(const QList<int>& sizes)
{
    QStringList parts;
    parts.reserve(sizes.size());
    for (const int s : sizes) {
        parts << QString::number(s);
    }
    return parts.join(QLatin1Char(','));
}

QList<int> splitSizes(const QString& text)
{
    QList<int> out;
    if (text.isEmpty()) {
        return out;
    }
    for (const QString& part : text.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        bool ok = false;
        const int v = part.toInt(&ok);
        // ⚠️ A malformed entry yields NOTHING rather than a partial list: ⛔ half a
        // set of splitter sizes would lay the panes out wrongly, which is worse than
        // falling back to the defaults.
        if (!ok) {
            return {};
        }
        out.append(v);
    }
    return out;
}

QString frameToString(const QRect& r)
{
    return QStringLiteral("%1,%2,%3,%4")
        .arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
}

QRect frameFromString(const QString& text)
{
    const QStringList parts = text.split(QLatin1Char(','), Qt::SkipEmptyParts);
    if (parts.size() != 4) {
        return {};   // ⚠️ invalid — the caller keeps the window's default geometry
    }
    bool ok1 = false, ok2 = false, ok3 = false, ok4 = false;
    const int x = parts[0].toInt(&ok1);
    const int y = parts[1].toInt(&ok2);
    const int w = parts[2].toInt(&ok3);
    const int h = parts[3].toInt(&ok4);
    if (!ok1 || !ok2 || !ok3 || !ok4 || w <= 0 || h <= 0) {
        return {};
    }
    return QRect(x, y, w, h);
}

}  // namespace

QString SessionStore::filePath() const
{
    if (appSupportRoot_.isEmpty()) {
        return {};
    }
    return appSupportRoot_ + QStringLiteral("/session.ini");
}

QList<SessionStore::Entry> SessionStore::entries() const
{
    QList<Entry> out;
    const QString path = filePath();
    if (path.isEmpty()) {
        return out;
    }
    QSettings settings(path, QSettings::IniFormat);
    for (const QString& group : settings.childGroups()) {
        if (!group.startsWith(QLatin1String("project"))) {
            continue;
        }
        // ⚠️ QSettings nests `project/<id>` as a CHILD GROUP of `project`, so the
        // ids come from inside that group, not from the top-level list.
        settings.beginGroup(group);
        for (const QString& id : settings.childGroups()) {
            settings.beginGroup(id);
            Entry e;
            e.projectID  = id;
            e.path       = settings.value(QLatin1String(kKeyPath)).toString();
            e.title      = settings.value(QLatin1String(kKeyTitle)).toString();
            e.frame      = frameFromString(settings.value(QLatin1String(kKeyFrame)).toString());
            e.maximized  = settings.value(QLatin1String(kKeyMaximized), false).toBool();
            e.paneSizes  = splitSizes(settings.value(QLatin1String(kKeyPanes)).toString());
            e.outerSizes = splitSizes(settings.value(QLatin1String(kKeyOuter)).toString());
            settings.endGroup();
            out.append(e);
        }
        settings.endGroup();
    }
    return out;
}

SessionStore::Entry SessionStore::entry(const QString& projectID) const
{
    for (const Entry& e : entries()) {
        if (e.projectID == projectID) {
            return e;
        }
    }
    return {};
}

void SessionStore::record(const Entry& e)
{
    const QString path = filePath();
    // ⚠️ An empty projectID is IGNORED — ✅ the same guard `OpenProjectRegistry`
    // applies. ⛔ A failed load has no identity and must not become a phantom row
    // that the next restore tries to reopen.
    if (path.isEmpty() || e.projectID.isEmpty()) {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QLatin1String(kGroupPrefix) + e.projectID);
    settings.setValue(QLatin1String(kKeyPath), e.path);
    settings.setValue(QLatin1String(kKeyOpen), true);
    if (!e.title.isEmpty()) {
        settings.setValue(QLatin1String(kKeyTitle), e.title);
    }
    if (e.frame.isValid()) {
        settings.setValue(QLatin1String(kKeyFrame), frameToString(e.frame));
    }
    settings.setValue(QLatin1String(kKeyMaximized), e.maximized);
    if (!e.paneSizes.isEmpty()) {
        settings.setValue(QLatin1String(kKeyPanes), joinSizes(e.paneSizes));
    }
    if (!e.outerSizes.isEmpty()) {
        settings.setValue(QLatin1String(kKeyOuter), joinSizes(e.outerSizes));
    }
    settings.endGroup();
    // ⚠️ FLUSH NOW — ⛔ a `--rm` container may SIGKILL before Qt's lazy flush.
    // ✅ Same habit and same reason as `onTimelineViewStateChanged` (`:2906`).
    settings.sync();
}

void SessionStore::setOpen(const QString& projectID, const QString& path,
                           const QString& title)
{
    const QString file = filePath();
    if (file.isEmpty() || projectID.isEmpty()) {
        return;
    }
    QSettings settings(file, QSettings::IniFormat);
    settings.beginGroup(QLatin1String(kGroupPrefix) + projectID);
    settings.setValue(QLatin1String(kKeyPath), path);
    if (!title.isEmpty()) {
        settings.setValue(QLatin1String(kKeyTitle), title);
    }
    settings.setValue(QLatin1String(kKeyOpen), true);
    settings.endGroup();
    settings.sync();
}

void SessionStore::setClosed(const QString& projectID)
{
    const QString path = filePath();
    if (path.isEmpty() || projectID.isEmpty()) {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QLatin1String(kGroupPrefix) + projectID);
    // ⛔ ONLY THE `open` FLAG CHANGES. ⚠️ Geometry and splitters are DELIBERATELY
    // left behind — [R-Q2]: a project the writer closes today should reopen exactly
    // where she left it tomorrow. ✅ Removing the group here would discard that.
    settings.setValue(QLatin1String(kKeyOpen), false);
    settings.endGroup();
    settings.sync();
}

QList<QString> SessionStore::openProjectIDs() const
{
    QList<QString> out;
    const QString path = filePath();
    if (path.isEmpty()) {
        return out;
    }
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QLatin1String("project"));
    for (const QString& id : settings.childGroups()) {
        settings.beginGroup(id);
        if (settings.value(QLatin1String(kKeyOpen), false).toBool()) {
            out.append(id);
        }
        settings.endGroup();
    }
    settings.endGroup();
    return out;
}

QRect SessionStore::landingFrame() const
{
    const QString path = filePath();
    if (path.isEmpty()) {
        return {};
    }
    QSettings settings(path, QSettings::IniFormat);
    return frameFromString(settings.value(QStringLiteral("landing/frame")).toString());
}

bool SessionStore::landingMaximized() const
{
    const QString path = filePath();
    if (path.isEmpty()) {
        return false;
    }
    QSettings settings(path, QSettings::IniFormat);
    return settings.value(QStringLiteral("landing/maximized"), false).toBool();
}

void SessionStore::recordLanding(const QRect& frame, bool maximized)
{
    const QString path = filePath();
    if (path.isEmpty()) {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    if (frame.isValid()) {
        settings.setValue(QStringLiteral("landing/frame"), frameToString(frame));
    }
    settings.setValue(QStringLiteral("landing/maximized"), maximized);
    // ⚠️ FLUSH NOW — same reason as every other write here.
    settings.sync();
}

void SessionStore::clearAll()
{
    const QString path = filePath();
    if (path.isEmpty()) {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    settings.clear();
    settings.sync();
}
