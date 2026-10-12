#pragma once

#include <QList>
#include <QPointer>
#include <QWidget>

#include "BufferService.hpp"

class QLabel;
class QToolButton;

// BuffersPalette — the one app-wide Copy Buffers palette (EP-026 S5, SP-179 / T-0606). ✅ Apple's shape:
// `Scrivi/Views/BuffersPalette.swift` — a floating panel that mirrors the FRONT project's slots 1–9, one row per slot:
// the slot number, a one-line preview of what the writer sees (or "empty"), when it was filled, ONE action button and a
// clear button.
//
// The action button follows the modifiers held, as the chords do ([I-0291]'s lesson: the same modifiers, exactly):
// none → copy (Ctrl+N) · Ctrl+Shift → paste (Ctrl+Shift+N) · Alt → cut (Alt+N). ✅ The palette never takes focus
// (`Qt::WindowDoesNotAcceptFocus`, `WA_ShowWithoutActivating`), so the editor keeps its selection and caret; the held
// modifiers are therefore read from an APPLICATION event filter, not from the palette's own key events.
//
// ✅ [SP-179] Q3: placement is the platform's — Wayland cannot place or keep a window on top reliably, and its position is
// never restored. Owned by `AppEnvironment`, which re-points it at the front project and routes its actions there.
class BuffersPalette : public QWidget
{
    Q_OBJECT

public:
    enum class Action { Copy, Paste, Cut };

    explicit BuffersPalette(QWidget* parent = nullptr);
    ~BuffersPalette() override;

    // Mirror `service`'s slots (nullptr = no project in front: every row empty and inert).
    void setService(BufferService* service);
    // Hide without reporting a writer's close (no project in front: the toggle stays on, as Apple's).
    void hideQuietly();
    [[nodiscard]] BufferService* service() const { return service_; }

    // The action for these held modifiers — each requires its modifiers exactly, as the chords do.
    [[nodiscard]] static Action actionFor(Qt::KeyboardModifiers mods);
    // The action every row's button shows now (for tests and the live pass).
    [[nodiscard]] Action shownAction() const { return shown_; }

signals:
    // A row's action button, with the action the held modifiers chose at the click.
    void actionRequested(BuffersPalette::Action action, const QString& bufferID);
    void clearRequested(const QString& bufferID);
    // The writer closed it with its own close button — the View ▸ Show Buffers check follows.
    void closedByWriter();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    struct Row {
        QString bufferID;
        QLabel* number = nullptr;
        QLabel* preview = nullptr;
        QLabel* when = nullptr;
        QToolButton* action = nullptr;
        QToolButton* clear = nullptr;
    };

    void rebuild();                         // the rows from the service
    void showAction(Action action);         // every row's button → `action`
    void updateButton(Row& row) const;

    QPointer<BufferService> service_;
    QMetaObject::Connection changedConnection_;
    QMetaObject::Connection destroyedConnection_;
    QList<Row> rows_;
    Action shown_ = Action::Copy;
    bool closingProgrammatically_ = false;
    // The modifiers of the last press on an action button — what the click acts on.
    Qt::KeyboardModifiers clickMods_;
};
