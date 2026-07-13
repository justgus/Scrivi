#include "history/HistoryService.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <limits>
#include <stdexcept>

namespace scrivi::history {

namespace {

// Parses an ISO-8601 UTC timestamp of the form "YYYY-MM-DDTHH:MM:SSZ" (the format
// minted by the ABI clock) into a Unix epoch second count. Returns nullopt on any
// parse failure, so callers treat unparseable stamps as "unknown age". Used only
// for relative age comparisons (stale-branch detection, §5/T-0212).
std::optional<std::int64_t> parseIso8601Utc(const std::string& s) {
    int y, mo, d, h, mi, se;
    if (std::sscanf(s.c_str(), "%d-%d-%dT%d:%d:%dZ", &y, &mo, &d, &h, &mi, &se) != 6)
        return std::nullopt;
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon  = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min  = mi;
    tm.tm_sec  = se;
#if defined(_WIN32)
    const std::int64_t epoch = static_cast<std::int64_t>(_mkgmtime(&tm));
#else
    const std::int64_t epoch = static_cast<std::int64_t>(timegm(&tm));
#endif
    if (epoch == -1) return std::nullopt;
    return epoch;
}

// True if byte `b` is a UTF-8 continuation byte (0b10xxxxxx). A scalar boundary
// is any position whose byte is NOT a continuation byte (or end-of-string).
bool isContinuation(unsigned char b) { return (b & 0xC0) == 0x80; }

bool isBoundary(const std::string& s, std::size_t i) {
    if (i == 0 || i >= s.size()) return true;
    return !isContinuation(static_cast<unsigned char>(s[i]));
}

// Backs `i` up to the nearest scalar boundary at or below it.
std::size_t floorToBoundary(const std::string& s, std::size_t i) {
    while (i > 0 && !isBoundary(s, i)) --i;
    return i;
}

// Computes a minimal diff turning `oldText` into `newText`: trim the common
// prefix and common suffix (on scalar boundaries), leaving the changed middle.
// Design §3.a.
Diff computeDiff(const std::string& oldText, const std::string& newText) {
    // Common prefix length (byte-wise), then floored to a scalar boundary.
    std::size_t prefix = 0;
    const std::size_t maxPrefix = std::min(oldText.size(), newText.size());
    while (prefix < maxPrefix && oldText[prefix] == newText[prefix]) ++prefix;
    prefix = floorToBoundary(oldText, prefix);

    // Common suffix length, not overlapping the prefix in either string.
    std::size_t suffix = 0;
    const std::size_t oldRemain = oldText.size() - prefix;
    const std::size_t newRemain = newText.size() - prefix;
    const std::size_t maxSuffix = std::min(oldRemain, newRemain);
    while (suffix < maxSuffix &&
           oldText[oldText.size() - 1 - suffix] == newText[newText.size() - 1 - suffix]) {
        ++suffix;
    }
    // Floor the suffix boundary in oldText so we don't split a scalar.
    std::size_t oldSuffixStart = oldText.size() - suffix;
    oldSuffixStart = floorToBoundary(oldText, oldSuffixStart);
    suffix = oldText.size() - oldSuffixStart;
    // Clamp suffix so removed/inserted regions stay non-negative.
    suffix = std::min(suffix, std::min(oldText.size() - prefix, newText.size() - prefix));

    Diff d;
    d.offsetUtf8 = prefix;
    d.removed = oldText.substr(prefix, oldText.size() - prefix - suffix);
    d.inserted = newText.substr(prefix, newText.size() - prefix - suffix);
    return d;
}

// Applies a diff forward: old scene text → new scene text.
std::string applyForward(const std::string& oldText, const Diff& d) {
    std::string out;
    out.reserve(oldText.size() - d.removed.size() + d.inserted.size());
    out.append(oldText, 0, d.offsetUtf8);
    out.append(d.inserted);
    out.append(oldText, d.offsetUtf8 + d.removed.size(), std::string::npos);
    return out;
}

// Applies a diff in reverse: new scene text → old scene text.
std::string applyReverse(const std::string& newText, const Diff& d) {
    std::string out;
    out.reserve(newText.size() - d.inserted.size() + d.removed.size());
    out.append(newText, 0, d.offsetUtf8);
    out.append(d.removed);
    out.append(newText, d.offsetUtf8 + d.inserted.size(), std::string::npos);
    return out;
}

// Max bytes of a fork-child preview shown in the popover (§10 T2). Truncated on a
// scalar boundary so we never split a UTF-8 sequence.
constexpr std::size_t kForkPreviewMaxBytes = 60;

// A short, single-line preview of what a branch child changed: prefer its inserted
// text (typing/paste), fall back to what it removed (delete/cut). Newlines are
// collapsed to spaces; truncated to a scalar boundary <= kForkPreviewMaxBytes.
std::string forkPreview(const EventNode& child) {
    std::string src = !child.diff.inserted.empty() ? child.diff.inserted
                                                    : child.diff.removed;
    for (char& c : src) { if (c == '\n' || c == '\r' || c == '\t') c = ' '; }
    if (src.size() > kForkPreviewMaxBytes) {
        std::size_t cut = floorToBoundary(src, kForkPreviewMaxBytes);
        src.resize(cut);
    }
    return src;
}

} // namespace

HistoryService::HistoryService(std::string sessionID, std::string rootTimestamp)
    : sessionID_(std::move(sessionID)) {
    EventNode root;
    root.eventID = "evt_root";
    root.kind = EventKind::Barrier;   // the root is a non-textual anchor
    root.barrierKind = "root";
    root.timestamp = std::move(rootTimestamp);
    root.sessionID = sessionID_;
    rootID_ = root.eventID;
    currentNodeID_ = root.eventID;
    nodes_.emplace(root.eventID, std::move(root));
}

EventNode& HistoryService::nodeRef(const std::string& id) {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) throw std::logic_error("HistoryService: unknown node " + id);
    return it->second;
}

