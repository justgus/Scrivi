# Scrivi — Per-Window / Per-Project Window Model Design (v0.1)

**Status:** ✅ APPROVED 2026-06-24. Implementation tracked under EP-018.
**Author:** Claude (with user)
**Date:** 2026-06-24
**Epic:** EP-018 (Per-Window / Per-Project Window Model) — surfaced while verifying T-0184 deep links under EP-017.
**Supersedes:** nothing — this fills a gap. The window↔project model was never specified.

---

## 1. Why this document exists

While verifying the T-0184 deep-link feature, a deep-link open produced **two windows
showing the same project**. Investigation showed the cause is architectural, not a
deep-link bug:

- `ScriviApp` holds **one** `AppEnvironment` as `@State` (`ScriviApp.swift:9`).
- It is injected into every `WindowGroup` instance via `.environment(env)`
  (`ScriviApp.swift:15`).
- `AppEnvironment` holds **one** project's state: `openProjectResult`,
  `viewportLoader`, `projectPreferences`, `timelineModel`, `pendingNavigationSceneID`,
  panel visibility, etc. (`AppEnvironment.swift:15–47`).

**Consequence:** every macOS window renders the *same single project* from the shared
environment. `closeProject()` then `openProject()` (`AppEnvironment.swift:207–208`) is
a single-project *switch*, not a way to have two projects open.

This directly conflicts with the user's stated requirements:

> The app is **non-reentrant per project** (the same project must not be open twice),
> but supports **multiple distinct projects open at once**, each in its own window, and
> must **restore where she left off** on relaunch.

The deep-link handler (T-0184) was built on top of the single-project model and is
therefore wrong for this app. A correct deep-link feature depends on the per-window /
per-project architecture proposed here. **This design must land before deep links
resume.**

### Source-of-truth check

`Scrivi_Project_Creation_and_Open_Flow_v0_2.md` specifies "restore where she left off"
only at the **project level** — restoring active scene, cursor (`restoredSelection`),
and scroll (`restoredScroll`) when a project opens (§ lines 83, 370, 387). It says
**nothing** about a window↔project mapping or multiple windows. So the per-window model
is a genuinely new decision, not a contradiction of an approved doc. This document
proposes it for approval; once approved it should be cross-referenced from the open-flow
doc.

---

## 2. Requirements (restated for sign-off)

| # | Requirement | Source |
|---|-------------|--------|
| R1 | Multiple **distinct** projects can be open simultaneously. | User |
| R2 | Each open project lives in **its own window**. | User |
| R3 | The same project is **non-reentrant**: opening an already-open project focuses its existing window rather than opening a second copy. | User |
| R4 | On relaunch, the app **restores the set of previously-open projects** (and within each, the scene/cursor/scroll already handled by the backend open flow). | User + open-flow v0.2 |
| R5 | A deep link opens/focuses the target project's window, then selects the item. | EP-017 / T-0184 |

**Open question for the user (Q1):** On a *cold launch with no deep link*, should Scrivi
restore **all** previously-open project windows, or only re-open the **last active**
one and offer the rest via "Open Recent"? (Affects R4 scope. See §7.)

---

## 3. The core architectural change

Move project state **out of the app-global `AppEnvironment`** and into a **per-window
project session**, keyed by a project identity that SwiftUI can use to route windows.

### 3.1 Two-tier state

**Tier A — `AppEnvironment` (one, app-global).** Keeps only what is genuinely
app-wide and project-independent:
- `engine` (the `ScriviEngine` — stateless facade, safe to share)
- `identityResult` / `authorshipRef` (the local author identity)
- `bootstrapError`
- `appSupportRoot`
- the **registry of open projects** (see §4) — for R3 non-reentrancy and R4 restore.

**Tier B — `ProjectSession` (one per open project window).** A new `@Observable
@MainActor` type owning everything currently per-project in `AppEnvironment`:
- `openProjectResult`, `projectRootPath`
- `viewportLoader`, `projectPreferences`, `timelineModel`
- `pendingNavigationSceneID`
- `spotlightDomainIdentifier`, `deepLinkAccessURL`
- per-window UI: `inspectorVisible`, `timelineVisible`, `showProjectSettings`,
  `projectError`

`openProject`, `closeProject`, `onAppResign`, and `donateSpotlight` move (mostly
verbatim) from `AppEnvironment` onto `ProjectSession`. The logic is largely unchanged;
it changes *owner*, not behavior.

### 3.2 Scene routing (the SwiftUI mechanism)

Use a `WindowGroup` parameterized by a `Codable & Hashable` value — the **projectID**
(or a small `ProjectWindowID` struct wrapping it):

```swift
WindowGroup(for: ProjectWindowID.self) { $windowID in
    ProjectWindowRootView(windowID: windowID)   // builds/looks up its ProjectSession
        .environment(env)
}
```

- Opening project P = `openWindow(value: ProjectWindowID(projectID: P))`.
- SwiftUI keys windows by that value, which gives us **R3 for free**: requesting a
  window for an already-open projectID **activates the existing window** instead of
  creating a duplicate (standard `WindowGroup(for:)` behavior).
- A separate, unparameterized `Window` (or the no-value `WindowGroup` branch) hosts the
  **Landing / project-picker** UI when nothing is open.

> **Design note / risk to validate (V1):** the exact "focus existing vs. create new"
> behavior of `WindowGroup(for:)` on macOS 26 must be confirmed empirically before we
> rely on it for R3. If it does not de-duplicate by value, we fall back to tracking
> open windows in the registry and calling `NSApp` window activation. This is the
> single biggest implementation risk and gets a spike (§8, step 0).

