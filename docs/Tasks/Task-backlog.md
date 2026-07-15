# Task Backlog

Tasks listed here are documented and ready for Sprint assignment.

New, unstarted tasks are listed as summary rows. Tasks that have been implemented but returned to the backlog before verification retain their full detail below the index table so no documentation is lost.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0218 | `[Linux]` Qt6/QML CMake skeleton — links `libScriviCore.a` + `scrivi.h` | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0219 | `[Linux]` `ScriviBridge` — QML ↔ C ABI marshalling + `scrivi_free` discipline | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0220 | `[Linux]` Hello-ScriviCore QML window — `scrivi_ensure_local_identity` round-trip | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0221 | `[Linux]` Docker (Qt6 + CMake) + Xvfb + VNC harness | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0222 | `[Linux]` CI job — build the Qt/QML app on relevant commits | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0175 | Spotlight integration (umbrella) — **superseded by EP-017** | EP-017 | ⚪ Superseded |
| T-0184 | Deep-link: result continuation opens project & selects item | EP-017 (SP-045) | ✅ Verified → `Verified/Task-verified-0184.md` |
| T-0185 | New Spotlight importer app-extension target (Xcode-authored) | EP-017 (SP-046) | ⚪ Descoped (I-0057 — `CSImportExtension` non-functional on macOS) |
| T-0186 | Link ScriviCore into the extension (Option A build graph) | EP-017 (SP-046) | ⚪ Descoped (I-0057) |
| T-0187 | Importer emits Spotlight attributes from facade JSON | EP-017 (SP-046) | ⚪ Descoped (I-0057) |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox pass | EP-017 (SP-046) | ⚪ Descoped (I-0057) |
| T-0189 | End-to-end verification (donor search, deep-link tap, donations succeed) | EP-017 (SP-047) | ✅ Verified → `Verified/Task-verified-0189.md` |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | EP-017 (SP-047) | ✅ Verified (DEFERRED) → `Verified/Task-verified-0190.md` |
| T-0197 | Enable Core Spotlight donation on iOS/iPadOS (+ iOS deep-link/bookmark consumer); visionOS when backend links | EP-017 (deferred from T-0190) | 🔵 Backlog |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3) | EP-018 (SP-048) | ✅ Done (2026-06-24) |
| T-0192 | Extract `ProjectSession`; move per-project state + methods off `AppEnvironment` (behavior-preserving) | EP-018 (SP-048) | ✅ Verified → `Verified/Task-verified-0192.md` |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` (projectID → session) | EP-018 (SP-048) | ✅ Verified → `Verified/Task-verified-0193.md` |
| T-0194 | Per-window project model — AppKit NSWindow per project (R1/R2/R3) + Welcome; single-instance; File menu | EP-018 (SP-049) | ✅ Verified → `Verified/Task-verified-0194.md` |
| T-0195 | Session manifest persistence + launch restore of all previously-open windows (R4) | EP-018 (SP-049) | ✅ Verified → `Verified/Task-verified-0195.md` |
| T-0196 | Rewrite deep-link handler on new model + scene-`ID` fix (R5); open-flow cross-ref; EP-018 verification | EP-018 (SP-050) | ✅ Verified — archived to Verified/Task-verified-0196.md |
| T-0198 | Undo/redo + copy buffers design doc & trade studies | EP-019 (SP-051) | ✅ Verified → `Verified/Task-verified-0198.md` |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | EP-019 (SP-051) | ✅ Done (2026-07-06) |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | EP-019 (SP-051) | ✅ Verified → `Verified/Task-verified-0200.md` |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + unit tests | EP-019 (SP-052) | ✅ Verified → `Verified/Task-verified-0201-0203.md` |
| T-0202 | C ABI: `scrivi_history_*` open/record/undo/redo/close | EP-019 (SP-052) | ✅ Verified → `Verified/Task-verified-0201-0203.md` |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | EP-019 (SP-052) | ✅ Verified → `Verified/Task-verified-0201-0203.md` |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | EP-019 (SP-053) | ✅ Verified → `Verified/Task-verified-0204-0206.md` |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | EP-019 (SP-053) | ✅ Verified → `Verified/Task-verified-0204-0206.md` |
| T-0206 | Barriers on structural operations | EP-019 (SP-053) | ✅ Verified → `Verified/Task-verified-0204-0206.md` |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | EP-019 (SP-054) | ✅ Verified → `Verified/Task-verified-0207-0209.md` |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | EP-019 (SP-054) | ✅ Verified → `Verified/Task-verified-0207-0209.md` |
| T-0209 | Session-boundary warning popup | EP-019 (SP-054) | ✅ Verified → `Verified/Task-verified-0207-0209.md` |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge | EP-019 (SP-055) | ✅ Verified → `Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (Trade T2 core interaction) | EP-019 (SP-055) | ✅ Verified → `Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | EP-019 (SP-055) | ✅ Verified → `Verified/Task-verified-0210-0212.md` |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | EP-019 (SP-056) | 🔵 Backlog |
| T-0214 | Buffer UX: keyboard HUD + palette + menu items; paste/cut history integration | EP-019 (SP-056) | 🔵 Backlog |
| T-0215 | History panel (Trade T2 management surface) | EP-019 (SP-057) | 🔵 Backlog |
| T-0216 | Perf/integration fixtures (100k events, 500 KB scene); gitignore migration | EP-019 (SP-057) | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | EP-019 (SP-057) | 🔵 Backlog |
| T-0234 | `[Linux]` **Shell flip** — `main.cpp` → `QApplication`+`QMainWindow` host, landing QML re-hosted via `QQuickWidget`; keep context props + `QFileDialog` picker + Quit; re-verify EP-021 loop over VNC | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0235 | `[Linux]` **Editor shell + read-only continuous viewport** — one `QPlainTextEdit`/`QTextDocument` (read-only, undo disabled), all scene bodies via `openProject` + `scrivi_open_scene` loop, scene-boundary markers + per-scene start offsets (`sceneStartMap` seed) | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0236 | `[Linux]` **Scene navigator** — ordered list + chapter grouping + live titles; click selects + scrolls viewport; display/select only (structure editing = EP-023) | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0237 | `[Linux]` **Verify AC1/AC2 + headless scene-load smoke** — multi-scene fixture asserts all bodies loaded, wired to CI; VNC click-through | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0238 | `[Linux]` **Editable viewport + dirty tracking** — `setReadOnly(false)`, edits mapped to owning scene via offset map, per-scene dirty flags, non-editable/non-deletable boundary markers | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0239 | `[Linux]` **Per-scene auto-save** — `scrivi_save_scene` on ~1.5s debounce + scene-switch + close/app-quit (VNC foreground-quit path); real `.md` on disk | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0240 | `[Linux]` **⌘↩ create scene** in-editor (Linux: **Ctrl+Return**) — save current, create-scene, insert boundary, caret into new segment, update offset map + navigator | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0241 | `[Linux]` **⌘⇧↩ create chapter** in-editor (Linux: **Ctrl+Shift+Return**) — save current, create-chapter, insert boundary, caret into new chapter's first scene, update offset map + navigator | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0242 | `[Linux]` **Verify AC3 + headless save smoke** — edit→`save_scene`→reopen asserts new bytes; VNC type/switch/⌘↩/⌘⇧↩/quit-save | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0243 | `[Linux]` **Scroll → active-scene promotion** — visible region → active scene via offset map; boundary crossing promotes + saves departing scene | EP-022 (SP-063) | ✅ Verified 2026-07-15 |
| T-0244 | `[Linux]` **Navigator ↔ scroll sync** — highlight follows scroll; navigator click takes caret to the clicked scene's start (scrolls into view); no feedback loop | EP-022 (SP-063) | ✅ Verified 2026-07-15 |
| T-0245 | `[Linux]` **Verify AC1/AC4 (scroll)** — VNC scroll-through tracks active scene + highlight; departing scenes saved; navigator click lands caret at the clicked scene's start | EP-022 (SP-063) | ✅ Verified 2026-07-15 |
| T-0246 | `[Linux]` **Cursor placement + focus** — editor takes focus on open, sensible caret, correct caret across boundaries (no jump-to-start), enforce non-editable/non-deletable markers | EP-022 (SP-064) | 🔵 Backlog |
| T-0247 | `[Linux]` **Quit→reopen surface restore** — persist active scene + anchor/focus/scroll on close; apply `openProject` `restored{}` on reopen | EP-022 (SP-064) | 🔵 Backlog |
| T-0248 | `[Linux]` **EP-022 verification + close prep** — full write→save→switch→scroll→quit→reopen loop VNC-verified; `.md` on disk; AC1–AC7 checklist; restore smoke; draft completion summary for user close | EP-022 (SP-064) | 🔵 Backlog |
| T-0249 | `[Linux]` **Manuscript navigation gestures** — Page Forward / Page Backward + jump to absolute manuscript start / end. Gestures/keystrokes **undecided** (laptops & tablets lack extended keyboards — no PageUp/Down/Home/End assumed); pick touch-friendly bindings. Raised during SP-063 VNC verification; unscheduled (likely EP-022 follow-up or EP-026 menus/parity). | EP-022 (unscheduled) | 🔵 Backlog |

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

### SP-046 — Layer 2: on-disk `.scrivi` importer extension — Implemented, Not Verified (2026-06-30)

All four implemented; macOS `ScriviApp` scheme builds clean and the OS registered the Spotlight
extension on install. Awaiting live verification (Spotlight finds `.scrivi` content with the app
closed).

**T-0185 — Importer extension target.** ✅ New macOS app-extension target **ScriviSpotlightImporter**
(bundle `com.caposoft.scrivi.ScriviSpotlightImporter`), created via Xcode's **Spotlight Importer**
template (`CSImportExtension`) and embedded in `ScriviApp`. Authored by Xcode, so the target +
`ImportExtension.swift` + `Info.plist` are managed in `project.pbxproj` by Xcode (no hand-edit; the
CLAUDE.md pbxproj rule applies to *hand-created* files, none here).

**T-0186 — Link ScriviCore (Option A).** ✅ Mirrors the app's recipe via Build Settings on the
extension: own `Build ScriviCore (CMake)` run-script phase → `build/ScriviCore/libScriviCore.a`;
`LIBRARY_SEARCH_PATHS=$(SRCROOT)/build/ScriviCore`, `OTHER_LDFLAGS=-lScriviCore -lc++`,
`SWIFT_INCLUDE_PATHS=$(SRCROOT)/ScriviCore/include/scrivi` (so `import ScriviCore` resolves the C
module). **Required `ENABLE_USER_SCRIPT_SANDBOXING=NO`** (matching the app) — the default `YES`
sandboxed the run-script and made CMake fail to read the root `CMakeLists.txt`.

**T-0187 — Emit attributes from facade JSON.** ✅ `ImportExtension.update(_:forFileAt:)` calls
`scrivi_extract_searchable_text(path)` (same facade as Layer 1; `scrivi_free`'d), decodes the
`scrivi.searchableContent.v1` envelope, and emits one `CSSearchableItemAttributeSet`:
`displayName` = project title; `textContent` = every record's title + description folded together
(so any scene/object content surfaces the package); `keywords` = deduped union. **Note:**
`CSImportExtension` indexes the package as a **single** item, so per-scene deep-linking stays Layer 1's
job (the in-app `CSSearchableIndex` donor) — this on-disk layer makes content findable, by design.

**T-0188 — UTI handling + sandbox.** ✅ `Info.plist` `CSSupportedContentTypes` =
`com.caposoft.scrivi.project` (the app exports this UTI), `CSExtensionLabel` = "Scrivi Project
Importer". Extension runs in the App Sandbox with **User Selected File = Read Only**. Perf pass on a
large fixture deferred to verification.

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

### EP-017 follow-up (deferred from T-0190)

**T-0197 — Enable Core Spotlight donation on iOS/iPadOS.** Broaden the `SpotlightDonor` /
`AppEnvironment` guards from `os(macOS)` to `os(macOS) || os(iOS)`, verify Spotlight entitlements on
the iOS target, and wire the iOS deep-link/bookmark **consumer** side so a tapped Spotlight result can
resolve a persisted project across launches (`ProjectBookmarkStore` is currently macOS-only). visionOS
is out of scope until its `ScriviEngine` backend links (I-0053 covered iOS/iPadOS only). Gated on:
(a) T-0189 done ✅, and (b) iOS bookmark/restore parity. Rationale + assessment in
`Verified/Task-verified-0190.md`.

## EP-019 Undo/Redo History & Copy Buffer Tasks (T-0198 – T-0217)

Design reference for every task: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`
(section numbers below refer to that doc). Sprint-level goals and exit criteria:
`docs/Sprints/Sprint-backlog.md` (SP-051–SP-057). Epic-level ACs: `docs/Epics/Epic-active.md`
(EP-019). Supersedes **I-0019** (⚪ Closed 2026-07-06 — OBE; requirement carried by EP-019 AC1).

