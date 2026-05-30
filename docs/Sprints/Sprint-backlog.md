# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started. At most one Sprint is Active at a time; the remainder queue here until their predecessor closes.

---

## SP-017: Cross-Platform Build — Ubuntu (GCC/Clang)

**Status:** 🔵 Planning
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Get ScriviCore building and all 159 CTests passing on Ubuntu using GCC 13+ (or Clang 17+) via CMake. Document every compiler gap, stdlib difference, or FetchContent issue encountered. No new features — a clean green build is the only success criterion.
**Start Date:** —
**End Date:** —
**Capacity:** —

### Planned Tasks

| ID | Title | Priority | Notes |
| -- | ----- | -------- | ----- |
| T-0055 | Ubuntu CMake Build — GCC/Clang Green + Gap Document | Critical | CI matrix or local Ubuntu VM/container |

### Sprint Notes

- Verify `cxx_std_23` feature flag works on GCC 13 and Clang 17; note any `<format>`, `<expected>`, or `<ranges>` gaps.
- Verify `FetchContent` for nlohmann/json and Catch2 resolves cleanly offline and online.
- Check `AtomicWrite` / `LocalFileSystem::atomicWriteTextFile` POSIX rename semantics vs. macOS.
- Check `AppSupportLayout` — needs `$XDG_DATA_HOME` / `~/.local/share/Scrivi` path for Linux.
- Check `SystemGitProvider` process execution — path quoting and executable lookup on Linux.
- Output: a written gap document (trade-study note or Task detail) recording all findings.

---

## SP-018: Cross-Platform Build — Windows (MSVC) + SecureStore Trade Study

**Status:** 🔵 Planning
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Get ScriviCore building and all CTests passing on Windows (MSVC 19.38+ / VS 2022). Implement `AppSupportLayout` for Windows (`%APPDATA%`) and Ubuntu (`$XDG_DATA_HOME`). Produce a written trade study selecting the `SecureStore` implementation strategy for each platform (DPAPI on Windows; libsecret vs. encrypted-file fallback on Linux).
**Start Date:** —
**End Date:** —
**Capacity:** —

### Planned Tasks

| ID | Title | Priority | Notes |
| -- | ----- | -------- | ----- |
| T-0056 | Windows CMake Build — MSVC Green + Gap Document | Critical | VS 2022 / GitHub Actions windows-latest |
| T-0057 | `AppSupportLayout` — Linux and Windows Platform Paths | High | `XDG_DATA_HOME` on Linux; `SHGetKnownFolderPath` on Windows |
| T-0058 | SecureStore Trade Study — Linux (libsecret vs. encrypted-file) and Windows (DPAPI) | High | Written trade-study doc; no implementation yet |

### Sprint Notes

- Windows atomic write: verify `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` path in `LocalFileSystem` (or add it).
- Windows path quoting in `SystemGitProvider::Process` — spaces in `Program Files` must be handled.
- MSVC C++23 feature parity: note any `std::format`, `std::expected`, or structured binding gaps vs. Apple Clang.
- SecureStore trade study should produce a concrete recommendation (not just a list of options) for both platforms, with a rationale that feeds the implementation sprint.

---

## SP-019: Multi-Scene C++ Core — `openProject` Scene List + `openScene`

**Status:** 🔵 Planning
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Extend `openProject` to return a full scene list alongside the active scene. Add `openScene` to the C++ facade and adapter, allowing the UI to switch the active scene without reopening the project. All three test suites (macOS ctest, swift test, Ubuntu ctest) must remain green.
**Start Date:** —
**End Date:** —
**Capacity:** —

### Planned Tasks

| ID | Title | Priority | Notes |
| -- | ----- | -------- | ----- |
| T-0059 | `OpenProjectResult` — Add Scene List (`std::vector<SceneSummary>`) | Critical | `ManuscriptOrderResolver` already has the data; populate the result |
| T-0060 | `openScene` Facade Method — Switch Active Scene | Critical | New `OpenSceneRequest` / `OpenSceneResult`; updates `workspace-state.json` |
| T-0061 | Adapter + Swift Engine — Expose `openScene` and Scene List | Critical | JSON envelope for scene list; Swift `SceneInfo` type |
| T-0062 | Integration Tests — Multi-Scene `openProject` and `openScene` | High | Multi-chapter, multi-scene fixture; round-trip workspace state |

### Sprint Notes

- `SceneSummary` already exists in `Types.hpp` — reuse as-is.
- `openScene` should accept a `SceneID` and return content + metadata + updated workspace state. Minimal new surface.
- Scene list ordering must match `ManuscriptOrderResolver` output (chapter order, then scene order within chapter).
- Swift `OpenProjectResult` gains a `scenes: [SceneInfo]` property; `SceneInfo` mirrors `SceneSummary`.
- SwiftUI scene navigation sidebar is **deferred to EP-008** — this sprint delivers the C++ and adapter layer only.
- If Ubuntu build (SP-017) surfaces interface-breaking compiler issues, resolve them before finalizing `Requests.hpp` / `Results.hpp` changes here.

---

*Last Updated: 2026-05-30 (SP-017, SP-018, SP-019 queued for EP-007)*