### 3.3 ProjectSession lifecycle vs. SwiftUI window lifecycle

`ProjectWindowID` is the *restorable* token (just a projectID — Codable). The live
`ProjectSession` (which owns non-Codable things like `viewportLoader`) is created when a
window appears for an ID and resolved through the registry. The registry maps
`projectID → ProjectSession?` and survives individual view rebuilds.

---

## 4. The open-project registry (Tier A)

A small `@Observable` member of `AppEnvironment`:

```swift
@MainActor @Observable final class OpenProjectRegistry {
    // projectID -> live session (nil entry = known/restorable but window not built yet)
    private(set) var sessions: [String: ProjectSession] = [:]

    func session(for projectID: String) -> ProjectSession?
    func register(_ session: ProjectSession)            // on window open
    func deregister(projectID: String)                  // on window close
    var openProjectIDs: [String] { Array(sessions.keys) }
}
```

Responsibilities:
- **R3:** before opening, check `sessions[projectID]`; if present, focus that window
  instead of opening.
- **R4:** the set of open projectIDs is the restoration manifest (persisted; see §5).
- Provides the deep-link handler a clean "is this project already open?" answer that is
  *global*, not "is it the one project in the shared env".

---

## 5. Persistence & restore (R4)

Persist a small **session manifest** — the list of open projectIDs and which was
frontmost — to `UserDefaults` (or `@AppStorage`). On launch:

1. Bootstrap identity (unchanged).
2. Read the manifest.
3. For each projectID, resolve its path via the **existing** `ProjectBookmarkStore`
   (already built in T-0184 — we reuse it, it is not wasted work) and `openWindow`.
4. Within each window, the backend open flow already restores scene/cursor/scroll
   (open-flow v0.2). No new backend work.

Manifest is updated when a project window opens or closes, and on `onAppResign`.

**Depends on Q1** for whether we restore all or just the last-active window.

---

## 6. What happens to T-0184

T-0184's deep-link handler is rewritten to sit on the new model, but its *building
blocks are reused*, not discarded:

- `ScriviDeepLink` parsing — **kept as-is.**
- `ProjectBookmarkStore` — **kept**, now also powering R4 restore.
- `SpotlightDonor` / `donateSpotlight` — **kept**, moves to `ProjectSession`.
- The handler's *control flow* (`closeProject` then `openProject` to "switch") is
  **replaced** by: resolve projectID → ask registry → focus existing window (R3) or
  `openWindow` a new one → set `pendingNavigationSceneID` on that window's session.

The earlier note also stands: **a scene deep link needs the scene's `scene_…` ID, not
its title** (`item=scene:Liszt` will not match). That fix is part of the rewritten
handler, deferred until this model lands.

---

## 7. Non-goals (this design)

- iOS/iPadOS window model (single-window there; `ProjectSession` still applies but
  routing differs — handle when those targets are active).
- CloudKit sync, collaborative reentrancy across devices.
- Changing the C++/ScriviCore backend at all. **This is a Swift-layer-only change.**

---

## 8. Proposed implementation sequence (after approval)

0. **Spike (V1):** confirm `WindowGroup(for:)` de-dup/focus behavior on macOS 26.
   Decide R3 mechanism. (No production code; throwaway.)
1. Introduce `ProjectSession`; move per-project state + methods off `AppEnvironment`
   (behavior-preserving refactor; app still single-window at this point).
2. Introduce `OpenProjectRegistry` in `AppEnvironment`.
3. Convert the scene to `WindowGroup(for: ProjectWindowID.self)` + Landing window;
   wire `openWindow`. Achieve R1/R2/R3.
4. Session manifest persistence + launch restore. Achieve R4 (per Q1).
5. Rewrite the deep-link handler on the new model + scene-ID fix. Achieve R5.
6. Update `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` with a cross-reference to
   this model; verify each step with the user.

Each numbered step is a candidate Task. Tasks T-0185+ are available in the EP-017 range.

---

## 9. Decisions — resolved 2026-06-24

- **Q1 (restore scope):** ✅ **Restore all** previously-open windows on cold launch.
  R4 is full multi-window restore (not last-active only).
- **Q2 (tracking placement):** ✅ This is its **own Epic, EP-018** (Per-Window /
  Per-Project Window Model), spanning **SP-048 → SP-050**, not part of EP-017. T-0184
  (EP-017) is **paused** pending EP-018; its reusable pieces are carried forward per §6.
- **R1–R5:** ✅ Confirmed as written in §2.
- **Design doc:** ✅ Approved.

### Tracking map (EP-018)

| Step (§8) | Task | Sprint |
| --------- | ---- | ------ |
| 0 — V1 spike (gates R3 mechanism) | T-0191 | SP-048 |
| 1 — extract `ProjectSession` | T-0192 | SP-048 |
| 2 — `OpenProjectRegistry` | T-0193 | SP-048 |
| 3 — `WindowGroup(for:)` + Landing window (R1/R2/R3) | T-0194 | SP-049 |
| 4 — session manifest + restore-all-windows (R4) | T-0195 | SP-049 |
| 5+6 — deep-link rewrite + scene-ID fix + open-flow cross-ref + verification (R5) | T-0196 | SP-050 |

---

*Implementation tracked under EP-018. The V1 spike (T-0191) gates the R3 mechanism
before any production refactor commits.*
