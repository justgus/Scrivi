#include "history/HistoryStore.hpp"

#include "util/Json.hpp"
#include "util/PathUtils.hpp"
#include "util/Hash.hpp"

#include <sstream>

namespace scrivi::history {

namespace {

// EventKind <-> wire string.
std::string kindToStr(EventKind k) {
    switch (k) {
        case EventKind::Typing:  return "typing";
        case EventKind::Delete:  return "delete";
        case EventKind::Replace: return "replace";
        case EventKind::Paste:   return "paste";
        case EventKind::Cut:     return "cut";
        case EventKind::Barrier: return "barrier";
    }
    return "typing";
}

EventKind kindFromStr(const std::string& s) {
    if (s == "delete")  return EventKind::Delete;
    if (s == "replace") return EventKind::Replace;
    if (s == "paste")   return EventKind::Paste;
    if (s == "cut")     return EventKind::Cut;
    if (s == "barrier") return EventKind::Barrier;
    return EventKind::Typing;
}

} // namespace

HistoryStore::HistoryStore(AbsolutePath historyDir, FileSystem* fs, Clock* clock)
    : historyDir_(std::move(historyDir)), fs_(fs), clock_(clock) {}

AbsolutePath HistoryStore::logPath() const {
    return util::join(historyDir_, activeSegment_);
}

AbsolutePath HistoryStore::statePath() const {
    return util::join(historyDir_, "state.json");
}

void HistoryStore::appendLine(const std::string& jsonLine) {
    if (!fs_) return;
    fs_->appendTextFile(logPath(), jsonLine + "\n");
    if (++recordsSinceCheckpoint_ >= 200) {
        checkpoint();
        recordsSinceCheckpoint_ = 0;
    }
}

bool HistoryStore::openOrCreate(const std::string& newSessionID,
                                const std::string& nowTimestamp, bool& outLoaded) {
    outLoaded = false;
    if (fs_) { fs_->createDirectories(historyDir_); }

    // Try to load an existing log by replaying its records.
    bool haveLog = false;
    std::string logText;
    if (fs_) {
        auto existsR = fs_->exists(logPath());
        if (existsR.ok() && existsR.value()) {
            auto readR = fs_->readTextFile(logPath());
            if (readR.ok()) { logText = readR.value(); haveLog = true; }
        }
    }

    if (haveLog && !logText.empty()) {
        auto svc = std::make_unique<HistoryService>(newSessionID, nowTimestamp);
        // Reset to an empty tree we control the pointers of.
        std::string rootID = "evt_root";
        std::string currentNodeID = rootID;
        std::string loadedSessionID = newSessionID;
        std::int64_t maxSeq = 0;
        bool sawAnyRecord = false;
        // Fork re-selections (ctl:setPrimary) collected during replay; applied
        // after finalizeLoad() derives childIDs (D4/SP-055). forkNodeID → childID.
        std::map<std::string, std::string> primaryOverrides;
        // Branch-aware eviction (§4.1) collected during replay; applied after
        // finalizeLoad() so evicted branches don't resurrect. Purges first, then
        // promotions in log order (mirrors persistEviction()).
        std::vector<std::string> purgedBranchRoots;
        std::vector<std::string> promotedRoots;

        std::istringstream ss(logText);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            auto parsedR = util::parseJson(line);
            if (!parsedR.ok()) {
                // Torn final line (partial write) — stop; everything before is intact.
                break;
            }
            const util::JsonDoc& d = parsedR.value();
            const std::string rec = d.getString("rec");
            const std::int64_t seq = d.getInt64("seq");
            if (seq > maxSeq) maxSeq = seq;
            sawAnyRecord = true;

            if (rec == "floor") {
                svc->addLoadedFloor(d.getString("sceneID"), d.getString("text"));
            } else if (rec == "event") {
                EventNode node;
                node.eventID = d.getString("eventID");
                if (d.contains("parentID") && !d.getString("parentID").empty()) {
                    node.parentID = d.getString("parentID");
                }
                node.kind = kindFromStr(d.getString("kind"));
                node.sceneID = d.getString("sceneID");
                node.cursorBefore = d.getInt64("cursorBefore");
                node.cursorAfter  = d.getInt64("cursorAfter");
                node.timestamp = d.getString("timestamp");
                node.sessionID = d.getString("sessionID");
                if (node.kind == EventKind::Barrier) {
                    util::JsonDoc b = d.getSubDoc("barrier");
                    node.barrierKind = b.getString("barrierKind");
                    node.barrierNote = b.getString("note");
                } else {
                    util::JsonDoc diff = d.getSubDoc("diff");
                    node.diff.offsetUtf8 = static_cast<std::size_t>(diff.getInt64("offsetUtf8"));
                    node.diff.removed  = diff.getString("removed");
                    node.diff.inserted = diff.getString("inserted");
                }
                // Recording a node advances the current pointer to it; in seq
                // order a later ctl:undo/redo may move it back. Last write wins.
                currentNodeID = node.eventID;
                svc->addLoadedNode(std::move(node));
            } else if (rec == "ctl") {
                const std::string op = d.getString("op");
                if (op == "undo" || op == "redo") {
                    currentNodeID = d.getString("nodeID");
                } else if (op == "session") {
                    loadedSessionID = d.getString("sessionID");
                } else if (op == "setPrimary") {
                    // Collect fork re-selections; applied after finalizeLoad()
                    // (childIDs must exist first). Last write per fork wins.
                    primaryOverrides[d.getString("forkNodeID")] = d.getString("childEventID");
                } else if (op == "purge") {
                    // Branch-aware eviction (§4.1): a non-primary subtree dropped.
                    // Applied after finalizeLoad() so the subtree doesn't resurrect.
                    purgedBranchRoots.push_back(d.getString("branchRootEventID"));
                } else if (op == "evict") {
                    // Root promotion (§4.1): the root advanced to this node.
                    // Applied by applyLoadedEviction() AFTER finalizeLoad(), which
                    // rebuilds from the ORIGINAL root/floor first; the promotion
                    // then re-folds the floor and advances rootID_.
                    promotedRoots.push_back(d.getString("newRootID"));
                }
            }
        }

        if (sawAnyRecord) {
            svc->setPointers(rootID, currentNodeID, loadedSessionID);
            svc->finalizeLoad();
            // §4.1/SP-055: replay branch-aware eviction (drop purged subtrees,
            // advance the root) BEFORE restoring fork primaries — evicted forks
            // are gone, so their overrides become harmless no-ops.
            svc->applyLoadedEviction(purgedBranchRoots, promotedRoots);
            // D4/SP-055: restore persisted fork primaries so a re-selected branch
            // survives reopen instead of snapping back to finalizeLoad()'s
            // "last child wins" default. The log's ctl:setPrimary records are the
            // source of truth; state.json.primaryChildren (loaded below) is a
            // redundant accelerator merged over the top.
            for (const auto& [forkID, childID] : primaryOverrides) {
                svc->applyPrimaryOverride(forkID, childID);
            }
            // I-0066: repair a self-inconsistent log in place. A node whose diff no
            // longer matches its scene (an orphan of a deleted/externally-changed
            // scene, or a corrupt record) is dropped with its subtree, and the
            // current pointer is walked back to a surviving ancestor. We persist a
            // ctl:purge for each dropped subtree root so the log stays clean on the
            // NEXT open (replay honors ctl:purge in applyLoadedEviction). The clamp
            // in applyForward (I-0065) already prevents a crash; this makes the fix
            // durable — the bad history self-heals instead of degrading every open.
            std::vector<std::string> prunedRoots = svc->pruneInconsistentNodes();
            for (const std::string& branchRoot : prunedRoots) {
                persistPurge(branchRoot);   // one ctl:purge per detached subtree root
            }
            // Load persisted per-scene head hashes + settings from state.json for
            // §6.b validation (best-effort; the log already rebuilt the tree).
            if (fs_) {
                auto stR = fs_->readTextFile(statePath());
                if (stR.ok()) {
                    auto sp = util::parseJson(stR.value());
                    if (sp.ok()) {
                        const util::JsonDoc& st = sp.value();
                        util::JsonDoc setDoc = st.getSubDoc("settings");
                        settings_.capacityEvents    = setDoc.getInt("capacityEvents", settings_.capacityEvents);
                        settings_.staleBranchDays   = setDoc.getInt("staleBranchDays", settings_.staleBranchDays);
                        settings_.idleRolloverHours = setDoc.getInt("idleRolloverHours", settings_.idleRolloverHours);
                        util::JsonDoc heads = st.getSubDoc("sceneHeads");
                        for (const auto& [sceneID, _] : svc->floorTexts()) {
                            util::JsonDoc h = heads.getSubDoc(sceneID);
                            const std::string sha = h.getString("sha256");
                            if (!sha.empty()) loadedHeadHashes_[sceneID] = sha;
                        }
                        // Merge state.json fork primaries over the log-derived ones
                        // (D4). Redundant with ctl:setPrimary but cheap insurance if
                        // the checkpoint is newer than the replayed log tail.
                        util::JsonDoc primaries = st.getSubDoc("primaryChildren");
                        for (const std::string& forkID : primaries.objectKeys()) {
                            svc->applyPrimaryOverride(forkID, primaries.getString(forkID));
                        }
                    }
                }
            }
            // Mint a NEW session for this open (Trade T5) — the loaded sessionID
            // is what distinguishes "previous session" nodes for the warning.
            svc->setSessionID(newSessionID);
            service_ = std::move(svc);
            lastSeq_ = maxSeq;
            outLoaded = true;
            // Record the session start so subsequent events carry it.
            persistCtl("session", "");
            return true;
        }
    }

