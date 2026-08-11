#include "history/HistoryService.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <deque>      // getTree — breadth-first descendant walk (T-0394)
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

// True when a node carries an applicable scene-local diff (i.e. is a textual event).
// The root anchor, hard barriers, and reversible structural nodes (T-0356) carry no
// diff, so eviction/promotion/prune/head-rebuild must NOT fold or match a diff for them.
bool carriesDiff(EventKind kind) {
    return kind != EventKind::Barrier && kind != EventKind::Structural;
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
//
// Defensive against a diff that does not match `oldText` — e.g. a persisted history
// event replayed against a scene whose baseline has since changed (deleted/edited
// externally), or a corrupt log. Such a mismatch must NOT crash: previously
// `reserve(oldText.size() - removed.size() + ...)` underflowed (unsigned) to a huge
// value and threw std::length_error, and `append(oldText, offset+removed, npos)`
// could throw std::out_of_range. Both are now clamped so replay degrades to a
// best-effort result instead of terminating the process (I-0065). The offset and the
// removed span are clamped into `oldText`; the tail is whatever remains after them.
std::string applyForward(const std::string& oldText, const Diff& d) {
    const std::size_t off  = std::min(d.offsetUtf8, oldText.size());
    const std::size_t cut  = std::min(off + d.removed.size(), oldText.size());
    std::string out;
    out.reserve(off + d.inserted.size() + (oldText.size() - cut));
    out.append(oldText, 0, off);
    out.append(d.inserted);
    out.append(oldText, cut, std::string::npos);
    return out;
}

// Applies a diff in reverse: new scene text → old scene text. Same clamping as
// applyForward (I-0065): the offset and the inserted span are clamped into `newText`
// so a mismatched/stale diff yields a best-effort result rather than throwing.
std::string applyReverse(const std::string& newText, const Diff& d) {
    const std::size_t off = std::min(d.offsetUtf8, newText.size());
    const std::size_t cut = std::min(off + d.inserted.size(), newText.size());
    std::string out;
    out.reserve(off + d.removed.size() + (newText.size() - cut));
    out.append(newText, 0, off);
    out.append(d.removed);
    out.append(newText, cut, std::string::npos);
    return out;
}

// True iff diff `d` is self-consistent against `oldText`: its offset lies within
// the text and the bytes it claims to remove actually appear there. A false result
// means the diff cannot have been produced from `oldText` — the log is corrupt or
// the node is an orphan of a deleted/changed scene (I-0065 / I-0066). applyForward
// clamps such a diff to a best-effort result; this predicate is how load-time
// pruning *detects* it so the bad node can be dropped rather than silently mangled.
bool diffMatches(const std::string& oldText, const Diff& d) {
    if (d.offsetUtf8 > oldText.size()) return false;
    if (d.offsetUtf8 + d.removed.size() > oldText.size()) return false;
    return oldText.compare(d.offsetUtf8, d.removed.size(), d.removed) == 0;
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

// T-0397 — classify an all-whitespace change so the UI can NAME it.
//
// forkPreview() rewrites \n/\r/\t to spaces and the card then trims, so a
// newline-only event arrived at the UI indistinguishable from an empty one and
// rendered as "(no text)" — the writer saw three such rows and could not tell what
// they were. The kind has to survive the diff rather than being flattened at preview
// time, so it travels as its own field.
//
// Returns "" for anything containing a non-whitespace byte (the overwhelmingly
// common case, where the preview text speaks for itself), otherwise the dominant
// whitespace kind plus its run length: "newline:2", "tab:1", "space:3".
// Mixed runs report whichever kind occurs most, ties resolving newline > tab > space
// — the coarser boundary is the more meaningful thing to name.
std::string whitespaceKind(const EventNode& node) {
    const std::string& src = !node.diff.inserted.empty() ? node.diff.inserted
                                                         : node.diff.removed;
    if (src.empty()) return {};

    std::size_t newlines = 0, tabs = 0, spaces = 0;
    for (const char c : src) {
        switch (c) {
            case '\n': ++newlines; break;
            case '\r': break;               // CRLF counts once, via the \n
            case '\t': ++tabs;    break;
            case ' ':  ++spaces;  break;
            default: return {};             // real text — nothing to name
        }
    }

    if (newlines > 0 && newlines >= tabs && newlines >= spaces)
        return "newline:" + std::to_string(newlines);
    if (tabs > 0 && tabs >= spaces)
        return "tab:" + std::to_string(tabs);
    if (spaces > 0)
        return "space:" + std::to_string(spaces);
    return {};                              // e.g. a lone \r
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
    node.bufferID = p.bufferID;   // cut-into-buffer provenance (Trade T3)

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
    // A non-empty structuralPayload makes this a REVERSIBLE structural node (EP-029 AC6 /
    // T-0356) that undo/redo step across, instead of a hard barrier that blocks undo.
    node.kind = p.structuralPayload.empty() ? EventKind::Barrier : EventKind::Structural;
    node.barrierKind = p.barrierKind;
    node.barrierNote = p.barrierNote;
    // BarrierParams has always carried sceneID, but recordBarrier never copied it —
    // so every barrier node (624 of 947 in a real project) had an EMPTY sceneID and
    // could not be attributed to a scene. That is what makes a 900-row history card
    // unreadable and blocks per-scene filtering. Fixed 2026-08-06 (I-0102).
    node.sceneID = p.sceneID;
    node.structuralPayload = p.structuralPayload;
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
    // Can undo when the current node is a text node OR a reversible structural node
    // (has a parent and is not the root). A hard Barrier blocks undo; a Structural node
    // (EP-029 AC6 / T-0356) is steppable — undo replays its inverse op.
    if (currentNodeID_ == rootID_) return false;
    const EventNode& cur = nodeRef(currentNodeID_);
    return cur.kind != EventKind::Barrier && cur.parentID.has_value();
}

bool HistoryService::canRedo() const {
    const EventNode& cur = nodeRef(currentNodeID_);
    if (!cur.primaryChildID.has_value()) return false;
    // Redo re-applies text nodes and reversible structural nodes; a hard barrier ahead
    // is not redoable, but a Structural node (T-0356) is (redo re-runs its forward op).
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

    // A reversible structural node (EP-029 AC6 / T-0356): step across it and hand the
    // app the inverse-op payload to replay (undo direction). No SceneChange — the app
    // runs the inverse fragment op + reloads the manuscript from disk. The pointer moves
    // to the parent so a subsequent undo continues into the pre-structural history.
    if (cur.kind == EventKind::Structural) {
        currentNodeID_ = *cur.parentID;
        r.moved = true;
        r.crossedStructural = true;
        r.structuralDirection = "undo";
        r.structuralPayload = cur.structuralPayload;
        r.nodeID = currentNodeID_;
        r.canUndo = canUndo();
        r.canRedo = canRedo();
        // Session-boundary warning still applies to where we landed.
        const EventNode& landed = nodeRef(currentNodeID_);
        if (landed.sessionID != sessionID_ &&
            warnedSessions_.find(landed.sessionID) == warnedSessions_.end()) {
            warnedSessions_.insert(landed.sessionID);
            r.crossedSessionBoundary = true;
            r.boundaryTimestamp = landed.timestamp;
        }
        r.forkAhead = forkAheadAt(currentNodeID_);
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

    // A reversible structural node (EP-029 AC6 / T-0356): step onto it and hand the app
    // the inverse-op payload to replay forward (redo direction). No SceneChange — the app
    // re-runs the structural op + reloads. The pointer advances onto the structural node.
    if (child.kind == EventKind::Structural) {
        currentNodeID_ = child.eventID;
        r.moved = true;
        r.crossedStructural = true;
        r.structuralDirection = "redo";
        r.structuralPayload = child.structuralPayload;
        r.nodeID = currentNodeID_;
        r.canUndo = canUndo();
        r.canRedo = canRedo();
        r.forkAhead = forkAheadAt(currentNodeID_);
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

// --- Windowed tree projection (EP-030 SP-092, T-0394) ----------------------

namespace {

std::string treeKindToStr(EventKind k) {
    switch (k) {
        case EventKind::Typing:     return "typing";
        case EventKind::Delete:     return "delete";
        case EventKind::Replace:    return "replace";
        case EventKind::Paste:      return "paste";
        case EventKind::Cut:        return "cut";
        case EventKind::Barrier:    return "barrier";
        case EventKind::Structural: return "structural";
    }
    return "typing";
}

// Default window size. Large enough that a card shows meaningful context without
// scrolling for it, small enough that a pathological history never serializes whole.
constexpr int kDefaultMaxNodes = 200;

} // namespace

TreeWindow HistoryService::getTree(const std::string& aroundNodeID, int maxNodes) const {
    TreeWindow out;
    out.rootID         = rootID_;
    out.currentNodeID  = currentNodeID_;
    out.totalNodeCount = static_cast<int>(nodes_.size());

    const int cap = maxNodes > 0 ? maxNodes : kDefaultMaxNodes;

    // An unknown anchor (e.g. a node purged since the card last rendered) falls back
    // to the current node rather than erroring — the card should still draw.
    std::string anchor = aroundNodeID;
    if (anchor.empty() || nodes_.find(anchor) == nodes_.end()) {
        anchor = currentNodeID_;
    }

    // The primary spine: root→current following primaryChildID. Used to flag nodes so
    // the card can draw the spine distinctly from side branches.
    std::set<std::string> spine;
    {
        // Walk UP from current to root — parentID is always single-valued, so this is
        // the unambiguous direction. (Walking down via primaryChildID can diverge from
        // the current pointer after a branch selection.)
        std::string id = currentNodeID_;
        while (!id.empty()) {
            spine.insert(id);
            auto it = nodes_.find(id);
            if (it == nodes_.end() || !it->second.parentID.has_value()) break;
            id = *it->second.parentID;
        }
    }

    // I-0107 — rebase each spine node's change offset onto the CURRENT text.
    //
    // diff.offsetUtf8 records where an edit landed at the moment it was recorded. Every
    // later edit in the same scene shifts that position, but the stored offset never
    // moves — so the card was testing the caret against historical coordinates. The
    // writer saw entries bold two characters late (their two leading newlines), and
    // entries whose stale span had drifted out of reach never bold at all.
    //
    // A later edit moves an older one ONLY IF it happened at or before the older
    // position — text inserted *after* a node leaves that node exactly where it was.
    // Each older node is therefore transformed through every newer edit individually,
    // oldest-first, the way operational transforms compose.
    //
    // ⚠️ A single running total is WRONG and was the first attempt: it shifted every
    // node by the full length of all later edits regardless of position, so an entry
    // followed by seven typed sentences was displaced by all seven (user-reported
    // 2026-08-10 — "highlights only when the cursor is six lines below"). The earlier
    // tests missed it because they only ever inserted at offset 0, which is always
    // "before" and so happens to look correct.
    //
    // Side branches are deliberately not rebased: their text is not in the current
    // document at all, so no caret position corresponds to them.
    std::map<std::string, std::size_t> rebasedOffset;
    {
        // The spine, oldest → newest, so each node can be transformed through the
        // edits that follow it in the order they actually happened.
        std::vector<std::string> spineOldestFirst;
        for (std::string id = currentNodeID_; !id.empty();) {
            auto it = nodes_.find(id);
            if (it == nodes_.end()) break;
            spineOldestFirst.push_back(id);
            if (!it->second.parentID.has_value()) break;
            id = *it->second.parentID;
        }
        std::reverse(spineOldestFirst.begin(), spineOldestFirst.end());

        for (std::size_t i = 0; i < spineOldestFirst.size(); ++i) {
            auto it = nodes_.find(spineOldestFirst[i]);
            if (it == nodes_.end()) continue;
            const EventNode& node = it->second;
            if (!carriesDiff(node.kind) || node.sceneID.empty()) continue;

            std::int64_t pos = static_cast<std::int64_t>(node.diff.offsetUtf8);

            // Transform through each SUBSEQUENT edit in the same scene.
            for (std::size_t j = i + 1; j < spineOldestFirst.size(); ++j) {
                auto jt = nodes_.find(spineOldestFirst[j]);
                if (jt == nodes_.end()) continue;
                const EventNode& later = jt->second;
                if (!carriesDiff(later.kind) || later.sceneID != node.sceneID) continue;

                const std::int64_t lOff = static_cast<std::int64_t>(later.diff.offsetUtf8);
                const std::int64_t lIns = static_cast<std::int64_t>(later.diff.inserted.size());
                const std::int64_t lDel = static_cast<std::int64_t>(later.diff.removed.size());

                // Strictly after this node's position → no effect on it.
                if (lOff > pos) continue;

                // A removal spanning this position collapses it back to the cut point;
                // otherwise the net change applies in full.
                if (lDel > 0 && lOff + lDel > pos) {
                    pos = lOff;
                } else {
                    pos += lIns - lDel;
                }
                if (pos < 0) pos = 0;
            }

            rebasedOffset[node.eventID] = static_cast<std::size_t>(pos);
        }
    }

    // Collect outward from the anchor: ancestors first (the writer's spine is the most
    // valuable context), then descendants breadth-first.
    std::vector<std::string> ordered;
    std::set<std::string> seen;

    for (std::string id = anchor; !id.empty(); ) {
        if (!seen.insert(id).second) break;
        ordered.push_back(id);
        auto it = nodes_.find(id);
        if (it == nodes_.end() || !it->second.parentID.has_value()) break;
        id = *it->second.parentID;
    }

    std::deque<std::string> queue{anchor};
    while (!queue.empty() && static_cast<int>(ordered.size()) < cap) {
        const std::string id = queue.front();
        queue.pop_front();
        auto it = nodes_.find(id);
        if (it == nodes_.end()) continue;
        for (const std::string& childID : it->second.childIDs) {
            if (!seen.insert(childID).second) continue;
            ordered.push_back(childID);
            queue.push_back(childID);
            if (static_cast<int>(ordered.size()) >= cap) break;
        }
    }

    if (static_cast<int>(ordered.size()) > cap) { ordered.resize(static_cast<std::size_t>(cap)); }

    out.nodes.reserve(ordered.size());
    for (const std::string& id : ordered) {
        auto it = nodes_.find(id);
        if (it == nodes_.end()) continue;
        const EventNode& n = it->second;

        TreeNode t;
        t.eventID        = n.eventID;
        t.parentID       = n.parentID.value_or("");
        t.primaryChildID = n.primaryChildID.value_or("");
        t.childIDs       = n.childIDs;
        t.kind           = treeKindToStr(n.kind);
        t.sceneID        = n.sceneID;
        t.preview        = forkPreview(n);
        t.timestamp      = n.timestamp;
        t.sessionID      = n.sessionID;
        t.bufferID       = n.bufferID;
        t.barrierKind    = n.barrierKind;
        t.barrierNote    = n.barrierNote;
        t.onPrimarySpine = spine.count(n.eventID) > 0;
        t.isCurrent      = (n.eventID == currentNodeID_);
        // I-0107: report the offset this event's text occupies in the CURRENT scene,
        // not where it landed when recorded. Off-spine nodes keep their stored offset —
        // their text is not in the document, so no caret can be inside them anyway.
        {
            auto rb = rebasedOffset.find(n.eventID);
            t.changeOffsetUtf8 = (rb != rebasedOffset.end()) ? rb->second : n.diff.offsetUtf8;
        }
        t.changeLength     = n.diff.inserted.size();
        t.removedLength    = n.diff.removed.size();   // I-0106 / T-0398
        t.whitespaceKind   = whitespaceKind(n);       // T-0397
        out.nodes.push_back(std::move(t));
    }

    out.truncated = static_cast<int>(out.nodes.size()) < out.totalNodeCount;
    return out;
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
        if (!carriesDiff(n->kind)) continue;   // root/barrier/structural carry no diff
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
        if (carriesDiff(keep.kind)) {
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
        if (carriesDiff(keep.kind)) {
            auto& floor = floorTexts_[keep.sceneID];
            floor = applyForward(floor, keep.diff);
        }
        const std::string oldRoot = rootID_;
        keep.parentID.reset();
        rootID_ = newRoot;
        if (oldRoot != newRoot) nodes_.erase(oldRoot);
    }

    // I-0110: the current pointer may have been INSIDE a subtree we just erased.
    //
    // Replaying a ctl:purge whose branch root is an ancestor of the loaded
    // currentNodeID deletes that node along with the subtree, leaving the pointer
    // dangling. The very next nodeRef() then threw `unknown node <id>`, which
    // propagated out of scrivi_history_open as an unhandled exception — so the
    // project opened with NO undo/redo at all, not merely a degraded history.
    //
    // Reported 2026-08-11 on the writer's real project: a purge record (of a branch
    // that was an ancestor of the newest node) sat at the very end of the log, so
    // every open replayed it and immediately failed.
    //
    // `pruneInconsistentNodes` already handles exactly this case at line ~1084; the
    // eviction path simply never got the same guard. Walk back to the nearest
    // surviving ancestor, root at worst — the same rule, so the two paths agree.
    if (nodes_.find(currentNodeID_) == nodes_.end()) {
        currentNodeID_ = rootID_;
    }

    rebuildHeadCache();
}

std::vector<std::string> HistoryService::pruneInconsistentNodes() {
    std::vector<std::string> droppedRoots;   // subtree roots detached (for the log)
    bool anyDropped = false;

    // DFS from the root carrying the per-scene replayed text down each path. A node
    // whose diff does not match its scene's text at that point cannot have been
    // produced from it (orphan of a deleted/changed scene, or corrupt record) — drop
    // it and its whole subtree, and do not recurse into it. childIDs must already be
    // derived (finalizeLoad ran). We iterate a work stack of (nodeID, per-scene text
    // snapshot) so sibling branches each see the correct pre-fork state.
    struct Frame { std::string id; std::map<std::string, std::string> text; };
    std::vector<Frame> stack;
    stack.push_back({rootID_, floorTexts_});

    while (!stack.empty()) {
        Frame frame = std::move(stack.back());
        stack.pop_back();

        auto nit = nodes_.find(frame.id);
        if (nit == nodes_.end()) continue;      // already erased via a parent
        const EventNode& node = nit->second;

        // Apply this node's diff to its scene (the root/barriers/structural carry none),
        // after validating it. A mismatch on a non-root textual node → prune the subtree.
        if (node.eventID != rootID_ && carriesDiff(node.kind)) {
            auto& sceneText = frame.text[node.sceneID];
            if (!diffMatches(sceneText, node.diff)) {
                // Detach from parent so no stale child ref survives, then erase the
                // subtree. Record every erased eventID (subtree included) for the log.
                if (node.parentID.has_value()) {
                    auto pit = nodes_.find(*node.parentID);
                    if (pit != nodes_.end()) {
                        auto& kids = pit->second.childIDs;
                        kids.erase(std::remove(kids.begin(), kids.end(), node.eventID),
                                   kids.end());
                        if (pit->second.primaryChildID.has_value() &&
                            *pit->second.primaryChildID == node.eventID) {
                            pit->second.primaryChildID =
                                kids.empty() ? std::optional<std::string>{} : kids.back();
                        }
                    }
                }
                // Record just this subtree ROOT — one ctl:purge replays the whole
                // subtree drop. eraseSubtree removes the root + all descendants.
                droppedRoots.push_back(node.eventID);
                anyDropped = true;
                eraseSubtree(node.eventID);
                continue;   // do not recurse into an erased subtree
            }
            sceneText = applyForward(sceneText, node.diff);
        }

        // Recurse into children with this node's (now-advanced) per-scene text.
        for (const std::string& childID : node.childIDs) {
            stack.push_back({childID, frame.text});
        }
    }

    if (anyDropped) {
        // The current pointer may have been inside a dropped subtree — walk it back
        // to the nearest surviving ancestor (root at worst) so the tree stays valid.
        if (nodes_.find(currentNodeID_) == nodes_.end()) {
            currentNodeID_ = rootID_;
        }
        rebuildHeadCache();
    }
    return droppedRoots;
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
