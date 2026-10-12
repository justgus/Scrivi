#pragma once

// SP-177 / T-0604 (EP-026 S1) — WHEN AN UNDO STEP ENDS: the capture rules, in the core.
//
// ✅ EP-026 Q1 (ruled 2026-10-11): these rules were Apple's Swift (`HistoryCapture.swift` + the text view's
// `isCommitBoundary`); they move here so every platform calls ONE copy. ✅ Design §4.a.1's *"coalescing stays app-side;
// HistoryService remains pure"* is kept: this is a SEPARATE component that calls the history engine through `CaptureSink`,
// exactly as the Swift did — `HistoryService` is not changed.
//
// The rules (design §4.a, §4.a.1, §4.d):
//   • an edit LATCHES (scene, full text, caret before/after); nothing is recorded until a trigger;
//   • a scene's floor is seeded once per session, on its first edit, with the text before that edit;
//   • 45 s idle ends a session — checked before an edit is folded in, and by the platform's timer at `idleDeadline()`;
//   • the character just typed is `.` `!` `?` or a newline → commit (✅ SP-177 Q4: exactly these, ASCII);
//   • a caret move to ANOTHER scene commits; within the scene it is SOFT — a whitespace-only change waits for real text;
//   • paste / cut commit what was pending, then label the next event (and a cut-into-buffer's slot);
//   • an autosave never seals a session, unless it is already idle (T-0396);
//   • a multi-scene edit records one event per scene under one group id (I-0270);
//   • undo is possible while an edit is pending.
// ✅ SP-177 Q1: the platform passes `nowMs` (any monotonic milliseconds); the core never reads a clock or owns a timer.
// ✅ SP-177 Q2: carets arrive in UTF-16 units of the scene text; the engine stores UTF-8 bytes, converted here.

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace scrivi::history {

// What the session asks of the history engine (the C API implements it over `HistoryStore`).
class CaptureSink {
public:
    virtual ~CaptureSink() = default;
    virtual void seed(const std::string& sceneID, const std::string& text) = 0;
    struct Recorded { bool noOp = true; bool canUndo = false; bool canRedo = false; };
    virtual Recorded record(const std::string& sceneID, const std::string& text, const std::string& kind,
                            std::int64_t cursorBefore, std::int64_t cursorAfter, const std::string& bufferID,
                            const std::string& groupID) = 0;
};

class CaptureSession {
public:
    static constexpr std::int64_t kIdleMs = 45'000;   // design §4.a trigger 7 (user-ruled 30–60 s band)

    // The writer changed `sceneID` to `text` (its full current text); `caretUtf16` is the caret after the edit;
    // `textBefore` is the scene before it (the floor, the first time). Returns true when an event was committed.
    bool noteEdit(CaptureSink& sink, const std::string& sceneID, const std::string& text, int caretUtf16,
                  const std::string& textBefore, std::int64_t nowMs);

    // The caret moved into `sceneID`. `editInFlight`: this move came from an edit in the same turn (not a move).
    // `applying`: an undo/redo is being applied — position only, never a commit.
    bool caretMoved(CaptureSink& sink, const std::string& sceneID, bool editInFlight, bool applying);

    // A paste or cut is about to change the text: commit what was pending; the next event is `kind` (and the slot).
    void begin(CaptureSink& sink, const std::string& kind, const std::string& bufferID);

    // Commit the pending edit with `trigger`. `soft` defers a whitespace-only change. Returns true when committed.
    bool flush(CaptureSink& sink, const std::string& trigger, const std::string& kind = "typing", bool soft = false);

    // An autosave is about to write (T-0396): commits only if the session is already idle.
    bool beforeSave(CaptureSink& sink, std::int64_t nowMs);

    // The platform's idle timer fired: commits if the session is idle at `nowMs` (a stale timer does nothing).
    bool idle(CaptureSink& sink, std::int64_t nowMs);

    struct GroupEdit { std::string sceneID; std::string textBefore; std::string textAfter; int caretUtf16 = 0; };
    // One edit across several scenes (I-0270): commits pending typing first, then one event per scene under `groupID`.
    void groupedEdit(CaptureSink& sink, const std::vector<GroupEdit>& edits, const std::string& kind,
                     const std::string& groupID);

    // Before a barrier: commits what was pending; returns the scene to attribute it to (`sceneID`, or the pending scene).
    std::string beforeBarrier(CaptureSink& sink, const std::string& sceneID);
    // After a barrier that rewrote scenes ([I-0287]): the engine already reads `sceneTexts` for them — they need no
    // seed, and the caret scene's whitespace reference is its new text.
    void afterBarrier(const std::map<std::string, std::string>& sceneTexts);

    // After an undo/redo was applied: the caret rests in `sceneID`, which now reads `text` (the whitespace reference).
    void noteApplied(const std::string& sceneID, const std::string& text);

    // The engine's can-undo/redo after any call that changed the pointer (undo, redo, barrier, purge, select branch).
    void noteEngineState(bool canUndo, bool canRedo) { engineCanUndo_ = canUndo; engineCanRedo_ = canRedo; }

    bool hasPending() const { return pendingSceneID_.has_value(); }
    bool canUndo() const { return engineCanUndo_ || hasPending(); }
    bool canRedo() const { return engineCanRedo_; }
    // When the platform's idle timer should fire (nowMs units), or -1 when nothing is pending.
    std::int64_t idleDeadline() const;

    // Helpers, public for tests.
    static std::int64_t utf8Offset(const std::string& text, int utf16);   // UTF-16 units → UTF-8 bytes
    static bool isCommitBoundary(const std::string& text, int caretUtf16);
    static bool isWhitespaceOnlyDelta(const std::string& a, const std::string& b);

private:
    bool isIdle(std::int64_t nowMs) const { return lastActivityMs_ && nowMs - *lastActivityMs_ >= kIdleMs; }
    void seedIfNeeded(CaptureSink& sink, const std::string& sceneID, const std::string& baseline);
    void clearPending();

    std::optional<std::string> pendingSceneID_;
    std::string pendingText_;
    std::int64_t pendingCursorBefore_ = 0;
    std::int64_t pendingCursorAfter_ = 0;
    std::optional<std::int64_t> lastActivityMs_;
    std::set<std::string> seeded_;
    std::string lastCommittedText_;
    std::optional<std::string> nextKind_;
    std::optional<std::string> nextBufferID_;
    std::optional<std::string> caretScene_;   // the scene the caret was last in (Apple: `lastCursorSegmentIndex`)
    bool engineCanUndo_ = false;
    bool engineCanRedo_ = false;
};

}   // namespace scrivi::history