    // Fresh history.
    service_ = std::make_unique<HistoryService>(newSessionID, nowTimestamp);
    lastSeq_ = 0;
    persistCtl("session", "");
    checkpoint();
    return true;
}

void HistoryStore::persistFloor(const std::string& sceneID, const std::string& text,
                                const std::string& reason) {
    util::JsonDoc d;
    d.setString("rec", "floor");
    d.setInt64("seq", ++lastSeq_);
    d.setString("sceneID", sceneID);
    d.setString("reason", reason);
    d.setString("text", text);
    d.setString("sha256", util::sha256Hex(text));
    d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
    appendLine(d.dump(-1));
}

void HistoryStore::persistEvent(const EventNode& node) {
    util::JsonDoc d;
    d.setString("rec", "event");
    d.setInt64("seq", ++lastSeq_);
    d.setString("eventID", node.eventID);
    d.setString("parentID", node.parentID.value_or(""));
    d.setString("kind", kindToStr(node.kind));
    d.setString("sceneID", node.sceneID);
    d.setInt64("cursorBefore", node.cursorBefore);
    d.setInt64("cursorAfter", node.cursorAfter);
    d.setString("timestamp", node.timestamp);
    d.setString("sessionID", node.sessionID);
    if (node.kind == EventKind::Barrier) {
        util::JsonDoc b;
        b.setString("barrierKind", node.barrierKind);
        b.setString("note", node.barrierNote);
        d.setSubDoc("barrier", std::move(b));
    } else {
        util::JsonDoc diff;
        diff.setInt64("offsetUtf8", static_cast<std::int64_t>(node.diff.offsetUtf8));
        diff.setString("removed", node.diff.removed);
        diff.setString("inserted", node.diff.inserted);
        d.setSubDoc("diff", std::move(diff));
    }
    appendLine(d.dump(-1));
}

