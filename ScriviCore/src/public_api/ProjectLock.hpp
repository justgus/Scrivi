#pragma once
// SP-173 / I-0285 — D1 + D2 of docs/Scrivi_Core_Concurrency_Design_v0_1.md, split in two by EP-052 (SP-194, T-0623;
// docs/Scrivi_Background_Writes_Design_v0_2.md §3, Q1 ruled 2026-10-26): every C ABI endpoint that takes a project takes
// ONE of its two locks. Internal to the C ABI layer (never in a public header); its own file so tests can hold a lock and
// prove a real `scrivi_*` call waits for it, or does not.
//
// - The DISK lock (D): an endpoint that may touch the project's files holds it for the whole call (READ / WRITE). Calls
//   that touch files never overlap (L1), as before.
//   Before it takes D, a file call waits for every job already in the project's write queue (W3), so it sees every
//   earlier queued write. ⛔ Not on the writer thread, and not inside another call, which could hold D.
// - The MEMORY lock (M): an endpoint that touches no file holds only M (MEMORY_READ / MEMORY_WRITE), so a caret move never
//   waits for a save. M also guards the revision; a file call takes it only for the instant its revision is read or bumped.
//
// Every WRITE bumps the revision; every envelope reports it (D2), so a background result can be applied only if nothing
// changed since it was read (L3).
//
// ⚠️ Lock order: D before M, never M before D (a memory call never asks for D). Then a registry's own mutex (history,
// index, locator) or a world's write lock. Those never call back into an endpoint, and no endpoint calls another.
//
// ⚠️ A memory call that makes a file operation is MISCLASSIFIED: it is reported on stderr, and fatal where tests ask.
//
// ⚠️ The key is the root LEXICALLY normalised, never canonicalised: `weakly_canonical` stats the path, a network round trip
// per call on a share. Two spellings of one project through a symlink get two entries (accepted).

#include "public_api/WriteQueue.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

namespace scrivi::abi {

struct ProjectLockEntry {
    std::mutex disk;
    std::mutex memory;
    int64_t revision = 0;   // under `memory`. This process's view; a fresh process starts at 0
    WriteQueue queue{disk};  // EP-052 (T-0624): the project's writes, in order
};

// The entry for a project root (created on first use, never erased).
ProjectLockEntry& projectLockEntry(const char* root);

struct ProjectCallContext {
    int64_t revision = 0;
    int64_t lockWaitMs = 0;
    // [SP-188] AC11 / EP-052 inventory: which endpoint, since when (after the lock), and the file operations before it.
    const char* endpoint = "";
    std::chrono::steady_clock::time_point started{};
    int64_t fileOpsAtStart = 0;
};

// The project call running on THIS thread, or nullptr outside one.
const ProjectCallContext* currentProjectCall();

class ProjectCallGuard {
public:
    // `unlocked`: holds NO lock for the call, because it WAITS for the write queue (`scrivi_wait_for_writes`,
    // `scrivi_history_close`): holding D would stop the writer, holding M every memory call.
    enum class Kind { read, write, memoryRead, memoryWrite, unlocked };

    // A null or empty root takes no lock: the endpoint's own validation reports it.
    ProjectCallGuard(const char* root, Kind kind, const char* endpoint = "");
    ~ProjectCallGuard();
    ProjectCallGuard(const ProjectCallGuard&) = delete;
    ProjectCallGuard& operator=(const ProjectCallGuard&) = delete;

private:
    ProjectLockEntry* entry_ = nullptr;
    std::unique_lock<std::mutex> lock_;   // D for read/write, M for the memory kinds
    bool memoryKind_ = false;
    ProjectCallContext context_;
    const ProjectCallContext* previous_ = nullptr;
};

// Memory calls that made a file operation, since the process started (a test reads it).
int64_t memoryCallFileOpViolations();
// Tests: a misclassified memory call aborts the process instead of only logging.
void setMemoryCallViolationsFatal(bool fatal);

}  // namespace scrivi::abi