const EventNode& HistoryService::nodeRef(const std::string& id) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) throw std::logic_error("HistoryService: unknown node " + id);
    return it->second;
}

std::string HistoryService::headTextForScene(const std::string& sceneID) const {
    auto it = headText_.find(sceneID);
    return it == headText_.end() ? std::string{} : it->second;
}

void HistoryService::seedSceneBaseline(const std::string& sceneID, const std::string& text) {
    // Only seed a scene we have not seen yet — never clobber an established head.
    if (headText_.find(sceneID) == headText_.end()) {
        headText_[sceneID] = text;
        floorTexts_[sceneID] = text;   // immutable baseline for persistence/replay
    }
}

void HistoryService::reseedSceneFloor(const std::string& sceneID, const std::string& text) {
    floorTexts_[sceneID] = text;
    headText_[sceneID]   = text;
}

RecordResult HistoryService::record(const RecordParams& p, std::string eventID) {
    const std::string oldText = headTextForScene(p.sceneID);
    if (oldText == p.newSceneText) {
        RecordResult r;
        r.noOp = true;
        return r;
    }

    EventNode node;
    node.eventID = std::move(eventID);
    node.parentID = currentNodeID_;
    node.kind = p.kind;
    node.sceneID = p.sceneID;
    node.diff = computeDiff(oldText, p.newSceneText);
    node.cursorBefore = p.cursorBefore;
    node.cursorAfter = p.cursorAfter;
    node.timestamp = p.timestamp;
    node.sessionID = sessionID_;

    const std::string newID = node.eventID;
    EventNode& parent = nodeRef(currentNodeID_);
    // Branching (§5): recording while the current node already has a child means
    // we forked — the parent kept the old chain(s) under childIDs; the new node
    // becomes the primary child (the new work is now "the" history). This falls
    // out of appending + re-primarying; the old branches remain fully navigable.
    const bool forked = !parent.childIDs.empty();
    parent.childIDs.push_back(newID);
    parent.primaryChildID = newID;

    nodes_.emplace(newID, std::move(node));
    currentNodeID_ = newID;
    headText_[p.sceneID] = p.newSceneText;

    RecordResult r;
    r.eventID = newID;
    r.createdBranch = forked;
    r.evictedCount = evictToCapacity(r.eviction);
    return r;
}

