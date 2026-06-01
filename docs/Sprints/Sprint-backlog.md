# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started. At most one Sprint is Active at a time; the remainder queue here until their predecessor closes.

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
