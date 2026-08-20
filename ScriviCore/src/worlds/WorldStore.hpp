#pragma once

// WorldStore — world packages, bindings, resolution, and write locking.
// EP-031 SP-097 (T-0381 / T-0382 / T-0383). Design: World Data Separation v0.1
// §6.1–§6.5, §7.

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "worlds/WorldTypes.hpp"

#include <string>
#include <vector>

namespace scrivi::worlds {

// The outcome of trying to reach a world's package on disk.
struct WorldResolution {
    WorldStatus  status = WorldStatus::unavailable;
    AbsolutePath packagePath;      // valid only when status == available
    WorldRecord  world;            // populated only when status == available

    // ⚠️ The best candidate path we TRIED, regardless of outcome (T-0419, I-0137).
    //
    // DELIBERATELY a separate field from `packagePath`, which means "verified"
    // and is trusted as such by every caller. This one means only "where we
    // looked" — it may not exist, may be unreadable, may belong to a different
    // world. Widening `packagePath` to carry it would silently weaken a
    // guarantee other code depends on; that is the mistake I-0115 taught.
    //
    // Why it must exist: WorldVolumeStatus.refine (Apple) distinguishes
    // `unmounted` from `offline` by inspecting the path's volume — and it can
    // only run on a world that is NOT available. Supplying only a verified path
    // guaranteed the refinement could never fire for the one case it exists for.
    AbsolutePath lastKnownPackagePath;
};

// One row for scrivi_list_worlds.
struct WorldSummary {
    std::string  worldID;
    std::string  displayName;      // live when available, else the binding's cache
    WorldStatus  status = WorldStatus::unavailable;
    AbsolutePath packagePath;      // set ONLY when status == available (verified)
    // ⚠️ Set whenever a candidate was tried — see WorldResolution above (T-0419).
    AbsolutePath lastKnownPackagePath;
    std::int64_t epochOffsetMs = 0;
};

class WorldStore {
public:
    explicit WorldStore(CoreServices& services);

    // --- project-side layout -------------------------------------------------
    [[nodiscard]] static AbsolutePath worldsDir(const AbsolutePath& projectRoot);
    [[nodiscard]] static AbsolutePath bindingPath(const AbsolutePath& projectRoot,
                                                  const std::string& worldID);

    // --- package-side layout -------------------------------------------------
    [[nodiscard]] static AbsolutePath worldJsonPath(const AbsolutePath& packagePath);
    [[nodiscard]] static AbsolutePath worldIndexPath(const AbsolutePath& packagePath);
    [[nodiscard]] static AbsolutePath lockPath(const AbsolutePath& packagePath);

    // --- T-0381: create ------------------------------------------------------
    // Creates the .scrivworld package AND this project's binding in one
    // operation. On failure the project is left exactly as it was (Doc 3 §7).
    [[nodiscard]] Result<WorldRecord> createWorld(const AbsolutePath& projectRoot,
                                                  const AbsolutePath& packagePath,
                                                  const std::string& displayName,
                                                  const std::string& epochLabel) const;

    // Binds an EXISTING package to this project. Read-only toward the world, so
    // it takes no lock (Doc 3 §7).
    [[nodiscard]] Result<WorldRecord> addWorld(const AbsolutePath& projectRoot,
                                                const AbsolutePath& packagePath) const;

    // --- T-0382: bindings + resolution --------------------------------------
    [[nodiscard]] Result<WorldBindingRecord> loadBinding(const AbsolutePath& projectRoot,
                                                          const std::string& worldID) const;
    [[nodiscard]] Result<void> saveBinding(const AbsolutePath& projectRoot,
                                            const WorldBindingRecord& binding) const;

    [[nodiscard]] Result<std::vector<std::string>> listBoundWorldIDs(
        const AbsolutePath& projectRoot) const;

    // Resolves a world to its package. Tries the binding's relative path first
    // (what survives moving a project and its worlds together), then the
    // absolute one. The first candidate whose world.json carries the MATCHING
    // worldID wins; a different worldID is NOT the world.
    //
    // Never fails: an unreachable world is a status, not an error (§6.4).
    [[nodiscard]] WorldResolution resolve(const AbsolutePath& projectRoot,
                                           const std::string& worldID) const;