RecordResult HistoryService::recordBarrier(const BarrierParams& p, std::string eventID) {
    EventNode node;
    node.eventID = std::move(eventID);
    node.parentID = currentNodeID_;
    node.kind = EventKind::Barrier;
    node.barrierKind = p.barrierKind;
    node.barrierNote = p.barrierNote;
    node.timestamp = p.timestamp;
    node.sessionID = sessionID_;

    const std::string newID = node.eventID;
    EventNode& parent = nodeRef(currentNodeID_);
    parent.childIDs.push_back(newID);
    parent.primaryChildID = newID;

    nodes_.emplace(newID, std::move(node));
    currentNodeID_ = newID;

    RecordResult r;
    r.eventID = newID;
    return r;
}

bool HistoryService::canUndo() const {
    // Can undo when the current node is a text node (has a parent and is not the
    // root). Barriers block undo but are themselves not undoable steps.
    if (currentNodeID_ == rootID_) return false;
    const EventNode& cur = nodeRef(currentNodeID_);
    return cur.kind != EventKind::Barrier && cur.parentID.has_value();
}

bool HistoryService::canRedo() const {
    const EventNode& cur = nodeRef(currentNodeID_);
    if (!cur.primaryChildID.has_value()) return false;
    // Redo only re-applies text nodes; a barrier ahead is not redoable here.
    return nodeRef(*cur.primaryChildID).kind != EventKind::Barrier;
}

StepResult HistoryService::undo() {
    StepResult r;
    const EventNode& cur = nodeRef(currentNodeID_);

    // At the root: nothing to undo. The root is the FLOOR of recorded history —
    // the oldest retained state, whose per-scene snapshot is either the empty
    // scene (project typed from scratch) or the pre-existing text captured when
    // history was first created (§5). Undo restores up TO the floor but never
    // below it; this is a hard stop with a clear notice. Note this is the history
    // *floor*, NOT a per-session wall — crossing session boundaries *within*
    // recorded history warns and continues (crossedSessionBoundary), it does not
    // stop here. (Session-spanning persistence: SP-054.)
    if (currentNodeID_ == rootID_ || !cur.parentID.has_value()) {
        r.stoppedAtBarrier = true;
        r.barrierKind = "historyStart";
        r.barrierNote = "Can't undo past the start of the recorded history.";
        r.nodeID = currentNodeID_;
        r.canUndo = false;
        r.canRedo = canRedo();
        return r;
    }

    // A barrier blocks undo with a notice; the pointer does not move (§4.5).
    if (cur.kind == EventKind::Barrier) {
        r.stoppedAtBarrier = true;
        r.barrierKind = cur.barrierKind;
        r.barrierNote = cur.barrierNote;
        r.nodeID = currentNodeID_;
        r.canUndo = false;
        r.canRedo = canRedo();
        return r;
    }

    // Reverse the departed node's diff to get the parent's scene text.
    const std::string sceneID = cur.sceneID;
    const std::string curText = headTextForScene(sceneID);
    const std::string parentText = applyReverse(curText, cur.diff);

    const std::string parentID = *cur.parentID;
    currentNodeID_ = parentID;
    headText_[sceneID] = parentText;

    SceneChange change;
    change.sceneID = sceneID;
    change.newText = parentText;
    change.cursorAfter = cur.cursorBefore;   // restore where the cursor was
    r.change = change;
    r.moved = true;
    r.nodeID = currentNodeID_;
    r.canUndo = canUndo();
    r.canRedo = canRedo();

    // Session-boundary crossing (§5): warn when the node we undo *into* belongs
    // to a session other than the current (open) one — i.e. we are stepping into
    // a previous session's work — and we have not already warned for crossing
    // into that session this run. Compared against sessionID_ (the current open
    // session), so it fires even when the entire loaded chain predates this
    // launch. `warnedSessions_` makes it once-per-crossing.
    const EventNode& landed = nodeRef(currentNodeID_);
    if (landed.sessionID != sessionID_ &&
        warnedSessions_.find(landed.sessionID) == warnedSessions_.end()) {
        warnedSessions_.insert(landed.sessionID);
        r.crossedSessionBoundary = true;
        r.boundaryTimestamp = landed.timestamp;
    }

    // If undo landed on a fork (>= 2 children), surface the branches (§10 T2.1).
    r.forkAhead = forkAheadAt(currentNodeID_);
    return r;
}

