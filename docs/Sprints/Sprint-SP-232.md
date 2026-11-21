---
sprint: SP-232
epic: EP-059
status: Planning
platform: Linux
created: 2026-11-21
---

# SP-232 — `[Linux]` [EP-059] F: the project window's toolbar

**Status:** 🔵 **Planning** — scope recorded, questions not yet drawn up (user, 2026-11-21: *"I'd like to scan the Epic for any tasks
that involve creating a menu bar of buttons across the top like macOS and iOS have. If missing, I'd like to make sure its added to the
Epic as a new Sprint and AC."*). Planned in full after [SP-231].
**Carries:** [T-0694].
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — part **F** (F1), new 2026-11-21. After [SP-231]: the toolbar's
View item carries Show World Warnings, which SP-231 builds.

---

## What the scan found (2026-11-21)

- **EP-059 had no toolbar AC**, and the gap review ([`../Scrivi_Linux_Capability_Gap_Review_v0_1.md`](../Scrivi_Linux_Capability_Gap_Review_v0_1.md))
  lists no toolbar gap, though it read the toolbar; only G8 names the toolbar's Navigate item.
- **[I-0244]** has recorded since 2026-09-22 that Linux has **no `QToolBar` at all**; it was ruled (2026-09-26) to become its own
  Epic after [EP-043], and none was filed. Re-counted 2026-11-21: still zero `QToolBar` / `addToolBar` in `platforms/linux/src/`.
- **The Mac's toolbar** (`ProjectToolbar.swift`, the iPad's and iPhone's the same with a few iOS-only items):
  Inspector (toggle) · Timeline (toggle) · Project Settings · Worlds · **Navigate** ▾ (Go to Scene / Chapter / Manuscript Start / End)
  · **New** ▾ (Scene, Chapter) · **Merge** ▾ (Scene, Chapter) · **View** ▾ (World Warnings, Markup Hints, Buffers) · **Import** ▾ ·
  **Export** ▾ (incl. Copy as HTML) · Close Project. Every item is a verb the menus already have.
- **Linux has every one of those verbs in its menus** (Go To from [SP-230]; Show World Warnings from [SP-231]), so the work is
  surfacing: a `QToolBar` whose items are the menus' own `QAction`s.

## Questions to draw up when planned

Which items (the Mac's eleven, or a subset); icons (theme icons vs bundled); text under icons or icons only; whether the Navigator
gets a toggle there (the Mac's sidebar button comes with its split view); whether it can be hidden (View ▸ Show Toolbar); the
landing window (its own window, with no menu bar) has none.

## Acceptance Criteria (draft)

- [ ] AC1 — (F1) A toolbar across the top of every project window with the Mac's items, each running the same action as its menu
      item and enabled/disabled with it.
- [ ] AC2 — Linux ctest (non-root, tests on), the smokes, CI green.
- [ ] AC3 — Live pass on the Linux VM.

## Progress log

- **2026-11-21** — 🔵 **Created in Planning**, issued via `next-id.py`; carries [T-0694]. Planned in full after [SP-231].

*Last Updated: 2026-11-21 (🔵 **Created in Planning**.)*
