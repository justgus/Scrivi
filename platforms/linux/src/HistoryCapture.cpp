#include "HistoryCapture.hpp"

#include "ScriviBridge.hpp"

#include <QDateTime>
#include <QElapsedTimer>
#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <algorithm>
#include <climits>

HistoryCapture::HistoryCapture(const QString& projectRootPath, QObject* parent)
    : QObject(parent), root_(projectRootPath), idleTimer_(new QTimer(this))
{
    // Fires the idle boundary on its own, at the core's deadline — so a writer who stops mid-sentence and walks away
    // still gets their last sentence sealed. A stale firing does nothing (the core checks the time).
    idleTimer_->setSingleShot(true);
    connect(idleTimer_, &QTimer::timeout, this, [this]() {
        if (open_) {
            take(ScriviBridge::captureIdle(root_, nowMs()));
        }
    });
}

HistoryCapture::~HistoryCapture() = default;

qint64 HistoryCapture::nowMs()
{
    static const QElapsedTimer clock = [] { QElapsedTimer t; t.start(); return t; }();
    return clock.elapsed();
}

QString HistoryCapture::friendlyTimestamp(const QString& iso8601)
{
    QDateTime when = QDateTime::fromString(iso8601, Qt::ISODateWithMs);
    if (!when.isValid()) {
        when = QDateTime::fromString(iso8601, Qt::ISODate);
    }
    if (!when.isValid()) {
        return {};
    }
    when = when.toLocalTime();
    const QLocale locale;
    const QString clock = locale.toString(when.time(), QLocale::ShortFormat);
    const QDate today = QDate::currentDate();
    if (when.date() == today) {
        return tr("earlier today at %1").arg(clock);
    }
    if (when.date() == today.addDays(-1)) {
        return tr("yesterday at %1").arg(clock);
    }
    return tr("on %1 at %2").arg(locale.toString(when.date(), QStringLiteral("MMM d")), clock);
}

void HistoryCapture::take(const QVariantMap& r)
{
    if (r.isEmpty()) {
        return;   // a failed call (logged by the bridge) — keep the last known state
    }
    canUndo_ = r.value(QStringLiteral("canUndo")).toBool();
    atBarrier_ = r.value(QStringLiteral("atBarrier")).toBool();
    canRedo_ = r.value(QStringLiteral("canRedo")).toBool();
    hasPending_ = r.value(QStringLiteral("hasPending")).toBool();
    undoCrossesSessionAt_ = r.value(QStringLiteral("undoCrossesSessionAt")).toString();
    idleTimer_->stop();
    const qint64 deadline = r.value(QStringLiteral("idleDeadlineMs"), -1).toLongLong();
    if (deadline >= 0) {
        idleTimer_->start(static_cast<int>(std::clamp<qint64>(deadline - nowMs(), 0, INT_MAX)));
    }
}

void HistoryCapture::refreshState()
{
    take(ScriviBridge::captureState(root_));
}

QString HistoryCapture::undoEntersEarlierSession()
{
    if (!open_) {
        return {};
    }
    refreshState();
    return undoCrossesSessionAt_;
}

void HistoryCapture::adoptOpened(bool opened)
{
    open_ = opened;
    if (open_) {
        refreshState();
    }
}

void HistoryCapture::close()
{
    if (!open_) {
        return;
    }
    // `scrivi_history_close` commits the pending typing first, so the final sentence is not lost (SP-177).
    // ⛔ Do NOT report the pending text as persisted here (Apple's 2026-08-10 lesson, `HistoryCapture.swift`): on quit it
    // may never have been written, and a recorded hash of unwritten text guarantees an external-change barrier.
    idleTimer_->stop();
    ScriviBridge::historyClose(root_);
    open_ = false;
    canUndo_ = canRedo_ = hasPending_ = atBarrier_ = false;
    undoCrossesSessionAt_.clear();
}