StepResult HistoryService::redo() {
    StepResult r;
    const EventNode& cur = nodeRef(currentNodeID_);

    if (!cur.primaryChildID.has_value()) {
        r.nodeID = currentNodeID_;
        r.canUndo = canUndo();
        r.canRedo = false;
        return r;
    }

    const EventNode& child = nodeRef(*cur.primaryChildID);
    if (child.kind == EventKind::Barrier) {
        // Redo never crosses a barrier automatically.
        r.nodeID = currentNodeID_;
        r.canUndo = canUndo();
        r.canRedo = false;
        return r;
    }

    const std::string sceneID = child.sceneID;
    const std::string curText = headTextForScene(sceneID);
    const std::string childText = applyForward(curText, child.diff);

    currentNodeID_ = child.eventID;
    headText_[sceneID] = childText;

    SceneChange change;
    change.sceneID = sceneID;
    change.newText = childText;
    change.cursorAfter = child.cursorAfter;
    r.change = change;
    r.moved = true;
    r.nodeID = currentNodeID_;
    r.canUndo = canUndo();
    r.canRedo = canRedo();

    // If redo reached a fork (>= 2 children), surface the branches (§10 T2.4).
    r.forkAhead = forkAheadAt(currentNodeID_);
    return r;
}

std::optional<ForkAhead> HistoryService::forkAheadAt(const std::string& nodeID) const {
    const EventNode& n = nodeRef(nodeID);
    if (n.childIDs.size() < 2) return std::nullopt;   // not a fork

    ForkAhead fa;
    fa.nodeID = nodeID;
    for (const std::string& childID : n.childIDs) {
        const EventNode& child = nodeRef(childID);
        ForkChild fc;
        fc.eventID   = child.eventID;
        fc.preview   = forkPreview(child);
        fc.timestamp = child.timestamp;
        fc.isPrimary = n.primaryChildID.has_value() && *n.primaryChildID == childID;
        fa.children.push_back(std::move(fc));
    }
    return fa;
}

SelectBranchResult HistoryService::selectBranch(const std::string& forkNodeID,
                                                const std::string& childEventID) {
    SelectBranchResult r;
    r.forkNodeID = forkNodeID;
    r.childEventID = childEventID;

    auto it = nodes_.find(forkNodeID);
    if (it == nodes_.end()) return r;   // unknown fork → ok stays false
    EventNode& fork = it->second;
    const bool isChild =
        std::find(fork.childIDs.begin(), fork.childIDs.end(), childEventID) != fork.childIDs.end();
    if (!isChild) return r;             // not a child of this fork → reject

    fork.primaryChildID = childEventID;
    r.ok = true;
    // canRedo reflects whether the pointer (unchanged) can now redo onto the
    // re-primaried branch: true when the current node is the fork itself.
    r.canRedo = (currentNodeID_ == forkNodeID) && canRedo();
    return r;
}