### SP-051 — Design sign-off, spike, schemas — ✅ Closed (2026-07-06)

All three tasks complete; sprint retrospective + full spike findings:
`docs/Sprints/Closed/Sprint-SP-051.md`.

**T-0198 — Design doc + trade studies.** ✅ Verified 2026-07-06 (user design sign-off; all six
trades ruled — T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A). Archived:
[Task-verified-0198.md](Verified/Task-verified-0198.md).

**T-0199 — ⌘Z-routing spike.** ✅ Done 2026-07-06 (two live user-observed rounds). The proposed
`UndoManager` proxy **failed** (menu never consulted it; `NSInternalInconsistencyException` per
keystroke via NSTextView's coalescer even with `allowsUndo=false`); the **adopted mechanism** is
first-responder `undo(_:)`/`redo(_:)` action methods + `validateUserInterfaceItem` — confirmed
receiving menu clicks and ⌘Z/⇧⌘Z. Findings in design §8/§12.6; spike code removed (no `.swift`
diff); governs T-0205.

**T-0200 — Schema spec.** ✅ Verified 2026-07-06 — design doc Appendix A (field-level
`scrivi.history.v1`/`scrivi.buffers.v1` spec), repair matrix §6.21, package structure §4/§16a/§17.
Archived: [Task-verified-0200.md](Verified/Task-verified-0200.md).

### SP-052 — Linear engine core

**T-0201 — `HistoryService`.** New `ScriviCore/src/history/`: record (snapshot-diff via
prefix/suffix trim on scalar boundaries), undo/redo on a linear chain, session markers, per-scene
head-text cache (design §3.a, §4, §5). Catch2 unit tests.

**T-0202 — C ABI.** `scrivi_history_open/record_event/record_barrier/undo/redo/close` in
`scrivi.h` + `scrivi_c_api.cpp` per the envelope conventions (design §7). pbxproj/CMake updated in
the same step for any new files (CLAUDE.md rule).

**T-0203 — Swift wrappers.** `ScriviEngine.swift` methods + decode types (pure decode, no logic);
interop tests round-tripping record→undo→redo.

### SP-053 — In-session undo/redo (macOS) — delivers EP-019 AC1 (ex-I-0019)

**T-0204 — `HistoryCapture` + triggers.** New `@MainActor` component owned by `ProjectSession`;
commit triggers per design §4.a (sentence terminators `.` `!` `?`, Return, cursor-move-with-pending,
paste/cut, scene switch, pre-auto-save flush via `flushThenSave`, resign/close); IME and rebuild
guards.

**T-0205 — Apply path + routing.** Ranged `replaceCharacters` inside `sceneBoundaries[segIdx]`
under `isRebuilding`, cursor restore, immediate `engine.saveScene`; `allowsUndo=false` + the
T-0199-validated ⌘Z routing (design §8).

**T-0206 — Barriers.** Structural ops (create/delete/merge/split/reorder) record barrier nodes;
undo stops at a barrier with a clear notice (design §4.5).

### SP-054 — Persistence, sessions, capacity

**T-0207 — Log + checkpoint.** Append-only JSONL + atomic `state.json` checkpoint, torn-line
truncation, log-tail replay, head-hash validation → `externalChange` barrier (design §6).

**T-0208 — Capacity + settings.** Eviction from root with auto-purge of aged-off branches
(never evict root→current path); `historySettings` per Trade T1 decision + Project Settings UI row.

**T-0209 — Session warning.** `crossedSessionBoundary` → popup once per crossing, showing the
boundary wall-clock time (Trade T5 ✅ approved: project-open span + 8 h idle rollover).

### SP-055 — Branching ✅ closed 2026-07-13 (T-0210/T-0211/T-0212 verified → `Verified/Task-verified-0210-0212.md`)

**T-0210 — Tree ops.** Fork-on-record (new sibling becomes primary), `select_branch` re-primary,
auto-purge on eviction; unit tests for the undo-type-fork-reselect scenario (design §5).

**T-0211 — Fork popover.** Inline transient popover at the caret listing fork children (preview +
timestamp), keyboard selection (Trade T2 option A), driven by the `forkAhead` envelope field.
Implements the approved T2 refinements (design §10 T2): appears when undo *lands on* a fork;
dismisses (never obstructs) when the writer undoes past it; appears immediately when redo reaches a
fork; redoing without selecting follows the primary child.

**T-0212 — Stale branches.** Detection thresholds + user-confirmed purge flow.

### SP-056 — Copy buffers

**T-0213 — Buffer store.** `history/buffers.json` (`scrivi.buffers.v1`), 9 numbered slots,
`scrivi_buffers_list/set/clear` + engine wrappers (design §9).

**T-0214 — Buffer UX + history integration.** Keyboard HUD + toggleable palette + Edit-menu items
(Trade T4); paste-from-buffer = `paste` event; cut-into-buffer = `cut` event; copy-into-buffer not
an event (Trade T3 ✅ approved); system pasteboard untouched.

### SP-057 — Panel, perf, verification

**T-0215 — History panel.** Windowed tree view (`scrivi_history_get_tree`), branch selection,
stale badges + purge (Trade T2 option B management surface).

**T-0216 — Perf + migration.** Integration fixtures: 100k-event history, 500 KB single scene;
`.gitignore` migration adding `history/` for existing projects (design §6.c, §11).

**T-0217 — Verification.** Update package-structure + repair-matrix docs; run EP-019 AC1–AC8
verification; prepare Epic for close (user approval required).

---

*Last Updated: 2026-07-13 (SP-055 closed: T-0210/T-0211/T-0212 verified & archived → `Verified/Task-verified-0210-0212.md`; EP-019 AC4 + AC5's deferred branch clauses delivered & verified. SP-056 (copy buffers) next.)*
