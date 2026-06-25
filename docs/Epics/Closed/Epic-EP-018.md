# Epic EP-018 — Closed

## EP-018: Per-Window / Per-Project Window Model

**Status:** ✅ Closed
**Goal:** Let a writer keep **multiple distinct projects open at once, each in its own window**; make a project **non-reentrant** (re-opening an open project focuses its existing window instead of duplicating it); and **restore all previously-open project windows** on relaunch. This is the architectural foundation that a correct deep-link feature (EP-017 T-0184) depends on.
**Date Created:** 2026-06-24
**Date Activated:** 2026-06-24
**Actual Close Date:** 2026-06-25
**Design Reference:** `docs/Scrivi_PerWindow_Project_Model_Design_v0_1.md` (✅ approved 2026-06-24)
**Span:** 3 Sprints (SP-048 → SP-050)
**Origin:** Surfaced while verifying EP-017 / T-0184. The app's single shared `AppEnvironment` (one `openProjectResult`/`viewportLoader`/`timelineModel`, injected into every `WindowGroup` instance) meant all windows rendered the same one project — incompatible with the user's per-project/per-window + restore requirements. Made its own Epic per user direction.

### Strategic Rationale

Multiple projects open simultaneously, one per window, restored where she left off, is a first-class capability for a serious authoring/worldbuilding tool — and a foundational one: EP-017 deep links and likely future navigation work sit on top of it. The single-project model predated these requirements; building more features on it would have deepened the mistake. **Swift-layer only — no ScriviCore/C++ changes.**

### Acceptance Criteria — all met

- [x] **R1** — Multiple **distinct** projects can be open simultaneously. *(User-verified 2026-06-25.)*
- [x] **R2** — Each open project lives in **its own window**. *(User-verified 2026-06-25.)*
- [x] **R3** — The same project is **non-reentrant**: re-opening an already-open project **focuses its existing window** rather than opening a second copy. *(User-verified 2026-06-25.)*
- [x] **R4** — On relaunch, the app **restores all previously-open project windows** (within each, the backend open flow already restores scene/cursor/scroll). *(User-verified 2026-06-25.)*
- [x] **R5** — A deep link opens/focuses the target project's window, then selects the item (T-0184 rebuilt on the new model, incl. the scene-`ID` fix). *(User-verified 2026-06-25 — deep link to chapter-005/scene 2 landed exactly on the target scene.)*
- [x] **AC-build** — macOS build and codesign clean; `ctest` green; no regression to existing project open/save/close behavior. *(User-verified 2026-06-25.)*

### Outcome — final architecture (diverged from the approved design's routing plan)

The design proposed SwiftUI `WindowGroup(for: ProjectWindowID.self)` for routing. In implementation
(SP-049 / T-0194) that was **abandoned on evidence** — `WindowGroup(for:)` caches dismissed windows
(reopening a closed project hung on "Loading…") and per-view `.onOpenURL` was unreliable across the
Window+WindowGroup split. **Project windows are managed via AppKit `NSWindow`/`NSWindowController`**
(`ProjectWindowManager`), one per session, with the `OpenProjectRegistry` as the authoritative R3
guard and `NSApplicationDelegate.application(_:open:)` for reliable URL delivery. The Welcome/Landing
surface stays a SwiftUI `Window`. The two-tier state split (`AppEnvironment` + per-window
`ProjectSession`) and the registry-powered R3/R4 from the design held as planned.

### Key Design Decisions (from approved design doc)

- **Two-tier state:** app-global `AppEnvironment` (engine + identity + `OpenProjectRegistry`); per-window `ProjectSession` owns all per-project state.
- **R3 guard:** `OpenProjectRegistry` is authoritative (native `WindowGroup(for:)` de-dup proved not race-safe in the T-0191 spike, then routing moved to AppKit entirely).
- **Restore scope (Q1):** restore **all** previously-open windows via a persisted session manifest reusing `ProjectBookmarkStore`.

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-048 | Foundation — V1 spike, `ProjectSession` extraction, open-project registry | ✅ Closed |
| SP-049 | Windowing & restore — AppKit per-project windows, Welcome, single-instance, restore-all-windows | ✅ Closed |
| SP-050 | Deep-link rewrite on new model + scene-ID fix; open-flow cross-ref; EP-018 verification | ✅ Closed |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3 mechanism) | SP-048 | ✅ Done — registry is authoritative R3 guard (native de-dup not race-safe) |
| T-0192 | Extract `ProjectSession`; move per-project state + methods off `AppEnvironment` (behavior-preserving) | SP-048 | ✅ Verified |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` (projectID → session; powers R3 + R4) | SP-048 | ✅ Verified |
| T-0194 | Per-window project model — AppKit NSWindow per project (R1/R2/R3) + Welcome window; single-instance; File menu | SP-049 | ✅ Verified |
| T-0195 | Session manifest persistence + launch restore of all previously-open windows (R4) | SP-049 | ✅ Verified |
| T-0196 | Rewrite deep-link handler on new model + scene-`ID` fix (R5); open-flow v0.2 cross-ref; EP-018 verification | SP-050 | ✅ Verified |

### Downstream

- **EP-017 AC5 unblocked.** The Spotlight deep-link (selecting a result opens the project & selects
  the scene) is implemented and verified through EP-018 / T-0196. EP-017 T-0184 is superseded; its
  remaining Spotlight-result-continuation wiring can finish under SP-045.

---

*Closed 2026-06-25 with user approval. All acceptance criteria R1–R5 + AC-build user-verified.*
