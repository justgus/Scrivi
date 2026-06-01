# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## EP-008: Multi-Scene Navigation and Cross-Platform Build


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

*Last Updated: 2026-05-31 (T-0056, T-0057, T-0058 implemented; all moved to unverified)*
