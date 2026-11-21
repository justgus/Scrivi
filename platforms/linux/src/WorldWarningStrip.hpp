#pragma once

#include <QCoreApplication>
#include <QFrame>
#include <QList>
#include <QSet>
#include <QString>

#include <functional>

class QLabel;

// WorldWarningStrip — [EP-059] D3 / [SP-231] (Apple: `WorldWarningView.swift`, EP-031 AC23/AC24).
//
// The project-wide notice that a bound world is unavailable: one line per world, the links held pending toward it,
// Manage Worlds… and ×. Passive and non-blocking — it never takes focus, and nothing destructive lives here (Doc 3 §4.6).
// ✅ Laid OVER the bottom edge of its parent (the manuscript column), as Apple's overlay ([I-0263]; SP-231 Q3): its
// appearing moves nothing. It places itself on every resize of the parent.
class WorldWarningStrip : public QFrame {
    Q_DECLARE_TR_FUNCTIONS(WorldWarningStrip)

public:
    // One unavailable world and what is waiting on it.
    struct Row {
        QString worldID;
        QString displayName;
        QString status;          // core WorldStatus name
        int     pendingCount = 0;
    };

    WorldWarningStrip(QWidget* parent, std::function<void()> onManageWorlds);

    // A fresh read. ✅ A world not flagged before re-reveals a dismissed strip, as Apple's.
    void setRows(const QList<Row>& rows, const QString& loadError = {});
    // View ▸ Show World Warnings (Apple: `worldWarningVisible`, on by default, not saved).
    void setShownByView(bool shown);
    bool shownByView() const { return shownByView_; }
    // The × (Apple: `dismissed`) — until another world goes away.
    void dismiss();

    const QList<Row>& rows() const { return rows_; }   // tests
    QString text() const;                              // tests: the headline and every line

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void refresh();
    void place();

    QLabel*    headline_ = nullptr;
    QLabel*    lines_    = nullptr;
    QList<Row> rows_;
    QString    loadError_;
    QSet<QString> lastSeenWorldIDs_;
    bool       dismissed_   = false;
    bool       shownByView_ = true;
};
