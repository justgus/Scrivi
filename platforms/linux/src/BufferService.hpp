#pragma once

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

// BufferService — the copy buffers 1–9 of one open project (EP-026 S5, SP-179 / T-0606).
// ✅ Apple's shape: `Scrivi/App/BufferService.swift`.
//
// Ten buffers, 0–9. Buffer 0 IS the system clipboard — ordinary Ctrl+C / Ctrl+V, never touched here. Buffers 1–9 are
// ScriviCore's persisted slots (`history/buffers.json`, `scrivi.buffers.v1` — the SAME file the Mac reads), mirrored here
// for display:
//   • Ctrl+N       → copy the selection into slot N (no text change ⇒ no undo step, Trade T3);
//   • Ctrl+Shift+N → paste slot N at the caret (one `paste` step);
//   • Alt+N        → cut the selection into slot N (one `cut` step, tagged with the slot).
// A slot holds the BALANCED stored source Ctrl+C keeps for Scrivi's own paste ([I-0289], [SP-179] Q2).
//
// ⛔ Owns NO text mutation (Apple's rule): the editor inserts and removes, so history and autosave see it as any edit.
// One per open project, owned by `EditorShell` beside `HistoryCapture` — `ProjectSession` holds state and never talks to
// the core. Best-effort, as Apple's: a failed read keeps the previous mirror; a failed write returns false.
class BufferService : public QObject
{
    Q_OBJECT

public:
    struct Slot {
        QString bufferID;
        QString text;        // STORED text (escapes included)
        QString updatedAt;   // ISO-8601
        QJsonObject range;   // a cross-scene copy's `scrivi.range.v1` ([SP-191] Q4a); empty for a plain-text slot
    };

    // The persistent slot IDs. Buffer 0 (the system clipboard) is deliberately not one.
    static const QStringList& slotIDs();

    explicit BufferService(const QString& projectRootPath, QObject* parent = nullptr);

    [[nodiscard]] const QString& projectRootPath() const { return root_; }

    // Reloads the mirror from ScriviCore (the Mac may have written the file); emits `changed`.
    void refresh();

    // The non-empty slots, ascending by bufferID.
    [[nodiscard]] const QList<Slot>& filled() const { return slots_; }
    // Slot `bufferID`'s stored text, or empty when the slot is empty.
    [[nodiscard]] QString text(const QString& bufferID) const;
    [[nodiscard]] QString updatedAt(const QString& bufferID) const;
    [[nodiscard]] bool isStructured(const QString& bufferID) const;
    // Slot `bufferID`'s Range, or empty (a plain-text slot, or an empty one). ✅ [SP-191] Q4a: the buffers hold Ranges, as
    // history does; a slot written before that (an old fragment) reads as empty in the core.
    [[nodiscard]] QJsonObject range(const QString& bufferID) const;
    // One line for the palette: the first non-blank line of what the writer SEES (no escapes), or empty.
    [[nodiscard]] QString preview(const QString& bufferID) const;

    // Create-or-replace slot `bufferID` with `text` (stored form) and, for a cross-scene copy, its `range` — `text` is then
    // the pieces joined by a blank line. Refreshes. False on a store failure.
    bool load(const QString& text, const QString& bufferID, const QJsonObject& range = {});
    // Empties slot `bufferID`. Refreshes. Whether a slot was removed.
    bool clear(const QString& bufferID);

signals:
    void changed();

private:
    [[nodiscard]] const Slot* find(const QString& bufferID) const;

    QString root_;
    QList<Slot> slots_;
};
