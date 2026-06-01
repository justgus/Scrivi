# Active Sprint

## SP-019: Multi-Scene C++ Core — `openProject` Scene List + `openScene`

**Status:** 🟡 Active
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Extend `openProject` to return a full scene list alongside the active scene. Add `openScene` to the C++ facade and adapter, allowing the UI to switch the active scene without reopening the project. All three test suites (macOS ctest, swift test, Ubuntu ctest) must remain green.
**Start Date:** 2026-06-01
**End Date:** —
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0059 | `OpenProjectResult` — Add Scene List | Critical | 🟠 Unverified |
| T-0060 | `openScene` Facade Method — Switch Active Scene | Critical | 🟠 Unverified |
| T-0061 | Adapter + Swift Engine — Expose `openScene` and Scene List | Critical | 🟠 Unverified |
| T-0062 | Integration Tests — Multi-Scene `openProject` and `openScene` | High | 🟠 Unverified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- `SceneSummary` already exists in `Types.hpp` — reuse as-is.
- `openScene` should accept a `SceneID` and return content + metadata + updated workspace state. Minimal new surface.
- Scene list ordering must match `ManuscriptOrderResolver` output (chapter order, then scene order within chapter).
- Swift `OpenProjectResult` gains a `scenes: [SceneInfo]` property; `SceneInfo` mirrors `SceneSummary`.
- SwiftUI scene navigation sidebar is **deferred to EP-008** — this sprint delivers the C++ and adapter layer only.

---

*Last Updated: 2026-06-01 (SP-019 activated; T-0059–T-0062 implemented — 165/165 ctest, 19/19 swift test)*
