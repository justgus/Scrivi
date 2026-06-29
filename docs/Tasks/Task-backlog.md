# Task Backlog

Tasks listed here are documented and ready for Sprint assignment.

New, unstarted tasks are listed as summary rows. Tasks that have been implemented but returned to the backlog before verification retain their full detail below the index table so no documentation is lost.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0175 | Spotlight integration (umbrella) — **superseded by EP-017** | EP-017 | ⚪ Superseded |
| T-0184 | Deep-link: result continuation opens project & selects item | EP-017 (SP-045) | 🟢 Implemented - Not Verified (core verified via T-0196; Spotlight-continuation path hardened, full tap verify → T-0189) |
| T-0185 | New Spotlight importer app-extension target + pbxproj wiring | EP-017 (SP-046) | 🔵 Backlog |
| T-0186 | Link ScriviCore (or facade) into the extension (Option A build graph) | EP-017 (SP-046) | 🔵 Backlog |
| T-0187 | Importer emits Spotlight attributes from facade JSON | EP-017 (SP-046) | 🔵 Backlog |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox/perf pass | EP-017 (SP-046) | 🔵 Backlog |
| T-0189 | End-to-end verification (app-closed search, deep-link, donations succeed) | EP-017 (SP-047) | 🔵 Backlog |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | EP-017 (SP-047) | 🔵 Backlog |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3) | EP-018 (SP-048) | ✅ Done (2026-06-24) |
| T-0192 | Extract `ProjectSession`; move per-project state + methods off `AppEnvironment` (behavior-preserving) | EP-018 (SP-048) | ✅ Verified → `Verified/Task-verified-0192.md` |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` (projectID → session) | EP-018 (SP-048) | ✅ Verified → `Verified/Task-verified-0193.md` |
| T-0194 | Per-window project model — AppKit NSWindow per project (R1/R2/R3) + Welcome; single-instance; File menu | EP-018 (SP-049) | ✅ Verified → `Verified/Task-verified-0194.md` |
| T-0195 | Session manifest persistence + launch restore of all previously-open windows (R4) | EP-018 (SP-049) | ✅ Verified → `Verified/Task-verified-0195.md` |
| T-0196 | Rewrite deep-link handler on new model + scene-`ID` fix (R5); open-flow cross-ref; EP-018 verification | EP-018 (SP-050) | ✅ Verified — archived to Verified/Task-verified-0196.md |

## Full Detail — Implemented Tasks Returned to Backlog

---

## T-0175: Spotlight Integration (umbrella) — Superseded by EP-017

**Status:** ⚪ Superseded
**Date Requested:** 2026-06-23 | **Superseded:** 2026-06-23
**Epic:** EP-017

Originally logged as a single design-gated task. Per user direction it was expanded into a full
Epic — **EP-017: Spotlight Search Integration** — and decomposed into Sprints SP-044–SP-047 and
Tasks T-0176–T-0190. See `docs/Epics/Epic-backlog.md` (EP-017) and
`docs/Scrivi_Spotlight_Integration_Design_v0_1.md`. This entry is retained as a pointer; no work
is tracked under T-0175.

---

## EP-017 Spotlight Tasks (T-0176 – T-0190)

Design reference for every task: `docs/Scrivi_Spotlight_Integration_Design_v0_1.md`. Sprint-level
acceptance criteria live in `docs/Sprints/Sprint-backlog.md`; Epic-level ACs in
`docs/Epics/Epic-backlog.md` (EP-017).

### SP-044 — Design sign-off & ScriviCore indexing facade — ✅ Closed & Verified (2026-06-23)

T-0176–T-0179 verified and archived. Full detail:
[T-0176](Verified/Task-verified-0176.md) · [T-0177](Verified/Task-verified-0177.md) ·
[T-0178](Verified/Task-verified-0178.md) · [T-0179](Verified/Task-verified-0179.md).
Sprint retrospective: `docs/Sprints/Closed/Sprint-SP-044.md`.

### SP-045 — Layer 1: in-app Core Spotlight donations & deep-link

**T-0180 — Entitlements + project config.** ✅ Verified & archived 2026-06-23 — enabled the App
Sandbox capability (the Core Spotlight prerequisite) + `scrivi://` URL scheme; verified by a clean
open→write→save→close cycle under the sandbox (no Console `deny`). Full detail:
[Task-verified-0180.md](Verified/Task-verified-0180.md). *Note:* whether the original
`CSInlineDonation` console errors are resolved is checked later at EP-017 AC9 / T-0189 (needs live
donations, T-0182).

