#pragma once

// HistoryStore — durable persistence for the undo/redo history (EP-019, SP-054).
//
// Owns the on-disk `history/` store for one project and keeps a HistoryService
// in sync with it:
//   history/log-000001.jsonl   append-only records: rec:"floor" | "event" | "ctl"
//   history/state.json         atomic checkpoint (load accelerator, §6.a/A.2)
//
// The log is the source of truth (Appendix A.1). On load the store reads the
// log, truncates a torn final line, and replays records to rebuild the in-memory
// tree. state.json is written as an accelerator + to carry pointers/session; on
// any inconsistency the log wins.
//
// Design: docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md §6, §5,
// Appendix A. Head-hash validation (§6.b) is performed by the ABI layer, which
// owns the scene files; the store exposes the pieces it needs.

#include "history/HistoryService.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace scrivi::history {

// Settings persisted alongside the history (mirrors project.json historySettings,
// Trade T1). project.json is canonical; state.json mirrors for self-containment.
struct HistorySettings {
    int capacityEvents   = 20000;
    int staleBranchDays  = 7;
    int idleRolloverHours = 8;
};

class HistoryStore {
public:
    // `historyDir` is <projectRoot>/history. `fs`/`clock` are borrowed (owned by
    // the ABI singleton). Does not touch disk until open() or a record call.
    HistoryStore(AbsolutePath historyDir, FileSystem* fs, Clock* clock);

    // Loads an existing history from disk (replaying the log) or creates a fresh
    // one. On return, `service()` is a valid tree. `newSessionID` is minted for
    // this open. Returns false only on unrecoverable I/O (caller then runs
    // in-memory only). `outLoaded` is set true if an existing history was read.
    bool openOrCreate(const std::string& newSessionID,
                      const std::string& nowTimestamp, bool& outLoaded);

    HistoryService& service() { return *service_; }

    // Persist operations — call AFTER the corresponding HistoryService mutation
    // so the on-disk log mirrors the in-memory tree.
    void persistFloor(const std::string& sceneID, const std::string& text,
                      const std::string& reason);
    void persistEvent(const EventNode& node);
    void persistCtl(const std::string& op, const std::string& nodeID);

    // Writes the state.json checkpoint (pointers, session, settings, seq, heads).
    void checkpoint();

    // Head-hash validation (§6.b). Compares `currentDiskText` for `sceneID`
    // against the head text hash persisted at the last close. On mismatch (the
    // scene was edited outside Scrivi between sessions) records an
    // `externalChange` barrier and re-seeds that scene's cached text from disk —
    // history is never silently wrong and the manuscript is never modified.
    // Returns true if a mismatch was detected (a barrier was recorded).
    bool validateSceneHead(const std::string& sceneID, const std::string& currentDiskText,
                           const std::string& nowTimestamp, const std::string& barrierEventID);

    const HistorySettings& settings() const { return settings_; }
    void setSettings(const HistorySettings& s) { settings_ = s; }

    std::int64_t lastSeq() const { return lastSeq_; }

private:
    AbsolutePath logPath() const;
    AbsolutePath statePath() const;
    void appendLine(const std::string& jsonLine);

    AbsolutePath historyDir_;
    FileSystem*  fs_    = nullptr;
    Clock*       clock_ = nullptr;

    std::unique_ptr<HistoryService> service_;
    HistorySettings settings_;
    std::int64_t lastSeq_ = 0;
    std::string  activeSegment_ = "log-000001.jsonl";
    int recordsSinceCheckpoint_ = 0;

    // sceneID → head-text sha256 persisted at last close (state.json sceneHeads).
    // Loaded at open for §6.b validation; empty for a fresh history.
    std::map<std::string, std::string> loadedHeadHashes_;
};

} // namespace scrivi::history
