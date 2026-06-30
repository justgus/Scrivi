# Active Sprint

> **Active: SP-046** (EP-017, Spotlight Layer 2 — on-disk importer extension), activated 2026-06-25. SP-044/SP-045/SP-048/SP-049/SP-050 are closed — see `Closed/`. Next after SP-046: SP-047 (verification & EP-017 close, in Planning).

## SP-046: Spotlight — Layer 2: On-Disk `.scrivi` Importer Extension

**Status:** 🟡 Active
**Epic:** EP-017
**Start Date:** 2026-06-25
**End Date:** —
**Goal:** Add a Spotlight importer app-extension so the OS indexes `.scrivi` package contents on disk
even when Scrivi is not running, reading project content via ScriviCore (Option A). Also restore
per-project window size/position across launches (I-0051), a polish item pulled in from the EP-018
windowing work.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0185 | New Spotlight importer app-extension target (Xcode-authored) | 🟢 Implemented - Not Verified |
| T-0186 | Link ScriviCore into the extension (Option A build graph) | 🟢 Implemented - Not Verified |
| T-0187 | Importer emits Spotlight attributes from facade JSON | 🟢 Implemented - Not Verified |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox pass | 🟢 Implemented - Not Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0051 | Restored project windows don't remember per-window size/position (stack at default) | ✅ Resolved - Verified (2026-06-29) |
| I-0052 | iOS target fails to build — macOS-only scene/commands in `ScriviApp` | ✅ Resolved - Verified (2026-06-26) |
| I-0053 | iOS `ScriviEngine` stubbed — ScriviCore not built/linked for iOS | ✅ Resolved - Verified (2026-06-29) |
| I-0055 | Restored full-screen project window doesn't return to Full Screen on relaunch (carved out of I-0051) | ✅ Resolved - Verified (2026-06-29) |
| I-0056 | macOS File ▸ Open Project panel won't let you select the `.scrivi` package | ✅ Resolved - Verified (2026-06-29) |

### Acceptance Criteria

- [ ] New importer extension target builds and codesigns; pbxproj updated in the same step as the new files.
- [ ] Extension reads project content via ScriviCore (no Swift JSON parsing of `.scrivi`).
- [ ] OS indexes `.scrivi` contents with Scrivi not running; results appear in Spotlight.
- [ ] Extension respects the app-extension sandbox; indexing completes within reasonable time on a large fixture project.
- [x] **(I-0051)** Each project window reopens at the same size/position it had when last quit/closed; two side-by-side projects return side-by-side, not stacked at default. *(Verified 2026-06-29. The zoom/maximized-state restore carved out to **I-0055** — Open: a restored-maximized window fills the screen but is not truly zoomed.)*

### Sprint Notes

- **Option A boundary (from SP-044 / T-0176):** the importer reads `.scrivi` content via ScriviCore
  (link the core or its read-only facade), NOT by parsing JSON in Swift. Same boundary the in-app
  donor obeys.
- **pbxproj discipline (CLAUDE.md):** SP-046 adds a new **app-extension target** plus its source
  files. Every new `.cpp`/`.hpp`/`.swift` and the target itself must be added to
  `Scrivi.xcodeproj/project.pbxproj` in the same step as creating them, before building — the user
  builds/commits from the command line, so missing entries break the build.
- **I-0051 (per-window frame restore):** pulled into this sprint as a windowing-polish follow-up to
  EP-018. Root cause + proposed direction are in `docs/Issues/Issue-backlog.md` — persist a
  per-projectID frame/zoom (e.g. `scrivi.projectWindow.<projectID>.frame`) written on
  resize/move/close, and apply it in `ProjectWindowController.init` instead of `window.center()`,
  clamping off-screen frames back on-screen. Swift-layer only; relates to the older single-window
  issue I-0017 (fold its zoom-timing problem in here).

### Retrospective

*(to be completed at sprint close)*

---

*Last Updated: 2026-06-25 (SP-046 activated for EP-017 Layer 2; I-0051 assigned to this sprint. SP-045 closed earlier today.)*
