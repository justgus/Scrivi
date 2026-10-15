#pragma once

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QTimer;

// HistoryCapture — the Linux side of the undo history (EP-026 S2, SP-178 / T-0605). One per open project, owned by
// EditorShell. ✅ Apple's shape: `Scrivi/App/HistoryCapture.swift` as SP-177 left it.
//
// ✅ WHEN an undo step ends is ScriviCore's (`scrivi_capture_*`, SP-177): the latch, floor seeding, the 45 s idle rule, the
// sentence boundary, the soft whitespace deferral, scene switch vs caret move, paste/cut labels, autosave-never-seals,
// grouped edits. ⛔ None of that is here. This class keeps only PLUMBING:
//  - it reports what happened (an edit, a caret move, a paste/cut, a save, an applied undo) with a monotonic `nowMs`;
//  - it runs the idle TIMER at the deadline the core returns (SP-177 Q1: the core never owns a clock or a timer);
//  - `isApplying` around an undo apply, so the text change it causes is not reported as an edit;
//  - the can-undo / can-redo the core last reported, for Edit ▸ Undo / Redo.
// History is best-effort, as on Apple: when it is not open every call is a no-op and editing carries on.
class HistoryCapture : public QObject
{
    Q_OBJECT

public:
    explicit HistoryCapture(const QString& projectRootPath, QObject* parent = nullptr);
    ~HistoryCapture() override;

    // ✅ [SP-178] Q1: `scrivi_history_open` and the per-scene validation ran in the LOAD WORKER; this adopts the result
    // on the UI thread. `opened` false (the open failed) leaves history closed — editing is unaffected.
    void adoptOpened(bool opened);
    // Closes the history (`scrivi_history_close` commits what is pending and writes the checkpoint). ⚠️ Call AFTER the
    // dirty scenes are saved (Apple's order: flush → save → close).
    void close();

    [[nodiscard]] bool isOpen() const { return open_; }
    [[nodiscard]] const QString& projectRootPath() const { return root_; }
    [[nodiscard]] bool canUndo() const { return canUndo_; }
    // [I-0302]: the newest step is a hard barrier — nothing to undo, but an undo there shows the barrier's notice.
    [[nodiscard]] bool atBarrier() const { return atBarrier_; }
    [[nodiscard]] bool canRedo() const { return canRedo_; }
    [[nodiscard]] bool hasPending() const { return hasPending_; }
    [[nodiscard]] bool isApplying() const { return applying_; }
    void setApplying(bool on) { applying_ = on; }

    // The monotonic clock every `nowMs` and deadline is in.
    static qint64 nowMs();
    // An ISO-8601 history timestamp for the session-boundary warning (T-0209) — "yesterday at 9:42 PM", "on Jul 3 at
    // 2:15 PM". Empty when it cannot be read. Apple: `HistoryTimestamp.friendly`.
    static QString friendlyTimestamp(const QString& iso8601);

    // ── Reports (the shell and the editor call these at Apple's call sites) ──
    // The writer changed `sceneID` to `text` (its whole stored text); `caretUtf16` the caret after the edit; `textBefore`
    // the scene before it. ⛔ Not while applying an undo/redo.
    void noteEdit(const QString& sceneID, const QString& text, int caretUtf16, const QString& textBefore);
    void caretMoved(const QString& sceneID, bool editInFlight);
    // A paste or cut is about to change the text; follow the change with `flush(kind, kind)`. `bufferID` tags a cut INTO a
    // copy buffer with its slot (Apple: `beginCutIntoBuffer(bufferID:)`, Trade T3); empty for an ordinary paste or cut.
    void beginPasteOrCut(const QString& kind, const QString& bufferID = {});
    void flush(const QString& trigger, const QString& kind = QStringLiteral("typing"));
    // ONE edit across several scenes ([I-0270]): `editsJson` = [{sceneID, textBefore, textAfter, caret}].
    void groupedEdit(const QString& editsJson, const QString& kind);
    // An autosave is about to write: never seals the session unless already idle (T-0396).
    void beforeSave();
    // The save path wrote `diskText` for `sceneID` — ⛔ only after a write that SUCCEEDED ([I-0259]).
    void noteScenePersisted(const QString& sceneID, const QString& diskText);
    // A structural barrier (§4.5) — the core commits what is pending first and attributes it to the pending scene.
    // `sceneTexts` ([I-0287]): sceneID → the text the op left in each scene it rewrote or created, exactly as the editor
    // will next report it — so the next edit's undo returns to THAT text, not the pre-op one. ⚠️ Record it AFTER the op.
    // A non-empty `structuralPayload` (EP-026 S4, Apple's `HistoryStructuralPayload`) records a REVERSIBLE structural
    // node instead: undo/redo step across it and hand the payload back. With it, `sceneTextsBefore` ([I-0293]): what
    // each scene that EXISTED before the op held then.
    void recordBarrier(const QString& kind, const QString& note, const QHash<QString, QString>& sceneTexts = {},
                       const QJsonObject& structuralPayload = {}, const QHash<QString, QString>& sceneTextsBefore = {});

    // ── Undo / Redo — the shell applies what comes back ──
    // {} when history is closed or the call failed.
    QVariantMap undo();
    QVariantMap redo();
    // After applying an undo/redo: the caret rests in `sceneID`, which now reads `text`.
    void noteApplied(const QString& sceneID, const QString& text);

private:
    // Takes the core's report: the can-state, and re-arms the idle timer at its deadline.
    void take(const QVariantMap& r);
    void refreshState();

    QString root_;
    bool open_ = false;
    bool applying_ = false;
    bool canUndo_ = false;
    bool atBarrier_ = false;
    bool canRedo_ = false;
    bool hasPending_ = false;
    QTimer* idleTimer_ = nullptr;
};
