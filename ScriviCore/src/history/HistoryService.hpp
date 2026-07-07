#pragma once

// HistoryService — the in-memory undo/redo history engine core (EP-019, SP-052).
//
// This is the linear (non-branching) engine per the SP-052 sprint note: every
// event is recorded on a single chain, but each node already carries parentID /
// primaryChildID so full branching (SP-055) drops in without a rewrite. Disk
// persistence (JSONL log + checkpoint) is SP-054; this layer is purely
// in-memory and UI-agnostic.
//
// Design reference: docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md
//   §3.a snapshot-diff in C++ (common-prefix/suffix trim on scalar boundaries)
//   §3.b undo/redo return full scene text
//   §4   event model / event record fields
//   §5   history tree model (current pointer, sessions)
//
// No JSON, no nlohmann, no UI types appear here — the C ABI (T-0202) marshals
// to/from the JSON envelopes. Offsets and cursor positions are scene-local
// UTF-8 byte offsets, C++-internal only (Swift never interprets them, §4.b).

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace scrivi::history {

// Kind of a recorded node. `barrier` is a structural stop (§4.5); the text
// kinds describe how the scene text changed at that node.
enum class EventKind {
    Typing,
    Delete,
    Replace,
    Paste,
    Cut,
    Barrier,
};

// A minimal scene-local diff: at `offsetUtf8`, `removed` bytes were replaced by
// `inserted` bytes. Offsets are UTF-8 byte offsets on scalar boundaries.
struct Diff {
    std::size_t offsetUtf8 = 0;
    std::string removed;
    std::string inserted;
};

// One history node. The root is the oldest retained state and has no diff.
struct EventNode {
    std::string eventID;
    std::optional<std::string> parentID;              // null only for the root
    std::optional<std::string> primaryChildID;        // redo follows this child
    std::vector<std::string> childIDs;                // derived order of children

    EventKind kind = EventKind::Typing;
    std::string sceneID;                              // empty for the root
    Diff diff;                                         // empty for the root/barrier

    std::int64_t cursorBefore = 0;
    std::int64_t cursorAfter = 0;
    std::string timestamp;                             // ISO-8601, minted by caller
    std::string sessionID;

    // Barrier-only: the structural reason undo stops here (§4.5).
    std::string barrierKind;
    std::string barrierNote;
};

// Parameters for recording a text event. `newSceneText` is the full current
// text of `sceneID`; the service diffs it against its cached head text.
struct RecordParams {
    std::string sceneID;
    std::string newSceneText;
    EventKind kind = EventKind::Typing;
    std::int64_t cursorBefore = 0;
    std::int64_t cursorAfter = 0;
    std::string timestamp;   // caller-supplied ISO-8601; kept verbatim
};

struct RecordResult {
    std::string eventID;
    bool createdBranch = false;   // always false in the linear engine
    int evictedCount = 0;         // always 0 until SP-054 capacity work
    bool noOp = false;            // true when newSceneText == head (no diff)
};

struct BarrierParams {
    std::string barrierKind;   // sceneSplit|sceneDelete|sceneMerge|... (§4.5)
    std::string barrierNote;   // human-readable "Can't undo past a scene merge"
    std::string timestamp;
};

// One scene's resulting text after an undo/redo step.
struct SceneChange {
    std::string sceneID;
    std::string newText;
    std::int64_t cursorAfter = 0;
};

// Result of an undo or redo step.
struct StepResult {
    bool moved = false;                    // false when nothing to undo/redo
    std::optional<SceneChange> change;     // the scene text to apply (if moved)
    std::string nodeID;                    // the new current node
    bool canUndo = false;
    bool canRedo = false;
    bool crossedSessionBoundary = false;   // undo landed on an earlier session
    std::string boundaryTimestamp;         // timestamp of the earlier-session node

    // Set when undo is blocked by a barrier (§4.5): the step does not move.
    bool stoppedAtBarrier = false;
    std::string barrierKind;
    std::string barrierNote;
};

// The linear history engine for one open project. In-memory only (SP-052).
class HistoryService {
public:
    // Opens a fresh, empty history with a single root node and mints a session.
    // `sessionID` and `timestamp` are supplied by the caller (the ABI layer,
    // which owns UUID/clock providers). `rootTimestamp` stamps the root node.
    HistoryService(std::string sessionID, std::string rootTimestamp);

    // Seeds a scene's ROOT FLOOR text without creating an event: the state undo
    // can restore up to but never below (the empty scene for a from-scratch
    // project, or the pre-existing text for a project that already had content
    // when history was first created). The first recorded event then diffs
    // against this real text, so undo stops at the floor instead of emptying the
    // scene. No-op if the scene already has a cached head text. Design §5.
    //
    // Conceptually this is a ONE-TIME, project-lifetime seed done when history is
    // first created (the root node's per-scene snapshot). In the SP-053 in-memory
    // engine, history opens empty every launch, so the Swift layer seeds on first
    // edit per session as a stand-in — behaviourally identical while history does
    // not persist. When persistence lands (SP-054) the seed moves to
    // history-creation time and is NOT repeated per session; the scene's current
    // on-disk text is then the HEAD (reached by replaying events), not the floor.
    void seedSceneBaseline(const std::string& sceneID, const std::string& text);

    // Records a text event by diffing `p.newSceneText` against the cached head
    // text for `p.sceneID`. If the text is unchanged, returns noOp=true and
    // records nothing. `eventID` is caller-minted.
    RecordResult record(const RecordParams& p, std::string eventID);

    // Records a structural barrier node (§4.5). Always advances the pointer.
    RecordResult recordBarrier(const BarrierParams& p, std::string eventID);

    // Moves the current pointer to the parent, returning the scene text that
    // undoing the departed node produces. Stops (without moving) at a barrier.
    StepResult undo();

    // Moves the current pointer to the primary child, returning that child's
    // resulting scene text.
    StepResult redo();

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] const std::string& sessionID() const { return sessionID_; }
    [[nodiscard]] const std::string& currentNodeID() const { return currentNodeID_; }

    // The service's cached head text for a scene (its text at the current node);
    // empty if the scene has never been recorded. Exposed for the ABI's
    // head-hash checkpoint work (SP-054) and for tests.
    [[nodiscard]] std::string headTextForScene(const std::string& sceneID) const;

private:
    EventNode& nodeRef(const std::string& id);
    [[nodiscard]] const EventNode& nodeRef(const std::string& id) const;

    // Rebuilds the per-scene head-text cache for `currentNodeID_` by walking the
    // root→current path and applying each node's diff to its scene.
    void rebuildHeadCache();

    std::map<std::string, EventNode> nodes_;   // eventID → node
    std::string rootID_;
    std::string currentNodeID_;
    std::string sessionID_;

    // sceneID → full text at the current node (the "head text" the next record
    // diffs against, and undo/redo restore to).
    std::map<std::string, std::string> headText_;
};

} // namespace scrivi::history
