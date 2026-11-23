# Sprint SP-238 — `[Linux]` [EP-059] C5: custom relationship types on Linux, as the Mac's

**Status:** 🔵 **PLANNING** — created 2026-11-22 (user, 2026-11-22: *"plan C5-C6"*). ⛔ **Blocked by [SP-237]** (the Mac's surface,
which this one adopts, and the endpoint fixes it relies on).
**Carries:** [T-0472] (C5 on Linux; filed 2026-08-25).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — part **C**: C5.
**Size:** S — the endpoint is bridged (`ScriviBridge::upsertRelationType`); one settings section and one form, in the shape [SP-237] sets.

---

## Plan

The Mac's shape, as [SP-237] rules and builds it (its Q2–Q4): Project Settings ▸ Relationship Types, New Relationship Type…, the
same form, and New Type… in the detail view's Relate… picker (`ObjectDetailDialog::chooseRelation`, [SP-236]). Questions, if any,
are written when [SP-237] closes, from what it built.

## Acceptance criteria

- [ ] AC1 — Linux's Project Settings lists and edits relationship types as the Mac's.
- [ ] AC2 — Relate… offers New Type…; the new type is selected and usable at once.
- [ ] AC3 — A smoke through a real project window; ctest and every smoke green in Docker; CI green.
- [ ] AC4 — Live pass on the Linux VM, on an existing project there.

## Progress log

- **2026-11-22** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-22: *"plan C5-C6"*); carries [T-0472] (existing). ⛔ Blocked by [SP-237].

*Last Updated: 2026-11-22 (Created; blocked by SP-237.)*
