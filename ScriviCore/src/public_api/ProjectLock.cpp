#include "public_api/ProjectLock.hpp"

#include "util/AtomicWrite.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <new>
#include <string>
#include <unordered_map>

#if !defined(_WIN32)
#include <pthread.h>
#endif

namespace scrivi::abi {

namespace {

struct ProjectLockRegistry {
    std::mutex mutex;
    std::unordered_map<std::string, std::unique_ptr<ProjectLockEntry>> byRoot;
};

ProjectLockRegistry& registry();

#if !defined(_WIN32)
// EP-052 (T-0624): a forked child (the [I-0299] tests end a session that way) gets fresh locks and queues with no writer
// thread; the next job starts one. Only the forking thread survives, and it holds none of these locks.
void resetAfterForkInChild() {
    auto& reg = registry();
    new (&reg.mutex) std::mutex();
    for (auto& [root, entry] : reg.byRoot) {
        new (&entry->disk) std::mutex();
        new (&entry->memory) std::mutex();
        entry->queue.resetInForkedChild();
    }
}
#endif

ProjectLockRegistry& registry() {
    static ProjectLockRegistry r = [] {
#if !defined(_WIN32)
        ::pthread_atfork(nullptr, nullptr, resetAfterForkInChild);
#endif
        return ProjectLockRegistry{};
    }();
    return r;
}

std::string lockKey(const char* root) {
    std::string key = std::filesystem::path(root).lexically_normal().string();
    while (key.size() > 1 && key.back() == '/') { key.pop_back(); }
    return key;
}

thread_local const ProjectCallContext* tlsCall = nullptr;

std::atomic<int64_t> violations{0};
std::atomic<bool> violationsFatal{false};

}  // namespace

ProjectLockEntry& projectLockEntry(const char* root) {
    auto& reg = registry();
    std::lock_guard<std::mutex> lock(reg.mutex);
    auto& slot = reg.byRoot[lockKey(root)];
    if (!slot) { slot = std::make_unique<ProjectLockEntry>(); }
    return *slot;   // stable: entries are never erased
}

const ProjectCallContext* currentProjectCall() { return tlsCall; }

ProjectCallGuard::ProjectCallGuard(const char* root, Kind kind, const char* endpoint) {
    if (root == nullptr || *root == '\0') { return; }
    entry_ = &projectLockEntry(root);
    memoryKind_ = kind == Kind::memoryRead || kind == Kind::memoryWrite;
    // The writer thread lands what an earlier call already counted: the revision moves when memory changes (v0.2 §3).
    const bool bumps = (kind == Kind::write || kind == Kind::memoryWrite) && !WriteQueue::onWriterThread();
    const auto t0 = std::chrono::steady_clock::now();
    if (kind == Kind::read || kind == Kind::write) {
        if (tlsCall == nullptr && !WriteQueue::onWriterThread() &&   // W3
            !entry_->queue.waitFor(0, std::chrono::milliseconds(-1))) {
            refused_ = true;
            entry_ = nullptr;
            return;
        }
        lock_ = std::unique_lock<std::mutex>(entry_->disk);
    } else if (memoryKind_) {
        lock_ = std::unique_lock<std::mutex>(entry_->memory);
    }
    context_.lockWaitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();
    if (memoryKind_) {
        if (bumps) { ++entry_->revision; }
        context_.revision = entry_->revision;
    } else {
        std::lock_guard<std::mutex> m(entry_->memory);   // D before M (or no lock at all)
        if (bumps) { ++entry_->revision; }
        context_.revision = entry_->revision;
    }
    context_.endpoint = endpoint;
    context_.project = root != nullptr ? root : "";
    context_.started = std::chrono::steady_clock::now();
    context_.fileOpsAtStart = scrivi::util::fileOperationsOnThisThread();
    previous_ = tlsCall;
    tlsCall = &context_;
}

ProjectCallGuard::~ProjectCallGuard() {
    if (entry_ == nullptr) { return; }
    tlsCall = previous_;
    if (memoryKind_ && scrivi::util::fileOperationsOnThisThread() != context_.fileOpsAtStart) {
        violations.fetch_add(1);
        std::fprintf(stderr, "[SCRIVI-LOCK] %s is a memory call but made a file operation\n", context_.endpoint);
        if (violationsFatal.load()) { std::abort(); }
    }
}

int64_t memoryCallFileOpViolations() { return violations.load(); }
void setMemoryCallViolationsFatal(bool fatal) { violationsFatal.store(fatal); }

}  // namespace scrivi::abi
