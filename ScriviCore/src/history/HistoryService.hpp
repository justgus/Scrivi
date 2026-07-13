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
#include <set>
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

// What branch-aware eviction did, so the store can persist matching ctl records
// (§4.1/§5). `promotedRoots` are the successive newRootIDs as the root advanced
// along the surviving path; `purgedBranchRoots` are the subtree roots dropped
// (non-primary branches whose branch point fell off the limit). Both empty when
// nothing was evicted. Replay honors these to reproduce eviction after reload.
struct EvictionDetail {
    std::vector<std::string> promotedRoots;      // ctl:evict per new root, in order
    std::vector<std::string> purgedBranchRoots;  // ctl:purge per dropped subtree
};

struct RecordResult {
    std::string eventID;
    bool createdBranch = false;   // true when this record forked (§5/D1)
    int evictedCount = 0;         // nodes dropped by capacity eviction (§4.1)
    bool noOp = false;            // true when newSceneText == head (no diff)
    EvictionDetail eviction;      // what eviction purged/promoted (for persistence)
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

// One forward branch at a fork node, for the inline fork popover (§10 T2 / SP-055).
struct ForkChild {
    std::string eventID;      // the child branch's first node
    std::string preview;      // short text preview of that branch's change
    std::string timestamp;    // the child's ISO-8601 timestamp
    bool isPrimary = false;   // true for the current primaryChildID
};

// Present on a StepResult only when the pointer LANDS ON a node with >= 2
// children (a real fork). Drives the T2 popover; absent otherwise. Design §7/§10.
struct ForkAhead {
    std::string nodeID;                 // the fork node the pointer is at
    std::vector<ForkChild> children;    // forward branches, in childIDs order
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

    // Present when the pointer landed on a fork (>= 2 children) — drives the
    // inline fork popover (§10 T2). Absent (nullopt) otherwise.
    std::optional<ForkAhead> forkAhead;
};

// Result of selecting a branch at a fork (re-primary; does not move the pointer).
struct SelectBranchResult {
    bool ok = false;
    std::string forkNodeID;
    std::string childEventID;
    bool canRedo = false;
};

// One stale branch: a non-primary subtree whose most recent activity (tip) is
// older than the stale threshold (§5, T-0212). `branchRootEventID` is the
// subtree root the user purges; the other fields describe it for the confirm UI.
struct StaleBranch {
    std::string branchRootEventID;  // the non-primary child that roots the subtree
    std::string forkNodeID;         // the fork this branch hangs off (its parent)
    std::string preview;            // short text preview of the branch's first change
    std::string tipTimestamp;       // ISO-8601 of the newest node in the subtree
    int nodeCount = 0;              // nodes in the subtree (what purge removes)
};

// Result of a user-confirmed purge of a branch subtree (§5, T-0212).
struct PurgeResult {
    bool ok = false;                // false when branchRootEventID is not purgeable
    std::string branchRootEventID;
    int purgedCount = 0;            // nodes removed
    bool canUndo = false;
    bool canRedo = false;
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

    // Forcibly resets a scene's floor AND current head text to `text`, replacing
    // any existing value. Used by §6.b external-change repair after an
    // externalChange barrier: the on-disk text becomes the new baseline. Unlike
    // seedSceneBaseline this overwrites; it does not touch the tree.
    void reseedSceneFloor(const std::string& sceneID, const std::string& text);

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

    // Re-primaries a fork: sets nodeRef(forkNodeID).primaryChildID = childEventID
    // (design §5, T2). Does NOT move the current pointer — the caller walks the
    // now-primary branch via redo(). Fails if childEventID is not a child of the
    // fork. Design §7 scrivi_history_select_branch.
    SelectBranchResult selectBranch(const std::string& forkNodeID,
                                    const std::string& childEventID);

    // Lists stale branches (§5, T-0212): every non-primary subtree whose tip
    // (newest timestamp in the subtree) is older than `staleBranchDays` before
    // `nowIso`. A branch on the root→current path is never reported (it holds the
    // live state). `nowIso` and `staleBranchDays` come from the ABI (which owns
    // the clock/settings); `staleBranchDays <= 0` reports nothing. Timestamps are
    // ISO-8601; unparseable ones are treated as not stale. Design §7
    // scrivi_history_list_stale_branches.
    [[nodiscard]] std::vector<StaleBranch> listStaleBranches(const std::string& nowIso,
                                                             int staleBranchDays) const;

    // Purges a branch subtree with user confirmation (§5, T-0212): erases
    // `branchRootEventID` and all its descendants. Rejects (ok=false) when the
    // node is unknown, IS the root, or lies on the root→current path (purging it
    // would strand the live pointer). Does NOT move the current pointer. Design §7
    // scrivi_history_purge_branch.
    PurgeResult purgeBranch(const std::string& branchRootEventID);

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] const std::string& sessionID() const { return sessionID_; }
    [[nodiscard]] const std::string& currentNodeID() const { return currentNodeID_; }

    // The service's cached head text for a scene (its text at the current node);
    // empty if the scene has never been recorded. Exposed for the ABI's
    // head-hash checkpoint work (SP-054) and for tests.
    [[nodiscard]] std::string headTextForScene(const std::string& sceneID) const;

    // ---- Persistence surface (SP-054, HistoryStore) ----------------------
    // These let HistoryStore serialize the tree to the log/checkpoint and
    // rebuild it on load. No JSON here — the store owns the wire format.