void HistoryStore::persistCtl(const std::string& op, const std::string& nodeID) {
    util::JsonDoc d;
    d.setString("rec", "ctl");
    d.setInt64("seq", ++lastSeq_);
    d.setString("op", op);
    d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
    if (!nodeID.empty()) d.setString("nodeID", nodeID);
    if (op == "session" && service_) d.setString("sessionID", service_->sessionID());
    appendLine(d.dump(-1));
}

void HistoryStore::persistSetPrimary(const std::string& forkNodeID,
                                     const std::string& childEventID) {
    util::JsonDoc d;
    d.setString("rec", "ctl");
    d.setInt64("seq", ++lastSeq_);
    d.setString("op", "setPrimary");
    d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
    d.setString("forkNodeID", forkNodeID);
    d.setString("childEventID", childEventID);
    appendLine(d.dump(-1));
}

void HistoryStore::persistEviction(const EvictionDetail& detail) {
    // Purge records first (the subtrees dropped), then the promotions in order —
    // replay applies them the same way: drop the subtree, then advance the root.
    for (const std::string& branchRoot : detail.purgedBranchRoots) {
        util::JsonDoc d;
        d.setString("rec", "ctl");
        d.setInt64("seq", ++lastSeq_);
        d.setString("op", "purge");
        d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
        d.setString("branchRootEventID", branchRoot);
        appendLine(d.dump(-1));
    }
    for (const std::string& newRoot : detail.promotedRoots) {
        util::JsonDoc d;
        d.setString("rec", "ctl");
        d.setInt64("seq", ++lastSeq_);
        d.setString("op", "evict");
        d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
        d.setString("newRootID", newRoot);
        appendLine(d.dump(-1));
    }
}

