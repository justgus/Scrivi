#include "history/HistoryStore.hpp"

#include "util/Json.hpp"
#include "util/PathUtils.hpp"
#include "util/Hash.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
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
        case EventKind::Cut:        return "cut";
        case EventKind::Barrier:    return "barrier";
        case EventKind::Structural: return "structural";
    }
    return "typing";
}

EventKind kindFromStr(const std::string& s) {
    if (s == "delete")     return EventKind::Delete;
    if (s == "replace")    return EventKind::Replace;
    if (s == "paste")      return EventKind::Paste;
    if (s == "cut")        return EventKind::Cut;
    if (s == "barrier")    return EventKind::Barrier;
    if (s == "structural") return EventKind::Structural;
    return EventKind::Typing;
}

// A diff as JSON: one change in the v0.1 shape, several as `hunks` (T-0643).
util::JsonDoc diffToJson(const Diff& d) {
    auto hunk = [](std::size_t offset, const std::string& removed, const std::string& inserted) {
        util::JsonDoc h;
        h.setInt64("offsetUtf8", static_cast<std::int64_t>(offset));
        h.setString("removed", removed);
        h.setString("inserted", inserted);
        return h;
    };
    if (d.more.empty()) return hunk(d.offsetUtf8, d.removed, d.inserted);
    util::JsonDoc out;
    out.appendToArray("hunks", hunk(d.offsetUtf8, d.removed, d.inserted));
    for (const auto& h : d.more) out.appendToArray("hunks", hunk(h.offsetUtf8, h.removed, h.inserted));
    return out;
}

Diff diffFromJson(const util::JsonDoc& j) {
    Diff d;
    if (const std::size_t n = j.arraySize("hunks"); n > 0) {
        for (std::size_t i = 0; i < n; ++i) {
            const util::JsonDoc h = j.arrayItem("hunks", i);
            Diff::Hunk c{static_cast<std::size_t>(h.getInt64("offsetUtf8")), h.getString("removed"), h.getString("inserted")};
            if (i == 0) {
                d.offsetUtf8 = c.offsetUtf8;
                d.removed = std::move(c.removed);
                d.inserted = std::move(c.inserted);
            } else {
                d.more.push_back(std::move(c));
            }
        }
        return d;
    }
    d.offsetUtf8 = static_cast<std::size_t>(j.getInt64("offsetUtf8"));
    d.removed  = j.getString("removed");
    d.inserted = j.getString("inserted");
    return d;
}

// T-0644 (b2): a barrier's texts — each scene as a change where the node has one, else whole (v0.1's shape).
void writeBarrierTexts(const EventNode& n, util::JsonDoc& b) {
    util::JsonDoc texts, diffs, before, beforeDiffs;
    bool anyText = false, anyDiff = false, anyBefore = false, anyBeforeDiff = false;
    for (const auto& [sceneID, text] : n.sceneTexts) {
        if (auto d = n.sceneTextDiffs.find(sceneID); d != n.sceneTextDiffs.end()) {
            diffs.setSubDoc(sceneID, diffToJson(d->second)); anyDiff = true;
        } else {
            texts.setString(sceneID, text); anyText = true;
        }
    }
    std::vector<std::string> same;
    for (const auto& [sceneID, text] : n.sceneTextsBefore) {
        if (n.sceneTextsBeforeSame.contains(sceneID)) {
            same.push_back(sceneID);
        } else if (auto d = n.sceneTextsBeforeDiffs.find(sceneID); d != n.sceneTextsBeforeDiffs.end()) {
            beforeDiffs.setSubDoc(sceneID, diffToJson(d->second)); anyBeforeDiff = true;
        } else {
            before.setString(sceneID, text); anyBefore = true;
        }
    }
    if (anyText) b.setSubDoc("sceneTexts", std::move(texts));
    if (anyDiff) b.setSubDoc("sceneTextDiffs", std::move(diffs));
    if (anyBefore) b.setSubDoc("sceneTextsBefore", std::move(before));
    if (anyBeforeDiff) b.setSubDoc("sceneTextsBeforeDiffs", std::move(beforeDiffs));
    for (const auto& sid : same) b.appendStringToArray("sceneTextsBeforeSame", sid);
}