std::vector<StaleBranch> HistoryService::listStaleBranches(const std::string& nowIso,
                                                           int staleBranchDays) const {
    std::vector<StaleBranch> out;
    if (staleBranchDays <= 0) return out;             // stale detection disabled

    std::optional<std::int64_t> nowEpoch = parseIso8601Utc(nowIso);
    if (!nowEpoch.has_value()) return out;            // no clock reference → report nothing
    const std::int64_t staleBefore = *nowEpoch - static_cast<std::int64_t>(staleBranchDays) * 86400;

    // The set of nodes on the root→current path — never report a branch that
    // holds the live state.
    std::set<std::string> onPath;
    {
        std::string id = currentNodeID_;
        while (true) {
            onPath.insert(id);
            const EventNode& n = nodeRef(id);
            if (!n.parentID.has_value()) break;
            id = *n.parentID;
        }
    }

    // A branch is a non-primary child subtree: for every node with >= 2 children,
    // each child other than the primary roots a candidate branch.
    for (const auto& [nodeID, node] : nodes_) {
        if (node.childIDs.size() < 2) continue;       // not a fork
        for (const std::string& childID : node.childIDs) {
            const bool isPrimary =
                node.primaryChildID.has_value() && *node.primaryChildID == childID;
            if (isPrimary) continue;                  // the primary line is not a stale branch
            if (onPath.count(childID)) continue;      // holds the live pointer

            // Newest timestamp anywhere in the subtree is the branch's "tip".
            std::string tip;
            std::int64_t tipEpoch = std::numeric_limits<std::int64_t>::min();
            std::vector<std::string> stack{childID};
            while (!stack.empty()) {
                const std::string id = stack.back();
                stack.pop_back();
                auto it = nodes_.find(id);
                if (it == nodes_.end()) continue;
                if (auto e = parseIso8601Utc(it->second.timestamp); e && *e > tipEpoch) {
                    tipEpoch = *e;
                    tip = it->second.timestamp;
                }
                for (const std::string& c : it->second.childIDs) stack.push_back(c);
            }
            // Unparseable timestamps throughout → treat as not stale (skip).
            if (tipEpoch == std::numeric_limits<std::int64_t>::min()) continue;
            if (tipEpoch >= staleBefore) continue;    // tip newer than threshold → not stale

            StaleBranch sb;
            sb.branchRootEventID = childID;
            sb.forkNodeID        = nodeID;
            sb.preview           = forkPreview(nodeRef(childID));
            sb.tipTimestamp      = tip;
            sb.nodeCount         = subtreeNodeCount(childID);
            out.push_back(std::move(sb));
        }
    }
    return out;
}

PurgeResult HistoryService::purgeBranch(const std::string& branchRootEventID) {
    PurgeResult r;
    r.branchRootEventID = branchRootEventID;

    auto it = nodes_.find(branchRootEventID);
    if (it == nodes_.end() || branchRootEventID == rootID_) {
        // Unknown node or the root itself is never purgeable.
        r.canUndo = canUndo();
        r.canRedo = canRedo();
        return r;
    }
    // Reject purging anything on the root→current path: it would strand the live
    // pointer inside a deleted subtree.
    {
        std::string id = currentNodeID_;
        while (true) {
            if (id == branchRootEventID) {
                r.canUndo = canUndo();
                r.canRedo = canRedo();
                return r;                             // on-path → reject (ok stays false)
            }
            const EventNode& n = nodeRef(id);
            if (!n.parentID.has_value()) break;
            id = *n.parentID;
        }
    }

    // Detach from the parent's child list, then erase the subtree.
    const std::optional<std::string> parentID = it->second.parentID;
    if (parentID.has_value()) {
        EventNode& parent = nodeRef(*parentID);
        auto& kids = parent.childIDs;
        kids.erase(std::remove(kids.begin(), kids.end(), branchRootEventID), kids.end());
        if (parent.primaryChildID.has_value() && *parent.primaryChildID == branchRootEventID) {
            // The purged branch was primary — fall back to the first surviving
            // child (or none), so redo still has a defined primary line.
            parent.primaryChildID = kids.empty() ? std::optional<std::string>{} : kids.front();
        }
    }
    r.purgedCount = eraseSubtree(branchRootEventID);
    r.ok = true;
    r.canUndo = canUndo();
    r.canRedo = canRedo();
    return r;
}

int HistoryService::subtreeNodeCount(const std::string& subtreeRootID) const {
    int count = 0;
    std::vector<std::string> stack{subtreeRootID};
    while (!stack.empty()) {
        const std::string id = stack.back();
        stack.pop_back();
        auto it = nodes_.find(id);
        if (it == nodes_.end()) continue;
        ++count;
        for (const std::string& c : it->second.childIDs) stack.push_back(c);
    }
    return count;
}

