# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-047 | Spotlight — verification, cross-platform assessment, Epic close | EP-017 | 🔵 Planning |

> SP-044/SP-045/SP-048/SP-049/SP-050 closed — see `Closed/`. EP-018 is complete & closed. **SP-046 is now active** (see `Sprint-active.md`); SP-047 below is next in Planning.

---

> **SP-046 is now active — see `Sprint-active.md`** (Spotlight Layer 2 importer extension; I-0051 per-window frame restore assigned to it).

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

*Last Updated: 2026-06-25 (SP-046 activated to Sprint-active; I-0051 assigned to it. SP-047 remains in Planning.)*