void readBarrierTexts(const util::JsonDoc& b, EventNode& node) {
    const util::JsonDoc texts = b.getSubDoc("sceneTexts");   // [I-0287]; absent in older logs
    for (const std::string& sid : texts.objectKeys()) node.sceneTexts[sid] = texts.getString(sid);
    const util::JsonDoc prior = b.getSubDoc("sceneTextsBefore");   // [I-0293]
    for (const std::string& sid : prior.objectKeys()) node.sceneTextsBefore[sid] = prior.getString(sid);
    const util::JsonDoc diffs = b.getSubDoc("sceneTextDiffs");   // T-0644: expanded in finalizeLoad
    for (const std::string& sid : diffs.objectKeys()) node.sceneTextDiffs[sid] = diffFromJson(diffs.getSubDoc(sid));
    const util::JsonDoc priorDiffs = b.getSubDoc("sceneTextsBeforeDiffs");
    for (const std::string& sid : priorDiffs.objectKeys()) {
        node.sceneTextsBeforeDiffs[sid] = diffFromJson(priorDiffs.getSubDoc(sid));
    }
    for (const auto& sid : b.getStringArray("sceneTextsBeforeSame")) node.sceneTextsBeforeSame.insert(sid);
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
    ++segmentRecords_;   // T-0400
    segmentBytes_ += static_cast<std::int64_t>(jsonLine.size()) + 1;   // T-0647
    if (writer_) {
        writer_(logPath(), jsonLine + '\n', true);
        if (++recordsSinceCheckpoint_ >= 200) {
            checkpoint();
            recordsSinceCheckpoint_ = 0;
        }
        return;
    }
    // I-0268: a failed append used to be DISCARDED while the node stayed in memory, so
    // the next record to land named a parent the log never got — and every later open
    // failed. Reported when the writer's USB drive dropped off the bus for ~70s with
    // the project open. Keep what did not land and write it ahead of the next record,
    // so the log stays in order and complete once the volume is back.
    unwrittenLines_ += jsonLine;
    unwrittenLines_ += '\n';
    if (fs_->appendTextFile(logPath(), unwrittenLines_).ok()) {
        unwrittenLines_.clear();
    }
    if (++recordsSinceCheckpoint_ >= 200) {
        checkpoint();
        recordsSinceCheckpoint_ = 0;
    }
}

