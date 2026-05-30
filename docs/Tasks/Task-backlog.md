# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## EP-008: Multi-Scene Navigation and Cross-Platform Build

### SP-017 — Cross-Platform Build: Ubuntu

#### T-0055: Ubuntu CMake Build — GCC/Clang Green + Gap Document

**Status:** 🔵 Backlog
**Sprint:** SP-017
**Epic:** EP-008
**Priority:** Critical

Get ScriviCore building and all CTests passing on Ubuntu using GCC 13+ or Clang 17+. Document every compiler/stdlib gap, FetchContent issue, or filesystem behavior difference found. No new features.

**Checklist:**
- `cxx_std_23` feature flag on GCC 13 / Clang 17 — note any `<format>`, `<expected>`, `<ranges>` gaps
- FetchContent for nlohmann/json and Catch2 resolves cleanly
- `AtomicWrite` / `LocalFileSystem::atomicWriteTextFile` POSIX rename semantics verified
- `AppSupportLayout` needs `$XDG_DATA_HOME` / `~/.local/share/Scrivi` path (feeds T-0057)
- `SystemGitProvider` process execution on Linux — path quoting and executable lookup
- Output: gap document in Task detail or a `docs/` trade-study note

---

### SP-018 — Cross-Platform Build: Windows + SecureStore Trade Study

#### T-0056: Windows CMake Build — MSVC Green + Gap Document

**Status:** 🔵 Backlog
**Sprint:** SP-018
**Epic:** EP-008
**Priority:** Critical

Get ScriviCore building and all CTests passing on Windows (MSVC 19.38+ / VS 2022). Document every MSVC-specific gap.

**Checklist:**
- MSVC C++23 feature parity vs. Apple Clang — `std::format`, `std::expected`, structured bindings
- `AtomicWrite` — verify `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` path or add it
- `SystemGitProvider` process execution — path quoting, spaces in `Program Files`
- FetchContent on Windows — any network/proxy issues
- Output: gap document in Task detail

---

#### T-0057: `AppSupportLayout` — Linux and Windows Platform Paths

**Status:** 🔵 Backlog
**Sprint:** SP-018
**Epic:** EP-008
**Priority:** High

Add Linux and Windows platform path resolution to `AppSupportLayout`. Currently macOS/Apple-only.

- Linux: `$XDG_DATA_HOME` if set, else `~/.local/share/Scrivi`
- Windows: `SHGetKnownFolderPath(FOLDERID_RoamingAppData)` → `%APPDATA%\Scrivi`; environment variable fallback

---

#### T-0058: SecureStore Trade Study — Linux and Windows

**Status:** 🔵 Backlog
**Sprint:** SP-018
**Epic:** EP-008
**Priority:** High

Produce a written trade study selecting the `SecureStore` implementation strategy for Linux and Windows. Must end with a concrete recommendation per platform, not just a list of options.

- **Linux:** libsecret (D-Bus Secret Service) vs. encrypted-file fallback (platform-derived key). Key question: what happens in CI, headless servers, and WSL where D-Bus may not be running?
- **Windows:** DPAPI (`CryptProtectData`) — per-user encryption, no external dependency, Win10+ ubiquitous. Confirm suitability; note any roaming profile concerns.
- Output: `docs/Scrivi_SecureStore_Platform_Trade_Study_v0_1.md`

---

### SP-019 — Multi-Scene C++ Core

#### T-0059: `OpenProjectResult` — Add Scene List (`std::vector<SceneSummary>`)

**Status:** 🔵 Backlog
**Sprint:** SP-019
**Epic:** EP-008
**Priority:** Critical

Extend `OpenProjectResult` (in `Results.hpp`) to include a `std::vector<SceneSummary> scenes` field. `ManuscriptOrderResolver` already produces ordered scene data; populate the result from it in `ProjectOpener`. `SceneSummary` already exists in `Types.hpp` — reuse as-is.

---

#### T-0060: `openScene` Facade Method — Switch Active Scene

**Status:** 🔵 Backlog
**Sprint:** SP-019
**Epic:** EP-008
**Priority:** Critical

Add `openScene` to the `ScriviCore` facade. Takes a `SceneID` (plus project root and app support root); reads scene content and metadata; updates `workspace-state.json` to record the new active scene; returns content, metadata, and restored cursor/scroll state.

New types needed: `OpenSceneRequest`, `OpenSceneResult` in `Requests.hpp` / `Results.hpp`.

---

#### T-0061: Adapter + Swift Engine — Expose `openScene` and Scene List

**Status:** 🔵 Backlog
**Sprint:** SP-019
**Epic:** EP-008
**Priority:** Critical

- Add `openScene(...)` method to `ScriviCoreAdapter` with JSON envelope output.
- Update `openProject` adapter serialization to include the `scenes` array.
- Add `openScene(...)` to `ScriviEngine.swift` with a `SceneInfo` Swift type mirroring `SceneSummary`.
- Update `OpenProjectResult` Swift type to include `scenes: [SceneInfo]`.

---

#### T-0062: Integration Tests — Multi-Scene `openProject` and `openScene`

**Status:** 🔵 Backlog
**Sprint:** SP-019
**Epic:** EP-008
**Priority:** High

Add integration tests covering:
- `openProject` on a multi-chapter, multi-scene fixture returns correct ordered scene list
- `openScene` loads the correct content and updates workspace state
- Re-opening the project after `openScene` restores the correct active scene
- `swift test` interop test for `openScene` round-trip

---

*Last Updated: 2026-05-30 (T-0055–T-0062 added for EP-008)*