    // Re-points a moved world, VERIFYING worldID before accepting the new path.
    [[nodiscard]] Result<void> relink(const AbsolutePath& projectRoot,
                                       const std::string& worldID,
                                       const AbsolutePath& newPackagePath) const;

    [[nodiscard]] Result<void> removeReference(const AbsolutePath& projectRoot,
                                                const std::string& worldID) const;

    [[nodiscard]] Result<std::vector<WorldSummary>> listWorlds(
        const AbsolutePath& projectRoot) const;

    // --- T-0384: the epoch chain ---------------------------------------------
    //
    //   project_time(event) = event.offsetMs
    //                       + timeline.epochOffsetMs   // → world time (world-relative, ALWAYS)
    //                       + binding.epochOffsetMs    // → project story-time
    //
    // Each layer owns its own epoch (Doc 1 §7.0). Timeline offsets are
    // WORLD-relative, never project-relative, which is what makes rebinding a
    // world change exactly ONE number instead of N.

    // Layer 2→3. Editing this NEVER mutates world.json — the world's own
    // epoch.label is intrinsic and travels with it when shared.
    [[nodiscard]] Result<void> setWorldEpochOffset(const AbsolutePath& projectRoot,
                                                    const std::string& worldID,
                                                    std::int64_t epochOffsetMs) const;

    // Layer 1→2: a historical timeline's offset against its WORLD's epoch.
    [[nodiscard]] Result<void> setTimelineEpochOffset(const AbsolutePath& projectRoot,
                                                       const std::string& worldID,
                                                       const std::string& timelineID,
                                                       std::int64_t epochOffsetMs) const;

    [[nodiscard]] Result<std::int64_t> timelineEpochOffset(const AbsolutePath& projectRoot,
                                                            const std::string& worldID,
                                                            const std::string& timelineID) const;

    // Resolves the full chain for one timeline: returns
    // timeline.epochOffsetMs + binding.epochOffsetMs, the amount to add to an
    // event's own offset to land on project story-time.
    [[nodiscard]] Result<std::int64_t> resolveTimelineProjectOffset(
        const AbsolutePath& projectRoot,
        const std::string& worldID,
        const std::string& timelineID) const;

    // --- T-0385: cached index ------------------------------------------------
    // Refreshes binding.cachedIndex from the world's own index.json. Called on
    // every successful resolve; the cache is NEVER authoritative (§6.3).
    [[nodiscard]] Result<void> refreshCachedIndex(const AbsolutePath& projectRoot,
                                                   const std::string& worldID,
                                                   const AbsolutePath& packagePath) const;

private:
    CoreServices& services_;
};

// --- T-0383: write locking ---------------------------------------------------
//
// A lock FILE, not an OS advisory lock: advisory semantics differ across
// Scrivi's seven targets and behave poorly on network volumes — which is
// exactly where a shared world lives (§6.5).
//
// Held for the DURATION OF A WRITE, never a session. Reads never lock and never
// block; any number of projects may read a world at once.
class WorldLock {
public:
    static constexpr int kStaleSeconds = 60;

    WorldLock(CoreServices& services, AbsolutePath packagePath);
    ~WorldLock();                       // releases if still held

    WorldLock(const WorldLock&)            = delete;
    WorldLock& operator=(const WorldLock&) = delete;

    // Acquires via createFileExclusive (T-0403) — the ONLY primitive that makes
    // "exactly one winner" true across processes. A lock whose heartbeatAt is
    // older than kStaleSeconds is presumed dead and broken; a crashed writer
    // therefore blocks others for at most a minute, never permanently.
    [[nodiscard]] Result<void> acquire(const std::string& projectID);

    // Refreshes heartbeatAt so a long write is not mistaken for a dead one.
    [[nodiscard]] Result<void> heartbeat();

    [[nodiscard]] Result<void> release();

    [[nodiscard]] bool held() const { return held_; }

private:
    CoreServices& services_;
    AbsolutePath  packagePath_;
    std::string   lockID_;
    bool          held_ = false;
};

} // namespace scrivi::worlds
