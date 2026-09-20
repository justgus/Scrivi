#pragma once

// ⚠️ SP-144 AC1 INSTRUMENT — attribute filesystem calls to the code that makes them.
//
// [I-0231] measured 8,044 `openat` + 14,362 `read` syscalls for ONE
// `scrivi_open_project` over a 150-file project, with each scene sidecar opened
// ~96x. `strace` proves the AMPLIFICATION but not its ORIGIN: it sees syscalls,
// not call sites, so it cannot say WHICH of the open path's passes re-reads a
// file or how many times each one does.
//
// ⛔ SP-144 AC1 forbids answering that by reading the code ("LOCATE the
// amplification by measurement, not by reading" — [I-0181]'s three
// inference-driven narrowings are the stated warning). This decorator is the
// measurement: it wraps the REAL FileSystem, forwards every call unchanged, and
// counts them per-path and per-phase.
//
// ✅ It counts CALLS, not syscalls. That is the useful unit here: a `readTextFile`
// is one call and a bounded number of syscalls, so a path read 96 times by the
// core is 96 calls regardless of how the platform layer spells them. The ratio
// to strace's figures is a constant, and the defect is the repetition.
//
// ⚠️ NOT a mock and NOT test-only behaviour: it changes no result, swallows no
// error, and reorders nothing. A measurement that altered the thing measured
// would be worthless for a before/after comparison (AC6).

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "scrivi/Services.hpp"

namespace scrivi::testing {

// One phase of an operation — the label a run of calls is attributed to.
// Phases are pushed by the harness around the passes it wants to separate, so
// the report reads "migrateScenes: 412 reads" rather than "412 reads".
class CountingFileSystem final : public FileSystem {
public:
    struct Counts {
        std::size_t exists            = 0;
        std::size_t isDirectory       = 0;
        std::size_t deviceID          = 0;
        std::size_t readTextFile      = 0;
        std::size_t listDirectory     = 0;
        std::size_t atomicWrite       = 0;
        std::size_t appendTextFile    = 0;
        std::size_t createDirectories = 0;
        std::size_t createExclusive   = 0;
        std::size_t removeFile        = 0;
        std::size_t renamePath        = 0;
        std::size_t copyFileInBlocks  = 0;

        // Every call that touches the filesystem at all, however it spells it.
        [[nodiscard]] std::size_t total() const {
            return exists + isDirectory + deviceID + readTextFile + listDirectory
                 + atomicWrite + appendTextFile + createDirectories + createExclusive
                 + removeFile + renamePath + copyFileInBlocks;
        }

        // The two that dominate [I-0231]: content reads and directory walks.
        [[nodiscard]] std::size_t reads() const { return readTextFile + listDirectory; }

        // Metadata probes. `exists` is broken out because an ABSENT world
        // `binding.json` cost 188 ENOENT probes (AC3), and those are `exists`
        // calls, not reads.
        [[nodiscard]] std::size_t probes() const { return exists + isDirectory + deviceID; }
    };

    explicit CountingFileSystem(FileSystem& inner) : inner_(inner) {}

    // --- phase labelling ---------------------------------------------------
    // Calls made while a phase is current are attributed to it AND to the
    // totals. Phases do not nest: pushing a new one ends the previous.
    void beginPhase(std::string name) { phase_ = std::move(name); }
    void endPhase() { phase_.clear(); }

    void reset() {
        total_ = Counts{};
        byPath_.clear();
        byPhase_.clear();
        byPhasePath_.clear();
        phase_.clear();
    }

    [[nodiscard]] const Counts& total() const { return total_; }
    [[nodiscard]] const std::map<std::string, Counts>& byPath()  const { return byPath_; }
    [[nodiscard]] const std::map<std::string, Counts>& byPhase() const { return byPhase_; }

    // Calls to `path` made during `phase` — the per-site figure AC1 asks for.
    [[nodiscard]] Counts forPhasePath(const std::string& phase, const std::string& path) const {
        auto it = byPhasePath_.find(phase + "\n" + path);
        return it == byPhasePath_.end() ? Counts{} : it->second;
    }

    // The worst-repeated paths, most-touched first — the shape of the defect in
    // one glance. `n` entries, or all of them when n == 0.
    [[nodiscard]] std::vector<std::pair<std::string, Counts>> hottestPaths(std::size_t n = 0) const {
        std::vector<std::pair<std::string, Counts>> v{byPath_.begin(), byPath_.end()};
        std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) {
            if (a.second.total() != b.second.total()) { return a.second.total() > b.second.total(); }
            return a.first < b.first;  // stable, so a report diffs cleanly
        });
        if (n != 0 && v.size() > n) { v.resize(n); }
        return v;
    }

    // --- FileSystem: forward everything, count everything -------------------
    Result<bool> exists(const AbsolutePath& path) override {
        tally(path, &Counts::exists);
        return inner_.exists(path);
    }

    Result<bool> isDirectory(const AbsolutePath& path) override {
        tally(path, &Counts::isDirectory);
        return inner_.isDirectory(path);
    }

    Result<std::uint64_t> deviceID(const AbsolutePath& path) override {
        tally(path, &Counts::deviceID);
        return inner_.deviceID(path);
    }

    Result<void> createDirectories(const AbsolutePath& path) override {
        tally(path, &Counts::createDirectories);
        return inner_.createDirectories(path);
    }

    Result<Utf8Text> readTextFile(const AbsolutePath& path) override {
        tally(path, &Counts::readTextFile);
        return inner_.readTextFile(path);
    }

    Result<void> atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text) override {
        tally(path, &Counts::atomicWrite);
        return inner_.atomicWriteTextFile(path, utf8Text);
    }

    Result<void> createFileExclusive(const AbsolutePath& path, std::string_view utf8Text) override {
        tally(path, &Counts::createExclusive);
        return inner_.createFileExclusive(path, utf8Text);
    }

    Result<void> appendTextFile(const AbsolutePath& path, std::string_view utf8Text) override {
        tally(path, &Counts::appendTextFile);
        return inner_.appendTextFile(path, utf8Text);
    }

    Result<std::vector<AbsolutePath>> listDirectory(const AbsolutePath& path) override {
        tally(path, &Counts::listDirectory);
        return inner_.listDirectory(path);
    }

    Result<void> removeFile(const AbsolutePath& path) override {
        tally(path, &Counts::removeFile);
        return inner_.removeFile(path);
    }

    Result<void> renamePath(const AbsolutePath& from, const AbsolutePath& to) override {
        tally(from, &Counts::renamePath);
        return inner_.renamePath(from, to);
    }

    Result<void> copyFileInBlocks(const AbsolutePath& from,
                                  const AbsolutePath& to,
                                  std::size_t blockSize,
                                  const std::function<Result<void>()>& onBlock) override {
        tally(from, &Counts::copyFileInBlocks);
        return inner_.copyFileInBlocks(from, to, blockSize, onBlock);
    }

private:
    void tally(const AbsolutePath& path, std::size_t Counts::* field) {
        ++(total_.*field);
        ++(byPath_[path].*field);
        if (!phase_.empty()) {
            ++(byPhase_[phase_].*field);
            ++(byPhasePath_[phase_ + "\n" + path].*field);
        }
    }

    FileSystem& inner_;
    std::string phase_;
    Counts      total_;

    std::map<std::string, Counts> byPath_;
    std::map<std::string, Counts> byPhase_;
    std::map<std::string, Counts> byPhasePath_;
};

} // namespace scrivi::testing