void HistoryService::rebuildHeadCache() {
    // Walk root→current collecting the path, then apply diffs forward per scene.
    std::vector<const EventNode*> path;
    const std::string* id = &currentNodeID_;
    while (true) {
        const EventNode& n = nodeRef(*id);
        path.push_back(&n);
        if (!n.parentID.has_value()) break;
        id = &(*n.parentID);
    }
    std::reverse(path.begin(), path.end());

    // Start each scene from its floor snapshot (empty if never seeded), then
    // apply the diffs along the root→current path. The ROOT node's own diff is
    // NOT applied: the root represents the floor state (its diff was folded into
    // floorTexts_ at eviction, or it is the original empty anchor).
    headText_ = floorTexts_;
    for (const EventNode* n : path) {
        if (n->kind == EventKind::Barrier) continue;
        if (n->eventID == rootID_) continue;
        auto& text = headText_[n->sceneID];
        text = applyForward(text, n->diff);
    }
}

// Returns the root's child that lies on the root→current path (i.e. is current
// or an ancestor of current), or empty if current IS the root. This child must
// be promoted — never evicted — so the current state stays reachable (§5).
std::string HistoryService::rootChildTowardCurrent() const {
    if (currentNodeID_ == rootID_) return {};
    // Walk up from current to the node whose parent is the root.
    std::string id = currentNodeID_;
    while (true) {
        const EventNode& n = nodeRef(id);
        if (!n.parentID.has_value()) return {};   // reached an orphan; shouldn't happen
        if (*n.parentID == rootID_) return id;     // this child is on the path
        id = *n.parentID;
    }
}

// Erases `subtreeRootID` and its entire descendant subtree from nodes_. Used to
// auto-purge a non-primary branch hanging off the root at eviction (§5), and by
// user-confirmed purge (T-0212). Returns the number of nodes removed.
int HistoryService::eraseSubtree(const std::string& subtreeRootID) {
    int removed = 0;
    std::vector<std::string> stack{subtreeRootID};
    while (!stack.empty()) {
        const std::string id = stack.back();
        stack.pop_back();
        auto it = nodes_.find(id);
        if (it == nodes_.end()) continue;
        for (const std::string& childID : it->second.childIDs) stack.push_back(childID);
        nodes_.erase(it);
        ++removed;
    }
    return removed;
}

int HistoryService::evictToCapacity(EvictionDetail& detail) {
    if (capacityEvents_ <= 0) return 0;   // unlimited
    int evicted = 0;
    while (eventCount() > capacityEvents_) {
        // The child that must survive: the one on the root→current path. If the
        // current pointer IS the root, there is nothing safe to promote — DEFER
        // (the current state must always remain reachable; §5).
        const std::string keepChildID = rootChildTowardCurrent();
        if (keepChildID.empty()) break;

        EventNode& root = nodeRef(rootID_);
        // Auto-purge every OTHER child subtree hanging off the root — a branch
        // dies when its branch point falls off the history limit (§5). Collect
        // first (mutating childIDs while iterating is unsafe).
        std::vector<std::string> purgeRoots;
        for (const std::string& childID : root.childIDs) {
            if (childID != keepChildID) purgeRoots.push_back(childID);
        }
        for (const std::string& p : purgeRoots) {
            evicted += eraseSubtree(p);
            detail.purgedBranchRoots.push_back(p);   // record for ctl:purge
        }

        // Promote the surviving child to the new root, folding its diff into the
        // per-scene floor so replay from the new root reproduces the same text.
        EventNode& keep = nodeRef(keepChildID);
        if (keep.kind != EventKind::Barrier) {
            auto& floor = floorTexts_[keep.sceneID];
            floor = applyForward(floor, keep.diff);
        }
        keep.parentID.reset();
        nodes_.erase(rootID_);
        rootID_ = keepChildID;
        detail.promotedRoots.push_back(keepChildID);   // record for ctl:evict
        ++evicted;
    }
    if (evicted > 0) rebuildHeadCache();
    return evicted;
}

void HistoryService::addLoadedFloor(const std::string& sceneID, std::string text) {
    floorTexts_[sceneID] = text;
    headText_[sceneID]   = std::move(text);
}