**T-0181 — `ScriviEngine` indexable-records API.** ✅ Verified & archived 2026-06-23 — added
`ScriviEngine.extractSearchableText` + `SearchableContentResult`/`SearchableItemResult` decode types
(pure decode, no backend logic in Swift); proven by interop test #18 (`xcodebuild test` 22/22 green).
Also wired `ScriviInteropTests` into the `ScriviApp` scheme so the whole suite runs from CLI/CI, and
fixed two latent git tests broken under the sandboxed test host. Full detail:
[Task-verified-0181.md](Verified/Task-verified-0181.md).

**T-0182 — Donate / update / delete.** ✅ Verified & archived 2026-06-23 — `SpotlightDonor` donates
on open/save and deletes by domain on close, wired into `AppEnvironment`; the Swift donation path was
proven correct in the debugger (extract→24 records→donate). **Known environment limitation:** the OS
rejects the donations (`SetStoreUpdateService` -4099) on the dev-signed sandboxed build, so nothing
indexes yet — confirmed not our logic / not a debugger artifact / not a hard sandbox deny; deferred to
T-0189 to confirm on a provisioned/installed build (also where AC9 is judged). Full detail:
[Task-verified-0182.md](Verified/Task-verified-0182.md).

**T-0183 — Markdown→plain-text extraction.** ✅ Verified & archived 2026-06-23 — no new code needed:
the `util/MarkdownStrip` core + facade application shipped under T-0178 (per design §4b it lives in
the ScriviCore facade, shared by both layers; Swift does no Markdown handling). Covered by the
MarkdownStrip unit tests + interop test #18. Full detail:
[Task-verified-0183.md](Verified/Task-verified-0183.md).

**T-0184 — Deep-link continuation.** Each item carries a `scrivi://open?project=…&item=…` URL.
App handles `CSSearchableItemActionType` / URL scheme to open the project and select the item.
*Implemented 2026-06-23 (not verified):* `ScriviDeepLink` (URL parse, 4 unit tests) +
`ProjectBookmarkStore` (projectID→security-scoped bookmark — the bookmark work deferred from T-0180:
recorded on panel-open, resolved/refreshed/released around a deep-link open). `AppEnvironment.handleDeepLink`
(URL-scheme path: select-if-open, else resolve bookmark → reopen → select; clear message if no
bookmark) + `handleSpotlightItem` (best-effort continuation). `ScriviApp` wires `onOpenURL` +
`onContinueUserActivity(CSSearchableItemActionType)`; `ManuscriptEditorView` consumes
`pendingNavigationSceneID` into its existing `navigateToSceneID`. pbxproj updated (2 new files, all
3 app targets). macOS build clean; `xcodebuild test` 26/26.
**Known limits (inherent):** (1) cold-start resolves only projects opened before in this app (sandbox
bookmark requirement); (2) the Spotlight-tap continuation carries only the item `uniqueIdentifier`
(no projectID for scenes) and is unreliable on SwiftUI macOS, so `scene:` taps only select when the
project is already open — the `scrivi://` URL scheme is the complete route.
**Verification (overlaps T-0189):** end-to-end from a tapped Spotlight result needs donations to index
first (blocked by T-0182 env issue). URL-scheme path is testable now via
`open "scrivi://open?project=<id>&item=scene:<id>"` against a previously-opened project on a build that
receives URL events.

**T-0184 resumed & completed on the per-window model (2026-06-25 — Implemented, Not Verified).**
EP-018 unblocked this. The core deep link (open/focus the right window, select scene by `scene_…` ID)
was already verified via EP-018 / T-0196. The remaining Spotlight-result *continuation* path
(`onContinueUserActivity(CSSearchableItemActionType)`) was finished and hardened:
- `handleSpotlightItem` now accepts an optional `relatedURL` and **prefers the full donated deep link**
  (`SpotlightDonor` sets it as `relatedUniqueIdentifier`). Because that URL carries the **projectID**,
  a tapped `scene:` result can now open even a **closed** project (via its bookmark) — removing the old
  inherent limit where scene taps only worked when the project was already open. Falls back to the
  uid-only logic when no related URL is present. (`AppEnvironment.swift:344-385`)
- `ScriviApp` recovers that URL from `activity.contentAttributeSet?.relatedUniqueIdentifier` and passes
  it through. (`ScriviApp.swift:189-199`)
- No new files. macOS `ScriviApp` build clean; interop suite 26/26 (incl. `ScriviDeepLink` parse).
**Still not user-verified:** the live Spotlight-tap continuation depends on donations indexing
(the T-0182 `SetStoreUpdateService` env issue) and is acknowledged unreliable on SwiftUI macOS; the
`scrivi://` URL scheme remains the fully-verified route. Full end-to-end Spotlight-tap verification is
T-0189 (SP-047).

### SP-046 — Layer 2: on-disk `.scrivi` importer extension

**T-0185 — Importer extension target + pbxproj.** New Spotlight importer app-extension target in
`Scrivi.xcodeproj`. Per CLAUDE.md, the target and all its source files are added to
`project.pbxproj` in the same step as creating them, before building.

