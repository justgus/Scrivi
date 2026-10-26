#include "public_api/WriteQueue.hpp"

#include <algorithm>
#include <exception>
#include <iterator>
#include <new>
#include <utility>

namespace scrivi::abi {

namespace {
thread_local bool tlsWriter = false;

const char* kindName(int k) {
    static const char* names[] = {"save", "structural", "history", "history"};
    return names[k];
}
}  // namespace

bool WriteQueue::onWriterThread() { return tlsWriter; }

void WriteQueue::resetInForkedChild() {
    // Only the forking thread survives a fork, so the mutex and condition may be in any state: make them new. The
    // parent's std::thread names a thread that is not here; it is overwritten, never joined.
    new (&mutex_) std::mutex();
    new (&changed_) std::condition_variable();
    new (&thread_) std::thread();
    pending_.clear();
    outcomes_.clear();
    unwritten_.clear();
    running_ = 0;
}

WriteQueue::~WriteQueue() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    changed_.notify_all();
    if (thread_.joinable()) { thread_.join(); }
}

void WriteQueue::startIfNeeded() {
    if (!thread_.joinable()) { thread_ = std::thread([this] { loop(); }); }
}

int64_t WriteQueue::push(Job job) {
    job.ticket = nextTicket_++;
    const int64_t ticket = job.ticket;
    pending_.push_back(std::move(job));
    startIfNeeded();
    return ticket;
}

int64_t WriteQueue::enqueueSave(const std::string& sceneID, std::function<util::JsonDoc()> run) {
    int64_t ticket = 0;
    int64_t superseded = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // W1 coalescing, never across a structural job (W7e).
        for (auto it = pending_.rbegin(); it != pending_.rend(); ++it) {
            if (it->kind == Kind::structural) { break; }
            if (it->kind == Kind::save && it->key == sceneID) {
                superseded = it->ticket;
                pending_.erase(std::next(it).base());
                break;
            }
        }
        ticket = push(Job{0, Kind::save, sceneID, std::move(run), nullptr, {}});
        if (superseded != 0) {
            util::JsonDoc o;
            o.setInt64("ticket", superseded);
            o.setString("kind", "save");
            o.setString("sceneID", sceneID);
            o.setBool("ok", true);
            o.setInt64("supersededBy", ticket);
            outcomes_.push_back(std::move(o));
        }
    }
    changed_.notify_all();
    return ticket;
}

int64_t WriteQueue::enqueueStructural(std::function<util::JsonDoc()> run) {
    int64_t ticket = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ticket = push(Job{0, Kind::structural, {}, std::move(run), nullptr, {}});
    }
    changed_.notify_all();
    return ticket;
}

void WriteQueue::enqueueHistoryAppend(FileSystem* fs, const AbsolutePath& log, const std::string& lines) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pending_.empty() && pending_.back().kind == Kind::historyAppend && pending_.back().key == log) {
            pending_.back().text += lines;   // the tail has not started: one append carries both
        } else {
            push(Job{0, Kind::historyAppend, log, {}, fs, lines});
        }
    }
    changed_.notify_all();
}

void WriteQueue::enqueueHistoryState(FileSystem* fs, const AbsolutePath& state, const std::string& json) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        push(Job{0, Kind::historyState, state, {}, fs, json});
    }
    changed_.notify_all();
}

int64_t WriteQueue::lastTicket() {
    std::lock_guard<std::mutex> lock(mutex_);
    return nextTicket_ - 1;
}

bool WriteQueue::landedLocked(int64_t ticket) const {
    if (running_ != 0 && running_ <= ticket) { return false; }
    return pending_.empty() || pending_.front().ticket > ticket;
}

bool WriteQueue::waitFor(int64_t ticket, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (ticket <= 0) { ticket = nextTicket_ - 1; }
    const auto landed = [&] { return landedLocked(ticket); };
    if (timeout.count() < 0) {
        changed_.wait(lock, landed);
        return true;
    }
    return changed_.wait_for(lock, timeout, landed);
}

std::vector<util::JsonDoc> WriteQueue::takeOutcomes() {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::exchange(outcomes_, {});
}

util::JsonDoc WriteQueue::runHistory(const Job& job) {
    std::lock_guard<std::mutex> disk(disk_);
    util::JsonDoc o;
    // A held-back append goes first, so the log stays in order once the volume is back ([I-0268]); a checkpoint is its
    // last chance, as `HistoryStore::checkpoint` was.
    if (job.kind == Kind::historyState) {
        for (auto& [path, lines] : unwritten_) {
            if (!lines.empty() && job.fs->appendTextFile(path, lines).ok()) { lines.clear(); }
        }
        const auto r = job.fs->atomicWriteTextFile(job.key, job.text);
        o.setBool("ok", r.ok());
        if (!r.ok()) { o.setString("message", r.error().message); }
        return o;
    }
    std::string& held = unwritten_[job.key];
    held += job.text;
    const auto r = job.fs->appendTextFile(job.key, held);
    if (r.ok()) { held.clear(); }
    o.setBool("ok", r.ok());
    if (!r.ok()) { o.setString("message", r.error().message); }
    return o;
}

void WriteQueue::loop() {
    tlsWriter = true;
    std::unique_lock<std::mutex> lock(mutex_);
    for (;;) {
        changed_.wait(lock, [&] { return stopping_ || !pending_.empty(); });
        if (pending_.empty()) { return; }   // stopping, and nothing left
        Job job = std::move(pending_.front());
        pending_.pop_front();
        running_ = job.ticket;
        lock.unlock();

        const bool history = job.kind == Kind::historyAppend || job.kind == Kind::historyState;
        util::JsonDoc outcome;
        try {
            outcome = history ? runHistory(job) : job.run();
        } catch (const std::exception& e) {   // the writer thread must outlive any one job
            outcome = util::JsonDoc{};
            outcome.setBool("ok", false);
            outcome.setString("message", e.what());
        }

        lock.lock();
        running_ = 0;
        if (!history || !outcome.getBool("ok")) {
            outcome.setInt64("ticket", job.ticket);
            outcome.setString("kind", kindName(static_cast<int>(job.kind)));
            if (job.kind == Kind::save) { outcome.setString("sceneID", job.key); }
            if (history) {
                outcome.setString("file", job.key);
                // One failed history write per file is enough to report; an outage must not grow this without bound.
                std::erase_if(outcomes_, [&](const util::JsonDoc& d) {
                    return d.getString("kind") == "history" && d.getString("file") == job.key;
                });
            }
            outcomes_.push_back(std::move(outcome));
        }
        changed_.notify_all();
    }
}

}  // namespace scrivi::abi