    // All nodes in insertion-independent order (map order by eventID). Used to
    // serialize the tree on checkpoint.
    [[nodiscard]] const std::map<std::string, EventNode>& nodes() const { return nodes_; }
    [[nodiscard]] const std::string& rootID() const { return rootID_; }
    // Per-scene floor (root) text — the baseline snapshots to persist.
    [[nodiscard]] const std::map<std::string, std::string>& floorTexts() const { return floorTexts_; }

    // Rehydration (load): install a node, set pointers, then finalize. The store
    // calls addLoadedFloor/addLoadedNode for each record, then setPointers, then
    // finalizeLoad() which derives childIDs and rebuilds the head cache.
    void addLoadedFloor(const std::string& sceneID, std::string text);
    void addLoadedNode(EventNode node);
    void setPointers(std::string rootID, std::string currentNodeID, std::string sessionID);
    void finalizeLoad();

    // Replays persisted branch-aware eviction (§4.1) after finalizeLoad() has
    // derived childIDs. For each purged branch root, erases that subtree; for
    // each promoted root (in order), folds the promoted child's diff into the
    // floor, detaches it as the new root, and erases the old root — reproducing
    // the in-memory eviction so evicted branches do NOT resurrect from the log.
    // Order matches persistEviction: purges first, then promotions. No-op on an
    // unknown ID (the record referenced a node already gone). Rebuilds the head
    // cache at the end. Called by HistoryStore during replay.
    void applyLoadedEviction(const std::vector<std::string>& purgedBranchRoots,
                             const std::vector<std::string>& promotedRoots);

    // Applies a persisted primary-child override for a fork (D4/SP-055): sets
    // nodeRef(forkNodeID).primaryChildID = childEventID if childEventID is a child.
    // Called by HistoryStore during replay for each ctl:setPrimary record and for
    // each state.json primaryChildren entry, AFTER finalizeLoad() derives childIDs.
    // No-op if either node is unknown or the child is not a child of the fork.
    void applyPrimaryOverride(const std::string& forkNodeID, const std::string& childEventID);

    // Replaces the current session id (used on idle rollover / new session at open).
    void setSessionID(std::string sessionID) { sessionID_ = std::move(sessionID); }

    // Capacity (Trade T1): the max number of event nodes retained. 0 = unlimited.
    // When record() pushes the count above this, the oldest node(s) are evicted
    // from the root (§4.1, branch-aware: non-primary subtrees off the root are
    // auto-purged, then the on-path child becomes the new root with its diff
    // folded into the per-scene floor). Never evicts a node on the root→current
    // path; DEFERS when the current pointer is the root itself.
    void setCapacity(int capacityEvents) { capacityEvents_ = capacityEvents; }
    [[nodiscard]] int capacity() const { return capacityEvents_; }
    // Number of event nodes currently retained (excludes the root anchor).
    [[nodiscard]] int eventCount() const { return static_cast<int>(nodes_.size()) - 1; }

private:
    EventNode& nodeRef(const std::string& id);
    [[nodiscard]] const EventNode& nodeRef(const std::string& id) const;

    // Rebuilds the per-scene head-text cache for `currentNodeID_` by walking the
    // root→current path and applying each node's diff to its scene.
    void rebuildHeadCache();

    // Returns fork data for `nodeID` when it has >= 2 children, else nullopt.
    // Used to populate StepResult.forkAhead after an undo/redo step.
    [[nodiscard]] std::optional<ForkAhead> forkAheadAt(const std::string& nodeID) const;

    // Evicts from the root while eventCount() > capacity (§5, branch-aware): at
    // each step auto-purges the root's non-primary child subtrees, then promotes
    // the child on the root→current path to the new root (folding its diff into
    // the floor). Never evicts a node on the root→current path; DEFERS (stops)
    // when the current pointer is the root itself. Returns the number of nodes
    // evicted and, via `detail`, the promoted roots and purged branch roots so
    // the store can persist matching ctl:evict/ctl:purge records (§4.1).
    int evictToCapacity(EvictionDetail& detail);

    // The root's child that lies on the root→current path (must be promoted, not
    // evicted). Empty when the current pointer IS the root.
    [[nodiscard]] std::string rootChildTowardCurrent() const;

    // Erases a node and its whole descendant subtree from nodes_. Returns the
    // count removed. Used by eviction auto-purge (§5) and user purge (T-0212).
    int eraseSubtree(const std::string& subtreeRootID);

    // Counts the nodes in a subtree without modifying it (for the stale-branch
    // confirm UI's "N steps" — T-0212). Unknown ID → 0.
    [[nodiscard]] int subtreeNodeCount(const std::string& subtreeRootID) const;

    std::map<std::string, EventNode> nodes_;   // eventID → node
    std::string rootID_;
    std::string currentNodeID_;
    std::string sessionID_;

    // sceneID → full text at the current node (the "head text" the next record
    // diffs against, and undo/redo restore to). Mutates as the pointer moves.
    std::map<std::string, std::string> headText_;

    // sceneID → floor (root) text: the baseline the scene entered history with.
    // Immutable after seeding; persisted as rec:"floor" and used as the starting
    // text when replaying the log on load.
    std::map<std::string, std::string> floorTexts_;

    // Earlier sessions we have already surfaced a boundary warning for this run,
    // so undo warns once per crossing (§5), not on every step into that session.
    std::set<std::string> warnedSessions_;

    // Max retained event nodes (Trade T1); 0 = unlimited.
    int capacityEvents_ = 0;
};

} // namespace scrivi::history
