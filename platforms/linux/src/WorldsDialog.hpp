#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class ScriviBridge;
class QVBoxLayout;
class QWidget;
class QLabel;

// WorldsDialog — the Linux Worlds manager (EP-035 / SP-127, T-0492–T-0495).
// The mirror of Apple's Scrivi/Views/WorldsView.swift (EP-031 SP-099, T-0408).
//
// ## ⚠️ Why this exists, and why RELINK is the reason
//
// ⚠️ Before this dialog, `addWorld`, `relinkWorld`, `getWorldStatus` and
// `getWorldBinding` were bridged across the ABI with ZERO callers — a capability
// with no surface. The one `listWorlds` caller (`EditorShell.cpp`) used it only
// to recover a display NAME for an error string.
//
// ⚠️ The consequence was concrete and user-visible: **a project whose world had
// moved could not be repaired from the Linux app at all.** The writer saw an
// honest error and had no affordance to act on it. EP-035 AC3 names relink as the
// first thing to build for exactly that reason.
//
// ## ⚠️ What is deliberately NOT mirrored from Apple
//
// - `WorldBookmarkStore` — macOS security-scoped bookmarks. ⚠️ There is no Linux
//   equivalent and porting one would be cargo cult; Linux has ordinary filesystem
//   permissions and needs no grant to persist.
// - World CREATION — deferred to T-0497. AC3 asks for added / status read /
//   relinked. ⚠️ Apple's create path is entangled with the grant store above.
// - "Remove All World References" — ⚠️ Apple omits it too: destructive to pending
//   links, and it must be sought out rather than sit beside routine actions.
class WorldsDialog : public QDialog {
    Q_OBJECT

public:
    WorldsDialog(ScriviBridge* bridge, QString projectRootPath, QWidget* parent = nullptr);

signals:
    // ⚠️ Emitted after ANY mutation that can change a world's resolved status.
    //
    // ⚠️ THE I-0130 RULE. Apple shipped this defect: a relink repaired the world
    // while the project-wide warning kept insisting it was missing until the
    // writer changed scenes. Reloading this dialog's own list is NOT enough —
    // the Scene Inspector's status line is a different widget reading the same
    // core state, and it must be told.
    //
    // ⚠️ Every mutating handler here emits this. A new action that forgets to is
    // the same defect again.
    void worldsChanged();

private:
    // One row's worth of what the core reported. Mirrors the `scrivi_list_worlds`
    // envelope fields exactly (scrivi_c_api.cpp:1163).
    struct Entry {
        QString worldID;
        QString displayName;
        QString status;          // core WorldStatus name
        QString statusReason;    // may be empty — "no further detail"
        QString packagePath;             // ⚠️ non-empty ONLY when available
        QString lastKnownPackagePath;    // ⚠️ where we LOOKED; never proof
    };

    void reload();
    void rebuildRows();
    QWidget* makeRow(const Entry& e);

    void relinkWorld(const Entry& e);
    void addExistingWorld();
    void removeWorldReference(const Entry& e);

    // Shows an error beneath the list. ⚠️ Never swallows one: a rejected relink
    // is the core refusing to bind the WRONG package, and that refusal is the
    // most useful thing the writer can be told.
    void showActionError(const QString& text);

    ScriviBridge* bridge_ = nullptr;
    QString       projectRootPath_;

    QVector<Entry> worlds_;

    QVBoxLayout* rowsLayout_ = nullptr;
    QLabel*      emptyLabel_ = nullptr;
    QLabel*      errorLabel_ = nullptr;
};
