# SP-014: Infrastructure — Keychain Identity and JsonDoc Double

**Status:** ✅ Closed
**Epic:** EP-006: Swift Interop and Apple Shell
**Goal:** Harden the Apple platform service layer before UI work begins: replace the in-memory `PrototypeSecureStore` with a real `KeychainSecureStore` so identity persists across launches, and add `setDouble`/`getDouble` to `JsonDoc` so `ScrollPosition` is stored correctly on disk.
**Start Date:** 2026-05-28
**End Date:** 2026-05-29
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0048 | `JsonDoc` Double Support — `setDouble` / `getDouble` | High | ✅ Verified |
| T-0049 | `KeychainSecureStore` — macOS Keychain Implementation | Critical | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

Both tasks were pure infrastructure with no SwiftUI dependency. T-0048 touched only C++ (`JsonDoc`) and the workspace-state schema — no pbxproj changes. T-0049 added two new files to `platforms/apple/Sources/ScriviCoreAdapter/` and required the `Security` framework linker flag in `Package.swift`.

Interop tests remained hermetic after T-0049 — Keychain wiring does not affect the test isolation model.

### Retrospective

Clean sprint: two well-scoped infrastructure tasks, both implemented and verified in a single session. Architecture v0.3 Open Questions 2 and 3 are now resolved. EP-006 can proceed to UI work (macOS app target, SwiftUI shell).

---

*Closed: 2026-05-29 (T-0048, T-0049 verified; user approved Sprint close)*
