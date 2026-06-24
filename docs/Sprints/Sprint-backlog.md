# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-046 | Spotlight — Layer 2: on-disk `.scrivi` importer extension | EP-017 | 🔵 Planning |
| SP-047 | Spotlight — verification, cross-platform assessment, Epic close | EP-017 | 🔵 Planning |
| SP-049 | Per-window model — windowing & restore-all-windows | EP-018 | 🔵 Planning |
| SP-050 | Per-window model — deep-link rewrite & EP-018 verification | EP-018 | 🔵 Planning |

> SP-044 closed 2026-06-23 — see `Closed/Sprint-SP-044.md`. SP-045 activated 2026-06-23 — see `Sprint-active.md`. SP-048 (EP-018) activated 2026-06-24 — see `Sprint-active.md`.

---

## SP-046: Spotlight — Layer 2: On-Disk `.scrivi` Importer Extension

**Status:** 🔵 Planning
**Epic:** EP-017
**Goal:** Add a Spotlight importer app-extension so the OS indexes `.scrivi` package contents on disk even when Scrivi is not running, reading project content via ScriviCore (Option A).

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0185 | New Spotlight importer app-extension target + pbxproj wiring | 🔵 Backlog |
| T-0186 | Link ScriviCore (or facade) into the extension (Option A build graph) | 🔵 Backlog |
| T-0187 | Importer emits Spotlight attributes from facade JSON | 🔵 Backlog |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox/perf pass | 🔵 Backlog |

### Acceptance Criteria

- [ ] New importer extension target builds and codesigns; pbxproj updated in the same step as the new files.
- [ ] Extension reads project content via ScriviCore (no Swift JSON parsing of `.scrivi`).
- [ ] OS indexes `.scrivi` contents with Scrivi not running; results appear in Spotlight.
- [ ] Extension respects the app-extension sandbox; indexing completes within reasonable time on a large fixture project.

---

## SP-047: Spotlight — Verification, Cross-Platform Assessment, Epic Close

**Status:** 🔵 Planning
**Epic:** EP-017
**Goal:** Full end-to-end verification of both layers, assess iOS/iPadOS/visionOS, confirm the original donation errors are resolved, and close EP-017.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0189 | End-to-end verification (app-closed search, deep-link, donations succeed) | 🔵 Backlog |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | 🔵 Backlog |

### Acceptance Criteria

- [ ] All EP-017 acceptance criteria verified by the user.
- [ ] Original `CSInlineDonation … SetStoreUpdateService` errors no longer appear (or documented benign).
- [ ] iOS/iPadOS/visionOS Spotlight either implemented or explicitly deferred with rationale.
- [ ] `ctest` green; macOS build + extension codesign clean; no regression to project open/save/close.

---

## SP-049: Per-Window Model — Windowing & Restore-All-Windows

**Status:** 🔵 Planning
**Epic:** EP-018
**Goal:** Convert the app to one window per project and restore all previously-open windows on relaunch, building on the SP-048 foundation and the R3 mechanism chosen by the T-0191 spike.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0194 | Convert scene to `WindowGroup(for: ProjectWindowID.self)` + Landing window; wire `openWindow` (R1/R2/R3) | 🔵 Backlog |
| T-0195 | Session manifest persistence + launch restore of all previously-open windows (R4) | 🔵 Backlog |

### Acceptance Criteria

- [ ] Multiple distinct projects open simultaneously, one per window (R1/R2).
- [ ] Re-opening an already-open project focuses its existing window (R3), using the spike-chosen mechanism.
- [ ] On relaunch, all previously-open project windows are restored (R4); per-window scene/cursor/scroll restored by the existing backend open flow.
- [ ] macOS build/codesign clean; `ctest` green; no regression to open/save/close.

---

## SP-050: Per-Window Model — Deep-Link Rewrite & EP-018 Verification

**Status:** 🔵 Planning
**Epic:** EP-018
**Goal:** Rewrite the deep-link handler (paused EP-017 T-0184) on the per-window model, fix the scene-ID matching, cross-reference the open-flow doc, and verify EP-018.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0196 | Rewrite deep-link handler on new model + scene-`ID` fix (R5); open-flow v0.2 cross-ref; EP-018 verification | 🔵 Backlog |

### Acceptance Criteria

- [ ] A deep link opens/focuses the target project's window and selects the item by its `scene_…` ID (R5).
- [ ] Reuses `ScriviDeepLink` / `ProjectBookmarkStore` / `SpotlightDonor`; the single-project switch control flow is gone.
- [ ] `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` cross-references the per-window model.
- [ ] EP-018 R1–R5 verified by the user; EP-017 AC5 unblocked.

---

*Last Updated: 2026-06-24 (EP-018 added: SP-049–SP-050 planned; SP-048 activated to Sprint-active. SP-044 closed; SP-045 active; SP-046–SP-047 remain in Planning.)*
