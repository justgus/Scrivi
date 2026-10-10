#pragma once
// SP-173 / I-0285 — D1 + D2 of docs/Scrivi_Core_Concurrency_Design_v0_1.md: one EXCLUSIVE lock and one REVISION per open
// project, taken by every C ABI endpoint that takes a project. Internal to the C ABI layer (never in a public header); its
// own file so tests can hold a project's lock and prove a real `scrivi_*` call waits for it.
//
// ✅ Q1 (user, 2026-10-10): EXCLUSIVE — every call on a project waits for every other, so no call sees another part-way
// through (L1). Every WRITE bumps the revision; every envelope reports it (D2), so a background result can be applied only
// if nothing changed since it was read (L3).
//
// ⚠️ The key is the root LEXICALLY normalised, never canonicalised: `weakly_canonical` stats the path, a network round trip
// per call on a share. Two spellings of one project through a symlink get two locks (accepted).
//
// ⚠️ Lock order: this lock FIRST, then a registry's own mutex (history, index, locator) or a world's write lock. Those never
// call back into an endpoint, and no endpoint calls another (checked 2026-10-10), so no cycle exists.

#include <cstdint>
#include <mutex>

namespace scrivi::abi {

struct ProjectLockEntry {
    std::mutex mutex;
    int64_t revision = 0;   // in memory: this process's view; a fresh process starts at 0
};

// The entry for a project root (created on first use, never erased).
ProjectLockEntry& projectLockEntry(const char* root);

struct ProjectCallContext {
    int64_t revision = 0;
    int64_t lockWaitMs = 0;
};

// The project call running on THIS thread, or nullptr outside one.
const ProjectCallContext* currentProjectCall();

class ProjectCallGuard {
public:
    enum class Kind { read, write };

    // A null or empty root takes no lock: the endpoint's own validation reports it.
    ProjectCallGuard(const char* root, Kind kind);
    ~ProjectCallGuard();
    ProjectCallGuard(const ProjectCallGuard&) = delete;
    ProjectCallGuard& operator=(const ProjectCallGuard&) = delete;

private:
    ProjectLockEntry* entry_ = nullptr;
    std::unique_lock<std::mutex> lock_;
    ProjectCallContext context_;
    const ProjectCallContext* previous_ = nullptr;
};

}  // namespace scrivi::abi