void HistoryCapture::noteEdit(const QString& sceneID, const QString& text, int caretUtf16, const QString& textBefore)
{
    if (!open_ || applying_) {
        return;
    }
    take(ScriviBridge::captureNoteEdit(root_, sceneID, text, caretUtf16, textBefore, nowMs()));
}

void HistoryCapture::caretMoved(const QString& sceneID, bool editInFlight)
{
    if (!open_) {
        return;
    }
    take(ScriviBridge::captureCaretMoved(root_, sceneID, editInFlight, applying_));
}

void HistoryCapture::beginPasteOrCut(const QString& kind, const QString& bufferID)
{
    if (!open_) {
        return;
    }
    take(ScriviBridge::captureBegin(root_, kind, bufferID));
}

void HistoryCapture::flush(const QString& trigger, const QString& kind)
{
    if (!open_) {
        return;
    }
    take(ScriviBridge::captureFlush(root_, trigger, kind));
}

void HistoryCapture::groupedEdit(const QString& editsJson, const QString& kind)
{
    if (!open_ || applying_) {
        return;
    }
    take(ScriviBridge::captureGroupedEdit(root_, editsJson, kind));
}

void HistoryCapture::beforeSave()
{
    if (!open_) {
        return;
    }
    take(ScriviBridge::captureBeforeSave(root_, nowMs()));
}

void HistoryCapture::noteScenePersisted(const QString& sceneID, const QString& diskText)
{
    if (!open_ || sceneID.isEmpty()) {
        return;
    }
    // Deliberately NOT a commit: it does not seal the pending typing (T-0396).
    ScriviBridge::historyNoteScenePersisted(root_, sceneID, diskText);
}

void HistoryCapture::recordBarrier(const QString& kind, const QString& note, const QHash<QString, QString>& sceneTexts,
                                   const QJsonObject& structuralPayload, const QHash<QString, QString>& sceneTextsBefore)
{
    if (!open_) {
        return;
    }
    take(ScriviBridge::captureRecordBarrier(root_, barrierParams(kind, note, sceneTexts, structuralPayload, sceneTextsBefore)));
}

QString HistoryCapture::barrierParams(const QString& kind, const QString& note, const QHash<QString, QString>& sceneTexts,
                                      const QJsonObject& structuralPayload, const QHash<QString, QString>& sceneTextsBefore)
{
    QJsonObject params{{QStringLiteral("barrierKind"), kind}, {QStringLiteral("note"), note}};
    const auto textsOf = [](const QHash<QString, QString>& h) {
        QJsonObject o;
        for (auto it = h.cbegin(); it != h.cend(); ++it) {
            o.insert(it.key(), it.value());
        }
        return o;
    };
    if (!sceneTexts.isEmpty()) {
        params.insert(QStringLiteral("sceneTexts"), textsOf(sceneTexts));
    }
    if (!structuralPayload.isEmpty()) {
        params.insert(QStringLiteral("structuralPayload"), structuralPayload);
        if (!sceneTextsBefore.isEmpty()) {
            params.insert(QStringLiteral("sceneTextsBefore"), textsOf(sceneTextsBefore));
        }
    }
    return QString::fromUtf8(QJsonDocument(params).toJson(QJsonDocument::Compact));
}

QVariantMap HistoryCapture::undo()
{
    if (!open_) {
        return {};
    }
    // The core commits any pending edit first, so the newest keystrokes are not discarded.
    const QVariantMap r = ScriviBridge::captureUndo(root_);
    refreshState();
    return r;
}

QVariantMap HistoryCapture::redo()
{
    if (!open_) {
        return {};
    }
    const QVariantMap r = ScriviBridge::captureRedo(root_);
    refreshState();
    return r;
}

void HistoryCapture::noteApplied(const QString& sceneID, const QString& text)
{
    if (!open_) {
        return;
    }
    take(ScriviBridge::captureNoteApplied(root_, sceneID, text));
}
