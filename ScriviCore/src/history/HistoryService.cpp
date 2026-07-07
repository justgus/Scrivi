#include "history/HistoryService.hpp"

#include <algorithm>
#include <stdexcept>

namespace scrivi::history {

namespace {

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
    }
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
    parent.childIDs.push_back(newID);
    // Linear engine: the new node always becomes the primary child. (Branching —
    // sibling-becomes-primary while preserving old chains — is SP-055.)
    parent.primaryChildID = newID;

    nodes_.emplace(newID, std::move(node));
    currentNodeID_ = newID;
    headText_[p.sceneID] = p.newSceneText;

    RecordResult r;
    r.eventID = newID;
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

    // Session-boundary crossing: the node we just left belonged to a session
    // that differs from the node we landed on. Warn once (§5).
    const EventNode& landed = nodeRef(currentNodeID_);
    if (landed.sessionID != cur.sessionID) {
        r.crossedSessionBoundary = true;
        r.boundaryTimestamp = landed.timestamp;
    }
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
    return r;
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

    headText_.clear();
    for (const EventNode* n : path) {
        if (n->kind == EventKind::Barrier) continue;
        auto& text = headText_[n->sceneID];
        text = applyForward(text, n->diff);
    }
}

} // namespace scrivi::history