void HistoryService::addLoadedNode(EventNode node) {
    const std::string id = node.eventID;
    nodes_[id] = std::move(node);
}

void HistoryService::setPointers(std::string rootID, std::string currentNodeID,
                                 std::string sessionID) {
    rootID_        = std::move(rootID);
    currentNodeID_ = std::move(currentNodeID);
    sessionID_     = std::move(sessionID);
}

void HistoryService::finalizeLoad() {
    // Derive childIDs from parent links, in eventID (map) order. Records replay in
    // append (seq) order, and eventIDs are minted monotonically, so this preserves
    // creation order. The last-recorded child is the DEFAULT primary — correct for
    // a single-child chain, and the correct fallback for a fork with no persisted
    // override. Real fork overrides (ctl:setPrimary / state.json primaryChildren)
    // are applied AFTER this by HistoryStore via applyPrimaryOverride() (D4/SP-055),
    // so a re-primaried branch survives close/reopen instead of snapping back to
    // "last child wins".
    for (auto& [id, node] : nodes_) { node.childIDs.clear(); node.primaryChildID.reset(); }
    for (auto& [id, node] : nodes_) {
        if (node.parentID.has_value()) {
            auto it = nodes_.find(*node.parentID);
            if (it != nodes_.end()) {
                it->second.childIDs.push_back(id);
                it->second.primaryChildID = id;   // default: last child wins
            }
        }
    }
    rebuildHeadCache();
}

void HistoryService::applyLoadedEviction(const std::vector<std::string>& purgedBranchRoots,
                                         const std::vector<std::string>& promotedRoots) {
    if (purgedBranchRoots.empty() && promotedRoots.empty()) return;

    // Drop each purged non-primary subtree (childIDs are already derived).
    // Detach the subtree root from its parent's child list FIRST so no stale
    // child reference is left behind — otherwise a surviving parent (a user
    // purge with no accompanying root promotion, T-0212) would still list the
    // erased branch and rebuildHeadCache would walk into an unknown node.
    for (const std::string& branchRoot : purgedBranchRoots) {
        auto it = nodes_.find(branchRoot);
        if (it == nodes_.end()) continue;
        if (it->second.parentID.has_value()) {
            auto pit = nodes_.find(*it->second.parentID);
            if (pit != nodes_.end()) {
                auto& kids = pit->second.childIDs;
                kids.erase(std::remove(kids.begin(), kids.end(), branchRoot), kids.end());
                if (pit->second.primaryChildID.has_value() &&
                    *pit->second.primaryChildID == branchRoot) {
                    pit->second.primaryChildID =
                        kids.empty() ? std::optional<std::string>{} : kids.back();
                }
            }
        }
        eraseSubtree(branchRoot);
    }

    // Advance the root along each promotion, mirroring evictToCapacity(): fold
    // the promoted child's diff into the floor, detach it, erase the old root.
    for (const std::string& newRoot : promotedRoots) {
        auto it = nodes_.find(newRoot);
        if (it == nodes_.end()) continue;   // already gone — nothing to promote
        EventNode& keep = it->second;
        if (keep.kind != EventKind::Barrier) {
            auto& floor = floorTexts_[keep.sceneID];
            floor = applyForward(floor, keep.diff);
        }
        const std::string oldRoot = rootID_;
        keep.parentID.reset();
        rootID_ = newRoot;
        if (oldRoot != newRoot) nodes_.erase(oldRoot);
    }

    rebuildHeadCache();
}

void HistoryService::applyPrimaryOverride(const std::string& forkNodeID,
                                          const std::string& childEventID) {
    auto it = nodes_.find(forkNodeID);
    if (it == nodes_.end()) return;
    EventNode& fork = it->second;
    const bool isChild =
        std::find(fork.childIDs.begin(), fork.childIDs.end(), childEventID) != fork.childIDs.end();
    if (!isChild) return;
    fork.primaryChildID = childEventID;
    // The override may change which branch is on the root→current path only when
    // it affects a node on that path; rebuild the head cache to stay consistent.
    rebuildHeadCache();
}

} // namespace scrivi::history
