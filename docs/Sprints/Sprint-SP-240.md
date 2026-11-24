# Sprint SP-240 — `[Linux]` [EP-059] A9: Delete and Change Kind for objects on Linux, as the Mac's

**Status:** 🔵 **PLANNING** — created 2026-11-23 (user, 2026-11-23: *"plan A8"*). ✅ **Unblocked 2026-11-23** ([SP-239] closed); was blocked by it (the Mac's shape, which
this one adopts, and its core guard).
**Carries:** [T-0703] (A9).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — part **A**: A9.
**Size:** S — both endpoints are bridged (`ScriviBridge::deleteObject`, `ScriviBridge::promoteObject`); one menu and one confirmation in
`ObjectDetailDialog`, in the shape [SP-239] sets.

---

## Plan

The Mac's shape, as [SP-239] rules and builds it. Questions, if any, are written when [SP-239] closes, from what it built.

## Acceptance criteria

- [ ] AC1 — Linux's detail window deletes and changes the kind of objects as the Mac's.
- [ ] AC2 — A smoke through a real project window; ctest and every smoke green in Docker; CI green.
- [ ] AC3 — Live pass on the Linux VM, on an existing project there.

## Progress log

- **2026-11-23** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-23: *"plan A8"*); carries [T-0703] (new). ⛔ Blocked by [SP-239].

*Last Updated: 2026-11-23 (Created; blocked by SP-239.)*
