#include "public_api/ProjectLock.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace scrivi::abi {

namespace {

struct ProjectLockRegistry {
    std::mutex mutex;
    std::unordered_map<std::string, std::unique_ptr<ProjectLockEntry>> byRoot;
};

ProjectLockRegistry& registry() {
    static ProjectLockRegistry r;
    return r;
}

std::string lockKey(const char* root) {
    std::string key = std::filesystem::path(root).lexically_normal().string();
    while (key.size() > 1 && key.back() == '/') { key.pop_back(); }
    return key;
}

thread_local const ProjectCallContext* tlsCall = nullptr;

}  // namespace

ProjectLockEntry& projectLockEntry(const char* root) {
    auto& reg = registry();
    std::lock_guard<std::mutex> lock(reg.mutex);
    auto& slot = reg.byRoot[lockKey(root)];
    if (!slot) { slot = std::make_unique<ProjectLockEntry>(); }
    return *slot;   // stable: entries are never erased
}

const ProjectCallContext* currentProjectCall() { return tlsCall; }

ProjectCallGuard::ProjectCallGuard(const char* root, Kind kind) {
    if (root == nullptr || *root == '\0') { return; }
    entry_ = &projectLockEntry(root);
    const auto t0 = std::chrono::steady_clock::now();
    lock_ = std::unique_lock<std::mutex>(entry_->mutex);
    context_.lockWaitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();
    if (kind == Kind::write) { ++entry_->revision; }
    context_.revision = entry_->revision;
    previous_ = tlsCall;
    tlsCall = &context_;
}

ProjectCallGuard::~ProjectCallGuard() {
    if (entry_ != nullptr) { tlsCall = previous_; }
}

}  // namespace scrivi::abi