bool HistoryStore::openOrCreate(const std::string& newSessionID,
                                const std::string& nowTimestamp, bool& outLoaded) {
    outLoaded = false;
    if (fs_) { fs_->createDirectories(historyDir_); }
    resolveActiveSegment();   // T-0400: the segment state.json names, not always log-000001

    // Try to load an existing log by replaying its records.
    bool haveLog = false;
    std::string logText;
    if (fs_) {
        auto existsR = fs_->exists(logPath());
        if (existsR.ok() && existsR.value()) {
            auto readR = fs_->readTextFile(logPath());
            if (readR.ok()) { logText = readR.value(); haveLog = true; segmentBytes_ = static_cast<std::int64_t>(logText.size()); }
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
        // Every node the replay pointed at, in log order — finalizeLoad()'s fallback
        // when the final pointer turns out to be an orphan (I-0268).
        std::vector<std::string> currentTrail;
        // Fork re-selections (ctl:setPrimary) collected during replay; applied
        // after finalizeLoad() derives childIDs (D4/SP-055). forkNodeID → childID.
        std::map<std::string, std::string> primaryOverrides;
        // [I-0288]: the externalChange barrier the PREVIOUS record wrote, if it was one — an old log's repair floor
        // record follows it directly.
        std::string lastExternalBarrierID;
        // Branch-aware eviction (§4.1) collected during replay; applied after
        // finalizeLoad() so evicted branches don't resurrect. Purges first, then
        // promotions in log order (mirrors persistEviction()).
        std::vector<std::string> purgedBranchRoots;
        std::vector<std::string> promotedRoots;
        // [I-0299]: the bytes each save wrote, as the log recorded them — newest wins. Applied after state.json, which a
        // session that ended without closing never got to update.
        std::map<std::string, std::string> replayedPersisted;
        // [I-0299]: did the last session in the log close? Only a session whose record says it records its close can be
        // judged — a log from an older build never wrote one, and must not be read as a crash.
        bool lastSessionRecordsClose = false;
        bool lastSessionClosed = false;

        // T-0400: how many records the segment holds, and how many its compaction wrote (`ctl:compacted`).
        std::int64_t totalRecords = 0;
        std::int64_t compactedRecords = 0;

        std::istringstream ss(logText);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            auto parsedR = util::parseJson(line);
            if (!parsedR.ok()) {
                // Torn final line (partial write) — stop; everything before is intact.
                break;
            }
            ++totalRecords;
            const util::JsonDoc& d = parsedR.value();
            const std::string rec = d.getString("rec");
            const std::int64_t seq = d.getInt64("seq");
            if (seq > maxSeq) maxSeq = seq;
            sawAnyRecord = true;

            if (rec == "persisted") {
                replayedPersisted[d.getString("sceneID")] = d.getString("sha256");
                continue;
            }
            if (rec == "floor") {
                // ⛔ [I-0288] — a log written before the fix: the externalChange repair wrote its barrier, then a floor
                // record for the same text. Replayed as a FLOOR it puts every earlier diff on top of the new text. It
                // belongs to that barrier, so attach it there — the shape the repair writes now.
                if (d.getString("reason") == "externalChange" && !lastExternalBarrierID.empty()) {
                    svc->attachLoadedBarrierText(lastExternalBarrierID, d.getString("sceneID"), d.getString("text"));
                } else {
                    svc->addLoadedFloor(d.getString("sceneID"), d.getString("text"));
                }
                lastExternalBarrierID.clear();
                continue;
            }
            lastExternalBarrierID.clear();
            if (rec == "event") {
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
                    readBarrierTexts(b, node);
                    if (node.barrierKind == "externalChange") lastExternalBarrierID = node.eventID;   // [I-0288]
                } else if (node.kind == EventKind::Structural) {
                    // Reversible structural node (T-0356): the barrier-style label plus the
                    // opaque inverse-op payload the app replays. No diff.
                    util::JsonDoc b = d.getSubDoc("barrier");
                    node.barrierKind = b.getString("barrierKind");
                    node.barrierNote = b.getString("note");
                    node.structuralPayload = d.getString("structuralPayload");   // a log before T-0645
                    if (node.structuralPayload.empty()) {
                        if (const auto payload = d.getSubDoc("structuralPayload"); !payload.objectKeys().empty()) {
                            node.structuralPayload = payload.dump(0);   // the core's own form (`scrivi_c_api.cpp`)
                        }
                    }
                    readBarrierTexts(b, node);   // [I-0293]; absent in older logs
                } else {
                    util::JsonDoc diff = d.getSubDoc("diff");
                    node.diff = diffFromJson(diff);   // one change, or several (T-0643)
                    node.bufferID = d.getString("bufferID");  // cut-into-buffer tag; "" when absent
                    node.groupID = d.getString("groupID");    // edit group (I-0270); "" when absent
                }
                // Recording a node advances the current pointer to it; in seq
                // order a later ctl:undo/redo may move it back. Last write wins.
                currentNodeID = node.eventID;
                currentTrail.push_back(currentNodeID);
                svc->addLoadedNode(std::move(node));
            } else if (rec == "ctl") {
                const std::string op = d.getString("op");
                if (op == "undo" || op == "redo") {
                    currentNodeID = d.getString("nodeID");
                    currentTrail.push_back(currentNodeID);
                } else if (op == "session") {
                    loadedSessionID = d.getString("sessionID");
                    // [I-0299]: a session that RECORDS its close, and has not (yet) — the next record may be its close.
                    lastSessionRecordsClose = d.getBool("closesRecorded");
                    lastSessionClosed = false;
                } else if (op == "close") {
                    lastSessionClosed = true;
                } else if (op == "setPrimary") {
                    // Collect fork re-selections; applied after finalizeLoad()
                    // (childIDs must exist first). Last write per fork wins.
                    primaryOverrides[d.getString("forkNodeID")] = d.getString("childEventID");
                } else if (op == "purge") {
                    // Branch-aware eviction (§4.1): a non-primary subtree dropped.
                    // Applied after finalizeLoad() so the subtree doesn't resurrect.
                    purgedBranchRoots.push_back(d.getString("branchRootEventID"));
                } else if (op == "root") {
                    // T-0400: a compacted segment names its root — an event whose diff the floors already hold.
                    rootID = d.getString("nodeID");
                } else if (op == "compacted") {
                    compactedRecords = d.getInt64("records");
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
            svc->finalizeLoad(currentTrail);
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
            // I-0111: adopt the replayed high-water mark BEFORE writing anything.
            // persistPurge() below stamps `++lastSeq_`, and lastSeq_ was still 0 here
            // (it used to be assigned ~34 lines further down, after the state.json
            // read), so every prune-driven purge was written with **seq 1** — 13 of
            // them in the writer's log, causing 11 sequence regressions. The records
            // replay correctly (order comes from file position, not seq), but the
            // sequence numbers were meaningless and made log forensics misleading.
            lastSeq_ = maxSeq;

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
            // [I-0299]: the log's record of what was last written wins over state.json — it is at least as new, and a
            // session that ended without closing never updated state.json. Same scenes as state.json covers (a floor).
            for (const auto& [sceneID, sha] : replayedPersisted) {
                if (svc->floorTexts().contains(sceneID)) loadedHeadHashes_[sceneID] = sha;
            }
            previousSessionEndedUncleanly_ = lastSessionRecordsClose && !lastSessionClosed;
            // Mint a NEW session for this open (Trade T5) — the loaded sessionID
            // is what distinguishes "previous session" nodes for the warning.
            svc->setSessionID(newSessionID);
            service_ = std::move(svc);
            // lastSeq_ was already adopted above, BEFORE the prune could write purge
            // records (I-0111). Any records written since have advanced it further, so
            // re-assigning maxSeq here would roll it backwards and reissue numbers.
            lastSeq_ = std::max(lastSeq_, maxSeq);
            outLoaded = true;
            segmentRecords_   = totalRecords;
            compactedRecords_ = compactedRecords;
            // T-0400: before this session's first record, so the compacted segment is followed by it.
            compactIfWorthIt(loadedSessionID);
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

std::string HistoryStore::eventLine(const EventNode& node, std::int64_t seq) const {
    util::JsonDoc d;
    d.setString("rec", "event");
    d.setInt64("seq", seq);
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
        // [I-0287] — written only when the op rewrote scene text, so other barriers keep their on-disk shape.
        writeBarrierTexts(node, b);   // T-0644: as changes where it can
        d.setSubDoc("barrier", std::move(b));
    } else if (node.kind == EventKind::Structural) {
        // Reversible structural node (T-0356): the barrier-style label plus the opaque
        // inverse-op payload the app replays on undo/redo. No diff.
        util::JsonDoc b;
        b.setString("barrierKind", node.barrierKind);
        b.setString("note", node.barrierNote);
        // [I-0293] — written only when present, so a node without them keeps its on-disk shape.
        writeBarrierTexts(node, b);   // T-0644: as changes where it can
        d.setSubDoc("barrier", std::move(b));
        // T-0645 (b3): as JSON, not a string the log escapes again (38% of dumas's log). Read as either, forever.
        if (auto payload = util::parseJson(node.structuralPayload); payload.ok() && payload.value().isObject()) {
            d.setSubDoc("structuralPayload", std::move(payload.value()));
        } else {
            d.setString("structuralPayload", node.structuralPayload);
        }
    } else {
        util::JsonDoc diff = diffToJson(node.diff);   // one change in the v0.1 shape, several as `hunks` (T-0643)
        d.setSubDoc("diff", std::move(diff));
        // Cut-into-buffer provenance (Trade T3) — written only when tagged, so
        // ordinary events keep their existing on-disk shape.
        if (!node.bufferID.empty()) {
            d.setString("bufferID", node.bufferID);
        }
        // Edit group (I-0270) — likewise written only when set.
        if (!node.groupID.empty()) {
            d.setString("groupID", node.groupID);
        }
    }
    return d.dump(-1);
}

void HistoryStore::persistEvent(const EventNode& node) {
    appendLine(eventLine(node, ++lastSeq_));
}

void HistoryStore::persistPersisted(const std::string& sceneID, const std::string& sha256) {
    util::JsonDoc d;
    d.setString("rec", "persisted");
    d.setInt64("seq", ++lastSeq_);
    d.setString("sceneID", sceneID);
    d.setString("sha256", sha256);
    d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
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
    if (op == "session") d.setBool("closesRecorded", true);   // [I-0299]: this session will write `close` when it closes
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
    // Last chance for records an outage held back (I-0268) — checkpoint runs at close.
    if (!writer_ && !unwrittenLines_.empty() && fs_->appendTextFile(logPath(), unwrittenLines_).ok()) {
        unwrittenLines_.clear();
    }
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
    //
    // I-0104: this must be the hash of the bytes ON DISK, because that is what
    // validateSceneHead() compares against at the next open. Prefer the hash the
    // save path recorded via noteScenePersisted(); only fall back to the replayed
    // head for a scene we never saw saved this session. Hashing the head here
    // unconditionally (the old behaviour) compared head-to-disk and so fired an
    // externalChange barrier on essentially every relaunch.
    // ⚠️ ORDER MATTERS, and getting it wrong caused a permanent barrier loop.
    //
    // Reported 2026-08-10: the same 14 scenes re-flagged on EVERY relaunch, whether or
    // not the writer touched them. Forensics on the real project were unambiguous —
    // `validateSceneHead`'s repair was working (the newest `floor` record in the log
    // matched the file byte-for-byte), but `state.json` held the hash of a *superseded*
    // floor (13 of 14 scenes). So each open detected the same stale baseline again,
    // warned again, repaired again, and then persisted the stale value again.
    //
    // The culprit was preferring `loadedHeadHashes_` here. That map is seeded at open
    // and reflects what state.json said THEN; a repair that re-seeds the floor
    // mid-session does not necessarily get written back before `checkpoint()` runs
    // (it also fires every 200 records, not only at close).
    //
    // The floor text is the authority: it is the exact text the repair adopted from
    // disk, and for an untouched scene it is what was loaded from disk. Hash THAT.
    // `persistedDiskHashes_` still wins when present, because a save this session knows
    // the written bytes precisely — including edits made after the floor was seeded.
    util::JsonDoc heads;
    for (const auto& [sceneID, text] : service_->floorTexts()) {
        util::JsonDoc h;
        auto dit = persistedDiskHashes_.find(sceneID);
        if (dit != persistedDiskHashes_.end()) {
            h.setString("sha256", dit->second);
        } else {
            auto lit = loadedHeadHashes_.find(sceneID);
            h.setString("sha256", lit != loadedHeadHashes_.end()
                                      ? lit->second
                                      : util::sha256Hex(service_->headTextForScene(sceneID)));
        }
        heads.setSubDoc(sceneID, std::move(h));
    }
    d.setSubDoc("sceneHeads", std::move(heads));

    if (writer_) {
        writer_(statePath(), d.dump(2), false);
    } else {
        fs_->atomicWriteTextFile(statePath(), d.dump(2));
    }
}

// ---------------------------------------------------------------------------
// T-0400 — compaction (design `Scrivi_History_Load_Design_v0_1.md` §2; the history design's §6.b, never built before).
// ---------------------------------------------------------------------------

void HistoryStore::resolveActiveSegment() {
    if (!fs_) return;
    std::string named;
    if (auto st = fs_->readTextFile(statePath()); st.ok()) {
        if (auto parsed = util::parseJson(st.value()); parsed.ok()) named = parsed.value().getString("activeLogSegment");
    }
    auto listed = fs_->listDirectory(historyDir_);
    if (!listed.ok()) return;
    std::vector<std::string> segments;
    for (const auto& path : listed.value()) {
        const std::string name = util::filename(path);
        if (name.starts_with("log-") && name.ends_with(".jsonl")) segments.push_back(name);
    }
    if (segments.empty()) return;   // a fresh history: log-000001
    std::sort(segments.begin(), segments.end());
    // ⚠️ state.json moves to a new segment only AFTER that segment is complete (§2.2), so the one it names is whole; with
    // no readable state.json, the newest segment is.
    activeSegment_ = std::find(segments.begin(), segments.end(), named) != segments.end() ? named : segments.back();
    // Any other segment is one a compaction replaced, or one written before a crash moved state.json to it.
    for (const auto& name : segments) {
        if (name != activeSegment_) (void)fs_->removeFile(util::join(historyDir_, name));
    }
}

bool HistoryStore::compactIfWorthIt(const std::string& sessionID) {
    if (!fs_ || !service_) return false;
    std::int64_t forks = 0;
    for (const auto& [id, node] : service_->nodes()) { if (node.childIDs.size() >= 2) ++forks; }
    // What a compaction would write (§2.1): the live nodes, floors, fork primaries, hashes and a few control records.
    const std::int64_t live = static_cast<std::int64_t>(service_->nodes().size() + service_->floorTexts().size())
                            + forks + static_cast<std::int64_t>(loadedHeadHashes_.size()) + 6;
    // Q1 (ruled 2026-11-01). ⚠️ "10,000 in all" is counted since the LAST compaction: a live tree of 20,000 events always
    // holds more than 10,000 records, and recompacting it at every open and close would rewrite it for nothing.
    if (segmentRecords_ - live <= 2000 && segmentRecords_ - compactedRecords_ <= 10000) return false;
    compact(sessionID);
    return true;
}

void HistoryStore::compact(const std::string& sessionID) {
    if (!fs_ || !service_) return;
    const HistoryService& svc = *service_;
    std::string text;
    std::int64_t records = 0;
    auto add = [&](const util::JsonDoc& d) { text += d.dump(-1); text += '\n'; ++records; };
    auto ctl = [&](const std::string& op) {
        util::JsonDoc d;
        d.setString("rec", "ctl");
        d.setInt64("seq", ++lastSeq_);
        d.setString("op", op);
        d.setString("timestamp", clock_ ? clock_->nowUTC() : std::string{});
        return d;
    };

    // 1. The last session, closed: replay then judges the NEXT session by its own records ([I-0299]).
    { auto d = ctl("session"); d.setString("sessionID", sessionID); d.setBool("closesRecorded", true); add(d); }
    add(ctl("close"));
    // 2. The floors.
    for (const auto& [sceneID, floor] : svc.floorTexts()) {
        util::JsonDoc d;
        d.setString("rec", "floor");
        d.setInt64("seq", ++lastSeq_);
        d.setString("sceneID", sceneID);
        d.setString("reason", "compacted");
        d.setString("text", floor);
        d.setString("sha256", util::sha256Hex(floor));
        add(d);
    }
    // 3. The live nodes, parents before children. A root promoted by eviction is written as an event with no parent and
    //    named by `ctl:root`; its diff is already in the floors, and replay never applies the root's.
    const auto& nodes = svc.nodes();
    if (svc.rootID() != "evt_root") {
        text += eventLine(nodes.at(svc.rootID()), ++lastSeq_);
        text += '\n';
        ++records;
        auto d = ctl("root");
        d.setString("nodeID", svc.rootID());
        add(d);
    }
    std::vector<std::string> stack(nodes.at(svc.rootID()).childIDs.rbegin(), nodes.at(svc.rootID()).childIDs.rend());
    while (!stack.empty()) {
        const EventNode& n = nodes.at(stack.back());
        stack.pop_back();
        text += eventLine(n, ++lastSeq_);
        text += '\n';
        ++records;
        stack.insert(stack.end(), n.childIDs.rbegin(), n.childIDs.rend());
    }
    // 4. Fork primaries; 5. the current node.
    for (const auto& [id, node] : nodes) {
        if (node.childIDs.size() < 2 || !node.primaryChildID.has_value()) continue;
        auto d = ctl("setPrimary");
        d.setString("forkNodeID", id);
        d.setString("childEventID", *node.primaryChildID);
        add(d);
    }
    { auto d = ctl("undo"); d.setString("nodeID", svc.currentNodeID()); add(d); }
    // 6. The bytes each scene last had on disk ([I-0299]); a save this session wins.
    std::map<std::string, std::string> hashes = loadedHeadHashes_;
    for (const auto& [sceneID, sha] : persistedDiskHashes_) hashes[sceneID] = sha;
    for (const auto& [sceneID, sha] : hashes) {
        util::JsonDoc d;
        d.setString("rec", "persisted");
        d.setInt64("seq", ++lastSeq_);
        d.setString("sceneID", sceneID);
        d.setString("sha256", sha);
        add(d);
    }
    // 7. Forensic; replay reads only `records`, to know what has been written since.
    {
        auto d = ctl("compacted");
        d.setString("fromSegment", activeSegment_);
        d.setInt64("liveNodes", static_cast<std::int64_t>(nodes.size()));
        d.setInt64("droppedRecords", std::max<std::int64_t>(0, segmentRecords_ - records));
        d.setInt64("records", records + 1);
        add(d);
    }

    // §2.2: the new segment whole (temp + rename), THEN state.json moved to it; the old one is removed at the next open.
    const int n = std::atoi(activeSegment_.substr(4, 6).c_str());
    char name[32];
    std::snprintf(name, sizeof name, "log-%06d.jsonl", n + 1);
    const AbsolutePath next = util::join(historyDir_, name);
    if (writer_) {
        writer_(next, text, false);
    } else if (!fs_->atomicWriteTextFile(next, text).ok()) {
        return;   // the old segment is still whole and still named: nothing lost
    }
    const std::string old = activeSegment_;
    activeSegment_ = name;
    unwrittenLines_.clear();   // held back for the old segment; the compacted one holds them
    segmentRecords_ = records;
    compactedRecords_ = records;
    segmentBytes_ = static_cast<std::int64_t>(text.size());
    checkpoint();
    if (!writer_) (void)fs_->removeFile(util::join(historyDir_, old));
}

void HistoryStore::noteScenePersisted(const std::string& sceneID, const std::string& diskText) {
    if (sceneID.empty()) return;
    const std::string h = util::sha256Hex(diskText);
    // ✅ [I-0299]: in the LOG as well, as it happens — state.json is written only at a checkpoint, and a session that ends
    // without closing (a force-quit, Xcode's Stop, a crash) lost every hash it held. Not repeated for unchanged bytes.
    if (auto it = persistedDiskHashes_.find(sceneID); it == persistedDiskHashes_.end() || it->second != h) {
        persistPersisted(sceneID, h);
    }
    persistedDiskHashes_[sceneID] = h;
    // Keep the in-session baseline in step too: a later validateSceneHead() in
    // this same session (e.g. a reopen without quitting) must not treat text we
    // just wrote ourselves as an external edit.
    loadedHeadHashes_[sceneID] = h;
}

bool HistoryStore::validateSceneHead(const std::string& sceneID,
                                     const std::string& currentDiskText,
                                     const std::string& nowTimestamp,
                                     const std::string& barrierEventID) {
    auto it = loadedHeadHashes_.find(sceneID);
    if (it == loadedHeadHashes_.end()) return false;   // no baseline to compare
    const std::string diskHash = util::sha256Hex(currentDiskText);
    if (diskHash == it->second) {
        // ✅ [I-0299]: the file is what Scrivi itself last wrote. After a session that ended WITHOUT closing, history's
        // copy can still be older — typing the autosave wrote but that had not yet become a step. Record that difference
        // as an ordinary step, so the next edit diffs against the real text (⛔ undoing it used to remove the pre-crash
        // typing too) and the writer can undo it. Never after a clean close, which commits all pending typing.
        if (previousSessionEndedUncleanly_ && service_ && service_->floorTexts().contains(sceneID)
            && util::sha256Hex(service_->headTextForScene(sceneID)) != diskHash) {
            RecordParams p;
            p.sceneID = sceneID;
            p.newSceneText = currentDiskText;
            p.timestamp = nowTimestamp;
            const auto r = service_->record(p, barrierEventID);
            if (auto nit = service_->nodes().find(r.eventID); nit != service_->nodes().end()) { persistEvent(nit->second); }
        }
        return false;                                  // unchanged — all good
    }

    // Mismatch: the scene was edited outside Scrivi. Record an externalChange
    // barrier so undo stops at this point. The manuscript is never modified by
    // this repair (§6.b).
    // ⛔ [I-0288]: the disk text goes ON THE BARRIER (sceneTexts, [I-0287]), so the scene reads it from this node on.
    // It used to RE-FLOOR the scene and persist a `floor` record — and the next open replayed every diff since the root
    // onto that new floor, including the ones computed against the OLD text: undo restored a mix of the two.
    if (service_) {
        BarrierParams bp;
        bp.barrierKind = "externalChange";
        bp.barrierNote = "This scene was changed outside Scrivi; undo stops here.";
        bp.sceneID = sceneID;   // attribute the barrier so the history card can filter (I-0102)
        bp.timestamp = nowTimestamp;
        bp.sceneTexts[sceneID] = currentDiskText;
        auto r = service_->recordBarrier(bp, barrierEventID);
        auto nit = service_->nodes().find(r.eventID);
        if (nit != service_->nodes().end()) { persistEvent(nit->second); }
    }
    loadedHeadHashes_[sceneID] = diskHash;   // don't re-warn for the same edit
    persistPersisted(sceneID, diskHash);     // [I-0299]: and survive a session that ends without closing
    return true;
}

} // namespace scrivi::history
