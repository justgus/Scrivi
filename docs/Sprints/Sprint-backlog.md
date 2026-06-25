# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-046 | Spotlight — Layer 2: on-disk `.scrivi` importer extension | EP-017 | 🔵 Planning |
| SP-047 | Spotlight — verification, cross-platform assessment, Epic close | EP-017 | 🔵 Planning |

> SP-044/SP-045/SP-048/SP-049/SP-050 closed — see `Closed/`. EP-018 is complete & closed. **No sprint is currently active** — SP-046 (below) is next up for EP-017; activate it to start.

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

*Last Updated: 2026-06-25 (SP-045 closed with user approval — no active sprint. SP-046–SP-047 remain in Planning; SP-046 is next to activate.)*
