# SP-046: Spotlight — Layer 2: On-Disk `.scrivi` Importer Extension

**Status:** ✅ Closed (2026-07-01, user-approved)
**Epic:** EP-017
**Start Date:** 2026-06-25
**End Date:** 2026-07-01
**Goal (original):** Add a Spotlight importer app-extension so the OS indexes `.scrivi` package contents
on disk even when Scrivi is not running, reading project content via ScriviCore (Option A). Also restore
per-project window size/position across launches (I-0051), a polish item pulled in from the EP-018
windowing work.

**Outcome:** The windowing polish shipped and verified. The Layer 2 on-disk importer was **descoped**
mid-sprint (I-0057): the modern `CSImportExtension` API it was built on is **non-functional on macOS**
(confirmed by Apple DTS, unchanged through macOS 26 Tahoe). EP-017 now delivers Spotlight via the
in-app `CSSearchableIndex` donor (Layer 1) only.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0185 | New Spotlight importer app-extension target (Xcode-authored) | ⚪ Descoped (I-0057 — `CSImportExtension` non-functional on macOS) |
| T-0186 | Link ScriviCore into the extension (Option A build graph) | ⚪ Descoped (I-0057) |
| T-0187 | Importer emits Spotlight attributes from facade JSON | ⚪ Descoped (I-0057) |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox pass | ⚪ Descoped (I-0057) |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0051 | Restored project windows don't remember per-window size/position (stack at default) | ✅ Resolved - Verified (2026-06-29) |
| I-0052 | iOS target fails to build — macOS-only scene/commands in `ScriviApp` | ✅ Resolved - Verified (2026-06-26) |
| I-0053 | iOS `ScriviEngine` stubbed — ScriviCore not built/linked for iOS | ✅ Resolved - Verified (2026-06-29) |
| I-0055 | Restored full-screen project window doesn't return to Full Screen on relaunch (carved out of I-0051) | ✅ Resolved - Verified (2026-06-29) |
| I-0056 | macOS File ▸ Open Project panel won't let you select the `.scrivi` package | ✅ Resolved - Verified (2026-06-29) |
| I-0057 | Spotlight on-disk importer (`CSImportExtension`) never runs on macOS — Layer 2 descoped to the in-app donor | ✅ Resolved - Verified (2026-07-01) |

### Acceptance Criteria (final)

- [x] ~~New importer extension target builds and codesigns; pbxproj updated in the same step as the new files.~~ **Descoped (I-0057)** — built & signed cleanly, but the API doesn't run on macOS; target removed.
- [x] ~~Extension reads project content via ScriviCore (no Swift JSON parsing of `.scrivi`).~~ **Descoped (I-0057)** — boundary was honored, but the extension never runs.
- [x] ~~OS indexes `.scrivi` contents with Scrivi not running; results appear in Spotlight.~~ **Descoped (I-0057)** — impossible via `CSImportExtension` on macOS (Apple DTS). Superseded by the in-app `CSSearchableIndex` donor (findable once the app has opened the project).
- [x] ~~Extension respects the app-extension sandbox; indexing completes within reasonable time on a large fixture project.~~ **Descoped (I-0057).**
- [x] **(I-0051)** Each project window reopens at the same size/position it had when last quit/closed; two side-by-side projects return side-by-side, not stacked at default. *(Verified 2026-06-29. Zoom/maximized-state restore carved out to I-0055 — also Verified 2026-06-29.)*

### Retrospective

**Completed & verified:**
- I-0051 — per-project window size/position restore across launches (the sprint's windowing goal).
- I-0055 — restored full-screen window returns to Full Screen (carved out of I-0051).
- I-0052 / I-0053 — iOS target builds and links the real ScriviCore backend (pulled-in blockers; verified on iPhone 17 Pro + iPad Pro / iOS 27.0).
- I-0054 — iPad Project Settings / Close Project reachability.
- I-0056 — macOS File ▸ Open Project selects the `.scrivi` package.
- I-0057 — Spotlight Layer 2 descope: `CSImportExtension` proven non-functional on macOS; `ScriviSpotlightImporter.appex` target/source/scheme removed; macOS app builds & codesigns clean without it.

**Descoped (not returned to backlog — closed by decision, not incomplete):**
- T-0185 / T-0186 / T-0187 / T-0188 — all four targeted the non-functional `CSImportExtension` API.
  Closed via the I-0057 Option-B decision (adopt the in-app donor). Re-openable only if the MDImporter
  (`.mdimporter` / `CFPlugIn`) path (Option A) is ever pursued.

**What went well:**
- The I-0057 diagnosis was decisive: the *absence* of a crash report + empty process-named logs, cross-checked against a healthy binary/signature/entitlements, correctly pointed away from "our bug" and to a non-functional platform API — confirmed by Apple DTS rather than guessed.
- The Option-A boundary discipline meant the descope cost no core logic: the surviving in-app donor already uses the same `scrivi_extract_searchable_text` facade.

**What to improve:**
- The API viability (does `CSImportExtension` actually run on macOS?) should have been validated in SP-044's design sign-off before an extension target was built. A one-line DTS/forum check would have saved the T-0185–T-0188 build effort.

**Carry-forward notes (→ SP-047):**
- Spotlight now has a **single** path: the in-app `CSSearchableIndex` donor (Layer 1). SP-047's
  end-to-end verification (T-0189) and its acceptance criteria must drop the "app-closed on-disk search"
  and "extension codesign" expectations — content is findable once the app has opened/edited a project.
- The MDImporter alternative (Option A) remains available if never-opened on-disk indexing becomes a
  hard requirement; it is currently deferred.

---

*Closed 2026-07-01 (user-approved). Windowing polish (I-0051/I-0055) and pulled-in iOS/UX fixes shipped; Layer 2 on-disk importer descoped to the in-app donor per I-0057. Next: SP-047 (EP-017 verification & close).*