void HistoryStore::persistPurge(const std::string& branchRootEventID) {
    // Same ctl:purge record eviction writes, so replay drops the subtree the same
    // way (§4.1 collects op=="purge" into purgedBranchRoots).
    util::JsonDoc d;
    d.setString("rec", "ctl");
    d.setInt64("seq", ++lastSeq_);
    d.setString("op", "purge");
    d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
    d.setString("branchRootEventID", branchRootEventID);
    appendLine(d.dump(-1));
}

void HistoryStore::checkpoint() {
    if (!fs_ || !service_) return;
    util::JsonDoc d;
    d.setString("schema", "scrivi.history.v1");

    util::JsonDoc s;
    s.setInt("capacityEvents", settings_.capacityEvents);
    s.setInt("staleBranchDays", settings_.staleBranchDays);
    s.setInt("idleRolloverHours", settings_.idleRolloverHours);
    d.setSubDoc("settings", std::move(s));

    d.setString("rootID", service_->rootID());
    d.setString("currentNodeID", service_->currentNodeID());
    d.setString("sessionID", service_->sessionID());
    d.setInt64("lastSeq", lastSeq_);
    d.setString("activeLogSegment", activeSegment_);

    // Fork primaries — forks only (nodes with >= 2 children); a single child is
    // implicitly primary and omitted (Appendix A.2). Load accelerator for D4.
    util::JsonDoc primaries;
    bool anyPrimary = false;
    for (const auto& [id, node] : service_->nodes()) {
        if (node.childIDs.size() >= 2 && node.primaryChildID.has_value()) {
            primaries.setString(id, *node.primaryChildID);
            anyPrimary = true;
        }
    }
    if (anyPrimary) d.setSubDoc("primaryChildren", std::move(primaries));

    // Per-scene head hashes for §6.b validation at next open.
    util::JsonDoc heads;
    for (const auto& [sceneID, text] : service_->floorTexts()) {
        util::JsonDoc h;
        h.setString("sha256", util::sha256Hex(service_->headTextForScene(sceneID)));
        heads.setSubDoc(sceneID, std::move(h));
    }
    d.setSubDoc("sceneHeads", std::move(heads));

    fs_->atomicWriteTextFile(statePath(), d.dump(2));
}

bool HistoryStore::validateSceneHead(const std::string& sceneID,
                                     const std::string& currentDiskText,
                                     const std::string& nowTimestamp,
                                     const std::string& barrierEventID) {
    auto it = loadedHeadHashes_.find(sceneID);
    if (it == loadedHeadHashes_.end()) return false;   // no baseline to compare
    const std::string diskHash = util::sha256Hex(currentDiskText);
    if (diskHash == it->second) return false;          // unchanged — all good

    // Mismatch: the scene was edited outside Scrivi. Record an externalChange
    // barrier so undo stops at this point, and re-seed the scene's cached text
    // from disk. The manuscript is never modified by this repair (§6.b).
    if (service_) {
        BarrierParams bp;
        bp.barrierKind = "externalChange";
        bp.barrierNote = "This scene was changed outside Scrivi; undo stops here.";
        bp.timestamp = nowTimestamp;
        auto r = service_->recordBarrier(bp, barrierEventID);
        auto nit = service_->nodes().find(r.eventID);
        if (nit != service_->nodes().end()) { persistEvent(nit->second); }
        // Re-seed this scene's floor from the on-disk text so future diffs and
        // undo restores use the real current text as the new baseline.
        service_->reseedSceneFloor(sceneID, currentDiskText);
        persistFloor(sceneID, currentDiskText, "externalChange");
    }
    loadedHeadHashes_[sceneID] = diskHash;   // don't re-warn for the same edit
    return true;
}

} // namespace scrivi::history
