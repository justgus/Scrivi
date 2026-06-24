# Sprint SP-048: Per-Window Model — Foundation (Spike, ProjectSession, Registry)

**Status:** ✅ Closed
**Epic:** EP-018
**Start Date:** 2026-06-24
**End Date:** 2026-06-24
**Goal:** De-risk and lay the foundation for the per-window/per-project model: prove the
`WindowGroup(for:)` focus-by-value behavior (V1 spike), then extract a per-window
`ProjectSession` off the shared `AppEnvironment` and add an `OpenProjectRegistry` — all
behavior-preserving (app stays single-window until SP-049). No ScriviCore/C++ changes.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3 mechanism) | ✅ Done |
| T-0192 | Extract `ProjectSession`; move per-project state + methods off `AppEnvironment` (behavior-preserving) | ✅ Verified |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` (projectID → session); authoritative R3 check-and-focus guard | ✅ Verified |

### Acceptance Criteria

- [x] T-0191 produced a clear result on `WindowGroup(for:)` focus-by-value on macOS 26, and the R3 mechanism was chosen and recorded before any production refactor landed. (Result: native de-dup is NOT race-safe → `OpenProjectRegistry` is the authoritative R3 guard.)
- [x] `ProjectSession` owns all former per-project state; `AppEnvironment` keeps only engine + identity + registry; app behavior unchanged (still single window this sprint).
- [x] `OpenProjectRegistry` exists and is wired into `AppEnvironment`.
- [x] macOS build clean; no regression to project open/save/close (user manual regression 2026-06-24).

### Sprint Notes

- **T-0191 result:** macOS 26 `WindowGroup(for:)` de-dups by value **only against already-established windows; NOT race-safe.** Two `openWindow(value:)` for the same value in quick succession → two windows; re-opening an already-open value → focuses it. **Decision:** `OpenProjectRegistry` (T-0193) is the **authoritative R3 guard** (check-and-focus before `openWindow`); native de-dup is a steady-state backstop only. Matters because restore-all (T-0195) and deep links open windows concurrently — the exact race native behavior misses. Spike code removed after the result was recorded. Full detail in design doc §3.2.
- Reuses `ProjectBookmarkStore` (built under T-0184) for the later restore manifest — that work is not wasted.

### Retrospective

**Completed:**
- V1 spike (T-0191) run, observed, recorded, and removed; R3 mechanism decided (registry-authoritative).
- `ProjectSession` (`Scrivi/App/ProjectSession.swift`) extracted; all per-project state + lifecycle moved off `AppEnvironment`, dependencies injected at construction; views migrated to `.environment(ProjectSession.self)`.
- `OpenProjectRegistry` (`Scrivi/App/OpenProjectRegistry.swift`) added and wired into open/close/deep-link as the authoritative R3 guard.
- Both new files registered in `project.pbxproj` (C050, C051).
- macOS build green throughout; user verified T-0192 + T-0193 via manual regression (open/edit/save/close, timeline, inspector, navigation, and the deep-link R3 path).

**Returned to Backlog:**
- None — all tasks completed and verified within the sprint.

**What went well:**
- Gating the refactor on the spike paid off: the "not race-safe" finding changed the R3 design *before* any production code committed to native de-dup, and gave the registry a concrete justification.
- The behavior-preserving discipline held — the app behaves identically (single window) after a large multi-file refactor.
- A confusing "second window" during manual testing was correctly diagnosed as a separate process (Xcode debugger vs. installed build), not a registry regression, and documented so it won't resurface as a phantom bug.

**Carried into SP-049:**
- T-0194 (`WindowGroup(for:)` + Landing window) and T-0195 (restore-all-windows) turn the foundation into actual per-window behavior. The registry becomes per-window-spanning; native de-dup remains a steady-state backstop only.

---

*Closed 2026-06-24 with user approval. EP-018 continues with SP-049.*