**T-0186 — Link ScriviCore into the extension (Option A).** Make ScriviCore (or its read-only
facade) buildable/linkable into the extension target under the app-extension sandbox. Depends on
the T-0176 build-graph decision.

**T-0187 — Emit attributes from facade JSON.** Importer calls `extractSearchableText` for the
package on disk and maps the JSON to `CSSearchableItemAttributeSet` values. Single indexing
truth shared with Layer 1.

**T-0188 — UTI handling + sandbox/perf pass.** Extension declares it handles
`com.caposoft.scrivi.project`; verify it runs within the app-extension sandbox and indexes a
large fixture project within reasonable time/memory.

### SP-047 — Verification, cross-platform assessment, Epic close

**T-0189 — End-to-end verification.** With Scrivi closed, Spotlight finds project/scene/object
content; selecting a result opens Scrivi at the item; donations now succeed so the original
`CSInlineDonation … SetStoreUpdateService` errors no longer appear (or are documented benign);
no regression to open/save/close.

**T-0190 — Cross-platform assessment + EP-017 verification.** Assess iOS/iPadOS/visionOS use of
the shared Core Spotlight APIs; implement or explicitly defer with rationale. Run final EP-017
acceptance-criteria verification and prepare the Epic for close.

---

## EP-018 Per-Window / Per-Project Window Model Tasks (T-0191 – T-0196)

Design: `docs/Scrivi_PerWindow_Project_Model_Design_v0_1.md` (✅ approved 2026-06-24). Swift-layer
only; no ScriviCore/C++ changes. The V1 spike (T-0191) gates the R3 mechanism before any
production refactor commits.

### SP-048 — Foundation (spike, ProjectSession, registry)

**T-0191 — V1 spike. ✅ Done 2026-06-24.** Throwaway `#if SPIKE_T0191` block in `ScriviApp.swift`
(built, run, observed, removed). **Result:** macOS 26 `WindowGroup(for:)` de-dups by value **only
against already-established windows; NOT race-safe** — two `openWindow(value:)` for the same value
in quick succession produced two windows; re-opening an already-open value focused it.
**Decision:** `OpenProjectRegistry` (T-0193) is the **authoritative R3 guard** (check-and-focus
before `openWindow`); native de-dup is a steady-state backstop only. Critical for restore-all
(T-0195) and deep links, which open windows concurrently. Recorded in design doc §3.2 and SP-048
notes.

**T-0192 — Extract `ProjectSession`.** Move all per-project state (`openProjectResult`,
`viewportLoader`, `projectPreferences`, `timelineModel`, `pendingNavigationSceneID`,
`spotlightDomainIdentifier`, `deepLinkAccessURL`, per-window UI flags) and the `openProject` /
`closeProject` / `onAppResign` / `donateSpotlight` methods off `AppEnvironment` onto a new
`@Observable @MainActor ProjectSession`. Behavior-preserving — app stays single-window this
sprint. New `.swift` files added to `project.pbxproj` in the same step (CLAUDE.md).

**T-0193 — `OpenProjectRegistry`.** Add an `@Observable` registry to `AppEnvironment` mapping
`projectID → ProjectSession`, with register/deregister/lookup. Powers R3 (already-open check) and
R4 (restore manifest). Not yet wired to windowing.

### SP-049 — Windowing & restore

**T-0194 — `WindowGroup(for:)` + Landing window.** Convert the scene to
`WindowGroup(for: ProjectWindowID.self)`; add a Landing/picker window for the no-project state;
open/focus projects via `openWindow(value:)`. Achieves R1/R2/R3 using the T-0191-chosen
mechanism.

**T-0195 — Manifest + restore-all.** Persist the set of open projectIDs (+ frontmost) to
UserDefaults; on launch, resolve each via `ProjectBookmarkStore` (reused from T-0184) and
`openWindow` to restore **all** previously-open windows (Q1). Per-window scene/cursor/scroll
restored by the existing backend open flow — no backend change.

### SP-050 — Deep-link rewrite & verification

**T-0196 — Deep-link rewrite + EP-018 verification.** Rebuild the paused T-0184 handler on the
new model: resolve projectID → registry → focus existing window or `openWindow` → set
`pendingNavigationSceneID` on that window's session. Fix scene matching to use the `scene_…` ID
(not title). Keep `ScriviDeepLink` / `ProjectBookmarkStore` / `SpotlightDonor`. Cross-reference
the per-window model from `Scrivi_Project_Creation_and_Open_Flow_v0_2.md`. Run EP-018 R1–R5
verification; unblock EP-017 AC5.

*Last Updated: 2026-06-29 (T-0123 removed from backlog — Verified and archived to Verified/Task-verified-0123.md; EP-012 closed.)*
