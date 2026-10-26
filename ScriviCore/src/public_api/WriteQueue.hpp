#pragma once
// EP-052 (SP-194, T-0624) — one write queue per project: docs/Scrivi_Background_Writes_Design_v0_1.md W1–W7, v0.2 §3.
//
// ONE writer thread per project, started by the first job, takes jobs first-in-first-out, so one project's writes land in
// the order they were made. Every job has a TICKET (increasing); `waitFor` blocks until a ticket has landed.
//
// - Saves and structural jobs are functions that call the synchronous endpoint (`scrivi_save_scene`, `scrivi_range_*`),
//   which takes the disk lock itself. Their envelope is the job's outcome (W4; a structural delta inside it, W7-Q1 (a)).
// - History writes are data: the log's appends join the job at the tail when it is a not-yet-started append to the same
//   log, and the writer writes them under the disk lock. An append that fails is kept and written ahead of the next
//   ([I-0268]), as `HistoryStore::appendLine` did.
// - A save queued again before its write starts replaces the earlier one, at the tail (W1); never across a structural
//   job (W7e). The replaced ticket's outcome says `supersededBy`.
//
// ⚠️ Lock order: a job runs with NO lock held by the writer except the disk lock for a history write. The queue's own
// mutex is a leaf: nothing is called while it is held.

#include "scrivi/Services.hpp"
#include "util/Json.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace scrivi::abi {

class WriteQueue {
public:
    explicit WriteQueue(std::mutex& disk) : disk_(disk) {}
    ~WriteQueue();
    WriteQueue(const WriteQueue&) = delete;
    WriteQueue& operator=(const WriteQueue&) = delete;

    // `run` returns the endpoint's parsed envelope ({ok, result} or {ok:false, error}).
    int64_t enqueueSave(const std::string& sceneID, std::function<util::JsonDoc()> run);
    int64_t enqueueStructural(std::function<util::JsonDoc()> run);
    void enqueueHistoryAppend(FileSystem* fs, const AbsolutePath& log, const std::string& lines);
    void enqueueHistoryState(FileSystem* fs, const AbsolutePath& state, const std::string& json);

    int64_t lastTicket();
    // Blocks until every job with a ticket <= `ticket` has landed (ticket <= 0: every job queued so far), or the time runs
    // out. True when landed. A negative timeout waits for as long as it takes.
    bool waitFor(int64_t ticket, std::chrono::milliseconds timeout);
    // Every save and structural outcome, and every failed history write, since the last call.
    std::vector<util::JsonDoc> takeOutcomes();

    static bool onWriterThread();
    // In a forked child: the parent's writer thread does not exist here, and its jobs are the parent's to write.
    void resetInForkedChild();

private:
    enum class Kind { save, structural, historyAppend, historyState };
    struct Job {
        int64_t ticket = 0;
        Kind kind = Kind::save;
        std::string key;   // sceneID for a save, the file for a history write
        std::function<util::JsonDoc()> run;
        FileSystem* fs = nullptr;
        std::string text;
    };

    int64_t push(Job job);   // under mutex_
    void startIfNeeded();    // under mutex_
    void loop();
    util::JsonDoc runHistory(const Job& job);
    bool landedLocked(int64_t ticket) const;

    std::mutex& disk_;
    std::mutex mutex_;
    std::condition_variable changed_;
    std::deque<Job> pending_;
    int64_t nextTicket_ = 1;
    int64_t running_ = 0;   // the ticket being written, 0 when idle
    bool stopping_ = false;
    std::thread thread_;
    std::vector<util::JsonDoc> outcomes_;
    std::map<std::string, std::string> unwritten_;   // writer thread only: log appends that failed, by log path
};

}  // namespace scrivi::abi
